#include <CRI/adx_tlk.h>
#include <CRI/adx.h>
#include <CRI/adx_fs.h>
#include <CRI/adxm.h>
#include <CRI/cri_std.h>
#include <CRI/lsc.h>

int adxt_unk570[4];
int adxt_vsync_rate;
float adxt_time_diff_ms;
void (*adxt_destroy_cb)(ADXT adxt);
int adxt_unk54C[6];
int adxt_time_resync_cnt;
int adxt_svr_stage;
int adxt_time_vsync_on;

/* V-sync count kept by the ADXM v-sync thread */

/* Time mode 0 reads the time from the decoder otherwise from the v-sync count corrected when the two drift apart */
/* What ADXT_ExecServer is doing 0 when it is idle */
/* How many times the v-sync time has been reset to the decoder time */
/* Called from ADXT_Destroy before anything else */
/* Last difference between the two times in milliseconds */
/* Time scale of the v-sync time */
/* 1 when the times drift apart take the decoder time again */

/* ADXERR ADXCRS and ADX are in the file before ADXSTM */

/* ADXSJD the stream joint decoder */

/* ADXAMP the amplitude extractor */

/* An ADX frame is 18 bytes per channel and holds 32 samples */
#define ADX_FRAME_LEN 18
#define ADX_FRAME_SMPL 32

static inline void adxt_start_sj(ADXT adxt, SJ sj)
{
	int i;

	for (i = 0; i < adxt->maxnch; i++) {
		SJ_Reset(adxt->sjo[i]);
	}
	ADXSJD_SetInSj(adxt->sjd, sj);
	adxt->sji = sj;
	ADXSJD_Start(adxt->sjd);
	adxt->stat = ADXT_STAT_DECINFO;
	adxt->lpcnt = 0;
	adxt->pstready_flag = 0;
	adxt->lesct = 0x7FFFFFFF;
	adxt->trpnsmpl = -1;
	adxt->tvofst = 0;
	adxt->decofst = 0;
	adxt->svcnt = adxt_svcnt;
	if (adxt->amp != 0) {
		ADXAMP_Start(adxt->amp);
	}
}

static inline int adxt_GetNumSmplObuf(ADXT adxt, int ch)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080837 ADXT_GetNumSmplObuf: parameter error");
		return -1;
	}
	if (adxt->sjo[ch] != 0) {
		return SJ_GetNumData(adxt->sjo[ch], SJ_LIN_DATA) / 2;
	}
	return 0;
}

/* The time from the decoder samples decoded less those still waiting in the output buffer and the renderer */
static inline void adxt_GetTimeDec(ADXT adxt, int* ncount, int* tscale)
{
	int ndec;

	if (adxt->stat == ADXT_STAT_PLAYING || adxt->stat == ADXT_STAT_DECEND) {
		*tscale = ADXSJD_GetSfreq(adxt->sjd);
		ndec = ADXSJD_GetDecNumSmpl(adxt->sjd);
		*ncount = ndec - (ADXRNA_GetNumData(adxt->rna) + adxt_GetNumSmplObuf(adxt, 0)) + adxt->decofst;
	} else if (adxt->stat == ADXT_STAT_PLAYEND) {
		*ncount = ADXSJD_GetTotalNumSmpl(adxt->sjd);
		*tscale = ADXSJD_GetSfreq(adxt->sjd);
		*ncount = *ncount * (16 / ADXSJD_GetOutBps(adxt->sjd));
		*ncount = *ncount + adxt->decofst;
	} else {
		*ncount = 0;
		*tscale = 1;
	}
	*ncount += adxt->time_ofst;
}

/* Restarts the v-sync time from the decoder time */
static inline void adxt_ResetTime(ADXT adxt)
{
	int save;
	int tscale;
	int ncount;

	save = adxt_time_vsync_on;
	adxt_time_vsync_on = 0;
	ADXT_GetTime(adxt, &ncount, &tscale);
	adxt_time_vsync_on = save;
	adxt->tvofst = (unsigned int)((float)adxt_vsync_rate * ((float)ncount / (float)tscale));
}

