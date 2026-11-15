#ifndef _ENEMY_ENEMYTEXFX_H_
#define _ENEMY_ENEMYTEXFX_H_

#include "core/types.h"

struct Vec3f;

extern "C" {
void texFxSuitEnd();
void texFxSuitLoadTex(int);
void texFxSuitScroll(f32, f32, int);
void texFxTintEnd();
void texFxTintLoadTex(int);
void texFxTintSetMtx(int, const Vec3f*, int);
void texFxWarpBegin(f32, int, int, int, int, int, f32);
void texFxWarpEnd();
void texFxWarpLoadMtx(int, int);
void texFxWarpLoadTex(int);
}

extern "C" {
void texFxTintBegin(int, const u32*);
void texFxScreenMtx(f32*);
}

extern "C" {
void texFxSuitBegin(int, u32);
}

extern "C" {
extern const int texFxTexMaps[];
extern const int texFxTexMtxs[];
extern const int texFxPostTexMtxs[];
extern const int texFxTexCoords[];
extern const int texFxTevStages[];
extern const int texFxKColors[];
extern const int texFxKColorSels[];
extern const int texFxTevRegs[];
extern const int texFxTevRegInputs[];
extern const int texFxIndStages[];
extern const int texFxIndMtxs[];
}

#endif
