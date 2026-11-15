#ifndef _TENEAM06_H
#define _TENEAM06_H
struct SetDesc;
struct TObject;
struct DrawTask;
struct CharaColli;

#include "enemy/ObjEnemy.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "enemy/TEneAm01Parts.h"
#include "enemy/TEneAm02.h"
#include "ui/TEnemyLifeBar.h"
#include "enemy/EnemySight.h"

class TEneAm06;

/** One object the head has touched and where it was */
struct Am06Spot { // size 0x10
	u16 mId; // _00 -1 = empty
	u16 _02;
	Vec3f mPos; // _04

	Am06Spot();
};

/** Up to four of those */
struct Am06Seen { // size 0x44
	int mCount; // _00
	Am06Spot mSpot[4]; // _04

	Am06Seen();
	void reset();
	void add(s16 id, TObjAction* obj);
	int find(s16 id, Vec3f* out);
};

/** The state machine The three words come first so the vtable lands at _0C */
struct Am06Brain {
	enum State {
		STATE_APPEAR  = 0,
		STATE_GLOW    = 1,
		STATE_COIL    = 2,
		STATE_SWAY    = 3,
		STATE_REAR    = 4,
		STATE_STRIKE  = 5,
		STATE_BITE    = 6,
		STATE_STAGGER = 7,
		STATE_SPIT    = 8,
		STATE_FLINCH  = 9,
		STATE_FLIP    = 10,
		STATE_DIE     = 11,
		STATE_DOWN    = 12,
	};

	int mState;       // _00
	int mNext;        // _04 an action to ask the owner for
	TEneAm06* mOwner; // _08

	inline Am06Brain()
	{
		mOwner = 0;
		mState = 0;
		mNext = -1;
	}

	virtual int checkInterrupt() = 0;               // _08 1 if it changed state or a hit should skip the check
	virtual void onPhase(int state, int phase) = 0; // _0C
	virtual ~Am06Brain() { }                           // _10 0x80049DBC
	virtual void update(TEneAm06* owner);           // _14 0x800489BC

	inline void changeState(int state)
	{
		onPhase(mState, 3);
		mState = state;
		onPhase(mState, 0);
	}
};

/** No fields of its own */
struct Am06BrainBase : public Am06Brain {
	Am06BrainBase();
	virtual ~Am06BrainBase(); // 0x80048C28
};

/**
 * The snake's brain. Two sights and two contact lists. The state numbers are not the action numbers:
 * glow is state 1 but action 2, and the rear is state 4 but action 1.
 *
 * @size 0x110
 */
struct Am06SnakeBrain : public Am06BrainBase {
	Am06SnakeBrain(TEneAm06* owner);
	virtual ~Am06SnakeBrain(); // 0x80048E24

	static Am06SnakeBrain* create(TEneAm06* owner); // 0x80048C84

	virtual int checkInterrupt();
	virtual void onPhase(int state, int phase);

	int nearSight();
	int look();
	u16 pick(Am06Seen* fresh, Am06Seen* previous);

	void phaseAppear(int phase);
	void phaseGlow(int phase);
	void phaseCoil(int phase);
	void phaseSway(int phase);
	void phaseRear(int phase);
	void phaseStrike(int phase);
	void phaseBite(int phase);
	void phaseStagger(int phase);
	void phaseSpit(int phase);
	void phaseFlinch(int phase);
	void phaseFlip(int phase);
	void phaseDie(int phase);
	void phaseDown(int phase);

	EnemySight mNear;  // _10 anyone within 200
	EnemySight mLook;  // _40 the cone used to strike
	int mCooldown;     // _70 look() stays quiet while this runs
	Am06Seen mList[2]; // _74 this frame and the last
	int mSide;         // _FC which list is filled this frame
	u16 mTarget;       // _100 0xFFFF = nobody
	u16 _102;
	int mStun;         // _104 copied from the owner when a stagger ends
	int mAgain;        // _108 a finished flip during the spit asks for the spit again
	int _10C;
};

/**
 * The snake. Seven points along its body, mSeg, follow the pose. The head and the bite are extra shapes
 * switched on for the attacks. Its brain is Am06SnakeBrain, built at the end of init.
 *
 * @size 0x2E4
 */
class TEneAm06 : public TObjEnemy {
public:
	enum Action {
		ACT_APPEAR  = 0,
		ACT_REAR    = 1,
		ACT_GLOW    = 2,
		ACT_COIL    = 3,
		ACT_SWAY    = 4,
		ACT_BITE    = 5,
		ACT_STRIKE  = 6,
		ACT_STAGGER = 7,
		ACT_SPIT    = 8,
		ACT_FLINCH  = 9,
		ACT_FLIP    = 10,
		ACT_DIE     = 11,
		ACT_DOWN    = 12,
	};

