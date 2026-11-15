#ifndef _ANIMAL_TANIBIRTH_H_
#define _ANIMAL_TANIBIRTH_H_

#include "core/types.h"

#include "stage/Stage.h"

// The hop an animal makes out of its egg. A constructor fills it, so it is set by the file's __sinit.
struct BirthJump : public Vec3f {
	BirthJump(f32 nx, f32 ny, f32 nz) { set(nx, ny, nz); }
};

extern "C" {
extern const char* gTAniBirthName;
}

extern BirthJump sBirthJump;

#endif
