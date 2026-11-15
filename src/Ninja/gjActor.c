#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

typedef struct GjVtxList {
	s16 type;     // _00 0 one matrix 1 and 2 two matrices
	s16 _02;      // _02
	u16 start;    // _04
	u16 count;    // _06
	void* data;   // _08
	void* weight; // _0C
} GjVtxList;

extern s32 gjActorFlag1;
extern u32 gjObjectEval8000;
extern u8* gjActorVerts;

s32 gjCalcActor(GjModel* model);
void gjBeginCalcActor(GjActor* actor);
void gjEndCalcActor(GjActor* actor);
void gjSetSkinGqr(void);
void gjSkinPosRigid(const f32* m, u8* out, void* data, s32 count);
void gjSkinPosWeight(const f32* m, u8* out, void* data, void* weight, s32 count);
void gjSkinPosWeight2(const f32* m, u8* out, void* data, void* weight, s32 count);
void gjSkinPosNrmARigid(const f32* m, u8* out, void* data, s32 count);
void gjSkinPosNrmAWeight(const f32* m, u8* out, void* data, void* weight, s32 count);
void gjSkinPosNrmAWeight2(const f32* m, u8* out, void* data, void* weight, s32 count);
void gjSkinPosNrmBRigid(const f32* m, u8* out, void* data, s32 count);
void gjSkinPosNrmBWeight(const f32* m, u8* out, void* data, void* weight, s32 count);
void gjSkinPosNrmBWeight2(const f32* m, u8* out, void* data, void* weight, s32 count);

f32 gjSavedNodeMatrix[12]; // a saved matrix for nodes with flags 0x800 and 0x1000

void gjBlendVerts(const f32* m, u8* out, void* a, f32 t, void* b, f32 rest, s32 count);

void gjSetRotation(const f32* m);
void gjWalkTree(NJS_OBJECT* obj, s32 (*f)(GjModel*));
void gjShapeBlendVerts(GjModel* model, void* a, f32 t, void* b, f32 rest);
void gjCalcActorModel(GjModel* model);
s32 gjCountTreeVerts(NJS_OBJECT* obj, s32 count);

// 0x802130E8
// Returns the rotation order 0 XYZ 1 ZXY 2 a quaternion
s32 gjGetMotionNodeData(NJS_OBJECT* obj, f32* pos, s32* ang, f32* scl, f32* quat)
{
	u32 flags = obj->evalflags;
	s32 type;

	if ((flags & 0x40) == 0) {
		if (gjCurMotionState->getPos(pos) == 0) {
			pos[0] = obj->pos[0];
			pos[1] = obj->pos[1];
			pos[2] = obj->pos[2];
		}
		if (gjCurMotionState->getAng != NULL) {
			if (gjCurMotionState->getAng(ang) != 0) {
				if (obj->evalflags & 0x20) {
					type = 1;
				} else {
					type = 0;
				}
			} else {
				ang[0] = obj->ang[0];
				ang[1] = obj->ang[1];
				ang[2] = obj->ang[2];
				if (obj->evalflags & 0x20) {
					type = 1;
				} else {
					type = 0;
				}
			}
		} else if (gjCurMotionState->getQuat((NJS_QUATERNION*)quat) != 0) {
			type = 2;
		} else {
			ang[0] = obj->ang[0];
			ang[1] = obj->ang[1];
			ang[2] = obj->ang[2];
			if (obj->evalflags & 0x20) {
				type = 1;
			} else {
				type = 0;
			}
		}
		if (gjCurMotionState->getScl(scl) == 0) {
			scl[0] = obj->scl[0];
			scl[1] = obj->scl[1];
			scl[2] = obj->scl[2];
		}
		njSetNextMotionNodeEx();
	} else {
		pos[0] = obj->pos[0];
		pos[1] = obj->pos[1];
		pos[2] = obj->pos[2];
		scl[0] = obj->scl[0];
		scl[1] = obj->scl[1];
		scl[2] = obj->scl[2];
		ang[0] = obj->ang[0];
		ang[1] = obj->ang[1];
		ang[2] = obj->ang[2];
		if (flags & 0x20) {
			type = 1;
		} else {
			type = 0;
		}
	}
	return type;
}

// 0x802132DC
// Vertices hold a position and a normal both as floats if the model or the actor asks for it
void gjDrawActor(GjActor* actor)
{
	NJS_MATRIX m;
	u8* verts = actor->verts;

	GXLoadPosMtxImm(_gj_matrix_, 0);
	GXLoadNrmMtxImm(_gj_matrix_, 0);
	if (actor->object->evalflags & 0x4000) {
		PSMTXCopy(_gj_matrix_, m);
		m[3]  = 0.0f;
		m[7]  = 0.0f;
		m[11] = 0.0f;
		GXLoadTexMtxImm(m, 0x2A, 0);
	}
	gjBeginDrawModel();
	if ((actor->object->evalflags & 0x8000) || (actor->flags & 1)) {
		GXSetArray(9, verts, 0x18);
		GXSetArray(10, verts + 0xC, 0x18);
		gjVertsAreFloat = 1;
	} else {
		GXSetArray(9, verts, 0xC);
		GXSetArray(10, verts + 6, 0xC);
	}
	_gjDrawModel(actor->object->model);
	gjEndDrawModel();
	gjVertsAreFloat = 0;
}

// 0x802133F4
// Replaces the current matrix's rotation
void gjSetRotation(const f32* m)
{
	f32* cur = _gj_matrix_;
	cur[0]   = m[0];
	cur[1]   = m[1];
	cur[2]   = m[2];
	cur[4]   = m[4];
	cur[5]   = m[5];
	cur[6]   = m[6];
	cur[8]   = m[8];
	cur[9]   = m[9];
	cur[10]  = m[10];
}

