#ifndef _ENGINE_FXEMITTER_H_
#define _ENGINE_FXEMITTER_H_
struct FxEmitterDesc;
struct FxMaterialRef;
struct FxTexUse;

#include "core/types.h"

struct FxBank;
struct FxBone;
struct FxEmitter;
struct FxParticle;
struct FxRig;

extern "C" {
void fxBoneBuild(FxBone*);
void fxBoneWorld(FxRig*, FxBone*);
FxEmitter* fxEmitterNew(FxBank*, int);
void fxEmitterStep(FxEmitter*, FxEmitter*);
void fxRibbonDraw(Fx*);
void fxRibbonDrawBegin(FxEmitter*, FxEmitter*);
void fxRibbonDrawEnd(FxEmitter*, FxEmitter*);
void fxRibbonPartStep(FxParticle*);
FxParticle* fxRibbonSpawn(FxEmitter*, f32, f32, f32, f32, f32, f32);
void fxRibbonUpdate(FxEmitter*);
void fxRigFree(FxRig*);
FxRig* fxRigNew(FxBank*, int);
void fxRigUpdate(FxRig*);
FxEmitter* fxSpriteNew(FxBank*, int);
FxParticle* fxSpriteSpawn(u16, u16, u8*, FxEmitter*, f32, f32, f32, f32, f32, f32, f32, f32, f32);
void fxSweepDraw(Fx*);
FxParticle* fxSweepDrawSpans(FxParticle*, int, const f32*, int);
FxParticle* fxSweepLoadMatrices(FxParticle*, int, const void*);
void fxSweepUpdate(FxEmitter*);
}

struct FxTexUse {
	u8 mFlags;
	u8 mTex;
};

struct FxMaterialRef {
	u16 mMaterial;
	u16 mTexGroup;
	FxTexUse* mTexUses;
};

/** A description of an emitter */
struct FxEmitterDesc {
	u16 mKind;      // _00
	u16 mLife;      // _02
	u8 mTex[2];     // _04
	u16 mMax;       // _06
	u32 mFlags;     // _08
	s16 mTexGroup;  // _0C
	s16 mMaterial;  // _0E
	f32 mRate;      // _10
	f32 _14;
	f32 _18;
	f32 mShapeB;    // _1C
	f32 mShapeC;    // _20
	f32 mShapeD;    // _24
	int mCount;     // _28
	u8* mScript; // _2C
	u16 mPartFlags; // _30
	u16 mPartLife;  // _32
	f32 mDir[3]; // _34
	f32 mGravity;   // _40
	f32 mDamp;      // _44
	f32 mScale;     // _48
	u8* mPartScript; // _4C
};

#include "engine/Fx.h"
extern "C" {
extern FxGroup gFxGroups[];
extern int sFxTevColorIn[];
extern int sFxTevAlphaIn[];
}

extern "C" {
extern void (*sFxEmitterUpdaters[2])(FxEmitter*);
extern void (*sFxEmitterDrawers[2])(Fx*);
}

#endif
