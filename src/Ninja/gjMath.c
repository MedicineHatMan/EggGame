#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

f32 gjMathConsts[2] = { 0.0f, 1.0f };
void gjMathSinCos(s32 ang);

// 0x80216A34
// clang-format off
asm void gjTranslate(f32 x, f32 y, f32 z)
{
	nofralloc
	lfs f0, gjMathConsts + 4
	lis r4, _gj_matrix_@ha
	ps_merge00 f6, f1, f2
	addi r4, r4, _gj_matrix_@l
	ps_merge00 f7, f3, f0
	lwz r5, 0x0(r4)
	psq_l f0, 0x0(r5), 0, 0
	psq_l f1, 0x8(r5), 0, 0
	psq_l f2, 0x10(r5), 0, 0
	ps_mul f8, f0, f6
	psq_l f3, 0x18(r5), 0, 0
	ps_mul f10, f2, f6
	psq_l f4, 0x20(r5), 0, 0
	psq_l f5, 0x28(r5), 0, 0
	ps_mul f12, f4, f6
	ps_madd f8, f1, f7, f8
	ps_madd f10, f3, f7, f10
	ps_madd f12, f5, f7, f12
	ps_sum0 f9, f8, f8, f8
	ps_sum0 f11, f10, f10, f10
	ps_sum0 f13, f12, f12, f12
	psq_st f9, 0xc(r5), 1, 0
	psq_st f11, 0x1c(r5), 1, 0
	psq_st f13, 0x2c(r5), 1, 0
	blr
}
// clang-format on

// 0x80216A98
// clang-format off
asm void gjMathSinCos(s32 ang)
{
	nofralloc
	lis r5, njSinTable@ha
	addi r5, r5, njSinTable@l
	lwz r5, 0x0(r5)
	clrlslwi r6, r3, 16, 3
	add r6, r5, r6
	lfs f1, 0x0(r6)
	lfs f2, 0x4(r6)
	blr
}
// clang-format on

// 0x80216AB8
// clang-format off
asm void gjRotateX(s32 ang)
{
	nofralloc
	mflr r4
	bl gjMathSinCos
	mtlr r4
	ps_merge00 f9, f1, f2
	fneg f1, f1
	ps_merge00 f8, f2, f1
	lis r4, _gj_matrix_@ha
	addi r4, r4, _gj_matrix_@l
	lwz r4, 0x0(r4)
	psq_l f0, 0x4(r4), 0, 0
	psq_l f1, 0x14(r4), 0, 0
	psq_l f2, 0x24(r4), 0, 0
	ps_muls0 f3, f8, f0
	ps_madds1 f0, f9, f0, f3
	ps_muls0 f3, f8, f1
	ps_madds1 f1, f9, f1, f3
	ps_muls0 f3, f8, f2
	ps_madds1 f2, f9, f2, f3
	psq_st f0, 0x4(r4), 0, 0
	psq_st f1, 0x14(r4), 0, 0
	psq_st f2, 0x24(r4), 0, 0
	blr
}
// clang-format on

// 0x80216B10
// clang-format off
asm void gjRotateY(s32 ang)
{
	nofralloc
	mflr r4
	bl gjMathSinCos
	mtlr r4
	ps_merge00 f8, f2, f1
	fneg f1, f1
	ps_merge00 f9, f1, f2
	lis r4, _gj_matrix_@ha
	addi r4, r4, _gj_matrix_@l
	lwz r4, 0x0(r4)
	lfs f0, 0x0(r4)
	lfs f1, 0x8(r4)
	lfs f2, 0x10(r4)
	lfs f3, 0x18(r4)
	lfs f4, 0x20(r4)
	lfs f5, 0x28(r4)
	ps_muls0 f0, f8, f0
	ps_madds0 f0, f9, f1, f0
	ps_merge10 f1, f0, f0
	stfs f0, 0x0(r4)
	stfs f1, 0x8(r4)
	ps_muls0 f2, f8, f2
	ps_madds0 f2, f9, f3, f2
	ps_merge10 f3, f2, f2
	stfs f2, 0x10(r4)
	stfs f3, 0x18(r4)
	ps_muls0 f4, f8, f4
	ps_madds0 f4, f9, f5, f4
	ps_merge10 f5, f4, f4
	stfs f4, 0x20(r4)
	stfs f5, 0x28(r4)
	blr
}
// clang-format on

