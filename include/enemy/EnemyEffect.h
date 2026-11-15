#ifndef _ENEMY_ENEMYEFFECT_H_
#define _ENEMY_ENEMYEFFECT_H_

#include "core/types.h"

struct EnemyEffect;
struct Vec3f;

extern "C" {
void enemyEffectLoad();
void enemyEffectRelease();
void enemyEffectSet(f32, f32, EnemyEffect*, const Vec3f*);
void enemyEffectSetLarge(f32, EnemyEffect*, const Vec3f*);
void enemyEffectSetSmall(f32, EnemyEffect*, const Vec3f*);
void enemyEffectStart(EnemyEffect*);
void enemyEffectUpdate(EnemyEffect*);
}

#endif
