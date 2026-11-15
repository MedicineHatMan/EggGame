#ifndef _TENEORANGEBOSS_H
#define _TENEORANGEBOSS_H
struct EnemyFrameKey;
struct SetDesc;
struct TObject;
struct DrawTask;
struct CharaColli;

#include "enemy/ObjEnemy.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "enemy/boss/BossParts.h"
#include "egg/ObjEgg.h"
#include "object/ObjCommon.h"
#include "enemy/EnemyLight.h"
#include "enemy/EnemySight.h"
#include "enemy/TEneAm01Parts.h"
#include "enemy/boss/TEneOrangeBossMap.h"
#include "sound/SeHolder.h"

class TCamPath;

// The orange boss class 0x0500 and the brain that drives it

class OrangeBrain;

/**
 * The boss's tuning, from enemyParamGet(5).
 * @size 0x30
 */
struct OrangeBossParams {
	int mHealth;          // _00
	int mHealthSteps[2];  // _04 the thresholds OrangeBrain::getStage counts past
	int mHurtDrop[2];     // _0C how much health lost since saveHealth ends a stage
	int mCenterWait;      // _14 frames the middle mirror stays up
	f32 mPush;            // _18 how hard the spin pushes the player away
	int mFlashFrames;     // _1C
	f32 mFlashRamp;       // _20
	int mDazeWait;        // _24
	int mPunchWait;       // _28
	int mFlashWait;       // _2C
};

/**
 * The orange boss.
 * @size 0x5AC
 */
class TEneOrangeBossBall;

class TEneOrangeBoss : public TObjEnemy {
public:
	inline void operator delete(void* mem);

	enum {
		// mBossFlags
		BOSS_HURT       = 0x8,   // with BOSS_DOWN the collider is off
		BOSS_DOWN       = 0x10,
		BOSS_READY      = 0x20,
		BOSS_HOLD       = 0x80,  // between saveHealth and the end of a stage
		BOSS_PUNCH_OUT  = 0x200, // a punch is flying
	};

	TEneOrangeBoss(TObject* parent, SetDesc* desc);
	virtual ~TEneOrangeBoss();                   // 0x80114C98

	virtual void vfn_10();                       // _10 0x80119FF0
	virtual void render();                       // _1C 0x8011A210
	virtual void readEnemyDesc(SetDesc* desc);   // _38 0x80115020
	virtual void vfn_3C();                       // _3C 0x80119FB4 prints the health
	virtual f32 vfn_40();                        // _40 0x80114E70
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x80115064
	virtual void vfn_6C();                       // _6C 0x80115758
	virtual void updateBody();                   // _70 0x801155E0
	virtual void setPos(const Vec3f* pos);             // _74 0x80114E2C
	virtual void syncCollider();                 // _78 0x80114FCC
	virtual void onActionPhase(int action, int phase); // _7C 0x801168C0
	virtual int getDamage(EnemyHit* hit);        // _A4 0x80115A3C
	virtual int getHitKind(const void* touch, HitInfo* info); // _A8 0x80115A24

