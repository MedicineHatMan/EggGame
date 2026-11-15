#include "stage/Stage.h"
#include "engine/Fx.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/FxSystem.h"
#include "engine/FxAnim.h"
#include "engine/FxSprite.h"
#include "engine/FxParticle.h"
#include "engine/FxParticle2.h"
#include "engine/FxEmitter.h"
#include "Ninja/ninja.h"
#include "Dolphin/vec.h"
#include "engine/FxShape.h"
#include "core/SdkCalls.h"
struct FxEmitter;
struct FxShape;

#define FX_PIPE (*(volatile f32*)0xCC008000)
#define FX_PIPE8 (*(volatile u8*)0xCC008000)
#define FX_ANGLE 10430.38043493439

struct FxRigDesc {
	u16 mBoneCount;  // _00
	u16 mLife;       // _02
	u32 mFlags;      // _04
	u8* mScript; // _08
};

// 0x800DEE60
FxEmitter* fxEmitterNew(FxBank* bank, int index)
{
	FxEmitter* fx = (FxEmitter*)fxEmitterAlloc();
	if (fx == NULL) {
		return NULL;
	}
	FxEmitterDesc* desc = (FxEmitterDesc*)bank->mDescs[index * 2 + 1];
	fx->mUpdate = (void (*)(Fx*))sFxEmitterUpdaters[desc->mKind];
	fx->mDraw   = sFxEmitterDrawers[desc->mKind];
	fx->mRemove = (void (*)(Fx*))fxEmitterFree;
	fx->mFlags |= desc->mFlags;
	fx->mKind = desc->mKind;
	fx->mStop = desc->mLife != 0 ? desc->mLife : 0xFFFF;
	fx->mRate      = desc->mRate;
	fx->mAccum     = 0.0f;
	fx->mMax       = desc->mMax;
	fx->mPartLife  = desc->mPartLife;
	fx->mPartFlags = desc->mPartFlags;
	fx->mTex[0]    = desc->mTex[0];
	fx->mTex[1]    = desc->mTex[1];
	fx->mTexGroup  = desc->mTexGroup;
	fx->mMaterial  = desc->mMaterial;
	fx->mDir[0] = desc->mDir[0];
	fx->mDir[1] = desc->mDir[1];
	fx->mDir[2] = desc->mDir[2];
	fx->mGravity = desc->mGravity;
	fx->mDamp    = desc->mDamp;
	fx->mScale   = desc->mScale;
	fx->mPartScript = desc->mPartScript;
	fx->mCount = 0;
	fx->mKids  = NULL;
	fx->_A0 = desc->_14;
	fx->_A4 = desc->_18;
	switch (fx->mKind) {
	case 1:
		*(u8*)&fx->mShapeA = desc->mCount;
		fx->mShapeB = desc->mShapeB;
		fx->mShapeC = desc->mShapeC;
		break;
	}
	fx->mLoops  = 0;
	fx->mPc     = 0;
	fx->mWait   = desc->mScript != NULL ? 1 : 0;
	fx->mScript = desc->mScript;
	return fx;
}

