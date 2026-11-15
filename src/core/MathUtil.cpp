#include "core/types.h"
#include "stage/Stage.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "core/SdkCalls.h"

extern const f32 gUnitVectorMin;

// Named constants come ahead of the pooled literals so this 0.0 is separate from the one the code below uses
// Nothing in the DOL reads it
extern "C" const f32 mathZero = 0.0f;
extern "C" const f32 mathOne  = 1.0f;

static inline f32 mathSqrt(f32 x)
{
	if (x > 0.0f) {
		f64 guess         = __frsqrte(x);
		guess             = 0.5 * guess * (3.0 - guess * guess * x);
		guess             = 0.5 * guess * (3.0 - guess * guess * x);
		guess             = 0.5 * guess * (3.0 - guess * guess * x);
		volatile f32 root = (f32)(x * guess);
		return root;
	}
	return x;
}

// 0x80061740
// The angle on the ground plane from one place to another
extern "C" int angleToXZ(const Vec3f* from, const Vec3f* to)
{
	return (int)(10430.38043493439 * (f32)atan2(to->x - from->x, to->z - from->z));
}

// 0x80061790
extern "C" int vecAngleXZ(const Vec3f* v)
{
	return (int)(10430.38043493439 * (f32)atan2(v->x, v->z));
}

// 0x800617D0
extern "C" f32 distXZ(const Vec3f* a, const Vec3f* b)
{
	f32 dz = a->z - b->z;
	f32 dx = a->x - b->x;
	return mathSqrt(dx * dx + dz * dz);
}

// 0x80061854
extern "C" f32 vecLenXZ(const Vec3f* v)
{
	return mathSqrt(v->x * v->x + v->z * v->z);
}

// 0x800618CC
// A ground plane vector of a length pointing along an angle
extern "C" void vecFromAngleXZ(f32 length, Vec3f* out, int angle)
{
	f32 s, c;
	njSinCos(angle, &s, &c);
	out->x = s * length;
	out->y = 0.0f;
	out->z = c * length;
}

// 0x8006193C
// A vector shorter than 0.000001 comes out as zero
extern "C" f32 vecNormalizeXZ(const Vec3f* v, Vec3f* out)
{
	f32 length = mathSqrt(v->x * v->x + v->z * v->z);
	if (length < gUnitVectorMin) {
		out->x = 0.0f;
		out->y = 0.0f;
		out->z = 0.0f;
		return 0.0f;
	}
	f32 scale = 1.0f / length;
	out->x    = v->x * scale;
	out->y    = 0.0f;
	out->z    = v->z * scale;
	return length;
}

// 0x800619FC
extern "C" f32 vecDotXZ(const Vec3f* a, const Vec3f* b)
{
	return a->x * b->x + a->z * b->z;
}

// 0x80061A18
extern "C" f32 vecCrossXZ(const Vec3f* a, const Vec3f* b)
{
	return a->z * b->x - a->x * b->z;
}

// 0x80061A34
// out = v - 2 * dot(v, n) * n a bounce off a surface with the normal n
extern "C" void vecReflect(const Vec3f* v, const Vec3f* n, Vec3f* out)
{
	f32 d = njInnerProduct(v, n);
	d *= -2.0f;
	gjMulAddVector(d, out, v, n);
}

// 0x80061A80
extern "C" void vecLerp(f32 t, const Vec3f* a, const Vec3f* b, Vec3f* out)
{
	Vec3f at;
	at.x = a->x;
	at.y = a->y;
	at.z = a->z;
	gjScaleVector(1.0f - t, &at, &at);
	gjMulAddVector(t, &at, &at, b);
	out->x = at.x;
	out->y = at.y;
	out->z = at.z;
}

// 0x80061B18
// Turns a vector about the Y axis
// clang-format off
extern "C" asm void vecRotateY(const Vec3f* in, Vec3f* out, int angle)
{
	nofralloc
	lis r6, njSinTable@ha
	addi r6, r6, njSinTable@l
	lwz r6, 0x0(r6)
	clrlslwi r5, r5, 16, 3
	psq_l f0, 0x0(r3), 0, 0
	add r5, r6, r5
	lfs f10, 0x0(r5)
	lfs f11, 0x4(r5)
	ps_merge00 f9, f11, f10
	fneg f10, f10
	ps_merge00 f8, f11, f10
	psq_l f1, 0x8(r3), 1, 0
	ps_muls0 f10, f8, f0
	ps_muls0 f11, f9, f1
	ps_sum0 f0, f10, f0, f11
	ps_sum0 f1, f11, f1, f10
	psq_st f0, 0x0(r4), 0, 0
	psq_st f1, 0x8(r4), 1, 0
	blr
}
// clang-format on

