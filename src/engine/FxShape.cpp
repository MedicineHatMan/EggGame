#include "stage/Stage.h"
#include "engine/Fx.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/FxSystem.h"
#include "engine/FxAnim.h"
#include "engine/FxShape.h"
#include "Ninja/ninja.h"
#include "engine/FxEmitter.h"

#define FX_PIPE  (*(volatile f32*)0xCC008000)
#define FX_PIPE8 (*(volatile u8*)0xCC008000)

#define FX_ANGLE 10430.38043493439

struct FxShapeParams {
	u8 mSlices; // _00
	u8 mSegs;   // _01
	u8 _02[2];
	f32 mFrom; // _04
	f32 mTo;   // _08
};

/** Texture matrix setup for a material */
struct FxMaterial {
	u8 _00[9];
	u8 mTexMtxCount; // _09
	u8 _0A[6];
	int* mTexMtxTypes; // _10
};

struct FxArray {
	u16 mType; // _00
	s16 _02;
	void* mData; // _04
};

struct FxModelPart {
	int _00;
	int mStrips;   // _04
	s16* mLengths; // _08
	u8* mIndices;  // _0C
};

struct FxModelEntry {
	int mType;
	FxModelPart* mPart;
};

struct FxModel {
	u32 _00[4];
	FxArray mPositions;     // _10
	FxArray mNormals;       // _18
	FxArray mTexCoords;     // _20
	int mEntryCount;        // _28
	FxModelEntry* mEntries; // _2C
};

extern "C" {
u32 sFxBoneMatrixSlots[] = {
	0x0000001E, 0x00000021, 0x0000003C, 0x00000040, 0x00000043, 0x0000007D,
};
}

static inline f32 fxRandom()
{
	return (f32)rand() * 3.0517578E-5f;
}

#define SHAPE_RAND_COLOR(i)                     \
	value = (f32)((s8)pc[i] << 1) * fxRandom(); \
	c     = (f32)fx->mColorTo[i] + value;       \
	if (c < 0.0f) {                             \
		c = 0.0f;                               \
	}                                           \
	if (c > 255.0f) {                           \
		c = 255.0f;                             \
	}                                           \
	fx->mColorTo[i] = (int)c

