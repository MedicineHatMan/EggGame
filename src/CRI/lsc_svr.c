#include <CRI/lsc.h>
#include <CRI/adx_stmc.h>

#pragma optimization_level 2
// 0x8022BD08
void lsc_ExecHndl(LSC lsc)
{
	LSC_STM* s;
	int st;
	int nsct;
	int ofst;
	int dir;
	int fname;

	if (lsc->pause == 1) {
		return;
	}
	if (lsc->stat != 2) {
		return;
	}
	if (lsc->num_stm <= 0) {
		return;
	}
	if (lsc->stms[lsc->rpos].stat == 1) {
		if (lsc->stm == 0) {
			LSC_CallErrFunc("E0007: lsc->fp=NULL\n");
		} else {
			s = &lsc->stms[lsc->rpos];
			st = ADXSTM_GetStat(lsc->stm);
			if (st == 4) {
				lsc->stat = 3;
			} else if (st == 2) {
				s->rd_sct = ADXSTM_Tell(lsc->stm);
			} else if (st == 3) {
				s->rd_sct = lsc->rd_sct;
				s->stat = 2;
			}
		}
	}
	if (lsc->stms[lsc->rpos].stat == 2) {
		nsct = ofst = dir = fname = 0;
		if (lsc->stm != 0) {
			if (lsc->lp_flg == 1) {
				LSC_STM* e = &lsc->stms[lsc->rpos];
				fname = (int)e->fname;
				dir = (int)e->dir;
				ofst = e->ofst;
				nsct = e->nsct;
			}
			lsc->num_stm--;
			lsc->rpos = (lsc->rpos + 1) % LSC_MAX_STM;
			if (lsc->num_stm <= 0) {
				lsc_CallStatFunc();
				lsc->stat = 1;
			}
			if (lsc->lp_flg == 1) {
				LSC_EntryFileRange(lsc, (const char*)fname, (void*)dir, ofst, nsct);
			}
		}
	}
	if (lsc->stms[lsc->rpos].stat == 0) {
		s = &lsc->stms[lsc->rpos];
		if (lsc->num_stm > 0) {
			ADXSTM_StopNw(lsc->stm);
			ADXSTM_ReleaseFileNw(lsc->stm);
			ADXSTM_BindFileNw(lsc->stm, s->fname, s->dir, s->ofst, s->nsct);
			ADXSTM_SetEos(lsc->stm, s->nsct);
			lsc->rd_sct = s->nsct;
			s->rd_sct = 0;
			lsc->stm_on = 0;
			if (lsc->stm_on == 0) {
				ADXSTM_SetBufSize(lsc->stm, lsc->flow_limit, lsc->sj_size);
				ADXSTM_Seek(lsc->stm, 0);
				ADXSTM_Start(lsc->stm);
				lsc->stm_on = 1;
			}
			s->stat = 1;
		}
	}
}
#pragma optimization_level reset
