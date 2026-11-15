#ifndef _ENGINE_FXSYSTEM_H_
#define _ENGINE_FXSYSTEM_H_
struct FxEmitter;
struct FxExtra;
struct FxParticle;
struct FxShape;
struct Fx;

#include "core/types.h"
#include "engine/Fx.h"

class TPolygonAnim;
struct FxParticle;

extern "C" {
void createTPolygonAnim__Fv();
void fxAddLayer0(void*);
void fxAddLayer1(void*);
void fxAddLayer2(void*);
void fxFogToggle(int);
void fxSetupStub();
void fxShapePoolLink(void*, int);
void fxRigPoolLink(void*, int);
void fxParticlePoolLink(void*, int);
void fxExtraPoolLink(void*, int);
void fxSetRigAlloc(void* (*alloc)(u32), void (*release)(void*));
void fxSetStepTable(void (**table)(FxParticle*));
void fxSetFlag40Callback(void*);
void fxSetCurrentMtx(int);
void fxSetVtxLayout(u32);
void fxLoadExtraTex();
void fxBankSet(u32, void*);
void fxDrawGroup(int);
void fxFlagOwner(void*);
void fxRemoveAll();
void fxRemoveBank(int);
void fxRemoveOwner(void*);
void fxSetCull(int);
void fxSetTevMode(int, int);
void fxSystemInit();
void fxSystemReset();
void fxSystemShutdown();
void fxUpdateGroup(int);
}

extern "C" {
Fx* fxSpawnFollow(int, int, int, f32*);
Fx* fxSpawnAt(int, int, int, const f32*);
void fxRemove(Fx*);
Fx* fxSpawn(int, int, int);
void fxStop(Fx*);
}

extern "C" {
void fxSetOwner(Fx*, void*);
void fxSetCoordArray(const void*, u8);
void fxSetTevColor(const u8*);
void fxShow(Fx*);
void fxHide(Fx*);
void fxResume(Fx*);
void fxPause(Fx*);
void fxSetFollow(Fx*, f32*);
void fxSetView(const f32*);
}

extern "C" {
int fxBankBind(int, u8*);
int fxBankUnbind(int, u8*);
}

extern "C" {
void fxSetZMode(int, u8);
}

extern "C" {
extern FxFactory gFxFactories[];
extern void (*gFxFlag40Callback)(int);
extern int gFxLastFlag40;
extern FxEmitter* gFxRigFreeList;
extern FxShape* gFxShapeFreeList;
extern int gFxDebugFlags;
extern int gFxGroupCount;
extern s8 gFxRenderReady;
extern FxParticle* gFxParticleFreeList;
extern FxExtra* gFxExtraFreeList;
extern u16 gFxParticleCount;
extern u16 gFxParticleExtraCount;
extern f32 gFxCoordArray[25][2];
extern u8 gFxVertexColor[2];
extern u8 gFxLastColor[4];
extern int gFxLastMatrixSlot;
extern int gFxLastBlendMode;
extern void* gFxLastArray;
extern int gFxLastMode;
// Reset as a word compared and kept as its first byte
union FxZModeWord {
	u32 mWord;
	u8 mMode;
};
extern FxZModeWord gFxLastZMode;
extern void* sFxShapeMem;
extern void* sFxRigMem;
extern void* sFxParticleMem;
extern void* sFxExtraMem;
extern TPolygonAnim* gFxPolygonAnim;
extern int sFxShapeLimit;
extern void* sFxShapePool;
extern int sFxShapePoolCount;
extern void* sFxRigPool;
extern int sFxRigPoolCount;
extern void* sFxParticlePool;
extern int sFxParticlePoolCount;
extern void* sFxExtraPool;
extern int sFxExtraPoolCount;
extern void* (*gFxRigAlloc)(u32);
extern void (*gFxRigRelease)(void*);
extern void (**gFxStepTable)(FxParticle*);
extern void* sFxTexImage;
extern u16 sFxTexWidth;
extern u16 sFxTexHeight;
extern int sFxTexFormat;
extern int sFxTevColorModes[][4];
extern int sFxTevAlphaModes[][4];
}

extern "C" {
extern u8 sFxDrawFuncs[0x10];
}

#endif
