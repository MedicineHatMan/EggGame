#ifndef _SOUND_SOUND3DPLAY_H_
#define _SOUND_SOUND3DPLAY_H_
struct Vec3f;

#include "core/types.h"

extern "C" {
int sePlay3D(int, const Vec3f*);
}

extern "C" {
int sePlay3DGroup(int, const Vec3f*, int);
}

extern "C" {
int sePlay3DRange(f32, int, const Vec3f*, u16);
int sePlay3DExtra(int, const Vec3f*, u16, int);
}

#endif