// 0x80213444
// Walks the tree without motion
void gjWalkTree(NJS_OBJECT* obj, s32 (*f)(GjModel*))
{
	do {
		u32 flags = obj->evalflags;
		if (flags & 0x800) {
			njGetMatrix(gjSavedNodeMatrix);
		}
		if ((flags & 7) == 7) {
			if ((flags & 8) == 0) {
				f(obj->model);
			}
			if ((flags & 0x10) == 0) {
				gjWalkTree(obj->child, f);
			}
		} else {
			njPushMatrixEx();
			if ((flags & 1) == 0) {
				njTranslateEx(obj->pos);
			}
			if ((flags & 2) == 0) {
				if (flags & 0x1000) {
					gjSetRotation(gjSavedNodeMatrix);
				} else if (flags & 0x400) {
					njQuaternionEx2(obj->quatRe, obj->ang);
				} else {
					njRotateEx(obj->ang, flags & 0x20);
				}
			}
			if ((flags & 4) == 0) {
				njScaleEx(obj->scl);
			}
			if ((flags & 8) == 0) {
				f(obj->model);
			}
			if ((flags & 0x10) == 0) {
				gjWalkTree(obj->child, f);
			}
			njPopMatrixEx();
		}
		obj = obj->sibling;
	} while (obj != NULL);
}

// 0x802135A0
void gjBeginCalcActor(GjActor* actor)
{
	gjActorVerts     = actor->verts;
	gjObjectEval8000 = actor->object->evalflags & 0x8000;
	gjActorFlag1     = actor->flags & 1;
	gjSetSkinGqr();
}

// 0x802135E4
// asm hand written
// clang-format off
asm void gjSetSkinGqr(void)
{
	nofralloc
	li r0, 0x807
	oris r0, r0, 0x807
	mtspr GQR2, r0
	li r0, 0x807
	oris r0, r0, 0x807
	mtspr GQR3, r0
	blr
}
// clang-format on

// 0x80213600
void gjEndCalcActor(GjActor* actor)
{
	DCFlushRange(actor->verts, actor->size);
}

// 0x8021362C
void gjBuildActorVertices(GjActor* actor)
{
	gjBeginCalcActor(actor);
	gjWalkTree(actor->object, gjCalcActor);
	gjEndCalcActor(actor);
}

// 0x80213670
s32 gjCalcActor(GjModel* model)
{
	GjVtxList* list;
	s32 type;

	if (model->vtxList == NULL) {
		return 1;
	}
	gjMtxPush();
	if (gjObjectEval8000 != 0) {
		for (list = model->vtxList; (type = list->type) != 3; list++) {
			u8* out = gjActorVerts + list->start * 0x18;
			switch (type) {
			case 0:
				gjSkinPosNrmARigid(_gj_matrix_, out, list->data, list->count);
				break;
			case 1:
				gjSkinPosNrmAWeight(_gj_matrix_, out, list->data, list->weight, list->count);
				break;
			case 2:
				gjSkinPosNrmAWeight2(_gj_matrix_, out, list->data, list->weight, list->count);
				break;
			}
		}
	} else if ((u32)gjActorFlag1 != 0) {
		for (list = model->vtxList; (type = list->type) != 3; list++) {
			u8* out = gjActorVerts + list->start * 0x18;
			switch (type) {
			case 0:
				gjSkinPosNrmBRigid(_gj_matrix_, out, list->data, list->count);
				break;
			case 1:
				gjSkinPosNrmBWeight(_gj_matrix_, out, list->data, list->weight, list->count);
				break;
			case 2:
				gjSkinPosNrmBWeight2(_gj_matrix_, out, list->data, list->weight, list->count);
				break;
			}
		}
	} else {
		for (list = model->vtxList; (type = list->type) != 3; list++) {
			u8* out = gjActorVerts + list->start * 0xC;
			switch (type) {
			case 0:
				gjSkinPosRigid(_gj_matrix_, out, list->data, list->count);
				break;
			case 1:
				gjSkinPosWeight(_gj_matrix_, out, list->data, list->weight, list->count);
				break;
			case 2:
				gjSkinPosWeight2(_gj_matrix_, out, list->data, list->weight, list->count);
				break;
			}
		}
	}
	njPopMatrixEx();
	return 1;
}

// 0x8021387C
// asm hand written
// clang-format off
asm void gjSkinPosRigid(const f32* m, u8* out, void* data, s32 count)
{
	nofralloc
	stwu r1, -0x40(r1)
	stfd f14, 0x8(r1)
	stfd f15, 0x10(r1)
	subi r6, r6, 0x1
	stfd f16, 0x18(r1)
	stfd f17, 0x20(r1)
	stfd f18, 0x28(r1)
	mtctr r6
	psq_l f0, 0x0(r3), 0, 0
	subi r5, r5, 0x4
	psq_l f1, 0x8(r3), 1, 0
	subi r4, r4, 0x2
	psq_l f6, 0x24(r3), 0, 0
	psq_lu f8, 0x4(r5), 0, 2
	psq_l f7, 0x2c(r3), 1, 0
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds0 f11, f0, f8, f6
	psq_l f2, 0xc(r3), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_l f3, 0x14(r3), 1, 0
	ps_muls1 f13, f0, f9
	psq_lu f10, 0x4(r5), 0, 2
	ps_muls1 f14, f1, f9
	psq_l f5, 0x20(r3), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l f4, 0x18(r3), 0, 0
	ps_madds0 f13, f2, f10, f13
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x4(r5), 0, 2
	cmpwi r6, 0x0
	beq L_80213964
L_80213914:
	ps_madds0 f11, f0, f8, f6
	psq_stu f15, 0x2(r4), 0, 2
	ps_madds0 f12, f1, f8, f7
	psq_stu f16, 0x4(r4), 1, 2
	ps_muls1 f13, f0, f9
	psq_stu f17, 0x2(r4), 0, 2
	ps_muls1 f14, f1, f9
	psq_stu f18, 0x4(r4), 1, 2
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f13, f2, f10, f13
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x4(r5), 0, 2
	bdnz L_80213914
L_80213964:
	psq_stu f15, 0x2(r4), 0, 2
	psq_stu f16, 0x4(r4), 1, 2
	psq_stu f17, 0x2(r4), 0, 2
	psq_stu f18, 0x4(r4), 1, 2
	lfd f14, 0x8(r1)
	lfd f15, 0x10(r1)
	lfd f16, 0x18(r1)
	lfd f17, 0x20(r1)
	lfd f18, 0x28(r1)
	addi r1, r1, 0x40
	blr
}
// clang-format on

