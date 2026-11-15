#include <CRI/cri_types.h>
#ifndef CRI_ADX_XPND_H
#define CRI_ADX_XPND_H

#define ADXPD_MAX_OBJ 16

#define ADXPD_STAT_STOP 0
#define ADXPD_STAT_START 1
#define ADXPD_STAT_EXEC 2
#define ADXPD_STAT_END 3

#define ADXPD_CH_MONO 1
#define ADXPD_CH_STEREO 2

typedef struct _adxpd_obj {
	int used;                /* 00 */
	int idx;                 /* 04 */
	int mode;                /* 08 */
	int stat;                /* 0C */
	int num_blk;             /* 10 */
	int ch;                  /* 14 */
	unsigned char* src;      /* 18 */
	int nblk;                /* 1C */
	short* dstl;             /* 20 */
	short* dstr;             /* 24 */
	short dly[2][2];         /* 28 the last two samples of each channel */
	short k0;                /* 30 */
	short k1;                /* 32 */
	short key;               /* 34 */
	short mul;               /* 36 */
	short add;               /* 38 */
	short pad3A;             /* 3A */
} ADXPD_OBJ;

void ADXPD_Init(void);
ADXPD ADXPD_Create(void);
void ADXPD_SetCoef(ADXPD xpd, int sfreq, int cof);
void ADXPD_SetDly(ADXPD xpd, short* dly1, short* dly2);
void ADXPD_GetDly(ADXPD xpd, short* dly1, short* dly2);
void ADXPD_SetExtPrm(ADXPD xpd, short key, short mul, short add);
void ADXPD_GetExtPrm(ADXPD xpd, short* key, short* mul, short* add);
void ADXPD_Destroy(ADXPD xpd);
int ADXPD_GetStat(ADXPD xpd);
int ADXPD_EntryMono(ADXPD xpd, unsigned char* src, int nblk, short* dst);
int ADXPD_EntrySte(ADXPD xpd, unsigned char* src, int nblk, short* dstl, short* dstr);
void ADXPD_Start(ADXPD xpd);
void ADXPD_Stop(ADXPD xpd);
void ADXPD_Reset(ADXPD xpd);
int ADXPD_GetNumBlk(ADXPD xpd);
void ADXPD_ExecHndl(ADXPD xpd);

/* The decoders in adx_dcd5.c Each returns the frames decoded */
int ADX_DecodeMono4(unsigned char* src, int nblk, short* dst, short* dly, short k0, short k1, short* key, short mul, short add);
int ADX_DecodeSte4(unsigned char* src, int nblk, short* dstl, short* dlyl, short* dstr, short* dlyr, short k0, short k1,
                   short* key, short mul, short add);

#endif
