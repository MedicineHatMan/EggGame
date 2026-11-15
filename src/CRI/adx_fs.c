#include <Dolphin/os.h>
#include <CRI/adx_fs.h>
#include <CRI/adx.h>

#pragma auto_inline off

#define ADXF_LE32(p) ((p)[0] | ((p)[1] << 8) | ((p)[2] << 16) | ((p)[3] << 24))

static inline void adxf_SetCmdHstry(int cmdid, int fg, int prm1, int prm2, int prm3)
{
	ADXF_CMD_HSTRY* h;

	adxf_hstry_no = adxf_hstry_no % ADXF_HSTRY_NUM;
	h             = &adxf_cmd_hstry[adxf_hstry_no];
	if (fg == 0) {
		adxf_cmd_ncall[cmdid]++;
	}
	h->cmdid  = cmdid;
	h->fg     = fg;
	h->ncall  = adxf_cmd_ncall[cmdid];
	h->prm[0] = prm1;
	h->prm[1] = prm2;
	h->prm[2] = prm3;
	adxf_hstry_no++;
}

/* Drops the stream joint made over the caller's buffer once reading has ended */
static inline void adxf_DestroySj(ADXF adxf)
{
	if (adxf->sj != 0 && adxf->sjflag == 0) {
		if (adxf_ocbi_fg == 1) {
			ADXF_Ocbi(adxf->buf, adxf->bsize);
		}
		SJ_Destroy(adxf->sj);
		adxf->sj = 0;
	}
}

static inline int adxf_GetStat(ADXF adxf)
{
	if (adxf == 0) {
		ADXERR_CallErrFunc1("E9040832:'adxf' is NULL.(ADXF_GetStat)");
		return ADXF_ERR_PRM;
	}
	return adxf->stat;
}

static inline void adxf_Stop(ADXF adxf)
{
	adxf_SetCmdHstry(ADXF_CMD_STOP, 0, (int)adxf, -1, -1);
	if (adxf == 0) {
		ADXERR_CallErrFunc1("E9040822:'adxf' is NULL.(ADXF_Stop)");
		return;
	}
	if (adxf->stat == ADXF_STAT_STOP) {
		return;
	}
	if (adxf->stat == ADXF_STAT_READEND) {
		adxf->stat = ADXF_STAT_STOP;
		return;
	}
	if (adxf->stm == 0) {
		ADXERR_CallErrFunc1("E9040823:'adxf->stm' is NULL.(ADXF_Stop)");
		return;
	}
	ADXSTM_Stop(adxf->stm);
	ADXCRS_Lock();
	adxf->rdsct = ADXSTM_Tell(adxf->stm) - adxf->skpos;
	adxf_DestroySj(adxf);
	adxf->stat = ADXF_STAT_STOP;
	ADXCRS_Unlock();
	adxf_SetCmdHstry(ADXF_CMD_STOP, 1, (int)adxf, -1, -1);
}

static inline int adxf_ReadNw(ADXF adxf, int nsct, void* buf)
{
	if ((unsigned int)buf & 0x1F) {
		ADXERR_CallErrFunc1("E0120401:'buf' isn't 32byte alignment.(ADXF_ReadNw)");
		return ADXF_ERR_PRM;
	}
	return ADXF_ReadNw32(adxf, nsct, buf);
}

static inline ADXF adxf_CreateAdxFs(void)
{
	ADXF adxf;
	int i;

	adxf = 0;
	for (i = 0; i < ADXF_MAX_OBJ; i++) {
		if (adxf_obj[i].used == 0) {
			adxf = &adxf_obj[i];
			break;
		}
	}
	if (adxf == 0) {
		return 0;
	}
	if ((adxf->stm = ADXSTM_Create(0, 0x100)) == 0) {
		ADXERR_CallErrFunc1("E02111001:can't create stm handle (adxf_CreateAdxFs)");
		return 0;
	}
	adxf->stat       = ADXF_STAT_STOP;
	adxf->rdstpos    = 0;
	adxf->rqsct      = 0;
	adxf->rdsct      = 0;
	adxf->rqrdsct    = 0x200;
	adxf->sjflag     = 0;
	adxf->sj         = 0;
	adxf->stopnw_flg = 0;
	adxf->used       = 1;
	return adxf;
}