	void init(SetDesc* desc);                    // 0x801148AC
	void setLookTarget(const Vec3f* pos);        // 0x8011506C
	void setSlot(int slot);                      // 0x80115088
	void setMoveTarget(const Vec3f* pos, int arg); // 0x801150B8
	void setHealth(int health);                  // 0x801150D8
	void clearHold();                            // 0x801150E4
	void setHold();                              // 0x801150F4
	void stageBeaten();                          // 0x80115104
	void hurtReact();                            // 0x80115154
	void footDust();                             // 0x80115198
	void landSplash();                           // 0x801151FC
	void clearLand();                            // 0x80115248
	int isLandClear();                           // 0x80115254
	void setLand();                              // 0x80115264
	int takeLand();                              // 0x80115270
	void killBalls();                            // 0x80115294
	int isAction0Done();                         // 0x801152D0
	int isAction4Done();                         // 0x801152F4
	int isAction2Done();                         // 0x80115318
	u32 isReady();                               // 0x8011533C
	int isAction5Done();                         // 0x80115348
	int isAction6Done();                         // 0x8011536C
	int isAction8Done();                         // 0x80115390
	int isAction10Done();                        // 0x801153B4
	void setFacing(const Vec3f* pos, int arg, u16 id); // 0x801153D8
	int isAction12Done();                        // 0x801153FC
	int isAction13Done();                        // 0x80115420
	int isAction14Done();                        // 0x80115444
	void throwPunch();                           // 0x80115468
	void clearPunchOut();                        // 0x80115550
	int isPunchOut();                            // 0x80115560
	int isAction15Done();                        // 0x8011556C
	int isAction17Done();                        // 0x80115590
	int isAction18Done();                        // 0x801155B4
	int dieAction();                             // 0x801155D8
	void makeBrain();                            // 0x801157D0
	void beginNodes();                           // 0x80115800
	void endNodes();                             // 0x80115878
	void updatePose();                           // 0x80115898
	void updatePoseRooted();                     // 0x80115908
	void calcPoints();                           // 0x801159C4
	static void localMatrix();                   // 0x80115A54 scales the model the yellow boss's orange crystal uses it too
	void bodyMatrix();                           // 0x80115A84
	void updateShadow();                         // 0x80115AC4
	static void gravity(Vec3f* vel);             // 0x80115C04
	void moveByNode(f32 rate, int node, Vec3f* out); // 0x80115C18
	int moveBody(Vec3f* vel);                    // 0x80115CC4
	void fallMove(Vec3f* vel);                   // 0x80115D7C
	void slideMove(f32 drag);                    // 0x80115DBC
	void jumpAt(f32 t);                          // 0x80115E38
	void slideTo(f32 t, f32 decel);              // 0x80115E80
	void playIdle();                             // 0x80115F68
	f32 planJump(f32 dist, f32* step, f32* speed); // 0x80115FB0
	void keysWalk(f32 total);                    // 0x80116020
	void keysJump(f32 total);                    // 0x801160D8
	void keysHop(f32 total);                     // 0x801161F4
	void keysThrow(f32 total, f32* t0, f32* t1); // 0x80116358
	int flipAngle(f32 t, f32 t0, f32 t1); // 0x80116490
	void followFoot(f32 t);                      // 0x801164B8
	void placeAtSlot();                          // 0x8011651C
	void placeAtMirror();                        // 0x801165A0
	void spawnFx(int model, int motion);         // 0x80116608
	void setAimNode(int node);                   // 0x80116698
	f32 aimDot();                                // 0x801166A8
	void aimRing();                              // 0x80116730
	void aimCenter();                            // 0x801167AC
	void bodyCylOn();                            // 0x80116824
	void bodyCylOff();                           // 0x8011689C
	void actAppear(int phase);                   // 0x801169E8 action 0
	void actCircle(int phase);                   // 0x80116CD0 action 1
	void actJumpIn(int phase);                   // 0x80116ECC action 2
	void actThrow(int phase);                    // 0x801173A0 action 3
	void actSettle(int phase);                   // 0x8011762C action 4
	void actHop(int phase);                      // 0x8011773C action 5
	void actDive(int phase);                     // 0x801179A8 action 6
	void actDaze(int phase);                     // 0x80117D84 action 7
	void actRecover(int phase);                  // 0x80117E94 action 8
	void actFall(int phase);                     // 0x80117F6C action 9
	f32 planWalkJump();                          // 0x80118030
	f32 keysLand();                              // 0x801180C0
	f32 planLeap(f32* t0, f32* t1);              // 0x801181C0
	void actToCenter(int phase);                 // 0x801182E4 action 10
	void actSpin(int phase);                     // 0x8011863C action 11
	f32 planFall(f32* t0, f32* t1);              // 0x801189D0
	void actLeap(int phase);                     // 0x80118BAC action 12
	f32 planCrouch();                            // 0x80118E74
	f32 planVault(f32* t0, f32* t1);             // 0x80118F38
	void actVault(int phase);                    // 0x80119010 action 13
	void actPunch(int phase);                    // 0x801192C4 action 14
	void actTurn(int phase);                     // 0x80119510 action 15
	void pickMotions(int* a, int* b);            // 0x80119670
	void setupDecoys();                          // 0x8011978C
	void actDecoys(int phase);                   // 0x80119870 action 16
	f32 planReturn();                            // 0x80119934
	void actReturn(int phase);                   // 0x80119AAC action 17
	void actHurt(int phase);                     // 0x80119C90 action 18
	void actDie(int phase);                      // 0x80119D68 action 19
	void drawReflections();                      // 0x8011A0D8
	void drawBody(int mirrored, f32* mtx, void* pose); // 0x8011A248

