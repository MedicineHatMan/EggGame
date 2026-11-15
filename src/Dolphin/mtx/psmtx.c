#include "Dolphin/mtx.h"

typedef f32 ROMtx[4][3];

/** Copies src into dest reordered for PSMTXROMultVecArray */
asm void PSMTXReorder(const register Mtx src, register ROMtx dest)
{
#ifdef __MWERKS__ // clang-format off
	nofralloc
	psq_l    f0, 0x0(src), 0, 0
	psq_l    f2, 0x10(src), 0, 0
	psq_l    f4, 0x20(src), 0, 0
	psq_l    f1, 0x8(src), 0, 0
	ps_merge00 f6, f0, f2
	psq_l    f3, 0x18(src), 0, 0
	ps_merge01 f12, f4, f0
	psq_l    f5, 0x28(src), 0, 0
	ps_merge11 f7, f2, f4
	psq_st   f6, 0x0(dest), 0, 0
	ps_merge00 f8, f1, f3
	psq_st   f12, 0x8(dest), 0, 0
	ps_merge01 f9, f5, f1
	psq_st   f7, 0x10(dest), 0, 0
	ps_merge11 f10, f3, f5
	psq_st   f8, 0x18(dest), 0, 0
	psq_st   f9, 0x20(dest), 0, 0
	psq_st   f10, 0x28(dest), 0, 0
	blr
#endif // clang-format on
}

/** dstBase[i] = m * srcBase[i] for count points */
asm void PSMTXROMultVecArray(const register ROMtx m, const register Vec* srcBase, register Vec* dstBase, register u32 count)
{
#ifdef __MWERKS__ // clang-format off
	nofralloc
	stwu     r1, -0x40(r1)
	stfd     f14, 0x8(r1)
	subi     r7, count, 0x1
	stfd     f15, 0x10(r1)
	srwi     r7, r7, 1
	stfd     f16, 0x18(r1)
	stfd     f17, 0x20(r1)
	stfd     f18, 0x28(r1)
	mtctr    r7
	psq_l    f0, 0x0(m), 0, 0
	subi     srcBase, srcBase, 0x8
	psq_l    f1, 0x8(m), 1, 0
	subi     dstBase, dstBase, 0x4
	psq_l    f6, 0x24(m), 0, 0
	psq_lu   f8, 0x8(srcBase), 0, 0
	psq_l    f7, 0x2c(m), 1, 0
	psq_lu   f9, 0x8(srcBase), 0, 0
	ps_madds0 f11, f0, f8, f6
	psq_l    f2, 0xc(m), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_l    f3, 0x14(m), 1, 0
	ps_madds1 f13, f0, f9, f6
	psq_lu   f10, 0x8(srcBase), 0, 0
	ps_madds1 f14, f1, f9, f7
	psq_l    f5, 0x20(m), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l    f4, 0x18(m), 0, 0
	ps_madds0 f13, f2, f10, f13
	psq_lu   f8, 0x8(srcBase), 0, 0
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu   f9, 0x8(srcBase), 0, 0
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu   f10, 0x8(srcBase), 0, 0
loop:
	ps_madds0 f11, f0, f8, f6
	psq_stu  f15, 0x4(dstBase), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_stu  f16, 0x8(dstBase), 1, 0
	ps_madds1 f13, f0, f9, f6
	psq_stu  f17, 0x4(dstBase), 0, 0
	ps_madds1 f14, f1, f9, f7
	psq_stu  f18, 0x8(dstBase), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu   f8, 0x8(srcBase), 0, 0
	ps_madds0 f13, f2, f10, f13
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu   f9, 0x8(srcBase), 0, 0
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu   f10, 0x8(srcBase), 0, 0
	bdnz     loop
	psq_stu  f15, 0x4(dstBase), 0, 0
	clrlwi.  r7, count, 31
	psq_stu  f16, 0x8(dstBase), 1, 0
	bne      exit
	psq_stu  f17, 0x4(dstBase), 0, 0
	psq_stu  f18, 0x8(dstBase), 1, 0
exit:
	lfd      f14, 0x8(r1)
	lfd      f15, 0x10(r1)
	lfd      f16, 0x18(r1)
	lfd      f17, 0x20(r1)
	lfd      f18, 0x28(r1)
	addi     r1, r1, 0x40
	blr
#endif // clang-format on
}