static inline int adxf_ChkPrmPt(int ptid, ADXF_PTINFO* ptinfo)
{
	if (ptid < 0 || ptid >= ADXF_MAX_PTID) {
		ADXERR_CallErrFunc1("E9040801:partition ID is range outside.(adxf_ChkPrmPt)");
		return ADXF_ERR_PRM;
	}
	if (ptinfo == 0) {
		ADXERR_CallErrFunc1("E9040802:'ptinfo' is NULL.(adxf_ChkPrmPt)");
		return ADXF_ERR_PRM;
	}
	return 0;
}

int adxf_LoadPtBothNw(int ptid, int afsptid, int afsflid, const char* fname, void* dir, ADXF_PTINFO* ptinfo, void* tmpbuf, int tbsize,
                      int rev);
int ADXF_GetPtStatEx(int ptid);

// 0x802290D0
int ADXF_LoadPartitionNw(int ptid, const char* fname, void* dir, ADXF_PTINFO* ptinfo)
{
	return adxf_LoadPtBothNw(ptid, 0, 0, fname, dir, ptinfo, (void*)(((unsigned int)adxf_tmpbuf_area + 0x20) & ~0x1F), ADXF_SCT, 0);
}

/* Loads partition ptid's table from the file fname or when it is null from entry afsflid of partition afsptid */
// 0x80228E80
int adxf_LoadPtBothNw(int ptid, int afsptid, int afsflid, const char* fname, void* dir, ADXF_PTINFO* ptinfo, void* tmpbuf, int tbsize,
                      int rev)
{
	int ret;
	int ofst;
	int fnsct;

	if (tmpbuf == 0) {
		ADXERR_CallErrFunc1("E1110701:'tmpbuf' is null.(load partition)");
		return ADXF_ERR_PRM;
	}
	if (tbsize <= 0) {
		ADXERR_CallErrFunc1("E1110702:'tbsize' <= 0.(load partition)");
		return ADXF_ERR_PRM;
	}
	ret = adxf_ChkPrmPt(ptid, ptinfo);
	if (ret < 0) {
		return ret;
	}
	if (adxf_afs_ptid >= 0) {
		ADXERR_CallErrFunc1("E0042401:multi-load partition.(load partition)");
		return -1;
	}
	adxf_afs_ptid     = ptid;
	adxf_ldptnw_nfile = 0;
	memset(ptinfo, 0, sizeof(ADXF_PTINFO));
	adxf_ptinfo[ptid] = ptinfo;
	ptinfo->next      = 0;
	if (fname == 0) {
		adxf_afs_hndl = ADXF_OpenAfs(afsptid, afsflid);
		if (adxf_afs_hndl == 0) {
			ADXERR_CallErrFunc1("E9040804:can't open file.(load partition)");
			return -1;
		}
		ret = ADXF_GetFnameRangeEx(afsptid, afsflid, ptinfo->fname, &dir, &ofst, &fnsct);
		if (ret < 0) {
			return ret;
		}
		ptinfo->ofst = ofst;
	} else {
		adxf_afs_hndl = ADXF_Open(fname, dir);
		if (adxf_afs_hndl == 0) {
			ADXERR_CallErrFunc1("E9040804:can't open file.(load partition)");
			return -1;
		}
		strncpy(ptinfo->fname, fname, ADXF_FNAME_LEN);
	}
	ptinfo->curdir  = dir;
	ptinfo->type    = 0;
	ptinfo->rev     = rev;
	adxf_tmpbuf     = tmpbuf;
	adxf_tbsize_sct = tbsize / ADXF_SCT;
	ret             = adxf_ReadNw(adxf_afs_hndl, adxf_tbsize_sct, tmpbuf);
	if (ret < 0) {
		ADXF_Close(adxf_afs_hndl);
		return ret;
	}
	ptinfo->nfile = 0;
	return 0;
}

// 0x80228E2C
void adxf_CloseLdptnwHn(void)
{
	ADXF_Close(adxf_afs_hndl);
	adxf_afs_hndl     = 0;
	adxf_afs_ptid     = -1;
	adxf_ldptnw_nfile = 0;
	adxf_tbsize_sct   = 0;
}

// 0x80228BFC
void ADXF_StopPtLd(void)
{
	if (adxf_afs_hndl != 0 && adxf_afs_ptid >= 0) {
		if (adxf_GetStat(adxf_afs_hndl) != ADXF_STAT_STOP) {
			adxf_Stop(adxf_afs_hndl);
		}
		adxf_CloseLdptnwHn();
	}
}

