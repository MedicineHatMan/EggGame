#ifndef _TENEBOSS02_H
#define _TENEBOSS02_H
struct EnemyFrameKey;
class TParticleGenerator;
struct SetDesc;
struct TObject;
struct GjActor;
struct DrawTask;
struct CharaColli;

#include "enemy/ObjEnemy.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
#include "enemy/boss/BossParts.h"
#include "egg/ObjEgg.h"
#include "object/ObjCommon.h"
#include "enemy/TEneAm01Parts.h"
#include "enemy/EnemySight.h"
#include "sound/SeHolder.h"

class TCamPath;

class TEneBoss02;
struct NJS_OBJECT;

/**
 * The boss's tail, a chain of eight points and the seven matrices between them. Built with 8 points by
 * TEneBoss02::init. A rope: each frame the first point is put at the root, and every other point hangs from the one
 * before at its length, pulled down by mDrop, carried by its speed and bent toward the direction of the link before.
 * See BossTail.cpp, 0x800766EC-0x80076E4C.
 *
 * @size 0x2C
 */
struct BossTail {
	BossTail(int count); // 0x800766EC
	~BossTail();         // 0x80076898

	void reset();                                     // 0x8007698C stops every point
	void savePrev();                                  // 0x800769E4
	void updateVel();                                 // 0x80076A30
	void step(int index, Vec3f* dir);                 // 0x80076AC8
	void update(const Vec3f* root, const Vec3f* dir); // 0x80076D84

	f32 mBend;       // _00 how hard a link turns toward the one before per angle unit
	f32 mDrop;       // _04 the fall a frame
	f32 mSpin;       // _08 angle units per unit of sideways pull
	f32 mDamp;       // _0C of the speed
	int mCount;      // _10
	f32* mMatrices;  // _14 0x30 bytes each
	HitVec* mExtra;  // _18 only cleared
	HitVec* mPoints; // _1C
	HitVec* mPrev;   // _20 the points of the frame before
	HitVec* mVel;    // _24
	f32* mLengths;   // _28 from each point to the one before
};

/** One step of the script motionKeyPlay plays on the boss's own motion */
struct Boss02Key {
	int mKind;  // _00
	f32 mValue; // _04
};

/** The boss's tuning enemyParamGet(2) Most rows have one value for each of the three health tiers */
struct Boss02Params {
	f32 mSuitA;         // _00
	f32 mSuitB;         // _04
	f32 mSuitC;         // _08
	f32 mSuitD;         // _0C
	f32 mSuitBase;      // _10 also lifts the suit's glow
	f32 mSuitE;         // _14
	f32 mSuitTop;       // _18
	int mHealth;        // _1C
	int mTierHealth[2]; // _20 a tier holds while the health is at least this
	int mShotGap[3];    // _28 frames between shots
	f32 mTurnRate[3];   // _34
	int mDives[3];      // _40 dives before it moves on
	int mSlams[3];      // _4C
	int mSlamDamage[3]; // _58 damage that ends the slams early
	f32 mMoveRate[3];   // _64
	int mDiveWait[3];   // _70
	f32 mView[2][3];    // _7C the boss camera above water and below
};

/** The state machine the same shape as BatBrain three words then the vtable at _0C */
struct Boss02Brain {
	int mState;         // _00
	int mNext;          // _04 an action to ask the owner for
	TEneBoss02* mOwner; // _08

	inline Boss02Brain()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt()               = 0; // _08 1 if it handled a hit
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~Boss02Brain() { }               // _10 0x8005F914
	virtual void update(TEneBoss02* owner);         // _14 0x8005DF9C

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** No fields of its own */
struct Boss02BrainBase : public Boss02Brain {
	Boss02BrainBase();
	virtual ~Boss02BrainBase(); // 0x8005E064
};

/**
 * The boss's brain. It runs a plan of states (sPlan) round after round: shoot at the player, open gates,
 * spit and slam, dive through gates, and rise again in the middle. A heavy hit cuts the plan short.
 *
 * @size 0x110
 */
struct Boss02FightBrain : public Boss02BrainBase {
	enum State {
		STATE_START  = 0,
		STATE_SHOOT  = 1,
		STATE_GATES  = 2,
		STATE_SPIT   = 3,
		STATE_STUN   = 4,
		STATE_SINK   = 5,
		STATE_SLAM   = 6,
		STATE_DIVE   = 7,
		STATE_HURT   = 8,
		STATE_RETURN = 9,
		STATE_DIE    = 10,
	};