// 0x800DC4EC
// Steps the timers and runs the script up to the next wait
void fxShapeRunScript(FxShape* fx)
{
	FxDesc* desc = fx->mDesc;

	if (fx->mColorFrames != 0) {
		if (--fx->mColorLeft == 0) {
			fx->mColorFrames = 0;
			fx->mColor[0]    = fx->mColorTo[0];
			fx->mColor[1]    = fx->mColorTo[1];
			fx->mColor[2]    = fx->mColorTo[2];
		}
	}
	if (fx->mAlphaFrames != 0) {
		if (--fx->mAlphaLeft == 0) {
			fx->mAlphaFrames = 0;
			fx->mColor[3]    = fx->mColorTo[3];
		}
	}
	if (fx->mRotFrames != 0) {
		fx->mRot += (fx->mRotTo - fx->mRot) / (f32)fx->mRotFrames;
		fx->mRotFrames--;
	}
	for (int i = 0; i < 2; i++) {
		if (fx->mUvFrames[i] != 0) {
			f32 step = 1.0f / (f32)fx->mUvFrames[i];
			fx->mUv[i][0] += step * (fx->mUvTo[i][0] - fx->mUv[i][0]);
			fx->mUv[i][1] += step * (fx->mUvTo[i][1] - fx->mUv[i][1]);
			fx->mUvFrames[i]--;
		}
	}

	if (fx->mWait != 0 && --fx->mWait == 0) {
		u8* pc = desc->mScript + fx->mPc;
		u16 wait;
		f32 value;
		f32 extra;
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
				switch (op & 0xC0) {
				case 0:
					break;
				case 0x40:
					pc += 2;
					break;
				}
				continue;
			}
			wait = 0;
			u32 code;
			if (!(((code = op & 0xF8) >= 0xB0 && code <= 0xC8) || (code = op & 0xFC) == 0xA0)) {
				code = op;
			}
			int n;
			FxKey* key;
			FxWave* wave;
			switch ((u8)code) {
			case 0x81:
				fx->mTex[pc[0]] = pc[1];
				pc += 2;
				break;
			case 0x83:
				fxShow(fx);
				break;
			case 0x84:
				fxHide(fx);
				break;
			case 0x80:
				pc = fxReadFrames(pc, (s16*)&fx->mRotFrames);
				pc = fxRead32(pc, (u8*)&value);
				fx->mRotTo += value;
				if (fx->mRotFrames == 0) {
					fx->mRot = fx->mRotTo;
				}
				break;
			case 0x89:
				n  = *pc++;
				pc = fxRead32(pc, (u8*)&value);
				pc = fxRead32(pc, (u8*)&extra);
				if (n != 0) {
					value += (extra * (f32)(int)((f32)(int)(n + 1) * fxRandom())) / (f32)(int)n;
				} else {
					value += extra * fxRandom();
				}
				fx->mRotTo += value;
				fx->mRot += value;
				break;
			case 0x87:
				switch (*pc++) {
				case 0:
					fx->mFlags &= ~0x10000;
					break;
				case 1:
					fx->mFlags |= 0x10000;
					break;
				case 2:
					fx->mFlags ^= 0x10000;
					break;
				}
				break;
			case 0x88:
				switch (*pc++) {
				case 0:
					fx->mFlags &= ~0x20000;
					break;
				case 1:
					fx->mFlags |= 0x20000;
					break;
				case 2:
					fx->mFlags ^= 0x20000;
					break;
				}
				break;
			case 0x8C:
				pc = fxRead32(pc, (u8*)&fx->mU);
				if (0.0f == fx->mU) {
					fx->mFlags |= 0x40000;
					fx->mU = 1.0f;
				} else {
					fx->mFlags &= ~0x40000;
				}
				break;
			case 0x8D:
				pc = fxRead32(pc, (u8*)&fx->mV);
				if (0.0f == fx->mV) {
					fx->mFlags |= 0x80000;
					fx->mV = 1.0f;
				} else {
					fx->mFlags &= ~0x80000;
				}
				break;
			case 0x90:
				n                    = *pc++;
				fx->mKeys[n].mFrames = 0;
				fx->mKeys[n].mFrom   = fx->mKeys[n].mTo;
				fx->mKeys[n].mTanTo  = 0.0f;
				pc                   = fxRead32(pc, (u8*)&fx->mKeys[n].mTo);
				break;
			case 0x91:
				n                   = *pc++;
				fx->mKeys[n].mType  = 0;
				fx->mKeys[n].mFrom  = fx->mKeys[n].mTo;
				fx->mKeys[n].mTanTo = 0.0f;
				pc                  = fxReadFrames(pc, (s16*)&fx->mKeys[n].mFrames);
				pc                  = fxRead32(pc, (u8*)&fx->mKeys[n].mTo);
				fx->mKeys[n].mFrame = fx->mKeys[n].mFrames;
				break;
			case 0x92:
				n                     = *pc++;
				fx->mKeys[n].mType    = 1;
				fx->mKeys[n].mFrom    = fx->mKeys[n].mTo;
				fx->mKeys[n].mTanFrom = fx->mKeys[n].mTanTo;
				fx->mKeys[n].mTanTo   = 0.0f;
				pc                    = fxReadFrames(pc, (s16*)&fx->mKeys[n].mFrames);
				pc                    = fxRead32(pc, (u8*)&fx->mKeys[n].mTo);
				fx->mKeys[n].mFrame   = fx->mKeys[n].mFrames;
				break;
			case 0x93:
				n                     = *pc++;
				fx->mKeys[n].mType    = 1;
				fx->mKeys[n].mFrom    = fx->mKeys[n].mTo;
				fx->mKeys[n].mTanFrom = fx->mKeys[n].mTanTo;
				pc                    = fxReadFrames(pc, (s16*)&fx->mKeys[n].mFrames);
				pc                    = fxRead32(pc, (u8*)&fx->mKeys[n].mTo);
				pc                    = fxRead32(pc, (u8*)&fx->mKeys[n].mTanTo);
				fx->mKeys[n].mFrame   = fx->mKeys[n].mFrames;
				break;
			case 0x94:
				n  = *pc++;
				pc = fxRead32(pc, (u8*)&fx->mKeys[n].mTanTo);
				break;
			case 0x98:
				fx->mWaves[(u8)(int)pc[0]].mType = pc[1];
				pc += 2;
				break;
			case 0x99:
				n                   = *pc++;
				pc                  = fxRead32(pc, (u8*)&value);
				fx->mWaves[n].mStep = (int)(FX_ANGLE * value);
				break;
			case 0x9A:
				n  = *pc++;
				pc = fxReadFrames(pc, (s16*)&fx->mWaves[(u8)n].mFrames);
				if (fx->mWaves[(u8)n].mFrames == 0) {
					pc = fxRead32(pc, (u8*)&fx->mWaves[(u8)n].mAmp);
				} else {
					pc = fxRead32(pc, (u8*)&fx->mWaves[(u8)n].mAmpTo);
				}
				break;
			case 0x85:
				pc              = fxReadFrames(pc, (s16*)&fx->mColorFrames);
				fx->mColorTo[0] = fx->mColor[0];
				fx->mColorTo[1] = fx->mColor[1];
				fx->mColorTo[2] = fx->mColor[2];
				fx->mColorTo[0] = pc[0];
				fx->mColorTo[1] = pc[1];
				fx->mColorTo[2] = pc[2];
				pc += 3;
				if (fx->mColorFrames == 0) {
					fx->mColor[0] = fx->mColorTo[0];
					fx->mColor[1] = fx->mColorTo[1];
					fx->mColor[2] = fx->mColorTo[2];
				} else {
					fx->mColorLeft = fx->mColorFrames;
				}
				break;
			case 0x86:
				pc = fxReadFrames(pc, (s16*)&fx->mAlphaFrames);
				if (fx->mAlphaFrames == 0) {
					fx->mColor[3] = *pc++;
				} else {
					fx->mAlphaLeft  = fx->mAlphaFrames;
					fx->mColorTo[3] = *pc++;
				}
				break;
			case 0x8B: {
				f32 c;
				SHAPE_RAND_COLOR(0);
				SHAPE_RAND_COLOR(1);
				SHAPE_RAND_COLOR(2);
				pc += 3;
				if (fx->mColorFrames == 0) {
					fx->mColor[0] = fx->mColorTo[0];
					fx->mColor[1] = fx->mColorTo[1];
					fx->mColor[2] = fx->mColorTo[2];
				}
				fx->mColorLeft = fx->mColorFrames;
				break;
			}
			case 0xA0:
				n  = *pc++;
				pc = fxReadCount(pc, &fx->mUvFrames[n]);
				if (op & 1) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mUvTo[n][0] += value;
				}
				if (op & 2) {
					pc = fxRead32(pc, (u8*)&value);
					fx->mUvTo[n][1] += value;
				}
				if (fx->mUvFrames[n] == 0) {
					fx->mUv[n][0] = fx->mUvTo[n][0];
					fx->mUv[n][1] = fx->mUvTo[n][1];
				}
				break;
			case 0xF0:
				fx->mLoops  = *pc++;
				fx->mLoopPc = pc - desc->mScript;
				break;
			case 0xF1:
				if (--fx->mLoops != 0) {
					pc = desc->mScript + fx->mLoopPc;
				}
				break;
			case 0xF2:
				fx->mMarkPc = pc - desc->mScript;
				break;
			case 0xF3:
				pc = desc->mScript + fx->mMarkPc;
				break;
			case 0xF4:
				n  = *pc++;
				pc = fxRead32(pc, (u8*)&value);
				if (fx->mEvent != NULL) {
					fx->mEvent(fx, n & 0xFF, value);
				}
				break;
			}
		} while (wait == 0);
		fx->mPc   = pc - desc->mScript;
		fx->mWait = wait;
	}

	if (fx->mStop != 0 && --fx->mStop == 0) {
		fx->mFlags |= 0x40000000;
	}
}

