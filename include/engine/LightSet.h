#ifndef _ENGINE_LIGHTSET_H_
#define _ENGINE_LIGHTSET_H_
struct Vec3f;
struct LightSlot;

#include "core/types.h"

extern "C" {
u32 lightSetApply();
void lightSetCreate();
void lightSetDestroy();
void lightSetFrom(int, int);
void lightSetLoad();
void lightSetTo(int, int);
}

extern "C" {
int lightSetIsValid(LightSlot*, int, int);
}

extern "C" {
int lightSetFindArea(Vec3f*);
void lightSetSelect(int);
}

extern "C" {
void lightSetBlend(f32);
}

#endif
