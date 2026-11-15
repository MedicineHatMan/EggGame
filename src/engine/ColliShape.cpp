#include "core/types.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "engine/GroundQuery.h"

// 0x8000AB0C
int colliCylinderContains(const Vec3f* a, const Vec3f* b, f32 radius, f32 height)
{
	f32 dy = a->y - b->y;
	f32 dx = a->x - b->x;
	f32 dz = a->z - b->z;
	if ((f32)__fabs(dy) <= height) {
		radius *= radius;
		return dz * dz + dx * dx <= radius;
	}
	return 0;
}

// 0x8000AB6C
int colliBoxContains(const Vec3f* point, const Vec3f* center, int angle, f32 sizeX, f32 sizeY, f32 sizeZ)
{
	f32 dx = point->x - center->x;
	f32 dz = point->z - center->z;
	if ((f32)__fabs(point->y - center->y) < sizeY) {
		if (angle != 0) {
			f32 s, c;
			njSinCos(-angle, &s, &c);
			f32 x = dx;
			dx = c * dx + s * dz;
			dz = c * dz - s * x;
		}
		if ((f32)__fabs(dx) < sizeX && (f32)__fabs(dz) < sizeZ) {
			return 1;
		}
	}
	return 0;
}

// A global with nothing to build
struct ColliShapeStatic {
	ColliShapeStatic() { }
};
static ColliShapeStatic sColliShapeStatic;
