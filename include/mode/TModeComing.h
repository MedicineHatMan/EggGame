#ifndef _MODE_TMODECOMING_H_
#define _MODE_TMODECOMING_H_
struct TArc;

#include "core/types.h"

class TAeObjComing;

extern "C" {
TAeObjComing* comingCreate();
void comingFree();
void comingLoad();
}

#include "Dolphin/GX/GXTypes.h"
#include "Ninja/njtypes.h"
extern "C" {
extern GXColor sModeComingClearColor;
extern TArc* sComingArc;
extern void* sComingAe;
extern NJS_TEXLIST* sComingTexList;
}

#endif
