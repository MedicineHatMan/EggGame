#ifndef _ENEMY_BOSS_TENEYELLOWBOSS_H_
#define _ENEMY_BOSS_TENEYELLOWBOSS_H_
struct EnemyFrameKey;
#include "Ninja/ninja.h"
#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemyLight.h"
#include "enemy/EnemySight.h"
#include "enemy/ObjEnemy.h"
#include "enemy/TEneAm01Parts.h"
#include "enemy/boss/BossParts.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "engine/FxHolder.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "sound/SeHolder.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct YellowBossAiBase;
struct YellowBossAiState;
struct SetDesc;
struct TObject;
struct TEneYellowBoss;
struct YellowBossAi;

#include "core/types.h"

struct Vec3f;

extern "C" {
void initTEneYellowBoss();
NJS_OBJECT* modelTEneYellowBoss(int);
NJS_MOTION** motionTEneYellowBoss(int);
const int* paramsTEneYellowBoss();
void randomPointTEneYellowBoss(Vec3f*, f32);
void shutdownTEneYellowBoss();
NJS_TEXLIST* texture2TEneYellowBoss();
NJS_TEXLIST* textureTEneYellowBoss();
}

extern "C" {
YellowBossAi* createYellowBossAi(TEneYellowBoss*);
}

extern "C" {
TObject* createTEneYellowBoss(TObject*, SetDesc*);
}

class TEneYellowBoss : public TObjEnemy {
public:
	enum {
		BOSS_SUIT   = 0x1, // draws with the suit's texture
		BOSS_OPEN   = 0x2, // hits count for more
		BOSS_COPY   = 0x4, // a copy No health bar and no shadow
		BOSS_SOLID  = 0x8, // its spheres hit the player
		BOSS_FADING = 0x10,
	};

	TEneYellowBoss(TObject* parent, SetDesc* desc);
	TEneYellowBoss(TObject* parent, YellowBossAi* ai);
	virtual ~TEneYellowBoss();

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
	void setMoveTo(const Vec3f* pos);
	void setMoveTo(const Vec3f* pos, int rotY);
	void setAim(const Vec3f* pos, int rotY);
	void setMaxHealth(int health);
	void clearOpen();
	void setOpen();
	void setCopy();
	void setSolid();
	void clearSolid();
	void onHardHit();
	void onLightHit();
	int findFreePart();
	void summonPart(int kind);
	int partsDone();
	void removeParts();
	int isDone0();
	int isDone1();
	int isDone3();
	int isPartsDone();
	int isDone4();
	int isDone6();
	int isDone7();
	void setTravelTime(f32 frames);
	int isDone9();
	int isDone11();
	int isDone12();
	int isDone14();
	int isDone16();
	int isDone18();
	int isDone19();
	void makeAi();
	void beginNodes();
	void endNodes();
	void updatePose();
	void updatePoseRooted();
	void calcPoints();
	void localMatrix();
	void bodyMatrix();
	void makeSolidMasks();
	void clearSolidMasks();
	void playDemo(int model, int motion);
	f32 riseTo(f32 height);
	void damp(f32 rate);
	static void addContacts(void* collider, Vec3f* out, int shape);
	int moveBy(const Vec3f* step);
	void moveDamped(f32 rate);
	int moveAlongPath(f32 time);
	void planDash();
	void puff();
	void burst();
	void stepSound(f32 frame);

	void actAppear(int phase);
	void actHover(int phase);
	void actBreath(int phase);
	void actSummon(int phase);
	void actStun(int phase);
	void actStagger(int phase);
	void actRise(int phase);
	void actLand(int phase);
	void actTurn(int phase);
	void actTeleport(int phase);
	void actFade(int phase);
	void actTurnStep(int phase);
	void actDash(int phase);
	void actDashTurn(int phase);
	void actSwipe(int phase);
	void actRecoil(int phase);
	void actTaunt(int phase);
	void actIdle(int phase);
	void beginDying();
	void endDying();
	void actDieA(int phase);
	void actDieB(int phase);
	void actDie(int phase);