// 0x80061B64
extern "C" void mtxConcat(f32* dst, const f32* a, const f32* b)
{
	PSMTXConcat(a, b, dst);
}

// 0x80061B98
extern "C" void mtxGetPos(const f32* m, Vec3f* out)
{
	out->x = m[3];
	out->y = m[7];
	out->z = m[11];
}

// 0x80061BB4
// The quaternion comes out with w first
extern "C" void mtxToQuat(const f32* m, f32* out)
{
	f32 q[4];
	C_QUATMtx(q, m);
	out[0] = q[3];
	out[1] = q[0];
	out[2] = q[1];
	out[3] = q[2];
}

// 0x80061C08
extern "C" void mtxMultVec(const f32* m, const Vec3f* in, Vec3f* out)
{
	PSMTXMultVec(m, (const NJS_VECTOR*)in, (NJS_VECTOR*)out);
}

// 0x80061C28
// Rotate and scale only leaving out the matrix's place
extern "C" void mtxMultVecSR(const f32* m, const Vec3f* in, Vec3f* out)
{
	PSMTXMultVecSR(m, (const NJS_VECTOR*)in, (NJS_VECTOR*)out);
}

// 0x80061C48
// clang-format off
extern "C" asm void mtxTranslateV(f32* dst, const f32* src, const Vec3f* v)
{
	nofralloc
	psq_l f6, 0x0(r5), 0, 0
	psq_l f7, 0x8(r5), 1, 0
	psq_l f0, 0x0(r4), 0, 0
	psq_l f1, 0x8(r4), 0, 0
	psq_l f2, 0x10(r4), 0, 0
	psq_l f3, 0x18(r4), 0, 0
	psq_l f4, 0x20(r4), 0, 0
	psq_l f5, 0x28(r4), 0, 0
	ps_mul f8, f0, f6
	ps_mul f10, f2, f6
	ps_mul f12, f4, f6
	ps_madd f8, f1, f7, f8
	ps_madd f10, f3, f7, f10
	ps_madd f12, f5, f7, f12
	ps_sum1 f1, f8, f1, f8
	ps_sum1 f3, f10, f3, f10
	ps_sum1 f5, f12, f5, f12
	psq_st f0, 0x0(r3), 0, 0
	psq_st f1, 0x8(r3), 0, 0
	psq_st f2, 0x10(r3), 0, 0
	psq_st f3, 0x18(r3), 0, 0
	psq_st f4, 0x20(r3), 0, 0
	psq_st f5, 0x28(r3), 0, 0
	blr
}
// clang-format on

// 0x80061CA8
// clang-format off
extern "C" asm void mtxTranslate(f32* dst, const f32* src, f32 x, f32 y, f32 z)
{
	nofralloc
	lfs f0, mathOne
	ps_merge00 f6, f1, f2
	ps_merge00 f7, f3, f0
	psq_l f0, 0x0(r4), 0, 0
	psq_l f1, 0x8(r4), 0, 0
	psq_l f2, 0x10(r4), 0, 0
	psq_l f3, 0x18(r4), 0, 0
	psq_l f4, 0x20(r4), 0, 0
	psq_l f5, 0x28(r4), 0, 0
	ps_mul f8, f0, f6
	ps_mul f10, f2, f6
	ps_mul f12, f4, f6
	ps_madd f8, f1, f7, f8
	ps_madd f10, f3, f7, f10
	ps_madd f12, f5, f7, f12
	ps_sum1 f1, f8, f1, f8
	ps_sum1 f3, f10, f3, f10
	ps_sum1 f5, f12, f5, f12
	psq_st f0, 0x0(r3), 0, 0
	psq_st f1, 0x8(r3), 0, 0
	psq_st f2, 0x10(r3), 0, 0
	psq_st f3, 0x18(r3), 0, 0
	psq_st f4, 0x20(r3), 0, 0
	psq_st f5, 0x28(r3), 0, 0
	blr
}
// clang-format on

// 0x80061D0C
// Takes the angle in r5 and leaves its sine in f1 and its cosine in f2
// clang-format off
extern "C" asm void mtxLoadSinCos()
{
	nofralloc
	lis r8, njSinTable@ha
	addi r8, r8, njSinTable@l
	lwz r8, 0x0(r8)
	clrlslwi r9, r5, 16, 3
	add r9, r8, r9
	lfs f1, 0x0(r9)
	lfs f2, 0x4(r9)
	blr
}
// clang-format on

