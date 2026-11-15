#include <CRI/adx.h>
#include <CRI/cri_std.h>
#pragma pool_data off

#define ADX_HDR_MAGIC 0x8000
#define ADX_END_MAGIC 0x8001

static const char adx_copyright[] = "(c)CRI";

/* MSL's inline sqrtf the result goes through a volatile float on the stack */
static inline float sqrtf(float x)
{
	static const double _half = .5;
	static const double _three = 3.0;
	volatile float y;

	if (x > 0.0f) {
		double guess = __frsqrte((double)x);
		guess = _half * guess * (_three - guess * guess * x);
		guess = _half * guess * (_three - guess * guess * x);
		guess = _half * guess * (_three - guess * guess * x);
		y = (float)(x * guess);
		return y;
	}
	return x;
}

// 0x8021F5A8
void ADX_GetCoefficient(int cof, int sfreq, short* c1, short* c2)
{
	float x;
	float a;
	float b;
	float s;
	float c;

	x = (6.2831855f * (float)cof) / (float)sfreq;
	c = (float)cos(x);
	a = sqrtf(2.0f) - c;
	b = sqrtf(2.0f) - 1.0f;
	s = sqrtf((a + b) * (a - b));
	c = (a - s) / b;
	*c1 = (short)(4096.0f * (2.0f * c));
	*c2 = (short)(4096.0f * (-c * c));
}

// 0x8021F52C
int ADX_DecodeFooter(void* buf, int len, short* ftlen)
{
	short* p = (short*)buf;
	int i;
	int pos;

	pos = 0x7FFFFFFF;
	for (i = 0; i < len - 1; i += 2, p++) {
		if (*p == (short)ADX_HDR_MAGIC) {
			pos = (i < 0x7FFFFFFF) ? i : 0x7FFFFFFF;
			break;
		}
	}
	if (pos != 0x7FFFFFFF) {
		*ftlen = pos;
		return 0;
	}
	*ftlen = 0;
	return -1;
}

// 0x8021F43C
int ADX_DecodeInfo(const void* hdr, int len, short* hdlen, signed char* type, signed char* bps, signed char* blk_len,
                   signed char* nch, int* sfreq, int* nsmpl, int* blk_smpl)
{
	unsigned char* p = (unsigned char*)hdr;

	if (len < 0x10) {
		return -1;
	}
	if ((unsigned short)((p[0] << 8) | p[1]) != ADX_HDR_MAGIC) {
		return -2;
	}
	*hdlen = ((p[2] << 8) | p[3]) + 4;
	*type = p[4];
	*blk_len = p[5];
	*bps = p[6];
	*nch = p[7];
	*sfreq = (p[8] << 24) | (p[9] << 16) | (p[10] << 8) | p[11];
	*nsmpl = (p[12] << 24) | (p[13] << 16) | (p[14] << 8) | p[15];
	if (*bps == 0) {
		*blk_smpl = 0;
	} else {
		*blk_smpl = ((*blk_len - 2) * 8) / *bps;
	}
	return 0;
}

// 0x8021F3F4
int ADX_DecodeInfoExADPCM2(void* hdr, int len, short* cof)
{
	short* h = (short*)hdr;

	if (len < 0x12) {
		return -1;
	}
	if (*(unsigned short*)h != ADX_HDR_MAGIC) {
		return -2;
	}
	if (h[1] < 0xE) {
		return -1;
	}
	*cof = ((unsigned short*)h)[8];
	return 0;
}

// 0x8021F3A4
int ADX_DecodeInfoExVer(void* hdr, int len, unsigned char* ver, unsigned char* flag)
{
	short* h = (short*)hdr;

	if (len < 0x14) {
		return -1;
	}
	if (*(unsigned short*)h != ADX_HDR_MAGIC) {
		return -2;
	}
	if (h[1] < 0x10) {
		return -1;
	}
	*ver = ((unsigned char*)hdr)[0x12];
	*flag = ((unsigned char*)hdr)[0x13];
	return 0;
}

// 0x8021F2D4
int ADX_DecodeInfoExIdly(void* hdr, int len, short* dly0, short* dly1)
{
	short* h = (short*)hdr;
	unsigned char ver;
	unsigned char flag;

	if (ADX_DecodeInfoExVer(hdr, len, &ver, &flag) != 0) {
		return -1;
	}
	if (ver >= 4) {
		if (len < 0x20) {
			return -1;
		}
		if (*(unsigned short*)h != ADX_HDR_MAGIC) {
			return -2;
		}
		if (h[1] < 0x1C) {
			return -1;
		}
		dly0[0] = ((unsigned short*)h)[0xC];
		dly1[0] = ((unsigned short*)h)[0xD];
		dly0[1] = ((unsigned short*)h)[0xE];
		dly1[1] = ((unsigned short*)h)[0xF];
	} else {
		dly1[1] = 0;
		dly0[1] = 0;
		dly1[0] = 0;
		dly0[0] = 0;
	}
	return 0;
}

// 0x8021F1B8
int ADX_DecodeInfoExLoop(void* hdr, int len, int* ins_nsmpl, short* nloop, short* lp_type, int* lp_spos, int* lp_sofst,
                         int* lp_epos, int* lp_eofst)
{
	short* h = (short*)hdr;
	unsigned char ver;
	unsigned char flag;
	int ret;
	int need;
	int ofs;

	*nloop = 0;
	ret = ADX_DecodeInfoExVer(hdr, len, &ver, &flag);
	if (ret != 0) {
		return ret;
	}
	need = 0x30;
	if (ver == 4) {
		need = 0x3C;
	}
	if (len < need) {
		return -1;
	}
	if (*(unsigned short*)h != ADX_HDR_MAGIC) {
		return -2;
	}
	if (h[1] < need - 4) {
		return -1;
	}
	ofs = 0x14;
	if (ver == 4) {
		ofs = 0x20;
	}
	*ins_nsmpl = *(short*)((char*)hdr + ofs);
	*nloop = *(short*)(&((char*)hdr)[ofs + 2]);
	if (*nloop != 1) {
		return -2;
	}
	*lp_type = *(short*)(&((char*)hdr)[ofs + 6]);
	*lp_spos = *(int*)(&((char*)hdr)[ofs + 8]);
	*lp_sofst = *(int*)(&((char*)hdr)[ofs + 0xC]);
	*lp_epos = *(int*)(&((char*)hdr)[ofs + 0x10]);
	*lp_eofst = *(int*)(&((char*)hdr)[ofs + 0x14]);
	return 0;
}

// 0x8021F180
int ADX_DecodeInfoEnd(void* buf, int len, short* endlen)
{
	short* h = (short*)buf;

	if (len < 0x10) {
		return -1;
	}
	if (*(unsigned short*)h != ADX_END_MAGIC) {
		return -2;
	}
	*endlen = h[1] + 4;
	return 0;
}

// 0x8021F0E4
int ADX_CalcHdrInfoLen(int ainf, int nloop, int ofst, int align)
{
	if (ainf == 0) {
		return align * ((nloop + 0x1B + strlen(adx_copyright) + ofst + align) / align) - ofst;
	}
	return align * ((nloop + 0x33 + strlen(adx_copyright) + ofst + align) / align) - ofst;
}
