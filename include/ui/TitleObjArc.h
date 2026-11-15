#ifndef _UI_TITLEOBJARC_H_
#define _UI_TITLEOBJARC_H_
#include "Ninja/njtypes.h"
struct NJS_OBJECT;
struct NJS_MOTION;

#include "core/types.h"

extern "C" {
NJS_MOTION** getTitleObjMotion(int);
void initTitleObjArc();
void initTitleObjNone();
void shutdownTitleObjArc();
void shutdownTitleObjNone();
}

extern "C" {
void** getTitleObjScene();
NJS_OBJECT** getTitleObjModels();
NJS_TEXLIST** getTitleObjTexLists();
}

#endif
