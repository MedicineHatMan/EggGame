#include "stage/Stage.h"
#include "engine/Fx.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/FxSystem.h"
#include "engine/FxAnim.h"
#include "engine/FxEmitter.h"
#include "engine/FxSprite.h"
#include "engine/FxParticle.h"
#include "engine/FxParticle2.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "Dolphin/vec.h"
#include "engine/FxShape.h"
#include "core/SdkCalls.h"
struct FxEmitter;
struct FxShape;

union FxPipe {
	u8 u8;
	f32 f32;
};
volatile FxPipe fxPipe : 0xCC008000;
#define FX_PIPE (fxPipe.f32)
#define FX_PIPE8 (fxPipe.u8)
#define FX_ANGLE 10430.38043493439

/** The two rotations of a 3D particle in its extra */
struct FxSpin {
	FxSpin* mNext;
	FxSpin* mPrev;
	f32 mRot[4];  // _08
	f32 mStep[4]; // _18
};

/** A point of a trail in a particle's extras */
struct FxTrail {
	FxTrail* mNext;
	FxTrail* mPrev;
	u16 mLife;   // _08
	u16 _0A;
	f32 mLength; // _0C to the next point
	f32 mTotal;  // _10 along the trail from the head
	f32 mPos[3]; // _14
};

extern "C" {
u8 sFxStripIndex3[3][3] = {
	{ 0, 4, 1 },
	{ 5, 6, 7 },
	{ 2, 8, 3 },
};
u8 sFxStripIndex5[5][5] = {
	{ 0, 9, 4, 10, 1 },
	{ 11, 12, 13, 14, 15 },
	{ 5, 16, 6, 17, 7 },
	{ 18, 19, 20, 21, 22 },
	{ 2, 23, 8, 24, 3 },
};

}

static inline double fxAbsD(double x)
{
	return __fabs(x);
}

static inline f32 fxRandom()
{
	return (f32)rand() * 3.0517578E-5f;
}

// Colours scale and rotation towards their targets
static inline void fxPartColors(FxParticle* part)
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
}

static inline void fxPartRot(FxParticle* part)
{
	if (part->mRotFrames != 0) {
		part->mRot += (part->mRotTo - part->mRot) / (f32)part->mRotFrames;
		part->mRotFrames--;
	}
}

// Particles on a cone turn around it
static inline void fxPartMove(FxParticle* part)
{
	if (part->mFlags & 0x200) {
		FxEmitter* owner = part->mOwner;
		f32 c1, c0, s1, s0;
		njSinCos((int)(FX_ANGLE * part->mGravity), &s0, &c0);
		njSinCos((int)(FX_ANGLE * part->mDamp), &s1, &c1);
		part->mVel[2] += owner->mShapeA;
		f32 radius = fxAbsD(owner->_A0);
		f32 slope  = fxAbsD(owner->_A4);
		radius = part->mVel[2] * (f32)tan(slope) + radius;
		radius *= part->mVel[1];
		part->mVel[0] += owner->mGravity;
		f32 y, x;
		njSinCos((int)(FX_ANGLE * part->mVel[0]), &y, &x);
		x *= radius;
		y *= radius;
		f32 h = part->mVel[2];
		const f32* m = owner->mFollow != NULL ? owner->mFollow : owner->mMatrix;
		part->mPos[0] = m[3] + (x * c1 + h * s1);
		part->mPos[1] = m[7] + (c1 * (h * s0) + (s1 * (-x * s0) + y * c0));
		part->mPos[2] = m[11] + (c1 * (h * c0) + (s1 * (-x * c0) - y * s0));
	} else {
		if (part->mFlags & 1) {
			part->mVel[1] += part->mGravity;
		}
		if (part->mFlags & 2) {
			PSVECScale(part->mVel, part->mVel, part->mDamp);
		}
		PSVECAdd((const Vec*)part->mVel, (const Vec*)part->mPos, (Vec*)part->mPos);
	}
}

// The opcodes 0xB0-0xCF take their low three bits as flags
static inline u32 fxPartCode(u32 op)
{
	u32 code;
	if (!(((code = op & 0xF8) >= 0xB0 && code <= 0xC8) || (code = op & 3) == 0xA0)) {
		code = op;
	}
	return code;
}

static inline void fxSetFlag(FxParticle* part, u32 how, u16 bit)
{
	switch (how) {
	case 0:
		part->mFlags &= ~bit;
		break;
	case 1:
		part->mFlags |= bit;
		break;
	case 2:
		part->mFlags ^= bit;
		break;
	}
}

// Moves a colour channel by up to the signed range either way kept within 0-255
static inline void fxRandomColor(u8* color, u8* range)
{
	f32 c = fxRandom();
	c = (f32)(*(s8*)range << 1) * c;
	c = (f32)*color + c;
	if (c < 0.0f) {
		c = 0.0f;
	}
	if (c > 255.0f) {
		c = 255.0f;
	}
	*color = (int)c;
}