// 0x80213990
// asm hand written
// clang-format off
asm void gjSkinPosWeight(const f32* m, u8* out, void* data, void* weight, s32 count)
{
	nofralloc
	stwu r1, -0x40(r1)
	stfd f14, 0x8(r1)
	stfd f15, 0x10(r1)
	subi r7, r7, 0x1
	stfd f16, 0x18(r1)
	stfd f17, 0x20(r1)
	stfd f18, 0x28(r1)
	stfd f19, 0x30(r1)
	mtctr r7
	psq_l f0, 0x0(r3), 0, 0
	subi r5, r5, 0x4
	psq_l f1, 0x8(r3), 1, 0
	subi r4, r4, 0x2
	psq_l f6, 0x24(r3), 0, 0
	subi r6, r6, 0x2
	psq_lu f8, 0x4(r5), 0, 2
	psq_l f7, 0x2c(r3), 1, 0
	psq_lu f9, 0x4(r5), 0, 2
	psq_lu f19, 0x4(r6), 1, 3
	ps_madds0 f11, f0, f8, f6
	psq_l f2, 0xc(r3), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_l f3, 0x14(r3), 1, 0
	ps_muls1 f13, f0, f9
	psq_lu f10, 0x4(r5), 0, 2
	ps_muls1 f14, f1, f9
	psq_l f5, 0x20(r3), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l f4, 0x18(r3), 0, 0
	ps_madds0 f13, f2, f10, f13
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x4(r5), 0, 2
	ps_muls0 f15, f15, f19
	ps_muls0 f16, f16, f19
	ps_muls0 f17, f17, f19
	ps_muls0 f18, f18, f19
	psq_lu f19, 0x4(r6), 1, 3
	cmpwi r7, 0x0
	beq L_80213AAC
L_80213A48:
	ps_madds0 f11, f0, f8, f6
	psq_stu f15, 0x2(r4), 0, 2
	ps_madds0 f12, f1, f8, f7
	psq_stu f16, 0x4(r4), 1, 2
	ps_muls1 f13, f0, f9
	psq_stu f17, 0x2(r4), 0, 2
	ps_muls1 f14, f1, f9
	psq_stu f18, 0x4(r4), 1, 2
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f13, f2, f10, f13
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x4(r5), 0, 2
	ps_muls0 f15, f15, f19
	ps_muls0 f16, f16, f19
	ps_muls0 f17, f17, f19
	ps_muls0 f18, f18, f19
	psq_lu f19, 0x4(r6), 1, 3
	bdnz L_80213A48
L_80213AAC:
	psq_stu f15, 0x2(r4), 0, 2
	psq_stu f16, 0x4(r4), 1, 2
	psq_stu f17, 0x2(r4), 0, 2
	psq_stu f18, 0x4(r4), 1, 2
	lfd f14, 0x8(r1)
	lfd f15, 0x10(r1)
	lfd f16, 0x18(r1)
	lfd f17, 0x20(r1)
	lfd f18, 0x28(r1)
	lfd f19, 0x30(r1)
	addi r1, r1, 0x40
	blr
}
// clang-format on

// 0x80213ADC
// asm hand written
// clang-format off
asm void gjSkinPosWeight2(const f32* m, u8* out, void* data, void* weight, s32 count)
{
	nofralloc
	stwu r1, -0x80(r1)
	stfd f14, 0x8(r1)
	stfd f15, 0x10(r1)
	subi r7, r7, 0x1
	stfd f16, 0x18(r1)
	stfd f17, 0x20(r1)
	stfd f18, 0x28(r1)
	stfd f19, 0x30(r1)
	stfd f20, 0x38(r1)
	stfd f21, 0x40(r1)
	stfd f22, 0x48(r1)
	stfd f23, 0x50(r1)
	mtctr r7
	psq_l f0, 0x0(r3), 0, 0
	subi r5, r5, 0x4
	psq_l f1, 0x8(r3), 1, 0
	subi r6, r6, 0x2
	psq_l f6, 0x24(r3), 0, 0
	psq_lu f8, 0x4(r5), 0, 2
	psq_l f7, 0x2c(r3), 1, 0
	psq_lu f9, 0x4(r5), 0, 2
	lhzu r9, 0x2(r6)
	mulli r9, r9, 0xc
	add r9, r9, r4
	psq_lu f19, 0x2(r6), 1, 3
	ps_madds0 f11, f0, f8, f6
	psq_l f2, 0xc(r3), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_l f3, 0x14(r3), 1, 0
	ps_muls1 f13, f0, f9
	psq_lu f10, 0x4(r5), 0, 2
	ps_muls1 f14, f1, f9
	psq_l f5, 0x20(r3), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l f4, 0x18(r3), 0, 0
	ps_madds0 f13, f2, f10, f13
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f14, f3, f10, f14
	psq_l f20, 0x0(r9), 0, 2
	ps_madds0 f11, f4, f9, f11
	psq_l f21, 0x4(r9), 1, 2
	ps_madds0 f12, f5, f9, f12
	psq_l f22, 0x6(r9), 0, 2
	ps_madds1 f13, f4, f10, f13
	psq_l f23, 0xa(r9), 1, 2
	ps_madds1 f14, f5, f10, f14
	mr r10, r9
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds0 f15, f11, f19, f20
	ps_madds0 f16, f12, f19, f21
	psq_lu f10, 0x4(r5), 0, 2
	ps_madds0 f17, f13, f19, f22
	ps_madds0 f18, f14, f19, f23
	cmpwi r7, 0x0
	beq L_80213C50
	lhzu r9, 0x2(r6)
	mulli r9, r9, 0xc
	add r9, r9, r4
	psq_lu f19, 0x2(r6), 1, 3
L_80213BCC:
	ps_madds0 f11, f0, f8, f6
	psq_st f15, 0x0(r10), 0, 2
	ps_madds0 f12, f1, f8, f7
	psq_st f16, 0x4(r10), 1, 2
	ps_muls1 f13, f0, f9
	psq_st f17, 0x6(r10), 0, 2
	ps_muls1 f14, f1, f9
	psq_st f18, 0xa(r10), 1, 2
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f13, f2, f10, f13
	ps_madds0 f14, f3, f10, f14
	psq_l f20, 0x0(r9), 0, 2
	ps_madds0 f11, f4, f9, f11
	psq_l f21, 0x4(r9), 1, 2
	ps_madds0 f12, f5, f9, f12
	psq_l f22, 0x6(r9), 0, 2
	ps_madds1 f13, f4, f10, f13
	psq_l f23, 0xa(r9), 1, 2
	ps_madds1 f14, f5, f10, f14
	mr r10, r9
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds0 f15, f11, f19, f20
	ps_madds0 f16, f12, f19, f21
	psq_lu f10, 0x4(r5), 0, 2
	ps_madds0 f17, f13, f19, f22
	ps_madds0 f18, f14, f19, f23
	lhzu r9, 0x2(r6)
	mulli r9, r9, 0xc
	add r9, r9, r4
	psq_lu f19, 0x2(r6), 1, 3
	bdnz L_80213BCC
L_80213C50:
	psq_st f15, 0x0(r10), 0, 2
	psq_st f16, 0x4(r10), 1, 2
	psq_st f17, 0x6(r10), 0, 2
	psq_st f18, 0xa(r10), 1, 2
	lfd f14, 0x8(r1)
	lfd f15, 0x10(r1)
	lfd f16, 0x18(r1)
	lfd f17, 0x20(r1)
	lfd f18, 0x28(r1)
	lfd f19, 0x30(r1)
	lfd f20, 0x38(r1)
	lfd f21, 0x40(r1)
	lfd f22, 0x48(r1)
	lfd f23, 0x50(r1)
	addi r1, r1, 0x80
	blr
}
// clang-format on

