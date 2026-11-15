#ifndef _ENGINE_FXSPRITE_H_
#define _ENGINE_FXSPRITE_H_

#include "core/types.h"

struct Fx;
struct FxBank;
struct FxEmitter;
struct FxParticle;

extern "C" {
void fxSpriteStub(Fx*, Fx*);
void* fxBankNullKind(FxBank*, int);
void fxConePos(FxParticle*, f32*, f32*, f32*);
void fxSpriteDraw(Fx*, FxEmitter*);
void fxSpriteDrawKind(Fx*);
void fxSpriteMaterial(Fx*, FxEmitter*, int);
void fxSpriteUpdate(FxEmitter*);
void fxStreakDraw(FxEmitter*);
FxEmitter* fxStreakNew(FxBank*, int);
void fxStreakPartStep(FxParticle*);
void fxStreakUpdate(FxEmitter*);
}

extern "C" {
extern f32 sFxSpriteTexMtx[4];
}

struct FxEmitter;
struct FxParticle;
typedef FxParticle* (*FxSpawnFn)(u16, u16, u8*, FxEmitter*, f32, f32, f32, f32, f32, f32, f32, f32, f32);
extern "C" {
void fxSpawnInWorld(FxEmitter* fx, FxEmitter* self, FxSpawnFn spawn);
}

#endif