// 0x80225004
ADXT ADXT_Create(int maxnch, void* work, int worksize)
{
	int i;
	ADXT adxt;
	int top;
	int wsize;
	int obuflen;

	top = ((int)work + 0x3F) & ~0x3F;
	wsize = worksize - (top - (int)work);
	if (maxnch < 0 || work == 0 || worksize < 0) {
		ADXERR_CallErrFunc1("E02080804 ADXT_Create: parameter error");
		return 0;
	}
	for (i = 0; i < ADXT_MAX_OBJ; i++) {
		if (adxt_obj[i].used == 0) {
			break;
		}
	}
	if (i == ADXT_MAX_OBJ) {
		return 0;
	}
	adxt = &adxt_obj[i];
	memset(adxt, 0, sizeof(ADX_TALK));
	adxt->maxnch = maxnch;
	obuflen = maxnch * 0x3060 * sizeof(short);
	adxt->ibuf = (char*)(top + obuflen);
	adxt->ibuflen = ((wsize - obuflen - 0x124) / ADXSTM_SCT) * ADXSTM_SCT;
	adxt->ibufxlen = ADXT_IBUF_XLEN;
	adxt->fname_buf = adxt->ibuflen + adxt->ibufxlen + adxt->ibuf;
	adxt->obuf = (short*)top;
	adxt->obufsize = ADXT_OBUF_SIZE;
	adxt->obufdist = ADXT_OBUF_DIST;
	adxt->sji = 0;
	adxt->sjf = SJRBF_Create(adxt->ibuf, adxt->ibuflen, adxt->ibufxlen);
	if (adxt->sjf == 0) {
		ADXT_Destroy(adxt);
		return 0;
	}
	if ((adxt->stm = ADXSTM_Create(adxt->sjf, 0)) == 0) {
		ADXT_Destroy(adxt);
		return 0;
	}
	for (i = 0; i < maxnch; i++) {
		adxt->sjo[i] = SJRBF_Create((char*)(adxt->obuf + adxt->obufdist * i), adxt->obufsize * 2,
		                            (adxt->obufdist - adxt->obufsize) * 2);
		if (adxt->sjo[i] == 0) {
			ADXT_Destroy(adxt);
			return 0;
		}
	}
	if ((adxt->sjd = ADXSJD_Create(adxt->sjf, maxnch, adxt->sjo)) == 0) {
		ADXT_Destroy(adxt);
		return 0;
	}
	if ((adxt->rna = ADXRNA_Create(adxt->sjo, maxnch, (short*)(top + maxnch * (ADXT_OBUF_DIST * 2)))) == 0) {
		ADXT_Destroy(adxt);
		return 0;
	}
	if ((adxt->lsc = LSC_Create(adxt->sjf)) == 0) {
		ADXT_Destroy(adxt);
		return 0;
	}
	LSC_SetStmHndl(adxt->lsc, adxt->stm);
	adxt->svrfreq = ADXT_DEF_SVRFREQ;
	adxt->maxsct = adxt->ibuflen / ADXSTM_SCT;
	adxt->minsct = (short)(0.85f * adxt->maxsct);
	adxt->outvol = 0;
	for (i = 0; i < maxnch; i++) {
		adxt->outpan[i] = ADXT_PAN_AUTO;
	}
	adxt->lpflg = 1;
	adxt->trp = 0;
	adxt->wpos = 0;
	adxt->mofst = 0;
	adxt->ercode = 0;
	adxt->edecpos = 0;
	adxt->edeccnt = 0;
	adxt->eshrtcnt = 0;
	adxt->autorcvr = 1;
	adxt->pause_flag = 0;
	adxt->time_ofst = 0;
	adxt->lnkflg = 0;
	adxt->used = 1;
	return adxt;
}

// 0x80224DC0
void ADXT_Destroy(ADXT adxt)
{
	int i;

	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080805 ADXT_Destroy: parameter error");
		return;
	}
	if (adxt_destroy_cb != 0) {
		adxt_destroy_cb(adxt);
	}
	if (adxt->used == 1) {
		ADXT_Stop(adxt);
	}
	if (adxt->rna != 0) {
		ADXRNA_Destroy(adxt->rna);
	}
	if (adxt->sjd != 0) {
		ADXSJD_Destroy(adxt->sjd);
	}
	if (adxt->stm != 0) {
		ADXSTM_Destroy(adxt->stm);
	}
	if (adxt->lsc != 0) {
		LSC_Destroy(adxt->lsc);
	}
	ADXCRS_Lock();
	if (adxt->sjf != 0) {
		SJ_Destroy(adxt->sjf);
	}
	for (i = 0; i < adxt->maxnch; i++) {
		if (adxt->sjo[i] != 0) {
			SJ_Destroy(adxt->sjo[i]);
		}
		if (adxt->ampsji[i] != 0) {
			SJ_Destroy(adxt->ampsji[i]);
		}
		if (adxt->ampsjo[i] != 0) {
			SJ_Destroy(adxt->ampsjo[i]);
		}
	}
	if (adxt->amp != 0) {
		ADXAMP_Destroy(adxt->amp);
	}
	memset(adxt, 0, sizeof(ADX_TALK));
	adxt->used = 0;
	ADXCRS_Unlock();
}

