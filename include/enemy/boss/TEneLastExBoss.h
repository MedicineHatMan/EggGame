#ifndef _ENEMY_BOSS_TENELASTEXBOSS_H_
#define _ENEMY_BOSS_TENELASTEXBOSS_H_
#include "Ninja/ninja.h"
#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemyLight.h"
#include "enemy/EnemySight.h"
#include "enemy/ObjEnemy.h"
#include "enemy/TEneAm01Parts.h"
#include "enemy/boss/BossParts.h"
#include "enemy/boss/TEneLastExBossCore.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "engine/FxHolder.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "sound/SeHolder.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct LastExBossAiBase;
class TParticleGenerator;
struct LastExBossAiState;
struct SetDesc;
struct TObject;
struct LastExBossAi;
struct TEneLastExBoss;

#include "core/types.h"

struct Vec3f;

extern "C" {
void hideLastExBoss();
void initTEneLastExBoss();
int lastExAngleFromCenter(const Vec3f*);
int lastExBossIsIdle();
f32 lastExDistFromCenter(const Vec3f*);
void lastExOnFloor(const Vec3f*, Vec3f*);
void lastExOutward(const Vec3f*, Vec3f*);
void lastExRandomPoint(f32, Vec3f*);
NJS_OBJECT* modelTEneLastExBoss(int);
NJS_MOTION** motionTEneLastExBoss(int);
void resetLastExBoss();
void showLastExBoss();
void shutdownTEneLastExBoss();
NJS_TEXLIST* textureTEneLastExBoss();
}

extern "C" {
LastExBossAi* createLastExBossAi(TEneLastExBoss*);
}

extern "C" {
TObject* createTEneLastExBoss(TObject*, SetDesc*);
}

/** Runs the current state's hooks */
class LastExBossAiBase {
public:
	inline LastExBossAiBase()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	int mState;             // _00
	int mNext;              // _04 the action to start or -1
	TEneLastExBoss* mOwner; // _08

	virtual int checkInterrupt()               = 0; // _08
	virtual void onPhase(int state, int phase) = 0; // _0C 0 enter 1 run 2 check 3 exit
	inline virtual ~LastExBossAiBase() { }          // _10
	virtual void tick(TEneLastExBoss* owner);       // _14
};

class LastExBossAiState : public LastExBossAiBase {
public:
	LastExBossAiState();
	virtual ~LastExBossAiState();
};

/**
 * Picks the boss's actions.
 *
 * @size 0x5C
 */
class LastExBossAi : public LastExBossAiState {
public:
	LastExBossAi(TEneLastExBoss* owner);
	virtual ~LastExBossAi();
	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void track();
	void aimAtTarget();
	void rememberHealth();
	int lostEnough();
	int getPhase();
	int checkHardHit();
	void pickNextAttack();
	void startFog();
	void stopFog();
	void setFeathers();
	void moveToFlank();
	void placeAtStart();
	void moveToRandom();
	int getStunFrames();
	void requestReset();

	void stateIntro(int phase);
	void stateIdle(int phase);
	void stateRush(int phase);
	void stateBurst(int phase);
	void stateCross(int phase);
	void stateFeathers(int phase);
	void stateCore(int phase);
	void stateDie(int phase);

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}

	EnemySight mSight; // _10
	int mHealthMark;   // _40
	int mNextAttack;   // _44
	int mReset;        // _48
	int mStep;         // _4C
	int mCount;        // _50
	int _54;           // _54
	int _58;           // _58
};

class TEneLastExBoss : public TObjEnemy {
public:
	enum {
		EX_SUIT    = 0x1,
		EX_OPEN    = 0x2, // hits count
		EX_NOTOUCH = 0x4,
		EX_FADING  = 0x8,  // drawn with mAlpha
		EX_CORE    = 0x10, // the body is gone and the core runs alone
		EX_FOG     = 0x20,
		EX_ATTACK  = 0x40,
		EX_GLOW    = 0x80, // mGlow follows the camera
		EX_HIDDEN  = 0x100,
		EX_STOMPED = 0x20000,
	};

	enum { COLLI_BODY = 0, COLLI_CORE = 1, COLLI_RUSH = 2 };

