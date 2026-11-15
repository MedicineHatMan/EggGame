#include <CRI/cri_types.h>
#ifndef CRI_ADX_TLK_H
#define CRI_ADX_TLK_H

#include <CRI/sj.h>
#include <CRI/adx_stmc.h>

#define ADXT_MAX_OBJ 16
#define ADXT_MAX_NCH 2

#define ADXT_STAT_STOP    0
#define ADXT_STAT_DECINFO 1
#define ADXT_STAT_PREP    2
#define ADXT_STAT_PLAYING 3
#define ADXT_STAT_DECEND  4
#define ADXT_STAT_PLAYEND 5
#define ADXT_STAT_ERROR   6

#define ADXT_PMODE_FNAME  0
#define ADXT_PMODE_AFS    1
#define ADXT_PMODE_MEM    2
#define ADXT_PMODE_SJ     3
#define ADXT_PMODE_SLFILE 4

#define ADXT_PAN_LEFT   (-15)
#define ADXT_PAN_CENTER 0
#define ADXT_PAN_RIGHT  15
#define ADXT_PAN_AUTO   (-128)

#define ADXT_DEF_SVRFREQ 60

#define ADXT_OBUF_WORK 0x60C0
#define ADXT_OBUF_SIZE 0x2000
#define ADXT_OBUF_DIST 0x2060
#define ADXT_IBUF_XLEN 0x24

typedef struct {
	signed char used;           /* 00 */
	signed char stat;           /* 01 */
	signed char pmode;          /* 02 */
	signed char maxnch;         /* 03 */
	ADXSJD sjd;                 /* 04 */
	ADXSTM stm;                 /* 08 */
	ADXRNA rna;                 /* 0C */
	SJ sjf;                     /* 10 */
	SJ sji;                     /* 14 */
	SJ sjo[ADXT_MAX_NCH];       /* 18 */
	char* ibuf;                 /* 20 */
	int ibuflen;                /* 24 */
	int ibufxlen;               /* 28 */
	short* obuf;                /* 2C */
	int obufsize;               /* 30 */
	int obufdist;               /* 34 */
	int svrfreq;                /* 38 */
	short maxsct;               /* 3C */
	short minsct;               /* 3E */
	short outvol;               /* 40 */
	short outpan[ADXT_MAX_NCH]; /* 42 */
	short pad46;                /* 46 */
	int maxdecsmpl;             /* 48 */
	int lpcnt;                  /* 4C */
	int lp_skiplen;             /* 50 */
	int trp;                    /* 54 */
	int wpos;                   /* 58 */
	int mofst;                  /* 5C */
	short ercode;               /* 60 */
	int edecpos;                /* 64 */
	short edeccnt;              /* 68 */
	short eshrtcnt;             /* 6A */
	signed char lpflg;          /* 6C */
	signed char autorcvr;       /* 6D */
	signed char fltmode;        /* 6E */
	signed char execflag;       /* 6F */
	signed char pstwait_flag;   /* 70 */
	signed char pstready_flag;  /* 71 */
	signed char pause_flag;     /* 72 */
	ADXAMP amp;                 /* 74 */
	SJ ampsji[ADXT_MAX_NCH];    /* 78 */
	SJ ampsjo[ADXT_MAX_NCH];    /* 80 */
	int time_ofst;              /* 88 */
	int lesct;                  /* 8C */
	int trpnsmpl;               /* 90 */
	LSC lsc;                    /* 94 */
	signed char lnkflg;         /* 98 */
	signed char rsv;            /* 99 */
	short rsv2;                 /* 9A */
	unsigned int tvofst;        /* 9C */
	unsigned int svcnt;         /* A0 */
	unsigned int decofst;       /* A4 */
	signed char stm_start;      /* A8 */
	char padA9[3];              /* A9 */
	char* fname_buf;            /* AC */
	const char* fname;          /* B0 */
	void* dir;                  /* B4 */
	int ofst;                   /* B8 */
	int nsct;                   /* BC */
} ADX_TALK;

