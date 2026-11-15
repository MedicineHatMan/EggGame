#ifndef _ANIMAL_ANISPAWN_H_
#define _ANIMAL_ANISPAWN_H_
struct TEggCreature;
struct Vec3f;

#include "core/types.h"

extern "C" {
void aniPreload(int);
}

extern "C" {
TEggCreature* aniSpawn(int, int, Vec3f*);
}

#endif
