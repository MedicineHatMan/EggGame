#ifndef MIRRORRIPPLE_H
#define MIRRORRIPPLE_H

#include "core/types.h"
#include "Ninja/ninja.h"

struct RippleVert { // size 0x28
	void init(f32 x, f32 y);

	f32 mX;      // _00 where it sits
	f32 mY;      // _04
	f32 mU;      // _08 where it draws the screen from
	f32 mV;      // _0C
	f32 mDirX;   // _10 mU mV - mX mY made unit
	f32 mDirY;   // _14
	f32 mVelX;   // _18
	f32 mVelY;   // _1C
	f32 mDist;   // _20 how far mU mV is off
	int mMoving; // _24
};

/** A ring that grows out from where the glass was touched and pushes the points it passes */
struct RippleSource { // size 0x1C
	void clear();
	void start(const NJS_POINT2* pos, f32 speed, f32 maxRadius);
	int isIdle();
	void step();

	NJS_POINT2 mPos; // _00
	f32 mRadius;     // _08
	f32 mPrevRadius; // _0C the radius last frame
	f32 mMaxRadius;  // _10
	f32 mSpeed;      // _14
	int mActive;     // _18
};

/** The glass of a mirror that ripples when something touches it */
struct MirrorRipple { // size 0x2858
	MirrorRipple();

	void buildGrid();
	void reset();
	f32 farthestSq(f32 x, f32 y, int vert, f32 best);
	void touch(f32 x, f32 y);
	void pushRing(const RippleSource* src, f32 outer, f32 inner, f32 strength);
	int isCalm();
	int spread();
	int settle();
	void update();

	RippleSource mSources[3];  // _00
	int mBusy;                 // _54
	RippleVert mVerts[16][16]; // _58 row 0 at the top
};

#endif
