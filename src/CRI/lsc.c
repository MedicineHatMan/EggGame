#include <CRI/lsc.h>
#include <CRI/adx.h>
#include <CRI/adx_stmc.h>

typedef struct {
	void (*func)(void*, void*);
	void* a;
	void* b;
} LSC_STATCB;

static LSC_STATCB lsc_stat_cb;

static inline void lsc_Clear(LSC lsc)
{
	if (lsc == 0) {
		LSC_CallErrFunc("E0003: Illigal parameter lsc=NULL\n");
		return;
	}
	if (lsc->stat == 0) {
		lsc->wpos    = 0;
		lsc->rpos    = 0;
		lsc->num_stm = 0;
	}
}

void LSC_Stop(LSC lsc);

// 0x8022BA60
static inline LSC lsc_AllocHn(void)
{
	LSC lsc;
	int i;

	lsc = 0;
	for (i = 0; i < LSC_MAX_OBJ; i++) {
		if (lsc_obj[i].used == 0) {
			lsc = &lsc_obj[i];
			break;
		}
	}
	return lsc;
}

LSC LSC_Create(SJ sj)
{
	int key;
	long j;
	LSC lsc;
	int a;
	int b;

	if (sj == 0) {
		LSC_CallErrFunc("E0001: Illigal parameter=sj (LSC_Create)\n");
		return 0;
	}
	LSC_LockCrs(&key);
	lsc = lsc_AllocHn();
	if (lsc == 0) {
		LSC_CallErrFunc("E0002: Not enough instance (LSC_Create)\n");
	} else {
		lsc->sj         = sj;
		lsc->stat       = 0;
		a               = (*(int (***)(SJ, int))sj)[9](sj, 1);
		b               = (*(int (***)(SJ, int))sj)[9](sj, 0);
		lsc->sj_size    = b + a;
		lsc->flow_limit = (lsc->sj_size * 8) / 10;
		j               = 0;
		while (j < LSC_MAX_STM) {
			lsc->stms[j].stat = 0;
			j++;
		}
		lsc->used = 1;
	}
	LSC_UnlockCrs(&key);
	return lsc;
}

// 0x8022B988
void LSC_Destroy(LSC lsc)
{
	if (lsc != 0) {
		LSC_Stop(lsc);
		lsc->used = 0;
		memset(lsc, 0, sizeof(LSC_OBJ));
	}
}

// 0x8022B980
void LSC_SetStmHndl(LSC lsc, ADXSTM stm)
{
	lsc->stm = stm;
}

// 0x8022B950
int LSC_EntryFname(LSC lsc, const char* fname)
{
	return LSC_EntryFileRange(lsc, fname, 0, 0, 0xFFFFF);
}

// 0x8022B710
int LSC_EntryFileRange(LSC lsc, const char* fname, void* dir, int ofst, int nsct)
{
	LSC_STM* s;
	int id;
	const int* p;
	int prev;
	unsigned int i;
	unsigned int n;

	if (lsc == 0) {
		LSC_CallErrFunc("E0003: Illigal parameter lsc=NULL\n");
		return -1;
	}
	if (lsc->num_stm >= LSC_MAX_STM) {
		return -1;
	}
	if (fname == 0) {
		LSC_CallErrFunc("E0011: Illigal parameter fname=%s\n", fname);
		return -1;
	}
	prev = lsc->stms[(lsc->wpos + 15) % LSC_MAX_STM].id;
	s    = &lsc->stms[lsc->wpos];
	if (prev == 0x7FFFFFFF) {
		id = 0;
	} else {
		id = prev + 1;
	}
	s->id     = id;
	s->fname  = fname;
	p         = (const int*)fname;
	n         = strlen(fname) >> 2;
	s->chksum = 0;
	for (i = 0; i < n; i++) {
		s->chksum += p[i];
	}
	s->ofst   = ofst;
	s->nsct   = nsct;
	s->dir    = dir;
	s->stat   = 0;
	s->rd_sct = 0;
	lsc->num_stm++;
	lsc->wpos = (lsc->wpos + 1) % LSC_MAX_STM;
	if (lsc->stat == 1) {
		lsc->stat = 2;
	}
	return id;
}

