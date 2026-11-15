#ifndef _ENEMYSIGHT_H
#define _ENEMYSIGHT_H

#include "object/ObjAction.h"

struct EnemySightBase { // size 0x20
	u16 mPlayerId; // _00 0xFFFF = none
	Vec3f mTarget; // _04 the player's position
	Vec3f mDelta;  // _10 from the enemy to the player

	EnemySightBase();
	virtual ~EnemySightBase();                                 // _08
	virtual void reset();                                      // _0C
	virtual int findInRange(f32 rangeSq, Vec3f* pos);         // _10
	virtual int findInCone(f32 rangeSq, Vec3f* pos, int rotY, int angle); // _14
	virtual int search(f32 farSq, f32 nearSq, Vec3f* pos, int rotY, int farAngle, int nearAngle); // _18
	virtual int findInBox(Vec3f* pos, int rotY, f32 halfWidth, f32 depth); // _1C
	virtual int track(u16 player, Vec3f* pos, int rotY);      // _20

	int canSee(TObjAction* player);
};

/** Finds a player to go after */
struct EnemySight : public EnemySightBase { // size 0x30
	EnemySight();
	virtual ~EnemySight();
	virtual void reset();
	virtual int findInRange(f32 rangeSq, Vec3f* pos);
	virtual int findInCone(f32 rangeSq, Vec3f* pos, int rotY, int angle);
	virtual int search(f32 farSq, f32 nearSq, Vec3f* pos, int rotY, int farAngle, int nearAngle);
	virtual int findInBox(Vec3f* pos, int rotY, f32 halfWidth, f32 depth);
	virtual int track(u16 player, Vec3f* pos, int rotY);

	f32 mHeightLimit; // _20 players this far above or below are ignored
	f32 mDistSq;      // _24 flat
	int mAngle;       // _28 toward the player
	int mAngleDiff;   // _2C from where the enemy faces
};

/** Who hit the enemy and for how much longer it holds a grudge with a pause before it looks again */
struct EnemyMemory { // size 0xC
	int mCooldown; // _00 it doesn't look while this runs
	int mChase;    // _04 frames left of going after mPlayer
	u16 mPlayer;   // _08 0xFFFF = nobody

	EnemyMemory();
	void reset();
	void tick();
	int isReady();
	int isChasing();
	void startCooldown();
	void setChase(u16 player);
	void clearChase();
	u16 getPlayer();
};

#endif
