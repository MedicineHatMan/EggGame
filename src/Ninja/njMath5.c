#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

#pragma explicit_zero_data on
f32 njMathZero = 0.0f;

// 0x802100B8
void njRotateX(f32* m, s32 ang)
{
	NJS_MATRIX r;
	f32* cur;
	if (m == NULL) {
		cur = _gj_matrix_;
	} else {
		cur = m;
	}
	PSMTXRotRad(r, 'x', NJ_ANG_TO_RAD * ang);
	PSMTXConcat(cur, r, cur);
}

// 0x80210138
void njMultiMatrix(f32* m, f32* n)
{
	if (m == NULL) {
		m = _gj_matrix_;
	}
	if (n == NULL) {
		n = _gj_matrix_;
	}
	PSMTXConcat(m, n, m);
}

// 0x80210174
// clang-format off
asm f32 njUnitVector(NJS_VECTOR* v)
{
	nofralloc
	lfs f2, 0x0(r3)
	lfs f3, 0x4(r3)
	fmuls f0, f2, f2
	lfs f4, 0x8(r3)
	fmadds f0, f3, f3, f0
	fmadds f1, f4, f4, f0
	frsqrte f0, f1
	fmuls f2, f0, f2
	stfs f2, 0x0(r3)
	fmuls f3, f0, f3
	stfs f3, 0x4(r3)
	fmuls f4, f0, f4
	stfs f4, 0x8(r3)
	fres f1, f0
	blr
} // clang-format on

// 0x802101B0
// Fails if the stack is full
s32 njPushMatrix(f32* m)
{
	f32* cur  = _gj_matrix_;
	f32* next = cur + 12;
	if (next > gjMatrixStackEnd) {
		return 0;
	}
	syMemCopy(next, m == NULL ? cur : m, sizeof(NJS_MATRIX));
	_gj_matrix_ = next;
	return 1;
}

// 0x80210218
// clang-format off
asm void njCalcPointEx(const NJS_POINT3* src, NJS_POINT3* dst)
{
	nofralloc
	lis r5, _gj_matrix_@ha
	psq_l f6, 0x0(r3), 0, 0
	psq_l f7, 0x8(r3), 1, 0
	addi r5, r5, _gj_matrix_@l
	lwz r5, 0x0(r5)
	psq_l f0, 0x0(r5), 0, 0
	psq_l f1, 0x8(r5), 0, 0
	ps_mul f8, f0, f6
	psq_l f2, 0x10(r5), 0, 0
	ps_madd f8, f1, f7, f8
	psq_l f3, 0x18(r5), 0, 0
	ps_sum0 f8, f8, f8, f8
	psq_l f4, 0x20(r5), 0, 0
	ps_mul f9, f2, f6
	psq_l f5, 0x28(r5), 0, 0
	ps_madd f9, f3, f7, f9
	psq_st f8, 0x0(r4), 1, 0
	ps_sum0 f9, f9, f9, f9
	ps_mul f10, f4, f6
	psq_st f9, 0x4(r4), 1, 0
	ps_madd f10, f5, f7, f10
	ps_sum0 f10, f10, f10, f10
	psq_st f10, 0x8(r4), 1, 0
	blr
}
// clang-format on

// 0x80210278
void njQuaternionEx(const NJS_QUATERNION* q)
{
	NJS_MATRIX m;
	GXQuaternion gx;
	gx.w = q->re;
	gx.x = q->im[0];
	gx.y = q->im[1];
	gx.z = q->im[2];
	PSMTXQuat(m, &gx);
	PSMTXConcat(_gj_matrix_, m, _gj_matrix_);
}

// 0x802102D0
void njQuaternionEx2(f32 w, const NJS_VECTOR* v)
{
	NJS_MATRIX m;
	GXQuaternion gx;
	gx.w = w;
	gx.x = v->x;
	gx.y = v->y;
	gx.z = v->z;
	PSMTXQuat(m, &gx);
	PSMTXConcat(_gj_matrix_, m, _gj_matrix_);
}