// 0x800DF000
// Runs the script makes new particles and steps the old
void fxEmitterStep(FxEmitter* fx, FxEmitter* self)
{
	if (self->mWait != 0 && --self->mWait == 0) {
		const u8* pc = self->mScript + self->mPc;
		u16 wait = 0;
		do {
			u32 op = *pc++;
			if (op == 0xFF) {
				wait = 0;
				if (!(fx->mFlags & 2)) {
					fx->mStop = 1;
				}
				break;
			}
			if (op < 0x80) {
				wait = op & 0x1F;
				if (op & 0x20) {
					wait = (wait << 8) + *pc++;
				}
				continue;
			}
			switch (op) {
			case 0x81:
				u8 tex = pc[0];
				u8 val = pc[1];
				pc += 2;
				self->mTex[tex] = val;
				break;
			case 0xF0:
				self->mLoops  = *pc++;
				self->mLoopPc = pc - self->mScript;
				break;
			case 0xF1:
				if (--self->mLoops != 0) {
					pc = self->mScript + self->mLoopPc;
				}
				break;
			case 0xF2:
				self->mMarkPc = pc - self->mScript;
				break;
			case 0xF3:
				pc = self->mScript + self->mMarkPc;
				break;
			}
		} while (wait == 0);
		self->mPc   = pc - self->mScript;
		self->mWait = wait;
	}
	if (self->mRate < 0.0f) {
		self->mAccum -= self->mRate;
	} else {
		self->mAccum += self->mRate * ((f32)rand() * 3.0517578E-5f);
	}
	while (self->mAccum >= 1.0f) {
		fxRibbonSpawn(self, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		self->mAccum -= 1.0f;
	}
	if (fx->mStop != 0 && --fx->mStop == 0) {
		fxEmitterStop(fx, self);
	}
	fxStepParticles(self, fxRibbonPartStep);
}

// 0x800DF258
void fxRibbonPartStep(FxParticle* part)
{
	if (part->mColorFrames != 0) {
		if (--part->mColorLeft == 0) {
			part->mColorFrames = 0;
			part->mColor[0]    = part->mColorTo[0];
			part->mColor[1]    = part->mColorTo[1];
			part->mColor[2]    = part->mColorTo[2];
		}
	}
	if (part->mAlphaFrames != 0) {
		if (--part->mAlphaLeft == 0) {
			part->mAlphaFrames = 0;
			part->mColor[3]    = part->mColorTo[3];
		}
	}
	if (part->mScaleFrames != 0) {
		part->mScale += (part->mScaleTo - part->mScale) / (f32)part->mScaleFrames;
		part->mScaleFrames--;
	}
	fxPartRunScript(part);
	if (part->mLife != 0 && --part->mLife == 0) {
		part->mFlags |= 0x100;
	}
	if (part->mFlags & 1) {
		part->mVel[1] += part->mGravity;
	}
	if (part->mFlags & 2) {
		PSVECScale(part->mVel, part->mVel, part->mDamp);
	}
	PSVECAdd((const Vec*)part->mVel, (const Vec*)part->mPos, (Vec*)part->mPos);
}

// 0x800DF3B0
void fxRibbonUpdate(FxEmitter* fx)
{
	FxParticle* part = (FxParticle*)fx;
	fxEmitterStep(fx, (FxEmitter*)part);
	part = ((FxEmitter*)part)->mKids;
	if (part != NULL && !(fx->mFlags & 0x10000000)) {
		FxExtra* extra = part->mExtra;
		f32* m = fx->mFollow != NULL ? fx->mFollow : fx->mMatrix;
		part->mPos[0] = m[3];
		part->mPos[1] = m[7];
		part->mPos[2] = m[11];
		f32 up[3];
		up[0] = 0.0f;
		up[1] = 0.0f;
		up[2] = 1.0f;
		PSMTXMultVecSR((const float*)m, (const NJS_VECTOR*)up, (NJS_VECTOR*)extra->mB);
		gjMulAddVector(part->mScale, (Vec3f*)extra->mA, (const Vec3f*)part->mPos, (const Vec3f*)extra->mB);
		for (part = part->mNext; part != NULL; part = part->mNext) {
			gjMulAddVector(part->mScale, (Vec3f*)part->mExtra->mA, (const Vec3f*)part->mPos, (const Vec3f*)part->mExtra->mB);
		}
	}
}

// 0x800DF48C
// A ribbon
static inline void fxRibbonVertex(f32* pos, u8* color, f32 s, f32 t)
{
	GXPosition3f32(pos[0], pos[1], pos[2]);
	FX_PIPE8 = color[0];
	FX_PIPE8 = color[1];
	FX_PIPE8 = color[2];
	FX_PIPE8 = color[3];
	FX_PIPE = s;
	FX_PIPE = t;
}

void fxRibbonDraw(Fx* obj)
{
	FxEmitter* fx = (FxEmitter*)obj;
	if (fx->mCount < 2) {
		return;
	}
	f32 u;
	f32 v0;
	f32 v1;
	f32 du;
	if (fx->mFlags & 0x10000) {
		u  = 0.0f;
		du = 1.0f;
	} else {
		u  = 0.0f;
		du = -1.0f;
	}
	if (fx->mFlags & 0x20000) {
		v0 = 1.0f;
		v1 = 0.0f;
	} else {
		v0 = 0.0f;
		v1 = 1.0f;
	}
	fxRibbonDrawBegin(fx, fx);
	fxSetCurrentMtx(0);
	fxSetVtxLayout(5);
	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, (fx->mCount & 0x7FFF) << 1);
	for (FxParticle* part = fx->mKids; part != NULL; part = part->mNext) {
		FxExtra* extra = part->mExtra;
		u8 color[4];
		fxPartColor(part, color);
		GXPosition3f32(part->mPos[0], part->mPos[1], part->mPos[2]);
		u8 c0 = color[0];
		FX_PIPE8 = c0;
		FX_PIPE8 = color[1];
		FX_PIPE8 = color[2];
		FX_PIPE8 = color[3];
		FX_PIPE  = u;
		FX_PIPE  = v0;
		GXPosition3f32(extra->mA[0], extra->mA[1], extra->mA[2]);
		FX_PIPE8 = c0;
		FX_PIPE8 = color[1];
		FX_PIPE8 = color[2];
		FX_PIPE8 = color[3];
		FX_PIPE  = u;
		u += du;
		FX_PIPE  = v1;
	}
	fxRibbonDrawEnd(fx, fx);
}

