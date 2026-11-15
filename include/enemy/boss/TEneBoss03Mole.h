#ifndef _TENEBOSS03MOLE_H
#define _TENEBOSS03MOLE_H
struct EnemyFrameKey;
struct SetDesc;
struct TObject;
struct Vec3f;
struct GjActor;
struct DrawTask;
struct CharaColli;

#include "enemy/ObjEnemy.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "egg/ObjEgg.h"
#include "object/ObjCommon.h"
#include "enemy/EnemyLight.h"
#include "enemy/EnemySight.h"
#include "enemy/TEneAm01Parts.h"
#include "enemy/boss/BossParts.h"
#include "sound/SeHolder.h"

class TCamPath;

class TEneBoss03Dino;
class TEneBoss03_EffBlur;
class TCamConversation;
struct MoleBrain;

/**
 * The mole's tuning, from enemyParamGet(3). Most of it comes in threes, picked by how hurt it is.
 * @size 0x4C
 */
struct MoleParams {
	int mHealthSteps[3]; // _00 the health thresholds MoleBrain::getStage counts past
	int mAppear;         // _0C handed to the body when it appears
	int mDigCount[3];    // _10 how many times it digs in one go
	int mAttack[3];      // _1C handed to the body's attack
	f32 mSpeed[3];       // _28
	f32 mCircleSpeed[3]; // _34
	int mHurtDrop[3];    // _40 how much health lost since MoleBrain::saveHealth makes it dive by stage
};

/**
 * The red boss's mole. It digs, circles the player underground and pops up to attack, while TEneBoss03Dino walks
 * the volcano. MoleBrain drives it. Most of its actions ride the dinosaur: the action picks the dinosaur's action and
 * the mole follows its aim each frame. See TEneBoss03Mole.cpp.
 * @size 0x300
 */
class TEneBoss03Mole : public TObjEnemy {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	enum {
		ACT_APPEAR     = 0,
		ACT_WALK       = 1, // 1 to 3 walk the dinosaur round its ring 200 out
		ACT_WALK_IN    = 2, // on the ring
		ACT_WALK_OUT   = 3, // 300 out
		ACT_FACE_IN    = 4,
		ACT_FACE_SIDE  = 5,
		ACT_ROAR       = 6,
		ACT_BREATH     = 7,
		ACT_BREATH_END = 8,
		ACT_SWIPE      = 9,
		ACT_BITE       = 10,
		ACT_IDLE       = 11,
		ACT_LEAP       = 12, // jumps off the dinosaur into the ground
		ACT_DIVE       = 13,
		ACT_DIG        = 14, // underground chasing mTarget
		ACT_POUNCE     = 15, // jumps back onto the dinosaur's head
		ACT_SURFACE    = 16,
		ACT_DOWN       = 17,
		ACT_DIE        = 18,
	};

	enum {
		// mMoleFlags
		MOLE_LANDED   = 0x1,  // a leap reached the ground
		MOLE_OFF_DINO = 0x2,  // other enemies can hit it
		MOLE_GUARD    = 0x4,  // getHitKind says 3
		MOLE_AT_ROCK  = 0x8,  // has stayed by a volcano rock long enough
		MOLE_SUIT     = 0x10, // drawn with the death suit
		MOLE_EXPOSED  = 0x20, // a player can hit it
	};

	TEneBoss03Mole(TObject* parent, SetDesc* desc);
	virtual ~TEneBoss03Mole();

	virtual void vfn_10();
	virtual void render();
	virtual void readEnemyDesc(SetDesc* desc);
	virtual void vfn_3C();
	virtual f32 vfn_40();
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void vfn_6C();
	virtual void updateBody();
	virtual void setPos(const Vec3f* pos);
	virtual void syncCollider();
	virtual void onActionPhase(int action, int phase);
	virtual int getDamage(EnemyHit* hit);
	virtual int getHitKind(const void* touch, HitInfo* info);

	void init(SetDesc* desc);
	void setupColliders();
	void rideDino();
	void burstRocks(int count);
	void updateShadow();
	int turnToTarget(int rate, Vec3f* dir);
	void moveToTarget(f32 maxSpeed, f32 accel, int rate);
	void chaseTarget();
	void slowDown(f32 minSpeed);
	void collidersOff();
	void collidersOn();
	void attackOnly();
	int nearRock();
	void biteFx();
	void startFx();
	void endFx();
	void killBlur();
	void updateBlur();
	void endBlur();
	void setTarget(Vec3f* target);
	void getDinoPos(Vec3f* out);
	void setDinoRate(f32 rate);
	void setMaxHealth(int health);
	void flinch();
	void knockOut();
	void setGuard();
	void clearGuard();
	int isAppearDone();
	int dinoAtRing();
	int dinoFaceInDone();
	int dinoFaceSideDone();
	int dinoRoarDone();
	int dinoRoarPeaked();
	void getDinoNode0(Vec3f* out);
	int dinoBreathing();
	int dinoBreathEndDone();
	int dinoSwipeDone();
	int dinoBiteDone();
	int hasLanded();
	int isDiveDone();
	void setRate(f32 rate);
	void getDinoMouth(Vec3f* out);
	int isAtRock();
	int isPounceDone();
	int isSurfaceDone();
	int isDownDone();
	void createBrain();
	void trackBegin();
	void trackEnd();
	void pose();
	void poseFromRoot();
	void setDinoAction(int action);
	void updateDino();
	void setDinoRing(int action);
	void followDino();
	void turnWithDino();
	void startDemo(int model, int motion);
	void actAppear(int phase);
	void actWalk(int phase, int action);
	void actFace(int phase, int action);
	void actRoar(int phase);
	void actBreath(int phase);
	void actBreathEnd(int phase);
	void actSwipe(int phase);
	void actBite(int phase);
	void actIdle(int phase);
	void actLeap(int phase);
	void actDive(int phase);
	void actDig(int phase);
	void actPounce(int phase);
	void actSurface(int phase);
	void actDown(int phase);
	void actDie(int phase);
	void bodyMatrix();
	void modelMatrix();

