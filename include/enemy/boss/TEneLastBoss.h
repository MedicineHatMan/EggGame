#ifndef _ENEMY_BOSS_TENELASTBOSS_H_
#define _ENEMY_BOSS_TENELASTBOSS_H_
#include "Ninja/ninja.h"
#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemyLight.h"
#include "enemy/EnemySight.h"
#include "enemy/ObjEnemy.h"
#include "enemy/TEneAm01Parts.h"
#include "enemy/boss/BossParts.h"
#include "enemy/boss/EneBoss.h"
#include "engine/DrawTask.h"
#include "engine/FxHolder.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "sound/SeHolder.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct LastBossAiBase;
struct LastBossAiState;
struct SetDesc;
struct TObject;
struct LastBossAi;
struct TEneLastBoss;

#include "core/types.h"

struct DarkFlashSlot;

extern "C" {
void createLastBossDark();
void darkFlashesAge(DarkFlashSlot*);
int darkFlashesCheck(f32, DarkFlashSlot*, u16);
void darkFlashesReset(DarkFlashSlot*);
int featherDamageTEneLastBoss();
void initLastBossDark();
void initTEneLastBoss();
NJS_OBJECT* modelTEneLastBoss(int);
NJS_MOTION** motionTEneLastBoss(int);
void shutdownLastBossDark();
void shutdownTEneLastBoss();
NJS_TEXLIST* textureTEneLastBoss();
}

extern "C" {
LastBossAi* createLastBossAi(TEneLastBoss*);
}

extern "C" {
TObject* createTEneLastBoss(TObject*, SetDesc*);
}

/** Runs the current state's hooks */
class LastBossAiBase {
public:
	inline LastBossAiBase()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	int mState;           // _00
	int mNext;            // _04 the action to start or -1
	TEneLastBoss* mOwner; // _08

	virtual int checkInterrupt()               = 0; // _08
	virtual void onPhase(int state, int phase) = 0; // _0C 0 enter 1 run 2 check 3 exit
	inline virtual ~LastBossAiBase() { }            // _10
	virtual void tick(TEneLastBoss* owner);         // _14
};

class LastBossAiState : public LastBossAiBase {
public:
	LastBossAiState();
	virtual ~LastBossAiState();
};

/**
 * Picks the boss's actions.
 *
 * @size 0x60
 */
class LastBossAi : public LastBossAiState {
public:
	LastBossAi(TEneLastBoss* owner);
	virtual ~LastBossAi();
	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void track();
	void aimAtTarget();
	void rememberHealth();
	int lostEnough();
	int getPhase();
	int getDarkCount();
	int getCircleFrames();
	int getHomeCount(int shots);
	int getHomeShots();
	int getHomeWait();
	int getStunFrames();

	void stateIntro(int phase);
	void stateWalk(int phase);
	void stateCircle(int phase);
	void stateSpiral(int phase);
	void stateHome(int phase);
	int checkStunHits(int* timer);
	void stateStun(int phase);
	void stateDie(int phase);

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}

	EnemySight mSight; // _10
	int mHealthMark;   // _40
	int mToggle;       // _44
	int mDarks;        // _48
	int mStep;         // _4C
	int mCount;        // _50
	int mCount2;       // _54
	int mAngle;        // _58
	int mAngleStep;    // _5C
};

class TEneLastBoss : public TObjEnemy {
public:
	enum {
		BOSS_SUIT    = 0x1,
		BOSS_OPEN    = 0x2,  // hits count for more
		BOSS_GUARD   = 0x4,  // hits on the guard shape count less
		BOSS_NOTOUCH = 0x8,
		BOSS_ARRIVED = 0x10, // reached mMoveTo
		BOSS_LANDED  = 0x20,
		BOSS_SPARK   = 0x40,
	};

	TEneLastBoss(TObject* parent, SetDesc* desc);
	virtual ~TEneLastBoss();

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