// 0x80210324
// clang-format off
asm void njCalcVectorEx(const NJS_VECTOR* src, NJS_VECTOR* dst)
{
	nofralloc
	lis r5, _gj_matrix_@ha
	psq_l f6, 0x0(r3), 0, 0
	psq_l f7, 0x8(r3), 1, 0
	addi r5, r5, _gj_matrix_@l
	lwz r5, 0x0(r5)
	lfs f8, njMathZero
	ps_merge00 f7, f7, f8
	psq_l f0, 0x0(r5), 0, 0
	psq_l f1, 0x8(r5), 0, 0
	ps_mul f8, f0, f6
	psq_l f2, 0x10(r5), 0, 0
	ps_madd f8, f1, f7, f8
	psq_l f3, 0x18(r5), 0, 0
	ps_sum0 f8, f8, f8, f8
	psq_l f4, 0x20(r5), 0, 0
	ps_mul f9, f2, f6
	psq_l f5, 0x28(r5), 0, 0
	ps_madd f9, f3, f7, f9
	psq_st f8, 0x0(r4), 1, 0
	ps_sum0 f9, f9, f9, f9
	ps_mul f10, f4, f6
	psq_st f9, 0x4(r4), 1, 0
	ps_madd f10, f5, f7, f10
	ps_sum0 f10, f10, f10, f10
	psq_st f10, 0x8(r4), 1, 0
	blr
}
// clang-format on
// 0x8021038C
// clang-format off
asm void njRotateEx(const s32* ang, s32 order)
{
	nofralloc
	lis r6, njSinTable@ha
	addi r6, r6, njSinTable@l
	lwz r6, 0x0(r6)
	lis r5, _gj_matrix_@ha
	addi r5, r5, _gj_matrix_@l
	lwz r5, 0x0(r5)
	psq_l f0, 0x0(r5), 0, 0
	psq_l f1, 0x8(r5), 1, 0
	psq_l f2, 0x10(r5), 0, 0
	psq_l f3, 0x18(r5), 1, 0
	psq_l f4, 0x20(r5), 0, 0
	psq_l f5, 0x28(r5), 1, 0
	cmpwi r4, 0x0
	bne L_80210408
L_802103C4:
	lwz r7, 0x8(r3)
	clrlslwi r7, r7, 16, 3
	cmpwi cr1, r7, 0x0
	beq cr1, L_80210404
	add r7, r6, r7
	lfs f10, 0x0(r7)
	lfs f11, 0x4(r7)
	ps_merge00 f9, f10, f11
	fneg f10, f10
	ps_merge00 f8, f11, f10
	ps_muls0 f10, f8, f0
	ps_madds1 f0, f9, f0, f10
	ps_muls0 f11, f8, f2
	ps_madds1 f2, f9, f2, f11
	ps_muls0 f12, f8, f4
	ps_madds1 f4, f9, f4, f12
L_80210404:
	bne L_802104A4
L_80210408:
	lwz r7, 0x4(r3)
	clrlslwi r7, r7, 16, 3
	cmpwi cr1, r7, 0x0
	beq cr1, L_80210454
	add r7, r6, r7
	lfs f10, 0x0(r7)
	lfs f11, 0x4(r7)
	ps_merge00 f8, f10, f11
	fneg f10, f10
	ps_merge00 f9, f11, f10
	ps_muls0 f10, f8, f0
	ps_madds0 f1, f9, f1, f10
	ps_merge11 f0, f1, f0
	ps_muls0 f11, f8, f2
	ps_madds0 f3, f9, f3, f11
	ps_merge11 f2, f3, f2
	ps_muls0 f12, f8, f4
	ps_madds0 f5, f9, f5, f12
	ps_merge11 f4, f5, f4
L_80210454:
	lwz r7, 0x0(r3)
	clrlslwi r7, r7, 16, 3
	cmpwi cr1, r7, 0x0
	beq cr1, L_802104A0
	add r7, r6, r7
	lfs f10, 0x0(r7)
	lfs f11, 0x4(r7)
	ps_merge00 f9, f11, f10
	fneg f10, f10
	ps_merge00 f8, f10, f11
	ps_muls1 f10, f8, f0
	ps_madds0 f1, f9, f1, f10
	ps_merge01 f0, f0, f1
	ps_muls1 f11, f8, f2
	ps_madds0 f3, f9, f3, f11
	ps_merge01 f2, f2, f3
	ps_muls1 f12, f8, f4
	ps_madds0 f5, f9, f5, f12
	ps_merge01 f4, f4, f5
L_802104A0:
	bne L_802103C4
L_802104A4:
	psq_st f0, 0x0(r5), 0, 0
	psq_st f1, 0x8(r5), 1, 0
	psq_st f2, 0x10(r5), 0, 0
	psq_st f3, 0x18(r5), 1, 0
	psq_st f4, 0x20(r5), 0, 0
	psq_st f5, 0x28(r5), 1, 0
	blr
}
// clang-format on
// 0x802104C0
// clang-format off
asm s32 njPushMatrixEx(void)
{
	nofralloc
	lis r4, _gj_matrix_@ha
	addi r4, r4, _gj_matrix_@l
	lwz r3, 0x0(r4)
	addi r5, r3, 0x30
	psq_l f0, 0x0(r3), 0, 0
	psq_lu f1, 0x8(r3), 0, 0
	stw r5, 0x0(r4)
	psq_lu f2, 0x8(r3), 0, 0
	psq_st f0, 0x0(r5), 0, 0
	psq_lu f3, 0x8(r3), 0, 0
	psq_stu f1, 0x8(r5), 0, 0
	psq_lu f4, 0x8(r3), 0, 0
	psq_stu f2, 0x8(r5), 0, 0
	psq_lu f5, 0x8(r3), 0, 0
	psq_stu f3, 0x8(r5), 0, 0
	psq_stu f4, 0x8(r5), 0, 0
	li r3, 0x1
	psq_stu f5, 0x8(r5), 0, 0
	blr
}
// clang-format on

