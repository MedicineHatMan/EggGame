#include <CRI/adx.h>

ADXSJD_OBJ adxsjd_obj[ADXSJD_MAX_OBJ];

// 0x8021AEAC
void ADXSJD_Init(void)
{
	ADXB_Init();
	memset(adxsjd_obj, 0, sizeof(adxsjd_obj));
}

// 0x8021AE7C
void ADXSJD_Finish(void)
{
	memset(adxsjd_obj, 0, sizeof(adxsjd_obj));
}

static void* adxsjd_get_wr(void* obj, int* wpos, int* wlen, int* nsmpl);

// 0x8021ABEC
ADXSJD ADXSJD_Create(SJ sji, int maxnch, SJ* sjo)
{
	ADXSJD sjd;
	char* buf;
	SJ rbf;
	int bsize;
	int xsize;
	int i;

	rbf = sjo[0];
	for (i = 0; i < ADXSJD_MAX_OBJ; i++) {
		if (adxsjd_obj[i].used == 0) {
			break;
		}
	}
	if (i == ADXSJD_MAX_OBJ) {
		return 0;
	}
	sjd       = &adxsjd_obj[i];
	buf       = SJRBF_GetBufPtr(rbf);
	bsize     = SJRBF_GetBufSize(rbf) / 2;
	xsize     = SJRBF_GetXtrSize(rbf) / 2;
	sjd->adxb = ADXB_Create(maxnch, (short*)buf, bsize, bsize + xsize);
	if (sjd->adxb == 0) {
		return 0;
	}
	ADXB_EntryGetWrFunc(sjd->adxb, adxsjd_get_wr, sjd);
	sjd->sji    = sji;
	sjd->maxnch = maxnch;
	for (i = 0; i < maxnch; i++) {
		sjd->sjo[i] = sjo[i];
	}
	sjd->stat         = ADXSJD_STAT_STOP;
	sjd->hdr_len      = 0;
	sjd->dec_nsmpl    = 0;
	sjd->dec_dtlen    = 0;
	sjd->dec_pos      = 0;
	sjd->max_dec_smpl = 0x7FFFFFFF;
	sjd->trap_nsmpl   = -1;
	sjd->trap_cnt     = 0;
	sjd->trap_dtlen   = 0;
	sjd->term_flg     = 0;
	sjd->trap_func    = 0;
	sjd->trap_obj     = 0;
	sjd->flt_func     = 0;
	sjd->flt_obj      = 0;
	sjd->used         = 1;
	return sjd;
}

// 0x8021AB98
void ADXSJD_Destroy(ADXSJD sjd)
{
	if (sjd != 0) {
		if (sjd->adxb != 0) {
			ADXB_Destroy(sjd->adxb);
		}
		ADXCRS_Lock();
		memset(sjd, 0, sizeof(ADXSJD_OBJ));
		ADXCRS_Unlock();
	}
}

// 0x8021AB8C
int ADXSJD_GetStat(ADXSJD sjd)
{
	return sjd->stat;
}

// 0x8021AB64
void ADXSJD_SetInSj(ADXSJD sjd, SJ sj)
{
	sjd->sji = sj;
	ADXB_SetAhxInSj(sjd->adxb, sj);
}

// 0x8021AB3C
void ADXSJD_SetMaxDecSmpl(ADXSJD sjd, int nsmpl)
{
	sjd->max_dec_smpl = nsmpl;
	ADXB_SetAhxDecSmpl(sjd->adxb, nsmpl);
}

// 0x8021AB38
void ADXSJD_TermSupply(ADXSJD sjd)
{
}

// 0x8021AAF8
void ADXSJD_Start(ADXSJD sjd)
{
	sjd->hdr_len      = 0;
	sjd->dec_nsmpl    = 0;
	sjd->dec_dtlen    = 0;
	sjd->dec_pos      = 0;
	sjd->max_dec_smpl = 0x7FFFFFFF;
	sjd->trap_nsmpl   = -1;
	sjd->trap_cnt     = 0;
	sjd->trap_dtlen   = 0;
	sjd->term_flg     = 0;
	sjd->stat         = ADXSJD_STAT_DECHDR;
}

