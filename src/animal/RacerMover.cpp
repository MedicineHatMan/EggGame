#include "animal/RacerMover.h"
#include "egg/ObjEgg.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "engine/GroundQuery.h"

extern "C" {
static inline f32 moverAbs(f32 x) { return (f32)__fabs(x); }

}

static inline Vec3f vsub(const Vec3f& a, const Vec3f& b)
{
	Vec3f r;
	gjSubVector(&r, &a, &b);
	return r;
}

static inline Vec3f vscale(const Vec3f& v, f32 s)
{
	Vec3f r;
	gjScaleVector(s, &r, &v);
	return r;
}

// 0x80199740
RacerMover::RacerMover()
{
	mKind      = 0;
	mAccel     = 0.1f;
	mTopSpeed  = 2.0f;
	mF0C       = 0.01f;
	mTurn      = 0x100;
	mTarget.x  = 0.0f;
	mTarget.y  = 0.0f;
	mTarget.z  = 0.0f;
	mPos.x     = 0.0f;
	mPos.y     = 0.0f;
	mPos.z     = 0.0f;
	mVel.x     = 0.0f;
	mVel.y     = 0.0f;
	mVel.z     = 0.0f;
	mRot[0]    = 0;
	mRot[1]    = 0;
	mRot[2]    = 0;
	mGravity   = -0.01f;
	mMinFrames = 60;
	mMaxFrames = 120;
	mFlags     = 0;
}

// 0x801997BC
void RacerMover::update()
{
	switch ((s8)mKind) {
	case 0:
		walkMover(this);
		break;
	case 1:
		flyMover(this);
		break;
	}
}

// 0x80199804
// Kind 0 run along the ground and launch over a wall or a drop
void walkMover(RacerMover* mover)
{
	EggHits hits;
	mover->mFlags &= ~0x1D;

	Vec3f probe = mover->mPos;
	probe.y += 10.0f;

	Vec3f ahead = vsub(mover->mPos, mover->mTarget);
	ahead.y     = 0.0f;
	if (njUnitVector(&ahead) < gUnitVectorMin) {
		ahead = gVecZero;
	}
	gjScaleVector(15.0f, &ahead, &ahead);
	njAddVector(&ahead, &mover->mTarget);
	ahead.y += 10.0f;

	f32 fall = mover->mVel.y + mover->mGravity;

	Vec3f dir;
	f32 speed;
	dir = vsub(mover->mTarget, mover->mPos);
	speed = vecLenXZ(&mover->mVel) + mover->mF0C;
	dir.y       = 0.0f;
	f32 cap     = mover->mAccel * vecLenXZ(&dir);
	if (speed > cap) {
		speed = cap;
	}
	if (speed > mover->mTopSpeed) {
		speed = mover->mTopSpeed;
	}

	if (speed > 0.0f) {
		mover->mRot[1] = angleApproach(mover->mRot[1], (int)(10430.38043493439 * (f32)atan2(dir.x, dir.z)), mover->mTurn);
	} else {
		const Vec3f& ref1 = vsub(mover->mTarget, mover->mPos);
		dir = ref1;
		mover->mRot[1] = angleApproach(mover->mRot[1], (int)(10430.38043493439 * (f32)atan2(dir.x, dir.z)), mover->mTurn);
	}

	if (fall < 0.0f) {
		hits.mMask = 0;
		if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&probe, 1) != 0) {
			if ((hits.mMask & 1) != 0 && hits.mHits[0].mPos.y - mover->mPos.y >= fall) {
				mover->mPos.y = hits.mHits[0].mPos.y;
				fall          = 0.0f;
				mover->mFlags |= 1;
				mover->mFlags &= ~2;
			}
		} else {
			fall = 0.0f;
		}
	}

	dir.x = 0.0f;
	dir.y = 0.0f;
	dir.z = 1.0f;
	vecRotateY(&dir, &dir, mover->mRot[1]);
	if ((mover->mFlags & 1) != 0) {
		Vec3f front = vscale(dir, 15.0f);
		njAddVector(&front, &probe);
		hits.mMask = 0;
		if (meshColliProbeLine((EggHit*)&hits, (const Vec3f*)&probe, (const Vec3f*)&front, 0x2201) != 0) {
			mover->mFlags |= 4;
		}
		hits.mMask = 0;
		if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&front, 1) != 0) {
			if (mover->mPos.y - hits.mHits[0].mPos.y >= 30.0f) {
				mover->mFlags |= 8;
			} else {
				mover->mFlags &= ~8;
			}
		} else {
			mover->mFlags |= 8;
		}
	}
	gjScaleVector(speed, &dir, &dir);

	if ((mover->mFlags & 4) != 0 || (mover->mFlags & 8) != 0) {
		hits.mMask = 0;
		dir.x      = 0.0f;
		dir.y      = 0.0f;
		dir.z      = 0.0f;
		if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&ahead, 1) != 0) {
			Vec3f land = hits.mHits[0].mPos;
			f32 rise   = fabs(ahead.y - land.y);
			f32 dist   = distXZ(&land, &mover->mPos);
			u32 turn   = (u16)((u16)angleToXZ(&mover->mPos, &land) - mover->mRot[1]);
			if (rise <= 80.0f && dist >= 50.0f && (turn < 0x1000 || turn > 0xF000)) {
				launchMover(mover, &mover->mLaunch, &mover->mPos, &land);
				mover->mFlags |= 0x10;
				mover->mFlags |= 2;
				return;
			}
		}
	}

	dir.y       = fall;
	mover->mVel = dir;
	njAddVector(&mover->mPos, &mover->mVel);
}

