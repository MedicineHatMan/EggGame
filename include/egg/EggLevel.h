#ifndef _EGG_EGGLEVEL_H_
#define _EGG_EGGLEVEL_H_
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct EggColl;
struct EggContact;
class TMobileObject;
struct TObjEgg;
struct Vec3f;

#include "core/types.h"

class TArc;
struct EggTable;

extern "C" {
void loadEggLevels();
extern TArc* sEggLevelArc;
extern EggTable* sEggLevelTable;
void unloadEggLevels();
}

extern "C" {
int eggFindHit(EggColl*, TObjEgg*, Vec3f*);
int eggFindHatchTouch(EggColl*, u16*);
void eggPushHeld(void*, EggContact*, EggContact*, TMobileObject*);
void eggSpawnShockWave(f32, u16, u16, Vec3f*, int, void*);
int eggFirstDamage(void*, EggContact*, EggContact*, void*);
int eggAnyTouch(void*, EggContact*, EggContact*);
}

struct EggContact {
	EggContact* mNext; // _00
	u8 _04[8];
	Vec3f mPos;        // _0C
	int mShape;        // _18
	Vec3f mNormal;     // _1C
};

struct EggColl {
	u8 _00[0x80];
	EggContact* mContacts; // _80
};

#endif