// 0x80228BDC
int ADXF_GetPtStatEx(int ptid)
{
	return ADXF_GetPtStat(ptid);
}

/* Goes on reading the partition table being loaded a buffer at a time and says how far it has got */
// 0x8022880C
int ADXF_GetPtStat(int ptid)
{
	int stat;
	ADXF_PTINFO* pt;
	unsigned short* top16;
	int* sz32;
	int* top32;
	unsigned short* sz16;
	int i;
	int n;
	int len;
	int nsct;
	unsigned char* p;

	sz16  = 0;
	top16 = 0;
	sz32  = 0;
	top32 = 0;
	if (ptid != adxf_afs_ptid) {
		ADXERR_CallErrFunc1("E0041303:illigal parameter 'ptid'.(ADXF_GetPtStat)");
		return ADXF_ERR_PRM;
	}
	stat = adxf_GetStat(adxf_afs_hndl);
	if (stat != ADXF_STAT_READEND) {
		return stat;
	}
	pt = adxf_ptinfo[ptid];
	if (pt->rev == 1) {
		top32 = pt->sz.i;
		sz32  = top32 + 1;
	} else {
		top16 = pt->sz.s;
		sz16  = top16 + 1;
	}
	if (pt->nfile == 0) {
		if (memcmp(adxf_tmpbuf, "AFS", 3) != 0) {
			ADXERR_CallErrFunc1("E0040701:Illigal format(not AFS).(ADXF_GetPtStat)");
			adxf_CloseLdptnwHn();
			return ADXF_STAT_ERROR;
		}
		n = ADXF_LE32(adxf_tmpbuf + 4);
		if (n > 0x10000) {
			ADXERR_CallErrFunc1("E0040702:Illigal number of file.(ADXF_GetPtStat)");
			adxf_CloseLdptnwHn();
			return ADXF_STAT_ERROR;
		}
		pt->nentry = n;
		pt->nfile  = (unsigned short)n;
		if (pt->rev == 1) {
			pt->size = ((pt->nfile + 1) * 4 + 0x11C) & ~3;
		} else {
			pt->size = ((pt->nfile + 1) * 2 + 0x11A) & ~3;
		}
		if (pt->rev == 1) {
			*top32 = ADXF_LE32(adxf_tmpbuf + 8) / ADXF_SCT;
		} else {
			*top16 = ADXF_LE32(adxf_tmpbuf + 8) / ADXF_SCT;
		}
		i = 3;
	} else {
		i = 1;
	}
	len = (adxf_tbsize_sct * ADXF_SCT) / 4;
	for (; i < len; i += 2) {
		if (pt->rev == 1) {
			p    = adxf_tmpbuf + i * 4;
			n    = ADXF_LE32(p);
			nsct = n / ADXF_SCT;
			if (n % ADXF_SCT > 0) {
				nsct++;
			}
			sz32[adxf_ldptnw_nfile++] = nsct;
		} else {
			p    = adxf_tmpbuf + i * 4;
			n    = ADXF_LE32(p);
			nsct = n / ADXF_SCT;
			if (n % ADXF_SCT > 0) {
				nsct++;
			}
			sz16[adxf_ldptnw_nfile++] = nsct;
		}
		if (adxf_ldptnw_nfile >= pt->nfile) {
			stat = ADXF_STAT_READEND;
			adxf_CloseLdptnwHn();
			break;
		}
	}
	if (i >= len) {
		if (adxf_ReadNw(adxf_afs_hndl, adxf_tbsize_sct, adxf_tmpbuf) < 0) {
			stat = ADXF_STAT_ERROR;
			adxf_CloseLdptnwHn();
		} else {
			stat = adxf_afs_hndl->stat;
		}
	}
	return stat;
}