// 0x800E3C9C
// Runs a particle's script up to the next wait
void fxPartRunScript(FxParticle* part)
{
	if (part->mWait != 0 && --part->mWait == 0) {
		u8* pc = part->mScript + part->mPc;
		u16 wait;
		do {
			u32 op = *pc;
			pc++;
			if (op == 0xFF) {
				part->mLife = 1;
				wait = 0;
				break;
			}
			if (op < 0x80) {
				wait = op & 0x1F;
				if (op & 0x20) {
					wait = (wait << 8) + *pc++;
				}
				switch (op & 0xC0) {
				case 0:
					break;
				case 0x40:
					part->_08 = *pc++;
					break;
				}
				continue;
			}
			wait = 0;
			u32 code = fxPartCode(op);
			f32 value;
			f32 extra;
			switch ((u8)code) {
			case 0x80:
				pc = fxReadFrames(pc, (s16*)&part->mRotFrames);
				pc = fxRead32(pc, (u8*)&value);
				part->mRotTo += value;
				if (part->mRotFrames == 0) {
					part->mRot = part->mRotTo;
				}
				break;
			case 0x82:
				part->_05 = *pc++;
				break;
			case 0x87:
				fxSetFlag(part, *pc++, 0x2000);
				break;
			case 0x88:
				fxSetFlag(part, *pc++, 0x4000);
				break;
			case 0x89: {
				int n = *pc++;
				pc = fxRead32(pc, (u8*)&value);
				pc = fxRead32(pc, (u8*)&extra);
				value = n != 0 ? value + (extra * (f32)(int)((f32)(n + 1) * fxRandom())) / (f32)n : value + extra * fxRandom();
				part->mRotTo += value;
				part->mRot += value;
				break;
			}
			case 0xB8:
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&value);
					part->mPos[0] += value;
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&value);
					part->mPos[1] += value;
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&value);
					part->mPos[2] += value;
				}
				break;
			case 0xC0:
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&part->mVel[0]);
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&part->mVel[1]);
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&part->mVel[2]);
				}
				break;
			case 0xC8:
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&value);
					part->mVel[0] += value;
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&value);
					part->mVel[1] += value;
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&value);
					part->mVel[2] += value;
				}
				break;
			case 0xD0:
				pc = fxRead32(pc, (u8*)&part->mGravity);
				break;
			case 0xD2:
				pc = fxRead32(pc, (u8*)&part->mDamp);
				break;
			case 0xD1:
				pc = fxReadFrames(pc, (s16*)&part->mScaleFrames);
				pc = fxRead32(pc, (u8*)&part->mScaleTo);
				if (part->mScaleFrames == 0) {
					part->mScale = part->mScaleTo;
				}
				break;
			case 0xD3:
				pc = fxReadFrames(pc, (s16*)&part->mScaleFrames);
				pc = fxRead32(pc, (u8*)&part->mScaleTo);
				pc = fxRead32(pc, (u8*)&value);
				part->mScaleTo += value * fxRandom();
				if (part->mScaleFrames == 0) {
					part->mScale = part->mScaleTo;
				}
				break;
			case 0x8A:
				pc = fxRead32(pc, (u8*)&value);
				if (value < 0.0f) {
					part->mFlags &= ~0x400;
				} else {
					part->mFlags |= 0x400;
					part->_58 = value;
				}
				break;
			case 0xD4:
				pc = fxRead32(pc, (u8*)&value);
				fxPartDeflect(part, (int)(FX_ANGLE * value));
				break;
			case 0xD5:
				pc = fxRead32(pc, (u8*)&value);
				pc = fxRead32(pc, (u8*)&extra);
				value += extra * fxRandom();
				extra = njScalor((Vec3f*)part->mVel);
				if (extra > 1.0E-5) {
					value /= extra;
					gjScaleVector(value, (Vec3f*)part->mVel, (const Vec3f*)part->mVel);
				}
				break;
			case 0xD6:
				pc = fxRead32(pc, (u8*)&value);
				gjScaleVector(value, (Vec3f*)part->mVel, (const Vec3f*)part->mVel);
				break;
			case 0xD7:
				part->mFlags |= 0x800;
				break;
			case 0xD8:
				part->mFlags &= ~0x800;
				break;
			case 0x85:
				pc = fxReadFrames(pc, (s16*)&part->mColorFrames);
				if (part->mColorFrames == 0) {
					part->mColor[0] = part->mColorTo[0] = pc[0];
					part->mColor[1] = part->mColorTo[1] = pc[1];
					part->mColor[2] = part->mColorTo[2] = pc[2];
					pc += 3;
				} else {
					part->mColorLeft  = part->mColorFrames;
					part->mColorTo[0] = pc[0];
					part->mColorTo[1] = pc[1];
					part->mColorTo[2] = pc[2];
					pc += 3;
				}
				break;
			case 0x86:
				pc = fxReadFrames(pc, (s16*)&part->mAlphaFrames);
				if (part->mAlphaFrames == 0) {
					part->mColor[3] = *pc++;
				} else {
					part->mAlphaLeft  = part->mAlphaFrames;
					part->mColorTo[3] = *pc++;
				}
				break;
			case 0x8B:
				fxRandomColor(&part->mColorTo[0], &pc[0]);
				fxRandomColor(&part->mColorTo[1], &pc[1]);
				fxRandomColor(&part->mColorTo[2], &pc[2]);
				pc += 3;
				if (part->mColorFrames == 0) {
					part->mColor[0] = part->mColorTo[0];
					part->mColor[1] = part->mColorTo[1];
					part->mColor[2] = part->mColorTo[2];
				}
				part->mColorLeft = part->mColorFrames;
				break;
			case 0xF0:
				part->mLoops  = *pc++;
				part->mLoopPc = pc - part->mScript;
				break;
			case 0xF1:
				if (--part->mLoops != 0) {
					pc = part->mScript + part->mLoopPc;
				}
				break;
			case 0xF2:
				part->mMarkPc = pc - part->mScript;
				break;
			case 0xF3:
				pc = part->mScript + part->mMarkPc;
				break;
			case 0xF5: {
				int n = *pc++;
				if (n == 0) {
					part->mStep = NULL;
				} else {
					part->mStep = gFxStepTable[n];
				}
				break;
			}
			}
		} while (wait == 0);
		part->mPc   = pc - part->mScript;
		part->mWait = wait;
	}
}

