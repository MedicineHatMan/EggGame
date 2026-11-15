#ifndef _TENEYELLOWBOSSTAMA_ORANGE_H
#define _TENEYELLOWBOSSTAMA_ORANGE_H
struct CharaColli;

#include "object/ObjAction.h"
struct DrawTask;
struct NJS_OBJECT;
#include "stage/SetManager.h"
#include "object/ObjCommon.h"
#include "enemy/EnemySight.h"

extern "C" {
void* objHeapAlloc(void*, u32);
void objHeapFree(void*, void*);
extern void* gObjHeap;
}

enum OrangeTamaState {
	ORANGE_TAMA_HELD  = 0, // in the crystal's hand
	ORANGE_TAMA_FLY   = 1,
	ORANGE_TAMA_SWELL = 2, // landed an unseen sphere that grows to 40
};

class TEneYellowBossTama_Orange : public TObjAction {
public:
	TEneYellowBossTama_Orange(TObject* parent, const Vec3f* pos);
	virtual ~TEneYellowBossTama_Orange();
	virtual void draw();                                              // _0C
	virtual void vfn_10();                                            // _10 hidden while off camera or swelling
	virtual void render();                                            // _1C
	virtual Vec3f* getPos();                                          // _20 defined in TEneYellowBossTama_ShockWave.cpp
	virtual int getKind();                                            // _2C defined in TEneYellowBossTama_ShockWave.cpp
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58

	void* operator new(unsigned long size) { return objHeapAlloc(gObjHeap, size); }
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	void placeColli();                            // 0x80150C64
	void placeShadow();                           // 0x80150CB0
	void setPos(const Vec3f* pos);                // 0x80150CE0 carried in the crystal's hand
	void throwAt(const Vec3f* target, int aimed); // 0x80150CFC aimed thrown at the player not a random point

	SetTransform mXform; // _30 mForce.y is the gravity
	ObjShadow mShadow;   // _74
	DrawTask* mDraw;     // _AC
	CharaColli* mShape;  // _B0
	EnemySight mSight;   // _B4 mHeightLimit is the range
	NJS_OBJECT* mModel;  // _E4
	int mAimed;          // _E8
	Vec3f mDrift;        // _EC the step after the player
	Vec3f mTarget;       // _F8 where it was thrown then where the player was last frame
	int mState;          // _104 an OrangeTamaState
	int mTimer;          // _108 frames at full size
	f32 mRadius;         // _10C
};

extern "C" {
extern u32 sOrangeTamaName[2];
}

#endif
