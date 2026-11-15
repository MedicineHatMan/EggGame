#ifndef _TENEAM07_H
#define _TENEAM07_H
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
#include "enemy/EnemyLight.h"
#include "enemy/TEneAm01Parts.h"
#include "enemy/TEneAm02.h"
#include "ui/TEnemyLifeBar.h"
#include "enemy/EnemySight.h"
#include "sound/SeHolder.h"

class TEneAm07;

/** The bee's state machine The three words come first so the vtable lands at _0C */
struct Am07Brain {
	enum State {
		STATE_APPEAR = 0,
		STATE_WANDER = 1, // drifts around a spot near home
		STATE_HOVER  = 2, // hovers over the player it saw
		STATE_DIVE   = 3, // the charge from far off
		STATE_STING  = 4, // jumps and stabs the ground from close in
		STATE_STUCK  = 5, // the stinger is in the ground
		STATE_FLIP   = 6,
		STATE_DIE    = 7,
	};

	int mState;       // _00
	int mNext;        // _04 an action to ask the owner for
	TEneAm07* mOwner; // _08

	inline Am07Brain()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt()               = 0; // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	virtual ~Am07Brain();                           // _10 0x800706E8 inline in TEneAm07Brain.cpp
	virtual void update(TEneAm07* owner);           // _14 0x8006F93C

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** No fields of its own */
struct Am07BrainBase : public Am07Brain {
	Am07BrainBase();          // 0x8006F9D4
	virtual ~Am07BrainBase(); // 0x8006FA08
};

/**
 * The bee's brain, vtable 0x80257120. It wanders until it sees a player within 270, then hovers over them, and
 * either stings (within 80) or dives (within 150, or while it holds a grudge).
 *
 * @size 0x54
 */
struct Am07BeeBrain : public Am07BrainBase {
	Am07BeeBrain(TEneAm07* owner); // 0x8006FAAC
	virtual ~Am07BeeBrain();       // 0x8006FB6C

	static Am07BeeBrain* create(TEneAm07* owner); // 0x8006FA64

	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void pickWanderSpot();
	int wanderTime();
	void aim();
	int look();
	void attack();
	int pickAttack();

	void phaseAppear(int phase);
	void phaseWander(int phase);
	void phaseHover(int phase);
	void phaseDive(int phase);
	void phaseSting(int phase);
	void phaseStuck(int phase);
	void phaseFlip(int phase);
	void phaseDie(int phase);

	EnemySight mSight;   // _10
	EnemyMemory mMemory; // _40
	int mTimer;          // _4C the wander's next pick or the stuck state's step
	int mWait;           // _50 frames stuck before pulling out
};

/**
 * The bee, class 0x0104. It flies, dives at a player, or jumps and drives its stinger into the ground, where it
 * sticks for a while. Ten points follow the pose. Its brain is Am07BeeBrain, built at the end of init.
 *
 * @size 0x360
 */
class TEneAm07 : public TObjEnemy {
public:
	enum Action {
		ACT_APPEAR = 0,
		ACT_WANDER = 1,
		ACT_HOVER  = 2,
		ACT_DIVE   = 3,
		ACT_STING  = 4,
		ACT_STUCK  = 5,
		ACT_PULL   = 6,
		ACT_FLIP   = 7,
		ACT_SINK   = 8, // dies with the stinger in the ground
		ACT_FALL   = 9, // dies in the air
	};

	enum {
		// mFlags2
		AM_STALLED  = 0x1,  // the wander has reached its spot
		AM_PITCH    = 0x2,  // the body pitches with mBody.mRot.x
		AM_GUARDED  = 0x4,  // shape 3 does no damage
		AM_STINGER  = 0x8,  // shapes 2 and 3 both hurt
		AM_CYLINDER = 0x10, // shape 2 is a cylinder down to the stinger not a sphere
		AM_STINGING = 0x20, // a flip hit is shrugged off
		AM_STUCK    = 0x40, // the stinger is in the ground
	};

	enum {
		SOUND_NONE = 0,
		SOUND_BUZZ = 1,
		SOUND_DIVE = 2,
	};

	TEneAm07(TObject* parent, SetDesc* desc);
	virtual ~TEneAm07();

	inline void* operator new(u32 size);
	inline void operator delete(void* mem);

