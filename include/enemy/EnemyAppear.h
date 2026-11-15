#ifndef _ENEMYAPPEAR_H
#define _ENEMYAPPEAR_H

#include "stage/Stage.h"
struct DrawTask;

struct GjActor;

/**
 * The egg an enemy hatches from. It falls in (or is thrown along a velocity), bounces off walls and lands; the enemy
 * waits for isLanded, takes its position and then grows out of the egg shape with hatchPose.
 *
 * @size 0x54
 */
class TEnemyAppear : public TObject {
public:
	enum {
		STATE_WAIT = 0, // the drop-in delay with the sparkle particles
		STATE_FALL = 1,
		STATE_LAND = 2,
		STATE_DONE = 3,
	};

	TEnemyAppear(TObject* parent, Vec3f pos, int dropIn); // 0x8001BA6C
	virtual ~TEnemyAppear();                               // 0x8001BBB8

	int isLanded();          // 0x8001BC20
	void getPos(Vec3f* out); // 0x8001BC44
	void launch(const Vec3f* vel); // 0x8001BC60 flies along vel instead of dropping straight down

	virtual void draw();   // _0C 0x8001BD18 the update
	virtual void vfn_10(); // _10 0x8001BF64 shown only while falling or landing
	virtual void render(); // _1C 0x8001BF98

	s16 mTimer;      // _20
	s16 _22;         // _22
	Vec3f mPos;      // _24
	f32 mVelY;       // _30
	f32 mSpeed;      // _34 along mDir
	Vec3f mDir;      // _38 flat
	f32 mScale;      // _44
	DrawTask* mDrawTask; // _48 from addDrawTask
	u32 mMoveFlags;  // _4C bit 0 = moves along mDir
	int mState;      // _50
};

/**
 * Drops a run of egg feed one every six frames, a random kind for the whole run.
 *
 * @size 0x3C
 */
class TEnemyDropSpawner : public TObject {
public:
	TEnemyDropSpawner(TObject* parent, const Vec3f* pos, int count, int big); // 0x8001CC98
	virtual ~TEnemyDropSpawner();                                       // 0x8001CD48

	virtual void draw(); // _0C 0x8001CDA4

	int mTimer; // _20
	int mCount; // _24 drops left
	int mKind;  // _28
	int mBig;   // _2C
	Vec3f mPos; // _30
};

extern "C" {
// The hatch timer runs from 1 down to 0 while the enemy grows from the egg shape into its model
void hatchClear(f32* t);                           // 0x8001C35C
void hatchStart(f32* t);                           // 0x8001C368
int hatchActive(f32* t);                           // 0x8001C374
int hatchDone(f32* t);                             // 0x8001C38C
void hatchStep(f32* t);                            // 0x8001C3A8
void hatchPose(f32* t, GjActor* pose, f32 height); // 0x8001C3BC

void setGqrS16();                                                 // 0x8001C000 GQR6 = s16 with 8 fraction bits
void poseEggShape(GjActor* pose, f32 t, f32 height);              // 0x8001C010
void poseEggBlend(GjActor* pose, f32 t, f32 height, f32 split);   // 0x8001C174
void poseHatch(GjActor* pose, f32 t, f32 height);                 // 0x8001C324

void spawnDropRun(Vec3f* pos, int count);           // 0x8001CC1C
void spawnDropRunEx(Vec3f* pos, int count, int big); // 0x8001CC40
}

#endif