/* The input buffer keeps between minsct and maxsct sectors and reading stops 25 sectors in until the header is known */
// 0x80224C78
void adxt_start_stm(ADXT adxt, const char* fname, void* dir, int ofst, int nsct)
{
	ADXSTM_SetBufSize(adxt->stm, adxt->minsct * ADXSTM_SCT, adxt->maxsct * ADXSTM_SCT);
	ADXSTM_SetEos(adxt->stm, 25);
	ADXSTM_EntryEosFunc(adxt->stm, 0, 0);
	ADXSTM_Seek(adxt->stm, 0);
	ADXSTM_StopNw(adxt->stm);
	ADXSTM_ReleaseFileNw(adxt->stm);
	ADXSTM_BindFileNw(adxt->stm, fname, dir, ofst, nsct);
	ADXSTM_Start(adxt->stm);
	adxt_start_sj(adxt, adxt->sjf);
}

// 0x80224A94
void ADXT_StartSj(ADXT adxt, SJ sj)
{
	if (adxt == 0 || sj == 0) {
		ADXERR_CallErrFunc1("E02080812 ADXT_StartSj: parameter error");
		return;
	}
	ADXT_Stop(adxt);
	ADXCRS_Lock();
	adxt_start_sj(adxt, sj);
	adxt->pmode = ADXT_PMODE_SJ;
	adxt->lnkflg = 1;
	ADXCRS_Unlock();
}

// 0x802249A0
void ADXT_Stop(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080813 ADXT_Stop: parameter error");
		return;
	}
	if (adxt->stm != 0) {
		ADXSTM_ReleaseFileNw(adxt->stm);
	}
	ADXCRS_Lock();
	if (adxt->pmode == ADXT_PMODE_SLFILE) {
		LSC_Stop(adxt->lsc);
		if (adxt->sji != 0) {
			SJ_Reset(adxt->sji);
		}
	}
	ADXCRS_Lock();
	ADXRNA_SetTransSw(adxt->rna, 0);
	ADXRNA_SetPlaySw(adxt->rna, 0);
	ADXSJD_Stop(adxt->sjd);
	if (adxt->pmode == ADXT_PMODE_MEM && adxt->sji != 0) {
		SJ_Destroy(adxt->sji);
	}
	if (adxt->amp != 0) {
		ADXAMP_Stop(adxt->amp);
	}
	adxt->sji = 0;
	adxt->stat = ADXT_STAT_STOP;
	adxt->stm_start = 0;
	ADXCRS_Unlock();
	ADXCRS_Unlock();
}

// 0x80224960
int ADXT_GetStat(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080814 ADXT_GetStat: parameter error");
		return -1;
	}
	return adxt->stat;
}

// 0x8022446C
void ADXT_GetTime(ADXT adxt, int* ncount, int* tscale)
{
	int save;
	int nc;
	int ts;
	int bps;

	if (adxt == 0 || ncount == 0 || tscale == 0) {
		ADXERR_CallErrFunc1("E02080815 ADXT_GetTime: parameter error");
		return;
	}
	if (adxt_time_vsync_on == 0) {
		adxt_GetTimeDec(adxt, ncount, tscale);
		return;
	}
	adxt_time_diff_ms = 0.0f;
	if (adxt->stat == ADXT_STAT_PLAYING || adxt->stat == ADXT_STAT_DECEND) {
		if (adxt->pause_flag == 0) {
			*ncount = adxt->tvofst + (adxt_svcnt - adxt->svcnt) * 100;
		} else {
			*ncount = adxt->tvofst;
		}
		adxt_GetTimeDec(adxt, &nc, &ts);
		adxt_time_diff_ms = 1000.0f * ((float)nc / (float)ts - (float)*ncount / (float)adxt_vsync_rate);
		if (adxt_time_diff_ms > 60.0f || adxt_time_diff_ms < -60.0f) {
			if (adxt_time_resync_on == 1) {
				save = adxt_time_vsync_on;
				adxt_time_vsync_on = 0;
				ADXT_GetTime(adxt, &nc, &ts);
				adxt_time_vsync_on = save;
				adxt_time_resync_cnt++;
			}
			adxt->tvofst = (unsigned int)((float)adxt_vsync_rate * ((float)nc / (float)ts));
			adxt->svcnt = adxt_svcnt;
		}
	} else if (adxt->stat == ADXT_STAT_PLAYEND) {
		nc = ADXSJD_GetTotalNumSmpl(adxt->sjd);
		ts = ADXSJD_GetSfreq(adxt->sjd);
		bps = ADXSJD_GetOutBps(adxt->sjd);
		bps = 16 / bps;
		nc = nc * bps;
		*ncount = (int)((float)adxt_vsync_rate * ((float)nc / (float)ts));
		*ncount += adxt->tvofst + 1;
	} else {
		*ncount = 0;
	}
	*ncount += adxt->time_ofst;
	*tscale = adxt_vsync_rate;
}