// 0x800E4668
void fxSpritePartStep(FxParticle* part)
{
	fxPartColors(part);
	fxPartRot(part);
	fxPartRunScript(part);
	if (part->mLife != 0 && --part->mLife == 0) {
		part->mFlags |= 0x100;
	}
	fxPartMove(part);
	if (part->mStep != NULL) {
		part->mStep(part);
	}
}

// 0x800E49C4
// Picks the four texture coordinates of a cell in a sheet of 1 2x2 or 4x4
void fxCellCoords(int mode, int cell, u8* out)
{
	int col, row;
	switch (mode & 0xFF) {
	case 0:
		out[0] = 0;
		out[1] = 1;
		out[2] = 2;
		out[3] = 3;
		break;
	case 1:
		col = cell & 1;
		row = (cell >> 1) & 1;
		out[0] = ((const u8(*)[3])sFxStripIndex3)[row][col];
		out[1] = ((const u8(*)[3])sFxStripIndex3)[row][col + 1];
		out[2] = ((const u8(*)[3])sFxStripIndex3)[row + 1][col];
		out[3] = ((const u8(*)[3])sFxStripIndex3)[row + 1][col + 1];
		break;
	case 2:
		col = cell & 3;
		row = (cell >> 2) & 3;
		out[0] = ((const u8(*)[5])sFxStripIndex5)[row][col];
		out[1] = ((const u8(*)[5])sFxStripIndex5)[row][col + 1];
		out[2] = ((const u8(*)[5])sFxStripIndex5)[row + 1][col];
		out[3] = ((const u8(*)[5])sFxStripIndex5)[row + 1][col + 1];
		break;
	}
}

// The fields every emitter kind reads from its description
static inline void fxEmitterReadDesc(FxEmitter* fx, FxEmitterDesc* desc)
{
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
	fxEmitterReadShape(fx, desc);
}

// 0x800E4A90
FxEmitter* fxQuadNew(FxBank* bank, int index)
{
	FxEmitter* fx = (FxEmitter*)fxEmitterAlloc();
	if (fx == NULL) {
		return NULL;
	}
	FxEmitterDesc* desc = (FxEmitterDesc*)bank->mDescs[index * 2 + 1];
	fx->mUpdate = (void (*)(Fx*))fxQuadUpdate;
	fx->mDraw   = (void (*)(Fx*))fxQuadDraw;
	fx->mRemove = (void (*)(Fx*))fxEmitterFree;
	fx->mFlags |= desc->mFlags;
	fx->mKind = desc->mKind;
	fx->mStop = desc->mLife != 0 ? desc->mLife : 0xFFFF;
	fxEmitterReadDesc(fx, desc);
	return fx;
}

// 0x800E4BE0
void fxQuadUpdate(FxEmitter* fx)
{
	if (fx->mRate < 0.0f) {
		fx->mAccum -= fx->mRate;
	} else {
		fx->mAccum += fx->mRate * fxRandom();
	}
	fxSpawnInWorld(fx, fx, fxQuadSpawn);
	if (fx->mStop != 0 && --fx->mStop == 0) {
		fxEmitterStop(fx, fx);
	}
	fxStepParticles(fx, fxQuadPartStep);
}

static inline void fxColor4u8(const u8 r, const u8 g, const u8 b, const u8 a)
{
	FX_PIPE8 = r;
	FX_PIPE8 = g;
	FX_PIPE8 = b;
	FX_PIPE8 = a;
}

// 0x800E4CAC
// One quad using a loaded matrix
void fxQuadDrawOne(FxParticle* part, int mtx)
{
	f32 size = part->mScale;
	u8 color[4];
	u8 uv[4];
	fxCellCoords(part->_05, part->_08, uv);
	fxPartColor(part, color);
	GXBegin(GX_QUADS, GX_VTXFMT7, 4);
	f32 z = 0.0f;
	FX_PIPE8 = mtx;
	FX_PIPE = -size;
	FX_PIPE = size;
	FX_PIPE = z;
	fxColor4u8(color[0], color[1], color[2], color[3]);
	FX_PIPE8 = uv[0];
	FX_PIPE8 = mtx;
	FX_PIPE = size;
	FX_PIPE = size;
	FX_PIPE = z;
	fxColor4u8(color[0], color[1], color[2], color[3]);
	FX_PIPE8 = uv[1];
	FX_PIPE8 = mtx;
	FX_PIPE = size;
	FX_PIPE = -size;
	FX_PIPE = z;
	fxColor4u8(color[0], color[1], color[2], color[3]);
	FX_PIPE8 = uv[3];
	FX_PIPE8 = mtx;
	FX_PIPE = -size;
	FX_PIPE = -size;
	FX_PIPE = z;
	fxColor4u8(color[0], color[1], color[2], color[3]);
	FX_PIPE8 = uv[2];
}