	Boss02FightBrain(TEneBoss02* owner);
	virtual ~Boss02FightBrain();

	static Boss02FightBrain* create(TEneBoss02* owner); // 0x8005E0C0

	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void nextState();
	int look();
	void aimAtPlayer(f32 margin);
	int getTier();
	void ringPoint(Vec3f* out, int step);
	void goHome();
	void setupShots(int* count, int* gap);
	int pickGates(int count);
	void aimGate(int n);
	int gateCount();
	void goToGate();
	int diveCount();
	void setMoveRate();
	int slamCount();
	int slamDamage();
	int diveWait();
	void glowOn();
	void glowOff();
	void endStun();
	void beginStun();

	void phaseStart(int phase);
	void phaseShoot(int phase);
	void phaseGates(int phase);
	void phaseSpit(int phase);
	void phaseStun(int phase);
	void phaseSink(int phase);
	void phaseSlam(int phase);
	void phaseDive(int phase);
	void phaseHurt(int phase);
	void phaseReturn(int phase);
	void phaseDie(int phase);

	EnemySight mSight; // _10
	int mPlanStep;     // _40 into sPlan
	int mRound;        // _44 turns the ring points a quarter each round
	int mSlots[10];    // _48 the gate slots pickGates took
	HitVec mGates[10]; // _70 where those gates are
	int mDives;        // _E8
	int mSlams;        // _EC
	u32 mFlags;        // _F0 1 = a heavy hit landed
	int mHealthMark;   // _F4 the health when the slams began
	TObject* mFx;      // _F8
	int mStep;         // _FC
	int mTimer;        // _100
	int mGap;          // _104
	int mShots;        // _108
	int _10C;          // _10C
};

/**
 * The blue boss, class 0x0200. A water serpent: it swims under the pool, spits water shots, opens gates on a
 * ring around the middle, and dives in and out through them.
 *
 * @size 0x3F8
 */
class TEneBoss02 : public TObjEnemy {
public:
	inline void operator delete(void* mem);

	enum {
		// mBossFlags
		BOSS_GLOW      = 0x1,   // the glow is up
		BOSS_FACING    = 0x2,   // the turn reached the aim
		BOSS_CLOSE     = 0x4,   // the aim is under 100 away
		BOSS_TILTED    = 0x8,   // diving nose down
		BOSS_BITE      = 0x10,  // the attack collider bites
		BOSS_SLAM      = 0x20,  // the attack collider slams
		BOSS_SPLASHED  = 0x40,  // the splash of this dive is done
		BOSS_OPEN      = 0x80,  // hits land hard
		BOSS_GUARDED   = 0x100, // hits land soft
		BOSS_SHOWN     = 0x200, // drawn wherever the camera is
		BOSS_SUIT      = 0x400, // the death suit is drawn
		BOSS_DEMO_POSE = 0x800, // the body follows the motion's root
	};

	enum Action {
		ACT_APPEAR     = 0,
		ACT_RISE       = 1,
		ACT_TURN       = 2,
		ACT_SHOOT      = 3,
		ACT_WAIT       = 4,
		ACT_SHOOT_GATE = 5,
		ACT_ROAR       = 6,
		ACT_STUN       = 7,
		ACT_SPIT       = 8,
		ACT_DIVE       = 9,
		ACT_DIVE_DEEP  = 10,
		ACT_LEAP       = 11,
		ACT_SWIM       = 12,
		ACT_SURFACE    = 13,
		ACT_GATE_RISE  = 14,
		ACT_DIE        = 15,
	};

	TEneBoss02(TObject* parent, SetDesc* desc);
	virtual ~TEneBoss02();

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
	virtual int getHitKind(const void* touch, HitInfo* info);           // _A8

	void init(SetDesc* desc);
	void makeColliders();
	int walkBody(NJS_OBJECT* obj);
	void measure();
	void measureBody();
	void measureTail();
	void playDemo(int model, int motion);

	void setTarget(const Vec3f* target);
	void setAim(const Vec3f* aim);
	int getMouth(Vec3f* out);
	int finishMotion();
	void setHealth(int health);
	int isAppearDone();
	int isRiseDone();
	int isFacing();
	int isClose();
	int isShootDone();
	void setAimSlot(Vec3f* aim, int slot);
	int isShootGateDone();
	int isRoarDone();
	int isSpitDone();
	int isDiveDone();
	int isDiveDeepDone();
	int isLeapDone();
	int isSurfaceDone();
	int isGateRiseDone();
	void onLightHit();
	void onHeavyHit();
	void setTurnRate(f32 rate);
	void setMoveRate(f32 rate);