// 0x80213C90
// asm hand written
// clang-format off
asm void gjSkinPosNrmARigid(const f32* m, u8* out, void* data, s32 count)
{
	nofralloc
	stwu r1, -0x40(r1)
	stfd f14, 0x8(r1)
	stfd f15, 0x10(r1)
	subi r6, r6, 0x1
	stfd f16, 0x18(r1)
	stfd f17, 0x20(r1)
	stfd f18, 0x28(r1)
	mtctr r6
	psq_l f0, 0x0(r3), 0, 0
	subi r5, r5, 0x8
	psq_l f1, 0x8(r3), 1, 0
	subi r4, r4, 0x4
	psq_l f6, 0x24(r3), 0, 0
	psq_lu f8, 0x8(r5), 0, 0
	psq_l f7, 0x2c(r3), 1, 0
	psq_lu f9, 0x8(r5), 0, 0
	ps_madds0 f11, f0, f8, f6
	psq_l f2, 0xc(r3), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_l f3, 0x14(r3), 1, 0
	ps_muls1 f13, f0, f9
	psq_lu f10, 0x8(r5), 0, 0
	ps_muls1 f14, f1, f9
	psq_l f5, 0x20(r3), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l f4, 0x18(r3), 0, 0
	ps_madds0 f13, f2, f10, f13
	psq_lu f8, 0x8(r5), 0, 2
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x8(r5), 0, 0
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x8(r5), 0, 0
	cmpwi r6, 0x0
	beq L_80213D78
L_80213D28:
	ps_madds0 f11, f0, f8, f6
	psq_stu f15, 0x4(r4), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_stu f16, 0x8(r4), 1, 0
	ps_muls1 f13, f0, f9
	psq_stu f17, 0x4(r4), 0, 0
	ps_muls1 f14, f1, f9
	psq_stu f18, 0x8(r4), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu f8, 0x8(r5), 0, 0
	ps_madds0 f13, f2, f10, f13
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x8(r5), 0, 0
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x8(r5), 0, 0
	bdnz L_80213D28
L_80213D78:
	psq_stu f15, 0x4(r4), 0, 0
	psq_stu f16, 0x8(r4), 1, 0
	psq_stu f17, 0x4(r4), 0, 0
	psq_stu f18, 0x8(r4), 1, 0
	lfd f14, 0x8(r1)
	lfd f15, 0x10(r1)
	lfd f16, 0x18(r1)
	lfd f17, 0x20(r1)
	lfd f18, 0x28(r1)
	addi r1, r1, 0x40
	blr
}
// clang-format on

// 0x80213DA4
// asm hand written
// clang-format off
asm void gjSkinPosNrmAWeight(const f32* m, u8* out, void* data, void* weight, s32 count)
{
	nofralloc
	stwu r1, -0x40(r1)
	stfd f14, 0x8(r1)
	stfd f15, 0x10(r1)
	subi r7, r7, 0x1
	stfd f16, 0x18(r1)
	stfd f17, 0x20(r1)
	stfd f18, 0x28(r1)
	stfd f19, 0x30(r1)
	mtctr r7
	psq_l f0, 0x0(r3), 0, 0
	subi r5, r5, 0x8
	psq_l f1, 0x8(r3), 1, 0
	subi r4, r4, 0x4
	psq_l f6, 0x24(r3), 0, 0
	subi r6, r6, 0x2
	psq_lu f8, 0x8(r5), 0, 0
	psq_l f7, 0x2c(r3), 1, 0
	psq_lu f9, 0x8(r5), 0, 0
	psq_lu f19, 0x4(r6), 1, 3
	ps_madds0 f11, f0, f8, f6
	psq_l f2, 0xc(r3), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_l f3, 0x14(r3), 1, 0
	ps_muls1 f13, f0, f9
	psq_lu f10, 0x8(r5), 0, 0
	ps_muls1 f14, f1, f9
	psq_l f5, 0x20(r3), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l f4, 0x18(r3), 0, 0
	ps_madds0 f13, f2, f10, f13
	psq_lu f8, 0x8(r5), 0, 0
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x8(r5), 0, 0
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x8(r5), 0, 0
	ps_muls0 f15, f15, f19
	ps_muls0 f16, f16, f19
	ps_muls0 f17, f17, f19
	ps_muls0 f18, f18, f19
	psq_lu f19, 0x4(r6), 1, 3
	cmpwi r7, 0x0
	beq L_80213EC0
L_80213E5C:
	ps_madds0 f11, f0, f8, f6
	psq_stu f15, 0x4(r4), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_stu f16, 0x8(r4), 1, 0
	ps_muls1 f13, f0, f9
	psq_stu f17, 0x4(r4), 0, 0
	ps_muls1 f14, f1, f9
	psq_stu f18, 0x8(r4), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu f8, 0x8(r5), 0, 0
	ps_madds0 f13, f2, f10, f13
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x8(r5), 0, 0
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x8(r5), 0, 0
	ps_muls0 f15, f15, f19
	ps_muls0 f16, f16, f19
	ps_muls0 f17, f17, f19
	ps_muls0 f18, f18, f19
	psq_lu f19, 0x4(r6), 1, 3
	bdnz L_80213E5C
L_80213EC0:
	psq_stu f15, 0x4(r4), 0, 0
	psq_stu f16, 0x8(r4), 1, 0
	psq_stu f17, 0x4(r4), 0, 0
	psq_stu f18, 0x8(r4), 1, 0
	lfd f14, 0x8(r1)
	lfd f15, 0x10(r1)
	lfd f16, 0x18(r1)
	lfd f17, 0x20(r1)
	lfd f18, 0x28(r1)
	lfd f19, 0x30(r1)
	addi r1, r1, 0x40
	blr
}
// clang-format on

