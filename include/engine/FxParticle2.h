#ifndef _ENGINE_FXPARTICLE2_H_
#define _ENGINE_FXPARTICLE2_H_

#include "core/types.h"

struct FxBank;
struct FxEmitter;
struct FxParticle;

extern "C" {
void fxCellCoords(int, int, u8*);
void fxPartRunScript(FxParticle*);
void fxQuadDraw(FxEmitter*);
void fxQuadDrawOne(FxParticle*, int);
FxEmitter* fxQuadNew(FxBank*, int);
void fxQuadPartStep(FxParticle*);
FxParticle* fxQuadSpawn(u16, u16, u8*, FxEmitter*, f32, f32, f32, f32, f32, f32, f32, f32, f32);
void fxQuadUpdate(FxEmitter*);
void fxQuatFromAngles(f32*, f32, f32, f32);
void fxQuatFromAxis(f32*, const f32*, f32);
u8* fxQuatTurnRandom(u8*, f32*);
void fxSpritePartStep(FxParticle*);
u8* fxTrailAddPoint(u8*, FxParticle*);
void fxTrailDraw(FxEmitter*);
FxEmitter* fxTrailNew(FxBank*, int);
void fxTrailPartStep(FxParticle*);
void fxTrailRunScript(FxParticle*);
void fxTrailUpdate(FxEmitter*);
}

extern "C" {
f32 fxNormalize(f32*, const f32*);
}

#endif