	virtual void vfn_10();                                              // _10
	virtual void render();                                              // _1C
	virtual void readEnemyDesc(SetDesc* desc);                          // _38
	virtual void vfn_3C();                                              // _3C
	virtual f32 vfn_40();                                               // _40
	virtual int onHit(const TObjAction* other, int shape, int* result); // _58
	virtual void vfn_6C();                                              // _6C
	virtual void updateBody();                                          // _70
	virtual void setPos(const Vec3f* pos);                              // _74
	virtual void syncCollider();                                        // _78
	virtual void onActionPhase(int action, int phase);                  // _7C
	virtual int getDamage(EnemyHit* hit);                               // _A4

	void init(SetDesc* desc);

	int stingDamage(int shape);
	void getHome(Vec3f* out) const;
	void setAnchor(const Vec3f* pos);
	void setGoal(const Vec3f* pos);
	void noteDeath();
	int isAppearDone();
	int isDiveDone();
	int isStingDone();
	int isPullDone();
	int canFlip();
	void noteFlip();
	int isFlipDone();
	void noteDie();
	int deathAction();
	void stopEffect();

	void createBrain();
	void beginPoints();
	void endPoints();
	void pose();
	void placePoints();
	void animate();
	void bodyMatrix();
	void hoverMatrix();
	int moveFlat(f32 bounce, Vec3f* vel);
	void getCenter(Vec3f* out);
	void getHead(Vec3f* out);
	void getTail(Vec3f* out);
	int updateGround();
	int moveVertical(f32 damp, const Vec3f* head, const Vec3f* tail, f32* dy);
	int moveVertical(f32 damp, f32* dy);
	void move();
	int turnToward(int angle, f32 a, f32 b, f32 c, f32 d, f32 e);
	int turnSlow(int angle);
	int goalAngle();
	int turnToGoal();
	int turnToGoalFast();
	void bank();
	void levelOut();
	int strafe(int* side, f32* dist, Vec3f* out);
	void wander(int* angle, int* rising);
	void hover(u32* phase);
	int dive(int* timer);
	void chase();
	void beginDeath();

	void actAppear(int phase);
	void actWander(int phase);
	void actHover(int phase);
	void actDive(int phase);
	void actSting(int phase);
	void actStuck(int phase);
	void actPull(int phase);
	void actFlip(int phase);
	void actSink(int phase);
	void actFall(int phase);

	Am07Brain* mBrain;   // _158
	void* mPose;         // _15C
	DrawTask* mDrawTask; // _160
	u32 mFlags2;         // _164
	Vec3f mPush;         // _168 added to the velocity when flipped
	int mHealthStep;     // _174
	f32 mHover;          // _178 how far above mBody.mPos the model sits
	f32 mTurnVel;        // _17C
	f32 mRadius;         // _180
	f32 mCylRadius;      // _184
	HitVec mPoints[10];  // _188 2 the head 4 and 6 the body 5 a scale axis 7 the stinger 8 and 9 the tail
	Vec3f mCenter;       // _200
	Vec3f mAnchor;       // _20C where it wanders or hovers
	Vec3f mGoal;         // _218 the player it is after
	Vec3f mGroundPos;    // _224
	Vec3f mGroundNormal; // _230
	TEnemyLifeBar* mBar; // _23C
	ObjShadow mShadow;   // _240
	EnemyFx* mFx;        // _278 the dive's trail
	EnemyLight mLight;   // _27C
	EnemyTint mTint;     // _2BC
	EnemyEffect mEffect; // _2C4
	s16 mSound;          // _314 SOUND_
	s16 mPrevSound;      // _316
	SeHolder mSe;        // _318
	int mStep;           // _34C
	int mTimer;          // _350
	int mPhase;          // _354 the wander's heading or the hover's sway
	int _358;            // _358
	f32 mDist;           // _35C
};

extern "C" {
void initTEneAm07();
void shutdownTEneAm07();
}

extern "C" {
TObject* createTEneAm07(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern ResBank sAm07Bank;
extern NJS_OBJECT** sAm07Models;
extern NJS_TEXLIST** sAm07Textures;
extern NJS_MOTION** sAm07Motion;
extern int* sAm07Param;
extern const int sAm07Nodes[];
extern const EnemyFrameKey* sAm07FrameKeys[];
}

#endif