// 0x800E4DD4
// Quads that turn in 3D eight at a time as that is all the matrix slots hold
void fxQuadDraw(FxEmitter* fx)
{
	u8* state = gFxRenderState;
	if ((u32)fx->mCount == 0) {
		return;
	}
	fxSpriteMaterial(fx, fx, 9);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	fxSetVtxLayout(7);
	if (gFxRenderReady != 1) {
		gFxRenderReady = 1;
		int* in = (int*)(state + 0x270);
		GXSetTevColorIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
		in = (int*)(state + 0x290);
		GXSetTevAlphaIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
	}
	fxSetCoordArray(gFxCoordArray, 8);
	fxSetCull(0);
	FxParticle* list[8];
	int count = 0;
	u32 slot  = 6;
	for (FxParticle* part = fx->mKids; part != NULL; part = part->mNext) {
		f32 m[3][4];
		PSMTXQuat((float*)m, (const GXQuaternion*)(((FxSpin*)part->mExtra)->mRot));
		m[0][3] = part->mPos[0];
		m[1][3] = part->mPos[1];
		m[2][3] = part->mPos[2];
		PSMTXConcat((const float*)state, (const float*)m, (float*)m);
		GXLoadPosMtxImm((const float*)m, slot);
		list[count++] = part;
		slot += 3;
		if (count == 8) {
			FxParticle** p = list;
			for (u32 mtx = 6; mtx != slot; mtx += 3) {
				fxQuadDrawOne(*p, mtx);
				p++;
			}
			count = 0;
			slot  = 6;
		}
	}
	if (count != 0) {
		FxParticle** p = list;
		for (u32 mtx = 6; mtx != slot; mtx += 3) {
			fxQuadDrawOne(*p, mtx);
			p++;
		}
	}
}

// 0x800E4F8C
// A rotation from three angles
void fxQuatFromAngles(f32* q, f32 x, f32 y, f32 z)
{
	f32 cx, cy, cz, sx, sy, sz;
	njSinCos((int)(FX_ANGLE * x) >> 1, &sx, &cx);
	njSinCos((int)(FX_ANGLE * y) >> 1, &sy, &cy);
	njSinCos((int)(FX_ANGLE * z) >> 1, &sz, &cz);
	q[3] = cz * (cx * cy) + sz * (sx * sy);
	q[0] = sz * (-cx * sy) + cz * (sx * cy);
	q[1] = sx * (cy * sz) + cx * (sy * cz);
	q[2] = sy * (-cz * sx) + cy * (sz * cx);
}

// 0x800E5100
// Normalises a vector by the frsqrte estimate and returns the fres estimate of its old length
asm f32 fxNormalize(f32* out, const f32* in)
{
	nofralloc
	lfs f2, 0x0(r4)
	lfs f3, 0x4(r4)
	fmuls f0, f2, f2
	lfs f4, 0x8(r4)
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
}

// 0x800E513C
// A rotation around an axis
void fxQuatFromAxis(f32* q, const f32* axis, f32 angle)
{
	f32 v[3];
	f32 s;
	fxNormalize(v, axis);
	njSinCos((int)(FX_ANGLE * (0.5f * angle)), &s, &q[3]);
	q[0] = s * v[0];
	q[1] = s * v[1];
	q[2] = s * v[2];
}

// 0x800E51D8
// Reads three angles with random additions and turns a rotation by them
u8* fxQuatTurnRandom(u8* pc, f32* rot)
{
	f32 x, y, z, r;
	f32 q[4];
	pc = fxRead32(pc, (u8*)&x);
	pc = fxRead32(pc, (u8*)&r);
	x += r * fxRandom();
	pc = fxRead32(pc, (u8*)&y);
	pc = fxRead32(pc, (u8*)&r);
	y += r * fxRandom();
	pc = fxRead32(pc, (u8*)&z);
	pc = fxRead32(pc, (u8*)&r);
	z += r * fxRandom();
	fxQuatFromAngles(q, x, y, z);
	PSQUATMultiply(q, rot, rot);
	return pc;
}

