#ifndef _TENEPURPLEBOSS_H
#define _TENEPURPLEBOSS_H
struct EnemyFrameKey;
struct SetDesc;
struct TObject;
struct GjActor;
struct DrawTask;

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

// The purple boss class 0x0400 an ice skater and the brain that drives it

class PurpleBrain;

/** The boss's tuning from enemyParamGet(4) */
struct PurpleBossParams {
	f32 mBallRadius;      // _00
	f32 mBallSpeeds[3];   // _04
	int mHealth;          // _10
	int mHealthSteps[3];  // _14 the thresholds PurpleBrain::getStage counts past
	f32 mSkateSpeed[3];   // _20 by stage
	int mSpinCount[3];    // _2C spins before it rests by stage
	int mHealthSteps2[6]; // _38 the thresholds for the throws
	int mBallCount[6];    // _50 balls in a throw
	int mBallLimit[6];    // _68 balls in the rink at once
};

/**
 * The purple boss.
 * @size 0x318
 */
class TEnePurpleBoss : public TObjEnemy {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	enum {
		// mBossFlags
		BOSS_HITTABLE = 0x1,  // skating the player can bounce it
		BOSS_BOUNCED  = 0x2,  // a player hit it while hittable
		BOSS_WALLED   = 0x4,  // ran into the wall while skating
		BOSS_SPINNING = 0x8,  // drawn on its quaternion the collider is the ice block
		BOSS_ICE      = 0x10, // the ice block shows
		BOSS_BUMPED   = 0x20, // the spin hit a wall this frame
		BOSS_DASH     = 0x40,
		BOSS_TRAIL    = 0x80,  // the skate trail
		BOSS_SPRAY    = 0x100, // the ice spray
		BOSS_MELT     = 0x200, // the death melt drawn with the suit
		BOSS_CLOSED   = 0x400, // onHit says no
	};

	TEnePurpleBoss(TObject* parent, SetDesc* desc);
	virtual ~TEnePurpleBoss(); // 0x800C825C

	virtual void vfn_10();                                            // _10 0x800CAE2C
	virtual void render();                                            // _1C 0x800CAED4
	virtual void readEnemyDesc(SetDesc* desc);                        // _38 0x800C849C
	virtual void vfn_3C();                                            // _3C 0x800CADF0 prints the health
	virtual f32 vfn_40();                                             // _40 0x800C8408
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x800C84E0
	virtual void vfn_6C();                                            // _6C 0x800C88D8
	virtual void updateBody();                                        // _70 0x800C87F4
	virtual void setPos(const Vec3f* pos);                            // _74 0x800C83C4
	virtual void syncCollider();                                      // _78 0x800C8410
	virtual void onActionPhase(int action, int phase);                // _7C 0x800C9734
	virtual int getDamage(EnemyHit* hit);                             // _A4 0x800C8B50
	virtual int getHitKind(const void* touch, HitInfo* info);         // _A8 0x800C8B38