	EnemyMotion mMotionData; // _158
	YellowBossAi* mAi;       // _194
	GjActor* mPose;          // _198
	DrawTask* mDrawTask;     // _19C
	u32 mBossFlags;          // _1A0
	Vec3f mHitDir;           // _1A4
	HitVec mPoint[5];        // _1B0
	Vec3f mMoveTo;           // _1EC
	int mMoveToRot;          // _1F8
	Vec3f mAim;              // _1FC
	int mAimRot;             // _208
	u16 _20C;                // _20C
	u8 _20E[2];
	Vec3f mFrom;        // _210
	int mMaxHealth;     // _21C
	ObjShadow mShadow;  // _220
	EnemyLight mLight;  // _258
	EnemyTint mTint;    // _298
	BossSuit mSuit;     // _2A0
	f32 mTravelTime;    // _2D8
	f32 mFade;          // _2DC
	int mSide;          // _2E0
	TCamPath* mFx;      // _2E4
	CharaColli* mShape; // _2E8
	SeCue mSe;          // _2EC
	TObject* mFx2;      // _2F0
	FxHolder mGlow;     // _2F4
	TObject* mPart[5];  // _328
	u8 _33C[0xC8];
	int mStep;  // _404
	int mBurst; // _408
	int mWait;  // _40C
	int _410;   // _410
	int _414;   // _414
	f32 mTime;  // _418
};

/** Runs the current state's hooks */
class YellowBossAiBase {
public:
	inline YellowBossAiBase()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	int mState;             // _00
	int mNext;              // _04 the action to start or -1
	TEneYellowBoss* mOwner; // _08

	virtual int checkInterrupt()               = 0; // _08
	virtual void onPhase(int state, int phase) = 0; // _0C 0 enter 1 run 2 check 3 exit
	inline virtual ~YellowBossAiBase() { }          // _10
	virtual void tick(TEneYellowBoss* owner);       // _14
};

class YellowBossAiState : public YellowBossAiBase {
public:
	YellowBossAiState();
	virtual ~YellowBossAiState();
};

/**
 * Picks the boss's actions. A copy of the boss has its own, with AI_COPY and mParent set.
 *
 * @size 0x98
 */
class YellowBossAi : public YellowBossAiState {
public:
	enum {
		AI_COPY    = 1, // this one runs a copy
		AI_VANISH  = 2, // the copy was told to go
		AI_WAITING = 4, // the copy is standing open to a hit
		AI_RELEASE = 8, // the copy should move
	};

	YellowBossAi(TEneYellowBoss* owner);
	virtual ~YellowBossAi();
	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void track();
	void aimAtTarget();
	int getPhase();
	void rememberHealth();
	int lostHealth(const int* table);
	int lostHealthA();
	int lostHealthB();
	void startTimer();
	int randomWait();
	int getCopyCount();
	void setDest(const Vec3f* dest, int wait, int end);
	void removeCopies();
	YellowBossAi* spawnCopy(const Vec3f* pos, int rotY, const Vec3f* dest, int wait, int end);
	void splitIntoCopies(int count);
	void vanishCopies();
	int copiesWaiting();
	void releaseCopies();
	void forgetCopy(YellowBossAi* copy);
	void becomeCopy(YellowBossAi* parent);
	void vanish();
	void forgetParent(YellowBossAi* parent);

	void stateIntro(int phase);
	void stateSummon(int phase);
	void stateOpen(int phase);
	void stateRecover(int phase);
	void stateSplit(int phase);
	void stateCopyAppear(int phase);
	void stateBarrage(int phase);
	void stateDash(int phase);
	void stateCopyWait(int phase);
	void stateTaunt(int phase);
	void stateDying(int phase);
	void stateCopyGone(int phase);

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}

	EnemySight mSight;      // _10
	u32 mFlags;             // _40
	int mHealthMark;        // _44
	int mNextPart;          // _48
	YellowBossAi* mCopy[8]; // _4C
	YellowBossAi* mParent;  // _6C
	Vec3f mDest;            // _70
	int _7C;                // _7C
	int mWaitFrames;        // _80
	int mEndFrames;         // _84
	int mTimer;             // _88
	int mStep;              // _8C
	int mCount;             // _90
	int _94;                // _94
};

#include "io/ResBank.h"
extern "C" {
extern ResBank sYellowBossBank;
extern NJS_OBJECT** sYellowBossModels;
extern NJS_TEXLIST** sYellowBossTextures;
extern NJS_MOTION** sYellowBossMotion;
extern int* sYellowBossParams;
extern FxBankFile sYellowBossFx;
extern const EnemyFrameKey* sYellowBossFrameKeys[];
extern const f32 sYellowAppearBursts[];
extern const f32 sYellowDeathBursts[];
}

#endif