// 0x800DF620
void fxSweepUpdate(FxEmitter* fx)
{
	fxEmitterStep(fx, fx);
	FxParticle* part = fx->mKids;
	if (part != NULL && !(fx->mFlags & 0x10000000)) {
		FxExtra* extra = part->mExtra;
		f32* m = fx->mFollow != NULL ? fx->mFollow : fx->mMatrix;
		part->mPos[0] = m[3];
		part->mPos[1] = m[7];
		part->mPos[2] = m[11];
		extra->mA[0] = m[0];
		extra->mA[1] = m[1];
		extra->mA[2] = m[2];
		extra->mB[0] = m[4];
		extra->mB[1] = m[5];
		extra->mB[2] = m[6];
		extra->mC[0] = m[8];
		extra->mC[1] = m[9];
		extra->mC[2] = m[10];
	}
}

// 0x800DF6E0
// Loads a matrix per particle from slot 6 up
FxParticle* fxSweepLoadMatrices(FxParticle* part, int count, const void* view)
{
	int slot = 6;
	for (; count > 0; count--) {
		f32 m[3][4];
		FxExtra* extra = part->mExtra;
		m[0][0] = extra->mA[0] * part->mScale;
		m[0][1] = extra->mA[1] * part->mScale;
		m[0][2] = extra->mA[2] * part->mScale;
		m[0][3] = part->mPos[0];
		m[1][0] = extra->mB[0] * part->mScale;
		m[1][1] = extra->mB[1] * part->mScale;
		m[1][2] = extra->mB[2] * part->mScale;
		m[1][3] = part->mPos[1];
		m[2][0] = extra->mC[0] * part->mScale;
		m[2][1] = extra->mC[1] * part->mScale;
		m[2][2] = extra->mC[2] * part->mScale;
		m[2][3] = part->mPos[2];
		PSMTXConcat((const float*)view, (const float*)m, (float*)m);
		GXLoadPosMtxImm((const float*)m, slot);
		slot += 3;
		part = part->mNext;	
	}
	return part;
}

