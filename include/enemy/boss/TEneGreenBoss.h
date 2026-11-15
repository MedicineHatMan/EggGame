#ifndef _TENEGREENBOSS_H
#define _TENEGREENBOSS_H
#include "Ninja/ninja.h"
#include "core/types.h"
struct SetDesc;
struct TObject;
struct GjActor;
struct DrawTask;
class TObjGreenGrassField;
struct CharaColli;

#include "enemy/ObjEnemy.h"
#include "enemy/boss/BossParts.h"
#include "egg/ObjEgg.h"
#include "object/ObjCommon.h"
#include "enemy/EnemyLight.h"
#include "enemy/EnemySight.h"
#include "enemy/TEneAm01Parts.h"
#include "engine/FxHolder.h"
#include "sound/SeHolder.h"
#include "sound/SoundHandle.h"

class TCamPath;

// The green boss class 0x0900 and the brain that drives it

class GreenBrain;

/**
 * The boss's tuning, from enemyParamGet(1). The int[3] and f32[3] tables are indexed by the stage, which
 * thresholdIndex picks from the health and mHealthSteps.
 * @size 0xA8
 */
struct GreenBossParams {
	int mHealth;         // _00
	int mHealthSteps[3]; // _04
	int mHurtDrop[3];    // _10 how much health lost since saveHealth ends a walk
	int mPathArg;        // _1C passed to every path query
	int mWalkTime[3];    // _20 how long the walk goes on
	int mDazeTime[3];    // _2C how long it stays dazed
	int mChaseTime[3];   // _38
	int mAttackTime[3];  // _44
	int mRestTime[3];    // _50
	f32 mSpeed[3];       // _5C
	f32 mRange[3];       // _68 close enough to attack
	f32 mSlowRate;       // _74
	f32 mFastRate;       // _78
	f32 mSwingRadius;    // _7C of the sphere swung around in action 12
	f32 mBoxSize[3];     // _80 of the box slammed down in action 13
	int mFacing[3];      // _8C the angle within which it counts as facing the player
	int mTurnRate;       // _98
	int mTurnRate2;      // _9C
	int mSpinRate;       // _A0
	int mHitAngle;       // _A4 the front arc an egg has to come from to hit
};

/**
 * The green boss.
 * @size 0x444
 */
class TEneGreenBoss : public TObjEnemy {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	enum {
		// mBossFlags
		BOSS_ALPHA     = 0x1,  // drawn see-through
		BOSS_ENGAGED   = 0x2,  // getHitKind says 3
		BOSS_GUARD     = 0x4,  // onHit lets nothing through
		BOSS_RECOVERED = 0x8,  // set once a daze ends
		BOSS_FELL      = 0x10, // set once it has gone down
	};

	TEneGreenBoss(TObject* parent, SetDesc* desc);
	virtual ~TEneGreenBoss(); // 0x8015FBE8

	virtual void vfn_10();                                            // _10 0x80163120
	virtual void render();                                            // _1C 0x801631C8
	virtual void readEnemyDesc(SetDesc* desc);                        // _38 0x8015FFC4
	virtual void vfn_3C();                                            // _3C 0x801630E4 prints the health
	virtual f32 vfn_40();                                             // _40 0x8015FDC8
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8015FFE4
	virtual void vfn_6C();                                            // _6C 0x8016056C
	virtual void updateBody();                                        // _70 0x80160490
	virtual void setPos(const Vec3f* pos);                            // _74 0x8015FD84
	virtual void syncCollider();                                      // _78 0x8015FDD0
	virtual void onActionPhase(int action, int phase);                // _7C 0x801611C8
	virtual int getDamage(EnemyHit* hit);                             // _A4 0x801607F8
	virtual int getHitKind(const void* touch, HitInfo* info);         // _A8 0x801607E0

