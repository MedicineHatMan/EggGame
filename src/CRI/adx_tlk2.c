#include <CRI/adx_tlk.h>
#include <CRI/adx.h>
#include <CRI/adx_fs.h>
#include <CRI/cri_std.h>

// 0x802254B0
void ADXT_StartAfs(ADXT adxt, int patid, int fid)
{
	int ofst;
	int nsct;
	void* dir;
	char buf[16];

	if (adxt == 0) {
		ADXERR_CallErrFunc1("E02080811 ADXT_StartAfs: parameter error");
		return;
	}
	ADXT_Stop(adxt);
	if (ADXF_GetFnameRangeEx(patid, fid, adxt->fname_buf, &dir, &ofst, &nsct) != 0) {
		return;
	}
	if (adxt->stm == 0) {
		ADXERR_ItoA2(patid, fid, buf, sizeof(buf));
		ADXERR_CallErrFunc2("E8101202 ADXT_StartAfs: can't open ", buf);
		adxt->ercode = -1;
		adxt->stat = ADXT_STAT_ERROR;
		return;
	}
	adxt->fname = adxt->fname_buf;
	adxt->dir = dir;
	adxt->ofst = ofst;
	adxt->nsct = nsct;
	adxt->stat = ADXT_STAT_DECINFO;
	adxt->stm_start = 1;
	adxt->pmode = ADXT_PMODE_AFS;
	adxt->lnkflg = 0;
}

// 0x8022541C
void ADXT_StartFname(ADXT adxt, const char* fname)
{
	if (adxt == 0 || fname == 0) {
		ADXERR_CallErrFunc1("E02080807 ADXT_StartFname: parameter error");
		return;
	}
	ADXT_Stop(adxt);
	strcpy(adxt->fname_buf, fname);
	adxt->fname = adxt->fname_buf;
	adxt->dir = 0;
	adxt->ofst = 0;
	adxt->nsct = ADXSTM_RD_ALL;
	adxt->stat = ADXT_STAT_DECINFO;
	adxt->stm_start = 1;
	adxt->pmode = ADXT_PMODE_FNAME;
	adxt->lnkflg = 0;
}