	EnemyMotion mMotion;   // _158
	OrangeBrain* mBrain;   // _194
	void* mPose;           // _198
	DrawTask* mDrawTask;       // _19C drawObjectTask
	DrawTask* mMirrorTask;     // _1A0 copies the frame for the mirrors
	f32 _1A4[12];          // _1A4
	u32 mBossFlags;        // _1D4
	Vec3f mHitDir;         // _1D8
	HitVec mNodes[16];     // _1E4 the points tracked on the body from sOrangeBossNodes
	Vec3f mMoveTarget;     // _2A4
	int mMoveArg;          // _2B0
	Vec3f mLookTarget;     // _2B4
	int mFacingArg;        // _2C0
	u16 mFacingId;         // _2C4
	u8 _2C6[2];            // _2C6
	Vec3f mJumpFrom;       // _2C8
	int mMaxHealth;        // _2D4
	ObjShadow mShadow;     // _2D8
	EnemyLight mLight;     // _310
	EnemyTint mTint;       // _350
	BossSuit mSuit;        // _358
	TCamPath* mFx;         // _390
	SeCue mSe;             // _394
	CharaColli* mBodyCyl;        // _398 the cylinder it stands in off unless asked for
	int mSlot;             // _39C the mirror it stands at
	Vec3f mSlotPos;        // _3A0
	int mSlotAngle;        // _3AC
	int mAimNode;          // _3B0
	f32 mAimSide;          // _3B4
	TEneOrangeBossBall* mBall; // _3B8
	f32 mFootY;            // _3BC the foot node's height last frame see followFoot
	f32 mRange[4];         // _3C0 two time spans in which the body follows the foot down
	TObject* mGlow[3];     // _3D0
	TObject* mSparks[9];   // _3DC the spin's particles on the nodes in gOrangeSparkNodes
	int mLand;             // _400
	MotionStep mKeys[6];   // _404 the motion timeline motionStepsPlay plays
	u8 _47C[0x50];         // _47C
	EnemyMotion mMotion2;  // _4CC
	EnemyAnim mAnim2;      // _508
	void* mPose2;          // _514
	HitVec mDecoyPos[7];   // _518 the copies in the other seven mirrors
	int mDecoyAngle[7];    // _56C
	int mActStep;          // _588
	int mActTimer;         // _58C
	int mBaseAngle;        // _590 the facing an action started from
	int _594;              // _594
	f32 mActTime;          // _598
	f32 mActEnd;           // _59C
	f32 _5A0;              // _5A0
	f32 _5A4;              // _5A4
	f32 _5A8;              // _5A8
};

extern "C" {
extern OrangeBossParams* gOrangeBossParams; // 0x802C5574
}

/** The same state machine as MoleBrainRoot in TEneBoss03Mole.h for the orange boss states run in phases 0 on entry 1
 * each frame 2 to check for a change 3 on exit */
struct OrangeBrainRoot {
	int mState;             // _00
	int mNext;              // _04 an action to ask the owner for
	TEneOrangeBoss* mOwner; // _08

	inline OrangeBrainRoot()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt() = 0;               // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~OrangeBrainRoot() { }           // _10 0x8011BE4C

	// _14 0x8011A3E0 Defined at the top of TEneOrangeBossBrain.cpp
	inline virtual void update(TEneOrangeBoss* owner);

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}

	inline void start(TEneOrangeBoss* owner)
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
struct OrangeBrainBase : public OrangeBrainRoot {
	OrangeBrainBase();          // 0x8011A478
	virtual ~OrangeBrainBase(); // 0x8011A4AC
};

