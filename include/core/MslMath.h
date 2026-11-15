#ifndef _CORE_MSLMATH_H_
#define _CORE_MSLMATH_H_

#include "core/types.h"

extern "C" {
f32 cosf(f32);
double log(double);
double log10(double);
f32 modff(f32, f32*);
f32 powf(f32, f32);
f32 sinf(f32);
f32 sqrtf(f32);
char* strupr(char*);
f32 tanf(f32);
}

#endif
