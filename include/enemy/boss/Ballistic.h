#ifndef _ENEMY_BOSS_BALLISTIC_H_
#define _ENEMY_BOSS_BALLISTIC_H_
#include "core/types.h"

struct Vec3f;

extern "C" {
f32 launchSpeedInSteps(int, f32, f32);
f32 gravityInSteps(int, f32, f32);
f32 speedForJumpHeight(f32, f32);
f32 launchSpeedFor(f32, f32, f32);
void ballisticSolve(f32, f32, f32, f32*, f32*);
void ballisticRoots(f32, f32, f32, f32*, f32*);
f32 flatVelocityTo(const Vec3f*, const Vec3f*, Vec3f*, f32);
void parabolaPoint(const Vec3f*, const Vec3f*, Vec3f*, f32, f32);
}

#endif
