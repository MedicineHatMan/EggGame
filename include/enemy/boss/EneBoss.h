#ifndef _ENEBOSS_H
#define _ENEBOSS_H
struct NJS_OBJECT;
struct ColliShape;
struct CharaColli;

#include "egg/EggCreature.h"
struct DrawTask;
#include "object/ObjCommon.h"
#include "stage/SetManager.h"
#include "engine/MotionPlayer.h"
#include "enemy/ObjEnemy.h"
#include "engine/FxHolder.h"
#include "sound/SeHolder.h"
#include "egg/ObjEgg.h"
#include "enemy/EnemySight.h"

// What the bosses and their parts share

// An effect object of which we need only the delete flag
struct EnemyFxBase {
	u8 _00[4];
	u16 mFlags; // _04 1 = free me
};

// The parent every TObject keeps at _10
inline TObject* objParent(TObject* obj)
{
	return obj->mParent;
}

// The task addDrawTask returns Bit 0 of the halfword at _14 hides it
inline void setTaskHidden(void* task, int hidden)
{
	if (hidden) {
		*(u16*)((u8*)task + 0x14) |= 1;
	} else {
		*(u16*)((u8*)task + 0x14) &= ~1;
	}
}

// TEneLastBossFeather Size 0x1F4
class TEneLastBossFeather : public TObjAction {
public:
	TEneLastBossFeather(TObject* parent);
	virtual ~TEneLastBossFeather();

	void* operator new(unsigned long size) { return objHeapAlloc(gObjHeap, size); }
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual Vec3f* getPos();                                              // _20 0x8015E9B0
	virtual int getKind();                                                // _2C 0x8015E9A8
	virtual int onHit(const TObjAction* other, int arg, int* result) = 0; // _58
	virtual void setAim(const Vec3f* pos, int rotY);                      // _6C 0x8015DA1C

	void orient(int rotY, const f32* mtx);                                         // 0x8015D608 points the feather up the boss's node
	void updateColli();                                                            // 0x8015D774
	void updateShadow();                                                           // 0x8015D7AC
	void trackNodes();                                                             // 0x8015D810
	void calcPoints();                                                             // 0x8015D860
	void emitTrail();                                                              // 0x8015D8C0
	void stopTrail();                                                              // 0x8015D96C
	void applyMatrix();                                                            // 0x8015D9D4
	void approachAim();                                                            // 0x8015DA3C
	void turnToAim();                                                              // 0x8015DAB8
	f32 steer(f32 ahead, f32 behind, f32 side, f32 maxSpeed, const Vec3f* target); // 0x8015DB1C
	static void dirToRot(const Vec3f* dir, SetRot* rot);                           // 0x8015DD4C

	SetTransform mXform;  // _30
	MotionPlayer mMotion; // _74
	EnemyAnim mClip;      // _B0
	CharaColli* mShape;   // _BC
	DrawTask* mDraw;      // _C0
	void* mAnim;          // _C4
	ObjShadow mShadow;    // _C8
	Vec3f mAim;           // _100 where it hovers
	int mAimRotY;         // _10C
	int mTrailKind;       // _110 the particle the trail drops
	int mTrailPhase;      // _114 which half of the points drops this frame
	int mGlowKind;        // _118
	HitVec mPoint[8];     // _11C
	TObject* mPart[4];    // _17C
	FxHolder mGlow;       // _18C
	SeHolder mRing;       // _1C0
};

/**
 * A dark feather. It hovers by the boss until the boss throws it, then it homes on a point and leaves a dark map
 * and a magic there. Vtable 0x80261290.
 *
 * @size 0x20C
 */
class TEneLastBossFeather_Dark : public TEneLastBossFeather {
public:
	TEneLastBossFeather_Dark(TObject* parent, const Vec3f* pos, int rotY, const f32* mtx); // 0x8015DE0C
	virtual ~TEneLastBossFeather_Dark();                                                   // 0x8015DEF4

	virtual void draw();                                              // _0C 0x8015E00C
	virtual void vfn_10();                                            // _10 0x8015E180
	virtual void render();                                            // _1C 0x8015E1E0
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8015DF50

	void launch(const Vec3f* dir, const Vec3f* target, f32 radius); // 0x8015DF58

	int mState; // _1F4 0 hovering 1 thrown
	int _1F8;
	Vec3f mTarget;  // _1FC
	f32 mMapRadius; // _208
};

enum FeatherHomeState {
	HOME_HOVER = 0, // on the boss's wing following setAim
	HOME_CHASE = 1, // thrown homes on the nearest player until its time runs out or it touches something
	HOME_BURST = 2, // a short pause after it bursts
	HOME_GROW  = 3, // the blast grows to a radius of 35
	HOME_FADE  = 4, // the blast is gone the effect plays out
};

/**
 * A homing feather. It hovers on the boss's wing until the boss throws it, then chases the nearest player and
 * bursts when it touches something or its time runs out. Vtable 0x80261220.
 *
 * @size 0x264
 */
class TEneLastBossFeather_Home : public TEneLastBossFeather {
public:
	TEneLastBossFeather_Home(TObject* parent, const Vec3f* pos, int rotY, const f32* mtx); // 0x8015E228
	virtual ~TEneLastBossFeather_Home();                                                   // 0x8015E328

	virtual void draw();                                              // _0C 0x8015E5F4
	virtual void vfn_10();                                            // _10 0x8015E8D8
	virtual void render();                                            // _1C 0x8015E954
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8015E420

	void launch(const Vec3f* dir); // 0x8015E3A8
	void move();                   // 0x8015E504 bounces off a floor at y 23
	void slowAndMove();            // 0x8015E55C
	void placeBlast();             // 0x8015E59C
	int isBursting();              // 0x8015E5D0 past HOME_CHASE

	int mState;        // _1F4 a FeatherHomeState
	int mTimer;        // _1F8
	f32 mRadius;       // _1FC the shape's while bursting
	FxAttach mBlast;   // _200
	EnemySight mSight; // _234 mHeightLimit is the range
};

f32 lastBossMagicGrowFrames(); // 0x8015EA14 35

#endif