// 0x80216B8C
// clang-format off
asm void gjRotateZ(s32 ang)
{
	nofralloc
	mflr r4
	bl gjMathSinCos
	mtlr r4
	ps_merge00 f9, f1, f2
	fneg f1, f1
	ps_merge00 f8, f2, f1
	lis r4, _gj_matrix_@ha
	addi r4, r4, _gj_matrix_@l
	lwz r4, 0x0(r4)
	psq_l f0, 0x0(r4), 0, 0
	psq_l f1, 0x10(r4), 0, 0
	psq_l f2, 0x20(r4), 0, 0
	ps_muls0 f3, f8, f0
	ps_madds1 f0, f9, f0, f3
	ps_muls0 f3, f8, f1
	ps_madds1 f1, f9, f1, f3
	ps_muls0 f3, f8, f2
	ps_madds1 f2, f9, f2, f3
	psq_st f0, 0x0(r4), 0, 0
	psq_st f1, 0x10(r4), 0, 0
	psq_st f2, 0x20(r4), 0, 0
	blr
}
// clang-format on

// 0x80216BE4
// clang-format off
asm void gjAddVector(const NJS_VECTOR* a, const NJS_VECTOR* b, NJS_VECTOR* out)
{
	nofralloc
	psq_l f0, 0x0(r4), 0, 0
	psq_l f2, 0x0(r5), 0, 0
	psq_l f1, 0x8(r4), 1, 0
	ps_add f0, f0, f2
	psq_l f3, 0x8(r5), 1, 0
	ps_add f1, f1, f3
	psq_st f0, 0x0(r3), 0, 0
	psq_st f1, 0x8(r3), 1, 0
	blr
}
// clang-format on

// 0x80216C08
// clang-format off
asm void gjSubVector(const NJS_VECTOR* a, const NJS_VECTOR* b, NJS_VECTOR* out)
{
	nofralloc
	psq_l f0, 0x0(r4), 0, 0
	psq_l f2, 0x0(r5), 0, 0
	psq_l f1, 0x8(r4), 1, 0
	ps_sub f0, f0, f2
	psq_l f3, 0x8(r5), 1, 0
	ps_sub f1, f1, f3
	psq_st f0, 0x0(r3), 0, 0
	psq_st f1, 0x8(r3), 1, 0
	blr
}
// clang-format on

// 0x80216C2C
// clang-format off
asm void gjScaleVector(f32 s, const NJS_VECTOR* a, NJS_VECTOR* out)
{
	nofralloc
	psq_l f4, 0x0(r4), 0, 0
	psq_l f5, 0x8(r4), 1, 0
	ps_muls0 f4, f4, f1
	ps_muls0 f5, f5, f1
	psq_st f4, 0x0(r3), 0, 0
	psq_st f5, 0x8(r3), 1, 0
	blr
}
// clang-format on

// 0x80216C48
// clang-format off
asm void gjMulAddVector(const NJS_VECTOR* a, const NJS_VECTOR* b, f32 s, NJS_VECTOR* out)
{
	nofralloc
	psq_l f4, 0x0(r4), 0, 0
	psq_l f6, 0x0(r5), 0, 0
	ps_madds0 f4, f6, f1, f4
	psq_l f5, 0x8(r4), 1, 0
	psq_l f7, 0x8(r5), 1, 0
	ps_madds0 f5, f7, f1, f5
	psq_st f4, 0x0(r3), 0, 0
	psq_st f5, 0x8(r3), 1, 0
	blr
}
// clang-format on
