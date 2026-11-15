#ifndef _ENGINE_DRAW2D_H_
#define _ENGINE_DRAW2D_H_

#include "core/types.h"

extern "C" {
void textDrawBegin(int);
void textDrawEnd();
void textDrawQuad(s16, s16, s16, s16, f32, f32, f32, f32);
void textDrawSet(f32, u32);
void textSetColor(u32);
void textSetDepth(f32);
void tlutInit();
}

#endif