// 0x800DF7D8
// Strips between one matrix and the next
FxParticle* fxSweepDrawSpans(FxParticle* part, int count, const f32* table, int steps)
{
	f32 du  = 1.0f / (f32)(steps - 1);
	int mtx = 6;
	int next = 9;
	for (; count > 0; count--) {
		u8 a[4];
		u8 b[4];
		fxPartColor(part, a);
		fxPartColor(part->mNext, b);
		f32 u = 0.0f;
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, (steps << 1) & 0xFFFF);
		for (int i = 0; i < steps; i++) {
			FX_PIPE8 = mtx;
			FX_PIPE  = table[i * 2];
			FX_PIPE  = table[i * 2 + 1];
			FX_PIPE  = 0.0f;
			FX_PIPE8 = a[0];
			FX_PIPE8 = a[1];
			FX_PIPE8 = a[2];
			FX_PIPE8 = a[3];
			FX_PIPE  = u;
			FX_PIPE  = 0.0f;
			FX_PIPE8 = next;
			FX_PIPE  = table[i * 2];
			FX_PIPE  = table[i * 2 + 1];
			FX_PIPE  = 0.0f;
			FX_PIPE8 = b[0];
			FX_PIPE8 = b[1];
			FX_PIPE8 = b[2];
			FX_PIPE8 = b[3];
			FX_PIPE  = u;
			u += du;
			FX_PIPE  = 1.0f;
		}
		part = part->mNext;
		mtx  = next;
		next += 3;
	}
	return part;
}

// 0x800DF974
// A surface swept along the particles seven spans at a time as that is all the matrix slots hold
void fxSweepDraw(Fx* obj)
{
	FxEmitter* fx = (FxEmitter*)obj;
	if (fx->mCount < 2) {
		return;
	}
	int spans = fx->mCount - 1;
	fxRibbonDrawBegin(fx, fx);
	fxSetVtxLayout(2);
	int steps = *(u8*)&fx->mShapeA;
	f32 table[32][2];
	f32 angle = fx->mShapeC;
	f32 step  = (fx->mShapeB - fx->mShapeC) / (f32)steps;
	for (int i = 0; i < steps + 1; i++) {
		njSinCos((int)(FX_ANGLE * angle), &table[i][0], &table[i][1]);
		angle += step;
	}
	f32 view[3][4];
	njGetMatrix((f32*)view);
	FxParticle* part = fx->mKids;
	int runs = spans / 7;
	for (int i = 0; i < runs; i++) {
		fxSweepLoadMatrices(part, 8, view);
		part = fxSweepDrawSpans(part, 7, table[0], steps + 1);
	}
	int rest = spans % 7;
	fxSweepLoadMatrices(part, rest + 1, view);
	fxSweepDrawSpans(part, rest, table[0], steps + 1);
	fxRibbonDrawEnd(fx, fx);
}

// 0x800DFAF8
void fxRibbonDrawBegin(FxEmitter* fx, FxEmitter* self)
{
	FxTexUse uses[3];
	FxMaterialRef ref;
	ref.mMaterial = self->mMaterial;
	ref.mTexGroup = self->mTexGroup;
	ref.mTexUses  = uses;
	for (int i = 0; i < 3; i++) {
		uses[i].mTex   = self->mTex[i];
		uses[i].mFlags = 0x12;
	}
	fxSetMaterial(fx, &ref);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	fxSetCull(0);
	if (gFxRenderReady != 1) {
		gFxRenderReady = 1;
		GXSetTevColorIn(GX_TEVSTAGE0, sFxTevColorIn[0], sFxTevColorIn[1], sFxTevColorIn[2], sFxTevColorIn[3]);
		GXSetTevAlphaIn(GX_TEVSTAGE0, sFxTevAlphaIn[0], sFxTevAlphaIn[1], sFxTevAlphaIn[2], sFxTevAlphaIn[3]);
	}
}

// 0x800DFBF0
void fxRibbonDrawEnd(FxEmitter*, FxEmitter*)
{
}

// 0x800DFBF4
// Starts a particle with an extra or returns NULL
FxParticle* fxRibbonSpawn(FxEmitter* fx, f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz)
{
	FxParticle* part = fxPartAllocExtra();
	if (part != NULL) {
		fx->mCount++;
		part->mNext = fx->mKids;
		fx->mKids   = part;
		part->mLoops   = 0;
		part->_05   = 0;
		part->mFlags = fx->mPartFlags;
		part->mLife  = fx->mPartLife != 0 ? fx->mPartLife : 0xFFFF;
		part->mScript = fx->mPartScript;
		part->mWait   = fx->mPartScript != NULL ? 1 : 0;
		part->mPc     = 0;
		part->mScaleFrames = 0;
		part->mRotFrames = 0;
		part->mColorFrames = 0;
		part->mAlphaFrames = 0;
		part->mGravity = fx->mGravity;
		part->mDamp    = fx->mDamp;
		part->mScale = part->mScaleTo = fx->mScale;
		part->mRotTo = 0.0f;
		part->mRot = 0.0f;
		part->mVel[0] = vx;
		part->mVel[1] = vy;
		part->mVel[2] = vz;
		part->mPos[0] = x;
		part->mPos[1] = y;
		part->mPos[2] = z;
		part->mColor[3] = 0xFF;
		part->mColor[2] = 0xFF;
		part->mColor[1] = 0xFF;
		part->mColor[0] = 0xFF;
		part->mStep = NULL;
	}
	return part;
}

