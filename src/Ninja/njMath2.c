#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

// 0x8020F888
// clang-format off
asm f32 njScalor2(const NJS_VECTOR* v)
{
	nofralloc
	lfs f0, 0x0(r3)
	lfs f1, 0x4(r3)
	fmuls f0, f0, f0
	lfs f2, 0x8(r3)
	fmadds f0, f1, f1, f0
	fmadds f1, f2, f2, f0
	blr
}
// clang-format on

// 0x8020F8A4
#pragma opt_propagation off
void njProjectScreen(f32* m, const NJS_POINT3* p3, NJS_POINT2* p2)
{
	f32 proj[7];
	f32 vp[6];
	f32 sz;
	f32 sy;
	f32 sx;
	f32* mtx;
	GXGetProjectionv(proj);
	GXGetViewportv(vp);
	mtx = m == NULL ? _gj_matrix_ : m;
	GXProject(p3->x, p3->y, p3->z, mtx, proj, vp, &sx, &sy, &sz);
	p2->x = sx;
	p2->y = sy;
}
#pragma opt_propagation reset

// 0x8020F93C
void njCalcVector(f32* m, const NJS_VECTOR* src, NJS_VECTOR* dst)
{
	if (m == NULL) {
		m = _gj_matrix_;
	}
	PSMTXMultVecSR(m, src, dst);
}

// 0x8020F968
void njCalcVectors(f32* m, const NJS_VECTOR* src, NJS_VECTOR* dst, s32 count)
{
	if (m == NULL) {
		m = _gj_matrix_;
	}
	PSMTXMultVecArraySR(m, src, dst, count);
}

// 0x8020F994
void njCreateQuaternion(s32 ang, f32 x, f32 y, f32 z, NJS_QUATERNION* q)
{
	f32 s;
	f32 c;
	s32 half = ang >> 1;
	s        = njSin(half);
	c        = njCos(half);
	q->re    = c;
	q->im[0] = s * x;
	q->im[1] = s * y;
	q->im[2] = s * z;
}

// 0x8020FA44
void njCreateQuaternionV(const NJS_VECTOR* v, s32 ang, NJS_QUATERNION* q)
{
	f32 s;
	f32 c;
	s32 half = ang >> 1;
	s        = njSin(half);
	c        = njCos(half);
	q->re    = c;
	q->im[0] = s * v->x;
	q->im[1] = s * v->y;
	q->im[2] = s * v->z;
}

// 0x8020FAD0
// Blends along the shorter way round then normalizes
void njInterpolateQuaternion(const NJS_QUATERNION* a, const NJS_QUATERNION* b, NJS_QUATERNION* out, f32 t)
{
	f32 ar = a->re;
	f32 ax = a->im[0];
	f32 ay = a->im[1];
	f32 az = a->im[2];
	f32 br = b->re;
	f32 bx = b->im[0];
	f32 by = b->im[1];
	f32 bz = b->im[2];
	f32 u  = 1.0f - t;
	f32 r;
	f32 x;
	f32 y;
	f32 z;
	f32 inv;

	if (az * bz + (ay * by + (ar * br + ax * bx)) > 0.0f) {
		r = ar * u + br * t;
		x = ax * u + bx * t;
		y = ay * u + by * t;
		z = az * u + bz * t;
	} else {
		r = ar * u - br * t;
		x = ax * u - bx * t;
		y = ay * u - by * t;
		z = az * u - bz * t;
	}
	inv        = 1.0f / sqrtf(z * z + (y * y + (r * r + x * x)));
	out->re    = r * inv;
	out->im[0] = x * inv;
	out->im[1] = y * inv;
	out->im[2] = z * inv;
}
