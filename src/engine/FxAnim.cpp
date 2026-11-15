#include "stage/Stage.h"
#include "engine/Fx.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/FxSystem.h"
#include "engine/FxAnim.h"
#include "engine/FxParticle.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"

#include "engine/FxEmitter.h"
#include "engine/FxShape.h"
#include "io/NinjaFile.h"

struct FxSimpleMaterial {
	u32 mFlags; // _00 1 = blended 2 = alpha test 4 = textured
	u8 mBlend;  // _04
};

struct FxTevStage {
	u8 mTex;    // _00
	u8 mCoord;  // _01
	u8 _02;
	u8 mCombine; // _03
	u8 mWarp;   // _04
	u8 _05[3];
};

struct FxFullMaterial {
	u8 mBlendMode;  // _00
	u8 mBlendSrc;   // _01
	u8 mBlendDst;   // _02
	u8 mBlendOp;    // _03
	u8 _04;
	u8 mZMode;      // _05
	u8 mZRef;       // _06
	u8 mTexCount;   // _07
	u8 mStages;     // _08
	u8 _09;
	u8 mCoords;     // _0A
	u8 mInds;       // _0B
	FxTevStage* mStageList; // _0C
	u32 _10;
	u8* mCoordList; // _14
	u8* mIndList;   // _18
};

// 0x800DB7D4
u8* fxRead16(u8* src, u8* dst)
{
	dst[0] = *src++;
	dst[1] = *src++;
	return src;
}

// 0x800DB7F0
u8* fxRead32(u8* src, u8* dst)
{
	dst[0] = *src++;
	dst[1] = *src++;
	dst[2] = *src++;
	dst[3] = *src++;
	return src;
}

// 0x800DB81C
u8* fxReadCount(u8* src, u16* out)
{
	u32 first = *src++;
	u16 value = first & 0x7F;
	if (first & 0x80) {
		value = (value << 8) + *src++;
	}
	*out = value;
	return src;
}

// 0x800DB850
u8* fxReadFrames(u8* src, s16* out)
{
	u32 first = *src++;
	u16 value = first & 0x7F;
	if (first & 0x80) {
		value = (value << 8) + *src++;
	}
	*out = value;
	return src;
}

// 0x800DB884
f32 fxKeyValue(FxKey* key)
{
	if (key->mFrames != 0) {
		switch (key->mType) {
		case 0:
			return key->mTo + (f32)key->mFrame * (key->mFrom - key->mTo) / (f32)key->mFrames;
		case 1:
			return hermite(key->mFrom, key->mTanFrom, key->mTo, key->mTanTo, 1.0f - (f32)key->mFrame / (f32)key->mFrames);
		}
	}
	return key->mTo;
}

// 0x800DB964
// A hermite curve
f32 hermite(f32 p0, f32 m0, f32 p1, f32 m1, f32 t)
{
	f32 t2 = t * t;
	f32 t3 = t2 * t;
	f32 h00 = 1.0f + (2.0f * t3 - 3.0f * t2);
	f32 h10 = t + (t3 - 2.0f * t2);
	f32 h01 = -2.0f * t3 + 3.0f * t2;
	f32 h11 = t3 - t2;
	return h11 * m1 + (h10 * m0 + (h00 * p0 + h01 * p1));
}

// 0x800DB9AC
f32 fxWaveValue(FxWave* wave)
{
	switch (wave->mType) {
	case 0:
		return 0.0f;
	case 1:
		return wave->mAmp * njSin(wave->mAngle);
	case 2:
		return wave->mAmp * ((wave->mAngle & 0x8000) == 0 ? 1.0f : -1.0f);
	case 3:
		return 0.0f;
	}
	return 0.0f;
}

// 0x800DBA4C
// Sets up a material and turns the fog callback on or off with the effect's flag
void fxSetMaterial(Fx* fx, FxMaterialRef* ref)
{
	FxBank* bank = gFxBanks[fx->mBank];
	FxMaterialEntry* entry = &bank->mMaterials[ref->mMaterial];
	if (gFxFlag40Callback != NULL) {
		u32 flag = fx->mFlags & 0x40;
		if (flag != gFxLastFlag40) {
			gFxLastFlag40 = flag;
			((void (*)(u8))gFxFlag40Callback)(flag == 0);
		}
	}
	switch (entry->mType) {
	case 0:
		fxSetSimpleMaterial(fx, bank, (FxSimpleMaterial*)entry->mData, ref);
		break;
	case 1:
		fxSetFullMaterial(fx, bank, (FxFullMaterial*)entry->mData, ref);
		break;
	}
}