	void createBrain();
	void beginPartPick();
	void endPartPick();
	void placeParts();
	void updatePose();
	void poseTail();
	int walkTail(NJS_OBJECT* obj, int* index);

	void armBite();
	void armSlam();
	void disarm();
	void setShape(const void* shapes);
	void floatHigh();
	void floatAt();
	void sink();
	void setLayer(int layer);
	void layerGlow();
	void layerNormal();
	void layerSuit();
	void glowOff();
	void glowOn();
	void keepGlowFx();
	void glowFlash();
	void glowFade();
	void updateGlow();
	void startMove(f32 time, const Vec3f* from, const Vec3f* to);
	f32 swim(f32 time, f32 rate, f32 phase);
	void moveStep(f32 time);
	void moveEase(f32 time, f32 end, f32 rate);
	void startTurn(f32 x, f32 z, f32 time);
	void turnStep(f32 time);
	void setOpen();
	void setNeutral();
	void setGuarded();
	void onShoot();
	void shotFx();
	void endSplash();
	void splash();
	void splashSoft();
	void keepTrail();
	void dropTrail();
	void ripple(f32 dist, u32 flag);
	void rippleBite();
	void rippleSlam();

	void actAppear(int phase);
	void actRise(int phase);
	void actTurn(int phase);
	void actShoot(int phase);
	void actWait(int phase);
	void actShootGate(int phase);
	void actRoar(int phase);
	void actStun(int phase);
	void actSpit(int phase);
	void planDive(f32 height, int motion, f32* time, f32* travel, f32* end, f32* tilt, Boss02Key* keys);
	void actDive(int phase, int action);
	void planLeap(f32* time, f32* travel, f32* top, f32* fall, f32* land, Boss02Key* keys);
	void actLeap(int phase);
	void actSwim(int phase);
	void actSurface(int phase);
	void planGateRise(f32* time, f32* travel, Boss02Key* keys);
	void actGateRise(int phase);
	void actDie(int phase);

	void bodyMatrix();
	void bodyTilt();
	void beginGlow();
	void endGlowDraw();
	void drawGlow();
	void endGlow();
	void beginSuit();
	void suitLift();
	void drawSuit();
	void endSuit();

	EnemyMotion mBossMotion;         // _158
	Boss02FightBrain* mBrain;        // _194
	GjActor* mPose;                  // _198
	DrawTask* mDrawTask;             // _19C
	int mLayer;                      // _1A0 the draw task's layer
	CharaColli* mAttack;             // _1A4 the bite and slam collider
	u32 mBossFlags;                  // _1A8
	Vec3f mKnock;                    // _1AC which way the last hit pushed
	int mMaxHealth;                  // _1B8
	TObject* _1BC;                   // _1BC
	Vec3f mTarget;                   // _1C0 where to go
	Vec3f mAim;                      // _1CC where to shoot or land
	int mAimSlot;                    // _1D8 the gate slot at mAim
	f32 mBaseY;                      // _1DC
	HitVec mParts[6];                // _1E0 posed from the bones
	Vec3f mNeck;                     // _228 the head from the body at frame 4
	Vec3f mNeckDir;                  // _234
	Vec3f mTailStep;                 // _240
	Vec3f mTipLocal;                 // _24C
	Vec3f mTip;                      // _258
	BossTail* mTail;                 // _264
	void (*mTransform)(NJS_OBJECT*); // _268 the node transform poseTail walks with
	void (*mNextNode)();             // _26C and the skip
	ObjShadow mShadow;               // _270
	f32 mTurnRate;                   // _2A8
	f32 mMoveRate;                   // _2AC
	int mGlowState;                  // _2B0 0 held 1 rising 2 off 3 falling
	f32 mGlow;                       // _2B4
	f32 mGlowWave;                   // _2B8
	int mGlowAngle;                  // _2BC
	f32 mSwimPhase;                  // _2C0
	int mTurnStep;                   // _2C4
	int mTurnFrom;                   // _2C8
	Vec3f mMoveFrom;                 // _2CC
	BossSuit mSuit;                  // _2D8
	EnemyTint mTint;                 // _310
	TObject* mTrail;                 // _318
	TParticleGenerator* mGlowFx;     // _31C
	int mView;                       // _320 0 above water 1 below
	int mViewShown;                  // _324
	TCamPath* mDemo;                 // _328
	SeCue mCue;                      // _32C
	Boss02Key mKeys[20];             // _330
	int mStep;                       // _3D0
	int mCount;                      // _3D4
	int mTurnSpeed;                  // _3D8
	u8 _3DC[4];                      // _3DC
	f32 mTime;                       // _3E0
	f32 mTime1;                      // _3E4
	f32 mTime2;                      // _3E8
	f32 mTime3;                      // _3EC
	f32 mTime4;                      // _3F0
	f32 mTime5;                      // _3F4
};

extern "C" {
void initBoss02Tail(); // 0x8005AE28
void freeBoss02Tail(); // 0x8005AEC8
}

/**
 * The water surface a pool or gate draws, in TEneBoss02_PoolPow.cpp. The base holds where it is and how big. The
 * names are ours.
 *
 * @size 0x18
 */
struct BossRippleBase {
	BossRippleBase(); // 0x800796B8