// 0x80228594
ADXF ADXF_Open(const char* fname, void* dir)
{
	ADXF adxf;
	int ret;

	adxf_SetCmdHstry(ADXF_CMD_OPEN, 0, (int)fname, (int)dir, -1);
	adxf = adxf_CreateAdxFs();
	if (adxf != 0) {
		if (fname == 0) {
			ADXERR_CallErrFunc1("E9081901:illigal parameter fname=null.(ADXF_Open)");
			ret = -1;
		} else {
			adxf->dir   = dir;
			adxf->fname = fname;
			adxf->skpos = 0;
			ADXSTM_BindFile(adxf->stm, adxf->fname, adxf->dir, 0, ADXSTM_RD_ALL);
			if (ADXSTM_GetStat(adxf->stm) == ADXF_STAT_ERROR) {
				ADXSTM_ReleaseFile(adxf->stm);
				ret = -1;
			} else {
				adxf->fsize = ADXSTM_GetFileLen(adxf->stm);
				adxf->fnsct = adxf->fsize / ADXF_SCT;
				adxf->ofst2 = ret = 0;
				adxf->nsct        = adxf->fnsct;
			}
		}
		if (ret < 0) {
			ADXF_Close(adxf);
			adxf = 0;
		}
	}
	adxf_SetCmdHstry(ADXF_CMD_OPEN, 1, (int)fname, (int)dir, -1);
	return adxf;
}

// 0x802282F8
ADXF ADXF_OpenAfs(int ptid, int flid)
{
	ADXF adxf;
	int ret;
	void* dir;
	int ofst;
	int fnsct;
	char fname[ADXF_FNAME_LEN];

	adxf_SetCmdHstry(ADXF_CMD_OPEN_AFS, 0, ptid, flid, -1);
	adxf = adxf_CreateAdxFs();
	if (adxf != 0) {
		if (ADXF_GetFnameRangeEx(ptid, flid, fname, &dir, &ofst, &fnsct) < 0) {
			ret = ADXF_ERR_PRM;
		} else {
			adxf->ofst  = ofst;
			adxf->dir   = dir;
			adxf->fname = adxf_ptinfo[ptid]->fname;
			adxf->nsct  = fnsct;
			adxf->ofst2 = ofst;
			adxf->skpos = 0;
			ADXSTM_BindFile(adxf->stm, adxf->fname, adxf->dir, adxf->ofst2, adxf->nsct);
			if (ADXSTM_GetStat(adxf->stm) == ADXF_STAT_ERROR) {
				ADXSTM_ReleaseFile(adxf->stm);
				ret = -1;
			} else {
				adxf->fnsct = fnsct;
				ret         = 0;
				adxf->fsize = fnsct * ADXF_SCT;
			}
		}
		if (ret < 0) {
			ADXF_Close(adxf);
			adxf = 0;
		}
	}
	adxf_SetCmdHstry(ADXF_CMD_OPEN_AFS, 1, ptid, flid, -1);
	return adxf;
}

// 0x80228038
void ADXF_Close(ADXF adxf)
{
	adxf_SetCmdHstry(ADXF_CMD_CLOSE, 0, (int)adxf, -1, -1);
	if (adxf == 0) {
		return;
	}
	if (adxf->stat == ADXF_STAT_READING) {
		adxf_Stop(adxf);
	}
	if (adxf->stm != 0) {
		ADXSTM_ReleaseFile(adxf->stm);
		ADXSTM_Destroy(adxf->stm);
	}
	memset(adxf, 0, sizeof(ADX_FS));
	adxf_SetCmdHstry(ADXF_CMD_CLOSE, 1, (int)adxf, -1, -1);
}

// 0x80227FDC
void ADXF_CloseAll(void)
{
	int i;

	for (i = 0; i < ADXF_MAX_OBJ; i++) {
		if (adxf_obj[i].used == 1) {
			ADXF_Close(&adxf_obj[i]);
		}
	}
}

/* Stripped by the link their strings stay in the pool */
int ADXF_ReadSj32(ADXF adxf, int nsct, SJ sj)
{
	if (adxf == 0) {
		ADXERR_CallErrFunc1("E9040811:'adxf' is NULL.(ADXF_ReadSj32)");
		return ADXF_ERR_PRM;
	}
	if (nsct < 0) {
		ADXERR_CallErrFunc1("E9040812:'nsct'is negative.(ADXF_ReadSj32)");
		return ADXF_ERR_PRM;
	}
	if (sj == 0) {
		ADXERR_CallErrFunc1("E9040813:'sj'is NULL.(ADXF_ReadSj32)");
		return ADXF_ERR_PRM;
	}
	return 0;
}