// 0x800DFD70
FxRig* fxRigNew(FxBank* bank, int index)
{
	FxRig* fx;
	FxRigDesc* desc = (FxRigDesc*)bank->mDescs[index * 2 + 1];
	fx = (FxRig*)gFxRigAlloc(desc->mBoneCount * 0x88 + 0x7C);
	if (fx != NULL) {
		fx->mUpdate = (void (*)(Fx*))fxRigUpdate;
		fx->mDraw   = NULL;
		fx->mRemove = (void (*)(Fx*))fxRigFree;
		fx->mFlags  = desc->mFlags;
		fx->mStop   = desc->mLife != 0 ? desc->mLife : 0xFFFF;
		fx->mWait   = desc->mScript != NULL ? 1 : 0;
		fx->mPc     = 0;
		fx->mScript = desc->mScript;
		fx->mBoneCount = desc->mBoneCount;
		fx->mBones  = (FxBone*)((u8*)fx + 0x7C);
		for (int i = 0; i < fx->mBoneCount; i++) {
			fx->mBones[i].mFlags = 0;
			njUnitMatrix(fx->mBones[i].mLocal[0]);
		}
	}
	return fx;
}

// 0x800DFE74
// Effects hanging from the rig keep their last matrix and finish
void fxRigFree(FxRig* fx)
{
	if (gFxDebugFlags & 1) {
		fxFlagOwner(fx);
	} else if (!(gFxDebugFlags & 2)) {
		if (fx->mFlags & 0x200) {
			fxFlagOwner(fx);
		} else if (fx->mFlags & 0x20000000) {
			for (int i = 0; i < gFxGroupCount; i++) {
				for (Fx* other = gFxGroups[i].mHead; other != NULL; other = other->mNext) {
					if (other->mOwner == fx && other->mFollow != NULL) {
						PSMTXCopy(other->mFollow, other->mMatrix);
						other->mFollow = NULL;
						if (other->mFlags & 2) {
							other->mStop = 1;
						}
					}
				}
			}
		}
	}
	gFxRigRelease(fx);
}

// 0x800DFF5C
// Builds a joint's local matrix
void fxBoneBuild(FxBone* bone)
{
	f32 s, c;
	PSMTXIdentity(bone->mLocal[0]);
	if (bone->mFlags & 1) {
		f32 m[3][4];
		PSMTXTrans((float*)m, bone->mPos[0], bone->mPos[1], bone->mPos[2]);
		PSMTXConcat((const float*)bone->mLocal, (const float*)m, (float*)bone->mLocal);
	}
	if (bone->mFlags & 2) {
		f32 m[3][4];
		if (bone->mRot[2] != 0) {
			njSinCos(bone->mRot[2], &s, &c);
			PSMTXRotTrig((float*)m, 'Z', s, c);
			PSMTXConcat((const float*)bone->mLocal, (const float*)m, (float*)bone->mLocal);
		}
		if (bone->mRot[1] != 0) {
			njSinCos(bone->mRot[1], &s, &c);
			PSMTXRotTrig((float*)m, 'Y', s, c);
			PSMTXConcat((const float*)bone->mLocal, (const float*)m, (float*)bone->mLocal);
		}
		if (bone->mRot[0] != 0) {
			njSinCos(bone->mRot[0], &s, &c);
			PSMTXRotTrig((float*)m, 'X', s, c);
			PSMTXConcat((const float*)bone->mLocal, (const float*)m, (float*)bone->mLocal);
		}
	}
	if (bone->mFlags & 4) {
		f32 m[3][4];
		PSMTXScale((float*)m, bone->mScale[0], bone->mScale[1], bone->mScale[2]);
		PSMTXConcat((const float*)bone->mLocal, (const float*)m, (float*)bone->mLocal);
	}
	bone->mFlags |= 8;
}

