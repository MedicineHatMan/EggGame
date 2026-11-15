#ifndef _EGG_TEGGCREATURETYPEE_H_
#define _EGG_TEGGCREATURETYPEE_H_
struct TArc;
struct TResource;

#include "core/types.h"

class TEggCreature;
class TEggCreatureTypeE;
struct Vec3f;

extern "C" {
TEggCreature* aniCreate(int, int, Vec3f*);
void aniFreeFiles();
void aniLoadFiles();
const char* aniModelArcName(int);
void eggTypeStopAllFx(TEggCreatureTypeE*);
}

extern "C" {
extern int sAnimalModelArcCount;
extern int sAnimalParamArcCount;
extern const char* sAnimalModelArcs[];
extern const char* sAnimalParamArcs[];
extern TArc* sAnimalParamArcRes[];
extern TArc* sAnimalCommonArc;
extern TResource* sAnimalCommonRes;
extern const char* gTEggCreatureTypeEName;
}

#endif
