#include <CRI/adx_xpnd.h>
#include <CRI/adx.h>

ADXPD_OBJ adxpd_obj[ADXPD_MAX_OBJ];

/* Set when a stereo decode stops after an odd number of frames */
int adxpd_init_fg;

/* The high-pass cutoff and the sampling rate the coefficients start from */
#define ADXPD_DEF_COF   500
#define ADXPD_DEF_SFREQ 44100

// 0x80226678
void ADXPD_Init(void)
{
	memset(adxpd_obj, 0, sizeof(adxpd_obj));
}

// 0x80226520
ADXPD ADXPD_Create(void)
{
	int i;
	ADXPD xpd;

	for (i = 0; i < ADXPD_MAX_OBJ; i++) {
		if (adxpd_obj[i].used == 0) {
			break;
		}
	}
	if (i == ADXPD_MAX_OBJ) {
		return 0;
	}
	xpd = &adxpd_obj[i];
	memset(xpd, 0, sizeof(ADXPD_OBJ));
	xpd->used = 1;
	xpd->idx  = i;
	xpd->mode = 0;
	xpd->stat = ADXPD_STAT_STOP;
	ADX_GetCoefficient(ADXPD_DEF_COF, ADXPD_DEF_SFREQ, &xpd->k0, &xpd->k1);
	memset(xpd->dly, 0, sizeof(xpd->dly));
	return xpd;
}

// 0x802264F0
void ADXPD_SetCoef(ADXPD xpd, int sfreq, int cof)
{
	ADX_GetCoefficient(cof, sfreq, &xpd->k0, &xpd->k1);
}

// 0x802264CC
void ADXPD_SetDly(ADXPD xpd, short* dly1, short* dly2)
{
	xpd->dly[0][0] = dly1[0];
	xpd->dly[0][1] = dly2[0];
	xpd->dly[1][0] = dly1[1];
	xpd->dly[1][1] = dly2[1];
}

// 0x802264A8
void ADXPD_GetDly(ADXPD xpd, short* dly1, short* dly2)
{
	dly1[0] = xpd->dly[0][0];
	dly2[0] = xpd->dly[0][1];
	dly1[1] = xpd->dly[1][0];
	dly2[1] = xpd->dly[1][1];
}

// 0x80226498
void ADXPD_SetExtPrm(ADXPD xpd, short key, short mul, short add)
{
	xpd->key = key;
	xpd->mul = mul;
	xpd->add = add;
}

// 0x8022647C
void ADXPD_GetExtPrm(ADXPD xpd, short* key, short* mul, short* add)
{
	*key = xpd->key;
	*mul = xpd->mul;
	*add = xpd->add;
}

// 0x80226444
void ADXPD_Destroy(ADXPD xpd)
{
	if (xpd != 0) {
		xpd->used = 0;
		memset(xpd, 0, sizeof(ADXPD_OBJ));
	}
}

// 0x8022643C
int ADXPD_GetStat(ADXPD xpd)
{
	return xpd->stat;
}

// 0x80226404
int ADXPD_EntryMono(ADXPD xpd, unsigned char* src, int nblk, short* dst)
{
	if (xpd->stat == ADXPD_STAT_STOP) {
		xpd->ch   = ADXPD_CH_MONO;
		xpd->src  = src;
		xpd->nblk = nblk;
		xpd->dstl = dst;
		xpd->dstr = 0;
		return 1;
	}
	return 0;
}

// 0x802263D0
int ADXPD_EntrySte(ADXPD xpd, unsigned char* src, int nblk, short* dstl, short* dstr)
{
	if (xpd->stat == ADXPD_STAT_STOP) {
		xpd->ch   = ADXPD_CH_STEREO;
		xpd->src  = src;
		xpd->nblk = nblk;
		xpd->dstl = dstl;
		xpd->dstr = dstr;
		return 1;
	}
	return 0;
}

// 0x802263B0
void ADXPD_Start(ADXPD xpd)
{
	if (xpd->stat == ADXPD_STAT_STOP) {
		xpd->num_blk = 0;
		xpd->stat    = ADXPD_STAT_START;
	}
}

// 0x8022637C
void ADXPD_Stop(ADXPD xpd)
{
	xpd->stat = ADXPD_STAT_STOP;
	memset(xpd->dly, 0, sizeof(xpd->dly));
}

// 0x80226364
void ADXPD_Reset(ADXPD xpd)
{
	if (xpd->stat == ADXPD_STAT_END) {
		xpd->stat = ADXPD_STAT_STOP;
	}
}

// 0x8022635C
int ADXPD_GetNumBlk(ADXPD xpd)
{
	return xpd->num_blk;
}

// 0x80226264
void ADXPD_ExecHndl(ADXPD xpd)
{
	if (xpd->stat == ADXPD_STAT_START) {
		xpd->stat = ADXPD_STAT_EXEC;
	}
	if (xpd->stat != ADXPD_STAT_EXEC) {
		return;
	}
	if (xpd->ch == ADXPD_CH_MONO) {
		xpd->num_blk = ADX_DecodeMono4(xpd->src, xpd->nblk, xpd->dstl, xpd->dly[0], xpd->k0, xpd->k1, &xpd->key, xpd->mul, xpd->add);
	} else {
		xpd->num_blk = ADX_DecodeSte4(xpd->src, xpd->nblk, xpd->dstl, xpd->dly[0], xpd->dstr, xpd->dly[1], xpd->k0, xpd->k1, &xpd->key,
		                              xpd->mul, xpd->add);
		if (xpd->num_blk % 2 == 1) {
			adxpd_init_fg = 1;
		}
	}
	xpd->stat = ADXPD_STAT_END;
}