// 0x800E5310
// A 3D particle with its own script
void fxQuadPartStep(FxParticle* part)
{
	fxPartColors(part);
	fxPartRot(part);
	if (part->mWait != 0 && --part->mWait == 0) {
		u8* pc = part->mScript + part->mPc;
		u16 wait;
		do {
			u32 op = *pc;
			pc++;
			if (op == 0xFF) {
				part->mLife = 1;
				wait = 0;
				break;
			}
			if (op < 0x80) {
				wait = op & 0x1F;
				if (op & 0x20) {
					wait = (wait << 8) + *pc++;
				}
				switch (op & 0xC0) {
				case 0:
					break;
				case 0x40:
					part->_08 = *pc++;
					break;
				}
				continue;
			}
			wait = 0;
			u32 code = fxPartCode(op);
			f32 axis[3];
			f32 value;
			f32 extra;
			switch ((u8)code) {
			case 0x82:
				part->_05 = *pc++;
				break;
			case 0xB8:
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&value);
					part->mPos[0] += value;
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&value);
					part->mPos[1] += value;
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&value);
					part->mPos[2] += value;
				}
				break;
			case 0xC0:
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&part->mVel[0]);
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&part->mVel[1]);
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&part->mVel[2]);
				}
				break;
			case 0xC8:
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&value);
					part->mVel[0] += value;
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&value);
					part->mVel[1] += value;
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&value);
					part->mVel[2] += value;
				}
				break;
			case 0xD0:
				pc = fxRead32(pc, (u8*)&part->mGravity);
				break;
			case 0xD2:
				pc = fxRead32(pc, (u8*)&part->mDamp);
				break;
			case 0xD1:
				pc = fxReadFrames(pc, (s16*)&part->mScaleFrames);
				pc = fxRead32(pc, (u8*)&part->mScaleTo);
				if (part->mScaleFrames == 0) {
					part->mScale = part->mScaleTo;
				}
				break;
			case 0xD3:
				pc = fxReadFrames(pc, (s16*)&part->mScaleFrames);
				pc = fxRead32(pc, (u8*)&part->mScaleTo);
				pc = fxRead32(pc, (u8*)&value);
				part->mScaleTo += value * fxRandom();
				if (part->mScaleFrames == 0) {
					part->mScale = part->mScaleTo;
				}
				break;
			case 0xD4:
				pc = fxRead32(pc, (u8*)&value);
				fxPartDeflect(part, (int)(FX_ANGLE * value));
				break;
			case 0xD5:
				pc = fxRead32(pc, (u8*)&value);
				pc = fxRead32(pc, (u8*)&extra);
				value += extra * fxRandom();
				extra = njScalor((Vec3f*)part->mVel);
				if (extra > 1.0E-5) {
					value /= extra;
					gjScaleVector(value, (Vec3f*)part->mVel, (const Vec3f*)part->mVel);
				}
				break;
			case 0xD6:
				pc = fxRead32(pc, (u8*)&value);
				gjScaleVector(value, (Vec3f*)part->mVel, (const Vec3f*)part->mVel);
				break;
			case 0xD7:
				part->mFlags |= 0x800;
				break;
			case 0xD8:
				part->mFlags &= ~0x800;
				break;
			case 0x85:
				pc = fxReadFrames(pc, (s16*)&part->mColorFrames);
				if (part->mColorFrames == 0) {
					part->mColor[0] = part->mColorTo[0] = pc[0];
					part->mColor[1] = part->mColorTo[1] = pc[1];
					part->mColor[2] = part->mColorTo[2] = pc[2];
					pc += 3;
				} else {
					part->mColorLeft  = part->mColorFrames;
					part->mColorTo[0] = pc[0];
					part->mColorTo[1] = pc[1];
					part->mColorTo[2] = pc[2];
					pc += 3;
				}
				break;
			case 0x86:
				pc = fxReadFrames(pc, (s16*)&part->mAlphaFrames);
				if (part->mAlphaFrames == 0) {
					part->mColor[3] = *pc++;
				} else {
					part->mAlphaLeft  = part->mAlphaFrames;
					part->mColorTo[3] = *pc++;
				}
				break;
			case 0x8B:
				fxRandomColor(&part->mColorTo[0], &pc[0]);
				fxRandomColor(&part->mColorTo[1], &pc[1]);
				fxRandomColor(&part->mColorTo[2], &pc[2]);
				pc += 3;
				if (part->mColorFrames == 0) {
					part->mColor[0] = part->mColorTo[0];
					part->mColor[1] = part->mColorTo[1];
					part->mColor[2] = part->mColorTo[2];
				}
				part->mColorLeft = part->mColorFrames;
				break;
			case 0xE0:
				pc = fxRead32(pc, (u8*)&axis[0]);
				pc = fxRead32(pc, (u8*)&axis[1]);
				pc = fxRead32(pc, (u8*)&axis[2]);
				fxQuatFromAngles(((FxSpin*)part->mExtra)->mRot, axis[0], axis[1], axis[2]);
				break;
			case 0xE1:
				pc = fxQuatTurnRandom(pc, ((FxSpin*)part->mExtra)->mRot);
				break;
			case 0xE2:
				part->mFlags |= 0x1000;
				pc = fxRead32(pc, (u8*)&axis[0]);
				pc = fxRead32(pc, (u8*)&axis[1]);
				pc = fxRead32(pc, (u8*)&axis[2]);
				pc = fxRead32(pc, (u8*)&value);
				fxQuatFromAxis(((FxSpin*)part->mExtra)->mStep, axis, value);
				break;
			case 0xE4:
				part->mFlags &= ~0x1000;
				break;
			case 0xF0:
				part->mLoops  = *pc++;
				part->mLoopPc = pc - part->mScript;
				break;
			case 0xF1:
				if (--part->mLoops != 0) {
					pc = part->mScript + part->mLoopPc;
				}
				break;
			case 0xF2:
				part->mMarkPc = pc - part->mScript;
				break;
			case 0xF3:
				pc = part->mScript + part->mMarkPc;
				break;
			case 0xF5: {
				int n = *pc++;
				if (n == 0) {
					part->mStep = NULL;
				} else {
					part->mStep = gFxStepTable[n];
				}
				break;
			}
			}
		} while (wait == 0);
		part->mPc   = pc - part->mScript;
		part->mWait = wait;
	}
	if (part->mLife != 0 && --part->mLife == 0) {
		part->mFlags |= 0x100;
	}
	fxPartMove(part);
	if (part->mFlags & 0x1000) {
		FxSpin* spin = (FxSpin*)part->mExtra;
		PSQUATMultiply(spin->mStep, spin->mRot, spin->mRot);
	}
	if (part->mStep != NULL) {
		part->mStep(part);
	}
}

