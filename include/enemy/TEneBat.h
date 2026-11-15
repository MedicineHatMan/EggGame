#ifndef _TENEBAT_H
#define _TENEBAT_H
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
#include "enemy/BatList.h"

class TEneBat;

/** The state machine The three words come first so the vtable lands at _0C */
struct BatBrain {
	enum State {
		STATE_APPEAR = 0,
		STATE_WANDER = 1,
		STATE_RISE   = 2,
		STATE_DIVE   = 3,
		STATE_STUN   = 4,
		STATE_FLIP   = 5,
		STATE_DIE    = 6,
	};

	int mState;      // _00
	int mNext;       // _04 an action to ask the owner for
	TEneBat* mOwner; // _08

	inline BatBrain()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt()               = 0; // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~BatBrain() { }                  // _10 0x800D9AD8
	virtual void update(TEneBat* owner);            // _14 0x800D8EA4

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** No fields of its own */
struct BatBrainBase : public BatBrain {
	BatBrainBase();
	virtual ~BatBrainBase(); // 0x800D8F70
};

/**
 * The bat's brain. Flies about its home, and when it sees a player close enough, rises over them and dives.
 *
 * @size 0x54
 */
struct BatHuntBrain : public BatBrainBase {
	BatHuntBrain(TEneBat* owner);
	virtual ~BatHuntBrain();

	static BatHuntBrain* create(TEneBat* owner); // 0x800D8FCC

	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void pickTarget();
	int look();
	void aim();

	void phaseAppear(int phase);
	void phaseWander(int phase);
	void phaseRise(int phase);
	void phaseDive(int phase);
	void phaseStun(int phase);
	void phaseFlip(int phase);
	void phaseDie(int phase);

	EnemySight mSight;   // _10
	EnemyMemory mMemory; // _40
	int mTimer;          // _4C frames until a new target or the step of the rise
	int mResting;        // _50 1 while hovering in place
};

/**
 * The bat, class 0x0108. It flies, rises over a player and dives at them. When it dies it lets go of four TEneAm01,
 * and goes once they have all gone too.
 *
 * @size 0x2FC
 */
class TEneBat : public TObjEnemy {
public:
	enum Action {
		ACT_APPEAR = 0,
		ACT_HOVER  = 1,
		ACT_FLY    = 2,
		ACT_RISE   = 3,
		ACT_AIM    = 4,
		ACT_DIVE   = 5,
		ACT_STUN   = 6,
		ACT_FLIP   = 7,
		ACT_DIE    = 8,
	};

	enum {
		// mFlags2
		BAT_ARRIVED = 0x1,
		BAT_ARMED   = 0x2,  // the dive's shapes are live and onHit hurts
		BAT_HIT     = 0x4,  // ends a dive
		BAT_LOW     = 0x8,  // near the ground from followGround
		BAT_SOUND   = 0x10, // the flapping sound is playing
		BAT_FLAP    = 0x20, // the flapping sound should play
	};

	TEneBat(TObject* parent, SetDesc* desc);
	virtual ~TEneBat();

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
	void noteDeath();
	int isAppearDone();
	int hasArrived();
	int isRiseDone();
	int isDiveDone();
	int canFlip();
	int isStunDone();
	int isFlipDone();
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
	int clampStep(f32 from, f32 to, f32 limit, f32* out);
	int move(Vec3f* vel);
	int drag(f32 rate);
	void hover(f32 rate);
	u8 followGround(f32 up, f32 down, Vec3f* pos, f32* vy);
	void arm();
	void disarm();
	void updateSound();
	void startFlap();
	void stopFlap();

	void actAppear(int phase);
	void actHover(int phase);
	void actFly(int phase);
	void actRise(int phase);
	void actAim(int phase);
	void actDive(int phase);
	void actStun(int phase);
	void actFlip(int phase);
	void actDie(int phase);

	BatHuntBrain* mBrain; // _158
	void* mPose;          // _15C
	DrawTask* mModel;     // _160 the task
	u32 mFlags2;          // _164
	Vec3f mKnock;         // _168 which way a hit pushes it
	EnemyEffect mEffect;  // _174
	HitVec mParts[2];     // _1C4 posed from the bones
	Vec3f mTarget;        // _1DC where to fly
	Vec3f mAim;           // _1E8 the player to dive at
	int mAimAngle;        // _1F4
	TEnemyLifeBar* mBar;  // _1F8
	ObjShadow mShadow;    // _1FC
	EnemyLight mLight;    // _234
	EnemyTint mTint;      // _274
	BatList mList;        // _27C the enemies it let go
	SoundHandle mSound;   // _288
	FxHolder mFx;         // _2B4 the dive's trail
	int mStep;            // _2E8
	int mTimer;           // _2EC
	int mAngle;           // _2F0
	f32 mHeight;          // _2F4
	f32 mRise;            // _2F8
};

extern "C" {
void initTEneBat();
void shutdownTEneBat();
}

extern "C" {
TObject* createTEneBat(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern ResBank sBatBank;
extern NJS_OBJECT** sBatModels;
extern NJS_TEXLIST** sBatTextures;
extern NJS_MOTION** sBatMotion;
extern const EnemyFrameKey* sBatFrameKeys[];
}

#endif