/* Starts reading nsct sectors into buf through a stream joint made over it */
// 0x80227D18
int ADXF_ReadNw32(ADXF adxf, int nsct, void* buf)
{
	SJ sj;
	int ret;
	int rest;

	adxf_SetCmdHstry(ADXF_CMD_READ_NW32, 0, (int)adxf, nsct, (int)buf);
	if (adxf == 0) {
		ADXERR_CallErrFunc1("E9040816:'adxf' is NULL.(ADXF_ReadNw32)");
		return ADXF_ERR_PRM;
	}
	if (nsct < 0) {
		ADXERR_CallErrFunc1("E9040817:'nsct' is negative.(ADXF_ReadNw32)");
		return ADXF_ERR_PRM;
	}
	if (buf == 0) {
		ADXERR_CallErrFunc1("E9040818:'buf' is NULL.(ADXF_ReadNw32)");
		return ADXF_ERR_PRM;
	}
	if (adxf->stat == ADXF_STAT_READING) {
		return 0;
	}
	if (adxf->sj != 0) {
		ADXERR_CallErrFunc1("E9040821:'sj' is NULL.(ADXF_ReadNw32)");
		return -1;
	}
	sj = SJRBF_Create(buf, nsct * ADXF_SCT, 0);
	if (sj == 0) {
		return -2;
	}
	adxf->buf   = buf;
	adxf->bsize = nsct * ADXF_SCT;
	ADXCRS_Lock();
	if (adxf_ocbi_fg == 1) {
		ADXF_Ocbi(adxf->buf, adxf->bsize);
	}
	if (ADXSTM_GetStat(adxf->stm) != ADXF_STAT_STOP) {
		ADXSTM_Stop(adxf->stm);
	}
	ADXCRS_Lock();
	adxf->rdstpos = adxf->ofst + adxf->skpos;
	rest          = adxf->fnsct - adxf->skpos;
	adxf->rqsct   = nsct < rest ? nsct : rest;
	adxf->rdsct = ret = 0;
	if (adxf->rqsct == 0) {
		adxf->stat = ADXF_STAT_READEND;
	} else {
		ADXSTM_SetEos(adxf->stm, -1);
		ADXSTM_SetSj(adxf->stm, sj);
		ADXSTM_SetReqRdSize(adxf->stm, adxf->rqrdsct);
		adxf->stat       = ADXF_STAT_READING;
		adxf->stopnw_flg = 0;
		ADXSTM_SetPause(adxf->stm, 0);
		ADXSTM_Seek(adxf->stm, adxf->skpos);
		ADXSTM_Start2(adxf->stm, adxf->rqsct);
		ret = adxf->rqsct;
	}
	ADXCRS_Unlock();
	if (ret <= 0) {
		SJ_Destroy(sj);
	} else {
		adxf->sj = sj;
	}
	adxf->sjflag = 0;
	ADXCRS_Unlock();
	adxf_SetCmdHstry(ADXF_CMD_READ_NW32, 1, (int)adxf, nsct, (int)buf);
	return ret;
}

int ADXF_ReadNw(ADXF adxf, int nsct, void* buf)
{
	return adxf_ReadNw(adxf, nsct, buf);
}

int ADXF_StopNw(ADXF adxf)
{
	if (adxf == 0) {
		ADXERR_CallErrFunc1("E2092301:'adxf' is NULL.(ADXF_StopNw)");
		return ADXF_ERR_PRM;
	}
	if (adxf->stm == 0) {
		ADXERR_CallErrFunc1("E2092302:'adxf->stm' is NULL.(ADXF_StopNw)");
		return ADXF_ERR_PRM;
	}
	ADXSTM_Stop(adxf->stm);
	adxf->stopnw_flg = 1;
	return 0;
}

void ADXF_Stop(ADXF adxf)
{
	adxf_Stop(adxf);
}

int ADXF_Seek(ADXF adxf, int pos, int type)
{
	if (adxf == 0) {
		ADXERR_CallErrFunc1("E9040825:'adxf' is NULL.(ADXF_Seek)");
		return ADXF_ERR_PRM;
	}
	adxf_Stop(adxf);
	if (type == 0) {
		adxf->skpos = pos;
	} else if (type == 1) {
		adxf->skpos += pos;
	} else if (type == 2) {
		adxf->skpos = adxf->fnsct + pos;
	} else {
		ADXERR_CallErrFunc1("E9040826:'type' is illigal.(ADXF_Seek)");
		return ADXF_ERR_PRM;
	}
	return adxf->skpos;
}

int ADXF_Tell(ADXF adxf)
{
	if (adxf == 0) {
		ADXERR_CallErrFunc1("E9040827:'adxf' is NULL.(ADXF_Tell)");
		return ADXF_ERR_PRM;
	}
	return adxf->skpos;
}