// 0x80061D2C
// clang-format off
extern "C" asm void mtxRotateX(f32* dst, const f32* src, int angle)
{
	nofralloc
	mflr r7
	bl mtxLoadSinCos
	mtlr r7
	ps_merge00 f9, f1, f2
	fneg f1, f1
	ps_merge00 f8, f2, f1
	psq_l f0, 0x0(r4), 0, 0
	psq_l f1, 0x8(r4), 0, 0
	psq_l f2, 0x10(r4), 0, 0
	psq_l f3, 0x18(r4), 0, 0
	psq_l f4, 0x20(r4), 0, 0
	psq_l f5, 0x28(r4), 0, 0
	ps_muls1 f6, f8, f0
	ps_madds0 f6, f9, f1, f6
	ps_merge00 f0, f0, f6
	ps_merge11 f1, f6, f1
	ps_muls1 f6, f8, f2
	ps_madds0 f6, f9, f3, f6
	ps_merge00 f2, f2, f6
	ps_merge11 f3, f6, f3
	ps_muls1 f6, f8, f4
	ps_madds0 f6, f9, f5, f6
	ps_merge00 f4, f4, f6
	ps_merge11 f5, f6, f5
	psq_st f0, 0x0(r3), 0, 0
	psq_st f1, 0x8(r3), 0, 0
	psq_st f2, 0x10(r3), 0, 0
	psq_st f3, 0x18(r3), 0, 0
	psq_st f4, 0x20(r3), 0, 0
	psq_st f5, 0x28(r3), 0, 0
	blr
}
// clang-format on

// 0x80061DA8
// clang-format off
extern "C" asm void mtxRotateY(f32* dst, const f32* src, int angle)
{
	nofralloc
	mflr r7
	bl mtxLoadSinCos
	mtlr r7
	ps_merge00 f8, f2, f1
	fneg f1, f1
	ps_merge00 f9, f1, f2
	psq_l f0, 0x0(r4), 0, 0
	psq_l f1, 0x8(r4), 0, 0
	psq_l f2, 0x10(r4), 0, 0
	psq_l f3, 0x18(r4), 0, 0
	psq_l f4, 0x20(r4), 0, 0
	psq_l f5, 0x28(r4), 0, 0
	ps_muls0 f6, f8, f0
	ps_madds0 f6, f9, f1, f6
	ps_merge01 f0, f6, f0
	ps_merge11 f1, f6, f1
	ps_muls0 f6, f8, f2
	ps_madds0 f6, f9, f3, f6
	ps_merge01 f2, f6, f2
	ps_merge11 f3, f6, f3
	ps_muls0 f6, f8, f4
	ps_madds0 f6, f9, f5, f6
	ps_merge01 f4, f6, f4
	ps_merge11 f5, f6, f5
	psq_st f0, 0x0(r3), 0, 0
	psq_st f1, 0x8(r3), 0, 0
	psq_st f2, 0x10(r3), 0, 0
	psq_st f3, 0x18(r3), 0, 0
	psq_st f4, 0x20(r3), 0, 0
	psq_st f5, 0x28(r3), 0, 0
	blr
}
// clang-format on

// 0x80061E24
// clang-format off
extern "C" asm void mtxRotateZ(f32* dst, const f32* src, int angle)
{
	nofralloc
	mflr r7
	bl mtxLoadSinCos
	mtlr r7
	ps_merge00 f9, f1, f2
	fneg f1, f1
	ps_merge00 f8, f2, f1
	psq_l f0, 0x0(r4), 0, 0
	psq_l f1, 0x8(r4), 0, 0
	psq_l f2, 0x10(r4), 0, 0
	psq_l f3, 0x18(r4), 0, 0
	psq_l f4, 0x20(r4), 0, 0
	psq_l f5, 0x28(r4), 0, 0
	ps_muls0 f6, f8, f0
	ps_madds1 f0, f9, f0, f6
	ps_muls0 f6, f8, f2
	ps_madds1 f2, f9, f2, f6
	ps_muls0 f6, f8, f4
	ps_madds1 f4, f9, f4, f6
	psq_st f0, 0x0(r3), 0, 0
	psq_st f1, 0x8(r3), 0, 0
	psq_st f2, 0x10(r3), 0, 0
	psq_st f3, 0x18(r3), 0, 0
	psq_st f4, 0x20(r3), 0, 0
	psq_st f5, 0x28(r3), 0, 0
	blr
}
// clang-format on

// 0x80061E88
// angles holds x y z
extern "C" void mtxRotateZYX(f32* dst, const f32* src, const int* angles)
{
	mtxRotateZ(dst, src, angles[2]);
	mtxRotateY(dst, dst, angles[1]);
	mtxRotateX(dst, dst, angles[0]);
}