// 0x80213EF0
// asm hand written
// clang-format off
asm void gjSkinPosNrmAWeight2(const f32* m, u8* out, void* data, void* weight, s32 count)
{
	nofralloc
	stwu r1, -0x80(r1)
	stfd f14, 0x8(r1)
	stfd f15, 0x10(r1)
	subi r7, r7, 0x1
	stfd f16, 0x18(r1)
	stfd f17, 0x20(r1)
	stfd f18, 0x28(r1)
	stfd f19, 0x30(r1)
	stfd f20, 0x38(r1)
	stfd f21, 0x40(r1)
	stfd f22, 0x48(r1)
	stfd f23, 0x50(r1)
	mtctr r7
	psq_l f0, 0x0(r3), 0, 0
	subi r5, r5, 0x8
	psq_l f1, 0x8(r3), 1, 0
	subi r6, r6, 0x2
	psq_l f6, 0x24(r3), 0, 0
	psq_lu f8, 0x8(r5), 0, 0
	psq_l f7, 0x2c(r3), 1, 0
	psq_lu f9, 0x8(r5), 0, 0
	lhzu r9, 0x2(r6)
	mulli r9, r9, 0x18
	add r9, r9, r4
	psq_lu f19, 0x2(r6), 1, 3
	ps_madds0 f11, f0, f8, f6
	psq_l f2, 0xc(r3), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_l f3, 0x14(r3), 1, 0
	ps_muls1 f13, f0, f9
	psq_lu f10, 0x8(r5), 0, 0
	ps_muls1 f14, f1, f9
	psq_l f5, 0x20(r3), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l f4, 0x18(r3), 0, 0
	ps_madds0 f13, f2, f10, f13
	psq_lu f8, 0x8(r5), 0, 0
	ps_madds0 f14, f3, f10, f14
	psq_l f20, 0x0(r9), 0, 0
	ps_madds0 f11, f4, f9, f11
	psq_l f21, 0x8(r9), 1, 0
	ps_madds0 f12, f5, f9, f12
	psq_l f22, 0xc(r9), 0, 0
	ps_madds1 f13, f4, f10, f13
	psq_l f23, 0x14(r9), 1, 0
	ps_madds1 f14, f5, f10, f14
	mr r10, r9
	psq_lu f9, 0x8(r5), 0, 0
	ps_madds0 f15, f11, f19, f20
	ps_madds0 f16, f12, f19, f21
	psq_lu f10, 0x8(r5), 0, 0
	ps_madds0 f17, f13, f19, f22
	ps_madds0 f18, f14, f19, f23
	cmpwi r7, 0x0
	beq L_80214064
	lhzu r9, 0x2(r6)
	mulli r9, r9, 0x18
	add r9, r9, r4
	psq_lu f19, 0x2(r6), 1, 3
L_80213FE0:
	ps_madds0 f11, f0, f8, f6
	psq_st f15, 0x0(r10), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_st f16, 0x8(r10), 1, 0
	ps_muls1 f13, f0, f9
	psq_st f17, 0xc(r10), 0, 0
	ps_muls1 f14, f1, f9
	psq_st f18, 0x14(r10), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu f8, 0x8(r5), 0, 0
	ps_madds0 f13, f2, f10, f13
	ps_madds0 f14, f3, f10, f14
	psq_l f20, 0x0(r9), 0, 0
	ps_madds0 f11, f4, f9, f11
	psq_l f21, 0x8(r9), 1, 0
	ps_madds0 f12, f5, f9, f12
	psq_l f22, 0xc(r9), 0, 0
	ps_madds1 f13, f4, f10, f13
	psq_l f23, 0x14(r9), 1, 0
	ps_madds1 f14, f5, f10, f14
	mr r10, r9
	psq_lu f9, 0x8(r5), 0, 0
	ps_madds0 f15, f11, f19, f20
	ps_madds0 f16, f12, f19, f21
	psq_lu f10, 0x8(r5), 0, 0
	ps_madds0 f17, f13, f19, f22
	ps_madds0 f18, f14, f19, f23
	lhzu r9, 0x2(r6)
	mulli r9, r9, 0x18
	add r9, r9, r4
	psq_lu f19, 0x2(r6), 1, 3
	bdnz L_80213FE0
L_80214064:
	psq_st f15, 0x0(r10), 0, 0
	psq_st f16, 0x8(r10), 1, 0
	psq_st f17, 0xc(r10), 0, 0
	psq_st f18, 0x14(r10), 1, 0
	lfd f14, 0x8(r1)
	lfd f15, 0x10(r1)
	lfd f16, 0x18(r1)
	lfd f17, 0x20(r1)
	lfd f18, 0x28(r1)
	lfd f19, 0x30(r1)
	lfd f20, 0x38(r1)
	lfd f21, 0x40(r1)
	lfd f22, 0x48(r1)
	lfd f23, 0x50(r1)
	addi r1, r1, 0x80
	blr
}
// clang-format on