// 0x800DD03C
void fxInitSphere(FxShape* fx, FxShape* self, void* params)
{
	fxChannelsInit(self, (FxChannel*)((u8*)params + 0xC), 1);
	fx->mUpdate = (void (*)(Fx*))fxUpdateSphere;
	fx->mDraw   = (void (*)(Fx*))fxDrawSphere;
}

// 0x800DD090
void fxInitDome(FxShape* fx, FxShape* self, void* params)
{
	fxChannelsInit(self, (FxChannel*)((u8*)params + 0xC), 1);
	fx->mUpdate = (void (*)(Fx*))fxUpdateDome;
	fx->mDraw   = (void (*)(Fx*))fxDrawDome;
}

// 0x800DD0E4
void fxInitCylinder(FxShape* fx, FxShape* self, void* params)
{
	fxChannelsInit(self, (FxChannel*)((u8*)params + 0xC), 3);
	fx->mUpdate = (void (*)(Fx*))fxUpdateCylinder;
	fx->mDraw   = (void (*)(Fx*))fxDrawCylinder;
}

// 0x800DD138
void fxInitCone(FxShape* fx, FxShape* self, void* params)
{
	fxChannelsInit(self, (FxChannel*)((u8*)params + 0xC), 4);
	fx->mUpdate = (void (*)(Fx*))fxUpdateCone;
	fx->mDraw   = (void (*)(Fx*))fxDrawCone;
}

// 0x800DD18C
void fxInitBlades(FxShape* fx, FxShape* self, void* params)
{
	fxChannelsInit(self, (FxChannel*)((u8*)params + 0xC), 4);
	fx->mUpdate = (void (*)(Fx*))fxUpdateBlades;
	fx->mDraw   = fxDrawBlades;
}

// 0x800DD1E0
void fxInitRing(FxShape* fx, FxShape* self, void* params)
{
	fxChannelsInit(self, (FxChannel*)((u8*)params + 0xC), 2);
	fx->mUpdate = (void (*)(Fx*))fxUpdateRing;
	fx->mDraw   = (void (*)(Fx*))fxDrawRing;
}

// 0x800DD234
void fxInitBentBlades(FxShape* fx, FxShape* self, void* params)
{
	fxChannelsInit(self, (FxChannel*)((u8*)params + 0x14), 4);
	fx->mUpdate = (void (*)(Fx*))fxUpdateBentBlades;
	fx->mDraw   = (void (*)(Fx*))fxDrawBentBlades;
}

// 0x800DD288
void fxInitModelShape(FxShape* fx, FxShape* self, void* params)
{
	fxChannelsInit(self, (FxChannel*)((u8*)params + 4), 6);
	fx->mUpdate = (void (*)(Fx*))fxUpdateModelShape;
	fx->mDraw   = fxDrawModelShape;
}

// 0x800DD2DC
void fxUpdateSphere(FxShape* fx)
{
	fxChannelsUpdate(fx, (FxChannel*)((u8*)fx->mDesc->mParams + 0xC), 1);
	fxShapeRunScript(fx);
}

// 0x800DD320
void fxUpdateDome(FxShape* fx)
{
	fxChannelsUpdate(fx, (FxChannel*)((u8*)fx->mDesc->mParams + 0xC), 1);
	fxShapeRunScript(fx);
}

// 0x800DD364
void fxUpdateCylinder(FxShape* fx)
{
	fxChannelsUpdate(fx, (FxChannel*)((u8*)fx->mDesc->mParams + 0xC), 3);
	fxShapeRunScript(fx);
}

