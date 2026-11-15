#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

// 0x802169C8
f32 njDistanceP2P(const NJS_POINT3* a, const NJS_POINT3* b)
{
	f32 d = (a->z - b->z) * (a->z - b->z) + ((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y));
	if (d < 0.025f) {
		return 0.0f;
	}
	return sqrtf(d);
}