	void init(SetDesc* desc);                                          // 0x800C7FF8
	void setTargetAngle(int angle);                                    // 0x800C859C
	void setHealth(int health);                                        // 0x800C85A4
	int isAction0Done();                                               // 0x800C85B0
	int isAction1Done();                                               // 0x800C85D4
	int wasBounced();                                                  // 0x800C85F8
	int hitWall();                                                     // 0x800C8604
	int isAction3Done();                                               // 0x800C8610
	int isAction4Done();                                               // 0x800C8634
	int isAction6Done();                                               // 0x800C8658
	int isAction7Done();                                               // 0x800C867C
	void setSkateSpeed(f32 speed);                                     // 0x800C86A0
	int wasBumped();                                                   // 0x800C86A8
	void throwBalls(int count);                                        // 0x800C86B4
	int isAction10Done();                                              // 0x800C8710
	int isAction11Done();                                              // 0x800C8734
	void stageBeaten();                                                // 0x800C8758
	void hurtReact();                                                  // 0x800C87A8
	int dieAction();                                                   // 0x800C87EC
	void makeBrain();                                                  // 0x800C8954
	void beginNodes();                                                 // 0x800C8984
	void endNodes();                                                   // 0x800C89B4
	void updatePose();                                                 // 0x800C89D4
	void updatePoseRooted();                                           // 0x800C8A20
	void calcPoints();                                                 // 0x800C8AD8
	void bodyMatrix();                                                 // 0x800C8B68
	static void gravity(Vec3f* vel);                                   // 0x800C8BF0
	int moveBody(Vec3f* vel);                                          // 0x800C8C04
	int fallMove(Vec3f* vel);                                          // 0x800C8CB8
	void skateMove(f32 drag);                                          // 0x800C8CF8
	void spinMove();                                                   // 0x800C8D74
	void openHit();                                                    // 0x800C90A4
	void closeHit();                                                   // 0x800C90C0
	void colliOn();                                                    // 0x800C90D0
	void colliOff();                                                   // 0x800C9100
	void resetSpeed();                                                 // 0x800C9130
	void aimSpeed();                                                   // 0x800C913C
	void dashBack(f32 scale);                                          // 0x800C921C
	void dashStep(int* step);                                          // 0x800C928C
	void endDash();                                                    // 0x800C9388
	void playSpin();                                                   // 0x800C939C
	void showIce();                                                    // 0x800C93E4
	void hideIce();                                                    // 0x800C9404
	void dustBurst(Vec3f* pos);                                        // 0x800C941C
	void bigDustBurst(Vec3f* pos);                                     // 0x800C9464
	void clearSkid();                                                  // 0x800C94A0
	void skidMark(int angle);                                          // 0x800C94AC
	void killSparks();                                                 // 0x800C9510
	void keepSparks(Vec3f* pos, int angle);                            // 0x800C9554
	void trailOff();                                                   // 0x800C95B8
	void trailOn();                                                    // 0x800C95C8
	void sprayOn();                                                    // 0x800C95D8
	void updateTrails();                                               // 0x800C95F4
	void spawnFx(int model, int motion);                               // 0x800C96A4
	void actAppear(int phase);                                         // 0x800C9808 action 0
	static int turnStep(f32 frame, int* angle, f32* rate, int target); // 0x800C9B14
	void actTurn(int phase);                                           // 0x800C9C34 action 1
	static int pushOff(SetTransform* body, void* anim, int angle);     // 0x800C9D00
	static int pastLaunch(void* anim);                                 // 0x800C9D9C
	void actSkate(int phase);                                          // 0x800C9DCC action 2
	static f32 brakeRate(f32 frame);                                   // 0x800CA018
	void actBrake(int phase);                                          // 0x800CA050 action 3
	void actDash(int phase);                                           // 0x800CA160 action 4
	void actDazed(int phase);                                          // 0x800CA230 action 5
	void actSlam(int phase);                                           // 0x800CA344 action 6
	void actFreeze(int phase);                                         // 0x800CA4F0 action 7
	void actSpinUp(int phase);                                         // 0x800CA730 action 8
	void actSpin(int phase);                                           // 0x800CA860 action 9
	void actDrop(int phase);                                           // 0x800CA99C action 10
	void actHurt(int phase);                                           // 0x800CAA94 action 11
	void actDie(int phase);                                            // 0x800CAB64 action 12

	PurpleBrain* mBrain; // _158
	GjActor* mPose;      // _15C
	DrawTask* mDrawTask; // _160 drawObjectTask
	u32 mBossFlags;      // _164
	Vec3f mHitDir;       // _168
	HitVec mNodes[6];    // _174 the points tracked on the body from sPurpleBossNodes
	Vec3f _1BC;          // _1BC
	Vec3f _1C8;          // _1C8
	int mTargetAngle;    // _1D4 set by the brain from where the player is
	int mMaxHealth;      // _1D8
	TObject* mDust;      // _1DC
	ObjShadow mShadow;   // _1E0
	EnemyLight mLight;   // _218
	EnemyTint mTint;     // _258
	f32 mSpeed;          // _260 skating
	f32 mSpinSpeed;      // _264
	BossSuit mSuit;      // _268
	TCamPath* mFx;       // _2A0
	SeHolder mSe;        // _2A4
	SeCue mCue;          // _2D8
	TObject* mTrail;     // _2DC
	TObject* mSpray;     // _2E0
	TObject* mSpark[2];  // _2E4
	f32 mIceScale;       // _2EC
	f32 mIceY;           // _2F0
	f32 mQuat[4];        // _2F4 the spin
	int mSkid;           // _304
	int mActStep;        // _308
	int mActTimer;       // _30C
	int mTurnRate;       // _310
	f32 mTurnSpeed;      // _314
};