// 0x800DD3A8
void fxUpdateCone(FxShape* fx)
{
	fxChannelsUpdate(fx, (FxChannel*)((u8*)fx->mDesc->mParams + 0xC), 4);
	fxShapeRunScript(fx);
}

// 0x800DD3EC
void fxUpdateBlades(FxShape* fx)
{
	fxChannelsUpdate(fx, (FxChannel*)((u8*)fx->mDesc->mParams + 0xC), 4);
	fxShapeRunScript(fx);
}

// 0x800DD430
void fxUpdateRing(FxShape* fx)
{
	fxChannelsUpdate(fx, (FxChannel*)((u8*)fx->mDesc->mParams + 0xC), 2);
	fxShapeRunScript(fx);
}

// 0x800DD474
void fxUpdateBentBlades(FxShape* fx)
{
	fxChannelsUpdate(fx, (FxChannel*)((u8*)fx->mDesc->mParams + 0x14), 4);
	fxShapeRunScript(fx);
}

// 0x800DD4B8
void fxUpdateModelShape(FxShape* fx)
{
	fxChannelsUpdate(fx, (FxChannel*)((u8*)fx->mDesc->mParams + 4), 6);
	fxShapeRunScript(fx);
}

// 0x800DD4FC
// The colour now part way between the two
void fxShapeColor(FxShape* fx, u8* out)
{
	if (fx->mColorFrames != 0) {
		int t  = (fx->mColorLeft << 16) / fx->mColorFrames;
		out[0] = ((fx->mColorTo[0] << 16) + t * (fx->mColor[0] - fx->mColorTo[0])) >> 16;
		out[1] = ((fx->mColorTo[1] << 16) + t * (fx->mColor[1] - fx->mColorTo[1])) >> 16;
		out[2] = ((fx->mColorTo[2] << 16) + t * (fx->mColor[2] - fx->mColorTo[2])) >> 16;
	} else {
		out[0] = fx->mColor[0];
		out[1] = fx->mColor[1];
		out[2] = fx->mColor[2];
	}
	if (fx->mAlphaFrames != 0) {
		int t  = (fx->mAlphaLeft << 16) / fx->mAlphaFrames;
		out[3] = ((fx->mColorTo[3] << 16) + t * (fx->mColor[3] - fx->mColorTo[3])) >> 16;
	} else {
		out[3] = fx->mColor[3];
	}
}

// 0x800DD5D8
// Loads the material's texture matrices
void fxLoadTexMatrices(Fx* fx, int material, const f32* view, f32* uv)
{
	FxMaterialEntry* mats = gFxBanks[fx->mBank]->mMaterials;
	if ((int)mats[material].mType == 1) {
		FxMaterial* mat = (FxMaterial*)mats[material].mData;
		for (int i = 0; i < mat->mTexMtxCount; i++) {
			f32 m[3][4];
			switch (mat->mTexMtxTypes[i]) {
			case 0:
				PSMTXIdentity(m[0]);
				m[0][3] = uv[i * 2];
				m[1][3] = uv[i * 2 + 1];
				GXLoadTexMtxImm((const f32*)m, sFxBoneMatrixSlots[i], GX_MTX3x4);
				break;
			case 2:
				PSMTXConcat((const f32*)sFxViewMtx, view, (f32*)m);
				GXLoadTexMtxImm((const f32*)m, sFxBoneMatrixSlots[i], GX_MTX3x4);
				break;
			case 3:
				PSMTXIdentity(m[0]);
				GXLoadTexMtxImm((const f32*)m, sFxBoneMatrixSlots[i], GX_MTX3x4);
				break;
			}
		}
	}
}

// 0x800DD710
// Everything a shape needs before its vertices
void fxShapeDrawBegin(Fx* fx, FxShape* shape, FxDesc*)
{
	u8* state = gFxRenderState;
	u8 color[4];
	f32 m[3][4];
	if (fx->mFollow != NULL) {
		PSMTXConcat((const f32*)state, fx->mFollow, (f32*)m);
	} else {
		PSMTXConcat((const f32*)state, (const f32*)fx->mMatrix, (f32*)m);
	}
	if (0.0f != shape->mRot) {
		f32 sin, cos;
		f32 r[3][4];
		njSinCos((int)(FX_ANGLE * shape->mRot), &sin, &cos);
		PSMTXRotTrig(r[0], 'Z', sin, cos);
		PSMTXConcat((const f32*)m, (const f32*)r, (f32*)m);
	}
	GXLoadPosMtxImm((const f32*)m, GX_PNMTX2);
	fxSetCurrentMtx(6);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

	FxTexUse uses[3];
	FxMaterialRef ref;
	ref.mMaterial = shape->mDesc->mMaterial;
	ref.mTexGroup = shape->mDesc->mTexGroup;
	ref.mTexUses  = uses;
	u8 flags      = 0;
	if (fx->mFlags & 0x40000) {
		flags |= 1;
	} else {
		flags |= 2;
	}
	if (fx->mFlags & 0x80000) {
		flags |= 8;
	} else {
		flags |= 0x10;
	}
	for (int i = 0; i < 3; i++) {
		uses[i].mTex   = shape->mTex[i];
		uses[i].mFlags = flags;
	}
	fxSetMaterial(fx, &ref);
	fxLoadTexMatrices(fx, shape->mDesc->mMaterial, m[0], shape->mUv[0]);
	if (fx->mFlags & 1) {
		fxSetCull(0);
	} else {
		fxSetCull(2);
	}
	fxSetVtxLayout(1);
	fxShapeColor(shape, color);
	u32 tev = *(u32*)color;
	fxSetTevColor((u8*)&tev);
	if (gFxRenderReady != 0) {
		gFxRenderReady = 0;
		int* in        = (int*)(state + 0x260);
		GXSetTevColorIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
		in = (int*)(state + 0x280);
		GXSetTevAlphaIn(GX_TEVSTAGE0, in[0], in[1], in[2], in[3]);
	}
}

