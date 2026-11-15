#include "stage/Stage.h"
#include "Ninja/ninja.h"

extern const f32 gUnitVectorMin = 1e-06f;

// 0x8001A16C
// clang-format off
extern "C" asm void vecAdd(Vec3f* out, const Vec3f* a, const Vec3f* b)
{
	nofralloc
	psq_l f0, 0(r4), 0, 0
	psq_l f2, 0(r5), 0, 0
	psq_l f1, 8(r4), 1, 0
	ps_add f0, f0, f2
	psq_l f3, 8(r5), 1, 0
	ps_add f1, f1, f3
	psq_st f0, 0(r3), 0, 0
	psq_st f1, 8(r3), 1, 0
	blr
}
// clang-format on

// 0x8001A190
// clang-format off
extern "C" asm void vecSub(Vec3f* out, const Vec3f* a, const Vec3f* b)
{
	nofralloc
	psq_l f0, 0(r4), 0, 0
	psq_l f2, 0(r5), 0, 0
	psq_l f1, 8(r4), 1, 0
	ps_sub f0, f0, f2
	psq_l f3, 8(r5), 1, 0
	ps_sub f1, f1, f3
	psq_st f0, 0(r3), 0, 0
	psq_st f1, 8(r3), 1, 0
	blr
}
// clang-format on

// The zero vecNormalize writes It has its own pool entry apart from the 0.0f in __sinit
static const f32 sNormZero = 0.0f;

// 0x8001A1B4
// Returns the length or zero and a zero vector if it's too short
// clang-format off
extern "C" asm f32 vecNormalize(Vec3f* out, const Vec3f* in)
{
	nofralloc
	lfs f2, 0(r4)
	lfs f3, 4(r4)
	fmuls f0, f2, f2
	lfs f4, 8(r4)
	fmadds f0, f3, f3, f0
	lfs f5, 1e-06f
	fmadds f1, f4, f4, f0
	fcmpo cr0, f1, f5
	bge ok
	lfs f1, sNormZero
	psq_st f1, 0(r3), 1, 0
	psq_st f1, 4(r3), 1, 0
	psq_st f1, 8(r3), 1, 0
	blr
ok:
	frsqrte f0, f1
	fmuls f2, f0, f2
	stfs f2, 0(r3)
	fmuls f3, f0, f3
	stfs f3, 4(r3)
	fmuls f4, f0, f4
	stfs f4, 8(r3)
	fres f1, f0
	blr
}
// clang-format on

// 0x8001A210
// out = a * scale + b
// clang-format off
extern "C" asm void vecScaleAdd(f32 scale, Vec3f* out, const Vec3f* a, const Vec3f* b)
{
	nofralloc
	psq_l f4, 0(r4), 0, 0
	psq_l f6, 0(r5), 0, 0
	ps_madds0 f4, f4, f1, f6
	psq_l f5, 8(r4), 1, 0
	psq_l f7, 8(r5), 1, 0
	ps_madds0 f5, f5, f1, f7
	psq_st f4, 0(r3), 0, 0
	psq_st f5, 8(r3), 1, 0
	blr
}
// clang-format on

// 0x8001A234
// clang-format off
extern "C" asm void vecScale(f32 scale, Vec3f* out, const Vec3f* in)
{
	nofralloc
	psq_l f4, 0(r4), 0, 0
	ps_muls0 f4, f4, f1
	psq_l f5, 8(r4), 1, 0
	ps_muls0 f5, f5, f1
	psq_st f4, 0(r3), 0, 0
	psq_st f5, 8(r3), 1, 0
	blr
}
// clang-format on

// 0x8001A250
// The distance between two points
static inline Vec3f vecSub(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjSubVector(&r, a, b);
	return r;
}

extern "C" f32 vecDistance(const Vec3f* a, const Vec3f* b)
{
	Vec3f copy = vecSub(a, b);
	return njScalor(&copy);
}

// 0x8001A29C
// The squared distance between two points
// clang-format off
extern "C" asm f32 vecDistanceSq(const Vec3f* a, const Vec3f* b)
{
	nofralloc
	psq_l f4, 0(r3), 0, 0
	psq_l f6, 0(r4), 0, 0
	ps_sub f4, f4, f6
	psq_l f5, 8(r3), 1, 0
	ps_mul f4, f4, f4
	psq_l f7, 8(r4), 1, 0
	ps_sub f5, f5, f7
	ps_madd f0, f5, f5, f4
	ps_sum0 f1, f0, f4, f0
	blr
}
// clang-format on

// 0x8001A2C4
// clang-format off
extern "C" asm void vecNegate(Vec3f* out, const Vec3f* in)
{
	nofralloc
	psq_l f4, 0(r4), 0, 0
	psq_l f5, 8(r4), 1, 0
	ps_neg f4, f4
	ps_neg f5, f5
	psq_st f4, 0(r3), 0, 0
	psq_st f5, 8(r3), 1, 0
	blr
}
// clang-format on

// 0x8001A2E0
// The length of a vector estimated
// clang-format off
extern "C" asm f32 vecLength(const Vec3f* in)
{
	nofralloc
	lfs f0, 0(r3)
	lfs f1, 4(r3)
	fmuls f0, f0, f0
	lfs f2, 8(r3)
	fmadds f0, f1, f1, f0
	fmadds f1, f2, f2, f0
	frsqrte f1, f1
	fres f1, f1
	blr
}
// clang-format on

// 0x8001A304
// The squared length of a vector
// clang-format off
extern "C" asm f32 vecLengthSq(const Vec3f* in)
{
	nofralloc
	lfs f0, 0(r3)
	lfs f1, 4(r3)
	fmuls f0, f0, f0
	lfs f2, 8(r3)
	fmadds f0, f1, f1, f0
	fmadds f1, f2, f2, f0
	blr
}
// clang-format on

// Zero and the unit axes which many files read
struct AxisVec : public Vec3f {
	AxisVec(f32 newX, f32 newY, f32 newZ) { set(newX, newY, newZ); }
};

extern "C" {
AxisVec gVecZero(0.0f, 0.0f, 0.0f); // all zero
AxisVec gAxisX(1.0f, 0.0f, 0.0f);   // the X axis
AxisVec gVecUp(0.0f, 1.0f, 0.0f);   // the Y axis
AxisVec gAxisZ(0.0f, 0.0f, 1.0f);   // the Z axis
}