int ADXF_GetFsizeSct(ADXF adxf)
{
	if (adxf == 0) {
		ADXERR_CallErrFunc1("E9040828:'adxf' is NULL.(ADXF_GetFsizeSct)");
		return ADXF_ERR_PRM;
	}
	return adxf->fnsct;
}

int ADXF_GetNumReqSct(ADXF adxf, int* seekpos)
{
	if (adxf == 0) {
		ADXERR_CallErrFunc1("E9040830:'adxf' is NULL.(ADXF_GetNumReqSct)");
		return ADXF_ERR_PRM;
	}
	*seekpos = adxf->rdstpos;
	return adxf->rqsct;
}

int ADXF_GetNumReadSct(ADXF adxf)
{
	if (adxf == 0) {
		ADXERR_CallErrFunc1("E9040831:'adxf' is NULL.(ADXF_GetNumReadSct)");
		return ADXF_ERR_PRM;
	}
	return adxf->rdsct;
}

int ADXF_GetStat(ADXF adxf)
{
	return adxf_GetStat(adxf);
}

// 0x80227B88
void ADXF_ExecServer(void)
{
	ADXF adxf;
	int i;

	ADXCRS_Lock();
	for (i = 0; i < ADXF_MAX_OBJ; i++) {
		adxf = &adxf_obj[i];
		if (adxf->used != 1) {
			continue;
		}
		if (adxf->stat == ADXF_STAT_READING) {
			adxf->stat  = ADXSTM_GetStat(adxf->stm);
			adxf->rdsct = ADXSTM_Tell(adxf->stm) - adxf->skpos;
			if (adxf->stat == ADXF_STAT_READEND || adxf->stat == ADXF_STAT_ERROR) {
				adxf->skpos += adxf->rdsct;
				adxf_DestroySj(adxf);
			}
		}
		if (adxf->stopnw_flg == 1 && ADXSTM_GetStat(adxf->stm) == ADXF_STAT_STOP) {
			adxf->rdsct = ADXSTM_Tell(adxf->stm) - adxf->skpos;
			adxf_DestroySj(adxf);
			adxf->stat       = ADXF_STAT_STOP;
			adxf->stopnw_flg = 0;
		}
	}
	ADXCRS_Unlock();
}

/* Gives entry flid of partition ptid the file it is in and where and how long it is in sectors */
// 0x802278F8
int ADXF_GetFnameRangeEx(int ptid, int flid, char* fname, void** dir, int* ofst, int* fnsct)
{
	int ret;
	ADXF_PTINFO* pt;
	int sct;
	int i;

	if (ptid < 0 || ptid >= ADXF_MAX_PTID) {
		ADXERR_CallErrFunc1("E9040828:'ptid' is range outside.");
		ret = ADXF_ERR_PRM;
	} else if (adxf_ptinfo[ptid] == 0) {
		ADXERR_CallErrFunc1("E9040828:'ptid' is range outside.");
		ret = ADXF_ERR_PRM;
	} else if (flid < 0 || flid >= adxf_ptinfo[ptid]->nfile) {
		ADXERR_CallErrFunc1("E9040828:'flid' is range outside.");
		ret = ADXF_ERR_PRM;
	} else {
		ret = 0;
	}
	if (ret < 0) {
		*dir   = 0;
		*ofst  = -1;
		*fnsct = -1;
		return ret;
	}
	pt = adxf_ptinfo[ptid];
	if (pt->rev == 1) {
		sct = pt->sz.i[0];
		for (i = 0; i < flid; i++) {
			sct += pt->sz.i[i + 1];
		}
		*fnsct = pt->sz.i[flid + 1];
	} else {
		sct = pt->sz.s[0];
		for (i = 0; i < flid; i++) {
			sct += pt->sz.s[i + 1];
		}
		*fnsct = pt->sz.s[flid + 1];
	}
	strncpy(fname, pt->fname, ADXF_FNAME_LEN);
	*dir  = pt->curdir;
	*ofst = pt->ofst + sct;
	return ret;
}

// 0x802278D8
void ADXF_Ocbi(void* buf, int size)
{
	DCInvalidateRange(buf, size);
}

void ADXF_SetReqRdSct(ADXF adxf, int nsct)
{
	if (adxf->stat == ADXF_STAT_READING) {
		ADXERR_CallErrFunc1("E0041201:state is reading(ADXF_SetReqRdSct)");
		return;
	}
	adxf->rqrdsct = nsct;
}