// 0x802140A4
// asm hand written
// clang-format off
asm void gjSkinPosNrmBRigid(const f32* m, u8* out, void* data, s32 count)
{
	nofralloc
	stwu r1, -0x40(r1)
	stfd f14, 0x8(r1)
	stfd f15, 0x10(r1)
	subi r6, r6, 0x1
	stfd f16, 0x18(r1)
	stfd f17, 0x20(r1)
	stfd f18, 0x28(r1)
	mtctr r6
	psq_l f0, 0x0(r3), 0, 0
	subi r5, r5, 0x4
	psq_l f1, 0x8(r3), 1, 0
	subi r4, r4, 0x4
	psq_l f6, 0x24(r3), 0, 0
	psq_lu f8, 0x4(r5), 0, 2
	psq_l f7, 0x2c(r3), 1, 0
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds0 f11, f0, f8, f6
	psq_l f2, 0xc(r3), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_l f3, 0x14(r3), 1, 0
	ps_muls1 f13, f0, f9
	psq_lu f10, 0x4(r5), 0, 2
	ps_muls1 f14, f1, f9
	psq_l f5, 0x20(r3), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l f4, 0x18(r3), 0, 0
	ps_madds0 f13, f2, f10, f13
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x4(r5), 0, 2
	cmpwi r6, 0x0
	beq L_8021418C
L_8021413C:
	ps_madds0 f11, f0, f8, f6
	psq_stu f15, 0x4(r4), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_stu f16, 0x8(r4), 1, 0
	ps_muls1 f13, f0, f9
	psq_stu f17, 0x4(r4), 0, 0
	ps_muls1 f14, f1, f9
	psq_stu f18, 0x8(r4), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f13, f2, f10, f13
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x4(r5), 0, 2
	bdnz L_8021413C
L_8021418C:
	psq_stu f15, 0x4(r4), 0, 0
	psq_stu f16, 0x8(r4), 1, 0
	psq_stu f17, 0x4(r4), 0, 0
	psq_stu f18, 0x8(r4), 1, 0
	lfd f14, 0x8(r1)
	lfd f15, 0x10(r1)
	lfd f16, 0x18(r1)
	lfd f17, 0x20(r1)
	lfd f18, 0x28(r1)
	addi r1, r1, 0x40
	blr
}
// clang-format on

// 0x802141B8
// asm hand written
// clang-format off
asm void gjSkinPosNrmBWeight(const f32* m, u8* out, void* data, void* weight, s32 count)
{
	nofralloc
	stwu r1, -0x40(r1)
	stfd f14, 0x8(r1)
	stfd f15, 0x10(r1)
	subi r7, r7, 0x1
	stfd f16, 0x18(r1)
	stfd f17, 0x20(r1)
	stfd f18, 0x28(r1)
	stfd f19, 0x30(r1)
	mtctr r7
	psq_l f0, 0x0(r3), 0, 0
	subi r5, r5, 0x4
	psq_l f1, 0x8(r3), 1, 0
	subi r4, r4, 0x4
	psq_l f6, 0x24(r3), 0, 0
	subi r6, r6, 0x2
	psq_lu f8, 0x4(r5), 0, 2
	psq_l f7, 0x2c(r3), 1, 0
	psq_lu f9, 0x4(r5), 0, 2
	psq_lu f19, 0x4(r6), 1, 3
	ps_madds0 f11, f0, f8, f6
	psq_l f2, 0xc(r3), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_l f3, 0x14(r3), 1, 0
	ps_muls1 f13, f0, f9
	psq_lu f10, 0x4(r5), 0, 2
	ps_muls1 f14, f1, f9
	psq_l f5, 0x20(r3), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l f4, 0x18(r3), 0, 0
	ps_madds0 f13, f2, f10, f13
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x4(r5), 0, 2
	ps_muls0 f15, f15, f19
	ps_muls0 f16, f16, f19
	ps_muls0 f17, f17, f19
	ps_muls0 f18, f18, f19
	psq_lu f19, 0x4(r6), 1, 3
	cmpwi r7, 0x0
	beq L_802142D4
L_80214270:
	ps_madds0 f11, f0, f8, f6
	psq_stu f15, 0x4(r4), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_stu f16, 0x8(r4), 1, 0
	ps_muls1 f13, f0, f9
	psq_stu f17, 0x4(r4), 0, 0
	ps_muls1 f14, f1, f9
	psq_stu f18, 0x8(r4), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f13, f2, f10, f13
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x4(r5), 0, 2
	ps_muls0 f15, f15, f19
	ps_muls0 f16, f16, f19
	ps_muls0 f17, f17, f19
	ps_muls0 f18, f18, f19
	psq_lu f19, 0x4(r6), 1, 3
	bdnz L_80214270
L_802142D4:
	psq_stu f15, 0x4(r4), 0, 0
	psq_stu f16, 0x8(r4), 1, 0
	psq_stu f17, 0x4(r4), 0, 0
	psq_stu f18, 0x8(r4), 1, 0
	lfd f14, 0x8(r1)
	lfd f15, 0x10(r1)
	lfd f16, 0x18(r1)
	lfd f17, 0x20(r1)
	lfd f18, 0x28(r1)
	lfd f19, 0x30(r1)
	addi r1, r1, 0x40
	blr
}
// clang-format on

