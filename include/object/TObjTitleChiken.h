#ifndef _TOBJTITLECHIKEN_H
#define _TOBJTITLECHIKEN_H
struct SetDesc;
struct TObject;

#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The chicken on the title screen. The spelling is the one stored in the DOL. Set float 0 is the chicken
 * scale and set float 1 the crow scale, each clamped to -1..10 and then added to 1. While gTitleScene is 3
 * it wears the crow model and motion. It rides path 0 and turns to face it.
 *
 * @size 0x158
 */
class TObjTitleChiken : public TSetActor {
public:
	TObjTitleChiken(TObject* parent, SetDesc* desc);
	virtual ~TObjTitleChiken();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void vfn_74();                    // _74

	void setup();    // ours
	void facePath(); // ours

	DrawTask* mDraw;      // _64
	ObjModel mModel;      // _68
	MotionPlayer mMotion; // _88
	EggCollide mPath;     // _C4 path 0
	f32 mMatrix[12];      // _E0
	ObjShadow mShadow;    // _110
	int mYaw;             // _148 the path heading
	int mCrow;            // _14C 1 while gTitleScene is 3
	f32 mScale[2];        // _150 by mCrow set float 0 plus 1 set float 1 plus 1
};

extern "C" {
TObject* createTObjTitleChiken(TObject*, SetDesc*);
}

extern "C" {
extern const char* gTObjTitleChikenName;
extern const MotionEntry sTitleChikenMotions[];
}

#endif
