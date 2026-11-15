#include <CRI/cri_types.h>
#ifndef CRI_LSC_H
#define CRI_LSC_H

#define LSC_MAX_OBJ 16
#define LSC_MAX_STM 16

typedef struct {
	int id;            /* 00 */
	const char* fname; /* 04 */
	int chksum;        /* 08 */
	void* dir;         /* 0C */
	int ofst;          /* 10 */
	int nsct;          /* 14 */
	int stat;          /* 18 */
	int rd_sct;        /* 1C */
} LSC_STM;

typedef struct _lsc_obj {
	signed char used;   /* 00 */
	signed char stat;   /* 01 */
	signed char stm_on; /* 02 */
	signed char lp_flg; /* 03 */
	signed char pause;  /* 04 */
	SJ sj;              /* 08 */
	int unkC;           /* 0C */
	int unk10;          /* 10 */
	int flow_limit;     /* 14 */
	int sj_size;        /* 18 */
	int wpos;           /* 1C */
	int rpos;           /* 20 */
	int num_stm;        /* 24 */
	ADXSTM stm;         /* 28 */
	int rd_sct;         /* 2C */
	int unk30;          /* 30 */
	int unk34;          /* 34 */
	LSC_STM stms[LSC_MAX_STM]; /* 38 */
} LSC_OBJ;

void LSC_CallErrFunc(const char* fmt, ...);
void LSC_EntryErrFunc(void (*func)(void*, char*), void* obj);
void LSC_LockCrs(int* key);
void LSC_UnlockCrs(int* key);
void lsc_CallStatFunc(void);
int LSC_EntryFileRange(LSC lsc, const char* fname, void* dir, int ofst, int nsct);
void lsc_ExecHndl(LSC lsc);
void LSC_Destroy(LSC lsc);

extern LSC_OBJ lsc_obj[LSC_MAX_OBJ];

void LSC_Init(void);
void LSC_Finish(void);
void LSC_ExecServer(void);
LSC LSC_Create(SJ sj);
void LSC_Stop(LSC lsc);
void LSC_SetStmHndl(LSC lsc, ADXSTM stm);
int LSC_GetStat(LSC lsc);

#endif