extern "C" {
extern PurpleBossParams* gPurpleBossParams; // 0x802C5214
}

/** The same state machine as OrangeBrainRoot in TEneOrangeBoss.h for the purple boss states run in phases 0 on entry
 * 1 each frame 2 to check for a change 3 on exit */
struct PurpleBrainRoot {
	int mState;             // _00
	int mNext;              // _04 an action to ask the owner for
	TEnePurpleBoss* mOwner; // _08

	inline PurpleBrainRoot()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt()               = 0; // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~PurpleBrainRoot() { }           // _10 0x800CBD44

	// _14 0x800CB09C Defined in TEnePurpleBossBrain.cpp
	inline virtual void update(TEnePurpleBoss* owner);

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}

	inline void start(TEnePurpleBoss* owner)
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

/** A layer with nothing of its own */
struct PurpleBrainBase : public PurpleBrainRoot {
	PurpleBrainBase();          // 0x800CB134
	virtual ~PurpleBrainBase(); // 0x800CB168
};

/**
 * What the purple boss does. Vtable 0x8025A310.
 * @size 0x4C
 */
struct PurpleBrain : public PurpleBrainBase {
	enum State {
		STATE_APPEAR  = 0,
		STATE_CHASE   = 1,
		STATE_BOUNCED = 2, // a player knocked it back
		STATE_DASH    = 3, // ran into the wall dazed then the freeze
		STATE_DROP    = 4, // after the spin dazed then the chase
		STATE_FREEZE  = 5,
		STATE_SPIN    = 6, // throws ice balls each time it hits the wall
		STATE_DIE     = 7,
	};

	PurpleBrain(TEnePurpleBoss* owner); // 0x800CB20C
	virtual ~PurpleBrain();             // 0x800CB2B8

	virtual int checkInterrupt();               // 0x800CB320
	virtual void onPhase(int state, int phase); // 0x800CB560

	int look();                     // 0x800CB3E0
	void aim();                     // 0x800CB454
	int getStage(const int* steps); // 0x800CB480
	void setSpeed();                // 0x800CB4B8
	void throwBalls();              // 0x800CB504
	void phaseAppear(int phase);    // 0x800CB5F8
	void phaseChase(int phase);     // 0x800CB6E0
	void phaseBounced(int phase);   // 0x800CB838
	void phaseDash(int phase);      // 0x800CB8D8 also STATE_DROP
	void phaseFreeze(int phase);    // 0x800CBAF4
	void phaseSpin(int phase);      // 0x800CBC0C
	void phaseDie(int phase);       // 0x800CBD00

	EnemySight mSight; // _10
	int mWait;         // _40 frames before it looks again
	int mStep;         // _44
	int mTimer;        // _48
};

PurpleBrain* createPurpleBrain(TEnePurpleBoss* owner); // 0x800CB1C4

extern "C" {
void initTEnePurpleBoss();
NJS_OBJECT* modelAtTEnePurpleBoss(int);
void* paramsTEnePurpleBoss();
NJS_TEXLIST* shapeTEnePurpleBoss();
void shutdownTEnePurpleBoss();
}

extern "C" {
TObject* createTEnePurpleBoss(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern ResBank sPurpleBossBank;
extern EnemyFrameKey* sPurpleBossFrameKeys[];
extern FxBankFile sPurpleBossFx;
extern NJS_OBJECT** sPurpleBossModels;
extern NJS_TEXLIST** sPurpleBossTextures;
extern NJS_MOTION** sPurpleBossMotion;
extern int gEggFeedFlag;
}

#endif
