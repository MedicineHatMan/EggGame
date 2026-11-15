#ifndef _OBJECT_TOBJSHARK01_H_
#define _OBJECT_TOBJSHARK01_H_
#include "Ninja/ninja.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjShark01;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjShark01(TObject*, SetDesc*);
}

class TObjShark01 : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjShark01(TObject* parent, SetDesc* desc);
	virtual ~TObjShark01();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int onHit(const TObjAction* other, int, int* info);
	virtual void vfn_74();

	int findPlayer();
	void stateSwim();
	void stateChase();
	void stateLeap();
	void chaseTarget();
	void pickTarget();
	void swimToTarget();
	void startLeap();
	void updateActor();
	void endChase();

	NJS_TEXLIST* mTexList;       // _64
	GjActor* mActor;         // _68
	MotionPlayer mMotion; // _6C
	ObjRef mPlayer;       // _A8
	DrawTask* mDrawTask;      // _B4
	Vec3f mVel;           // _B8
	Vec3f mHome;          // _C4
	f32 mRadius;          // _D0 how far it swims from home
	CharaColli* mTerritory;     // _D4
	Vec3f mTarget;        // _D8
	int mAct;             // _E4 0 = swim 1 = chase 2 = leap
	int mMotionId;        // _E8
	int mSwimTimer;       // _EC
	int mSplashed;        // _F0
	f32 mSpeed;           // _F4
	int mTurnRate;        // _F8
};

extern "C" {
extern const char* gTObjShark01Name;
}

#endif
