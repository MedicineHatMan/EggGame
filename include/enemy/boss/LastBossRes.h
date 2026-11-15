#ifndef _ENEMY_BOSS_LASTBOSSRES_H_
#define _ENEMY_BOSS_LASTBOSSRES_H_

#include "core/types.h"
struct NJS_OBJECT;

class SetDesc;
class TObject;

extern "C" {
TObject* createTObjLastBossInd(TObject*, SetDesc*);
extern int gLastBossIndCount;
extern int* gLastBossIndModels;
extern int gLastBossIndShown;
extern int gLastBossIndTex;
void lastBossFree();
NJS_OBJECT* lastBossGetModel(int);
void lastBossIndHide();
void lastBossIndShow();
void lastBossLoad();
void lastExBossFree();
void lastExBossLoad();
}

#endif
