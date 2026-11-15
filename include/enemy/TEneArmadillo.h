#ifndef _TENEARMADILLO_H
#define _TENEARMADILLO_H
struct EnemyFrameKey;
struct SetDesc;
struct TObject;
struct DrawTask;

#include "enemy/ObjEnemy.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemySight.h"
#include "enemy/EnemyLight.h"
#include "engine/FxHolder.h"
#include "sound/SoundHandle.h"
#include "enemy/TEneAm01Parts.h"
#include "ui/TEnemyLifeBar.h"

class TEneArmadillo;

/** The state machine laid out like BatBrain three words then the vtable at _0C */
struct ArmadilloBrain {
	enum State {
		STATE_APPEAR = 0,
		STATE_WANDER = 1,
		STATE_ROLL   = 2, // rolls at the player it saw
		STATE_CURL   = 3, // the three that follow a hit once the charge count runs out
		STATE_HOLD   = 4,
		STATE_UNCURL = 5,
		STATE_HURT   = 6,
		STATE_DIE    = 7,
	};

	int mState;            // _00
	int mNext;             // _04 an action to ask the owner for
	TEneArmadillo* mOwner; // _08

	inline ArmadilloBrain()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt()               = 0; // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~ArmadilloBrain() { }            // _10 0x800F8C74
	virtual void update(TEneArmadillo* owner);      // _14 0x800F7EA8

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** No fields of its own */
struct ArmadilloBrainBase : public ArmadilloBrain {
	ArmadilloBrainBase();
	virtual ~ArmadilloBrainBase(); // 0x800F7F74
};

/**
 * The armadillo's brain. Walks about its home, and rolls at a player that comes close. After a hit it counts down,
 * then curls up, holds, and uncurls.
 *
 * @size 0x5C
 */
struct ArmadilloHuntBrain : public ArmadilloBrainBase {
	ArmadilloHuntBrain(TEneArmadillo* owner);
	virtual ~ArmadilloHuntBrain();

	static ArmadilloHuntBrain* create(TEneArmadillo* owner); // 0x800F7FD0

	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void pickTarget();
	int look(f32 rangeSq, int angle);
	int lookNear();
	int lookFar();
	void aim();
	void resetCharge();
	void startCharge();
	int chargeDone();

	void phaseAppear(int phase);
	void phaseWander(int phase);
	void phaseRoll(int phase);
	void phaseCurl(int phase);
	void phaseHold(int phase);
	void phaseUncurl(int phase);
	void phaseHurt(int phase);
	void phaseDie(int phase);

	EnemySight mSight;   // _10
	EnemyMemory mMemory; // _40
	s16 mCharging;       // _4C 1 while mCharge counts down
	int mCharge;         // _50 frames from a hit to the curl from the enemy parameters
	int mTimer;          // _54
	int mResting;        // _58 1 while standing still
};

/**
 * The armadillo, class 0x010A. It walks, and when it sees a player it curls into a ball, spins up and rolls at them.
 *
 * @size 0x300
 */
class TEneArmadillo : public TObjEnemy {
public:
	enum Action {
		ACT_APPEAR = 0,
		ACT_IDLE   = 1,
		ACT_WALK   = 2,
		ACT_ROLL   = 3,
		ACT_CURL   = 4,
		ACT_HOLD   = 5,
		ACT_UNCURL = 6,
		ACT_KNOCK  = 7, // hit from behind
		ACT_FLIP   = 8, // hit from the front
		ACT_DIE    = 9,
	};

	enum {
		// mFlags2
		ARM_ARRIVED   = 0x1,
		ARM_BALL      = 0x2,  // drawn rolled up turned by mBody.mRot.x
		ARM_ARMED     = 0x4,  // the shape is live and onHit hurts
		ARM_GUARDED   = 0x8,  // only kind-1 hits get through
		ARM_RECOVERED = 0x10, // the knock motion has ended
		ARM_20        = 0x20,
		ARM_STOP      = 0x40, // the brain asks the roll to end
	};

