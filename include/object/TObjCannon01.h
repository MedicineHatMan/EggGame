#ifndef _OBJECT_TOBJCANNON01_H_
#define _OBJECT_TOBJCANNON01_H_
#include "Ninja/ninja.h"
#include "Ninja/njtypes.h"
#include "enemy/EnemyLight.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjCannon01;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjCannon01(TObject*, SetDesc*);
}

class TObjCannon01 : public TSetActor {
public:
	enum {
		// mAct
		ACT_WAIT    = 0, // until something is in the scope
		ACT_FIRE    = 1, // aiming then firing
		ACT_RECOVER = 2, // until the ball is gone
		// mPhase while firing
		PHASE_START = 0,
		PHASE_AIM   = 1, // turning and raising the barrel
		PHASE_OPEN  = 2, // the open motion firing at frame 20
	};

	TObjCannon01(TObject* parent, SetDesc* desc);
	virtual ~TObjCannon01();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void reset();                     // _74

	void aimMouth();
	Vec3f landingPoint();
	void aimAt(const Vec3f* target);
	int caught();
	void advanceOpen();
	void stateWait();
	void stateFire();
	void stateRecover();

	NJS_OBJECT** mModels;     // _64 stgobj_common's 3 the barrel 4 the base
	NJS_TEXLIST** mTextures;  // _68
	NJS_MOTION* mOpenMotion;  // _6C the barrel opening to fire
	DrawTask* mDraw;          // _70
	CharaColli* mScopeColli;  // _74 the scope's cylinder
	ObjRef mTarget;           // _78 what is in the scope
	int mAct;                 // _84
	int mPhase;               // _88
	f32 mScale;               // _8C
	f32 mPower;               // _90 the shot's speed along mDir
	Vec3f mDir;               // _94 the set aim
	f32 mOpen;                // _A0 the open motion's frame
	f32 mOpenStep;            // _A4
	int mPitch;               // _A8 the barrel's
	Vec3f mMouth;             // _AC where shots leave
	Vec3f mShotVel;           // _B8 the next shot's velocity
	Vec3f mScope;             // _C4 the scope's middle where the set aim lands
	Vec3f mFoot;              // _D0 the model's middle for the view test
	f32 mHeight;              // _DC the model's radius
	EnemyLight mLight;        // _E0
};

/** What a shot falls by each frame for the cannon and its balls */
#define CANNON_GRAVITY 0.16333334f

/**
 * A cannon's shot. It flies until it comes down through the height of the spot it was aimed at, bursts into a
 * TObjBlast there, and deletes itself once the blast is gone. The name is ours.
 *
 * @size 0x7C
 */
class TCannonBall : public TObject {
public:
	enum {
		// mState
		STATE_FLY   = 0,
		STATE_BURST = 1,
		STATE_GONE  = 2, // until the blast is gone
	};

	TCannonBall(TObject* parent, const Vec3f* pos, const Vec3f* vel, const Vec3f* target, f32 scale);
	virtual ~TCannonBall();

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	void fly();
	void burst();
	void waitGone();

	int mState;          // _20
	Vec3f mTarget;       // _24
	f32 mScale;          // _30
	SetTransform mXform; // _34
	DrawTask* mDraw;     // _78
};

typedef char TCannonBall_size_check[(sizeof(TCannonBall) == 0x7C) ? 1 : -1];

#endif
