#ifndef _OBJECT_TOBJEGGCANNON_H_
#define _OBJECT_TOBJEGGCANNON_H_
#include "Ninja/ninja.h"
#include "camera/TCamera.h"
#include "enemy/EnemyLight.h"
#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "engine/dlist.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "sound/SoundHandle.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct SetDesc;
struct TObjEggCannon;
struct TObject;

#include "core/types.h"

extern "C" {
void initTObjEggCannon();
void termTObjEggCannon();
}

extern "C" {
TObject* createTObjEggCannon(TObject*, SetDesc*);
}

class TObjEggCannon : public TSetActor {
public:
	enum State {
		CANNON_WAIT  = 0, // empty waiting for an egg to be pushed in
		CANNON_AIM   = 1, // holding one aimed by the stick
		CANNON_FIRED = 2, // waiting mFixTime frames after a shot
	};

	enum Flag {
		CANNON_DRAW_MODEL = 0x1,
		CANNON_DRAW_SCOPE = 0x2,
	};

	TObjEggCannon(TObject* parent, SetDesc* desc);
	virtual ~TObjEggCannon();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                            // _0C
	virtual void vfn_10();                          // _10
	virtual void render();                          // _1C
	virtual void getNodeMatrix(int node, f32* mtx); // _24 an egg in the barrel goes here
	virtual void initFromDesc(SetDesc* desc);       // _30
	virtual void debugPrint(void*);                 // _34
	virtual int sendToHandle(ObjMsg* msg);          // _4C
	virtual void init();                            // _74

	void makeShape();
	void placeSpheres();
	void freeShape();
	void setState(int state);
	void stateWait();
	void stateAim();
	void stateFired();
	void startCamera();
	void endCamera();
	void updateCamera();
	BOOL checkEgg();
	BOOL fire();
	void updateMatrix();
	void drawScope();

	DrawTask* mDraw;           // _64
	ObjDrawState mDrawState;   // _68
	ObjModel mModel;           // _8C
	f32 mMtx[12];              // _AC
	MotionPlayer mMotion;      // _DC
	int mPitch;                // _118
	int mYaw;                  // _11C
	int mRoll;                 // _120
	int mPitchMin;             // _124
	int mYawMin;               // _128
	int mRollMin;              // _12C
	int mPitchMax;             // _130
	int mYawMax;               // _134
	int mRollMax;              // _138
	Vec3f mLightPos;           // _13C
	f32 mPower;                // _148
	u16 mEggId;                // _14C 0xFFFF = none
	TObjAction* mEgg;          // _150
	TCamFix* mCamera;          // _154
	int mFixTime;              // _158
	ObjShadow mShadow;         // _15C
	SoundHandle mSnd;          // _194
	s16 mTimer;                // _1C0
	EnemyLight mLight;         // _1C4
	u32 mFlags2;               // _204 Flag
	int mState;                // _208 State
};

#endif