// 0x800E5E8C
// Starts a 3D particle with an extra
FxParticle* fxQuadSpawn(u16 flags, u16 life, u8* script, FxEmitter* fx, f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, f32 gravity, f32 damp,
                        f32 scale)
{
	FxParticle* part = fxPartAllocExtra();
	if (part != NULL) {
		fx->mCount++;
		part->mNext = fx->mKids;
		fx->mKids   = part;
		part->mLoops = 0;
		part->_05    = 0;
		part->_09    = 0;
		part->_08    = 0;
		part->mFlags = flags;
		part->mLife  = life != 0 ? life : 0xFFFF;
		part->mScript = script;
		part->mWait   = script != NULL ? 1 : 0;
		part->mPc     = 0;
		part->mScaleFrames = 0;
		part->mRotFrames   = 0;
		part->mColorFrames = 0;
		part->mAlphaFrames = 0;
		part->mGravity = gravity;
		part->mDamp    = damp;
		part->mScaleTo = scale;
		part->mScale   = scale;
		part->mRotTo   = 0.0f;
		part->mRot     = 0.0f;
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
		part->mStep  = NULL;
		part->mOwner = fx;
		FxSpin* spin = (FxSpin*)part->mExtra;
		spin->mRot[2] = 0.0f;
		spin->mRot[1] = 0.0f;
		spin->mRot[0] = 0.0f;
		spin->mRot[3] = 1.0f;
		spin->mStep[2] = 0.0f;
		spin->mStep[1] = 0.0f;
		spin->mStep[0] = 0.0f;
		spin->mStep[3] = 1.0f;
	}
	return part;
}

// 0x800E6068
FxEmitter* fxTrailNew(FxBank* bank, int index)
{
	FxEmitter* fx = (FxEmitter*)fxEmitterAlloc();
	if (fx == NULL) {
		return NULL;
	}
	FxEmitterDesc* desc = (FxEmitterDesc*)bank->mDescs[index * 2 + 1];
	fx->mUpdate = (void (*)(Fx*))fxTrailUpdate;
	fx->mDraw   = (void (*)(Fx*))fxTrailDraw;
	fx->mRemove = (void (*)(Fx*))fxEmitterFree;
	fx->mFlags |= desc->mFlags;
	fx->mKind = desc->mKind;
	fx->mStop = desc->mLife != 0 ? desc->mLife : 0xFFFF;
	fxEmitterReadDesc(fx, desc);
	return fx;
}

// 0x800E61B8
void fxTrailUpdate(FxEmitter* fx)
{
	if (fx->mRate < 0.0f) {
		fx->mAccum -= fx->mRate;
	} else {
		fx->mAccum += fx->mRate * fxRandom();
	}
	fxSpawnInWorld(fx, fx, fxSpriteSpawn);
	if (fx->mStop != 0 && --fx->mStop == 0) {
		fxEmitterStop(fx, fx);
	}
	fxStepParticles(fx, fxTrailPartStep);
}

// 0x800E6284
// Adds a point to a trail
u8* fxTrailAddPoint(u8* pc, FxParticle* part)
{
	FxTrail* head = (FxTrail*)part->mExtra;
	if (head != NULL && head->mLength < 1.0E-5) {
		fxExtraFree(part, part->mExtra);
	}
	FxTrail* point = (FxTrail*)fxExtraAlloc(part);
	if (point == NULL) {
		head = (FxTrail*)part->mExtra;
		if (head != NULL) {
			point = head->mPrev;
			point->mNext = head;
			point->mPrev->mNext = NULL;
			part->mExtra = (FxExtra*)point;
		} else {
			u16 unused;
			return fxReadFrames(pc, (s16*)&unused);
		}
	}
	pc = fxReadFrames(pc, (s16*)&point->mLife);
	if (point->mLife == 0) {
		point->mLife = 0xFFFF;
	}
	point->mTotal  = 0.0f;
	point->mLength = 0.0f;
	point->mPos[0] = part->mPos[0];
	point->mPos[1] = part->mPos[1];
	point->mPos[2] = part->mPos[2];
	return pc;
}