	void init();                                               // 0x8015F844
	void syncSwing();                                          // 0x8015FE9C
	void syncBox();                                            // 0x8015FEFC
	void setLookTarget(const Vec3f* pos, int angle);           // 0x801600E0
	void setHealth(int health);                                // 0x80160100
	void clearEngaged();                                       // 0x8016010C
	void setEngaged();                                         // 0x8016011C
	void clearGuard();                                         // 0x8016012C
	void setGuard();                                           // 0x8016013C
	void clearRecovered();                                     // 0x8016014C
	void setRecovered();                                       // 0x8016015C
	u32 isRecovered();                                         // 0x8016016C
	void clearFell();                                          // 0x80160178
	void setFell();                                            // 0x80160188
	void beginDeath();                                         // 0x80160198
	void hurtReact();                                          // 0x801601E8
	int isAction0Done();                                       // 0x8016022C
	int isAction1Done();                                       // 0x80160250
	int isAction2Done();                                       // 0x80160274
	int isAction3Done();                                       // 0x80160298
	int isAction4Done();                                       // 0x801602BC
	int isAction5Done();                                       // 0x801602E0
	int isAction6Done();                                       // 0x80160304
	int isAction7Done();                                       // 0x80160328
	int isAction8Done();                                       // 0x8016034C
	int isAction9Done();                                       // 0x80160370
	int isAction11Done();                                      // 0x80160394
	int isAction12Done();                                      // 0x801603B8
	int isAction13Done();                                      // 0x801603DC
	int isAction14Done();                                      // 0x80160400
	int isAction15Done();                                      // 0x80160424
	int isAction16Done();                                      // 0x80160448
	int isAction20Done();                                      // 0x8016046C
	void makeBrain();                                          // 0x801605F0
	void beginNodes();                                         // 0x80160620
	void endNodes();                                           // 0x80160650
	void updatePose();                                         // 0x80160670
	void updatePoseRooted();                                   // 0x801606DC
	void calcPoints();                                         // 0x80160780
	void bodyMatrix();                                         // 0x80160810
	void updateShadow();                                       // 0x80160864
	void spawnFx(int model, int motion);                       // 0x801608EC
	void moveByNode(int node, Vec3f* out);                     // 0x8016097C
	void damp(f32 rate);                                       // 0x80160A14
	int moveBody(Vec3f* vel);                                  // 0x80160A30
	void slideMove(f32 rate);                                  // 0x80160AB0
	f32 blendTime(s16 key);                                    // 0x80160AFC
	void setScale(f32 scale);                                  // 0x80160B20
	int shrink(f32 min, f32 rate);                             // 0x80160BC0
	int grow(f32 max, f32 rate);                               // 0x80160C18
	void startDust();                                          // 0x80160C70
	void moveDust();                                           // 0x80160D10
	void stopDust();                                           // 0x80160DA0
	void startSplash();                                        // 0x80160E04
	void moveSplash();                                         // 0x80160E84
	void stopSplash();                                         // 0x80160EF0
	void startTrail();                                         // 0x80160F34
	void moveTrail();                                          // 0x80160F7C
	void stopTrail();                                          // 0x80160FD0
	void startSpinFx();                                        // 0x80160FF4
	void moveSpinFx(const Vec3f* pos, int angle);              // 0x80161094
	void stopSpinFx();                                         // 0x80161164
	void actAppear(int phase);                                 // 0x80161308 action 0
	void actStand(int phase);                                  // 0x80161518 action 17
	void actTurn(int phase);                                   // 0x80161598 action 18
	void actIdle(int phase);                                   // 0x80161614 action 1
	void actShrink(int phase);                                 // 0x80161690 action 2
	int turnStep(int* angle, f32* step, int rate, int target); // 0x80161780
	void actTurnLeft(int phase);                               // 0x80161898 action 3
	void actTurnRight(int phase);                              // 0x80161974 action 4
	void actWalk(int phase);                                   // 0x80161A50 action 5
	void actRun(int phase);                                    // 0x80161BEC action 6
	int isDazeOver(int timer);                                 // 0x80161D94
	void actDaze(int phase);                                   // 0x80161DF0 action 7
	void actGrow(int phase);                                   // 0x80161F30 action 8
	void actDown(int phase);                                   // 0x8016209C action 9
	void actSink(int phase);                                   // 0x80162274 action 10
	void actRoar(int phase);                                   // 0x8016238C action 11
	void actSwing(int phase);                                  // 0x80162478 action 12
	void actSlam(int phase);                                   // 0x8016263C action 13
	void actRise(int phase);                                   // 0x801627AC action 14
	void addRotY(int delta);                                   // 0x8016284C
	void actSpin(int phase);                                   // 0x8016285C action 15
	void actSpinEnd(int phase);                                // 0x80162AA4 action 16
	void beginHurt();                                          // 0x80162B2C
	void endHurt();                                            // 0x80162B6C
	void actHurt(int phase);                                   // 0x80162BAC action 19
	void actHurtSink(int phase);                               // 0x80162C44 action 20
	void actDie(int phase);                                    // 0x80162D68 action 21

	EnemyMotion mMotion;     // _158
	GreenBrain* mBrain;      // _194
	GjActor* mPose;          // _198
	DrawTask* mDrawTask;     // _19C drawObjectTask
	u32 mBossFlags;          // _1A0
	Vec3f mHitDir;           // _1A4
	HitVec mNodes[13];       // _1B0 the points tracked on the body from sGreenBossNodes
	Vec3f mMoveTarget;       // _24C
	Vec3f mLookTarget;       // _258
	int mLookAngle;          // _264
	int mMaxHealth;          // _268
	ObjShadow mShadow;       // _26C
	EnemyLight mLight;       // _2A4
	EnemyTint mTint;         // _2E4
	BossSuit mSuit;          // _2EC
	TObject* mFx[10];        // _324 kept particles 0-2 dust 3 the sink splash 4-5 splash 6 trail 7-9 spin
	FxAttach mStepFx;        // _34C
	FxAttach mSlamFx;        // _380
	FxAttach mSwingFx;       // _3B4
	SoundHandle mLoop;       // _3E8
	CharaColli* mSwingColli; // _414 a sphere on in action 12
	CharaColli* mBoxColli;   // _418 a box on in action 13
	f32 mScale;              // _41C
	f32 mSpeed;              // _420 how fast the motions run
	TCamPath* mModelFx;      // _424 from createCamPath
	SeCue mSe;               // _428
	int mActStep;            // _42C
	int mActTimer;           // _430
	int mActLimit;           // _434
	int mBaseAngle;          // _438 the facing an action started from
	int mSpinRate;           // _43C
	f32 mActTime;            // _440
};