	TEneArmadillo(TObject* parent, SetDesc* desc);
	virtual ~TEneArmadillo();

	inline void* operator new(u32 size);
	inline void operator delete(void* mem);

	virtual void vfn_10();                                              // _10 hides it off screen
	virtual void render();                                              // _1C
	virtual void readEnemyDesc(SetDesc* desc);                          // _38
	virtual void vfn_3C();                                              // _3C prints debug text
	virtual f32 vfn_40();                                               // _40
	virtual int onHit(const TObjAction* other, int shape, int* result); // _58
	virtual void vfn_6C();                                              // _6C
	virtual void updateBody();                                          // _70
	virtual void setPos(const Vec3f* pos);                              // _74
	virtual void syncCollider();                                        // _78
	virtual void onActionPhase(int action, int phase);                  // _7C
	virtual int getDamage(EnemyHit* hit);                               // _A4
	virtual int getHitKind(const void* touch, HitInfo* info);           // _A8

	void init(SetDesc* desc);
	void setTarget(const Vec3f* target);
	void setAim(const Vec3f* pos, int angle);
	void noteHit();
	void noteDeath();
	int isAppearDone();
	void stopRoll();
	int hasArrived();
	int isRollDone();
	int isCurlDone();
	int isUncurlDone();
	int hurtAction();
	int isRecovered();
	void noteStun();
	void noteDie();
	int deathAction();
	void stopEffect();
	void createBrain();
	void beginBones();
	void endBones();
	void updatePose();
	void placeParts();
	void applyTransform();
	void stepFoot(f32 dist, int part, Vec3f* out);
	int move(Vec3f* vel);
	void walk(f32 dist, void* arg);
	void drag(f32 rate);
	int turnToAim(int rate);
	void arm();
	void disarm();
	void guard();
	void unguard();
	void kickDust();
	void stopDust();
	void puffAt(f32 frame, int id);
	int startKnock();
	void tumble(int angle);
	void endKnock();

	void actAppear(int phase);
	void actIdle(int phase);
	void actWalk(int phase);
	void actRoll(int phase);
	void actCurl(int phase);
	void actHold(int phase);
	void actUncurl(int phase);
	void actKnock(int phase);
	void actFlip(int phase);
	void actDie(int phase);

	ArmadilloHuntBrain* mBrain; // _158
	void* mPose;                // _15C
	DrawTask* mModel;           // _160 the task
	u32 mFlags2;                // _164
	Vec3f mKnock;               // _168 which way a hit pushes it
	HitVec mParts[3];           // _174 posed from the bones
	Vec3f mTarget;              // _198 where to walk
	Vec3f mAim;                 // _1A4 the player to roll at
	int mAimAngle;              // _1B0
	TEnemyLifeBar* mBar;        // _1B4
	ObjShadow mShadow;          // _1B8
	EnemyLight mLight;          // _1F0
	EnemyTint mTint;            // _230
	EnemyEffect mEffect;        // _238
	TObject* mDust[2];          // _288 the two dust trails of the roll
	FxHolder mFx;               // _290 the roll's trail
	SoundHandle mSound;         // _2C4 the spin-up sound
	int mStep;                  // _2F0
	int mTimer;                 // _2F4
	int mSpin;                  // _2F8 how fast the ball turns or the knock's angle
	f32 _2FC;                   // _2FC
};

extern "C" {
void initTEneArmadillo();
void shutdownTEneArmadillo();
}

extern "C" {
TObject* createTEneArmadillo(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern ResBank gArmadilloBank;
extern NJS_OBJECT** sArmadilloModels;
extern NJS_TEXLIST** sArmadilloTextures;
extern NJS_MOTION** sArmadilloMotions;
extern int* sArmadilloParams;
extern void* sArmadilloClassName;
extern const int sArmadilloTrackNodes[];
extern const EnemyFrameKey* sArmadilloFrameKeys[];
}

#endif
