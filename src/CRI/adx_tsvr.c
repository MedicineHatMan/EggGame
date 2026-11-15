#include <CRI/adx_tlk.h>
#include <CRI/adx.h>
#include <CRI/lsc.h>

unsigned int adxt_time_resync_on = 1;
/* adxt_time_vsync_on adxt_svr_stage and the other time globals are adx_tlk.c's only its functions use them and the
 * DOL reaches them off one pooled base there */

/* V-sync count kept by the ADXM v-sync thread */

/* Kept for debugging only [0] samples left in the renderer [1] the last output buffer's data [2] channels */
int adxt_dbg[3];

/* ADX header and end code checks in the ADX file */

/* ADXSJD the stream joint decoder */
#define ADXSJD_STAT_DECINFO_END 2
#define ADXSJD_STAT_DECEND 3

#define ADXSTM_STAT_READEND 3
#define ADXSTM_STAT_ERROR 4
#define LSC_STAT_ERROR 3

/* At the loop start sets the next trap at the loop end and hands the decoder over to adxt_trap_entry */
// 0x802261BC
void adxt_trap_loop(void* obj)
{
	ADXT adxt;
	ADXSJD sjd;
	int lsmpl;
	int lofst;
	int esmpl;

	adxt = (ADXT)obj;
	sjd = adxt->sjd;
	lsmpl = ADXSJD_GetLpStartPos(sjd);
	lofst = ADXSJD_GetLpStartOfst(sjd);
	esmpl = ADXSJD_GetLpEndPos(sjd);
	ADXSJD_GetExtPrm(sjd);
	ADXSJD_SetTrapCnt(sjd, 0);
	ADXSJD_SetTrapNumSmpl(sjd, adxt->trpnsmpl = esmpl - lsmpl);
	ADXSJD_SetTrapDtLen(sjd, lofst);
	ADXSJD_SetDecPos(sjd, lsmpl);
	ADXSJD_EntryTrapFunc(sjd, (void (*)(void*))adxt_trap_entry, adxt);
}

/* At the loop end skips what is left of the sector and starts the decoder again from the loop start */
// 0x8022603C
void adxt_trap_entry(void* obj)
{
	ADXT adxt;
	ADXSJD sjd;
	SJ sj;
	int lsmpl;
	int esmpl;
	int lofst;
	SJCK ck;

	adxt = (ADXT)obj;
	sjd = adxt->sjd;
	sj = adxt->sji;
	lsmpl = ADXSJD_GetLpStartPos(sjd);
	lofst = ADXSJD_GetLpStartOfst(sjd);
	esmpl = ADXSJD_GetLpEndPos(sjd);
	if (adxt->stm == 0 && adxt->lpflg == 0) {
		ADXSJD_SetTrapNumSmpl(adxt->sjd, -1);
		return;
	}
	SJ_GetChunk(sj, SJ_LIN_DATA, adxt->lp_skiplen, &ck);
	if (ck.len < adxt->lp_skiplen) {
		ADXERR_CallErrFunc1("E8101201 adxt_trap_entry: not enough data");
	}
	SJ_PutChunk(sj, SJ_LIN_FREE, &ck);
	ADXSJD_SetTrapCnt(sjd, 0);
	ADXSJD_SetTrapNumSmpl(sjd, adxt->trpnsmpl = esmpl - lsmpl);
	ADXSJD_SetTrapDtLen(sjd, lofst);
	ADXSJD_SetDecPos(sjd, lsmpl);
	if (adxt->pmode == ADXT_PMODE_MEM) {
		SJ_Reset(sj);
		SJ_GetChunk(sj, SJ_LIN_DATA, lofst, &ck);
		SJ_PutChunk(sj, SJ_LIN_FREE, &ck);
	}
	ADXSJD_SetExtPrm(sjd);
	adxt->lpcnt++;
}

/* The stream hit its end sector without looping reading ends otherwise it goes back to the loop start */
// 0x80225FC4
void adxt_eos_entry(void* obj)
{
	ADXT adxt;
	ADXSTM stm;
	int ofst;

	adxt = (ADXT)obj;
	stm = adxt->stm;
	ofst = ADXSJD_GetLpStartOfst(adxt->sjd);
	if (adxt->lpflg == 0) {
		ADXSJD_SetTrapNumSmpl(adxt->sjd, -1);
		ADXSTM_SetEos(adxt->stm, 0x7FFFFFFF);
	} else {
		ADXSTM_Seek(stm, ofst / ADXSTM_SCT);
	}
}