	u8 _158[0x10];             // _158
	CharaColli* mHurt;         // _168 the collider other things hit
	MoleBrain* mBrain;         // _16C
	TEneBoss03Dino* mDino;     // _170
	GjActor* mPose;            // _174
	DrawTask* mDrawTask;       // _178
	u32 mMoleFlags;            // _17C
	Vec3f mPush;               // _180 how far the last hit pushed it
	HitVec mNodes[5];          // _18C node positions from sMoleTrackNodes
	Vec3f mTarget;             // _1C8
	Vec3f mFrom;               // _1D4 where a leap started
	int mMaxHealth;            // _1E0
	ObjShadow mShadow;         // _1E4
	EnemyLight mLight;         // _21C
	EnemyTint mTint;           // _25C
	f32 mRate;                 // _264 the motion rate
	TEneBoss03_EffBlur* mBlur; // _268
	BossSuit mSuit;            // _26C
	TObject* _2A4;             // _2A4
	TCamPath* mDemo;           // _2A8 the cut-in camera as it appears and dies
	SeCue mCue;                // _2AC
	SeHolder mSe;              // _2B0
	int mStep;                 // _2E4
	int mTimer;                // _2E8
	int mAngle;                // _2EC the yaw or pitch a leap starts at
	f32 _2F0;                  // _2F0 the time of a leap the turn rate of a pounce or time by a rock
	f32 _2F4;                  // _2F4 the turn rate of a leap or the time of a pounce
	f32 mFallAt;               // _2F8 when a pounce starts its falling motion
	f32 mFlight;               // _2FC how long a pounce lasts
};

/** The same state machine as EnemyBrain in TEneAm01.h built for the mole states run in phases 0 on entry 1 each
 * frame 2 to check for a change 3 on exit */
struct MoleBrainRoot {
	int mState;             // _00
	int mNext;              // _04 an action to ask the owner for
	TEneBoss03Mole* mOwner; // _08

	inline MoleBrainRoot()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt()               = 0; // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~MoleBrainRoot() { }             // _10 0x8007D584

	// _14 0x8007D5CC
	inline virtual void update(TEneBoss03Mole* owner)
	{
		mOwner = owner;
		mNext  = -1;
		if (checkInterrupt() == 0) {
			onPhase(mState, 2);
		}
		onPhase(mState, 1);
		if (mNext != -1) {
			mOwner->setAction(mNext);
		}
	}

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}

	inline void start(TEneBoss03Mole* owner)
	{
		mOwner = owner;
		mNext  = -1;
		mState = 0;
		onPhase(mState, 0);
		if (mNext != -1) {
			mOwner->setAction(mNext);
		}
	}
};

MoleBrain* createMoleBrain(TEneBoss03Mole* owner); // 0x8007C1C0

/** A layer with nothing of its own */
struct MoleBrainBase : public MoleBrainRoot {
	MoleBrainBase();
	virtual ~MoleBrainBase();
};

/**
 * What the mole does. It appears, then digs a few times, and between digs circles the player underground before
 * surfacing to attack. Being hurt past a step makes it dive and come up somewhere else.
 * @size 0x70
 */
struct MoleBrain : public MoleBrainBase {
	enum State {
		STATE_APPEAR = 0,
		STATE_DIG    = 1,
		STATE_HUNT   = 2,
		STATE_SHOW   = 3,
		STATE_DIE    = 4,
	};

	MoleBrain(TEneBoss03Mole* owner);
	virtual ~MoleBrain();

	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	int randomWait();
	int look();
	int getStage();
	int getDigCount();
	int getAttack();
	void startCircle();
	void circle();
	int isNearGoal();
	void startCamera();
	void updateCamera();
	void endCamera();
	void setSpeed();
	void saveHealth();
	int isHurtStep();
	int checkHurt();
	void phaseAppear(int phase);
	void phaseDig(int phase);
	void phaseHunt(int phase);
	void phaseShow(int phase);
	void phaseDie(int phase);

	EnemySight mSight;         // _10
	f32 _40;                   // _40
	int _44;                   // _44
	int _48;                   // _48
	TCamConversation* mCamera; // _4C the close-up while it surfaces
	int _50;                   // _50
	int mSavedHealth;          // _54
	int mTurn;                 // _58 0x95 or -0x95 which way it circles
	int mWave;                 // _5C the angle of the in and out sway while circling
	int mStep;                 // _60
	int mTimer;                // _64
	int mCount;                // _68
	int mDigs;                 // _6C
};

extern "C" {
void createBoss03Magma();
void initTEneBoss03Mole();
NJS_OBJECT** modelTEneBoss03Mole();
NJS_MOTION** motionTEneBoss03Mole(int);
NJS_TEXLIST** shapeTEneBoss03Mole();
void shutdownTEneBoss03Mole();
}

extern "C" {
void bindTEneBoss03Mole(const Vec3f*);
}

extern "C" {
TObject* createTEneBoss03Mole(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern ResBank gBoss03Bank;
extern NJS_OBJECT** sMoleModels;
extern NJS_TEXLIST** sMoleTextures;
extern NJS_MOTION** sMoleMotions;
extern MoleParams* gMoleParams;
extern const char* gTEneBoss03MoleName;
extern const EnemyFrameKey* sMoleFrameKeys[4];
extern const int sMoleTrackNodes[];
}

#endif