// 0x800DBB18
void fxSetSimpleMaterial(Fx*, FxBank* bank, FxSimpleMaterial* mat, FxMaterialRef* ref)
{
	if (mat->mFlags & 4) {
		GXSetNumTevStages(1);
		GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
		fxSetTevMode(0, 0);
		GXSetNumTexGens(1);
		GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2X4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
		GXLoadTexMtxImm((const float*)gjIdentityMatrix, GX_TEXMTX0, GX_MTX2x4);
		fxBindTexture(bank, ref->mTexGroup, ref->mTexUses, 0);
	} else {
		GXSetNumTevStages(1);
		GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		fxSetTevMode(0, 4);
		GXSetNumTexGens(0);
	}
	GXSetNumIndStages(0);
	GXSetTevDirect(GX_TEVSTAGE0);
	if (mat->mFlags & 1) {
		if (mat->mFlags & 2) {
			fxSetZMode(3, 0x80);
		} else {
			fxSetZMode(2, 0);
		}
		fxSetBlend(mat->mBlend, 1);
	} else {
		fxSetZMode(1, 0);
		fxSetBlend(mat->mBlend, 0);
	}
}

// 0x800DBC64
void fxSetFullMaterial(Fx* fx, FxBank*, FxFullMaterial* mat, FxMaterialRef* ref)
{
	u32* tables = (u32*)gFxFactories;
	int i;
	for (i = 0; i < mat->mTexCount; i++) {
		fxBindTexture(gFxBanks[fx->mBank], ref->mTexGroup, &ref->mTexUses[i], i);
	}
	fxSetZMode(mat->mZMode, mat->mZRef);
	GXSetBlendMode(mat->mBlendMode, mat->mBlendSrc, mat->mBlendDst, mat->mBlendOp);
	GXSetNumTevStages(mat->mStages);
	GXSetNumIndStages(mat->mInds);
	GXSetNumTexGens(mat->mCoords);
	FxTevStage* stage = mat->mStageList;
	for (i = 0; i < mat->mStages; i++) {
		int map;
		if (!(stage->mTex & 0x80)) {
			map = stage->mTex;
		} else if (stage->mTex == 0x80) {
			map = 7;
		} else {
			map = 0xFF;
		}
		GXSetTevOrder(i, stage->mCoord, map, GX_COLOR0A0);
		fxSetTevMode(i, stage->mCombine);
		if (stage->mWarp != 0xFF) {
			GXSetTevIndWarp(i, stage->mWarp, GX_TRUE, GX_FALSE, GX_ITM_0);
		} else {
			GXSetTevDirect(i);
		}
		stage++;
	}
	u8* coord = mat->mCoordList;
	for (i = 0; i < mat->mCoords; i++) {
		GXSetTexCoordGen2(i, GX_TG_MTX3X4, tables[8 + coord[0]], tables[14 + coord[1]], coord[2], tables[17 + coord[3]]);
		coord += 4;
	}
	u8* ind = mat->mIndList;
	for (i = 0; i < mat->mInds; i++) {
		GXSetIndTexOrder(i, ind[1], ind[0]);
		ind += 4;
	}
}

// 0x800DBE40
// Binds one texture of a group
void fxBindTexture(FxBank* bank, int group, FxTexUse* use, int map)
{
	if (bank->mTextures == NULL) {
		return;
	}
	if ((u16)group >= bank->mTexGroupCount) {
		return;
	}
	FxTexGroup* tex = &bank->mTexGroups[(u16)group];
	if (use->mTex >= tex->mCount) {
		return;
	}
	njSetTexture(*bank->mTextures);
	int s;
	if (use->mFlags & 1) {
		s = 0;
	} else if (use->mFlags & 2) {
		s = 1;
	} else {
		s = 2;
	}
	int t;
	if (use->mFlags & 8) {
		t = 0;
	} else if (use->mFlags & 0x10) {
		t = 1;
	} else {
		t = 2;
	}
	gjLoadTextureNum(tex->mTextures[use->mTex], s, t, map);
}

// 0x800DBF10
void fxSetBlend(int mode, int on)
{
	if (on != 0) {
		switch (mode) {
		case 0:
			GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
			break;
		case 1:
			GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
			break;
		case 2:
			GXSetBlendMode(GX_BM_BLEND, GX_BL_INVDSTCOL, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
			break;
		}
	} else {
		GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_CLEAR);
	}
}

// 0x800DBFB4
void fxEmitterFree(FxEmitter* fx)
{
	if (fx->mKids != NULL) {
		fxPartFreeList(fx->mKids);
	}
	fx->mNext = (Fx*)gFxRigFreeList;
	gFxRigFreeList = fx;
}

// 0x800DBFF8
// Steps every particle and frees the finished ones
void fxStepParticles(FxEmitter* fx, void (*step)(FxParticle*))
{
	FxParticle* part = fx->mKids;
	FxParticle* next;
	FxParticle* prev = NULL;
	while (part != NULL) {
		next = part->mNext;
		step(part);
		if (part->mFlags & 0x100) {
			if (prev == NULL) {
				fx->mKids = next;
			} else {
				prev->mNext = next;
			}
			fx->mCount--;
			fxPartFree(part);
		} else {
			prev = part;
		}
		part = next;
	}
}

