#ifndef _ANIMAL_TANIRACEPATH_H_
#define _ANIMAL_TANIRACEPATH_H_
#include "egg/ObjEgg.h"
#include "stage/Stage.h"
struct AniRaceDesc;
struct TAniRacePath;
class TAnimalRacer;
struct Vec3f;

#include "core/types.h"

extern "C" {
void aniRaceClearRacer(TAnimalRacer*);
void aniRaceEnd();
void aniRaceFinish();
u16 aniRaceGetCourse();
u16 aniRaceGetLaps();
u16 aniRaceGetRank();
void aniRaceInit();
void aniRaceMake();
void aniRacePlaceFirst();
void aniRacePlaceSecond();
void aniRacePlaced();
void aniRaceReset();
void aniRaceRun();
void aniRaceSetGlobal(int);
void aniRaceSetSpeed(f32);
void aniRaceToStart();
}

extern "C" {
const Vec3f* aniRaceGetPos();
void aniRaceClearPath(TAniRacePath*);
}

extern "C" {
int aniRaceHasState();
int aniRaceSetup(AniRaceDesc*);
}

extern "C" {
int aniRaceSetupMake();
int aniRaceStart();
}

/**
 * @size 0x5C
 */
class TAniRacePath : public TObject {
public:
	TAniRacePath(TObject* parent);
	virtual ~TAniRacePath();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void reset();
	void toStart();
	void runRail();
	void runPlaced();
	void finish();

	EggCollide mRail; // _20
	u8 mPlace;        // _3C 1 or 2
	u8 _3D[3];
	int mTiming;      // _40
	int mClock;       // _44 in sixteenths while mTiming is set
	int mState;       // _48
	Vec3f mPos;       // _4C
	f32 mSpeed;       // _58
};

extern "C" {
extern const char* gTAniRacePathName;
extern int sAniRaceGlobal;
extern Vec3f sRacePathStart;
}

#endif
