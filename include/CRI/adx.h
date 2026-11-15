#include <CRI/cri_types.h>
#ifndef CRI_ADX_H
#define CRI_ADX_H

#include <CRI/sj.h>

extern void* memset(void*, int, unsigned long);
extern void* memcpy(void*, const void*, unsigned long);
extern int memcmp(const void*, const void*, unsigned long);
extern unsigned long strlen(const char*);
extern char* strncpy(char*, const char*, unsigned long);
extern char* strncat(char*, const char*, unsigned long);
extern int sprintf(char*, const char*, ...);

/* ---- ADXPD the ADPCM expander */

extern void ADXPD_Init(void);                                                                      /* ADXPD_Init */
extern ADXPD ADXPD_Create(void);                                                                   /* ADXPD_Create */
extern void ADXPD_Destroy(ADXPD xpd);                                                              /* ADXPD_Destroy */
extern int ADXPD_GetStat(ADXPD xpd);                                                               /* ADXPD_GetStat */
extern void ADXPD_Stop(ADXPD xpd);                                                                 /* ADXPD_Stop */
extern void ADXPD_Reset(ADXPD xpd);                                                                /* ADXPD_Reset */
extern int ADXPD_GetNumBlk(ADXPD xpd);                                                             /* ADXPD_GetNumBlk */
extern void ADXPD_Start(ADXPD xpd);                                                                /* ADXPD_Start */
extern void ADXPD_ExecHndl(ADXPD xpd);                                                             /* ADXPD_ExecHndl */
extern int ADXPD_EntrySte(ADXPD xpd, unsigned char* ibuf, int nblk, short* obuf_l, short* obuf_r); /* ADXPD_EntrySte */
extern int ADXPD_EntryMono(ADXPD xpd, unsigned char* ibuf, int nblk, short* obuf);                 /* ADXPD_EntryMono */
extern void ADXPD_SetCoef(ADXPD xpd, int sfreq, int cof);                                          /* ADXPD_SetCoef */
extern void ADXPD_SetDly(ADXPD xpd, short* dly0, short* dly1);                                     /* ADXPD_SetDly */
extern void ADXPD_GetDly(ADXPD xpd, short* dly0, short* dly1);                                     /* ADXPD_GetDly */
extern void ADXPD_SetExtPrm(ADXPD xpd, short k0, short km, short ka);                              /* ADXPD_SetExtPrm */
extern void ADXPD_GetExtPrm(ADXPD xpd, short* k0, short* km, short* ka);                           /* ADXPD_GetExtPrm */

/* ---- ADXB ---- */
#define ADXB_MAX_OBJ 16

#define ADXB_STAT_STOP   0
#define ADXB_STAT_EXEC   1
#define ADXB_STAT_DECODE 2
#define ADXB_STAT_DECEND 3

#define ADXB_FMT_ADX  0
#define ADXB_FMT_WAV  1
#define ADXB_FMT_SPSD 2
#define ADXB_FMT_AIFF 3
#define ADXB_FMT_AU   4
#define ADXB_FMT_AHX  10
#define ADXB_FMT_AC3  20

typedef void* (*ADXB_GETWRFN)(void* obj, int* wpos, int* wlen, int* nsmpl);
typedef void (*ADXB_ADDWRFN)(void* obj, int dtlen, int nsmpl);

