#ifndef _TENEPENGUIN_H
#define _TENEPENGUIN_H
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
#include "sound/SoundHandle.h"
#include "enemy/TEneAm01Parts.h"
#include "ui/TEnemyLifeBar.h"

class TEnePenguin;

/** The state machine the same shape as BatBrain */
struct PenguinBrain {
	enum State {
		STATE_APPEAR = 0,
		STATE_WANDER = 1,
		STATE_SLIDE  = 2, // slides at the player
		STATE_REST   = 3, // gets up and waits
		STATE_STUN   = 4,
		STATE_DIE    = 5,
	};

	int mState;          // _00
	int mNext;           // _04 an action to ask the owner for
	TEnePenguin* mOwner; // _08

	inline PenguinBrain()
	{
		mOwner = 0;
		mState = 0;
		mNext = -1;
	}

	virtual int checkInterrupt() = 0;               // _08 1 if it changed state
	virtual void onPhase(int state, int phase) = 0; // _0C
	inline virtual ~PenguinBrain() { }           // _10 0x800ACD7C
	virtual void update(TEnePenguin* owner);        // _14 0x800AC2E0

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** No fields of its own */
struct PenguinBrainBase : public PenguinBrain {
	PenguinBrainBase();
	virtual ~PenguinBrainBase(); // 0x800AC3AC
};

/**
 * The penguin's brain. Waddles about its home, and when it sees a player, slides at them on its belly.
 *
 * @size 0x54
 */
struct PenguinHuntBrain : public PenguinBrainBase {
	PenguinHuntBrain(TEnePenguin* owner);
	virtual ~PenguinHuntBrain();

	static PenguinHuntBrain* create(TEnePenguin* owner); // 0x800AC408

	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	void pickTarget();
	int look();
	void aim();

	void phaseAppear(int phase);
	void phaseWander(int phase);
	void phaseSlide(int phase);
	void phaseRest(int phase);
	void phaseStun(int phase);
	void phaseDie(int phase);

	EnemySight mSight;   // _10
	EnemyMemory mMemory; // _40
	int mTimer;          // _4C
	int mResting;        // _50 1 once the step of the state is done
};

/** The penguin's tuning from enemyParamGet(9) */
struct PenguinParams {
	f32 mMaxSpeed;  // _00 of the slide
	f32 mSpinPush;  // _04 added to the push while turning one way
	f32 mSpinPull;  // _08 and the other
	f32 mSideDrag;  // _0C kept of the sideways speed
	int mSlideTime; // _10 frames of the slide
};

/**
 * The penguin, class 0x0107. It waddles about and slides at players on its belly. When the slide ends it spins to a
 * stop, gets up and rests.
 *
 * @size 0x314
 */
class TEnePenguin : public TObjEnemy {
public:
	enum Action {
		ACT_APPEAR    = 0,
		ACT_IDLE      = 1,
		ACT_WALK      = 2,
		ACT_SLIDE     = 3,
		ACT_SPIN      = 4, // the slide spins to a stop
		ACT_GETUP     = 5,
		ACT_REST      = 6,
		ACT_STUN      = 7,
		ACT_DIE       = 8,
	};

	enum {
		// mFlags2
		PEN_ARRIVED  = 0x1,
		PEN_ARMED    = 0x2,  // the slide's shapes are live and onHit hurts
		PEN_TILTED   = 0x4,  // on its belly applyTransform tilts it
		PEN_SLIDING  = 0x10, // a hit doesn't stun it
		PEN_SOUND    = 0x20, // the sliding sound is playing
		PEN_SLIDENOW = 0x40, // the sliding sound should play
	};

	enum {
		// mFxMode
		FX_NONE  = 0,
		FX_SLIDE = 1, // spray off the belly and both feet
		FX_DUST  = 2, // a puff over the head
	};

	TEnePenguin(TObject* parent, SetDesc* desc);
	virtual ~TEnePenguin();

	inline void* operator new(u32 size);
	inline void operator delete(void* mem);

	virtual void vfn_10();                     // _10 hides it off screen
	virtual void render();                     // _1C
	virtual void readEnemyDesc(SetDesc* desc); // _38
	virtual void vfn_3C();                     // _3C prints debug text
	virtual f32 vfn_40();                      // _40
	virtual int onHit(const TObjAction* other, int shape, int* result); // _58
	virtual void vfn_6C();                     // _6C
	virtual void updateBody();                 // _70
	virtual void setPos(const Vec3f* pos);           // _74
	virtual void syncCollider();               // _78
	virtual void onActionPhase(int action, int phase); // _7C
	virtual int getDamage(EnemyHit* hit);      // _A4
	virtual int getHitKind(const void* touch, HitInfo* info); // _A8

	void init(SetDesc* desc);
	void setTarget(const Vec3f* target);
	void setAim(const Vec3f* aim);
	void noteDeath();
	void stopEffect();
	int isAppearDone();
	int hasArrived();
	int isSlideDone();
	int canStun();
	int isRestDone();
	void noteStun();
	void noteDie();
	int deathAction();
	int isStunDone();
	void createBrain();
	void beginBones();
	void endBones();
	void updatePose();
	void placeParts();
	void applyTransform();
	void walkStep(f32 rate, int part, Vec3f* out);
	int move(Vec3f* vel);
	int slideMove(Vec3f* vel);
	void walk(f32 rate, void* arg);
	void drag(f32 rate);
	void arm();
	void disarm();
	void tilt(f32 speed);
	void updateSound();
	void startSlideSound();
	void stopSlideSound();
	void updateFx();
	void setFx(int mode);
	void killFx();

	void actAppear(int phase);
	void actIdle(int phase);
	void actWalk(int phase);
	void actSlide(int phase);
	void actSpin(int phase);
	void actGetUp(int phase);
	void actRest(int phase);
	void actStun(int phase);
	void actDie(int phase);

	PenguinHuntBrain* mBrain; // _158
	void* mPose;              // _15C
	DrawTask* mModel;             // _160 the task
	u32 mFlags2;              // _164
	Vec3f mKnock;             // _168 which way a hit pushes it
	HitVec mParts[7];         // _174 posed from the bones
	Vec3f mTarget;            // _1C8 where to walk
	Vec3f mAim;               // _1D4 the player to slide at
	Vec3f mDir;               // _1E0 which way the slide goes
	int mSpin;                // _1EC turn each frame
	TEnemyLifeBar* mBar;      // _1F0
	ObjShadow mShadow;        // _1F4
	EnemyLight mLight;        // _22C
	EnemyTint mTint;          // _26C
	SoundHandle mSound;       // _274
	EnemyEffect mEffect;      // _2A0
	int mFxMode;              // _2F0
	int mFxNext;              // _2F4 -1 = no change
	TObject* mFx[4];          // _2F8
	int mStep;                // _308
	int mAngle;               // _30C
	f32 mUnk310;              // _310
};

extern "C" {
void initTEnePenguin();
void shutdownTEnePenguin();
}

extern "C" {
TObject* createTEnePenguin(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern ResBank sPenguinBank;
extern NJS_OBJECT** sPenguinModels;
extern NJS_TEXLIST** sPenguinTextures;
extern NJS_MOTION** sPenguinMotion;
extern PenguinParams* gPenguinParams;
extern const char* gTEnePenguinName;
extern const EnemyFrameKey* sPenguinFrameKeys[];
}

#endif
