#ifndef _EGG_EGGPLANE_H_
#define _EGG_EGGPLANE_H_

#include "core/types.h"

struct Vec3f;

extern "C" {
f32 planeHeight(const Vec3f*, const Vec3f*, const Vec3f*);
f32 planeHeightInline(const Vec3f*, const Vec3f*, const Vec3f*);
int planeMeet(f32, const Vec3f*, const Vec3f*, const Vec3f*, Vec3f*);
int pointInTriangle(const Vec3f*, const Vec3f*, const Vec3f*, const Vec3f*, const Vec3f*);
}

#endif
