#ifndef _OBJECT_TOBJHIYOKO_H_
#define _OBJECT_TOBJHIYOKO_H_
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
struct TObjHiyoko;
struct TObject;

#include "core/types.h"

extern "C" {
void hiyokoMatColor(int, const u32*);
void initTObjHiyoko();
void termTObjHiyoko();
}

extern "C" {
TObject* createTObjHiyoko(TObject*, SetDesc*);
}

class TObjHiyoko : public TSetActor {
public:
	TObjHiyoko(TObject* parent, SetDesc* desc);
	virtual ~TObjHiyoko();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
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
	void stateHopUp();
	void stateHopDown();
	void stateAct();
	void stateTurn();
	void stateTalk();
	int fall();

	NJS_OBJECT* mModels[6];     // _64 the body then one per equip part
	NJS_TEXLIST* mTexLists[6];  // _7C
	DrawTask* mDraw;            // _94
	ObjModel mObj;              // _98
	ObjDrawState mEquip1State;  // _B8
	ObjDrawState mEquip2State;  // _DC its colour also tints the body
	ObjDrawState mEquip8State;  // _100
	ObjDrawState mEquip10State; // _124
	MotionPlayer mMotion;       // _148
	ObjShadow mShadow;          // _184
	f32 mMatrix[12];            // _1BC
	f32 mHop;                   // _1EC height above mPos
	Vec3f mScale;               // _1F0 y squashes when landed on
	f32 _1FC;                   // _1FC
	f32 mSquashVel;             // _200 springs mScale.y back to 1
	f32 _204;                   // _204
	ObjRef mTalker;             // _208
	Vec3f mTarget;              // _214 where it runs to
	int mTargetAngle;           // _220
	Vec3f mVel;                 // _224
	f32 mSpeed;                 // _230 0.5 + set float 2
	f32 mRange;                 // _234 50 + set float 1
	u32 mEquip;                 // _238 from sHiyokoEquip 1 2 4 8 0x10 each draw a part
	EnemyLight mLight;          // _23C
	TSimpleText* mText;         // _27C
	int mMsgId;                 // _280
	TCamConversation* mConversation; // _284
	int mMode;                  // _288 0 WAIT 1 RUN
	int mState;                 // _28C HiyokoState
	int mNextState;             // _290
	s16 mTimer;                 // _294
	s16 mWaitTime;              // _296
	int  mLightSet;            // _298
};

extern "C" {
extern const char* gTObjHiyokoName;
extern const MotionEntry sHiyokoMotionTable[];
}

#endif