// 0x80214304
// asm hand written
// clang-format off
asm void gjSkinPosNrmBWeight2(const f32* m, u8* out, void* data, void* weight, s32 count)
{
	nofralloc
	stwu r1, -0x80(r1)
	stfd f14, 0x8(r1)
	stfd f15, 0x10(r1)
	subi r7, r7, 0x1
	stfd f16, 0x18(r1)
	stfd f17, 0x20(r1)
	stfd f18, 0x28(r1)
	stfd f19, 0x30(r1)
	stfd f20, 0x38(r1)
	stfd f21, 0x40(r1)
	stfd f22, 0x48(r1)
	stfd f23, 0x50(r1)
	mtctr r7
	psq_l f0, 0x0(r3), 0, 0
	subi r5, r5, 0x4
	psq_l f1, 0x8(r3), 1, 0
	subi r6, r6, 0x2
	psq_l f6, 0x24(r3), 0, 0
	psq_lu f8, 0x4(r5), 0, 2
	psq_l f7, 0x2c(r3), 1, 0
	psq_lu f9, 0x4(r5), 0, 2
	lhzu r9, 0x2(r6)
	mulli r9, r9, 0x18
	add r9, r9, r4
	psq_lu f19, 0x2(r6), 1, 3
	ps_madds0 f11, f0, f8, f6
	psq_l f2, 0xc(r3), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_l f3, 0x14(r3), 1, 0
	ps_muls1 f13, f0, f9
	psq_lu f10, 0x4(r5), 0, 2
	ps_muls1 f14, f1, f9
	psq_l f5, 0x20(r3), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l f4, 0x18(r3), 0, 0
	ps_madds0 f13, f2, f10, f13
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f14, f3, f10, f14
	psq_l f20, 0x0(r9), 0, 0
	ps_madds0 f11, f4, f9, f11
	psq_l f21, 0x8(r9), 1, 0
	ps_madds0 f12, f5, f9, f12
	psq_l f22, 0xc(r9), 0, 0
	ps_madds1 f13, f4, f10, f13
	psq_l f23, 0x14(r9), 1, 0
	ps_madds1 f14, f5, f10, f14
	mr r10, r9
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds0 f15, f11, f19, f20
	ps_madds0 f16, f12, f19, f21
	psq_lu f10, 0x4(r5), 0, 2
	ps_madds0 f17, f13, f19, f22
	ps_madds0 f18, f14, f19, f23
	cmpwi r7, 0x0
	beq L_80214478
	lhzu r9, 0x2(r6)
	mulli r9, r9, 0x18
	add r9, r9, r4
	psq_lu f19, 0x2(r6), 1, 3
L_802143F4:
	ps_madds0 f11, f0, f8, f6
	psq_st f15, 0x0(r10), 0, 0
	ps_madds0 f12, f1, f8, f7
	psq_st f16, 0x8(r10), 1, 0
	ps_muls1 f13, f0, f9
	psq_st f17, 0xc(r10), 0, 0
	ps_muls1 f14, f1, f9
	psq_st f18, 0x14(r10), 1, 0
	ps_madds1 f11, f2, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu f8, 0x4(r5), 0, 2
	ps_madds0 f13, f2, f10, f13
	ps_madds0 f14, f3, f10, f14
	psq_l f20, 0x0(r9), 0, 0
	ps_madds0 f11, f4, f9, f11
	psq_l f21, 0x8(r9), 1, 0
	ps_madds0 f12, f5, f9, f12
	psq_l f22, 0xc(r9), 0, 0
	ps_madds1 f13, f4, f10, f13
	psq_l f23, 0x14(r9), 1, 0
	ps_madds1 f14, f5, f10, f14
	mr r10, r9
	psq_lu f9, 0x4(r5), 0, 2
	ps_madds0 f15, f11, f19, f20
	ps_madds0 f16, f12, f19, f21
	psq_lu f10, 0x4(r5), 0, 2
	ps_madds0 f17, f13, f19, f22
	ps_madds0 f18, f14, f19, f23
	lhzu r9, 0x2(r6)
	mulli r9, r9, 0x18
	add r9, r9, r4
	psq_lu f19, 0x2(r6), 1, 3
	bdnz L_802143F4
L_80214478:
	psq_st f15, 0x0(r10), 0, 0
	psq_st f16, 0x8(r10), 1, 0
	psq_st f17, 0xc(r10), 0, 0
	psq_st f18, 0x14(r10), 1, 0
	lfd f14, 0x8(r1)
	lfd f15, 0x10(r1)
	lfd f16, 0x18(r1)
	lfd f17, 0x20(r1)
	lfd f18, 0x28(r1)
	lfd f19, 0x30(r1)
	lfd f20, 0x38(r1)
	lfd f21, 0x40(r1)
	lfd f22, 0x48(r1)
	lfd f23, 0x50(r1)
	addi r1, r1, 0x80
	blr
}
// clang-format on