// 0x800DD940
void fxShapeDrawEnd(Fx*, FxShape*)
{
}

// 0x800DD944
// A sphere
void fxDrawSphere(FxShape* fx)
{
	FxShapeParams* params = (FxShapeParams*)fx->mDesc->mParams;
	f32 ring;
	f32 z;
	fxShapeDrawBegin(fx, fx, fx->mDesc);
	f32 radius = fx->mValues[0];
	int verts  = (params->mSegs + 1) << 1;
	int from   = (int)(FX_ANGLE * params->mFrom);
	int to     = (int)(FX_ANGLE * params->mTo);
	if (from == to) {
		to = from + 0x10000;
	}
	int step = (to - from) / params->mSegs;
	f32 u0   = 0.0f;
	f32 u1   = fx->mU;
	f32 v    = 0.0f;
	f32 v1   = fx->mV;
	int late = 0x8000 / params->mSlices;
	if (fx->mFlags & 0x10000) {
		u0 = fx->mU;
		u1 = 0.0f;
	}
	if (fx->mFlags & 0x20000) {
		v  = v1;
		v1 = 0.0f;
	}
	z         = radius;
	ring      = 0.0f;
	f32 du    = (u1 - u0) / (f32)params->mSegs;
	f32 dv    = (v1 - v) / (f32)params->mSlices;
	int angle = 0;
	f32 s, c;
	for (int i = params->mSlices; i > 0; i--) {
		f32 vn = v + dv;
		f32 sin, cos;
		njSinCos(angle + late, &sin, &cos);
		f32 ringN = sin * radius;
		f32 zN    = cos * radius;
		f32 u     = 0.0f;
		int a     = from;
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, (u16)verts);
		for (int j = params->mSegs + 1; j != 0; j--) {
			njSinCos(a, &s, &c);
			a += step;
			f32 cc  = c;
			FX_PIPE = ringN * cc;
			f32 ss  = s;
			FX_PIPE = ringN * ss;
			FX_PIPE = zN;
			FX_PIPE = u;
			FX_PIPE = vn;
			FX_PIPE = ring * cc;
			FX_PIPE = ring * ss;
			FX_PIPE = z;
			FX_PIPE = u;
			u += du;
			FX_PIPE = v;
		}
		v = vn;
		angle += late;
		ring = ringN;
		z    = zN;
	}
	fxShapeDrawEnd(fx, fx);
}

// 0x800DDBD4
// A half sphere
void fxDrawDome(FxShape* fx)
{
	FxShapeParams* params = (FxShapeParams*)fx->mDesc->mParams;
	f32 ring;
	f32 z;
	fxShapeDrawBegin(fx, fx, fx->mDesc);
	f32 radius = fx->mValues[0];
	int verts  = (params->mSegs + 1) << 1;
	int from   = (int)(FX_ANGLE * params->mFrom);
	int to     = (int)(FX_ANGLE * params->mTo);
	if (from == to) {
		to = from + 0x10000;
	}
	int step = (to - from) / params->mSegs;
	f32 u0   = 0.0f;
	f32 u1   = fx->mU;
	f32 v    = 0.0f;
	f32 v1   = fx->mV;
	int late = 0x4000 / params->mSlices;
	if (fx->mFlags & 0x10000) {
		u0 = fx->mU;
		u1 = 0.0f;
	}
	if (fx->mFlags & 0x20000) {
		v  = v1;
		v1 = 0.0f;
	}
	z         = radius;
	ring      = 0.0f;
	f32 du    = (u1 - u0) / (f32)params->mSegs;
	f32 dv    = (v1 - v) / (f32)params->mSlices;
	int angle = 0;
	f32 s, c;
	for (int i = params->mSlices; i != 0; i--) {
		f32 vn = v + dv;
		f32 sin, cos;
		njSinCos(angle + late, &sin, &cos);
		f32 ringN = sin * radius;
		f32 zN    = cos * radius;
		f32 u     = 0.0f;
		int a     = 0;
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, (u16)verts);
		for (int j = params->mSegs + 1; j != 0; j--) {
			njSinCos(a, &s, &c);
			a += step;
			f32 cc  = c;
			FX_PIPE = ringN * cc;
			f32 ss  = s;
			FX_PIPE = ringN * ss;
			FX_PIPE = zN;
			FX_PIPE = u;
			FX_PIPE = vn;
			FX_PIPE = ring * cc;
			FX_PIPE = ring * ss;
			FX_PIPE = z;
			FX_PIPE = u;
			u += du;
			FX_PIPE = v;
		}
		v = vn;
		angle += late;
		ring = ringN;
		z    = zN;
	}
	fxShapeDrawEnd(fx, fx);
}

