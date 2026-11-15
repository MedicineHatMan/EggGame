#ifndef _TOBJORANGEFIREWORKS_H
#define _TOBJORANGEFIREWORKS_H

#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A burst of firework particles. mWait is gFireworkFrames divided by the count, and the count is held
 * between 1 and 60. Each burst picks a particle from gFireworkParticles and flashes the landscape.
 *
 * @size 0x38
 */
class TObjOrangeFireworks : public TObject {
public:
	TObjOrangeFireworks(f32 spread, TObject* parent, const Vec3f* pos, u32 count);
	virtual ~TObjOrangeFireworks();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	Vec3f mPos;  // _20
	f32 mSpread; // _2C
	int mTimer;  // _30
	int mWait;   // _34
};

class TObject;
struct Vec3f;

extern "C" {
TObjOrangeFireworks* createTObjOrangeFireworks(f32, TObject*, Vec3f*, u32);
}

/** One colour of the fireworks set at start-up */
struct FireworkColor {
	FireworkColor(u32 rgba) { mRgba = rgba; }
	u32 mRgba;
};

extern "C" {
extern const char* gTObjOrangeFireworksBallName;
extern const char* gTObjOrangeFireworksName;
extern u32 sFireworkRed;
extern u32 sFireworkGreen;
extern u32 sFireworkBlue;
extern FireworkColor sFireworkColors[];
extern const f32 gFireworkFrames[1];
}

#endif
