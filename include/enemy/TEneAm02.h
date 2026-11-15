#ifndef _TENEAM02_H
#define _TENEAM02_H
struct EnemyFrameKey;
struct SetDesc;
struct TObject;
struct DrawTask;
struct CharaColli;

#include "enemy/ObjEnemy.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "enemy/EnemySight.h"
#include "engine/RockFlight.h"
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "enemy/TEneAm01Parts.h"
#include "enemy/TEneAm02_Rock.h"
#include "ui/TEnemyLifeBar.h"
#include "enemy/EnemyLight.h"

/** What this class needs of the objects it keeps its effects its rock and its health bar */
struct EnemyFx {
	u8 _00[4];
	u16 mFlags; // _04 1 = free me
	u8 _06[0x1E];
	Vec3f mPos; // _24 effects only
};

struct Am02SeekBrain;

/**
 * The big enemy that swings both arms, or throws rocks.
 *
 * @size 0x2C0
 */
class TEneAm02 : public TObjEnemy {
public:
	enum Action {
		ACT_APPEAR     = 0,
		ACT_WALK       = 1,
		ACT_CHASE      = 2,
		ACT_ATTACK     = 3,
		ACT_THROW      = 4,
		ACT_STUN       = 5,
		ACT_FLIP       = 6,
		ACT_DIE        = 7,
		ACT_DIE_THROWN = 8,
	};

	enum {
		// mFlags2
		AM_ARRIVED     = 0x1,
		AM_FLIP_DONE   = 0x2,
		AM_DYING       = 0x4,
		AM_HIDDEN      = 0x8, // tested by slot _10 never set here
		AM_ATTACK_DONE = 0x10,
		AM_WEAK        = 0x20, // from the set data Only the debug text reads it here
		AM_STUN_DONE   = 0x40,
		AM_THROWN      = 0x100,
		AM_BRACED      = 0x200, // during a swing when a hit won't flip it
	};

	TEneAm02(TObject* parent, SetDesc* desc);
	virtual ~TEneAm02();

	inline void* operator new(u32 size);
	inline void operator delete(void* mem);

	virtual void vfn_10();                                              // _10 hides it off screen
	virtual void render();                                              // _1C draws the model
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
	virtual int getHitKind(const void* other, HitInfo* info);           // _A8

	void init(SetDesc* desc);
	int isAppearDone();
	void setTarget(const Vec3f* target);
	int hasArrived();
	int isAttackDone();
	int isStunDone();
	void hitEffects();
	int isFlipDone();
	void hitSound();
	int canFlip();
	void deathSound();
	int pickDeath();
	void startEffect();
	int pickVariant(SetDesc* desc);
	void createBrain();
	void partMove(f32 dist, int part, Vec3f* out);
	int move(Vec3f* vel);
	int turnToTarget(int rate);
	void stepFoot(f32 dist, int foot);
	void checkArrived(int rate);
	void coast(f32 drag);
	int walkFoot();
	int chaseFoot();
	void beginBones();
	void endBones();
	void updatePose();
	void updateParts();
	void applyTransform();
	void followPart(EnemyFx** fx, int part);
	void show();
	void hide();

	void actAppear(int phase);
	void actWalk(int phase);
	void actChase(int phase);
	void actAttack(int phase);
	void actThrow(int phase);
	void actStun(int phase);
	void actFlip(int phase);
	void actDie(int phase);
	void actDieThrown(int phase);

	int mVariant;          // _158 0 or 1 Picks the brain
	Am02SeekBrain* mBrain; // _15C
	void* mPose;           // _160
	CharaColli* mSwing;    // _164 the collider for its swing
	DrawTask* mModelTask;  // _168
	u32 mFlags2;           // _16C
	Vec3f mTarget;         // _170
	Vec3f mHitPos;         // _17C
	ObjShadow mShadow;     // _188
	TEneAm02_Rock* mRock;  // _1C0 created while the throw is held
	EnemyFx* mTrailA;      // _1C4
	EnemyFx* mTrailB;      // _1C8
	int mHealthStep;       // _1CC a fifth of the health
	int mLastDamage;       // _1D0 the damage of a throw
	RockFlight* mThrow;    // _1D4 moves it through the air when thrown
	TEnemyLifeBar* mBar;   // _1D8
	EnemyLight mLight;     // _1DC
	EnemyTint mTint;       // _21C
	HitVec mParts[5];      // _224 0 and 1 are the feet 2 centres the collider 3 and 4 trail in a swing
	EnemyEffect mEffect;   // _260
	int mStep;             // _2B0
	int mTimer;            // _2B4
	int mAngle;            // _2B8
	f32 mSpeed;            // _2BC
};

