#ifndef _TENEANTSQUEEN_H
#define _TENEANTSQUEEN_H
struct EnemyFrameKey;
struct SetDesc;
struct TObject;
struct DrawTask;

#include "enemy/ObjEnemy.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemySight.h"
#include "enemy/EnemyLight.h"
#include "enemy/TEneAm01Parts.h"
#include "ui/TEnemyLifeBar.h"

class TEneAntsQueen;

/**
 * A node in a list of the ants a queen has called. The queen holds the head, and each ant's brain holds a link, kept in
 * order of the slot it takes around the queen. Every link knows the queen. The destructor unlinks it. The names are
 * ours.
 *
 * @size 0x14
 */
struct AntLink {
	AntLink();  // 0x800923E0
	~AntLink(); // 0x80092400

	void setOwner(TEneAntsQueen* owner);               // 0x80092468
	TEneAntsQueen* getOwner();                         // 0x80092470
	void clearOwners();                                // 0x80092478 from this link to the end
	void insert(AntLink* head, int slot, void* brain); // 0x80092494 this one goes in after head by slot
	int count();                                       // 0x800924E0 the links after this one

	AntLink* mPrev;        // _00
	AntLink* mNext;        // _04
	int mSlot;             // _08 -1 when unset
	void* mBrain;          // _0C the ant's brain
	TEneAntsQueen* mOwner; // _10
};

/** The state machine The three words come first so the vtable lands at _0C */
struct AntsQueenBrain {
	enum State {
		STATE_APPEAR = 0,
		STATE_WANDER = 1,
		STATE_SUMMON = 2,
		STATE_ATTACK = 3,
		STATE_STUN   = 5,
		STATE_FLIP   = 6,
		STATE_DIE    = 7,
	};

	int mState;            // _00
	int mNext;             // _04 an action to ask the owner for
	TEneAntsQueen* mOwner; // _08

	inline AntsQueenBrain()
	{
		mOwner = 0;
		mState = 0;
		mNext  = -1;
	}

	virtual int checkInterrupt()               = 0; // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~AntsQueenBrain() { }            // _10 0x8009511C
	virtual void update(TEneAntsQueen* owner);      // _14 0x80094110

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** No fields of its own */
struct AntsQueenBrainBase : public AntsQueenBrain {
	AntsQueenBrainBase();          // 0x800941A8
	virtual ~AntsQueenBrainBase(); // 0x800941DC
};

/**
 * The queen's brain, vtable 0x80258508. Walks about its home, calls up ants while there's room for them, and throws
 * bombs at a player it sees, backing off one that comes too close.
 *
 * @size 0x58
 */
struct AntsQueenHuntBrain : public AntsQueenBrainBase {
	AntsQueenHuntBrain(TEneAntsQueen* owner); // 0x80094280
	virtual ~AntsQueenHuntBrain();            // 0x8009433C

	static AntsQueenHuntBrain* create(TEneAntsQueen* owner); // 0x80094238

	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void pickTarget();
	void pickRetreat();
	int look();
	int canSummon();
	int tickSummon();
	void summonOrWander();
	void pickSlot();

	void phaseAppear(int phase);
	void phaseWander(int phase);
	void phaseSummon(int phase);
	void phaseAttack(int phase);
	void phaseStun(int phase);
	void phaseFlip(int phase);
	void phaseDie(int phase);

	EnemySight mSight;   // _10
	EnemyMemory mMemory; // _40
	int mSummonTimer;    // _4C frames until it may call ants again
	int mStep;           // _50 the timer in a wander
	int mTimer;          // _54 1 while resting in a wander
};

/**
 * The ant queen, class 0x0106. Calls up TEneAm01s as its ants, up to ten, each with a slot on a ring about it, and
 * throws TEneAntsBombs.
 *
 * @size 0x2E0
 */
class TEneAntsQueen : public TObjEnemy {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	enum Action {
		ACT_APPEAR  = 0,
		ACT_IDLE    = 1,
		ACT_WALK    = 2,
		ACT_RETREAT = 3, // walks away from mTarget
		ACT_SUMMON  = 4,
		ACT_THROW   = 5,
		ACT_STUN    = 6,
		ACT_FLIP    = 7,
		ACT_DIE     = 8,
	};

	enum {
		// mFlags2
		QUEEN_ARRIVED  = 0x1,
		QUEEN_THROWING = 0x4,
		QUEEN_SUMMONED = 0x8,
	};

	TEneAntsQueen(TObject* parent, SetDesc* desc);
	virtual ~TEneAntsQueen();

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
	virtual int getHitKind(const void* touch, HitInfo* info);           // _A8

	void init(SetDesc* desc);
	void setTarget(const Vec3f* target);
	void setAim(const Vec3f* pos, int angle);
	AntLink* getAnts();
	void hitEffects();
	int canSummon();
	int getMaxAnts();
	void alertAnts(u16 player);
	int isAppearDone();
	int hasArrived();
	int isSummonDone();
	int hasSummoned();
	void setSlot(int angle, int slot);
	int isThrowDone();
	int isStunDone();
	int isNotThrowing();
	int isFlipDone();
	void stunEffects();
	void dieEffects();
	int deathAction();
	void startEffect();
	void createBrain();
	void beginBones();
	void endBones();
	void updatePose();
	void updateParts();
	void applyTransform();
	void partMove(f32 dist, int part, Vec3f* out);
	int move(Vec3f* vel);
	void walk(f32 dist, int rate, int offset);
	void coast(f32 drag);
	void getMouth(Vec3f* pos, Vec3f* dir);
	void throwEffects(Vec3f* pos, Vec3f* dir);
	void getHome(Vec3f* out);
	void getSlotPos(int slot, Vec3f* out);

	void actAppear(int phase);
	void actIdle(int phase);
	void actWalk(int phase);
	void actRetreat(int phase);
	void actSummon(int phase);
	void actThrow(int phase);
	void actStun(int phase);
	void actDie(int phase);

	AntsQueenHuntBrain* mBrain; // _158
	void* mPose;                // _15C
	DrawTask* mModelTask;       // _160
	u32 mFlags2;                // _164
	Vec3f mKnock;               // _168 which way a hit pushes it
	HitVec mParts[7];           // _174 posed from the bones
	Vec3f mTarget;              // _1C8 where to walk
	Vec3f mAim;                 // _1D4 the player to throw at
	int mAimAngle;              // _1E0 also the angle of the slot an ant is called to
	int mMaxAnts;               // _1E4 10
	int mSlot;                  // _1E8 the slot the next ant takes
	TEnemyLifeBar* mBar;        // _1EC
	ObjShadow mShadow;          // _1F0
	EnemyLight mLight;          // _228
	EnemyTint mTint;            // _268
	AntLink mAnts;              // _270 the head
	EnemyEffect mEffect;        // _284
	int mStep;                  // _2D4
	int mTurnStep;              // _2D8
	f32 mAppearTimer2;          // _2DC
};

extern "C" {
void initTEneAntsQueen();
void shutdownTEneAntsQueen();
}

extern "C" {
TObject* createTEneAntsQueen(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern ResBank sAntsQueenBank;
extern NJS_OBJECT** sAntsQueenModels;
extern NJS_TEXLIST** sAntsQueenTextures;
extern NJS_MOTION** sAntsQueenMotion;
extern const char* gTEneAntsQueenName;
extern const EnemyFrameKey* sAntsQueenFrameKeys[];
}

#endif
