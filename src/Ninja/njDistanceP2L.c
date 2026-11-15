#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

typedef struct NJS_LINE {
	f32 px, py, pz;
	f32 vx, vy, vz;
} NJS_LINE;

// 0x802168A4
// Also gives the nearest point on the line if asked
f32 njDistanceP2L(const NJS_POINT3* p, const NJS_LINE* l, NJS_POINT3* out)
{
	f32 vy = l->vy;
	f32 vx = l->vx;
	f32 vz = l->vz;
	f32 px;
	f32 y;
	f32 py;
	f32 x;
	f32 pz;
	f32 z;
	f32 t = (vz * ((z = p->z) - (pz = l->pz)) + (vx * ((x = p->x) - (px = l->px)) + vy * ((y = p->y) - (py = l->py)))) / (vz * vz + (vx * vx + vy * vy));
	f32 d;

	if (out != NULL) {
		out->x = px + vx * t;
		out->y = py + vy * t;
		out->z = pz + vz * t;
		d = (z - out->z) * (z - out->z) + ((x - out->x) * (x - out->x) + (y - out->y) * (y - out->y));
	} else {
		f32 dz;
		f32 dx;
		f32 dy;
		d = (dz = z - (pz + vz * t)) * dz + ((dx = x - (px + vx * t)) * dx + (dy = y - (py + vy * t)) * dy);
	}
	if (d < 0.025f) {
		return 0.0f;
	}
	return sqrtf(d);
}
