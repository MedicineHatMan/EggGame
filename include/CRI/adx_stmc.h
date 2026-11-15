#include <CRI/cri_types.h>
#ifndef CRI_ADX_STMC_H
#define CRI_ADX_STMC_H

#include <CRI/sj.h>
#include <CRI/cri_cvfs.h>

#define ADXSTM_MAX 0x28
#define ADXSTM_SCT 0x800
#define ADXSTM_RD_ALL 0xFFFFF

typedef struct _adxstm_obj {
	signed char used;            /* 00 */
	signed char stat;            /* 01 */
	signed char read_flg;        /* 02 */
	signed char err_cnt;         /* 03 */
	SJ sj;                       /* 04 */
	CVFS cvfs;                   /* 08 */
	int ofst;                    /* 0C */
	int file_len;                /* 10 */
	int buf_max;                 /* 14 */
	int flow_limit;              /* 18 */
	int rd_sct;                  /* 1C */
	SJCK ck;                     /* 20 */
	int req_rd;                  /* 28 */
	int eos;                     /* 2C */
	int rd_bytes;                /* 30 */
	void (*eos_func)(void* obj); /* 34 */
	void* eos_obj;               /* 38 */
	int sj_total;                /* 3C */
	signed char pause;           /* 40 */
	signed char bind_req;        /* 41 */
	signed char close_req;       /* 42 */
	signed char start_req;       /* 43 */
	signed char stop_req;        /* 44 */
	signed char cvfs_open;       /* 45 */
	signed char pad46;           /* 46 */
	signed char pad47;           /* 47 */
	int pad48;                   /* 48 */
	const char* fname;           /* 4C */
	void* dir;                   /* 50 */
	int cur_ofst;                /* 54 */
	unsigned int rd_limit;       /* 58 */
} ADXSTM_OBJ;

int ADXSTM_Init(void);
void ADXSTM_Finish(void);
ADXSTM ADXSTM_Create(SJ sj, int maxsct);
void ADXSTM_Destroy(ADXSTM stm);
void ADXSTM_SetSj(ADXSTM stm, SJ sj);
void ADXSTM_SetPause(ADXSTM stm, int sw);
int ADXSTM_GetFileLen(ADXSTM stm);
int ADXSTM_SetReqRdSize(ADXSTM stm, int size);
int ADXSTM_SetBufSize(ADXSTM stm, int min, int max);
void ADXSTM_ExecServer(void);
void ADXSTM_SetEos(ADXSTM stm, int eos);
void ADXSTM_EntryEosFunc(ADXSTM stm, void (*func)(void*), void* obj);
void ADXSTM_Stop(ADXSTM stm);
void ADXSTM_StopNw(ADXSTM stm);
int ADXSTM_Start2(ADXSTM stm, unsigned int limit);
int ADXSTM_Start(ADXSTM stm);
int ADXSTM_Tell(ADXSTM stm);
int ADXSTM_Seek(ADXSTM stm, int ofst);
int ADXSTM_GetStat(ADXSTM stm);
void ADXSTM_ReleaseFile(ADXSTM stm);
void ADXSTM_ReleaseFileNw(ADXSTM stm);
void ADXSTM_BindFile(ADXSTM stm, const char* fname, void* dir, int ofst, int nsct);
void ADXSTM_BindFileNw(ADXSTM stm, const char* fname, void* dir, int ofst, int nsct);

#endif
