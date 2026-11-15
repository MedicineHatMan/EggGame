#ifndef _ENGINE_FXSHAPE_H_
#define _ENGINE_FXSHAPE_H_
struct FxBank;
struct FxArray;
struct FxModel;
struct FxModelPart;

#include "core/types.h"

struct Fx;
struct FxDesc;
struct FxShape;

extern "C" {
void fxInitSphere(FxShape*, FxShape*, void*);
void fxInitDome(FxShape*, FxShape*, void*);
void fxInitCylinder(FxShape*, FxShape*, void*);
void fxInitCone(FxShape*, FxShape*, void*);
void fxInitBlades(FxShape*, FxShape*, void*);
void fxInitRing(FxShape*, FxShape*, void*);
void fxInitBentBlades(FxShape*, FxShape*, void*);
void fxInitModelShape(FxShape*, FxShape*, void*);
void fxUpdateSphere(FxShape*);
void fxUpdateDome(FxShape*);
void fxUpdateCylinder(FxShape*);
void fxUpdateCone(FxShape*);
void fxUpdateBlades(FxShape*);
void fxUpdateRing(FxShape*);
void fxUpdateBentBlades(FxShape*);
void fxUpdateModelShape(FxShape*);
void fxMtxTranslate(const f32*, f32*, f32, f32, f32);
void fxMtxScale(const f32*, f32*, f32, f32, f32);
void fxDrawBentBlades(FxShape*);
void fxDrawBlades(FxShape*);
void fxDrawCone(FxShape*);
void fxDrawCylinder(FxShape*);
void fxDrawDome(FxShape*);
void fxDrawModelShape(Fx*);
void fxDrawRing(FxShape*);
void fxDrawSphere(FxShape*);
void fxShapeColor(FxShape*, u8*);
void fxShapeDrawBegin(Fx*, FxShape*, FxDesc*);
void fxShapeDrawEnd(Fx*, FxShape*);
void fxShapeRunScript(FxShape*);
}

extern "C" {
FxShape* fxEmitterAlloc();
void fxLoadTexMatrices(Fx*, int, const f32*, f32*);
void fxDrawStrips(FxModelPart*, FxModel*);
void fxDrawModel(FxModel*);
void fxSetPositions(FxArray*);
void fxSetNormals(FxArray*);
void fxSetTexCoords(FxArray*);
}

extern "C" {
extern FxBank* gFxBanks[];
extern u8 gFxRenderState[];
extern f32 sFxViewMtx[4][4];
}

extern "C" {
extern void (*gFxShapeInits[8])(FxShape*, FxShape*, void*);
}

#endif
