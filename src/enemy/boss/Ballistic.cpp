#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "object/ObjAction.h"
#include "enemy/boss/Ballistic.h"

extern "C" f32 ballisticArc(f32 t)
{
	return (1.0f - t) * (2.0f * t);
}

// 0x8008528C
extern "C" f32 launchSpeedInSteps(int steps, f32 height, f32 gravity)
{
	f32 n = (f32)steps;
	return height / n - (0.5f * gravity) * (1.0f + n);
}

// 0x800852CC
extern "C" f32 gravityInSteps(int steps, f32 speed, f32 height)
{
	f32 n = (f32)steps;
	return (height - speed * n) / ((0.5f * n) * (1.0f + n));
}

// 0x80085310
extern "C" f32 speedForJumpHeight(f32 gravity, f32 height)
{
	return __fres(__frsqrte(-2.0f * gravity * height));
}

// 0x8008532C
extern "C" f32 launchSpeedFor(f32 frames, f32 height, f32 gravity)
{
	return height / frames - (0.5f * gravity) * frames;
}

// 0x80085340
extern "C" void ballisticSolve(f32 a, f32 gravity, f32 c, f32* outSpeed, f32* outHeight)
{
	*outSpeed  = quadRoot(a * a / (4.0f * gravity), -a, c);
	*outHeight = (-*outSpeed * *outSpeed) / (2.0f * gravity);
}

// 0x800853B8
extern "C" void ballisticRoots(f32 a, f32 b, f32 c, f32* first, f32* second)
{
	a *= 0.5f;
	c *= -1.0f;
	quadRoots(a, b, c, first, second);
}

// 0x800853E8
extern "C" f32 flatVelocityTo(const Vec3f* from, const Vec3f* to, Vec3f* vel, f32 speed)
{
	vel->x = to->x;
	vel->y = to->y;
	vel->z = to->z;
	njSubVector(vel, from);
	f32 frames = vecNormalizeXZ(vel, vel) / speed;
	gjScaleVector(speed, vel, vel);
	return frames;
}

// 0x80085480
extern "C" void parabolaPoint(const Vec3f* start, const Vec3f* vel, Vec3f* out, f32 gravity, f32 time)
{
	gjMulAddVector(time, out, start, vel);
	out->y = time * ((0.5f * gravity) * time) + out->y;
}