/* Stripped from the DOL the pool keeps a 100.0f between ADXT_GetNumSmpl's string and ADXT_GetTime's with no string
 * of its own */
float adxt_GetTimeRate(ADXT adxt)
{
	return (float)adxt->time_ofst / 100.0f;
}

// 0x80224414
int ADXT_GetNumSmpl(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080817 ADXT_GetNumSmpl: parameter error");
		return -1;
	}
	if (adxt->stat >= ADXT_STAT_PREP) {
		return ADXSJD_GetTotalNumSmpl(adxt->sjd);
	}
	return 0;
}

// 0x802243BC
int ADXT_GetSfreq(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080819 ADXT_GetSfreq: parameter error");
		return -1;
	}
	if (adxt->stat >= ADXT_STAT_PREP) {
		return ADXSJD_GetSfreq(adxt->sjd);
	}
	return 0;
}

// 0x80224364
int ADXT_GetNumChan(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080820 ADXT_GetNumChan: parameter error");
		return -1;
	}
	if (adxt->stat >= ADXT_STAT_PREP) {
		return ADXSJD_GetNumChan(adxt->sjd);
	}
	return 0;
}

/* Stripped from the DOL its string is left in the pool */
int ADXT_GetHdrLen(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080822 ADXT_GetHdrLen: parameter error");
		return -1;
	}
	return 0;
}

/* Stripped from the DOL its string is left in the pool */
int ADXT_GetFmtBps(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080821 ADXT_GetFmtBps: parameter error");
		return -1;
	}
	return 0;
}

// 0x802242FC
void ADXT_SetOutPan(ADXT adxt, int ch, int pan)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080825 ADXT_SetOutPan: parameter error");
		return;
	}
	adxt->outpan[ch] = pan;
	if (ch < adxt->maxnch) {
		ADXRNA_SetOutPan(adxt->rna, ch, pan);
	} else {
		ADXERR_CallErrFunc1("E8101208 ADXT_SetOutPan: parameter error");
	}
}

// 0x802242B8
int ADXT_GetOutPan(ADXT adxt, int ch)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080826 ADXT_GetOutPan: parameter error");
		return 0;
	}
	return adxt->outpan[ch];
}

// 0x80224278
void ADXT_SetOutVol(ADXT adxt, int vol)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080823 ADXT_SetOutVol: parameter error");
		return;
	}
	adxt->outvol = vol;
	ADXRNA_SetOutVol(adxt->rna, vol);
}

// 0x8022423C
int ADXT_GetOutVol(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080824 ADXT_GetOutVol: parameter error");
		return 0;
	}
	return adxt->outvol;
}

// 0x80224204
void ADXT_SetSvrFreq(ADXT adxt, int freq)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080840 ADXT_SetSvrFreq: parameter error");
		return;
	}
	adxt->svrfreq = freq;
}

// 0x802241FC
void ADXT_SetAutoRcvr(ADXT adxt, int rmode)
{
	adxt->autorcvr = rmode;
}

