#include "camera/TCamera.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "enemy/EnemyFx.h"
#include "camera/CamCurve.h"
#include "camera/CamChase.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "animal/RacerMover.h"

struct CamCurveState {
	u32 mFlags;      // _00 1 drifting 2 held
	f32 mHeight;     // _04
	f32 mLift;       // _08
	Vec3f mDrift;    // _0C
	Vec3f mLastDir;  // _18
	int mSteady;     // _24 frames the way it goes has held
	Vec3f mOffset;   // _28
	Vec3f mPos;      // _34
	f32 mSpring[5];  // _40
	Vec3f mVel;      // _54
	CamTrail mTrail; // _60
};

#define CURVE(c) ((CamCurveState*)c)

extern "C" {

// 0x80098B70
void camCurveStart(CamCurve* curve, const Vec3f* at)
{
	CamCurveState* c = CURVE(curve);
	c->mHeight       = 0.0f;
	c->mLift         = c->mHeight;
	c->mSpring[0]    = 0.1f;
	c->mSpring[1]    = 0.7f;
	c->mSpring[2]    = 0.02f;
	c->mSpring[3]    = 0.05f;
	c->mSpring[4]    = 0.1f;
	camCurveReset(curve, at);
}

// 0x80098BC4
void camCurveReset(CamCurve* curve, const Vec3f* at)
{
	CamCurveState* c = CURVE(curve);
	c->mFlags        = 0;
	c->mLastDir.x    = 0.0f;
	c->mLastDir.y    = 0.0f;
	c->mLastDir.z    = 0.0f;
	c->mSteady       = 0;
	c->mDrift.x      = 0.0f;
	c->mDrift.y      = 0.0f;
	c->mDrift.z      = 0.0f;
	c->mOffset.x     = 0.0f;
	c->mOffset.y     = 0.0f;
	c->mOffset.z     = 0.0f;
	c->mOffset.y     = c->mLift;
	c->mPos          = *at;
	njAddVector(&c->mPos, &c->mOffset);
	c->mVel.x = 0.0f;
	c->mVel.y = 0.0f;
	c->mVel.z = 0.0f;
	camTrailClear(&c->mTrail);
}

// 0x80098C64
void camCurveGetPos(const CamCurve* curve, Vec3f* out)
{
	*out = CURVE(curve)->mPos;
}

// 0x80098C80
// Drifts ahead once the head has gone one way for 18 frames and lets the drift die away once it has been still for a
// second
int camCurveDrift(CamCurve* curve, int still, const Vec3f* dir)
{
	CamCurveState* c = CURVE(curve);
	int result       = 0;
	if ((f32)still > 60.0f) {
		if ((c->mFlags & 1) != 0) {
			gjScaleVector(0.9f, &c->mDrift, &c->mDrift);
		}
	} else if (still == 0) {
		if (njInnerProduct(dir, &c->mLastDir) >= 0.98f) {
			c->mSteady++;
			if (c->mSteady >= 18) {
				c->mDrift = *dir;
				gjScaleVector(26.0f, &c->mDrift, &c->mDrift);
				c->mFlags &= ~1;
			}
			result = 1;
		} else {
			c->mSteady  = 0;
			c->mDrift.x = 0.0f;
			c->mDrift.y = 0.0f;
			c->mDrift.z = 0.0f;
			camCurveSetDrifting(curve);
			result = 2;
		}
		c->mLastDir = *dir;
	}
	return result;
}

// 0x80098DB0
void camCurveSetHeight(f32 height, CamCurve* curve)
{
	CURVE(curve)->mHeight = height;
}

// 0x80098DB8
void camCurveSetLift(f32 lift, CamCurve* curve)
{
	CURVE(curve)->mLift = lift;
}

// 0x80098DC0
void camCurveResetLift(CamCurve* curve)
{
	CURVE(curve)->mLift = CURVE(curve)->mHeight;
}

// 0x80098DCC
void camCurveSetDrifting(CamCurve* curve)
{
	CURVE(curve)->mFlags |= 1;
}

// 0x80098DDC
// Stops drifting
void camCurveHold(CamCurve* curve)
{
	CamCurveState* c = CURVE(curve);
	c->mFlags |= 2;
	c->mLastDir.x = 0.0f;
	c->mLastDir.y = 0.0f;
	c->mLastDir.z = 0.0f;
	c->mSteady    = 0;
	c->mDrift.x   = 0.0f;
	c->mDrift.y   = 0.0f;
	c->mDrift.z   = 0.0f;
}

// 0x80098E10
// Springs the offset to the drift and puts the point there from the head
void camCurveUpdate(CamCurve* curve, CamHead* head)
{
	CamCurveState* c = CURVE(curve);
	Vec3f dir;
	camHeadGetDir(head, &dir);
	int still = camHeadGetStill(head);
	if ((c->mFlags & 2) == 0) {
		camCurveDrift(curve, still, &dir);
	}
	Vec3f goal;
	f32 x = c->mDrift.x;
	f32 y = c->mDrift.y;
	f32 z = c->mDrift.z;
	y += c->mLift;
	goal.set(x, y, z);
	njSubVector(&goal, &c->mOffset);
	camSpring(0.2f, c->mSpring, &c->mVel, &goal);
	njAddVector(&c->mOffset, &c->mVel);
	camHeadGetPos(head, &c->mPos);
	njAddVector(&c->mPos, &c->mOffset);
	camTrailPush(10.0f, &c->mTrail, &c->mPos);
}

// 0x80098EE0
f32 camSpring(f32 rate, f32* params, Vec3f* vel, const Vec3f* goal)
{
	return springVelocity(params[0], rate, vel, params[3], goal, params[4], params[1], params[2]);
}
}
