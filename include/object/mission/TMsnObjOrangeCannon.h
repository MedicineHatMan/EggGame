#ifndef _OBJECT_MISSION_TMSNOBJORANGECANNON_H_
#define _OBJECT_MISSION_TMSNOBJORANGECANNON_H_
struct TArc;
#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemyLight.h"
#include "engine/DrawTask.h"
#include "engine/Fx.h"
#include "engine/FxHolder.h"
#include "engine/ModelDraw.h"
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "object/orange/TObjOrangeFireworks.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct CannonNode;
struct SetDesc;
struct TMsnObjOrangeCannon;
struct TObject;

#include "core/types.h"

extern "C" {
void loadOrangeCannonAssets();
void releaseOrangeCannonAssets();
void resetOrangeCannonAssets();
}

extern "C" {
extern f32 gOrangeCannonSetRange; // 0x802C45E0
TObject* createTMsnObjOrangeCannon(TObject*, SetDesc*);
}

extern "C" {
int prepareOrangeCannonAssets();
}

/** A model node as the cannon's archives hold it */
struct CannonNode { // size 0x38
	u32 mFlags;           // _00
	void* mModel;         // _04
	Vec3f mPos;           // _08
	SetRot mAng;          // _14
	Vec3f mScale;         // _20
	CannonNode* mChild;   // _2C
	CannonNode* mSibling; // _30
	f32 _34;              // _34
};

class TMsnObjOrangeCannon : public TSetActor {
public:
	TMsnObjOrangeCannon(TObject* parent, SetDesc* desc);
	virtual ~TMsnObjOrangeCannon();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);

	void setUp();
	void updateWait();
	void updateStart();
	void updateFire();
	void updateDone();
	void stepOpen();
	void stepAim();
	void stepShoot();
	void stepWait();
	void stepEnd();

	DrawTask* mDrawTask;  // _64
	ObjModel mModel;      // _68
	ObjDrawState mState;  // _88
	MotionPlayer mMotion; // _AC
	f32 mMatrix[12];      // _E8
	ObjShadow mShadow;    // _118
	EnemyLight mLight;    // _150
	Vec3f mLightPos;      // _190
	f32 mLightRadius;     // _19C
	HitVec mMuzzle[2];    // _1A0 the two barrel nodes' places in the world
	FxVec mMuzzleRot[3];  // _1B8 their angles
	CannonNode mBurst;    // _1DC the burst model without its children
	ModelDraw mDraw;      // _214
	MotionTrack mTrack;   // _230
	TCamPath* mEvent;     // _24C
	TObjOrangeFireworks* mFireworks; // _250
	u16 mTimer;           // _254
	Fx* mFx;            // _258
	f32 mFxMtx[12];       // _25C the burst node's matrix
	ObjRef mPlayer;       // _28C who pushed the ball in
	int mAct;             // _298 0 waiting for a ball 1 starting the event 2 firing 3 done
	int mStep;            // _29C the firing step
};

#include "Ninja/njtypes.h"
extern "C" {
extern const MotionEntry sMsnOrangeCannonMotions[];
extern TArc* gOrangeCannonArc;
extern NJS_OBJECT* gOrangeCannonModels;
extern NJS_TEXLIST* gOrangeCannonTexLists;
extern NJS_MOTION** gOrangeCannonMotions;
extern void* gOrangeCannonPath;
}

#endif
