#ifndef _TENEFROG_H
#define _TENEFROG_H
struct EnemyFrameKey;
struct SetDesc;
struct TObject;
struct GjActor;
struct DrawTask;
struct CharaColli;

#include "enemy/ObjEnemy.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemySight.h"
#include "enemy/EnemyLight.h"
#include "engine/MotionPlayer.h"
#include "enemy/TEneAm01Parts.h"
#include "ui/TEnemyLifeBar.h"

class TEneFrog;

/** The state machine The three words come first so the vtable lands at _0C */
struct FrogBrain {
	enum State {
		STATE_APPEAR = 0,
		STATE_WANDER = 1,
		STATE_ATTACK = 2,
		STATE_STUN   = 3,
		STATE_DIE    = 4,
	};

	int mState;       // _00
	int mNext;        // _04 an action to ask the owner for
	TEneFrog* mOwner; // _08

	inline FrogBrain()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt()               = 0; // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~FrogBrain() { }                 // _10 0x800F1854
	virtual void update(TEneFrog* owner);           // _14 0x800F0E64

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** No fields of its own */
struct FrogBrainBase : public FrogBrain {
	FrogBrainBase();
	virtual ~FrogBrainBase(); // 0x800F0F30
};

/**
 * The frog's brain. Hops between random points about its home, and licks at a player it sees close enough.
 *
 * @size 0x58
 */
struct FrogHuntBrain : public FrogBrainBase {
	FrogHuntBrain(TEneFrog* owner);
	virtual ~FrogHuntBrain();

	static FrogHuntBrain* create(TEneFrog* owner); // 0x800F0F8C

	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void pickTarget();
	int look();
	void aim();

	void phaseAppear(int phase);
	void phaseWander(int phase);
	void phaseAttack(int phase);
	void phaseStun(int phase);
	void phaseDie(int phase);

	EnemySight mSight;   // _10
	EnemyMemory mMemory; // _40
	int mTimer;          // _4C frames until a new target or until the attack ends
	int mResting;        // _50 1 while sitting still
	int mHops;           // _54 hops since the last rest
};

/**
 * The frog, class 0x0109. Hops about, and shoots its tongue at a player in front of it. The tongue is a second
 * model with its own motion, and a second collider, a box from the mouth to the tip.
 *
 * @size 0x38C
 */
class TEneFrog : public TObjEnemy {
public:
	enum Action {
		ACT_APPEAR    = 0,
		ACT_IDLE      = 1,
		ACT_HOP       = 2,
		ACT_LICK      = 3,
		ACT_RETRACT   = 4,
		ACT_STUN      = 5,
		ACT_STUN_LICK = 6, // stunned with the tongue out
		ACT_DIE       = 7,
	};

	enum {
		// mFlags2
		FROG_ARRIVED     = 0x1,
		FROG_OVERSHOT    = 0x2,  // the last hop took it further from the target
		FROG_TONGUE_SHOW = 0x4,  // the tongue is drawn
		FROG_TONGUE_HURT = 0x8,  // the tongue's collider is live and onHit hurts
		FROG_TONGUE_FX   = 0x10, // the tongue trails particles
	};

	TEneFrog(TObject* parent, SetDesc* desc);
	virtual ~TEneFrog();

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

	void init(SetDesc* desc);
	void setTarget(const Vec3f* target);
	void setAim(const Vec3f* pos, int angle);
	void noteHurt();
	int isAppearDone();
	int hasArrived();
	int isGrounded();
	int isRetractDone();
	int stunAction();
	int isStunDone();
	void noteStun();
	void noteDie();
	int deathAction();
	void stopEffect();
	void createBrain();
	void beginBones();
	void endBones();
	void updatePose();
	void placeParts();
	void beginTongueBones();
	void endTongueBones();
	void updateTonguePose();
	void applyTransform();
	void applyTongueTransform();
	int moveFiltered(Vec3f* vel);
	int move(Vec3f* vel);
	void drag(f32 rate);
	void keepTongueFx();
	void stopTongueFx();

	void actAppear(int phase);
	void actIdle(int phase);
	void actHop(int phase);
	void actLick(int phase);
	void actRetract(int phase);
	void actStun(int phase);
	void actStunLick(int phase);
	void actDie(int phase);

	FrogHuntBrain* mBrain;       // _158
	GjActor* mPose;              // _15C
	DrawTask* mModel;            // _160 the task
	u32 mFlags2;                 // _164
	Vec3f mKnock;                // _168 which way a hit pushes it
	HitVec mParts[5];            // _174 posed from the bones
	Vec3f mTarget;               // _1B0 where to hop
	Vec3f mAim;                  // _1BC the player to lick
	int mAimAngle;               // _1C8
	TEnemyLifeBar* mBar;         // _1CC
	ObjShadow mShadow;           // _1D0
	EnemyLight mLight;           // _208
	EnemyTint mTint;             // _248
	EnemyEffect mEffect;         // _250
	CharaColli* mTongueCollider; // _2A0
	MotionPlayer mTongueMotion;  // _2A4
	EnemyAnim mTongueAnim;       // _2E0
	GjActor* mTonguePose;        // _2EC
	int mTonguePitch;            // _2F0
	int mTongueYaw;              // _2F4
	int mTongueYawPrev;          // _2F8
	HitVec mTongue[8];           // _2FC posed from the tongue's bones root to tip
	TObject* mTongueFx[8];       // _35C
	int mStep;                   // _37C
	int mSubStep;                // _380 the tongue's step in a lick the fall's in a death
	int mHopAngle;               // _384
	f32 mBestDist;               // _388 the closest a hop has come to mTarget
};

extern "C" {
void initTEneFrog();
void shutdownTEneFrog();
}

extern "C" {
TObject* createTEneFrog(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern ResBank sFrogBank;
extern NJS_OBJECT** sFrogModels;
extern NJS_TEXLIST** sFrogTextures;
extern NJS_MOTION** sFrogMotion;
extern NJS_MOTION** sFrogTongueMotion;
extern int* gFrogParams;
extern const char* gTEneFrogName;
extern const EnemyFrameKey* sFrogFrameKeys[];
}

#endif