// 0x80224140
void ADXT_ExecServer(void)
{
	ADXT adxt;
	int i;

	ADXM_Lock();
	if (adxt_svr_stage != 0) {
		ADXM_Unlock();
		return;
	}
	ADXM_Unlock();
	ADXCRS_Lock();
	adxt_svr_stage = 1;
	ADXSJD_ExecServer();
	adxt_svr_stage = 2;
	for (i = 0; i < ADXT_MAX_OBJ; i++) {
		adxt = &adxt_obj[i];
		if (adxt->used == 1) {
			ADXT_ExecHndl(adxt);
		}
	}
	adxt_svr_stage = 3;
	ADXRNA_ExecServer();
	adxt_svr_stage = 0;
	ADXCRS_Unlock();
}

/* ADXT_SetReloadTime down to ADXT_IsCompleted are stripped from the DOL */
void ADXT_SetReloadTime(ADXT adxt, float time, int nch, int sfreq)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080838 ADXT_SetReloadTime: parameter error");
		return;
	}
}

void ADXT_SetReloadSct(ADXT adxt, int minsct)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080839 ADXT_SetReloadSct: parameter error");
		return;
	}
	adxt->minsct = minsct;
}

int ADXT_GetNumSctIbuf(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080834 ADXT_GetNumSctIbuf: parameter error");
		return -1;
	}
	return 0;
}

float ADXT_GetIbufRemainTime(ADXT adxt)
{
	int stat;
	int nch;
	int sfreq;
	int nobuf;

	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080835 ADXT_GetIbufRemainTime: parameter error");
		return -1.0f;
	}
	stat = ADXT_GetStat(adxt);
	nch = ADXT_GetNumChan(adxt);
	sfreq = ADXT_GetSfreq(adxt);
	nobuf = adxt_GetNumSmplObuf(adxt, 0);
	if (stat == ADXT_STAT_PLAYING || nch == 0) {
		return 0;
	}
	return (float)nobuf / (float)sfreq;
}

int ADXT_IsIbufSafety(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080836 ADXT_IsIbufSafety: parameter error");
		return 0;
	}
	return adxt_time_diff_ms > 0.0f;
}

int ADXT_IsCompleted(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080802 ADXT_IsCompleted: parameter error");
		return 0;
	}
	return adxt->stat == ADXT_STAT_PLAYEND;
}

// 0x80224104
int ADXT_GetErrCode(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080843 ADXT_GetErrCode: parameter error");
		return -1;
	}
	return adxt->ercode;
}

// 0x802240BC
void ADXT_ClearErrCode(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080844 ADXT_ClearErrCode: parameter error");
		return;
	}
	adxt->ercode = 0;
	adxt->edecpos = 0;
	adxt->edeccnt = 0;
	adxt->eshrtcnt = 0;
}

/* ADXT_GetLpCnt down to ADXT_IsReadyPlayStart are stripped from the DOL */
int ADXT_GetLpCnt(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080829 ADXT_GetLpCnt: parameter error");
		return -1;
	}
	return adxt->lpcnt;
}

void ADXT_SetLpFlg(ADXT adxt, int flg)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080828 ADXT_SetLpFlg: parameter error");
		return;
	}
	adxt->lpflg = flg;
}

SJ ADXT_GetInputSj(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080833 ADXT_GetInputSj: parameter error");
		return 0;
	}
	return adxt->sji;
}

void ADXT_SetWaitPlayStart(ADXT adxt, int flg)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080830 ADXT_SetWaitPlayStart: parameter error");
		return;
	}
	adxt->pstwait_flag = flg;
}

int ADXT_IsReadyPlayStart(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080831 ADXT_IsReadyPlayStart: parameter error");
		return 0;
	}
	return adxt->pstready_flag;
}

// 0x80223F6C
void ADXT_Pause(ADXT adxt, int sw)
{
	int stat;

	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080846 ADXT_Pause: parameter error");
		return;
	}
	stat = adxt->stat;
	if (sw == adxt->pause_flag) {
		return;
	}
	ADXCRS_Lock();
	adxt->pause_flag = sw;
	if (stat == ADXT_STAT_PLAYING || stat == ADXT_STAT_DECEND) {
		if (sw == 1) {
			ADXRNA_SetPlaySw(adxt->rna, 0);
		} else {
			ADXRNA_SetPlaySw(adxt->rna, 1);
			adxt->svcnt = adxt_svcnt;
		}
		adxt_ResetTime(adxt);
	}
	ADXCRS_Unlock();
}

/* Stripped from the DOL its string is left in the pool */
int ADXT_GetStatPause(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080847 ADXT_GetStatPause: parameter error");
		return -1;
	}
	return adxt->pause_flag;
}

