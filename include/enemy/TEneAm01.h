#ifndef _TENEAM01_H
#define _TENEAM01_H

#include "enemy/ObjEnemy.h"
struct DrawTask;
#include "object/ObjCommon.h"
#include "enemy/EnemySight.h"
#include "engine/RockFlight.h"
#include "enemy/TEneAm01Parts.h"

class TEneAm01;

/** Decides what a small enemy does next as a set of states that each run in phases 0 on entry 1 each frame 2 to
 * check for a change 3 on exit */
struct EnemyBrain {
	enum State {
		STATE_APPEAR = 0,
		STATE_FLIP   = 2,
		STATE_DIE    = 3,
		STATE_WANDER = 4,
		STATE_CHASE  = 5,
	};

	int mState;       // _00
	int mNext;        // _04 an action to ask the owner for
	TEneAm01* mOwner; // _08

	inline EnemyBrain()
	{
		mOwner = 0;
		mState = 0;
		mNext = -1;
	}

	virtual int checkInterrupt() = 0;               // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~EnemyBrain();                   // _10
	virtual void update(TEneAm01* owner);           // _14
	virtual void onAppearDone() = 0;                // _18

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** The states every brain here shares and what's needed to find a player */
struct EnemySeekBrain : public EnemyBrain {
	EnemySeekBrain();
	inline virtual ~EnemySeekBrain();

	virtual int checkInterrupt();           // _08
	virtual void onFlipDone() = 0;          // _1C
	virtual void onDeathStart();            // _20
	virtual void attach(TEneAm01* owner) = 0; // _24

	void phaseAppear(int phase);
	void phaseFlip(int phase);
	void phaseDie(int phase);

	EnemySight mSight;   // _10
	EnemyMemory mMemory; // _40
	int mStep;           // _4C
	int mTimer;          // _50
};

/** TEneAm01's variant 0 Walks around its home stopping now and then and runs at a player it sees */
struct Am01WanderBrain : public EnemySeekBrain {
	Am01WanderBrain();
	virtual ~Am01WanderBrain();

	virtual void onPhase(int state, int phase);
	virtual void onAppearDone();
	virtual void onFlipDone();
	virtual void attach(TEneAm01* owner);

	void pickTarget();
	int look();
	void phaseWander(int phase);
	void phaseChase(int phase);
};

/** TEneAm01's variant 1 Leaps around its home and at a player it sees */
struct Am01LeapBrain : public EnemySeekBrain {
	Am01LeapBrain();
	virtual ~Am01LeapBrain();

	virtual void onPhase(int state, int phase);
	virtual void onAppearDone();
	virtual void onFlipDone();
	virtual void attach(TEneAm01* owner);
	virtual void startWander(); // _28

	void pickTarget();
	int look();
	void startAiming();
	void phaseLeap(int phase);
};

/**
 * The commonest small enemy, in every normal theme.
 *
 * @size 0x264
 */
class TEneAm01 : public TObjEnemy {
public:
	enum Action {
		ACT_APPEAR       = 0,
		ACT_APPEAR_AIMED = 1,
		ACT_STAGGER      = 2,
		ACT_IDLE         = 3,
		ACT_WALK         = 4,
		ACT_CHASE        = 5,  // as walk but brakes on sharp turns and notices the player touching it
		ACT_LEAP         = 6,
		ACT_8            = 8,  // empty
		ACT_FLIP         = 9,
		ACT_DIE          = 10,
		ACT_DIE_CRUSHED  = 11,
		ACT_DIE_THROWN   = 12,
	};

	enum {
		// mFlags2
		AM_ARRIVED    = 0x1,
		AM_TOUCHED    = 0x2,  // a player touched it
		AM_FLIP_DONE  = 0x4,
		AM_SPIN       = 0x20, // draws with mBody.mRot.x
		AM_PASSED     = 0x40, // the leap has gone past its target
		AM_SQUASH     = 0x80, // draws with mSquash
		AM_LEAP_AIMED = 0x100,
		AM_STAGGERED  = 0x200,
		AM_SIMPLE_HIT = 0x400, // collide without the slope filter
		AM_CRUSHED    = 0x1000,
		AM_THROWN     = 0x2000,
		AM_NO_DROP    = 0x8000,
	};

	TEneAm01(TObject* parent, SetDesc* desc);
	TEneAm01(TObject* parent, int model, EnemySeekBrain* brain);
	virtual ~TEneAm01();

	inline void* operator new(u32 size) { return objHeapAlloc(gObjHeap, size); }
	inline void operator delete(void* mem);

	virtual void vfn_10();                     // _10 hides it off screen
	virtual void render();                     // _1C draws the model
	virtual void readEnemyDesc(SetDesc* desc); // _38
	virtual void vfn_3C();                     // _3C prints debug text
	virtual f32 vfn_40();                      // _40
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58
	virtual void vfn_6C();                     // _6C
	virtual void updateBody();                 // _70
	virtual void setPos(const Vec3f* pos);           // _74
	virtual void syncCollider();               // _78
	virtual void onActionPhase(int action, int phase); // _7C
	virtual int showsHitEffect();              // _AC

	void init();
	int isAppearDone();
	int isAppearAimedDone();
	int isStaggerDone();
	void setTarget(const Vec3f* target);
	int hasArrived();
	int wasTouched();
	void setLeapAimed();
	void clearLeapAimed();
	int isFlipDone();
	int pickDeath();
	int pickVariant(SetDesc* desc);
	void createBrain();
	int moveStep(f32 dirX, f32 dirZ);
	void checkPlayerTouch();
	void resetCollider();
	void noOp();
	void beginDeath();
	void setModel(int index);
	void setVelocity(const Vec3f* vel);
	void dropItem();
	void startEffect();

	void actAppear(int phase);
	void actAppearAimed(int phase);
	void actStagger(int phase);
	void actIdle(int phase);
	void actWalk(int phase);
	void actLeap(int phase);
	void act8(int phase);
	void actFlip(int phase);
	void actDie(int phase);
	void actDieCrushed(int phase);
	void actDieThrown(int phase);

	int mVariant;             // _158 0 or 1 Picks the brain
	EnemySeekBrain* mBrain;   // _15C
	void* mPose;              // _160
	DrawTask* mModel;             // _164 the task from addDrawTask
	void* mTexture;           // _168
	u32 mFlags2;              // _16C
	f32 mTopSpeed;            // _170
	f32 mSpeed;               // _174
	Vec3f mTarget;            // _178
	Vec3f mLeapTarget;        // _184
	Vec3f mHitPos;            // _190
	f32 mSquash;              // _19C
	int _1A0;                 // _1A0
	int mLastDamage;          // _1A4 the damage of a throw
	RockFlight* mThrow;       // _1A8 moves it through the air when thrown
	int mSlot;                // _1AC
	EnemyCrush mCrush;        // _1B0
	EnemyBreak* mBreak;       // _1C0 its death arc
	EnemyTint mTint;          // _1C4
	ObjShadow mShadow;        // _1CC
	EnemyEffect mEffect;      // _204
	int mStep;                // _254
	int mTimer;               // _258
	int mAngle;               // _25C
	f32 mDist;                // _260
};

extern "C" TEneAm01* createTEneAm01WithBrain(TObject* parent, int model, EnemySeekBrain* brain);

class SetDesc;

extern "C" {
extern const int kEnemyInfoX;
extern const int kEnemyInfoLine;
}

#endif