// 0x80199D24
// Kind 1 fly at the target banking into the turn
void flyMover(RacerMover* mover)
{
	Vec3f dir = vsub(mover->mTarget, mover->mPos);
	f32 speed = njScalor(&mover->mVel);
	speed += mover->mF0C;
	f32 cap   = mover->mAccel * njScalor(&dir);
	if (speed > cap) {
		speed = cap;
	}
	if (speed > mover->mTopSpeed) {
		speed = mover->mTopSpeed;
	}

	Vec3f vel;
	int old = mover->mRot[1];
	int now;
	if (speed > 0.0f) {
		now = angleApproach(old, (int)(10430.38043493439 * (f32)atan2(dir.x, dir.z)), mover->mTurn);
	} else {
		vel = vsub(mover->mTarget, mover->mPos);
		now = angleApproach(old, (int)(10430.38043493439 * (f32)atan2(vel.x, vel.z)), mover->mTurn);
	}
	mover->mRot[1] = now;

	int turned = now - old;
	int roll;
	if (turned > 0x40) {
		roll = angleApproach(mover->mRot[2], -0x1000, 0x80);
	} else if (turned < -0x40) {
		roll = angleApproach(mover->mRot[2], 0x1000, 0x80);
	} else {
		roll = angleApproach(mover->mRot[2], 0, 0x80);
	}
	mover->mRot[2] = roll;

	vel.x = 0.0f;
	vel.y = 0.0f;
	vel.z = 1.0f;
	vecRotateY(&vel, &vel, mover->mRot[1]);
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	vel.y = dir.y;
	if (njUnitVector(&vel) < gUnitVectorMin) {
		vel = gVecZero;
	}
	gjScaleVector(speed, &vel, &vel);
	mover->mVel = vel;
	njAddVector(&mover->mPos, &mover->mVel);
}

// 0x80199F80
// The velocity that carries from to to under mGravity taking 65 units a frame clamped to mMinFrames..mMaxFrames
void launchMover(RacerMover* mover, Vec3f* out, const Vec3f* from, const Vec3f* to)
{
	*out       = vsub(*to, *from);
	f64 absY   = __fabs(out->y);
	f32 rise   = absY;
	f32 run    = vecNormalizeXZ(out, out);
	u32 frames;
	if (rise > run) {
		frames = rise / 65.0f * mover->mMinFrames;
	} else {
		frames = run / 65.0f * mover->mMinFrames;
	}
	if (frames < mover->mMinFrames) {
		frames = mover->mMinFrames;
	} else if (frames > mover->mMaxFrames) {
		frames = mover->mMaxFrames;
	}
	f32 vy = 0.5f * -mover->mGravity * frames + rise / frames;
	gjScaleVector(run / frames, out, out);
	out->y = vy;
}
