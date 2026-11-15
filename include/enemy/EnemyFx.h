#ifndef _ENEMY_ENEMYFX_H_
#define _ENEMY_ENEMYFX_H_
struct Quat;

#include "core/types.h"

struct GjActor;
struct Vec3f;

extern "C" {
void* actorCreate(void*);
void alphaTestBegin();
void alphaTestEnd();
f32 approachSpeed(f32, f32, f32, f32, f32, f32, f32);
f32 distanceFalloff(f32);
int thresholdIndex(int, const int*);
void enemyFxBurst50(const Vec3f*);
void enemyFxDeath44(const Vec3f*);
void enemyFxLoadStub();
void enemyFxFreeStub();
int hatchActive(f32*);
void hatchClear(f32*);
int hatchDone(f32*);
void hatchPose(f32*, GjActor*, f32);
void hatchStart(f32*);
void hatchStep(f32*);
void poseEggBlend(GjActor*, f32, f32, f32);
void poseEggShape(GjActor*, f32, f32);
void poseHatch(GjActor*, f32, f32);
int rampInt(f32, f32, f32, int);
int randomInt(f32, f32);
void randomPointInRing(f32, f32, const Vec3f*, Vec3f*);
void spawnDropRun(Vec3f*, int);
void spawnDropRunEx(Vec3f*, int, int);
f32 springVelocity(f32, f32, Vec3f*, f32, const Vec3f*, f32, f32, f32);
}

extern "C" {
void fxSpawnFlat(int, void*, const Vec3f*);
}

extern "C" {
void* enemyDescGetKind(void*, s16*, s16*);
void particleKeepAt(void**, const Vec3f*, int, int);
void enemyFxKeepAt(void**, const Vec3f*, int);
void particleKeepAtRot(void**, const Vec3f*, int, int, int, int);
void enemyFxKeepAtRot(void**, const Vec3f*, int, int, int);
void rollQuat(const Vec3f*, Quat*, f32);
void actorFree(void**);
void actorApplyMotion(void*, void*);
void enemyFxBurst4E(const Vec3f*);
void enemyFxBurst4F(const Vec3f*);
void enemyFxSparks(f32, const Vec3f*, const Vec3f*);
void enemyFxDeath4A(const Vec3f*);
void enemyFxDeath44Se(const Vec3f*);
void enemyFxDeath4ASe(const Vec3f*);
}

extern "C" {
int healthDropReached(int, int, const int*, const int*);
void enemyFxSpawnAtPos(int, void*, const Vec3f*);
void fxSpawnTurned(int, void*, const Vec3f*, int, int);
void fxSpawnScaled(f32, f32, f32, int, void*, const Vec3f*, int, int);
}

#endif
