#ifndef _ENEMY_BOSS_TENEORANGEBOSSFLOOR_H_
#define _ENEMY_BOSS_TENEORANGEBOSSFLOOR_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "egg/ObjEgg.h"
#include "stage/Stage.h"
struct TEneOrangeBossFloor;
struct TObject;

#include "core/types.h"

extern "C" {
void orangeFloorClear();
void orangeFloorEnd();
void orangeFloorForget(void*);
void orangeFloorPhase0();
void orangeFloorPhase1();
void orangeFloorPhase2();
void orangeFloorStart();
}

extern "C" {
TEneOrangeBossFloor* createTEneOrangeBossFloor(TObject*);
}

class TEneOrangeBossFloor : public TObject {
public:
	TEneOrangeBossFloor(TObject* parent);
	virtual ~TEneOrangeBossFloor();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	HitVec mPoint[8];    // _20
	int mAngle[8];       // _80
	int mIndex;          // _A0
	f32 mFrame;          // _A4
	DrawTask* mDraw;     // _A8
	NJS_MOTION* mMotion; // _AC

	void setPhase(int phase);
};

extern "C" {
extern const int gOrangeFloorModels[3];
}

#endif