// 0x800DC08C
// Stops an emitter It stays until its particles are gone
void fxEmitterStop(Fx* fx, FxEmitter* self)
{
	if (fx->mFlags & 0x200) {
		fxPartFreeList(self->mKids);
		self->mKids  = NULL;
		self->mCount = 0;
	}
	if ((u32)self->mCount != 0) {
		self->mRate = 0.0f;
		fx->mStop = 1;
	} else {
		fx->mFlags |= 0x40000000;
	}
}

// 0x800DC104
// The factory of the script effects
FxShape* fxShapeNew(FxBank* bank, int index)
{
	FxShape* fx = gFxShapeFreeList;
	if (fx == NULL) {
		return NULL;
	}
	gFxShapeFreeList = (FxShape*)fx->mNext;
	fx->mFlags = 0;
	FxDesc* desc = (FxDesc*)bank->mDescs[index * 2 + 1];
	fx->mRemove = (void (*)(Fx*))fxShapeFree;
	fx->mFlags |= desc->mFlags;
	fx->mDesc = desc;
	fx->mStop = desc->mLife != 0 ? desc->mLife : 0xFFFF;
	fx->mPc    = 0;
	fx->mWait  = desc->mScript != NULL ? 1 : 0;
	fx->mLoops = 0;
	fx->mColorFrames = 0;
	fx->mAlphaFrames = 0;
	fx->mUvFrames[1] = 0;
	fx->mUvFrames[0] = 0;
	fx->mRotFrames   = 0;
	fx->mColor[3] = 0xFF;
	fx->mColor[2] = 0xFF;
	fx->mColor[1] = 0xFF;
	fx->mColor[0] = 0xFF;
	fx->mV = 1.0f;
	fx->mU = 1.0f;
	fx->mRotTo = 0.0f;
	fx->mRot   = 0.0f;
	fx->mUv[0][1] = 0.0f;
	fx->mUv[0][0] = 0.0f;
	fx->mUv[1][1] = 0.0f;
	fx->mUv[1][0] = 0.0f;
	fx->mUvTo[0][1] = 0.0f;
	fx->mUvTo[0][0] = 0.0f;
	fx->mUvTo[1][1] = 0.0f;
	fx->mUvTo[1][0] = 0.0f;
	syMemSet(fx->mTex, 0, 3);
	gFxShapeInits[desc->mShape](fx, fx, desc->mParams);
	return fx;
}

// 0x800DC258
void fxShapeFree(FxShape* fx)
{
	fx->mNext = (Fx*)gFxShapeFreeList;
	gFxShapeFreeList = fx;
}

// 0x800DC268
// Clears the keys and waves and loads the base values
void fxChannelsInit(FxShape* fx, const FxChannel* channels, int count)
{
	syMemSet(fx->mKeys, 0, 0x60);
	syMemSet(fx->mWaves, 0, 0x40);
	for (int i = 0; i < count; i++) {
		fx->mValues[i] = channels[i].mBase;
	}
}

// 0x800DC2E4
// Steps the keys and waves and works out the values
void fxChannelsUpdate(FxShape* fx, const FxChannel* channels, int count)
{
	int i;
	for (i = 0; i < 4; i++) {
		FxKey* key = &fx->mKeys[i];
		if (key->mFrames != 0) {
			if (--key->mFrame == 0) {
				key->mFrames = 0;
			}
		}
	}
	for (i = 0; i < 4; i++) {
		FxWave* wave = &fx->mWaves[i];
		if (wave->mFrames != 0) {
			wave->mAmp += (wave->mAmpTo - wave->mAmp) / (f32)wave->mFrames;
			wave->mFrames--;
		}
		wave->mAngle += wave->mStep;
	}
	for (i = 0; i < count; i++) {
		const FxChannel* ch = &channels[i];
		FxKey* key = &fx->mKeys[ch->mKey];
		FxWave* wave = &fx->mWaves[ch->mWave];
		f32 sum;
		switch (ch->mMode) {
		case 1:
			fx->mValues[i] = fxKeyValue(key);
			break;
		case 2:
			fx->mValues[i] = fxWaveValue(wave);
			break;
		case 3:
			sum = fxWaveValue(wave);
			fx->mValues[i] = fxKeyValue(key) + sum;
			break;
		case 4:
			fx->mValues[i] = ch->mBase + fxKeyValue(key);
			break;
		case 5:
			fx->mValues[i] = ch->mBase + fxWaveValue(wave);
			break;
		case 6:
			sum = fxWaveValue(wave);
			fx->mValues[i] = (ch->mBase + fxKeyValue(key)) + sum;
			break;
		}
	}
}
