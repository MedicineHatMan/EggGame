#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

#pragma explicit_zero_data on
f32 njMathZeroOne[2] = { 0.0f, 1.0f }; // only the 1.0f is read

// 0x8020F4E0
asm void njAddVector(NJS_VECTOR* a, const NJS_VECTOR* b)
{
	nofralloc
	psq_l f0, 0x0(r3), 0, 0
	psq_l f2, 0x0(r4), 0, 0
	psq_l f1, 0x8(r3), 1, 0
	ps_add f0, f2, f0
	psq_l f3, 0x8(r4), 1, 0
	ps_add f1, f3, f1
	psq_st f0, 0x0(r3), 0, 0
	psq_st f1, 0x8(r3), 1, 0
	blr
}

// 0x8020F504
void njCalcPoint(f32* m, const NJS_POINT3* src, NJS_POINT3* dst)
{
	if (m == NULL) {
		m = _gj_matrix_;
	}
	PSMTXMultVec(m, src, dst);
}

// 0x8020F530
void njCalcPoints(f32* m, const NJS_POINT3* src, NJS_POINT3* dst, s32 count)
{
	if (m == NULL) {
		m = _gj_matrix_;
	}
	PSMTXMultVecArray(m, src, dst, count);
}

// 0x8020F55C
void njGetMatrix(f32* m)
{
	f32* cur = _gj_matrix_;
	if (m == NULL) {
		m = cur;
	}
	PSMTXCopy(cur, m);
}

// 0x8020F590
void njGetTranslation(f32* m, NJS_POINT3* p)
{
	if (m == NULL) {
		m = _gj_matrix_;
	}
	p->x = m[3];
	p->y = m[7];
	p->z = m[11];
}

// 0x8020F5B8
asm f32 njInnerProduct(const NJS_VECTOR* a, const NJS_VECTOR* b)
{
	nofralloc
	lfs f0, 0x0(r3)
	lfs f3, 0x0(r4)
	fmuls f0, f3, f0
	lfs f1, 0x4(r3)
	lfs f4, 0x4(r4)
	fmadds f0, f1, f4, f0
	lfs f2, 0x8(r3)
	lfs f5, 0x8(r4)
	fmadds f1, f2, f5, f0
	blr
}

// 0x8020F5E0
void njInvertMatrix(f32* m)
{
	if (m == NULL) {
		m = _gj_matrix_;
	}
	PSMTXInverse(m, m);
}

// 0x8020F610
void njMultiQuaternion(const NJS_QUATERNION* a, const NJS_QUATERNION* b, NJS_QUATERNION* out)
{
	f32 ar = a->re;
	f32 ax = a->im[0];
	f32 ay = a->im[1];
	f32 az = a->im[2];
	f32 br = b->re;
	f32 bx = b->im[0];
	f32 by = b->im[1];
	f32 bz = b->im[2];

	out->re = ar * br - ax * bx - ay * by - az * bz;
	out->im[0] = ay * bz + (ax * br + ar * bx - az * by);
	out->im[1] = ar * by + (ay * br + az * bx) - ax * bz;
	out->im[2] = ar * bz + (ax * by + (az * br - ay * bx));
}

// 0x8020F6B4
asm f32 njOuterProduct(const NJS_VECTOR* a, const NJS_VECTOR* b, NJS_VECTOR* out)
{
	nofralloc
	lfs f8, 0x4(r4)
	lfs f6, 0x8(r3)
	fmuls f0, f8, f6
	lfs f5, 0x4(r3)
	lfs f9, 0x8(r4)
	fmsubs f0, f9, f5, f0
	lfs f4, 0x0(r3)
	fmuls f1, f9, f4
	lfs f7, 0x0(r4)
	fmsubs f1, f7, f6, f1
	stfs f0, 0x0(r5)
	fmuls f2, f7, f5
	stfs f1, 0x4(r5)
	fmsubs f2, f8, f4, f2
	fmuls f10, f0, f0
	stfs f2, 0x8(r5)
	fmadd f10, f1, f1, f10
	fmadd f10, f2, f2, f10
	frsqrte f1, f10
	fres f1, f1
	blr
}

// 0x8020F708
// Applies Z then X then Y to a point
void njRotateZXY(f32* m, s32 angx, s32 angy, s32 angz)
{
	if (angy != 0) {
		njRotateY(m, angy);
	}
	if (angx != 0) {
		njRotateX(m, angx);
	}
	if (angz != 0) {
		njRotateZ(m, angz);
	}
}

// 0x8020F780
void njScale(f32* m, f32 x, f32 y, f32 z)
{
	NJS_MATRIX s;
	f32* cur;
	if (m == NULL) {
		cur = _gj_matrix_;
	} else {
		cur = m;
	}
	PSMTXScale(s, x, y, z);
	PSMTXConcat(cur, s, cur);
}

// 0x8020F7D0
asm f32 njScalor(const NJS_VECTOR* v)
{
	nofralloc
	lfs f0, 0x0(r3)
	lfs f1, 0x4(r3)
	fmuls f0, f0, f0
	lfs f2, 0x8(r3)
	fmadds f0, f1, f1, f0
	fmadds f1, f2, f2, f0
	frsqrte f1, f1
	fres f1, f1
	blr
}
// 0x8020F7F4
asm void __nj_unit_matrix(f32* m)
{
	nofralloc
	lfs f1, njMathZeroOne+4
	ps_sub f0, f1, f1
	ps_merge01 f1, f1, f0
	psq_st f0, 0x8(r3), 0, 0
	stfs f0, 0x10(r3)
	stfs f0, 0x1c(r3)
	psq_st f0, 0x20(r3), 0, 0
	psq_st f1, 0x0(r3), 0, 0
	psq_st f1, 0x14(r3), 0, 0
	psq_st f1, 0x28(r3), 0, 0
	blr
}

// 0x8020F820
void njUnitMatrix(f32* m)
{
	if (m == NULL) {
		m = _gj_matrix_;
	}
	__nj_unit_matrix(m);
}

// 0x8020F84C
// Clears the rotation and keeps the translation
void njUnitRotPortion(f32* m)
{
	if (m == NULL) {
		m = _gj_matrix_;
	}
	m[0] = 1.0f;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[4] = 0.0f;
	m[5] = 1.0f;
	m[6] = 0.0f;
	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = 1.0f;
}
