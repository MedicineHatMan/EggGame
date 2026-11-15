#ifndef _OBJECT_GREEN_GREENOBJBANK_H
#define _OBJECT_GREEN_GREENOBJBANK_H

#include "core/types.h"
struct NJS_OBJECT;
struct NJS_TEXLIST;

extern "C" {
NJS_OBJECT** getGreenObjModels();
NJS_TEXLIST** getGreenObjTextures();
void greenObjExit();
void greenObjFree();
void greenObjInit();
void greenObjLoad();
}

#endif
