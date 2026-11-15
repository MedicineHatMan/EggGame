#ifndef _OBJENEMY_H
#define _OBJENEMY_H
struct CharaColli;

#include "object/ObjAction.h"
struct NJS_MOTION;
#include "engine/MotionPlayer.h"
#include "enemy/EnemyAnim.h"
#include "stage/SetManager.h"

class SetDesc;
class TEnemyAppear;

/**
 * The best hit found in a frame. The player's id is in mObjId or mHolderId, see TObjEnemy::getHitPlayer.
 *
 * @size 0x24
 */
struct EnemyHit {
	u16 mObjId;    // _00 who hit
	u16 mHolderId; // _02 who holds them
	int mShape;    // _04 the shape that was touched
	int mKind;     // _08 from getHitKind The highest wins
	HitInfo mInfo; // _0C
};

/**
 * Where a hit landed: the point of contact, the shape's centre, and the point on the shape's surface.
 *
 * @size 0x24
 */
struct EnemyContact {
	Vec3f mPos;     // _00
	Vec3f mCenter;  // _0C
	Vec3f mSurface; // _18
};

/** Up to four object ids touching this frame */
struct EnemyTouchList { // size 0xC
	int mCount;
	u16 mIds[4];
};

/** Up to four object ids that can't hit again until their timers run out */
struct EnemyCooldownList { // size 0x14
	int mCount;
	s16 mTimers[4];
	u16 mIds[4];
};

/** A row of the table setFrameKey reads the first row past the current frame gives the value */
struct EnemyFrameKey {
	f32 mTime;
	int mValue;
};

class SlopeFilter;

// Laid out like a SlopeFilter spinMove sets only its vtable
struct EnemyFilter {
	void* mVtable; // _00
	Vec3f mDir;    // _04
};

/**
 * The enemy base. The name is the one its setup stores.
 *
 * Slot _0C calls updateBody. Slots _5C to _68 all call onActionPhase with a phase of 0 enter, 2 exit, 3 and 1.
 *
 * @size 0x158
 */
class TObjEnemy : public TObjAction {
public:
	enum {
		// mHitFlags set by applyDamage from EnemyHit::mKind unless noted
		HIT_KIND3 = 0x1,
		HIT_DEAD  = 0x2, // health ran out
		HIT_KIND1 = 0x4,
		HIT_KIND2 = 0x8,
		HIT_KIND4 = 0x10,
		HIT_DONE  = 0x20, // the current action has finished

		// mEnemyFlags
		ENEMY_AIRBORNE   = 0x1,
		ENEMY_APPEARING  = 0x2,
		ENEMY_INTANGIBLE = 0x4,  // takeHits does nothing
		ENEMY_FLIPPING   = 0x8,
		ENEMY_BLOCKED    = 0x10, // canAct says no
		ENEMY_THROWN     = 0x20,
		ENEMY_LOCKED_ON  = 0x80, // holds the lock from lockOn
		ENEMY_KILLED     = 0x100,
		ENEMY_CANT_ACT   = 0x200,
		ENEMY_FROM_DESC  = 0x400, // from the description killed then ends the link for good
	};

	TObjEnemy(TObject* parent);
	TObjEnemy(TObject* parent, SetDesc* desc);
	virtual ~TObjEnemy();

	inline void operator delete(void* mem);

	virtual void draw();                       // _0C
	virtual Vec3f* getPos();                   // _20
	virtual int getRotY();                     // _28
	virtual int getKind();                     // _2C
	virtual void readEnemyDesc(SetDesc* desc); // _38
	virtual int canAct();                      // _44
	virtual void enterAction(int action);      // _5C
	virtual void exitAction(int action);       // _60
	virtual void vfn_64(int action);           // _64
	virtual void vfn_68(int action);           // _68
	virtual void vfn_6C() = 0;                 // _6C
	virtual void updateBody() = 0;             // _70 the per-frame work
	virtual void setPos(const Vec3f* pos);           // _74
	virtual void syncCollider();               // _78 moves the collider to the body
	virtual void onActionPhase(int action, int phase) = 0; // _7C
	virtual int isAirborne();                  // _80
	virtual int isAppearing();                 // _84
	virtual int isIntangible();                // _88
	virtual int isFlipping();                  // _8C
	virtual int isBlocked();                   // _90
	virtual int isThrown();                    // _94
	virtual int vfn_98();                      // _98 flag 0x40
	virtual int vfn_9C();                      // _9C not blocked
	virtual int vfn_A0();                      // _A0 not blocked
	virtual int getDamage(EnemyHit* hit);      // _A4
	virtual int getHitKind(const void* touch, HitInfo* info); // _A8
	virtual int showsHitEffect();              // _AC

