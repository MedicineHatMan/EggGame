#ifndef _OBJECT_LAST_TOBJLASTANIMUS_H_
#define _OBJECT_LAST_TOBJLASTANIMUS_H_
#include "Ninja/ninja.h"
#include "Ninja/njtypes.h"
#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemyLight.h"
#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
#include "ui/TSimpleText.h"
struct SetDesc;
struct TObjLastAnimus;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjLastAnimus(TObject*, SetDesc*);
}

class TObjLastAnimus : public TSetActor {
public:
	TObjLastAnimus(TObject* parent, SetDesc* desc);
	virtual ~TObjLastAnimus();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc); // 0x8018EDAC
	virtual void debugPrint(void*);           // 0x8018EEB0
	virtual int sendToHandle(ObjMsg* msg);
	virtual void reset(); // _74

	void makeShape();
	void makeSphere();
	void moveShape();
	void freeShape();
	int checkTouch();
	void calcMatrix();
	void calcShadow();
	void setShadow(EggHits* hits);
	void startConversation();
	void endConversation();
	void startMode();
	void updateWait();
	void updateRun();
	int setState(int state);
	int nextState();
	void startWalk();
	void updateState();
	void stateWait();
	void stateRun();
	void stateAct();
	void stateTurn();
	void stateTalk();
	int fall();

	NJS_OBJECT* mModel;      // _64
	NJS_TEXLIST* mTexList;   // _68
	DrawTask* mDraw;         // _6C
	ObjModel mObj;           // _70
	MotionPlayer mMotion;    // _90
	ObjShadow mShadow;       // _CC
	f32 mMatrix[12];         // _104
	f32 mHop;                // _134 height above mPos
	Vec3f mScale;            // _138 y squashes when landed on
	f32 _144;                // _144
	f32 mSquashVel;          // _148 springs mScale.y back to 1
	f32 _14C;                // _14C
	ObjRef mTalker;          // _150
	Vec3f mTarget;           // _15C where it walks to
	int mTargetAngle;        // _168
	Vec3f mVel;              // _16C
	f32 mSpeed;              // _178 0.5 + set float 2
	f32 mRange;              // _17C set float 1
	EnemyLight mLight;       // _180
	TSimpleText* mText;      // _1C0
	int mMsgId;              // _1C4
	TCamConversation* mConversation; // _1C8
	int  mLightSet;         // _1CC
	int mMode;               // _1D0 0 WAIT 1 RUN
	int mState;              // _1D4 AnimusState
	int mNextState;          // _1D8
	s16 mTimer;              // _1DC
	s16 mWaitTime;           // _1DE
};

extern "C" {
extern const char* gTObjLastAnimusName;
extern const MotionEntry sLastAnimusMotionTable[];
extern const char* sLastAnimusModeNames[2];
}

#endif