/* At the end of the data of a linked stream skips the end code and the next header then decodes on */
// 0x80225CE4
void adxt_trap_link(void* obj)
{
	ADXT adxt;
	ADXSJD sjd;
	SJ sj;
	int skip1;
	int skip2;
	int ret1;
	int ret2;
	short hdrlen;
	short endlen;
	SJCK ck1;
	SJCK ck3;
	SJCK ck2;
	SJCK ck4;

	adxt = (ADXT)obj;
	sjd = adxt->sjd;
	sj = adxt->sji;
	if (adxt->lnkflg == 0) {
		return;
	}
	endlen = 0;
	SJ_GetChunk(sj, SJ_LIN_DATA, 0x7FFFFFFF, &ck1);
	SJ_GetChunk(sj, SJ_LIN_DATA, 0x7FFFFFFF, &ck2);
	if (ADX_DecodeInfoEnd(ck1.data, ck1.len, &hdrlen) != 0) {
		adxt->lnkflg = 0;
		SJ_UngetChunk(sj, SJ_LIN_DATA, &ck2);
		SJ_UngetChunk(sj, SJ_LIN_DATA, &ck1);
		return;
	}
	skip1 = hdrlen;
	ret1 = ADX_DecodeFooter(ck1.data + skip1, ck1.len - skip1, &hdrlen);
	if (ret1 == 0) {
		ret2 = -1;
	} else {
		ret2 = ADX_DecodeFooter(ck2.data, ck2.len, &endlen);
	}
	skip1 += hdrlen;
	skip2 = endlen;
	if (ret1 != 0 && ret2 != 0) {
		SJ_UngetChunk(sj, SJ_LIN_DATA, &ck2);
		SJ_UngetChunk(sj, SJ_LIN_DATA, &ck1);
		adxt->lnkflg = 0;
		return;
	}
	if (ret1 == 0) {
		SJ_UngetChunk(sj, SJ_LIN_DATA, &ck2);
		SJ_SplitChunk(&ck1, skip1, &ck1, &ck3);
		SJ_PutChunk(sj, SJ_LIN_FREE, &ck1);
		SJ_UngetChunk(sj, SJ_LIN_DATA, &ck3);
	} else {
		SJ_PutChunk(sj, SJ_LIN_FREE, &ck1);
		SJ_SplitChunk(&ck2, skip2, &ck2, &ck4);
		SJ_PutChunk(sj, SJ_LIN_FREE, &ck2);
		SJ_UngetChunk(sj, SJ_LIN_DATA, &ck4);
	}
	adxt->decofst += ADXSJD_GetDecNumSmpl(sjd);
	ADXSJD_Stop(sjd);
	ADXSJD_Start(sjd);
	ADXSJD_ExecHndl(sjd);
	if (ADXSJD_GetStat(sjd) != ADXSJD_STAT_DECINFO_END) {
		adxt->lnkflg = 0;
		return;
	}
	ADXSJD_SetMaxDecSmpl(sjd, adxt->maxdecsmpl);
	ADXSJD_SetTrapNumSmpl(sjd, ADXSJD_GetTotalNumSmpl(sjd));
	ADXSJD_SetTrapDtLen(sjd, 0);
	ADXSJD_SetTrapCnt(sjd, 0);
}

/* Starts the stream if it is not running yet and once the decoder has read the header sets up the loop and the
 * renderer */
