#ifndef _TENEYELLOWBOSSTAMA_BLUE_H
#define _TENEYELLOWBOSSTAMA_BLUE_H
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

enum BlueTamaState {
	BLUE_TAMA_FLY      = 0,
	BLUE_TAMA_SQUASH   = 1, // landed flattening to 0.4
	BLUE_TAMA_UNSQUASH = 2, // back to 1 then it hops again
	BLUE_TAMA_FADE     = 3, // after the third bounce
};

class TEneYellowBossTama_Blue : public TObjAction {
public:
	TEneYellowBossTama_Blue(TObject* parent, const Vec3f* pos, const Vec3f* dir);
	virtual ~TEneYellowBossTama_Blue();
	virtual void draw();                                              // _0C
	virtual void vfn_10();                                            // _10 hidden while off camera
	virtual void render();                                            // _1C
	virtual Vec3f* getPos();                                          // _20 defined in TEneYellowBossTama_ShockWave.cpp
	virtual int getKind();                                            // _2C defined in TEneYellowBossTama_ShockWave.cpp
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58

	void* operator new(unsigned long size) { return objHeapAlloc(gObjHeap, size); }
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	void placeColli();                                             // 0x801514B0
	void launchAt(const Vec3f* from, const Vec3f* to, Vec3f* vel); // 0x80151514

	SetTransform mXform; // _30
	ObjShadow mShadow;   // _74
	DrawTask* mDraw;     // _AC
	CharaColli* mShape;  // _B0
	EnemySight mSight;   // _B4 mHeightLimit is the range
	NJS_OBJECT* mModel;  // _E4
	Vec3f mDir;          // _E8 where the next hop lands
	int mState;          // _F4 a BlueTamaState
	int mBounces;        // _F8
	f32 mSquash;         // _FC its height the width grows as it shrinks
};

extern "C" {
extern u32 sBlueTamaName[2];
}

#endif
