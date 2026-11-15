#ifndef _ENGINE_FOGBLEND_H_
#define _ENGINE_FOGBLEND_H_

#include "core/types.h"

struct Vec3f;

extern "C" {
void fogBlendHit(Vec3f*, int, u16);
void fogBlendReset();
}

extern "C" {
f32 fogBlendUpdate();
}

extern "C" {
int fogBlendIsDay();
}

#endif