	void setupParts();
	void removeDarks();
	void removeHomes();
	void setMoveTo(const Vec3f* pos);
	void setAim(const Vec3f* pos, int rotY);
	void setMaxHealth(int health);
	void clearOpen();
	void setOpen();
	void setGuard();
	void clearGuard();
	void setTouchable();
	void setUntouchable();
	void onHardHit();
	void onLightHit();
	int hasArrived();
	int isDone0();
	void setDarkCount(int count);
	int isDone3();
	int isDone4();
	int hasLanded();
	int isDone5();
	void setHomeCount(int count);
	int isDone6();
	int isDone7();
	int isDone8();
	int isDone9();
	int isDone11();
	int isDone12();
	void makeAi();
	void beginNodes();
	void endNodes();
	void updatePose();
	void updatePoseRooted();
	void calcPoints();
	void bodyMatrix();
	void updateShadow();
	int moveOrbit();
	int slowOrbit();
	int moveBy(const Vec3f* step);
	void placeOnWing(f32 t, Vec3f* out, int* rotY);
	f32 getFeatherStep(int count);
	void hideFeather(int index);
	void hideFeathers();
	void showFeathers();
	void playDemo(int model, int motion);
	void launchDark(int index, const Vec3f* pos);

	void actAppear(int phase);
	void actIdle(int phase);
	void actMove(int phase);
	void actDarks(int phase);
	void actBurst(int phase);
	void actSpin(int phase);
	void actHome(int phase);
	void actHomeEnd(int phase);
	void actStunA(int phase);
	void actStunB(int phase);
	void actDown(int phase);
	void actUp(int phase);
	void actFlinch(int phase);
	void actDie(int phase);

	EnemyMotion mMotionData; // _158
	LastBossAi* mAi;       // _194
	GjActor* mPose;           // _198
	DrawTask* mDrawTask;       // _19C
	void* mFeatherPose;    // _1A0
	u32 mBossFlags;        // _1A4
	Vec3f mHitDir;         // _1A8
	HitVec mPoint[16];      // _1B4
	f32 mFeatherMtx[7][12]; // _274
	Vec3f mMoveTo;         // _3C4
	Vec3f mAim;            // _3D0
	int mAimRot;           // _3DC
	Vec3f mFrom;           // _3E0
	int mMaxHealth;        // _3EC
	ObjShadow mShadow;     // _3F0
	EnemyLight mLight;     // _428
	EnemyTint mTint;       // _468
	BossSuit mSuit;        // _470
	f32 mRadiusVel;        // _4A8
	int mAngleVel;         // _4AC
	f32 mFeatherSize[7];   // _4B0
	int mDarkCount;        // _4CC
	TEneLastBossFeather_Dark* mDark[10];    // _4D0
	int mHomeCount;        // _4F8
	TEneLastBossFeather_Home* mHome[4]; // _4FC
	TObject* mFx;          // _50C
	FxHolder mGlow;        // _510
	FxHolder mGlow2;       // _544
	FxAttach mTail;        // _578
	TCamPath* mDemo;       // _5AC
	SeCue mSe;                // _5B0
	int mColorMode;        // _5B4
	TObject* mDarkness;    // _5B8
	int mStep;             // _5BC
	int mWait;             // _5C0
	int _5C4;              // _5C4
	int _5C8;              // _5C8
	f32 _5CC;              // _5CC
};

#include "io/ResBank.h"
extern "C" {
extern ResBank sLastBossResBank;
extern ResBank gLastBossDarkBank;
extern NJS_OBJECT** sLastBossModelList;
extern NJS_TEXLIST** sLastBossTextures;
extern NJS_MOTION** sLastBossMotion;
extern int* sLastBossParams;
extern FxBankFile sLastBossFx;
extern const int* sLastBossNodeList;
extern f32* sLastBossMtxOut;
void polarOfTEneLastBoss(const Vec3f* pos, f32* radius, int* angle); // 0x801566EC
void fromPolarTEneLastBoss(f32 radius, Vec3f* out, int angle);       // 0x80156784
void randomOnRingTEneLastBoss(Vec3f* out, f32 radius);               // 0x801567B8
}

#endif