// 0x8021AAC0
void ADXSJD_Stop(ADXSJD sjd)
{
	ADXB_Stop(sjd->adxb);
	sjd->stat = ADXSJD_STAT_STOP;
}

// 0x8021A8CC
static void adxsjd_decode_prep(ADXSJD sjd)
{
	ADXB adxb;
	SJ sji;
	int hdlen;
	int fmt;
	SJCK ck;
	SJCK ck2;

	sji  = sjd->sji;
	adxb = sjd->adxb;
	SJ_GetChunk(sji, SJ_LIN_DATA, 0x1000, &ck);
	if (ck.len < 0x10) {
		SJ_UngetChunk(sji, SJ_LIN_DATA, &ck);
		return;
	}
	hdlen = ADXB_DecodeHeader(adxb, ck.data, ck.len);
	if (hdlen == 0 || hdlen > ck.len) {
		SJ_UngetChunk(sji, SJ_LIN_DATA, &ck);
		return;
	}
	if (hdlen < 0) {
		SJ_UngetChunk(sji, SJ_LIN_DATA, &ck);
		ADXERR_CallErrFunc2("E03010901 ADXB_DecodeHeader: ", "Can not decode this file format.");
		sjd->stat = ADXSJD_STAT_ERROR;
		return;
	}
	sjd->hdr_len = hdlen;
	if (ADXB_GetFormat(adxb) == ADXB_FMT_AU) {
		sjd->term_flg = 1;
	}
	if (ADXB_GetFormat(adxb) == ADXB_FMT_SPSD) {
		memcpy(sjd->hdr, ck.data, (ck.len < 0x40L) ? ck.len : 0x40L);
	}
	fmt = ADXB_GetFormat(adxb);
	if (fmt == ADXB_FMT_AHX || fmt == ADXB_FMT_AHX + 1 || fmt == ADXB_FMT_AC3) {
		SJ_UngetChunk(sji, SJ_LIN_DATA, &ck);
	} else {
		SJ_SplitChunk(&ck, hdlen, &ck, &ck2);
		SJ_PutChunk(sji, SJ_LIN_FREE, &ck);
		SJ_UngetChunk(sji, SJ_LIN_DATA, &ck2);
	}
	sjd->stat = ADXSJD_STAT_DECODE;
}

// 0x8021A7D4
static void* adxsjd_get_wr(void* obj, int* wpos, int* wlen, int* nsmpl)
{
	ADXSJD sjd = (ADXSJD)obj;
	SJ sjo;
	int i;
	int n;

	sjo = sjd->sjo[0];
	for (i = 0; i < ADXB_GetNumChan(sjd->adxb); i++) {
		SJ_GetChunk(sjd->sjo[i], SJ_LIN_FREE, 0x4000, &sjd->cko[i]);
	}
	*wpos = (sjd->cko[0].data - SJRBF_GetBufPtr(sjo)) / 2;
	n     = sjd->max_dec_smpl;
	if (sjd->cko[0].len / 2 < n) {
		n = sjd->cko[0].len / 2;
	}
	*wlen = n;
	if (sjd->trap_nsmpl >= 0) {
		*nsmpl = sjd->trap_nsmpl - sjd->trap_cnt;
	} else {
		*nsmpl = 0x1FFFFFFF;
	}
	return ADXB_GetPcmBuf(sjd->adxb);
}

