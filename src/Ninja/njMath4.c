#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

#pragma explicit_zero_data on
f32 njMathZeroOne2[2] = { 0.0f, 1.0f }; // only the 1.0f is read

// 0x8020FCB4
void njRotateZ(f32* m, s32 ang)
{
	NJS_MATRIX r;
	f32* cur;
	if (m == NULL) {
		cur = _gj_matrix_;
	} else {
		cur = m;
	}
	PSMTXRotRad(r, 'z', NJ_ANG_TO_RAD * ang);
	PSMTXConcat(cur, r, cur);
}

// 0x8020FD34
// Copies from src to dst
void njSetMatrix(f32* dst, f32* src)
{
	f32* d;
	f32* s;
	if (dst == NULL) {
		d = _gj_matrix_;
	} else {
		d = dst;
	}
	if (src == NULL) {
		s = _gj_matrix_;
	} else {
		s = src;
	}
	PSMTXCopy(s, d);
}

// 0x8020FD80
asm void njSubVector(NJS_VECTOR* a, const NJS_VECTOR* b)
{
	nofralloc
	psq_l f0, 0x0(r3), 0, 0
	psq_l f2, 0x0(r4), 0, 0
	psq_l f1, 0x8(r3), 1, 0
	ps_sub f0, f0, f2
	psq_l f3, 0x8(r4), 1, 0
	ps_sub f1, f1, f3
	psq_st f0, 0x0(r3), 0, 0
	psq_st f1, 0x8(r3), 1, 0
	blr
}

// 0x8020FDA4
void njXYZAngleToQuaternion(const s32* ang, NJS_QUATERNION* q)
{
	f32 sx = njSin(ang[0] >> 1);
	f32 cx = njCos(ang[0] >> 1);
	f32 sy = njSin(ang[1] >> 1);
	f32 cy = njCos(ang[1] >> 1);
	f32 sz = njSin(ang[2] >> 1);
	f32 cz = njCos(ang[2] >> 1);

	q->re = cz * (cx * cy) + sz * (sx * sy);
	q->im[0] = sz * (-cx * sy) + cz * (sx * cy);
	q->im[1] = sx * (cy * sz) + cx * (sy * cz);
	q->im[2] = sy * (-cz * sx) + cy * (sz * cx);
}

// 0x8020FEEC
void njZXYAngleToQuaternion(const s32* ang, NJS_QUATERNION* q)
{
	f32 sx = njSin(ang[0] >> 1);
	f32 cx = njCos(ang[0] >> 1);
	f32 sy = njSin(ang[1] >> 1);
	f32 cy = njCos(ang[1] >> 1);
	f32 sz = njSin(ang[2] >> 1);
	f32 cz = njCos(ang[2] >> 1);

	q->re = cz * (cx * cy) + sz * (sx * sy);
	q->im[0] = sz * (cx * sy) + cz * (sx * cy);
	q->im[1] = sx * (-cy * sz) + cx * (sy * cz);
	q->im[2] = sy * (-cz * sx) + cy * (sz * cx);
}

// 0x80210034
void njTranslate(f32* m, f32 x, f32 y, f32 z)
{
	if (m == NULL) {
		m = _gj_matrix_;
	}
	__internal_njTranslate(m, x, y, z);
}

// 0x80210060
asm void __internal_njTranslate(f32* m, f32 x, f32 y, f32 z)
{
	nofralloc
	lfs f0, njMathZeroOne2+4
	ps_merge00 f6, f1, f2
	ps_merge00 f7, f3, f0
	psq_l f0, 0x0(r3), 0, 0
	psq_l f1, 0x8(r3), 0, 0
	psq_l f2, 0x10(r3), 0, 0
	ps_mul f8, f0, f6
	psq_l f3, 0x18(r3), 0, 0
	ps_mul f10, f2, f6
	psq_l f4, 0x20(r3), 0, 0
	psq_l f5, 0x28(r3), 0, 0
	ps_mul f12, f4, f6
	ps_madd f8, f1, f7, f8
	ps_madd f10, f3, f7, f10
	ps_madd f12, f5, f7, f12
	ps_sum0 f9, f8, f8, f8
	ps_sum0 f11, f10, f10, f10
	ps_sum0 f13, f12, f12, f12
	psq_st f9, 0xc(r3), 1, 0
	psq_st f11, 0x1c(r3), 1, 0
	psq_st f13, 0x2c(r3), 1, 0
	blr
}