typedef ADX_TALK* ADXT;

void ADXT_SetTimeOfst(ADXT adxt, int ofst);
int ADXT_IsHeader(const unsigned char* adr, int siz, int* hdrsiz);
int ADXT_IsEndcode(const unsigned char* adr, int siz, int* endsiz);
int ADXT_InsertSilence(ADXT adxt, int nch, int nsmpl);
void ADXT_Pause(ADXT adxt, int sw);
void ADXT_ClearErrCode(ADXT adxt);
int ADXT_GetErrCode(ADXT adxt);
void ADXT_ExecServer(void);
void ADXT_SetAutoRcvr(ADXT adxt, int rmode);
void ADXT_SetSvrFreq(ADXT adxt, int freq);
int ADXT_GetOutVol(ADXT adxt);
void ADXT_SetOutVol(ADXT adxt, int vol);
int ADXT_GetOutPan(ADXT adxt, int ch);
void ADXT_SetOutPan(ADXT adxt, int ch, int pan);
int ADXT_GetNumChan(ADXT adxt);
int ADXT_GetSfreq(ADXT adxt);
int ADXT_GetNumSmpl(ADXT adxt);
void ADXT_GetTime(ADXT adxt, int* ncount, int* tscale);
int ADXT_GetStat(ADXT adxt);
void ADXT_Stop(ADXT adxt);
void ADXT_StartSj(ADXT adxt, SJ sj);
void adxt_start_stm(ADXT adxt, const char* fname, void* dir, int ofst, int nsct);
void ADXT_Destroy(ADXT adxt);
ADXT ADXT_Create(int maxnch, void* work, int worksize);
void ADXT_StartFname(ADXT adxt, const char* fname);
void ADXT_StartAfs(ADXT adxt, int patid, int fid);

void ADXT_ExecHndl(ADXT adxt);
void adxt_stat_decinfo(ADXT adxt);
void adxt_eos_entry(void* obj);
void adxt_trap_entry(void* obj);

/* ADXRNA the renderer a thin layer over the AXRNA calls in the next file */
void ADXRNA_Init(void);
void ADXRNA_Finish(void);
void ADXRNA_EntryErrFunc(void (*func)(void*, char*), void* obj);
ADXRNA ADXRNA_Create(SJ* sjo, int maxnch, short* work);
void ADXRNA_Destroy(ADXRNA rna);
void ADXRNA_SetTransSw(ADXRNA rna, int sw);
void ADXRNA_SetPlaySw(ADXRNA rna, int sw);
int ADXRNA_GetNumData(ADXRNA rna);
int ADXRNA_GetNumRoom(ADXRNA rna);
void ADXRNA_ExecServer(void);
void ADXRNA_SetNumChan(ADXRNA rna, int nch);
void ADXRNA_SetSfreq(ADXRNA rna, int sfreq);
void ADXRNA_SetOutVol(ADXRNA rna, int vol);
void ADXRNA_SetOutPan(ADXRNA rna, int ch, int pan);
void ADXRNA_SetBitPerSmpl(ADXRNA rna, int bps);
void ADXRNA_SetTotalNumSmpl(ADXRNA rna, int nsmpl);
int ADXRNA_SetStmHdInfo(ADXRNA rna, void* info);

void ADXRNA_SetSfreqFix(ADXRNA rna, short sw);
void ADXT_ExecFsSvr(void);
int ADXRNA_GetPauseTime(ADXRNA rna);
void adxt_GetDecParam(ADXT adxt, int* a, int* b);
void adxt_SetDecParam(ADXT adxt, int a, int b);

extern unsigned int adxt_svcnt;
extern ADX_TALK adxt_obj[ADXT_MAX_OBJ];

extern int adxt_time_vsync_on;
extern int adxt_svr_stage;
extern int adxt_time_resync_cnt;
extern float adxt_time_diff_ms;
extern int adxt_vsync_rate;

extern unsigned int adxt_time_resync_on;

extern void (*adxt_destroy_cb)(ADXT adxt);

#endif