extern "C" {
extern GreenBossParams* gGreenBossParams; // 0x802C57FC
}

/** The same state machine as OrangeBrainRoot in TEneOrangeBoss.h for the green boss states run in phases 0 on entry
 * 1 each frame 2 to check for a change 3 on exit */
struct GreenBrainRoot {
	int mState;            // _00
	int mNext;             // _04 an action to ask the owner for
	TEneGreenBoss* mOwner; // _08

	inline GreenBrainRoot()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt()               = 0; // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~GreenBrainRoot() { }            // _10 0x80164B40

	// _14 0x801632C0 Defined in TEneGreenBossBrain.cpp
	inline virtual void update(TEneGreenBoss* owner);

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** A layer with nothing of its own */
struct GreenBrainBase : public GreenBrainRoot {
	GreenBrainBase();          // 0x80163358
	virtual ~GreenBrainBase(); // 0x8016338C
};

/**
 * What the green boss does. Vtable 0x80261878.
 * @size 0x64
 */
struct GreenBrain : public GreenBrainBase {
	enum State {
		STATE_APPEAR = 0,
		STATE_WAKE   = 1,
		STATE_WALK   = 2,
		STATE_DAZE   = 3,
		STATE_HURT   = 4,
		STATE_GROW   = 5,
		STATE_CHASE  = 6,
		STATE_SHRINK = 7,
		STATE_ATTACK = 8,
		STATE_SPIN   = 9,
		STATE_10     = 10,
		STATE_DIE    = 11,
	};

	GreenBrain(TEneGreenBoss* owner); // 0x80163430
	virtual ~GreenBrain();            // 0x8016351C

	virtual int checkInterrupt();               // 0x80163584
	virtual void onPhase(int state, int phase); // 0x80163AE4

	void look();                         // 0x80163638
	int findNode();                      // 0x80163690
	void aim();                          // 0x801636F0
	void lookAt(Vec3f* pos);             // 0x80163720
	void saveHealth();                   // 0x8016377C
	int getStage();                      // 0x8016378C
	int isHurtStep();                    // 0x801637BC
	int isOver(int count, int* table);   // 0x80163828
	int isWalkOver(int count);           // 0x80163874
	int isChaseOver(int count);          // 0x8016389C
	int isAttackOver(int count);         // 0x801638C4
	int isRestOver(int count);           // 0x801638EC
	int isInRange(Vec3f* pos);           // 0x80163914
	int isFacing(int angle);             // 0x80163998
	void setSpeed(f32 rate);             // 0x801639F4
	void setNode(int node);              // 0x80163A50
	void splash(f32 radius, Vec3f* pos); // 0x80163A60
	void pathOn();                       // 0x80163A9C
	void pathOff();                      // 0x80163AC0
	void phaseAppear(int phase);         // 0x80163BAC
	void phaseWake(int phase);           // 0x80163CD8
	void phaseWalk(int phase);           // 0x80163E3C
	void phaseDaze(int phase);           // 0x8016415C
	void phaseHurt(int phase);           // 0x80164204
	void phaseGrow(int phase);           // 0x801642D0
	void phaseChase(int phase);          // 0x801643F0
	void phaseShrink(int phase);         // 0x80164510
	void phaseAttack(int phase);         // 0x801645B8
	void phaseSpin(int phase);           // 0x80164898
	void phaseState10(int phase);        // 0x80164A80
	void phaseDie(int phase);            // 0x80164ADC

	TObjGreenGrassField* mPath; // _10 the walk graph from createTObjGreenGrassField
	EnemySight mSight;          // _14
	int mSavedHealth;           // _44
	int mNode;                  // _48 the graph point it goes to
	int mPrevNode;              // _4C
	int mStep;                  // _50
	int mTimer;                 // _54
	int mTimer2;                // _58
	int _5C;                    // _5C
	int _60;                    // _60
};

GreenBrain* createGreenBrain(TEneGreenBoss* owner); // 0x801633E8

extern "C" {
void initTEneGreenBoss();
void* paramsTEneGreenBoss();
void shutdownTEneGreenBoss();
}

extern "C" {
TObject* createTEneGreenBoss(TObject*, SetDesc*);
}

#include "engine/MotionPlayer.h"

#include "io/ResBank.h"
extern "C" {
extern ResBank sGreenBossBank;
extern FxBankFile sGreenBossFx;
extern NJS_OBJECT** sGreenBossModels;
extern NJS_TEXLIST** sGreenBossTextures;
extern NJS_MOTION** sGreenBossMotion;
extern MotionEntry sGreenBossMotionKeys[23];
}

#endif
