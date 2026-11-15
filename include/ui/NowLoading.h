#ifndef _UI_NOWLOADING_H_
#define _UI_NOWLOADING_H_

#include "core/types.h"

extern "C" {
void nowLoadingDisable();
void nowLoadingDraw();
void nowLoadingDrawBack();
void nowLoadingDrawDots();
void nowLoadingDrawEgg(f32);
f32 nowLoadingDrawPart(f32, f32, u32, int);
void nowLoadingDrawText(f32, f32);
u32 nowLoadingGetColor(f32);
f32 nowLoadingGetHop(f32);
void nowLoadingInit();
int nowLoadingIsDisabled();
void nowLoadingReset();
void nowLoadingUpdate();
}

#include "Dolphin/GX/GXTypes.h"
extern "C" {
extern const GXColor sNowLoadingFogColor;
}

#endif
