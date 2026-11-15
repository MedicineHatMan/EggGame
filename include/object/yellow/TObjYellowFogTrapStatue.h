#ifndef _OBJECT_YELLOW_TOBJYELLOWFOGTRAPSTATUE_H_
#define _OBJECT_YELLOW_TOBJYELLOWFOGTRAPSTATUE_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "camera/TCamera.h"
#include "engine/Fx.h"
#include "engine/MotionPlayer.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjYellowFogTrapStatue;

#include "core/types.h"

class SetDesc;
class TObject;

extern "C" {
TObject* createTObjYellowFogTrapStatue(TObject*, SetDesc*);
int triggerFogTrapStatue();
}

class TObjYellowFogTrapStatue : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjYellowFogTrapStatue(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowFogTrapStatue();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void updatePlace();
	int startEvent();
	int startCamera();
	void startBreath();
	void stopBreath();
	void stateWait();
	void statePlay();
	void stateOff();

	NJS_OBJECT* mModelData; // _64
	NJS_TEXLIST* mTexList;  // _68
	DrawTask* mDrawTask;    // _6C
	ObjModel mModel;        // _70
	MotionPlayer mMotion;   // _90
	f32 mMatrix[12];        // _CC
	u16 mSwitchId;          // _FC
	TCamera* mEvent;        // _100 whichever of the next two is running
	TCamPath* mFullEvent;   // _104
	TCamera* mCamera;       // _108 a TCamFix
	Fx* mBreath;            // _10C
	f32 mBreathMtx[12];     // _110
	int mFrame;             // _140
	int mStep;              // _144
	Vec3f mCenter;          // _148
	f32 mRadius;            // _154
	int mFirstTime;         // _158
	int mAct;               // _15C 0 = wait 1 and 2 = play 3 = off
};

extern "C" {
extern const char* gTObjYellowFogTrapStatueName;
extern void* sFogTrapStatueObj;
}

#endif
