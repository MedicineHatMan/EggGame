#include "stage/Stage.h"
#include "core/VecMath.h"
#include "Ninja/ninja.h"

extern "C" {

// 0x80100584
// Signed height of point above the plane through onPlane
f32 planeHeight(const Vec3f* point, const Vec3f* onPlane, const Vec3f* normal)
{
	Vec3f d;

	vecSub(&d, point, onPlane);
	return njInnerProduct(&d, normal);
}

// 0x801005D0
// The same written out
f32 planeHeightInline(const Vec3f* point, const Vec3f* onPlane, const Vec3f* normal)
{
	return normal->x * (point->x - onPlane->x) + normal->y * (point->y - onPlane->y) + normal->z * (point->z - onPlane->z);
}

// 0x80100610
// 1 when point lies inside the triangle a b c as seen along normal
bool pointInTriangle(const Vec3f* point, const Vec3f* a, const Vec3f* b, const Vec3f* c, const Vec3f* normal)
{
	Vec3f toA;
	Vec3f toB;
	Vec3f toC;
	Vec3f side;

	vecSub(&toA, a, point);
	vecSub(&toB, b, point);
	njOuterProduct(&toA, &toB, &side);
	if (njInnerProduct(&side, normal) < 0.0f) {
		return 0;
	}
	vecSub(&toC, c, point);
	njOuterProduct(&toB, &toC, &side);
	if (njInnerProduct(&side, normal) < 0.0f) {
		return 0;
	}
	njOuterProduct(&toC, &toA, &side);
	return !(njInnerProduct(&side, normal) < 0.0f);
}

// 0x80100704
// Where the step dir from from crosses the plane height below it
// 0 when it moves away from the plane or does not reach it within the step
int planeMeet(f32 height, const Vec3f* from, const Vec3f* dir, const Vec3f* normal, Vec3f* out)
{
	f32 dot = njInnerProduct(dir, normal);
	if (dot >= 0.0f) {
		return 0;
	}
	f32 t = -height / dot;
	if (t < 0.0f || t > 1.0f) {
		return 0;
	}
	vecScaleAdd(t, out, dir, from);
	return 1;
}
}
