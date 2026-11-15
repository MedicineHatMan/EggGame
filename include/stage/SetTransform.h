#ifndef _STAGE_SETTRANSFORM_H_
#define _STAGE_SETTRANSFORM_H_

#include "core/types.h"

struct SetTransformBase;
struct Vec3f;

extern "C" {
void transAddDrag(f32, SetTransformBase*);
void transAddForce(SetTransformBase*, const Vec3f*);
}

#endif
