#ifndef _ENGINE_FXPARTICLE_H_
#define _ENGINE_FXPARTICLE_H_
struct FxEmitterDesc;

#include "core/types.h"

struct FxEmitter;
struct FxExtra;
struct FxParticle;

extern "C" {
FxExtra* fxExtraAlloc(FxParticle*);
void fxExtraFree(FxParticle*, FxExtra*);
FxParticle* fxPartAlloc();
FxParticle* fxPartAllocExtra();
void fxPartColor(FxParticle*, u8*);
void fxPartDeflect(FxParticle*, int);
void fxPartFree(FxParticle*);
void fxPartFreeList(FxParticle*);
void fxSpawnLocal(FxEmitter*, FxEmitter*);
}

extern "C" {
void fxEmitterReadShape(FxEmitter*, FxEmitterDesc*);
}

#endif