// 0x8021A5CC
#pragma push
#pragma auto_inline off
static void adxsjd_decexec_start(ADXSJD sjd)
{
	ADXB adxb;
	SJ sji;

	adxb = sjd->adxb;
	sji  = sjd->sji;
	if (sjd->trap_nsmpl >= 0 && sjd->trap_cnt >= sjd->trap_nsmpl && sjd->trap_func != 0) {
		sjd->trap_func(sjd->trap_obj);
	}
	if (sjd->term_flg == 1 && SJ_GetNumData(sji, SJ_LIN_DATA) == 0) {
		sjd->stat = ADXSJD_STAT_DECEND;
		return;
	}
	SJ_GetChunk(sji, SJ_LIN_DATA, 0x7FFFFFFF, &sjd->cki);
	if (ADXB_GetFormat(adxb) == ADXB_FMT_ADX && sjd->cki.len >= 2 && (unsigned short)*(short*)sjd->cki.data == 0x8001) {
		sjd->stat = ADXSJD_STAT_DECEND;
		SJ_UngetChunk(sji, SJ_LIN_DATA, &sjd->cki);
		return;
	}
	if (sjd->dec_pos >= ADXB_GetTotalNumSmpl(sjd->adxb)) {
		sjd->stat = ADXSJD_STAT_DECEND;
		SJ_UngetChunk(sji, SJ_LIN_DATA, &sjd->cki);
		return;
	}
	if (SJ_GetNumData(sjd->sjo[0], SJ_LIN_FREE) / 2 < ADXB_GetBlkSmpl(sjd->adxb)) {
		SJ_UngetChunk(sji, SJ_LIN_DATA, &sjd->cki);
		return;
	}
	if (ADXB_GetFormat(adxb) == ADXB_FMT_AHX) {
		SJ_UngetChunk(sji, SJ_LIN_DATA, &sjd->cki);
	}
	ADXB_EntryData(adxb, sjd->cki.data, sjd->cki.len);
	ADXB_Start(adxb);
}
#pragma pop

/* Hands the decoded samples to the output joints and the used input back */
static void adxsjd_decexec_end(ADXSJD sjd)
{
	ADXB adxb;
	SJ sji;
	int total;
	int dtlen;
	int nsmpl;
	int n;
	int i;
	SJCK ck1;
	SJCK ck2;

	adxb  = sjd->adxb;
	sji   = sjd->sji;
	total = ADXB_GetTotalNumSmpl(adxb);
	dtlen = ADXB_GetDecDtLen(adxb);
	nsmpl = ADXB_GetDecNumSmpl(adxb);
	n     = total - sjd->dec_pos;
	if (nsmpl < n) {
		n = nsmpl;
	}
	SJ_SplitChunk(&sjd->cki, dtlen, &ck1, &ck2);
	SJ_PutChunk(sji, SJ_LIN_FREE, &ck1);
	SJ_UngetChunk(sji, SJ_LIN_DATA, &ck2);
	for (i = 0; i < ADXB_GetNumChan(sjd->adxb); i++) {
		SJ_SplitChunk(&sjd->cko[i], n * 2, &ck1, &ck2);
		if (sjd->flt_func != 0) {
			sjd->flt_func(sjd->flt_obj, i, ck1.data, ck1.len);
		}
		SJ_PutChunk(sjd->sjo[i], SJ_LIN_DATA, &ck1);
		SJ_UngetChunk(sjd->sjo[i], SJ_LIN_FREE, &ck2);
	}
	sjd->dec_nsmpl += n;
	sjd->dec_dtlen += dtlen;
	sjd->dec_pos += n;
	sjd->trap_cnt += n;
	sjd->trap_dtlen += dtlen;
	ADXB_Reset(adxb);
}

/* AHX and AC3 write their own output only the counts move here */
static void adxsjd_decexec_extern(ADXSJD sjd)
{
	ADXB adxb;
	int total;
	int dtlen;
	int nsmpl;
	int n;

	adxb  = sjd->adxb;
	total = ADXB_GetTotalNumSmpl(adxb);
	dtlen = ADXB_GetDecDtLen(adxb);
	nsmpl = ADXB_GetDecNumSmpl(adxb);
	n     = total - sjd->dec_pos;
	if (nsmpl < n) {
		n = nsmpl;
	}
	sjd->dec_nsmpl += n;
	sjd->dec_dtlen += dtlen;
	sjd->dec_pos += n;
}