// 0x802258B0
void adxt_stat_decinfo(ADXT adxt)
{
	ADXSJD sjd;
	int sfreq;
	int nloop;
	int nch;
	int nsmpl;
	int blk;
	int ofst;
	volatile int prm1;
	volatile int prm2;
	int sct;
	char buf[32];
	int bps;
	void* info;

	sjd = adxt->sjd;
	prm1 = 0;
	prm2 = 0;
	if ((adxt->pmode == ADXT_PMODE_FNAME || adxt->pmode == ADXT_PMODE_AFS) && adxt->stm_start == 1) {
		if (ADXSTM_GetStat(adxt->stm) == 2) {
			return;
		}
		if (adxt->sjf != 0) {
			SJ_Reset(adxt->sjf);
		}
		adxt_start_stm(adxt, adxt->fname, adxt->dir, adxt->ofst, adxt->nsct);
		adxt->stm_start = 0;
	}
	if (ADXSJD_GetStat(sjd) != ADXSJD_STAT_DECINFO_END) {
		return;
	}
	nch = ADXSJD_GetNumChan(sjd);
	if (nch > (signed char)adxt->maxnch) {
		ADXERR_ItoA2(nch, adxt->maxnch, buf, 16);
		ADXERR_CallErrFunc2("E9081001 adxt_stat_decinfo: can't play this number of channels", buf);
		ADXT_Stop(adxt);
		return;
	}
	sfreq = ADXSJD_GetSfreq(sjd);
	nloop = ADXSJD_GetNumLoop(sjd);
	if (nloop > 0) {
		adxt->maxdecsmpl = (sfreq / adxt->svrfreq) * 3;
	} else {
		adxt->maxdecsmpl = (sfreq / adxt->svrfreq) * 3 / 2;
	}
	blk = ADXSJD_GetBlkSmpl(sjd) * 2;
	adxt->maxdecsmpl = blk * ((adxt->maxdecsmpl + blk) / blk);
	ADXSJD_SetMaxDecSmpl(sjd, adxt->maxdecsmpl);
	if (nloop > 0) {
		if (adxt->pmode == ADXT_PMODE_MEM) {
			adxt->lp_skiplen = 0;
		} else {
			ofst = ADXSJD_GetLpEndOfst(sjd);
			adxt->lp_skiplen = ADXSTM_SCT - ofst % ADXSTM_SCT;
			adxt->lp_skiplen = adxt->lp_skiplen % ADXSTM_SCT;
			sct = (ofst + ADXSTM_SCT - 1) / ADXSTM_SCT;
			adxt->lesct = sct;
			ADXSTM_SetEos(adxt->stm, sct);
			ADXSTM_EntryEosFunc(adxt->stm, adxt_eos_entry, adxt);
		}
		ADXSJD_GetLpEndPos(sjd);
		adxt->trpnsmpl = ADXSJD_GetLpStartPos(sjd);
		ADXSJD_SetTrapNumSmpl(sjd, adxt->trpnsmpl);
		ADXSJD_SetTrapDtLen(sjd, 0);
		ADXSJD_SetTrapCnt(sjd, 0);
		ADXSJD_EntryTrapFunc(sjd, adxt_trap_loop, adxt);
	} else {
		if (adxt->stm != 0) {
			ADXSTM_SetEos(adxt->stm, 0x7FFFFFFF);
		}
		ADXSJD_SetTrapNumSmpl(sjd, ADXSJD_GetTotalNumSmpl(sjd));
		ADXSJD_SetTrapDtLen(sjd, 0);
		ADXSJD_SetTrapCnt(sjd, 0);
		ADXSJD_EntryTrapFunc(sjd, adxt_trap_link, adxt);
	}
	sfreq = ADXSJD_GetSfreq(sjd);
	nch = ADXSJD_GetNumChan(sjd);
	nsmpl = ADXSJD_GetTotalNumSmpl(sjd);
	bps = ADXSJD_GetOutBps(sjd);
	ADXRNA_SetBitPerSmpl(adxt->rna, bps);
	ADXRNA_SetSfreq(adxt->rna, sfreq);
	ADXRNA_SetNumChan(adxt->rna, nch);
	ADXRNA_SetTotalNumSmpl(adxt->rna, nsmpl);
	ADXRNA_SetOutVol(adxt->rna, adxt->outvol);
	adxt_GetDecParam(adxt, (int*)&prm1, (int*)&prm2);
	if (prm1 != 0 || prm2 != 0) {
		adxt_SetDecParam(adxt, prm1, prm2);
	}
	if (ADXSJD_GetNumChan(adxt->sjd) == 1) {
		if (adxt->outpan[0] == ADXT_PAN_AUTO) {
			ADXRNA_SetOutPan(adxt->rna, 0, ADXT_PAN_CENTER);
		} else {
			ADXRNA_SetOutPan(adxt->rna, 0, adxt->outpan[0]);
		}
	} else {
		if (adxt->outpan[0] == ADXT_PAN_AUTO) {
			ADXRNA_SetOutPan(adxt->rna, 0, ADXT_PAN_LEFT);
		} else {
			ADXRNA_SetOutPan(adxt->rna, 0, adxt->outpan[0]);
		}
		if (adxt->outpan[1] == ADXT_PAN_AUTO) {
			ADXRNA_SetOutPan(adxt->rna, 1, ADXT_PAN_RIGHT);
		} else {
			ADXRNA_SetOutPan(adxt->rna, 1, adxt->outpan[1]);
		}
	}
	if (adxt->amp != 0) {
		ADXAMP_SetSfreq(adxt->amp, sfreq);
	}
	if (ADXSJD_GetFormat(sjd) == 2) {
		info = ADXSJD_GetSpsdInfo(sjd);
		ADXRNA_SetStmHdInfo(adxt->rna, info);
	}
	ADXRNA_SetTransSw(adxt->rna, 1);
	adxt->stat = ADXT_STAT_PREP;
}