/**
 * What the orange boss does. Vtable 0x8025DF94.
 * @size 0x5C
 */
struct OrangeBrain : public OrangeBrainBase {
	enum State {
		STATE_APPEAR  = 0,
		STATE_1    = 1,
		STATE_2    = 2,
		STATE_3   = 3,
		STATE_4    = 4,
		STATE_5   = 5,
		STATE_6    = 6,
		STATE_7  = 7,
		STATE_8   = 8,
		STATE_PUNCH   = 9,
		STATE_10   = 10,
		STATE_11   = 11,
		STATE_12    = 12,
		STATE_13 = 13,
		STATE_DIE     = 14,
	};

	OrangeBrain(TEneOrangeBoss* owner); // 0x8011A550
	virtual ~OrangeBrain();             // 0x8011A624

	virtual int checkInterrupt();               // 0x8011A690
	virtual void onPhase(int state, int phase); // 0x8011A8EC

	void look();            // 0x8011A744
	void aim();             // 0x8011A79C
	void saveHealth();      // 0x8011A7C8
	int isHurtStep();       // 0x8011A7F8
	int getStage();         // 0x8011A858
	int playerSlot();       // 0x8011A888
	int attackState();      // 0x8011A8BC
	void phaseAppear(int phase);  // 0x8011A9D8
	void phaseState1(int phase);    // 0x8011AACC
	void phaseState2(int phase);    // 0x8011AC2C
	void phaseState3(int phase);   // 0x8011ACEC
	void phaseState4(int phase);    // 0x8011AEBC
	void phaseState5(int phase);   // 0x8011AF90
	void phaseState6(int phase);    // 0x8011B080
	void phaseState7(int phase);  // 0x8011B1E8
	void phaseState8(int phase);   // 0x8011B37C
	void phasePunch(int phase);   // 0x8011B44C
	void phaseState10(int phase);   // 0x8011B6F4
	void phaseState11(int phase);   // 0x8011B9D4
	void phaseState12(int phase);    // 0x8011BAA8
	void phaseState13(int phase); // 0x8011BCD0
	void phaseDie(int phase);     // 0x8011BDF8

	EnemySight mSight;  // _10
	int mSavedHealth;   // _40
	int mReturnState;   // _44 where STATE_12 goes once it is over
	int mHurtWait;      // _48
	int mHurtRecover;   // _4C 1 = STATE_13 starts at its second part
	int mStep;          // _50
	int mTimer;         // _54
	int mPick;          // _58 the mirror picked in STATE_10
};

OrangeBrain* createOrangeBrain(TEneOrangeBoss* owner); // 0x8011A508

// The orange boss's throw walk shared with the yellow boss's orange crystal
int throwFoot(void* anim);                                            // 0x801170F8 the foot it steps with or -1
int throwHand(int* step, void* anim, Vec3f* nodes, Vec3f* out); // 0x80117190
int throwStepFrame(void* anim);                                       // 0x80117348

struct Vec3f;

extern "C" {
void initTEneOrangeBoss();
NJS_OBJECT* modelAtTEneOrangeBoss(int);
void orangeBossColliderBit8(void*);
void orangeBossFitCollider(void*, Vec3f*);
void* orangeBossMakeCollider(u16);
void* paramsTEneOrangeBoss();
void shutdownTEneOrangeBoss();
NJS_TEXLIST* texListTEneOrangeBoss();
}

extern "C" {
TObject* createTEneOrangeBoss(TObject*, SetDesc*);
}

#include "engine/MotionPlayer.h"
#include "io/ResBank.h"
extern "C" {
extern ResBank sOrangeBossBank;
extern MotionEntry sOrangeBossMotionKeys[];
extern FxBankFile sOrangeBossFx;
extern NJS_OBJECT** sOrangeBossModels;
extern NJS_TEXLIST** sOrangeBossTextures;
extern NJS_MOTION** sOrangeBossMotion;
extern const EnemyFrameKey* sOrangeBossFrameKeys[4];
}

#endif