// 0x8021A374
void ADXSJD_ExecHndl(ADXSJD sjd)
{
	ADXB adxb;

	if (sjd->stat == ADXSJD_STAT_DECODE) {
		adxb = sjd->adxb;
		if (ADXB_GetStat(adxb) == ADXB_STAT_STOP) {
			adxsjd_decexec_start(sjd);
		}
		ADXB_ExecHndl(adxb);
		if (ADXB_GetStat(adxb) == ADXB_STAT_DECEND) {
			adxsjd_decexec_end(sjd);
		}
		if (adxb->format == ADXB_FMT_AHX || adxb->format == ADXB_FMT_AC3) {
			adxsjd_decexec_extern(sjd);
		}
	} else if (sjd->stat == ADXSJD_STAT_DECHDR) {
		adxsjd_decode_prep(sjd);
	}
}

// 0x8021A0F4
void ADXSJD_ExecServer(void)
{
	ADXSJD sjd;
	int i;

	for (i = 0; i < ADXSJD_MAX_OBJ; i++) {
		sjd = &adxsjd_obj[i];
		if (sjd->used == 1) {
			ADXSJD_ExecHndl(sjd);
		}
	}
}

// 0x8021A0EC
int ADXSJD_GetDecNumSmpl(ADXSJD sjd)
{
	return sjd->dec_nsmpl;
}

// 0x8021A0E4
void ADXSJD_SetDecPos(ADXSJD sjd, int pos)
{
	sjd->dec_pos = pos;
}

// 0x8021A0D8
void ADXSJD_EntryTrapFunc(ADXSJD sjd, void (*func)(void*), void* obj)
{
	sjd->trap_func = func;
	sjd->trap_obj  = obj;
}

// 0x8021A0D0
void ADXSJD_SetTrapNumSmpl(ADXSJD sjd, int nsmpl)
{
	sjd->trap_nsmpl = nsmpl;
}

// 0x8021A0C8
void ADXSJD_SetTrapCnt(ADXSJD sjd, int cnt)
{
	sjd->trap_cnt = cnt;
}

// 0x8021A0C0
void ADXSJD_SetTrapDtLen(ADXSJD sjd, int dtlen)
{
	sjd->trap_dtlen = dtlen;
}

// 0x8021A09C
int ADXSJD_GetFormat(ADXSJD sjd)
{
	return ADXB_GetFormat(sjd->adxb);
}

// 0x8021A078
int ADXSJD_GetSfreq(ADXSJD sjd)
{
	return ADXB_GetSfreq(sjd->adxb);
}

// 0x8021A054
int ADXSJD_GetNumChan(ADXSJD sjd)
{
	return ADXB_GetNumChan(sjd->adxb);
}

// 0x8021A030
int ADXSJD_GetOutBps(ADXSJD sjd)
{
	return ADXB_GetOutBps(sjd->adxb);
}

// 0x8021A00C
int ADXSJD_GetBlkSmpl(ADXSJD sjd)
{
	return ADXB_GetBlkSmpl(sjd->adxb);
}

// 0x80219FE8
int ADXSJD_GetTotalNumSmpl(ADXSJD sjd)
{
	return ADXB_GetTotalNumSmpl(sjd->adxb);
}

// 0x80219FC4
int ADXSJD_GetNumLoop(ADXSJD sjd)
{
	return ADXB_GetNumLoop(sjd->adxb);
}

// 0x80219FA0
int ADXSJD_GetLpStartPos(ADXSJD sjd)
{
	return ADXB_GetLpStartPos(sjd->adxb);
}

// 0x80219F7C
int ADXSJD_GetLpStartOfst(ADXSJD sjd)
{
	return ADXB_GetLpStartOfst(sjd->adxb);
}

// 0x80219F58
int ADXSJD_GetLpEndPos(ADXSJD sjd)
{
	return ADXB_GetLpEndPos(sjd->adxb);
}

// 0x80219F34
int ADXSJD_GetLpEndOfst(ADXSJD sjd)
{
	return ADXB_GetLpEndOfst(sjd->adxb);
}

// 0x80219F2C
char* ADXSJD_GetSpsdInfo(ADXSJD sjd)
{
	return sjd->hdr;
}

// 0x80219F08
void ADXSJD_GetExtPrm(ADXSJD sjd)
{
	ADXB_GetExtPrm(sjd->adxb);
}

// 0x80219EE4
void ADXSJD_SetExtPrm(ADXSJD sjd)
{
	ADXB_SetExtPrm(sjd->adxb);
}