// 0x8021050C
s32 njPopMatrixEx(void)
{
	f32* p = _gj_matrix_ - 12;
	if (p < gjMatrixStack) {
		return 0;
	}
	_gj_matrix_ = p;
	return 1;
}

// 0x80210534
// clang-format off
asm void njScaleEx(const NJS_VECTOR* v)
{
	nofralloc
	psq_l f0, 0x0(r3), 0, 0
	lis r4, _gj_matrix_@ha
	psq_l f1, 0x8(r3), 1, 0
	addi r4, r4, _gj_matrix_@l
	lwz r5, 0x0(r4)
	psq_l f2, 0x0(r5), 0, 0
	ps_mul f4, f0, f2
	psq_l f3, 0x8(r5), 1, 0
	fmul f5, f1, f3
	psq_l f6, 0x10(r5), 0, 0
	psq_l f7, 0x18(r5), 1, 0
	psq_st f4, 0x0(r5), 0, 0
	ps_mul f8, f0, f6
	psq_st f5, 0x8(r5), 1, 0
	fmul f9, f1, f7
	psq_l f2, 0x20(r5), 0, 0
	psq_l f3, 0x28(r5), 1, 0
	psq_st f8, 0x10(r5), 0, 0
	psq_st f9, 0x18(r5), 1, 0
	ps_mul f4, f0, f2
	fmul f5, f1, f3
	psq_st f4, 0x20(r5), 0, 0
	psq_st f5, 0x28(r5), 1, 0
	blr
}
// clang-format on
// 0x80210594
// clang-format off
asm void njTranslateEx(const NJS_VECTOR* v)
{
	nofralloc
	lis r4, _gj_matrix_@ha
	psq_l f6, 0x0(r3), 0, 0
	psq_l f7, 0x8(r3), 1, 0
	addi r4, r4, _gj_matrix_@l
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
