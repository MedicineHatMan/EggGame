#include <CRI/adx_xpnd.h>
#include <CRI/adx.h>

const int adx_nibble_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, -8, -7, -6, -5, -4, -3, -2, -1 };

#define ADX_FRAME_LEN 18
#define ADX_FRAME_DATA 16

#define ADX_CLIP(x)                       \
	if ((x) > 32767 || (x) < -32768) {    \
		if ((x) < -32768) {               \
			(x) = -32768;                 \
		} else if ((x) > 32767) {         \
			(x) = 32767;                  \
		}                                 \
	}

/* The frame's scale its header masked by the key which then steps on */
#define ADX_SCALE(hdr, key, mul, add, scale)     \
	{                                            \
		short k = *(key);                        \
		*(key) = (add) + k * (mul);              \
		(scale) = (((hdr) ^ k) & 0x1FFF) + 1;    \
		*(key) &= 0x7FFF;                        \
	}

// 0x8022763C
int ADX_DecodeMono4(unsigned char* src, int nblk, short* dst, short* dly, short k0, short k1, short* key, short mul, short add)
{
	int i;
	int j;
	int p1;
	int p2;
	int x;
	short hdr;
	short scale;
	int b;

	int m = mul;
	p1 = dly[0];
	p2 = dly[1];
	for (i = 0; i < nblk; i++) {
		hdr = *(short*)src;
		if (hdr & 0x8000) {
			return i;
		}
		ADX_SCALE(hdr, key, m, add, scale);
		src += 2;
		for (j = 0; j < ADX_FRAME_DATA; j++) {
			b = (signed char)*src++;
			x = (b >> 4) * scale + ((k0 * p1 + k1 * p2) >> 12);
			ADX_CLIP(x);
			dst[0] = x;
			p1 = adx_nibble_tbl[b & 0xF] * scale + ((k0 * x + k1 * p1) >> 12);
			ADX_CLIP(p1);
			dst[1] = p1;
			p2 = x;
			dst += 2;
		}
	}
	dly[0] = p1;
	dly[1] = p2;
	return nblk;
}

/* Stereo mixed down both outputs get left + right * 0.7 */
// 0x8022734C
int ADX_DecodeSte4AsMono(unsigned char* src, int nblk, short* dstl, short* dlyl, short* dstr, short* dlyr, short k0, short k1, short* key,
                short mul, short add)
{
	int i;
	int j;
	int nloop;
	int l1;
	int l2;
	int r1;
	int r2;
	int m;
	short hdr;
	short scl;
	short scr;
	int bl;
	int br;

	nloop = nblk / 2;
	l1 = dlyl[0];
	l2 = dlyl[1];
	r1 = dlyr[0];
	r2 = dlyr[1];
	for (i = 0; i < nloop; i++) {
		hdr = *(short*)src;
		if (hdr & 0x8000) {
			return i * 2;
		}
		ADX_SCALE(hdr, key, mul, add, scl);
		hdr = *(short*)(src + ADX_FRAME_LEN);
		if (hdr & 0x8000) {
			return i * 2;
		}
		ADX_SCALE(hdr, key, mul, add, scr);
		src += 2;
		for (j = 0; j < ADX_FRAME_DATA; j++) {
			bl = (signed char)src[0];
			br = (signed char)src[ADX_FRAME_LEN];
			src++;
			l2 = (bl >> 4) * scl + ((k0 * l1 + k1 * l2) >> 12);
			ADX_CLIP(l2);
			r2 = (br >> 4) * scr + ((k0 * r1 + k1 * r2) >> 12);
			ADX_CLIP(r2);
			m = (l2 + r2) * 7 / 10;
			ADX_CLIP(m);
			dstr[0] = m;
			dstl[0] = m;
			l1 = adx_nibble_tbl[bl & 0xF] * scl + ((k0 * l2 + k1 * l1) >> 12);
			ADX_CLIP(l1);
			r1 = adx_nibble_tbl[br & 0xF] * scr + ((k0 * r2 + k1 * r1) >> 12);
			ADX_CLIP(r1);
			m = (l1 + r1) * 7 / 10;
			ADX_CLIP(m);
			dstr[1] = m;
			dstr += 2;
			dstl[1] = m;
			dstl += 2;
		}
		src += ADX_FRAME_LEN;
	}
	dlyl[0] = l1;
	dlyl[1] = l2;
	dlyr[0] = r1;
	dlyr[1] = r2;
	return nblk;
}

/* Stereo frames come in pairs left then right */
// 0x802270EC
int ADX_DecodeSte4AsSte(unsigned char* src, int nblk, short* dstl, short* dlyl, short* dstr, short* dlyr, short k0, short k1, short* key,
                short mul, short add)
{
	int i;
	int j;
	int nloop;
	int l1;
	int l2;
	int r1;
	int r2;
	short hdr;
	short scl;
	short scr;
	int bl;
	int br;

	nloop = nblk / 2;
	l1 = dlyl[0];
	l2 = dlyl[1];
	r1 = dlyr[0];
	r2 = dlyr[1];
	for (i = 0; i < nloop; i++) {
		hdr = *(short*)src;
		if (hdr & 0x8000) {
			return i * 2;
		}
		ADX_SCALE(hdr, key, mul, add, scl);
		hdr = *(short*)(src + ADX_FRAME_LEN);
		if (hdr & 0x8000) {
			return i * 2;
		}
		ADX_SCALE(hdr, key, mul, add, scr);
		src += 2;
		for (j = 0; j < ADX_FRAME_DATA; j++) {
			bl = (signed char)src[0];
			br = (signed char)src[ADX_FRAME_LEN];
			src++;
			l2 = (bl >> 4) * scl + ((k0 * l1 + k1 * l2) >> 12);
			ADX_CLIP(l2);
			r2 = (br >> 4) * scr + ((k0 * r1 + k1 * r2) >> 12);
			ADX_CLIP(r2);
			dstl[0] = l2;
			dstr[0] = r2;
			l1 = adx_nibble_tbl[bl & 0xF] * scl + ((k0 * l2 + k1 * l1) >> 12);
			ADX_CLIP(l1);
			r1 = adx_nibble_tbl[br & 0xF] * scr + ((k0 * r2 + k1 * r1) >> 12);
			ADX_CLIP(r1);
			dstl[1] = l1;
			dstl += 2;
			dstr[1] = r1;
			dstr += 2;
		}
		src += ADX_FRAME_LEN;
	}
	dlyl[0] = l1;
	dlyl[1] = l2;
	dlyr[0] = r1;
	dlyr[1] = r2;
	return nblk;
}

// 0x80227088
int ADX_DecodeSte4(unsigned char* src, int nblk, short* dstl, short* dlyl, short* dstr, short* dlyr, short k0, short k1,
                   short* key, short mul, short add)
{
	if (adx_decode_ste_as_mono_sw == 0) {
		return ADX_DecodeSte4AsSte(src, nblk, dstl, dlyl, dstr, dlyr, k0, k1, key, mul, add);
	}
	return ADX_DecodeSte4AsMono(src, nblk, dstl, dlyl, dstr, dlyr, k0, k1, key, mul, add);
}
