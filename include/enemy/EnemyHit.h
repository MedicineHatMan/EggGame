#ifndef _ENEMY_ENEMYHIT_H_
#define _ENEMY_ENEMYHIT_H_
struct TArc;
struct EnemyContact;
struct EnemyCooldownList;
struct EnemyHit;
struct EnemyTouchList;
struct Vec3f;

#include "core/types.h"

extern "C" {
void* enemyParamGet(int);
void enemyParamLoad();
void enemyParamRelease();
}

extern "C" {
void touchListInit(EnemyTouchList*);
void touchListAdd(EnemyTouchList*, u32);
int touchListHas(EnemyTouchList*, u32);
void cooldownListInit(EnemyCooldownList*);
void cooldownListAdd(EnemyCooldownList*, u32);
int cooldownListHas(EnemyCooldownList*, u32);
void cooldownListUpdate(EnemyCooldownList*);
void enemyHitInit(EnemyHit*);
void enemyHitPushAway(Vec3f*, const Vec3f*, const Vec3f*, Vec3f*);
u16 enemyHitPlayer(EnemyHit*);
void enemyContactInit(EnemyContact*);
}

extern "C" {
int enemyHitTestFlag(EnemyHit*, int);
}

/** The tuning every enemy shares, from enemyParamGet(0) */
struct EnemyCommonParams {
	int _00;          // _00
	int mCooldown;    // _04 frames before an enemy looks again
	int mChase;       // _08 frames an enemy keeps chasing
	int mDropAFlag;   // _0C
	int mDropACount;  // _10
	int mDropBFlag;   // _14
	int mDropBCount;  // _18
	f32 mFilterX;     // _1C
	int mFilterAngle; // _20
};

extern "C" {
extern TArc* sEnemyHitArc;
extern EnemyCommonParams* gEnemyParams;
extern f32 gEnemyFilterX;
extern f32 gEnemyFilterY;
extern f32 gEnemyFilterZ;
}

#endif
