#ifndef _CORE_VECMATH_H_
#define _CORE_VECMATH_H_

#include "core/types.h"

struct Vec3f;

extern "C" {
f32 vecDistance(const Vec3f*, const Vec3f*);
}

extern "C" {
void vecScaleAdd(f32, Vec3f*, const Vec3f*, const Vec3f*);
f32 vecNormalize(Vec3f*, const Vec3f*);
void vecScale(f32, Vec3f*, const Vec3f*);
void vecAdd(Vec3f*, const Vec3f*, const Vec3f*);
void vecSub(Vec3f*, const Vec3f*, const Vec3f*);
f32 vecLength(const Vec3f*);
f32 vecDistanceSq(const Vec3f*, const Vec3f*);
void vecNegate(Vec3f*, const Vec3f*);
f32 vecLengthSq(const Vec3f*);
}

#endif
