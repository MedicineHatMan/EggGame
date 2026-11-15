#ifndef _ENEMY_ENEMYGROUP_H_
#define _ENEMY_ENEMYGROUP_H_

#include "core/types.h"

extern "C" {
void enemyGroupBlockAll();
int* enemyGroupCollect(s16, s16, int*, int);
void enemyGroupFinish(s16);
int enemyGroupFirstStep(s16, s16*);
int enemyGroupNextStep(s16, s16, s16*);
void enemyGroupPrune(int*, int*);
void enemyGroupShow(s16);
void enemyGroupSpawn(int*, int);
}

extern "C" {
int isEnemyGrouped(s16);
}

#endif
