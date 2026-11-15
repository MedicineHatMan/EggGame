#ifndef _TENEAM08_H
#define _TENEAM08_H
struct EnemyFrameKey;
struct SetDesc;
struct TObject;
struct DrawTask;
struct CharaColli;

#include "enemy/ObjEnemy.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemyLight.h"
#include "enemy/TEneAm01Parts.h"
#include "ui/TEnemyLifeBar.h"
#include "enemy/EnemySight.h"
#include "engine/FxHolder.h"

class TEneAm08;

/** The lizard's state machine laid out like Am07Brain three words then the vtable at _0C */
struct Am08Brain {
	enum State {
		STATE_APPEAR  = 0,
		STATE_WANDER  = 1, // walks between spots near home
		STATE_STALK   = 2, // threatens a player it saw then closes in
		STATE_ATTACK  = 3, // the lunge and the slide after it
		STATE_RECOVER = 4,
		STATE_FLIP    = 5,
		STATE_DIE     = 6,
	};

	int mState;       // _00
	int mNext;        // _04 an action to ask the owner for
	TEneAm08* mOwner; // _08

	inline Am08Brain()
	{
		mOwner = 0;
		mState = 0;
		mNext = -1;
	}

	virtual int checkInterrupt() = 0;               // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	virtual ~Am08Brain() { }                           // _10 0x80076018
	virtual void update(TEneAm08* owner);           // _14 0x80075330

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** No fields of its own */
struct Am08BrainBase : public Am08Brain {
	Am08BrainBase();          // 0x800753C8
	virtual ~Am08BrainBase(); // 0x800753FC
};

/**
 * The lizard's brain, vtable 0x802574B4. It wanders until it sees a player, threatens them, walks up, and lunges
 * once its bite collider touches something.
 *
 * @size 0x54
 */
struct Am08LizardBrain : public Am08BrainBase {
	Am08LizardBrain(TEneAm08* owner); // 0x800754A0
	virtual ~Am08LizardBrain();       // 0x80075558

	static Am08LizardBrain* create(TEneAm08* owner); // 0x80075458

	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void pickWanderSpot();
	int look(f32 farSq, f32 nearSq, int farAngle, int nearAngle);
	int lookNear();
	int lookFar();
	void aim();

	void phaseAppear(int phase);
	void phaseWander(int phase);
	void phaseStalk(int phase);
	void phaseAttack(int phase);
	void phaseRecover(int phase);
	void phaseFlip(int phase);
	void phaseDie(int phase);

	EnemySight mSight;   // _10
	EnemyMemory mMemory; // _40
	int mTimer;          // _4C the wander's next pick or a state's step
	int mWait;           // _50
};

/**
 * The lizard, class 0x0105. It walks about, rears up at a player it sees, then lunges and slides. Eight points
 * follow the pose. A second collider (mBite) sweeps out in front while it closes in.
 *
 * @size 0x318
 */
class TEneAm08 : public TObjEnemy {
public:
	enum Action {
		ACT_APPEAR   = 0,
		ACT_IDLE     = 1,
		ACT_WALK     = 2,
		ACT_THREAT   = 3,
		ACT_APPROACH = 4,
		ACT_LUNGE    = 5,
		ACT_SLIDE    = 6,
		ACT_RECOVER  = 7,
		ACT_FLIP_LUNGING = 8, // flipped mid-lunge
		ACT_FLIP     = 9,
		ACT_DIE      = 10,
		ACT_DIE_LUNGING = 11,
	};

	enum {
		// mFlags2
		AM_ARRIVED  = 0x1,  // the walk has reached its spot
		AM_BITE     = 0x4,  // mBite is live
		AM_BITING   = 0x10, // shapes 4 and 5 hurt
		AM_LUNGING  = 0x20,
		AM_GUARDING = 0x40, // a flip hit is shrugged off
		AM_TRAIL    = 0x80, // the dust trail runs
	};

	TEneAm08(TObject* parent, SetDesc* desc);
	virtual ~TEneAm08();