typedef struct {
	short used;          /* 00 */
	short hdr_ok;        /* 02 */
	int stat;            /* 04 */
	ADXPD xpd;           /* 08 */
	signed char type;    /* 0C */
	signed char bps;     /* 0D */
	signed char nch;     /* 0E */
	signed char blk_len; /* 0F */
	int blk_smpl;        /* 10 */
	int sfreq;           /* 14 */
	int total_smpl;      /* 18 */
	short cof;           /* 1C */
	short pad1E;         /* 1E */
	int lp_ins_nsmpl;    /* 20 */
	short num_loop;      /* 24 */
	short lp_type;       /* 26 */
	int lp_spos;         /* 28 */
	int lp_sofst;        /* 2C */
	int lp_epos;         /* 30 */
	int lp_eofst;        /* 34 */
	int maxnch;          /* 38 */
	short* pcm_buf;      /* 3C */
	int pcm_bsize;       /* 40 */
	int pcm_bdist;       /* 44 */
	void* ibuf;          /* 48 */
	int ibuf_nblk;       /* 4C */
	int dec_nch;         /* 50 */
	int dec_blk_len;     /* 54 */
	int dec_blk_smpl;    /* 58 */
	short* obuf;         /* 5C */
	int obuf_size;       /* 60 */
	int obuf_dist;       /* 64 */
	int wpos;            /* 68 */
	int wlen;            /* 6C */
	int wnsmpl;          /* 70 */
	int unk74;           /* 74 */
	ADXB_GETWRFN get_wr; /* 78 */
	void* get_wr_obj;    /* 7C */
	ADXB_ADDWRFN add_wr; /* 80 */
	void* add_wr_obj;    /* 84 */
	int total_dec_smpl;  /* 88 */
	int pcm_wpos;        /* 8C */
	int dec_nsmpl;       /* 90 */
	int dec_dtlen;       /* 94 */
	short format;        /* 98 */
	short pcm_type;      /* 9A */
	short key0;          /* 9C */
	short keym;          /* 9E */
	short keya;          /* A0 */
	short ext_k0;        /* A2 */
	short ext_km;        /* A4 */
	short ext_ka;        /* A6 */
	short dly0[2];       /* A8 */
	short dly1[2];       /* AC */
	void* ahx;           /* B0 */
	int ahx_dec_smpl;    /* B4 */
	int ahx_dec_frm;     /* B8 */
} ADXB_OBJ;

typedef ADXB_OBJ* ADXB;

void ADXB_Init(void);
ADXB ADXB_Create(int maxnch, short* pcm_buf, int pcm_bsize, int pcm_bdist);
void ADXB_Destroy(ADXB adxb);
int ADXB_DecodeHeader(ADXB adxb, void* hdr, int len);
int ADXB_DecodeHeaderAdx(ADXB adxb, void* hdr, int len);
void ADXB_EntryGetWrFunc(ADXB adxb, ADXB_GETWRFN func, void* obj);
short* ADXB_GetPcmBuf(ADXB adxb);
int ADXB_GetFormat(ADXB adxb);
int ADXB_GetSfreq(ADXB adxb);
int ADXB_GetNumChan(ADXB adxb);
int ADXB_GetOutBps(ADXB adxb);
int ADXB_GetBlkSmpl(ADXB adxb);
int ADXB_GetTotalNumSmpl(ADXB adxb);
int ADXB_GetNumLoop(ADXB adxb);
int ADXB_GetLpStartPos(ADXB adxb);
int ADXB_GetLpStartOfst(ADXB adxb);
int ADXB_GetLpEndPos(ADXB adxb);
int ADXB_GetLpEndOfst(ADXB adxb);
void ADXB_SetExtPrm(ADXB adxb);
void ADXB_GetExtPrm(ADXB adxb);
int ADXB_GetStat(ADXB adxb);
void ADXB_EntryData(ADXB adxb, void* ibuf, int len);
void ADXB_Start(ADXB adxb);
void ADXB_Stop(ADXB adxb);
void ADXB_Reset(ADXB adxb);
int ADXB_GetDecDtLen(ADXB adxb);
int ADXB_GetDecNumSmpl(ADXB adxb);
void ADXB_ExecOneAdx(ADXB adxb);
void ADXB_ExecHndl(ADXB adxb);

int ADXB_CheckWav(void* hdr);
int ADXB_DecodeHeaderWav(ADXB adxb, void* hdr, int len);
void ADXB_ExecOneWav(ADXB adxb);
int ADXB_CheckAiff(void* hdr);
int ADXB_DecodeHeaderAiff(ADXB adxb, void* hdr, int len);
void ADXB_ExecOneAiff(ADXB adxb);
int ADXB_CheckAu(void* hdr);
int ADXB_DecodeHeaderAu(ADXB adxb, void* hdr, int len);
void ADXB_ExecOneAu(ADXB adxb);
int ADXB_CheckSpsd(void* hdr);
int ADXB_DecodeHeaderSpsd(ADXB adxb, void* hdr, int len);
void ADXB_ExecOneSpsd(ADXB adxb);
void ADXB_ExecOneAhx(ADXB adxb);
void ADXB_SetAhxDecSmpl(ADXB adxb, int nsmpl);
void ADXB_SetAhxInSj(ADXB adxb, SJ sj);