// 0x800E636C
// Runs a trail particle's script
void fxTrailRunScript(FxParticle* part)
{
	if (part->mWait != 0 && --part->mWait == 0) {
		u8* pc = part->mScript + part->mPc;
		u16 wait;
		do {
			u32 op = *pc;
			pc++;
			if (op == 0xFF) {
				part->mLife = 1;
				wait = 0;
				break;
			}
			if (op < 0x80) {
				wait = op & 0x1F;
				if (op & 0x20) {
					wait = (wait << 8) + *pc++;
				}
				switch (op & 0xC0) {
				case 0:
					break;
				case 0x40:
					part->_08 = *pc++;
					break;
				}
				continue;
			}
			wait = 0;
			u32 code = fxPartCode(op);
			f32 value;
			f32 extra;
			switch ((u8)code) {
			case 0x80:
				pc = fxReadFrames(pc, (s16*)&part->mRotFrames);
				pc = fxRead32(pc, (u8*)&value);
				part->mRotTo += value;
				if (part->mRotFrames == 0) {
					part->mRot = part->mRotTo;
				}
				break;
			case 0x87:
				fxSetFlag(part, *pc++, 0x2000);
				break;
			case 0x88:
				fxSetFlag(part, *pc++, 0x4000);
				break;
			case 0x8A:
				pc = fxRead32(pc, (u8*)&value);
				if (value < 0.0f) {
					part->mFlags &= ~0x400;
				} else {
					part->mFlags |= 0x400;
					part->_58 = value;
				}
				break;
			case 0xB8:
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&value);
					part->mPos[0] += value;
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&value);
					part->mPos[1] += value;
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&value);
					part->mPos[2] += value;
				}
				break;
			case 0xC0:
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&part->mVel[0]);
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&part->mVel[1]);
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&part->mVel[2]);
				}
				break;
			case 0xC8:
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&value);
					part->mVel[0] += value;
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&value);
					part->mVel[1] += value;
				}
				if (op & 4) {
					pc = fxRead32(pc, (u8*)&value);
					part->mVel[2] += value;
				}
				break;
			case 0xD0:
				pc = fxRead32(pc, (u8*)&part->mGravity);
				break;
			case 0xD2:
				pc = fxRead32(pc, (u8*)&part->mDamp);
				break;
			case 0xD1:
				pc = fxReadFrames(pc, (s16*)&part->mScaleFrames);
				pc = fxRead32(pc, (u8*)&part->mScaleTo);
				if (part->mScaleFrames == 0) {
					part->mScale = part->mScaleTo;
				}
				break;
			case 0xD3:
				pc = fxReadFrames(pc, (s16*)&part->mScaleFrames);
				pc = fxRead32(pc, (u8*)&part->mScaleTo);
				pc = fxRead32(pc, (u8*)&value);
				part->mScaleTo += value * fxRandom();
				if (part->mScaleFrames == 0) {
					part->mScale = part->mScaleTo;
				}
				break;
			case 0xD4:
				pc = fxRead32(pc, (u8*)&value);
				fxPartDeflect(part, (int)(FX_ANGLE * value));
				break;
			case 0xD5:
				pc = fxRead32(pc, (u8*)&value);
				pc = fxRead32(pc, (u8*)&extra);
				value += extra * fxRandom();
				extra = njScalor((Vec3f*)part->mVel);
				if (extra > 1.0E-5) {
					value /= extra;
					gjScaleVector(value, (Vec3f*)part->mVel, (const Vec3f*)part->mVel);
				}
				break;
			case 0xD6:
				pc = fxRead32(pc, (u8*)&value);
				gjScaleVector(value, (Vec3f*)part->mVel, (const Vec3f*)part->mVel);
				break;
			case 0x85:
				pc = fxReadFrames(pc, (s16*)&part->mColorFrames);
				if (part->mColorFrames == 0) {
					part->mColor[0] = part->mColorTo[0] = pc[0];
					part->mColor[1] = part->mColorTo[1] = pc[1];
					part->mColor[2] = part->mColorTo[2] = pc[2];
					pc += 3;
				} else {
					part->mColorLeft  = part->mColorFrames;
					part->mColorTo[0] = pc[0];
					part->mColorTo[1] = pc[1];
					part->mColorTo[2] = pc[2];
					pc += 3;
				}
				break;
			case 0x86:
				pc = fxReadFrames(pc, (s16*)&part->mAlphaFrames);
				if (part->mAlphaFrames == 0) {
					part->mColor[3] = *pc++;
				} else {
					part->mAlphaLeft  = part->mAlphaFrames;
					part->mColorTo[3] = *pc++;
				}
				break;
			case 0x8B:
				fxRandomColor(&part->mColorTo[0], &pc[0]);
				fxRandomColor(&part->mColorTo[1], &pc[1]);
				fxRandomColor(&part->mColorTo[2], &pc[2]);
				pc += 3;
				if (part->mColorFrames == 0) {
					part->mColor[0] = part->mColorTo[0];
					part->mColor[1] = part->mColorTo[1];
					part->mColor[2] = part->mColorTo[2];
				}
				part->mColorLeft = part->mColorFrames;
				break;
			case 0xF0:
				part->mLoops  = *pc++;
				part->mLoopPc = pc - part->mScript;
				break;
			case 0xF1:
				if (--part->mLoops != 0) {
					pc = part->mScript + part->mLoopPc;
				}
				break;
			case 0xF2:
				part->mMarkPc = pc - part->mScript;
				break;
			case 0xF3:
				pc = part->mScript + part->mMarkPc;
				break;
			case 0xF5: {
				int n = *pc++;
				if (n == 0) {
					part->mStep = NULL;
				} else {
					part->mStep = gFxStepTable[n];
				}
				break;
			}
			case 0xF6:
				pc = fxTrailAddPoint(pc, part);
				break;
			}
		} while (wait == 0);
		part->mPc   = pc - part->mScript;
		part->mWait = wait;
	}
}