// 0x800E00B8
void fxBoneWorld(FxRig* fx, FxBone* bone)
{
	if (bone->mFlags & 8) {
		const f32* m = fx->mFollow != NULL ? fx->mFollow : fx->mMatrix;
		PSMTXConcat(m, (const float*)bone->mLocal, (float*)bone->mWorld);
		bone->mFlags &= ~8;
	}
}

// 0x800E011C
void fxRigUpdate(FxRig* fx)
{
	const f32* base = fx->mFollow != NULL ? fx->mFollow : fx->mMatrix;
	FxBone* bone = fx->mBones;
	for (int i = 0; i < fx->mBoneCount; i++, bone++) {
		if (bone->mFlags != 0) {
			fxBoneBuild(bone);
		}
		PSMTXConcat(base, (const float*)bone->mLocal, (float*)bone->mWorld);
	}

	if (fx->mWait != 0 && --fx->mWait == 0) {
		u16 wait;
		u8* pc;
		pc = fx->mScript + fx->mPc;
		do {
			u32 op = *pc++;
			if (op == 0xFF) {
				wait = 0;
				if (!(fx->mFlags & 2)) {
					fx->mStop = 1;
				}
				break;
			}
			if (op < 0x80) {
				wait = op & 0x1F;
				if (op & 0x20) {
					wait = (wait << 8) + *pc++;
				}
				continue;
			}
			wait = 0;
			u32 code = op;
			if ((op & 0xF8) >= 0xD0 && (op & 0xF8) <= 0xE0) {
				code = op & 0xF8;
			}
			int n;
			int group;
			f32 value;
			s16 effect;
			Fx* made;
			switch (code) {
			case 0xF9:
				group = pc[1];
				bone  = &fx->mBones[pc[0]];
				pc   += 2;
				pc    = fxRead16(pc, (u8*)&effect);
				if (bone->mFlags != 0) {
					fxBoneBuild(bone);
				}
				fxBoneWorld(fx, bone);
				made = fxSpawnAt(group, fx->mBank, effect, bone->mWorld[0]);
				if (made != NULL) {
					fxSetOwner(made, fx);
				}
				break;
			case 0xFA:
				group = pc[1];
				bone  = &fx->mBones[pc[0]];
				pc   += 2;
				pc    = fxRead16(pc, (u8*)&effect);
				if (bone->mFlags != 0) {
					fxBoneBuild(bone);
				}
				fxBoneWorld(fx, bone);
				made = fxSpawnFollow(group, fx->mBank, effect, bone->mWorld[0]);
				if (made != NULL) {
					fx->mFlags |= 0x20000000;
					fxSetOwner(made, fx);
				}
				break;
			case 0xD0:
				n = *pc++;
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mBones[n].mPos[0] = value;
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mBones[n].mPos[1] = value;
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mBones[n].mPos[2] = value;
				}
				fx->mBones[n].mFlags |= 1;
				break;
			case 0xD8:
				n = *pc++;
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mBones[n].mRot[0] = (int)(FX_ANGLE * value);
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mBones[n].mRot[1] = (int)(FX_ANGLE * value);
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mBones[n].mRot[2] = (int)(FX_ANGLE * value);
				}
				fx->mBones[n].mFlags |= 2;
				break;
			case 0xE0:
				n = *pc++;
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mBones[n].mScale[0] = value;
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mBones[n].mScale[1] = value;
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mBones[n].mScale[2] = value;
				}
				fx->mBones[n].mFlags |= 4;
				break;
			case 0xF0:
				fx->mLoops  = *pc++;
				fx->mLoopPc = pc - fx->mScript;
				break;
			case 0xF1:
				if (--fx->mLoops != 0) {
					pc = fx->mScript + fx->mLoopPc;
				}
				break;
			case 0xF2:
				fx->mMarkPc = pc - fx->mScript;
				break;
			case 0xF3:
				pc = fx->mScript + fx->mMarkPc;
				break;
			}
		} while (wait == 0);
		fx->mPc   = pc - fx->mScript;
		fx->mWait = wait;
	}
	if (fx->mStop != 0 && --fx->mStop == 0) {
		fx->mFlags |= 0x40000000;
	}
}