// 0x800DDE60
// A cylinder
void fxDrawCylinder(FxShape* fx)
{
	FxShapeParams* params = (FxShapeParams*)fx->mDesc->mParams;
	int i;
	int a;
	fxShapeDrawBegin(fx, fx, fx->mDesc);
	u32 slices = params->mSlices;
	int count  = (slices + 1) << 1;
	f32 radius = fx->mValues[0];
	f32 z1     = fx->mValues[1];
	f32 z0     = fx->mValues[2];
	int from   = (int)(FX_ANGLE * params->mFrom);
	int to     = (int)(FX_ANGLE * params->mTo);
	if (from == to) {
		to = from + 0x10000;
	}
	int step = (to - from) / (int)slices;
	f32 u    = 0.0f;
	f32 u1   = fx->mU;
	f32 v0   = 0.0f;
	f32 v1   = fx->mV;
	a        = from;
	if (fx->mFlags & 0x10000) {
		u  = fx->mU;
		u1 = 0.0f;
	}
	if (fx->mFlags & 0x20000) {
		v0 = v1;
		v1 = 0.0f;
	}
	f32 du = (u1 - u) / (f32)slices;
	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, count);
	for (i = params->mSlices + 1; i != 0; i--) {
		f32 s, c;
		njSinCos(a, &s, &c);
		a += step;
		f32 x   = radius * c;
		FX_PIPE = x;
		f32 y   = radius * s;
		FX_PIPE = y;
		FX_PIPE = z0;
		FX_PIPE = u;
		FX_PIPE = v1;
		FX_PIPE = x;
		FX_PIPE = y;
		FX_PIPE = z1;
		FX_PIPE = u;
		u += du;
		FX_PIPE = v0;
	}
	fxShapeDrawEnd(fx, fx);
}

// 0x800DE040
// A cone with its end cut off
void fxDrawCone(FxShape* fx)
{
	FxShapeParams* params = (FxShapeParams*)fx->mDesc->mParams;
	fxShapeDrawBegin(fx, fx, fx->mDesc);
	int verts = (params->mSlices + 1) << 1;
	f32 r1    = fx->mValues[0];
	f32 r0    = fx->mValues[1];
	f32 z1    = fx->mValues[2];
	f32 z0    = fx->mValues[3];
	int from  = (int)(FX_ANGLE * params->mFrom);
	int to    = (int)(FX_ANGLE * params->mTo);
	if (from == to) {
		to = from + 0x10000;
	}
	int i;
	int a;
	int step = (to - from) / params->mSlices;
	a        = from;
	f32 u    = 0.0f;
	f32 u1   = fx->mU;
	f32 v0   = 0.0f;
	f32 v1   = fx->mV;
	if (fx->mFlags & 0x10000) {
		u  = fx->mU;
		u1 = 0.0f;
	}
	if (fx->mFlags & 0x20000) {
		v0 = v1;
		v1 = 0.0f;
	}
	f32 du = (u1 - u) / (f32)params->mSlices;
	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, (u16)verts);
	for (i = params->mSlices + 1; i != 0; i--) {
		f32 s, c;
		njSinCos(a, &s, &c);
		a += step;
		f32 x0  = r0 * c;
		f32 x1  = r1 * c;
		FX_PIPE = x0;
		f32 y0  = r0;
		y0 *= s;
		f32 y1  = r1 * s;
		FX_PIPE = y0;
		FX_PIPE = z0;
		FX_PIPE = u;
		FX_PIPE = v1;
		FX_PIPE = x1;
		FX_PIPE = y1;
		FX_PIPE = z1;
		FX_PIPE = u;
		u += du;
		FX_PIPE = v0;
	}
	fxShapeDrawEnd(fx, fx);
}

// 0x800DE23C
// Flat blades around a circle
void fxDrawBlades(Fx* base)
{
	FxShape* fx           = (FxShape*)base;
	FxShapeParams* params = (FxShapeParams*)fx->mDesc->mParams;
	fxShapeDrawBegin(fx, fx, fx->mDesc);
	f32 r0 = fx->mValues[0];
	f32 r1 = fx->mValues[1];
	f32 z0;
	f32 z1    = fx->mValues[2];
	z0        = fx->mValues[3];
	int verts = params->mSegs << 2;
	int from  = (int)(FX_ANGLE * params->mFrom);
	int to    = (int)(FX_ANGLE * params->mTo);
	if (from == to) {
		to = from + 0x10000;
	}
	int a;
	int step = (to - from) / params->mSegs;
	a        = from + step / 2;
	f32 u0   = 0.0f;
	f32 u1   = 1.0f;
	f32 v0   = 1.0f;
	f32 v1   = 0.0f;
	if (fx->mFlags & 0x10000) {
		u0 = 1.0f;
		u1 = 0.0f;
	}
	if (fx->mFlags & 0x20000) {
		v0 = 0.0f;
		v1 = 1.0f;
	}
	f32 s, c;
	njSinCos(step >> 1, &s, &c);
	f32 half = r0 * s;
	r0 *= c;
	GXBegin(GX_QUADS, GX_VTXFMT7, verts & 0xFFFF);
	for (int i = params->mSegs; i != 0; i--) {
		njSinCos(a, &s, &c);
		a += step;
		f32 x0  = r0 * c;
		f32 y0  = r0 * s;
		f32 x1  = r1 * c;
		f32 y1  = r1 * s;
		f32 dx  = half * -s;
		f32 dy  = half * c;
		FX_PIPE = x0 + dx;
		FX_PIPE = y0 + dy;
		FX_PIPE = z0;
		FX_PIPE = u0;
		FX_PIPE = v0;
		FX_PIPE = x0 - dx;
		FX_PIPE = y0 - dy;
		FX_PIPE = z0;
		FX_PIPE = u1;
		FX_PIPE = v0;
		FX_PIPE = x1 - dx;
		FX_PIPE = y1 - dy;
		FX_PIPE = z1;
		FX_PIPE = u1;
		FX_PIPE = v1;
		FX_PIPE = x1 + dx;
		FX_PIPE = y1 + dy;
		FX_PIPE = z1;
		FX_PIPE = u0;
		FX_PIPE = v1;
	}
	fxShapeDrawEnd(fx, fx);
}