	TEneLastExBoss(TObject* parent, SetDesc* desc);
	virtual ~TEneLastExBoss();

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

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	void setShown(int shown);
	int isIdle();
	void requestReset();
	void setupParts();
	void placeAt(const Vec3f* pos);
	void setHeight();
	void placeShape(f32 radius, void* shape, int point);
	void setMoveTo(const Vec3f* pos);
	void setMoveToHigh(const Vec3f* pos);
	void setAim(const Vec3f* pos, int rotY);
	void setMaxHealth(int health);
	void clearOpen();
	void setOpen();
	void setTouchable();
	void setUntouchable();
	void onHardHit();
	void onLightHit();
	int isDone0();
	int isDone1();
	int isDone2();
	int isDone3();
	int isDone4();
	int isDone5();
	int isDone7();
	int isDone8();
	void setFeatherCount(int count);
	int isDone9();
	int isDone10();
	int isDone11();
	int isDone12();
	int isDone13();
	void makeAi();
	void beginNodes();
	void endNodes();
	void updatePose();
	void updatePoseRooted();
	void updateVerts();
	void calcPoints();
	void bodyMatrix();
	int angleToMoveTo();
	void updateShadow();
	void damp(f32 rate);
	void dampAndMove(f32 rate);
	void moveCore();
	void aimVel();
	void accel();
	void steer();
	f32 moveToward();
	void riseTo(f32 height);
	void riseToward(f32 height);
	f32 coreToward(f32 height);
	void unroll();
	void blowVerts(f32 radius, f32 speed, const Vec3f* center);
	void moveVerts();
	void dropVerts();
	void gatherVerts();
	void stopFx();
	void startFx(int set);
	void bigShadow();
	void smallShadow();
	void playDemo(int model, int motion);
	void startFog();
	void stopFog();
	void stepSound(f32 frame);

	void actAppear(int phase);
	void actTurn(int phase);
	void actRush(int phase);
	void actApproach(int phase);
	void actBurstUp(int phase);
	void actBurstMid(int phase);
	void actBurstEnd(int phase);
	void actRise(int phase);
	void actCross(int phase);
	void actFeathers(int phase);
	void actBlowUp(int phase);
	void actCoreRun(int phase);
	void actCoreHit(int phase);
	void actRebuild(int phase);
	void actDie(int phase);

	EnemyMotion mMotionData;   // _158
	LastExBossAi* mAi;         // _194
	GjActor* mPose;            // _198
	DrawTask* mDrawTask;       // _19C
	void* mPose2;              // _1A0
	CharaColli* mHurt;         // _1A4
	int mVertCount;            // _1A8
	s16* mVertOrder;           // _1AC
	HitVec* mVertPos;          // _1B0
	HitVec* mVertVel;          // _1B4
	u8* mVertAge;              // _1B8 0 attached 20 gone
	int mFxCount;              // _1BC
	int mFxCursor;             // _1C0
	TParticleGenerator** mFx;  // _1C4
	int mFxSet;                // _1C8
	u32 mBossFlags;            // _1CC
	int mColliMode;            // _1D0
	Vec3f mHitDir;             // _1D4
	HitVec mPoint[18];         // _1E0
	Vec3f mMoveTo;             // _2B8
	Vec3f mAim;                // _2C4
	int mAimRot;               // _2D0
	Vec3f mAnchor;             // _2D4
	int mMaxHealth;            // _2E0
	ObjShadow mShadow;         // _2E4
	EnemyLight mLight;         // _31C
	EnemyTint mTint;           // _35C
	BossSuit mSuit;            // _364
	int mFeatherCount;         // _39C
	Vec3f mBlastAt;            // _3A0
	int mAlpha;                // _3AC
	TEneLastExBossCore* mCore; // _3B0
	Vec3f mCorePos;            // _3B4
	TCamPath* mDemo;           // _3C0
	SeCue mSe;                 // _3C4
	int mFogState;             // _3C8
	int mFogTimer;             // _3CC
	TObject* mExtra;           // _3D0
	FxHolder mGlow;            // _3D4
	int mStep;                 // _408
	int mCount;                // _40C
	int mTurnTo;               // _410
	int _414;                  // _414
	f32 mTime;                 // _418
};

#include "io/ResBank.h"
extern "C" {
extern ResBank sLastExBossBank;
extern NJS_OBJECT** sLastExBossModels;
extern NJS_TEXLIST** sLastExBossTextures;
extern NJS_MOTION** sLastExBossMotion;
extern int* sLastExParams;
extern FxBankFile sLastExBossFx;
extern TEneLastExBoss* gLastExBoss;
extern const int* sLastExFeatherSets[4];
extern const int sLastExFeatherPoints[];
}

#endif