/* Shared tail of the PCM header readers the decode parameters follow the header fields */
#define ADXB_SET_PCM_PRM(adxb)                      \
	do {                                            \
		(adxb)->cof            = 0;                 \
		(adxb)->lp_type        = 0;                 \
		(adxb)->num_loop       = 0;                 \
		(adxb)->lp_eofst       = 0;                 \
		(adxb)->lp_epos        = 0;                 \
		(adxb)->lp_sofst       = 0;                 \
		(adxb)->lp_spos        = 0;                 \
		(adxb)->lp_ins_nsmpl   = 0;                 \
		(adxb)->dec_nch        = (adxb)->nch;       \
		(adxb)->dec_blk_len    = (adxb)->blk_len;   \
		(adxb)->dec_blk_smpl   = (adxb)->blk_smpl;  \
		(adxb)->obuf           = (adxb)->pcm_buf;   \
		(adxb)->obuf_size      = (adxb)->pcm_bsize; \
		(adxb)->obuf_dist      = (adxb)->pcm_bdist; \
		(adxb)->pcm_wpos       = 0;                 \
		(adxb)->total_dec_smpl = 0;                 \
	} while (0)

/* ---- ADXSJD ---- */
#define ADXSJD_MAX_OBJ 16
#define ADXSJD_MAX_NCH 2

#define ADXSJD_STAT_STOP   0
#define ADXSJD_STAT_DECHDR 1
#define ADXSJD_STAT_DECODE 2
#define ADXSJD_STAT_DECEND 3
#define ADXSJD_STAT_ERROR  4

typedef struct _adxsjd_obj {
	signed char used;                                         /* 00 */
	signed char stat;                                         /* 01 */
	signed char maxnch;                                       /* 02 */
	signed char term_flg;                                     /* 03 */
	ADXB adxb;                                                /* 04 */
	SJ sji;                                                   /* 08 */
	SJ sjo[ADXSJD_MAX_NCH];                                   /* 0C */
	SJCK cki;                                                 /* 14 */
	SJCK cko[ADXSJD_MAX_NCH];                                 /* 1C */
	int dec_nsmpl;                                            /* 2C */
	int dec_dtlen;                                            /* 30 */
	int dec_pos;                                              /* 34 */
	int max_dec_smpl;                                         /* 38 */
	int trap_nsmpl;                                           /* 3C */
	int trap_cnt;                                             /* 40 */
	int trap_dtlen;                                           /* 44 */
	void (*trap_func)(void* obj);                             /* 48 */
	void* trap_obj;                                           /* 4C */
	void (*flt_func)(void* obj, int ch, char* data, int len); /* 50 */
	void* flt_obj;                                            /* 54 */
	char hdr[0x40];                                           /* 58 */
	int hdr_len;                                              /* 98 */
} ADXSJD_OBJ;

void ADXSJD_Init(void);
void ADXSJD_Finish(void);
ADXSJD ADXSJD_Create(SJ sji, int maxnch, SJ* sjo);
void ADXSJD_Destroy(ADXSJD sjd);
void ADXSJD_ExecServer(void);
void ADXSJD_ExecHndl(ADXSJD sjd);

/* ---- ADXAMP ---- */
typedef struct _adxamp_obj {
	signed char used;   /* 00 */
	signed char stat;   /* 01 */
	signed char maxnch; /* 02 */
	signed char pad3;   /* 03 */
	SJ sji[2];          /* 04 */
	SJ sjo[2];          /* 0C */
	int amp[2];         /* 14 */
	int unk1C;          /* 1C */
	int sfreq;          /* 20 */
	int unk24;          /* 24 */
	int unk28;          /* 28 */
	int frm_cnt;        /* 2C */
} ADXAMP_OBJ;

