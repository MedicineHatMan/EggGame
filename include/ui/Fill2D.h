#ifndef _UI_FILL2D_H_
#define _UI_FILL2D_H_

#include "core/types.h"

extern "C" {
void fill2DBegin(int);
void fill2DEnd();
void fill2DSetColor(u32);
}

extern "C" {
void fill2DRect(f32, const f32*, int);
}

#endif