// 0x800E6C08
// A trail particle Works out the distances along the trail and drops points that are done
void fxTrailPartStep(FxParticle* part)
{
	f32 d[3];
	fxPartColors(part);
	fxTrailRunScript(part);
	if (part->mLife != 0 && --part->mLife == 0) {
		part->mFlags |= 0x100;
	}
	// The same motion as fxPartMove written out here its locals sit on the stack in the opposite order
	if (part->mFlags & 0x200) {
		FxEmitter* owner = part->mOwner;
		f32 c1, c0, s1, s0;
		njSinCos((int)(FX_ANGLE * part->mGravity), &s0, &c0);
		njSinCos((int)(FX_ANGLE * part->mDamp), &s1, &c1);
		part->mVel[2] += owner->mShapeA;
		f32 radius = fxAbsD(owner->_A0);
		f32 slope  = fxAbsD(owner->_A4);
		radius = part->mVel[2] * (f32)tan(slope) + radius;
		radius *= part->mVel[1];
		part->mVel[0] += owner->mGravity;
		f32 y, x;
		njSinCos((int)(FX_ANGLE * part->mVel[0]), &y, &x);
		x *= radius;
		y *= radius;
		f32 h = part->mVel[2];
		const f32* m = owner->mFollow != NULL ? owner->mFollow : owner->mMatrix;
		part->mPos[0] = m[3] + (x * c1 + h * s1);
		part->mPos[1] = m[7] + (c1 * (h * s0) + (s1 * (-x * s0) + y * c0));
		part->mPos[2] = m[11] + (c1 * (h * c0) + (s1 * (-x * c0) - y * s0));
	} else {
		if (part->mFlags & 1) {
			part->mVel[1] += part->mGravity;
		}
		if (part->mFlags & 2) {
			PSVECScale(part->mVel, part->mVel, part->mDamp);
		}
		PSVECAdd((const Vec*)part->mVel, (const Vec*)part->mPos, (Vec*)part->mPos);
	}
	FxTrail* point = (FxTrail*)part->mExtra;
	if (point != NULL) {
		gjSubVector((Vec3f*)d, (const Vec3f*)part->mPos, (const Vec3f*)point->mPos);
		point->mLength = njScalor((Vec3f*)d);
		f32 total = 0.0f;
		while (point != NULL) {
			total += point->mLength;
			point->mTotal = total;
			if (point->mLife != 0) {
				point->mLife--;
			}
			point = point->mNext;
		}
		while (((FxTrail*)part->mExtra)->mPrev->mLife == 0) {
			fxExtraFree(part, (FxExtra*)((FxTrail*)part->mExtra)->mPrev);
		}
	}
	if (part->mStep != NULL) {
		part->mStep(part);
	}
}

// 0x800E6F98
// Draws the trails a quad between each pair of points
void fxTrailDraw(FxEmitter* fx)
{
	u8* state = gFxRenderState;
	if ((u32)fx->mCount == 0) {
		return;
	}
	fxSetCurrentMtx(3);
	fxSetCull(2);
	fxSpriteMaterial(fx, fx, 9);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	fxSetCoordArray(gFxCoordArray, 8);
	for (FxParticle* part = fx->mKids; part != NULL; part = part->mNext) {
		if (part->mExtra == NULL) {
			continue;
		}
		u8 color[4];
		fxPartColor(part, color);
		if (part->mFlags & 0x400) {
			if (gFxRenderReady != 1) {
				gFxRenderReady = 1;
				int* in = (int*)(state + 0x270);
				GXSetTevColorIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
				in = (int*)(state + 0x290);
				GXSetTevAlphaIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
			}
			fxSetVtxLayout(5);
		} else {
			if (gFxRenderReady != 0) {
				gFxRenderReady = 0;
				int* in = (int*)(state + 0x260);
				GXSetTevColorIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
				in = (int*)(state + 0x280);
				GXSetTevAlphaIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
			}
			fxSetVtxLayout(0);
			u32 tev = *(u32*)color;
			fxSetTevColor((u8*)&tev);
		}
		f32 walked = 0.0f;
		f32 length = part->mRot;
		f32 a[3];
		PSMTXMultVec((const float*)state, (const NJS_VECTOR*)part->mPos, (NJS_VECTOR*)a);
		for (FxTrail* point = (FxTrail*)part->mExtra; point != NULL; point = point->mNext) {
			if (walked >= length) {
				break;
			}
			f32 b[3];
			f32 s, c;
			PSMTXMultVec((const float*)state, (const NJS_VECTOR*)point->mPos, (NJS_VECTOR*)b);
			if (point->mTotal > length) {
				c = (length - walked) / point->mLength;
				b[0] = c * (b[0] - a[0]) + a[0];
				b[1] = c * (b[1] - a[1]) + a[1];
				b[2] = c * (b[2] - a[2]) + a[2];
			}
			njSinCos((int)(FX_ANGLE * (f32)atan2(b[1] - a[1], b[0] - a[0])) + 0x4000, &s, &c);
			c *= part->mScale;
			s *= part->mScale;
			if (!(part->mFlags & 0x400)) {
				if (point == (FxTrail*)part->mExtra) {
					GXBegin(GX_QUADS, GX_VTXFMT7, 4);
					FX_PIPE  = a[0] - c;
					FX_PIPE  = a[1] - s;
					FX_PIPE  = a[2];
					FX_PIPE8 = 0;
					FX_PIPE  = a[0] + c;
					FX_PIPE  = a[1] + s;
					FX_PIPE  = a[2];
					FX_PIPE8 = 1;
					FX_PIPE  = b[0] + c;
					FX_PIPE  = b[1] + s;
					FX_PIPE  = b[2];
					FX_PIPE8 = 7;
					FX_PIPE  = b[0] - c;
					FX_PIPE  = b[1] - s;
					FX_PIPE  = b[2];
					FX_PIPE8 = 5;
				} else {
					GXBegin(GX_QUADS, GX_VTXFMT7, 4);
					FX_PIPE  = a[0] - c;
					FX_PIPE  = a[1] - s;
					FX_PIPE  = a[2];
					FX_PIPE8 = 5;
					FX_PIPE  = a[0] + c;
					FX_PIPE  = a[1] + s;
					FX_PIPE  = a[2];
					FX_PIPE8 = 7;
					FX_PIPE  = b[0] + c;
					FX_PIPE  = b[1] + s;
					FX_PIPE  = b[2];
					FX_PIPE8 = 3;
					FX_PIPE  = b[0] - c;
					FX_PIPE  = b[1] - s;
					FX_PIPE  = b[2];
					FX_PIPE8 = 2;
				}
			}
			a[0] = b[0];
			a[1] = b[1];
			a[2] = b[2];
			walked = point->mTotal;
		}
	}
}