// 0x8022B600
void LSC_Start(LSC lsc)
{
	int key;

	if (lsc == 0) {
		LSC_CallErrFunc("E0003: Illigal parameter lsc=NULL\n");
		return;
	}
	LSC_LockCrs(&key);
	if (lsc->stat != 0) {
		LSC_Stop(lsc);
	}
	if (lsc->num_stm > 0) {
		lsc->stat = 2;
	} else {
		lsc->stat = 1;
	}
	LSC_UnlockCrs(&key);
}

// 0x8022B544
void LSC_Stop(LSC lsc)
{
	if (lsc == 0) {
		LSC_CallErrFunc("E0003: Illigal parameter lsc=NULL\n");
		return;
	}
	if (lsc->stat != 0) {
		lsc->stat = 0;
		if (lsc->stm != 0 && lsc->stm_on == 1) {
			ADXSTM_Stop(lsc->stm);
			lsc->stm_on = 0;
		}
		lsc->rd_sct = 0;
		lsc_Clear(lsc);
		lsc->unk34 = 0;
	}
}

// 0x8022B4D8
void LSC_ExecServer(void)
{
	int key;
	int i;

	LSC_LockCrs(&key);
	for (i = 0; i < LSC_MAX_OBJ; i++) {
		if (lsc_obj[i].used == 1) {
			lsc_ExecHndl(&lsc_obj[i]);
		}
	}
	LSC_UnlockCrs(&key);
}

// 0x8022B494
int LSC_GetStat(LSC lsc)
{
	if (lsc == 0) {
		LSC_CallErrFunc("E0003: Illigal parameter lsc=NULL\n");
		return -1;
	}
	return lsc->stat;
}

// 0x8022B454
int LSC_GetNumStm(LSC lsc)
{
	if (lsc == 0) {
		LSC_CallErrFunc("E0003: Illigal parameter lsc=NULL\n");
		return -1;
	}
	return lsc->num_stm;
}

// 0x8022B3C4
int LSC_GetStmId(LSC lsc, int no)
{
	if (lsc == 0) {
		LSC_CallErrFunc("E0003: Illigal parameter lsc=NULL\n");
		return -1;
	}
	if (no < 0 || no >= lsc->num_stm) {
		LSC_CallErrFunc("E0009: Illigal parameter no=%d\n", no);
		return -1;
	}
	return lsc->stms[(lsc->rpos + no) % LSC_MAX_STM].id;
}

// 0x8022B2C4
const char* LSC_GetStmFname(LSC lsc, int id)
{
	int i;

	if (lsc == 0) {
		LSC_CallErrFunc("E0003: Illigal parameter lsc=NULL\n");
		return 0;
	}
	for (i = 0; i < LSC_MAX_STM; i++) {
		if (lsc->stms[i].id == id) {
			break;
		}
	}
	if (i == LSC_MAX_STM) {
		LSC_CallErrFunc("E0012: Can not find stream ID =%d\n", id);
		return 0;
	}
	return lsc->stms[i].fname;
}

// 0x8022B260
void LSC_SetFlowLimit(LSC lsc, int min)
{
	if (lsc == 0) {
		LSC_CallErrFunc("E0003: Illigal parameter lsc=NULL\n");
		return;
	}
	if (min < 0 || min > lsc->sj_size) {
		LSC_CallErrFunc("E0010: Illigal parameter min=%d\n", min);
		return;
	}
	lsc->flow_limit = min;
}

// 0x8022B220
void lsc_CallStatFunc(void)
{
	if (lsc_stat_cb.func != 0) {
		lsc_stat_cb.func(lsc_stat_cb.a, lsc_stat_cb.b);
	}
}

// 0x8022B1E4
void LSC_SetLpFlg(LSC lsc, int flg)
{
	if (lsc == 0) {
		LSC_CallErrFunc("E0003: Illigal parameter lsc=NULL\n");
		return;
	}
	lsc->lp_flg = flg;
}