// 0x800E062C
FxEmitter* fxSpriteNew(FxBank* bank, int index)
{
	FxEmitter* fx = (FxEmitter*)fxEmitterAlloc();
	if (fx == NULL) {
		return NULL;
	}
	FxEmitterDesc* desc = (FxEmitterDesc*)bank->mDescs[index * 2 + 1];
	fx->mUpdate = (void (*)(Fx*))fxSpriteUpdate;
	fx->mDraw   = (void (*)(Fx*))fxSpriteDrawKind;
	fx->mRemove = (void (*)(Fx*))fxEmitterFree;
	fx->mFlags |= desc->mFlags;
	fx->mKind = desc->mKind;
	fx->mStop = desc->mLife != 0 ? desc->mLife : 0xFFFF;
	FxEmitter* e = fx;
	e->mRate      = desc->mRate;
	e->mAccum     = 0.0f;
	e->mMax       = desc->mMax;
	e->mPartLife  = desc->mPartLife;
	e->mPartFlags = desc->mPartFlags;
	e->mTex[0]    = desc->mTex[0];
	e->mTex[1]    = desc->mTex[1];
	e->mTexGroup  = desc->mTexGroup;
	e->mMaterial  = desc->mMaterial;
	e->mDir[0] = desc->mDir[0];
	e->mDir[1] = desc->mDir[1];
	e->mDir[2] = desc->mDir[2];
	e->mGravity = desc->mGravity;
	e->mDamp    = desc->mDamp;
	e->mScale   = desc->mScale;
	e->mPartScript = desc->mPartScript;
	e->mCount = 0;
	e->mKids  = NULL;
	e->_A0 = desc->_14;
	e->_A4 = desc->_18;
	fxEmitterReadShape(e, desc);
	return e;
}

// 0x800E077C
// Starts a particle from these values
FxParticle* fxSpriteSpawn(u16 flags, u16 life, u8* script, FxEmitter* fx, f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, f32 gravity, f32 damp,
                        f32 scale)
{
	FxParticle* part = fxPartAlloc();
	if (part != NULL) {
		fx->mCount++;
		part->mNext = fx->mKids;
		fx->mKids   = part;
		part->mLoops   = 0;
		part->_05   = 0;
		part->_09   = 0;
		part->_08   = 0;
		part->mFlags = flags;
		part->mLife  = life != 0 ? life : 0xFFFF;
		part->mScript = script;
		part->mWait   = script != NULL ? 1 : 0;
		part->mPc     = 0;
		part->mExtraCount = 0;
		part->mScaleFrames = 0;
		part->mRotFrames = 0;
		part->mColorFrames = 0;
		part->mAlphaFrames = 0;
		part->mGravity = gravity;
		part->mDamp    = damp;
		part->mScaleTo = scale;
		part->mScale   = scale;
		part->mRotTo = 0.0f;
		part->mRot = 0.0f;
		part->mVel[0] = vx;
		part->mVel[1] = vy;
		part->mVel[2] = vz;
		part->mPos[0] = x;
		part->mPos[1] = y;
		part->mPos[2] = z;
		part->mColor[0] = 0xFF;
		part->mColor[1] = 0xFF;
		part->mColor[2] = 0xFF;
		part->mColor[3] = 0xFF;
		part->mStep = NULL;
		part->mExtra = NULL;
		part->mOwner = fx;
	}
	return part;
}

void (*sFxEmitterUpdaters[2])(FxEmitter*) = { fxRibbonUpdate, fxSweepUpdate };
void (*sFxEmitterDrawers[2])(Fx*) = { fxRibbonDraw, fxSweepDraw };