// 0x802144B8
// asm hand written
// clang-format off
asm void gjBlendVerts(const f32* m, u8* out, void* a, f32 t, void* b, f32 rest, s32 count)
{
	nofralloc
	stwu r1, -0x40(r1)
	stfd f14, 0x8(r1)
	stfd f15, 0x10(r1)
	subi r7, r7, 0x1
	stfd f16, 0x18(r1)
	stfd f17, 0x20(r1)
	stfd f18, 0x28(r1)
	stfd f19, 0x30(r1)
	stfd f20, 0x34(r1)
	stfd f21, 0x38(r1)
	mtctr r7
	psq_l f0, 0x0(r3), 0, 0
	subi r5, r5, 0x4
	subi r6, r6, 0x4
	psq_l f22, 0x8(r3), 1, 0
	subi r4, r4, 0x2
	psq_l f6, 0x24(r3), 0, 0
	psq_l f7, 0x2c(r3), 1, 0
	psq_lu f8, 0x4(r5), 0, 2
	psq_lu f19, 0x4(r6), 0, 2
	ps_muls0 f8, f8, f2
	ps_madds0 f8, f19, f1, f8
	psq_lu f9, 0x4(r5), 0, 2
	psq_lu f20, 0x4(r6), 0, 2
	ps_muls0 f9, f9, f2
	ps_madds0 f9, f20, f1, f9
	ps_madds0 f11, f0, f8, f6
	psq_l f23, 0xc(r3), 0, 0
	ps_madds0 f12, f22, f8, f7
	psq_l f3, 0x14(r3), 1, 0
	ps_muls1 f13, f0, f9
	psq_lu f10, 0x4(r5), 0, 2
	psq_lu f21, 0x4(r6), 0, 2
	ps_muls0 f10, f10, f2
	ps_madds0 f10, f21, f1, f10
	ps_muls1 f14, f22, f9
	psq_l f5, 0x20(r3), 1, 0
	ps_madds1 f11, f23, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_l f4, 0x18(r3), 0, 0
	ps_madds0 f13, f23, f10, f13
	psq_lu f8, 0x4(r5), 0, 2
	psq_lu f19, 0x4(r6), 0, 2
	ps_muls0 f8, f8, f2
	ps_madds0 f8, f19, f1, f8
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x4(r5), 0, 2
	psq_lu f20, 0x4(r6), 0, 2
	ps_muls0 f9, f9, f2
	ps_madds0 f9, f20, f1, f9
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x4(r5), 0, 2
	psq_lu f21, 0x4(r6), 0, 2
	ps_muls0 f10, f10, f2
	ps_madds0 f10, f21, f1, f10
	cmpwi r7, 0x0
	beq L_8021461C
L_802145A8:
	ps_madds0 f11, f0, f8, f6
	psq_stu f15, 0x2(r4), 0, 2
	ps_madds0 f12, f22, f8, f7
	psq_stu f16, 0x4(r4), 1, 2
	ps_muls1 f13, f0, f9
	psq_stu f17, 0x2(r4), 0, 2
	ps_muls1 f14, f22, f9
	psq_stu f18, 0x4(r4), 1, 2
	ps_madds1 f11, f23, f8, f11
	ps_madds1 f12, f3, f8, f12
	psq_lu f8, 0x4(r5), 0, 2
	ps_muls0 f8, f8, f2
	psq_lu f19, 0x4(r6), 0, 2
	ps_madds0 f8, f19, f1, f8
	ps_madds0 f13, f23, f10, f13
	ps_madds0 f14, f3, f10, f14
	ps_madds0 f15, f4, f9, f11
	ps_madds0 f16, f5, f9, f12
	psq_lu f9, 0x4(r5), 0, 2
	psq_lu f20, 0x4(r6), 0, 2
	ps_muls0 f9, f9, f2
	ps_madds0 f9, f20, f1, f9
	ps_madds1 f17, f4, f10, f13
	ps_madds1 f18, f5, f10, f14
	psq_lu f10, 0x4(r5), 0, 2
	psq_lu f21, 0x4(r6), 0, 2
	ps_muls0 f10, f10, f2
	ps_madds0 f10, f21, f1, f10
	bdnz L_802145A8
L_8021461C:
	psq_stu f15, 0x2(r4), 0, 2
	psq_stu f16, 0x4(r4), 1, 2
	psq_stu f17, 0x2(r4), 0, 2
	psq_stu f18, 0x4(r4), 1, 2
	lfd f14, 0x8(r1)
	lfd f15, 0x10(r1)
	lfd f16, 0x18(r1)
	lfd f17, 0x20(r1)
	lfd f18, 0x28(r1)
	lfd f19, 0x30(r1)
	lfd f19, 0x34(r1)
	lfd f19, 0x38(r1)
	addi r1, r1, 0x40
	blr
}
// clang-format on

// 0x80214654
// Shape motion blends two vertex sets into the buffer
void gjShapeBlendVerts(GjModel* model, void* a, f32 t, void* b, f32 rest)
{
	GjVtxList* list;
	s32 type;

	if (model->vtxList != NULL) {
		gjMtxPush();
		for (list = model->vtxList; (type = list->type) != 3; list++) {
			u8* out = gjActorVerts + list->start * 0xC;
			switch (type) {
			case 0:
				gjBlendVerts(_gj_matrix_, out, a, t, b, rest, list->count);
				break;
			}
		}
		njPopMatrixEx();
	}
}

// 0x80214710
void gjCalcActorModel(GjModel* model)
{
	gjCalcActor(model);
}

// 0x80214730
// An object without flag 0x2000 hangs here
u8* gjCreateActor(NJS_OBJECT* obj, u8* mem)
{
	u8* verts;
	GjActor* actor;
	s32 n;

	if ((obj->evalflags & 0x2000) == 0) {
		for (;;) { }
	}
	actor         = (GjActor*)(verts = mem);
	actor->object = obj;
	actor->flags  = 0;
	verts += 0x20;
	n            = gjCountTreeVerts(obj, 0);
	actor->verts = verts;
	if (obj->evalflags & 0x8000) {
		actor->size = n * 0x18;
	} else {
		actor->size = n * 0xC;
	}
	verts += actor->size;
	return verts;
}

// 0x802147D0
u8* gjCreateActorEx(NJS_OBJECT* obj, u8* mem, u32 flags)
{
	u8* verts;
	GjActor* actor;
	s32 n;

	if ((obj->evalflags & 0x2000) == 0) {
		for (;;) { }
	}
	actor         = (GjActor*)(verts = mem);
	actor->object = obj;
	actor->flags  = flags;
	verts += 0x20;
	n            = gjCountTreeVerts(obj, 0);
	actor->verts = verts;
	if (obj->evalflags & 0x8000) {
		actor->size = n * 0x18;
	} else if (flags & 1) {
		actor->size = n * 0x18;
	} else {
		actor->size = n * 0xC;
	}
	verts += actor->size;
	return verts;
}

// 0x80214890
s32 gjActorSize(NJS_OBJECT* obj)
{
	if (obj->evalflags & 0x8000) {
		return gjCountTreeVerts(obj, 0) * 0x18 + 0x20;
	}
	return gjCountTreeVerts(obj, 0) * 0xC + 0x20;
}

// 0x802148E0
// Counts the tree's vertices Type 2 runs reuse earlier ones
s32 gjCountTreeVerts(NJS_OBJECT* obj, s32 count)
{
	do {
		u32 flags = obj->evalflags;
		if ((flags & 8) == 0) {
			GjModel* model = obj->model;
			if (model->vtxList != NULL) {
				GjVtxList* list;
				s32 type;
				for (list = model->vtxList; (type = list->type) != 3; list++) {
					if (type == 0 || type == 1) {
						count += list->count;
					}
				}
			}
		}
		if ((flags & 0x10) == 0) {
			count = gjCountTreeVerts(obj->child, count);
		}
		obj = obj->sibling;
	} while (obj != NULL);
	return count;
}