// 0x800DE498
// A flat ring
void fxDrawRing(FxShape* fx)
{
	FxShapeParams* params = (FxShapeParams*)fx->mDesc->mParams;
	fxShapeDrawBegin(fx, fx, fx->mDesc);
	int verts = (params->mSlices + 1) << 1;
	f32 r1    = fx->mValues[0];
	f32 r0    = fx->mValues[1];
	int from  = (int)(FX_ANGLE * params->mFrom);
	int to    = (int)(FX_ANGLE * params->mTo);
	if (from == to) {
		to = from + 0x10000;
	}
	int i;
	int a;
	int step = (to - from) / params->mSlices;
	a        = from;
	f32 u    = 0.0f;
	f32 u1   = fx->mU;
	f32 v0   = 0.0f;
	f32 v1   = fx->mV;
	if (fx->mFlags & 0x10000) {
		u  = fx->mU;
		u1 = 0.0f;
	}
	if (fx->mFlags & 0x20000) {
		v0 = v1;
		v1 = 0.0f;
	}
	f32 du = (u1 - u) / (f32)params->mSlices;
	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, (u16)verts);
	f32 z = 0.0f;
	for (i = params->mSlices + 1; i != 0; i--) {
		f32 s, c;
		njSinCos(a, &s, &c);
		a += step;
		f32 x0  = r0 * c;
		f32 x1  = r1 * c;
		FX_PIPE = x0;
		f32 y0  = r0;
		y0 *= s;
		f32 y1  = r1 * s;
		FX_PIPE = y0;
		FX_PIPE = z;
		FX_PIPE = u;
		FX_PIPE = v1;
		FX_PIPE = x1;
		FX_PIPE = y1;
		FX_PIPE = z;
		FX_PIPE = u;
		u += du;
		FX_PIPE = v0;
	}
	fxShapeDrawEnd(fx, fx);
}

// 0x800DE680
// Blades that bend upward
void fxDrawBentBlades(FxShape* fx)
{
	FxShapeParams* params = (FxShapeParams*)fx->mDesc->mParams;
	f32* angles           = (f32*)((u8*)params + 0xC);
	fxShapeDrawBegin(fx, fx, fx->mDesc);
	f32 r1 = fx->mValues[0];
	f32 r0 = fx->mValues[1];
	f32 z;
	f32 len  = fx->mValues[2];
	z        = fx->mValues[3];
	int from = (int)(FX_ANGLE * params->mFrom);
	int to   = (int)(FX_ANGLE * params->mTo);
	if (from == to) {
		to = from + 0x10000;
	}
	int i;
	int a;
	int step = (to - from) / (params->mSegs - 1);
	a        = from;
	f32 u0   = 0.0f;
	f32 u1   = 1.0f;
	f32 v0   = 0.0f;
	f32 v1   = 1.0f;
	int bend;
	int bendD = (int)(FX_ANGLE * (angles[1] - angles[0])) / (params->mSegs - 1);
	bend      = (int)(FX_ANGLE * angles[0]);
	if (fx->mFlags & 0x10000) {
		u0 = 1.0f;
		u1 = 0.0f;
	}
	if (fx->mFlags & 0x20000) {
		v0 = 1.0f;
		v1 = 0.0f;
	}
	len -= z;
	GXBegin(GX_QUADS, GX_VTXFMT7, params->mSegs << 2);
	for (i = params->mSegs; i != 0; i--) {
		f32 s, c, bs, bc;
		njSinCos(a, &s, &c);
		njSinCos(bend, &bs, &bc);
		a += step;
		bend += bendD;
		f32 x1  = r1 * c;
		f32 y1  = r1 * s;
		f32 x0  = r0 * c;
		f32 y0  = r0 * s;
		f32 dy  = bs * (c * len);
		f32 dx  = bs * (-s * len);
		FX_PIPE = x1 + dx;
		FX_PIPE = y1 + dy;
		f32 top = len * bc + z;
		FX_PIPE = top;
		FX_PIPE = u0;
		FX_PIPE = v0;
		FX_PIPE = x0 + dx;
		FX_PIPE = y0 + dy;
		FX_PIPE = top;
		FX_PIPE = u1;
		FX_PIPE = v0;
		FX_PIPE = x0;
		FX_PIPE = y0;
		FX_PIPE = z;
		FX_PIPE = u1;
		FX_PIPE = v1;
		FX_PIPE = x1;
		FX_PIPE = y1;
		FX_PIPE = z;
		FX_PIPE = u0;
		FX_PIPE = v1;
	}
	fxShapeDrawEnd(fx, fx);
}