	inline void* operator new(u32 size);
	inline void operator delete(void* mem);

	virtual void vfn_10();                     // _10
	virtual void render();                     // _1C
	virtual void readEnemyDesc(SetDesc* desc); // _38
	virtual void vfn_3C();                     // _3C
	virtual f32 vfn_40();                      // _40
	virtual int onHit(const TObjAction* other, int shape, int* result); // _58
	virtual void vfn_6C();                     // _6C
	virtual void updateBody();                 // _70
	virtual void setPos(const Vec3f* pos);           // _74
	virtual void syncCollider();               // _78
	virtual void onActionPhase(int action, int phase); // _7C
	virtual int getDamage(EnemyHit* hit);      // _A4

	void init(SetDesc* desc);

	void lerpPoints(int a, int b, Vec3f* out);
	int isBiting(int shape);
	void setAnchor(const Vec3f* pos);
	void setGoal(const Vec3f* pos, int angle);
	int isLunging();
	void noteDeath();
	void stopEffect();
	int isAppearDone();
	int hasArrived();
	int isThreatDone();
	int canBite();
	int isLungeDone();
	int isSlideDone();
	int isRecoverDone();
	int canFlip();
	void noteFlip();
	int isFlipLungingDone();
	int isFlipDone();
	void noteDie();
	int deathAction();

	void createBrain();
	void pose();
	void beginPoints();
	void endPoints();
	void placePoints();
	void bodyMatrix();
	void stepAlong(int point, Vec3f* vel);
	void drag(f32 rate, Vec3f* vel);
	void fall(Vec3f* vel);
	int move(Vec3f* vel);
	void setForward(f32 speed);
	f32 flatSpeed();

	void beginFlip();
	void flipped();
	void endFlip();
	void beginDeath(int* timer);
	void dying(int* timer);
	void endDeath();

	void actAppear(int phase);
	void actIdle(int phase);
	void actWalk(int phase);
	void actThreat(int phase);
	void actApproach(int phase);
	void actLunge(int phase);
	void actSlide(int phase);
	void actRecover(int phase);
	void actFlipLunging(int phase);
	void actFlip(int phase);
	void actDie(int phase);
	void actDieLunging(int phase);

	Am08LizardBrain* mBrain; // _158
	void* mPose;          // _15C
	DrawTask* mDrawTask;      // _160
	CharaColli* mBite;          // _164 a one-shape collider in front of the mouth
	u32 mFlags2;          // _168
	Vec3f mPush;          // _16C the velocity a flip starts with
	int mHealthStep;      // _178
	HitVec mPoints[8];    // _17C 0 the head 5 the body 6 and 7 the tail
	f32 mRadius;          // _1DC flat distance to point 7 for the ground collision
	Vec3f mAnchor;        // _1E0 where it walks to
	Vec3f mGoal;          // _1EC the player it is after
	int mGoalAngle;       // _1F8 toward mGoal
	f32 mBiteZ;           // _1FC how far in front mBite sits
	f32 _200;             // _200
	TEnemyLifeBar* mBar;  // _204
	ObjShadow mShadow;    // _208
	EnemyLight mLight;    // _240
	EnemyTint mTint;      // _280
	EnemyEffect mEffect;  // _288
	FxHolder mFx;         // _2D8 the dust trail
	int mStep;            // _30C
	int _310;             // _310
	f32 _314;             // _314
};

extern "C" {
void initTEneAm08();
void shutdownTEneAm08();
}

extern "C" {
TObject* createTEneAm08(TObject*, SetDesc*);
}

#include "engine/CharaColli.h"
#include "io/ResBank.h"
extern "C" {
extern NJS_OBJECT** sAm08Models;
extern NJS_TEXLIST** sAm08Textures;
extern NJS_MOTION** sAm08Motion;
extern const ColliShape sAm08BiteShape;
extern const int sAm08Nodes[];
extern const EnemyFrameKey* sAm08FrameKeys[];
}

#endif