	void setup();
	int returnZero();
	u16 getHitPlayer();
	void updateArea();

	// Rising into the stage driven one step at a time by the subclass's action
	void beginAppear(int* step);
	void runAppear(int* step);
	void runAppearAimed(int* step);
	int appearLanded();
	int tickAppear();
	void endAppear(int* step);

	// The pose buffer of a model and drawing with it
	void* allocPose(void* model);
	void freePose(void** pose);
	void applyPose(void* pose, void* anim);
	void applyPoseFaded(void* pose, f32 rate, void* anim, f32* timer);

	void setFrameKey(const EnemyFrameKey* table);
	void clearFrameKey();
	void useArea();

	void clearHitFlags();
	int isDone();
	int wasHitHard();
	int wasHitKind2();
	int wasHitKind3();
	int isDead();
	int wasHitKind1();
	int isActionDone(int action);
	void setDone();
	int waitMotion();
	int waitMotion(MotionPlayer* motion);

	void updateTouches();
	int takeHits(EnemyHit* hit, EnemyContact* contact);
	void applyDamage(EnemyHit* hit);
	void fillAttack(void* where, HitInfo* out);

	// Moving against the collider and the ground
	void turnTo(int angle, int rate);
	void faceToward(Vec3f* target, Vec3f* from, void* arg);
	void getFlatPush(Vec3f* out);
	void moveAlong(f32 dist, Vec3f* pos, Vec3f* dir, Vec3f* out);
	void getPush(Vec3f* out);
	void setAirborne(u32 result);
	int moveVertical(f32 dy, Vec3f* pos, void* out);
	int probeWall(f32 dist, Vec3f* from, Vec3f* to, void* a, void* b, void* c);
	int collide(f32 dist, Vec3f* from, Vec3f* to, void* out);
	int collideFiltered(f32 dist, Vec3f* from, Vec3f* to, void* out);
	int collide(f32 dist, Vec3f* from, Vec3f* to, void* filter, void* out);
	int probeLine(Vec3f* from, Vec3f* to, void* a, void* b);

	void resetLink();
	void killed(int timed);
	int isInView(f32 far, f32 radius, void* pos, void* arg);
	void creditKiller();
	int lockOn();
	void unlock();

	void slain();
	void setCantAct();
	void clearCantAct();
	void checkFellOut();
	void noOp();

	MotionPlayer mMotion;        // _30
	u32 mHitFlags;               // _6C
	u32 mEnemyFlags;             // _70
	SetTransform mBody;          // _74
	EnemyAnim mAnim;             // _B8
	CharaColli* mCollider;             // _C4
	Vec3f mHome;                 // _C8
	int mHealth;                 // _D4
	SetLinkMember mLink;         // _D8
	s16 mFrameKey;               // _E4
	s16 mArea;                   // _E6
	f32 mAppearTimer;            // _E8
	TEnemyAppear* mAppearObj;    // _EC built by runAppear
	EnemyHit mHit;               // _F0
	EnemyContact mContact;       // _114
	EnemyTouchList mTouching;    // _138
	EnemyCooldownList mCooldown; // _144
};

class TObjAction;
struct Vec3f;

extern "C" {
void __ct__6HitVecFv();
void beginLit(void*, void*);
int divideBy5(int);
void drawLit(void*, void*, void*);
void drawModelLit(void*);
void endLit();
void hideBossBar();
void initBossBar();
void initEnemyFilter(SlopeFilter*);
int isEggObj(const TObjAction*);
int isEnemyObj(const TObjAction*);
int isPlayerId(u16);
int isPlayerObj(const TObjAction*);
int isPlayerOrKind4(const TObjAction*);
void noOp__9TObjEnemyFv(int);
void pulseBossView(const Vec3f*, f32);
void resetBossView();
void sendBossDefeated(TObjEnemy*);
void setBossBar(int, int);
void setBossView(f32, f32, f32);
void spawnDrop(void*, int, int);
void spawnDropB(void*);
}

extern "C" {
int probeGroundLine(Vec3f*, Vec3f*, Vec3f*, Vec3f*);
}

extern "C" {
void spawnDropA(void*);
void drawFirstFrame(NJS_MOTION**, int, void*);
void poseFirstFrame(NJS_MOTION**, int, void*);
NJS_MOTION* firstMotionFrame(NJS_MOTION**, int, f32*);
}

extern "C" {
int probeGround(Vec3f*, Vec3f*, Vec3f*);
}

extern "C" {
extern const char* gTObjEnemyName;
}

#endif