// 0x802255A8
void ADXT_ExecHndl(ADXT adxt)
{
	int* dbg;
	int nch;
	int i;
	int j;
	ADXSJD sjd;
	ADXRNA rna;
	int ndata;
	int nroom;
	int len;
	SJ sj;
	SJCK ck;

	dbg = adxt_dbg;
	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080842 ADXT_ExecHndl: parameter error");
		return;
	}
	if (adxt->stat == ADXT_STAT_PLAYING) {
		if (ADXSJD_GetStat(adxt->sjd) == ADXSJD_STAT_DECEND) {
			nch = ADXSJD_GetNumChan(adxt->sjd);
			dbg[2] = nch;
			for (i = 0; i < nch; i++) {
				dbg[1] = SJ_GetNumData(adxt->sjo[i], SJ_LIN_DATA);
				if (dbg[1] >= 0x40) {
					break;
				}
			}
			if (i == nch) {
				ADXRNA_SetTransSw(adxt->rna, 0);
				adxt->stat = ADXT_STAT_DECEND;
			}
		}
	} else if (adxt->stat == ADXT_STAT_DECINFO) {
		adxt_stat_decinfo(adxt);
	} else if (adxt->stat == ADXT_STAT_PREP) {
		rna = adxt->rna;
		sjd = adxt->sjd;
		ndata = ADXRNA_GetNumData(rna);
		nroom = ADXRNA_GetNumRoom(rna);
		if (ndata >= adxt->maxdecsmpl * 2 || nroom <= ADXSJD_GetBlkSmpl(sjd) || ADXSJD_GetStat(adxt->sjd) == ADXSJD_STAT_DECEND) {
			if (adxt->pstwait_flag == 0) {
				if (adxt->pause_flag == 0) {
					ADXRNA_SetPlaySw(rna, 1);
					adxt->tvofst = 0;
					adxt->svcnt = adxt_svcnt;
				}
				adxt->stat = ADXT_STAT_PLAYING;
			}
			adxt->pstready_flag = 1;
		}
		if (ADXSJD_GetStat(adxt->sjd) == ADXSJD_STAT_DECEND) {
			nch = ADXT_GetNumChan(adxt);
			len = adxt->maxdecsmpl * nch * 2;
			for (j = 0; j < nch; j++) {
				sj = adxt->sjo[j];
				SJ_GetChunk(sj, SJ_LIN_FREE, len, &ck);
				memset(ck.data, 0, ck.len);
				SJ_PutChunk(sj, SJ_LIN_DATA, &ck);
			}
		}
	} else if (adxt->stat == ADXT_STAT_DECEND) {
		dbg[0] = ADXRNA_GetNumData(adxt->rna);
		if (ADXRNA_GetNumData(adxt->rna) <= 0) {
			ADXRNA_SetPlaySw(adxt->rna, 0);
			adxt->stat = ADXT_STAT_PLAYEND;
		}
	}
	if (adxt->stm != 0 && ADXT_GetStat(adxt) != ADXT_STAT_STOP) {
		switch (adxt->pmode) {
		case ADXT_PMODE_FNAME:
		case ADXT_PMODE_AFS:
			if (ADXSTM_GetStat(adxt->stm) == ADXSTM_STAT_READEND) {
				ADXSJD_TermSupply(adxt->sjd);
			}
			break;
		case ADXT_PMODE_MEM:
			ADXSJD_TermSupply(adxt->sjd);
			break;
		case ADXT_PMODE_SJ:
			break;
		default:
			break;
		}
	}
	if (adxt->stm != 0 && ADXSTM_GetStat(adxt->stm) == ADXSTM_STAT_ERROR) {
		adxt->ercode = -1;
		adxt->stat = ADXT_STAT_ERROR;
	}
	if (adxt->lsc != 0 && LSC_GetStat(adxt->lsc) == LSC_STAT_ERROR) {
		adxt->ercode = -1;
		adxt->stat = ADXT_STAT_ERROR;
	}
}