	enum {
		// mFlags2
		AM_REARED  = 0x1,   // the rear motion has finished
		AM_COILED  = 0x2,
		AM_SWAY    = 0x4,
		AM_BITTEN  = 0x8,   // the bite motion has finished
		AM_STRUCK  = 0x10,
		AM_HURTING = 0x20,  // onHit deals damage and only on shape 3
		AM_BITE    = 0x40,  // the bite shape is live
		AM_HEAD    = 0x80,  // the head shape is live
		AM_GRABBED = 0x100,
		AM_GLOW    = 0x200, // an effect is attached and one body point is posed
		AM_AIMED   = 0x400,
	};

	TEneAm06(TObject* parent, SetDesc* desc);
	virtual ~TEneAm06();

	inline void* operator new(u32 size);
	inline void operator delete(void* mem);

	virtual void vfn_10();                     // _10
	virtual void render();                     // _1C
	virtual void readEnemyDesc(SetDesc* desc); // _38
	virtual void vfn_3C();                     // _3C
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

	int isReared();
	int isCoiled();
	int isStruck();
	int isBitten();
	int isAimed();
	void setAim(int angle);
	void* headShape();
	void setHold(int angle);
	void setGrabId(u16 id);
	int getGrab();
	int stunFrames();
	void grab();
	void release();
	int isAppearDone();
	int isStaggerDone();
	int isFlinchDone();
	int isFlipDone();
	int isDownDone();
	void noteDeath();
	void noteFlip();
	void noteCrush();
	void noteDie();
	int deathAction();
	void stopEffect();
	int pickVariant(SetDesc* desc);

	void createBrain();
	void applyMotion();
	void placePoints();
	int approachAngle(int from, int to, int rate);
	void easeTurn(int diff, int rate);
	void stepYaw(int step);
	void clearTurn();
	void sway();
	void track();
	void fall();
	void armHead();
	void disarmHead();
	void showHead();
	void hideHead();
	void armBite();
	void disarmBite();
	void showBite();
	void hideBite();
	void placeBody();
	void placeHead();
	void placeBite();

	void actAppear(int phase);
	void actRear(int phase);
	void actGlow(int phase);
	void actCoil(int phase);
	void actSway(int phase);
	void actBite(int phase);
	void actStrike(int phase);
	void actStagger(int phase);
	void actSpit(int phase);
	void actFlinch(int phase);
	void actFlip(int phase);
	void actDie(int phase);
	void actDown(int phase);

	int mVariant;            // _158 always 0
	Am06Brain* mBrain;       // _15C
	void* mPose;             // _160
	DrawTask* mModel;            // _164 the task
	CharaColli* mHead;             // _168
	CharaColli* mBite;             // _16C
	u32 mFlags2;             // _170
	Vec3f mTarget;           // _174
	HitVec mSeg[7];          // _180
	int mYaw;                // _1D4
	int mTurn;               // _1D8 added into mYaw then consumed by stepYaw
	int mTurnMode;           // _1DC
	int mAim;                // _1E0
	int mHold;               // _1E4
	int mHealthStep;         // _1E8
	TEnemyLifeBar* mBar;     // _1EC
	ObjShadow mShadow;       // _1F0
	u16 mGrabId;             // _228 0xFFFF = nobody
	u8 _22A[2];              // _22A
	int mGrab;               // _22C 0 none 1 accepted 2 refused
	int mStunFrames;         // _230
	EnemyFx* mFx;            // _234
	EnemyLight mLight;       // _238
	EnemyTint mTint;         // _278
	EnemyEffect mEffect;     // _280
	int mStep;               // _2D0
	int mWindup;             // _2D4
	int mBitten;             // _2D8
	int _2DC;                // _2DC
	f32 _2E0;                // _2E0
};

extern "C" {
void initTEneAm06();
void shutdownTEneAm06();
}

extern "C" {
TObject* createTEneAm06(TObject*, SetDesc*);
}

#include "engine/CharaColli.h"
#include "io/ResBank.h"
extern "C" {
extern ResBank sAm06Bank;
extern NJS_OBJECT** sAm06Models;
extern NJS_TEXLIST** sAm06Textures;
extern NJS_MOTION** sAm06Motion;
extern int sAm06SegAngle;
extern const ColliShape sAm06AttackShapes[];
}

#endif