	Vec3f mPos;  // _00
	f32 mRadius; // _0C
	u32 mFlags;  // _10 1 = the grid needs placing again

	// _14 = vtable
	virtual ~BossRippleBase();                                                     // 0x800796E4
	virtual void update()                                                     = 0; // _0C
	virtual void ripple(f32 inner, f32 outer, f32 strength, const Vec3f* pos) = 0; // _10
	virtual void draw(f32 alpha)                                              = 0; // _14
};

/**
 * A 10 by 10 grid of points over the disc. A ripple pushes the points in a ring outwards, and each point springs back
 * to its rest place. Drawn as nine triangle strips, textured by the rest places.
 *
 * @size 0xC98
 */
struct BossRipple : public BossRippleBase {
	BossRipple(); // 0x800797D8

	virtual ~BossRipple();                                                     // 0x800798D8
	virtual void update();                                                     // 0x80079D1C
	virtual void ripple(f32 inner, f32 outer, f32 strength, const Vec3f* pos); // 0x80079930
	virtual void draw(f32 alpha);                                              // 0x80079D7C

	void flush(); // 0x80079AC8
	int place();  // 0x80079B08 1 if it moved the grid
	int relax();  // 0x80079C20 1 while any point is still moving

	HitVec mVerts[100]; // _18 where each point is drawn
	f32 mRest[100][2];  // _4C8 x and z where each point rests
	f32 mVel[100][2];   // _7E8
	int mMoving[100];   // _B08
};

extern "C" {
void bossRippleInit();                                            // 0x80079534 the grid's unit directions
void bossRippleExit();                                            // 0x80079624
BossRipple* createBossRipple();                                   // 0x80079660
BossRipple* newBossRipple();                                      // 0x80079680
void bossRippleSetPos(BossRippleBase* surface, const Vec3f* pos); // 0x8007972C
void bossRippleSetRadius(BossRippleBase* surface, f32 radius);    // 0x800797C4
}

// TEneBoss02's body helpers shared with the yellow boss's blue crystal
void bodyBase(const Vec3f* pos, const HitVec* parts, Vec3f* out);                     // 0x8005A178
void placeBody(void* colli, Vec3f* pos);                                              // 0x8005A19C
void fitShadow(f32 waterY, const Vec3f* pos, const HitVec* parts, ObjShadow* shadow); // 0x8005A2A4
int turnToward(f32 frame, f32 rate, int target, int* angle, int* step);               // 0x8005C14C
int atShotFrame(void* anim);                                                          // 0x8005C4E0
void tiltMatrix(int angle);                                                           // 0x8005DC34

extern "C" {
void* createBoss02Body(int);
void boss02CreatePoolWatcher();
void initTEneBoss02();
const void* paramsTEneBoss02();
NJS_TEXLIST** shapeTEneBoss02();
void shutdownTEneBoss02();
}

extern "C" {
TObject* createTEneBoss02(TObject*, SetDesc*);
}

#include "engine/MotionPlayer.h"
#include "io/ResBank.h"
extern "C" {
extern ResBank sBoss02Bank;
extern const int sBoss02BodyNodes[];
extern FxBankFile sBlueBossFx;
extern NJS_OBJECT** sBoss02Models;
extern NJS_TEXLIST** sBoss02Textures;
extern NJS_MOTION** sBoss02Motion;
extern Boss02Params* gBoss02Params;
extern MotionEntry sBoss02Motions[];
extern const EnemyFrameKey* sBoss02FrameKeys[];
}

#endif
