#ifndef _ENGINE_FXANIM_H_
#define _ENGINE_FXANIM_H_
struct FxFullMaterial;
struct FxMaterialRef;
struct FxSimpleMaterial;
struct FxTexUse;

#include "core/types.h"

struct Fx;
struct FxBank;
struct FxChannel;
struct FxEmitter;
struct FxKey;
struct FxShape;
struct FxWave;

extern "C" {
void fxChannelsInit(FxShape*, const FxChannel*, int);
void fxChannelsUpdate(FxShape*, const FxChannel*, int);
void fxEmitterFree(FxEmitter*);
void fxEmitterStop(Fx*, FxEmitter*);
f32 fxKeyValue(FxKey*);
u8* fxReadCount(u8*, u16*);
void fxSetBlend(int, int);
void fxShapeFree(FxShape*);
FxShape* fxShapeNew(FxBank*, int);
f32 fxWaveValue(FxWave*);
f32 hermite(f32, f32, f32, f32, f32);
}

extern "C" {
void fxSetMaterial(Fx*, FxMaterialRef*);
void fxSetSimpleMaterial(Fx*, FxBank*, FxSimpleMaterial*, FxMaterialRef*);
void fxSetFullMaterial(Fx*, FxBank*, FxFullMaterial*, FxMaterialRef*);
void fxBindTexture(FxBank*, int, FxTexUse*, int);
u8* fxRead16(u8*, u8*);
u8* fxRead32(u8*, u8*);
u8* fxReadFrames(u8*, s16*);
}

struct FxEmitter;
struct FxParticle;
extern "C" {
void fxStepParticles(FxEmitter* fx, void (*step)(FxParticle*));
}

#endif
