#ifndef _ENEMY_BOSS_TENEORANGEBOSSMIRROR_H_
#define _ENEMY_BOSS_TENEORANGEBOSSMIRROR_H_

#include "core/types.h"

struct Vec3f;

extern "C" {
void initOrangeBossMap();
void mirrorAimAt(int, const Vec3f*);
void mirrorCenterAimAt(const Vec3f*);
void mirrorCenterClearFlag20();
int mirrorCenterDown();
u32 mirrorCenterFlag20();
void mirrorCenterHollow();
u16 mirrorCenterId();
void mirrorCenterLower(int);
void mirrorCenterMesh(int);
void mirrorCenterMoveSe();
void mirrorCenterRaise(int);
void mirrorCenterSetFlag2();
void mirrorCenterSetFlag40();
void mirrorCenterSolid();
void mirrorCenterTurn(int);
int mirrorCenterUp();
void mirrorCopyScreen();
int mirrorIsClear(int);
int mirrorRingAllDown();
int mirrorRingAllUp();
void mirrorRingClearFlag1();
void mirrorRingLower();
void mirrorRingMoveSe();
void mirrorRingRaise();
void mirrorRingSetFlag1();
void mirrorRingSetFlag2();
void mirrorRingSetTarget(int);
int mirrorSlotAngle(int);
int mirrorSlotDir(int, Vec3f*);
int mirrorSlotFacing(f32, const Vec3f*, int*);
int mirrorSlotOf(const Vec3f*);
int mirrorSlotOpposite(const Vec3f*);
void mirrorSlotPos(int, Vec3f*, int*);
void mirrorsCreate();
void mirrorsHide();
void mirrorsReset();
void shutdownOrangeBossMap();
}

#endif
