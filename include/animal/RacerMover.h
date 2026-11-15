#ifndef _RACERMOVER_H
#define _RACERMOVER_H

#include "core/types.h"
#include "stage/Stage.h"

struct Rot3i {
	int v[3];

	operator int*() { return v; }
	operator const int*() const { return v; }
};

/**
 * Steers a point toward mTarget. Kind 0 runs on the ground (it probes the mesh, and jumps a gap
 * or ledge with launchMover), kind 1 flies straight at the target and banks into turns.
 * The names are ours. TAnimalRacer keeps one at +0x3C.
 *
 * @size 0x60
 */
struct RacerMover {
	RacerMover();  // 0x80199740
	void update(); // 0x801997BC

	u8 mKind;      // _00 0 ground 1 air
	u8 _01[3];
	f32 mAccel;    // _04 the speed is at most mCap times the distance left
	f32 mTopSpeed; // _08
	f32 mF0C;      // _0C added to the speed each frame
	u32 mTurn;     // _10 0x400
	Vec3f mTarget; // _14
	Vec3f mPos;    // _20
	Vec3f mVel;    // _2C
	Vec3f mLaunch; // _38 the velocity to jump with when flag 0x10 is set
	Rot3i mRot;    // _44 [1] heading [2] bank
	f32 mGravity;  // _50 negative
	int mMinFrames; // _54 a jump takes at least this many frames 60
	int mMaxFrames; // _58 and at most this many 120
	u32 mFlags;    // _5C 1 landed 2 hold the rate 4 wall ahead 8 drop ahead 0x10 launch
};

inline void updateMover(RacerMover* mover)
{
	mover->update();
}

extern "C" {
extern const f32 gUnitVectorMin;
}

void walkMover(RacerMover* mover);
void flyMover(RacerMover* mover);
void launchMover(RacerMover* mover, Vec3f* out, const Vec3f* from, const Vec3f* to);

#endif
