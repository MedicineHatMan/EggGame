#ifndef _OBJECT_MISSION_TMSNOBJCHICKEN_H_
#define _OBJECT_MISSION_TMSNOBJCHICKEN_H_
struct ColliShape;
#include "camera/TCamera.h"
struct NJS_MOTION;
struct NJS_OBJECT;
#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct SetDesc;
struct TObject;
struct TMsnObjChicken;

#include "core/types.h"

extern "C" {
void chickenBuild(TMsnObjChicken*);
}

extern "C" {
TObject* createTMsnObjChicken(TObject*, SetDesc*);
}

struct TArc;
struct NJS_TEXLIST;

// The set-init pair that loads geobj_chicken.arc
extern "C" {
void clearChickenArc();
int loadChickenArc();
void termChickenArc();

extern TArc* gChickenArc;
extern NJS_OBJECT* gChickenModel;
extern NJS_TEXLIST** gChickenTexLists;
extern NJS_MOTION** gChickenMotions;
extern int sChickenLoaded;
}

class TMsnObjChicken : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TMsnObjChicken(TObject* parent, SetDesc* desc);
	virtual ~TMsnObjChicken();

	virtual void draw();   // _0C the update
	virtual void vfn_10(); // _10 hides it off camera
	virtual void render(); // _1C
	virtual int getKind(); // _2C
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);

	void placeMatrix();
	void waitRider();
	void takeRider();
	void leave();
	void fly();

	ObjModel mModel;      // _64
	MotionPlayer mMotion; // _84
	DrawTask* mDrawTask;  // _C0
	f32 mMatrix[12];      // _C4
	int mType;            // _F4
	int mState;           // _F8
	ObjRef mRider;        // _FC
	int mRiderAngle;      // _108
	u16 mUnlockSwitch;    // _10C
	ObjShadow mShadow;    // _110
	TCamPath* mCamera;    // _148
	u16 mTimer;           // _14C
	int mLights;          // _150
};

extern "C" {
extern MotionEntry sMsnChickenMotions[4];
extern ColliShape sMsnChickenShape[1];
}

#endif