/* Empty in this build stat_decinfo asks it for two parameters and passes them to adxt_GetDecParam when either is set */
// 0x80223F68
void adxt_SetDecParam(ADXT adxt, int a, int b)
{
}

// 0x80223F64
void adxt_GetDecParam(ADXT adxt, int* a, int* b)
{
}

/* While paused runs the server once and restarts the v-sync time from the decoder then returns what the renderer
 * reports */
// 0x80223E00
int adxt_GetTimePause(ADXT adxt)
{
	int ret;
	unsigned int vcnt;
	int save;
	int ncount;
	int tscale;

	if (adxt->pause_flag == 0) {
		return 0;
	}
	ret = ADXRNA_GetPauseTime(adxt->rna);
	ADXT_ExecServer();
	save = adxt_time_vsync_on;
	adxt_time_vsync_on = 0;
	ADXT_GetTime(adxt, &ncount, &tscale);
	adxt_time_vsync_on = save;
	adxt->tvofst = (unsigned int)((float)adxt_vsync_rate * ((float)ncount / (float)tscale));
	vcnt = adxt_svcnt;
	adxt->svcnt = vcnt;
	return ret;
}

// 0x80223DF8
void ADXT_SetTimeOfst(ADXT adxt, int ofst)
{
	adxt->time_ofst = ofst;
}

// 0x80223D64
int ADXT_IsHeader(const unsigned char* adr, int siz, int* hdrsiz)
{
	short hdrlen;
	signed char enc;
	signed char bits;
	signed char blklen;
	signed char nch;
	int sfreq;
	int nsmpl;
	int blksmpl;

	if (siz < 2) {
		return 0;
	}
	if (*(const unsigned short*)adr != 0x8000) {
		return 0;
	}
	if (ADX_DecodeInfo(adr, siz, &hdrlen, &enc, &bits, &blklen, &nch, &sfreq, &nsmpl, &blksmpl) < 0) {
		return 0;
	}
	*hdrsiz = hdrlen;
	return 1;
}

// 0x80223D34
int ADXT_IsEndcode(const unsigned char* adr, int siz, int* endsiz)
{
	if (siz < 2) {
		return 0;
	}
	if (*(const unsigned short*)adr != 0x8001) {
		return 0;
	}
	*endsiz = siz;
	return 1;
}

/* Writes silent frames into the input stream joint in up to two chunks and returns the samples written */
// 0x80223BB4
int ADXT_InsertSilence(ADXT adxt, int nch, int nsmpl)
{
	int unit;
	int len;
	SJ sj;
	int n1;
	int n2;
	SJCK ck;
	SJCK rest;

	sj = adxt->sji;
	if (sj == 0) {
		return 0;
	}
	unit = nch * ADX_FRAME_LEN;
	len = (nsmpl / ADX_FRAME_SMPL) * unit;
	SJ_GetChunk(adxt->sji, SJ_LIN_FREE, len, &ck);
	n1 = unit * (ck.len / unit);
	memset(ck.data, 0, n1);
	SJ_SplitChunk(&ck, n1, &ck, &rest);
	SJ_PutChunk(sj, SJ_LIN_DATA, &ck);
	SJ_UngetChunk(sj, SJ_LIN_FREE, &rest);
	len -= n1;
	SJ_GetChunk(sj, SJ_LIN_FREE, len, &ck);
	n2 = unit * (ck.len / unit);
	memset(ck.data, 0, n2);
	SJ_SplitChunk(&ck, n2, &ck, &rest);
	SJ_PutChunk(sj, SJ_LIN_DATA, &ck);
	SJ_UngetChunk(sj, SJ_LIN_FREE, &rest);
	return ((n1 + n2) / unit) * ADX_FRAME_SMPL;
}

// 0x80223B94
void ADXT_SetDecodeSteAsMonoSw(int sw)
{
	ADX_SetDecodeSteAsMonoSw(sw);
}

/* Stripped from the DOL its string is left in the pool */
int ADXT_GetDecNumSmpl(ADXT adxt)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080818 ADXT_GetDecNumSmpl: parameter error");
		return -1;
	}
	return ADXSJD_GetDecNumSmpl(adxt->sjd);
}

/* Stripped from the DOL its string is left in the pool */
void ADXT_SetKeyString(ADXT adxt, const char* str)
{
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080860 ADXT_SetKeyString: parameter error");
		return;
	}
}