// 0x80061EDC
extern "C" void mtxConcatQuat(f32* dst, const f32* src, const f32* q)
{
	f32 rot[12];
	mtxFromQuat(rot, q);
	mtxConcat(dst, src, rot);
}

// 0x80061F24
// clang-format off
extern "C" asm void mtxScaleV(f32* dst, const f32* src, const Vec3f* v)
{
	nofralloc
	psq_l f6, 0x0(r5), 0, 0
	psq_l f7, 0x8(r5), 1, 0
	psq_l f0, 0x0(r4), 0, 0
	psq_l f1, 0x8(r4), 0, 0
	ps_mul f0, f0, f6
	psq_l f2, 0x10(r4), 0, 0
	ps_mul f1, f1, f7
	psq_l f3, 0x18(r4), 0, 0
	ps_mul f2, f2, f6
	psq_l f4, 0x20(r4), 0, 0
	ps_mul f3, f3, f7
	psq_l f5, 0x28(r4), 0, 0
	ps_mul f4, f4, f6
	psq_st f0, 0x0(r3), 0, 0
	ps_mul f5, f5, f7
	psq_st f1, 0x8(r3), 0, 0
	psq_st f2, 0x10(r3), 0, 0
	psq_st f3, 0x18(r3), 0, 0
	psq_st f4, 0x20(r3), 0, 0
	psq_st f5, 0x28(r3), 0, 0
	blr
}
// clang-format on

// 0x80061F78
// clang-format off
extern "C" asm void mtxScale(f32 x, f32 y, f32 z, f32* dst, const f32* src)
{
	nofralloc
	lfs f0, mathOne
	ps_merge00 f6, f1, f2
	ps_merge00 f7, f3, f0
	psq_l f0, 0x0(r4), 0, 0
	psq_l f1, 0x8(r4), 0, 0
	ps_mul f0, f0, f6
	psq_l f2, 0x10(r4), 0, 0
	ps_mul f1, f1, f7
	psq_l f3, 0x18(r4), 0, 0
	ps_mul f2, f2, f6
	psq_l f4, 0x20(r4), 0, 0
	ps_mul f3, f3, f7
	psq_l f5, 0x28(r4), 0, 0
	ps_mul f4, f4, f6
	psq_st f0, 0x0(r3), 0, 0
	ps_mul f5, f5, f7
	psq_st f1, 0x8(r3), 0, 0
	psq_st f2, 0x10(r3), 0, 0
	psq_st f3, 0x18(r3), 0, 0
	psq_st f4, 0x20(r3), 0, 0
	psq_st f5, 0x28(r3), 0, 0
	blr
}
// clang-format on

// 0x80061FD0
extern "C" int mtxInverse(f32* dst, const f32* src)
{
	return PSMTXInverse(src, dst);
}

// 0x80061FFC
extern "C" void mtxTranspose(f32* dst, const f32* src)
{
	PSMTXTranspose(src, dst);
}

// 0x80062028
extern "C" void mtxCopy(f32* dst, const f32* src)
{
	PSMTXCopy(src, dst);
}

// 0x80062054
extern "C" f32 lerpF(f32 a, f32 b, f32 t)
{
	return t * (b - a) + a;
}

// 0x80062060
extern "C" void mtxIdentity(f32* m)
{
	PSMTXIdentity(m);
}

// 0x80062080
extern "C" void mtxSetTransV(f32* m, const Vec3f* v)
{
	PSMTXTrans(m, v->x, v->y, v->z);
}

// 0x800620AC
extern "C" void mtxSetTrans(f32* m, f32 x, f32 y, f32 z)
{
	PSMTXTrans(m, x, y, z);
}

// 0x800620CC
extern "C" void mtxSetRotX(f32* m, int angle)
{
	f32 s, c;
	njSinCos(angle, &s, &c);
	PSMTXRotTrig(m, 'x', s, c);
}

// 0x80062118
extern "C" void mtxSetRotY(f32* m, int angle)
{
	f32 s, c;
	njSinCos(angle, &s, &c);
	PSMTXRotTrig(m, 'y', s, c);
}

// 0x80062164
// The quaternion goes in with w first
extern "C" void mtxFromQuat(f32* m, const f32* q)
{
	f32 sdk[4];
	sdk[3] = q[0];
	sdk[0] = q[1];
	sdk[1] = q[2];
	sdk[2] = q[3];
	PSMTXQuat(m, (const GXQuaternion*)sdk);
}

// 0x800621A8
extern "C" void mtxSetScaleV(f32* m, const Vec3f* v)
{
	PSMTXScale(m, v->x, v->y, v->z);
}

// 0x800621D4
extern "C" void mtxSetScale(f32* m, f32 x, f32 y, f32 z)
{
	PSMTXScale(m, x, y, z);
}