/** The same state machine as EnemyBrain in TEneAm01.h built separately */
struct Am02Brain {
	enum State {
		STATE_APPEAR = 0,
		STATE_STUN   = 1,
		STATE_FLIP   = 2,
		STATE_DIE    = 3,
		STATE_WANDER = 4,
		STATE_CHASE  = 5,
		STATE_ATTACK = 6,
	};

	int mState;       // _00
	int mNext;        // _04 an action to ask the owner for
	TEneAm02* mOwner; // _08

	inline Am02Brain()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt()               = 0; // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~Am02Brain() { }                 // _10 0x80021E34
	inline virtual void update(TEneAm02* owner);    // _14 0x80021D9C

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}

	// Enters the first state Both brains do this from their constructors
	inline void start(TEneAm02* owner)
	{
		mOwner = owner;
		mNext  = -1;
		mState = STATE_APPEAR;
		onPhase(mState, 0);
		if (mNext != -1) {
			mOwner->setAction(mNext);
		}
	}
};

/** The states both of TEneAm02's brains share and what's needed to find a player */
struct Am02SeekBrain : public Am02Brain {
	Am02SeekBrain();
	inline virtual ~Am02SeekBrain(); // 0x80021D2C defined in the .cpp files

	virtual int checkInterrupt();    // _08
	virtual void onAppearDone() = 0; // _18
	virtual void onStunDone()   = 0; // _1C
	virtual void onFlipDone()   = 0; // _20

	void pickTarget(f32 radius);
	void phaseAppear(int phase);
	void phaseStun(int phase);
	void phaseFlip(int phase);
	void phaseDie(int phase);

	EnemySight mSight;   // _10
	EnemyMemory mMemory; // _40
	int mStep;           // _4C
	int mTimer;          // _50
};

/** Any other variant Walks around its home chases a player it sees and attacks once close and facing it */
struct Am02AttackBrain : public Am02SeekBrain { // size 0x54
	Am02AttackBrain(TEneAm02* owner);
	virtual ~Am02AttackBrain();

	virtual void onPhase(int state, int phase);
	virtual void onAppearDone();
	virtual void onStunDone();
	virtual void onFlipDone();

	void pickHomeTarget();
	int look();
	void phaseWander(int phase);
	void phaseChase(int phase);
	void phaseAttack(int phase);
};

/** Variant 1 Walks around its home and throws rocks at a player it sees */
struct Am02ThrowBrain : public Am02SeekBrain { // size 0x5C
	Am02ThrowBrain(TEneAm02* owner);
	virtual ~Am02ThrowBrain();

	virtual void onPhase(int state, int phase);
	virtual void onAppearDone();
	virtual void onStunDone();
	virtual void onFlipDone();

	void pickHomeTarget();
	int look();
	void phaseWander(int phase);
	void phaseThrow(int phase);

	// mStep and mTimer are still set up by the base but this brain uses its own
	int mWanderStep;  // _54
	int mWanderTimer; // _58
};

extern "C" {
void initTEneAm02();
void* modelTEneAm02();
void* shapeTEneAm02();
void shutdownTEneAm02();
}

extern "C" {
TObject* createTEneAm02(TObject*, SetDesc*);
}

#include "engine/CharaColli.h"
extern "C" {
extern const ColliShape sAm02SwingShapes[];
extern const EnemyFrameKey* sAm02FrameKeys[];
extern NJS_OBJECT** sAm02Models;
extern NJS_TEXLIST** sAm02Textures;
extern NJS_MOTION** sAm02Motion;
}

#endif
