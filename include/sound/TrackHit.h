#ifndef SOUND_TRACKHIT_H
#define SOUND_TRACKHIT_H

#include "stage/Stage.h"

struct TrackHit {
	Vec3f mPos; // _00
	f32 mDist;  // _0C starts at the reach
	int mFound; // _10
};

#endif