// 0x800DE8F4
// A model from the bank moved and scaled by the values
void fxDrawModelShape(Fx* base)
{
	FxShape* fx = (FxShape*)base;
	f32 m[3][4];
	u8 color[4];
	FxTexUse uses[3];
	FxMaterialRef ref;
	int i;
	u8* params;

	params = (u8*)fx->mDesc->mParams;
	if (fx->mFollow != NULL) {
		PSMTXConcat((const f32*)gFxRenderState, fx->mFollow, (f32*)m);
	} else {
		PSMTXConcat((const f32*)gFxRenderState, (const f32*)fx->mMatrix, (f32*)m);
	}
	fxMtxTranslate(m[0], m[0], fx->mValues[0], fx->mValues[1], fx->mValues[2]);
	fxMtxScale(m[0], m[0], fx->mValues[3], fx->mValues[4], fx->mValues[5]);
	GXLoadPosMtxImm((const f32*)m, GX_PNMTX0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	fxShapeColor(fx, color);
	GXSetChanMatColor(GX_COLOR0A0, *(GXColor*)color);

	ref.mMaterial = fx->mDesc->mMaterial;
	ref.mTexGroup = fx->mDesc->mTexGroup;
	ref.mTexUses  = uses;
	i             = 0;
	while (i < 3) {
		uses[i].mTex   = fx->mTex[i];
		uses[i].mFlags = 0x12;
		i++;
	}
	fxSetMaterial(fx, &ref);
	fxSetCull(0);
	fxDrawModel(*(FxModel**)(gFxBanks[fx->mBank]->mModels + (params[0] << 3) + 4));
}

// 0x800DEA70
// Triangle strips of u8 indices
void fxDrawStrips(FxModelPart* part, FxModel* model)
{
	int i;
	int length;
	u16* lengths = (u16*)part->mLengths;
	u8* indices  = part->mIndices;
	fxSetVtxLayout(3);
	for (i = part->mStrips; i > 0; i--) {
		length = *lengths++;
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, length);
		for (; length > 0; length--) {
			FX_PIPE8 = indices[0];
			FX_PIPE8 = indices[1];
			FX_PIPE8 = indices[2];
			indices += 3;
		}
	}
}

// 0x800DEB08
void fxDrawModel(FxModel* model)
{
	fxSetPositions(&model->mPositions);
	fxSetNormals(&model->mNormals);
	fxSetTexCoords(&model->mTexCoords);
	for (int i = 0; i < model->mEntryCount; i++) {
		FxModelEntry* entry = &model->mEntries[i];
		switch (entry->mType) {
		case 0:
			fxDrawStrips(entry->mPart, model);
			break;
		case 1:
			fxDrawStrips(entry->mPart, model);
			break;
		}
	}
}

// 0x800DEBA8
void fxSetPositions(FxArray* array)
{
	switch (array->mType) {
	case 1:
		GXSetArray(GX_VA_POS, array->mData, 0xC);
		GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		break;
	case 2:
		GXSetArray(GX_VA_POS, array->mData, 6);
		GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XYZ, GX_S16, 8);
		break;
	}
}

// 0x800DEC34
void fxSetNormals(FxArray* array)
{
	switch (array->mType) {
	case 1:
		GXSetArray(GX_VA_NRM, array->mData, 0xC);
		GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
		break;
	case 2:
		GXSetArray(GX_VA_NRM, array->mData, 6);
		GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 0xE);
		break;
	case 3:
		GXSetArray(GX_VA_NRM, array->mData, 3);
		GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 6);
		break;
	}
}

// 0x800DECF8
void fxSetTexCoords(FxArray* array)
{
	switch (array->mType) {
	case 1:
		fxSetCoordArray(array->mData, 8);
		GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
		break;
	case 2:
		fxSetCoordArray(array->mData, 4);
		GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
		break;
	case 3:
		fxSetCoordArray(array->mData, 4);
		GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX0, GX_TEX_ST, GX_S16, 0xA);
		break;
	}
}

// 0x800DEDB0
void fxMtxTranslate(const f32* in, f32* out, f32 x, f32 y, f32 z)
{
	f32 m[3][4];
	PSMTXTrans(m[0], x, y, z);
	PSMTXConcat(in, (const f32*)m, out);
}

// 0x800DEDF4
void fxMtxScale(const f32* in, f32* out, f32 x, f32 y, f32 z)
{
	f32 m[3][4];
	PSMTXScale(m[0], x, y, z);
	PSMTXConcat(in, (const f32*)m, out);
}

// 0x800DEE38
FxShape* fxEmitterAlloc()
{
	FxShape* fx = (FxShape*)gFxRigFreeList;
	if (fx == NULL) {
		return NULL;
	}
	gFxRigFreeList = (FxEmitter*)fx->mNext;
	fx->mFlags     = 0;
	return fx;
}

void (*gFxShapeInits[8])(FxShape*, FxShape*, void*) = {
	fxInitSphere, fxInitDome, fxInitCylinder, fxInitCone, fxInitBlades, fxInitRing, fxInitBentBlades, fxInitModelShape,
};