/* ---- ADX header decoding ---- */
int ADX_CalcHdrInfoLen(int ainf, int nloop, int ofst, int align);
int ADX_DecodeInfoEnd(void* buf, int len, short* endlen);
int ADX_DecodeInfoExLoop(void* hdr, int len, int* ins_nsmpl, short* nloop, short* lp_type, int* lp_spos, int* lp_sofst, int* lp_epos,
                         int* lp_eofst);
int ADX_DecodeInfoExIdly(void* hdr, int len, short* dly0, short* dly1);
int ADX_DecodeInfoExVer(void* hdr, int len, unsigned char* ver, unsigned char* flag);
int ADX_DecodeInfoExADPCM2(void* hdr, int len, short* cof);
int ADX_DecodeInfo(const void* hdr, int len, short* hdlen, signed char* type, signed char* bps, signed char* blk_len, signed char* nch,
                   int* sfreq, int* nsmpl, int* blk_smpl);
int ADX_DecodeFooter(void* buf, int len, short* ftlen);
void ADX_GetCoefficient(int cof, int sfreq, short* c1, short* c2);
void ADX_SetDecodeSteAsMonoSw(int sw);
extern int adx_decode_ste_as_mono_sw;

/* ---- ADXCRS / ADXERR ---- */
void ADXCRS_Init(void);
void ADXCRS_Lock(void);
void ADXCRS_Unlock(void);

void ADXERR_Init(void);
void ADXERR_Finish(void);
void ADXERR_CallErrFunc1(const char* msg);
void ADXERR_CallErrFunc2(const char* msg1, const char* msg2);
void ADXERR_ItoA2(int n1, int n2, char* str, int len);

/* ---- Others the core calls ---- */
void SVM_Lock(void);
void SVM_Unlock(void);
void SVM_CallErr(const char* fmt, ...);
void SJ_SplitChunk(SJCK* ck, int nbyte, SJCK* ck1, SJCK* ck2);
char* SJRBF_GetBufPtr(SJ sj);
int SJRBF_GetBufSize(SJ sj);
int SJRBF_GetXtrSize(SJ sj);

void ADXSJD_SetInSj(ADXSJD sjd, SJ sj);
void ADXSJD_Start(ADXSJD sjd);
void ADXSJD_Stop(ADXSJD sjd);
int ADXSJD_GetDecNumSmpl(ADXSJD sjd);
int ADXSJD_GetSfreq(ADXSJD sjd);
int ADXSJD_GetNumChan(ADXSJD sjd);
int ADXSJD_GetOutBps(ADXSJD sjd);
int ADXSJD_GetTotalNumSmpl(ADXSJD sjd);
void ADXAMP_Destroy(ADXAMP amp);
void ADXAMP_Start(ADXAMP amp);
void ADXAMP_Stop(ADXAMP amp);
int ADXSJD_GetStat(ADXSJD sjd);
void ADXSJD_SetMaxDecSmpl(ADXSJD sjd, int nsmpl);
void ADXSJD_TermSupply(ADXSJD sjd);
void ADXSJD_SetDecPos(ADXSJD sjd, int pos);
void ADXSJD_EntryTrapFunc(ADXSJD sjd, void (*func)(void*), void* obj);
void ADXSJD_SetTrapNumSmpl(ADXSJD sjd, int nsmpl);
void ADXSJD_SetTrapCnt(ADXSJD sjd, int cnt);
void ADXSJD_SetTrapDtLen(ADXSJD sjd, int dtlen);
int ADXSJD_GetFormat(ADXSJD sjd);
int ADXSJD_GetBlkSmpl(ADXSJD sjd);
int ADXSJD_GetNumLoop(ADXSJD sjd);
int ADXSJD_GetLpStartPos(ADXSJD sjd);
int ADXSJD_GetLpStartOfst(ADXSJD sjd);
int ADXSJD_GetLpEndPos(ADXSJD sjd);
int ADXSJD_GetLpEndOfst(ADXSJD sjd);
char* ADXSJD_GetSpsdInfo(ADXSJD sjd);
void ADXSJD_GetExtPrm(ADXSJD sjd);
void ADXSJD_SetExtPrm(ADXSJD sjd);
void ADXAMP_SetSfreq(ADXAMP amp, int sfreq);

#endif
