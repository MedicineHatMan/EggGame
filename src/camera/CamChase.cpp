#include "camera/CamChase.h"
#include "egg/ObjEgg.h"
#include "engine/MeshColli.h"
#include "object/ObjAction.h"
#include "enemy/EnemyFx.h"
#include "camera/CamCurve.h"
#include "camera/CamPad.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "engine/GroundQuery.h"
#include "core/MathUtil.h"
#include "camera/TCameraCtrl.h"
#include "animal/RacerMover.h"
struct TObjAction;

/** Where the lock-on target is from camLockGetTarget */
struct CamLockTarget {
	u16 mId;    // _00
	u8 _02[2];
	Vec3f mPos; // _04
};

extern "C" {

void camChasePush(CamChase* chase, MeshList* list, const Vec3f* dir, u32 hiBit, u32 loBit, f32 range, f32 cur,
                  f32 min, f32 max);

static inline void flatUnit(Vec3f* v)
{
	if (njUnitVector(v) < gUnitVectorMin) {
		*v = gVecZero;
	}
}

// 0x80095A44
// 1 if `to` is ahead of `from` -1 if behind
f32 camAngleSign(int from, int to, int* dist)
{
	int diff = angleDiff(from, to);
	f32 sign;
	if (diff <= 0) {
		sign  = -1.0f;
		*dist = -diff;
	} else {
		sign  = 1.0f;
		*dist = diff;
	}
	return sign;
}

// 0x80095A90
f32 camAngleToMove(f32 dist, int angle)
{
	return 45.0 * (9.587379924285257e-05 * angle);
}

// 0x80095ACC
int camMoveToAngle(f32 dist, f32 move)
{
	return 10430.38043493439 * (move / 45.0f);
}

// 0x80095AF4
u32 camLineHitList(EggHits* hits, MeshList* list, const Vec3f* from, const Vec3f* to, u32 mask)
{
	Vec3f d;
	d = *from;
	njSubVector(&d, to);
	if (njScalor(&d) >= 1000.0f) {
		return 0;
	}
	return meshListProbeLine((EggHit*)hits, list, from, to, mask);
}

// 0x80095B88
int camLineHit(EggHit* hits, const Vec3f* from, const Vec3f* to, int mask)
{
	Vec3f d;
	d.x = from->x;
	d.y = from->y;
	d.z = from->z;
	njSubVector(&d, to);
	if (njScalor(&d) >= 1000.0f) {
		return 0;
	}
	return (int)meshColliProbeLine(hits, from, to, mask);
}

// 0x80095C14
f32 camMaxScaled(f32 a, f32 scale, f32 min)
{
	if (!(a * scale < min)) {
		return a * scale;
	}
	return min;
}

// 0x80095C28
f32 camAccel(f32 v, f32 up, f32 down, f32 limit)
{
	f32 add;
	if (v < 0.0f) {
		add = down;
	} else {
		add = up;
	}
	if (v + add > limit) {
		return limit;
	}
	return v + add;
}

// 0x80095C50
void camTurn(f32 dist, f32 move, f32 sign, int* rot)
{
	int step = camMoveToAngle(dist, move);
	if (sign > 0.0f) {
		*rot += step;
	} else {
		*rot -= step;
	}
}

// 0x80095CB8
// How far along dir from pos the first wall is between from and to
f32 camProbeAlong(MeshList* list, const Vec3f* pos, const Vec3f* dir, f32 from, f32 to)
{
	EggHits hits;
	Vec3f start;
	Vec3f end;

	gjMulAddVector(from, &start, pos, dir);
	gjMulAddVector(to, &end, pos, dir);
	if (camLineHitList(&hits, list, &start, &end, 0x20) != 0) {
		end = hits.mHits[0].mPos;
		njSubVector(&end, pos);
		return njScalor(&end);
	}
	return 1000.0f;
}

// 0x80095DAC
// How far to move so the camera sits between the walls hi and lo away keeping min and max of room
// Bit 1 of flags is set for the hi wall bit 2 for the lo one
f32 camFitRange(f32 cur, f32 hi, f32 lo, f32 min, f32 max, u32* flags)
{
	*flags = 0;
	if (hi - lo < min - max) {
		f32 mid = 0.5f;
		mid *= hi + lo;
		*flags |= 3;
		return mid;
	}
	if (hi < min) {
		*flags |= 1;
		return hi - min;
	}
	if (lo > max) {
		*flags |= 2;
		return lo - max;
	}
	if (cur > 0.0f) {
		if (hi - cur < min) {
			hi -= min;
			cur = 0.4f * hi;
			*flags |= 1;
		}
	} else if (lo - cur > max) {
		lo -= max;
		cur = 0.4f * lo;
		*flags |= 2;
	}
	return cur;
}

// 0x80095E74
void camChaseInit(CamChase* chase, CamHead* head, CamCurve* curve)
{
	chase->mHead       = head;
	chase->mCurve      = curve;
	chase->mView       = 0;
	chase->mBaseHeight = 25.0f;
	chase->mBaseDist   = -87.0f;
	chase->mHeight     = chase->mBaseHeight;
	chase->mDist       = chase->mBaseDist;
	chase->mOffset[0]  = chase->mHeight;
	chase->mOffset[1]  = chase->mDist;
	chase->mPitchMax   = 0x2000;
	chase->mPitch      = chase->mPitchMax;
	chase->mPitchMin   = 0;
	chase->mBaseSpeed  = 1.0f;
	chase->mRiseRate   = 0.5f;
	chase->mFallRate   = 0.08f;
	chase->mDropRate   = 0.01f;
	chase->mFollowRate = 0.5f;
	chase->mSpeed      = chase->mBaseSpeed;
	camChaseReset(chase, camHeadGetRot(chase->mHead));
}

// 0x80095F30
void camChaseReset(CamChase* chase, int rot)
{
	Vec3f at;

	chase->mState         = CHASE_FOLLOW;
	chase->mGround.mCount = 0;
	chase->mGround.mVel   = 0.0f;
	chase->mGround.mPos   = 0.0f;
	chase->mStick.mCount  = 0;
	chase->mStick.mVel    = 0.0f;
	chase->mStick.mPos    = 0.0f;
	chase->mWallCount     = 0;
	chase->_50            = 0.0f;
	chase->mWall          = 0.0f;
	chase->mSideCount     = 0;
	chase->mSideVel       = 0.0f;
	chase->_60            = 0.0f;
	chase->mSide          = 0.0f;
	chase->_68            = 0.0f;
	chase->mRot           = rot;
	chase->mRotVel        = 0.0f;
	chase->mTarget.x      = 0.0f;
	chase->mTarget.y      = 0.0f;
	chase->mTarget.z      = 0.0f;
	chase->mToTarget.x    = 0.0f;
	chase->mToTarget.y    = 0.0f;
	chase->mToTarget.z    = 0.0f;
	camTrailClear(&chase->mTrail);
	chase->mHits  = 0;
	chase->mFlags = 0;
	camChaseClearFollow(chase);
	chase->mSub      = 0;
	chase->mTimer    = 0;
	chase->mSubFlag  = 0;
	chase->mSubRot   = 0;
	chase->mSubStep  = 0.0f;
	camHeadGetPos(chase->mHead, &at);
	camChaseEyeAt(chase, &at, &chase->mPos);
}

// 0x80096010
// Resets then puts the eye straight at its place or in front of the wall in the way
void camChaseSnap(CamChase* chase, int rot)
{
	Vec3f at;
	Vec3f eye;

	camChaseReset(chase, rot);
	camHeadGetPos(chase->mHead, &at);
	camChaseEyeAt(chase, &at, &eye);
	EggHits hits;
	if (camLineHit(hits.mHits, &at, &eye, 0x20) != 0) {
		Vec3f pos;
		Vec3f normal;
		pos    = hits.mHits[0].mPos;
		normal = hits.mHits[0].mNormal;
		normal.y     = 0.0f;
		gjMulAddVector(10.0f, &chase->mPos, &pos, &normal);
	} else {
		chase->mPos = eye;
	}
}

// 0x800960F8
void camChaseGetPos(const CamChase* chase, Vec3f* out)
{
	*out = chase->mPos;
}

// 0x80096114
void camChaseSetBase(CamChase* chase, f32 height, f32 dist)
{
	chase->mBaseHeight = height;
	chase->mBaseDist   = dist;
}

// 0x80096120
void camChaseSetOffset(CamChase* chase, f32 height, f32 dist)
{
	chase->mHeight = height;
	chase->mDist   = dist;
}

// 0x8009612C
void camChaseResetOffset(CamChase* chase)
{
	chase->mHeight = chase->mBaseHeight;
	chase->mDist   = chase->mBaseDist;
}

// 0x80096140
void camChaseSetBaseSpeed(CamChase* chase, f32 speed)
{
	if (speed < 0.0f) {
		speed = 0.0f;
	}
	chase->mBaseSpeed = speed;
}

// 0x80096158
void camChaseSetSpeed(CamChase* chase, f32 speed)
{
	if (speed < 0.0f) {
		speed = 0.0f;
	}
	chase->mSpeed = speed;
}

// 0x80096170
void camChaseResetSpeed(CamChase* chase)
{
	chase->mSpeed = chase->mBaseSpeed;
}

// 0x8009617C
void camChaseSetPitch(CamChase* chase, int pitch)
{
	chase->mPitch = pitch;
}

// 0x80096184
void camChaseResetPitch(CamChase* chase)
{
	chase->mPitch = chase->mPitchMax;
}

// 0x80096190
// Where the eye goes for a head at `at`
void camChaseEyeAt(CamChase* chase, const Vec3f* at, Vec3f* out)
{
	out->x = 0.0f;
	out->y = chase->mOffset[0];
	out->z = chase->mOffset[1];
	vecRotateY(out, out, chase->mRot);
	njAddVector(out, at);
}

// 0x800961F4
// Moves the eye along dir to keep it between the walls either way
void camChasePush(CamChase* chase, MeshList* list, const Vec3f* dir, u32 hiBit, u32 loBit, f32 range, f32 cur,
                  f32 min, f32 max)
{
	u32 flags;
	f32 hi    = camProbeAlong(list, &chase->mPos, dir, -2.0f, range);
	f32 lo    = -camProbeAlong(list, &chase->mPos, dir, 2.0f, -range);
	cur = camFitRange(cur, hi, lo, min, max, &flags);
	if (flags & 1) {
		chase->mHits |= hiBit;
	} else if (flags & 2) {
		chase->mHits |= loBit;
	}
	gjMulAddVector(cur, &chase->mPos, &chase->mPos, dir);
}

// 0x80096314
void camChasePushUp(CamChase* chase, f32 range, MeshList* list, const Vec3f* goal)
{
	f32 gy = goal->y;
	Vec3f up;
	up.z = up.x = up.y = 0.0f;
	up.y = 1.0f;
	gy -= chase->mPos.y;
	camChasePush(chase, list, &up, 1, 2, range, gy, 5.0f, -5.0f);
}

// 0x8009636C
void camChasePushSide(CamChase* chase, f32 range, MeshList* list, const Vec3f* goal, const Vec3f* head)
{
	Vec3f move;
	Vec3f ahead;
	Vec3f side;

	move = *goal;
	njSubVector(&move, &chase->mPos);
	move.y = 0.0f;
	ahead = *head;
	njSubVector(&ahead, &chase->mPos);
	ahead.y = 0.0f;
	flatUnit(&ahead);
	side.x = -ahead.z;
	side.y = 0.0f;
	side.z = ahead.x;
	camChasePush(chase, list, &ahead, 0x10, 0x20, range, njInnerProduct(&move, &ahead), 5.0f, -5.0f);
	camChasePush(chase, list, &side, 8, 4, range, njInnerProduct(&move, &side), 5.0f, -5.0f);
}

// 0x800964BC
// Moves the eye toward its place this frame round the walls
void camChaseMove(CamChase* chase)
{
	Vec3f at;
	Vec3f goal;
	MeshList list;
	MeshSphere sphere;
	Vec3f normal;
	Vec3f along;
	Vec3f tmp;
	Vec3f slide;
	Vec3f gap;
	f32 rate;
	int close;

	chase->mHits = 0;
	queryBegin(&list);
	camHeadGetPos(chase->mHead, &at);
	camChaseEyeAt(chase, &at, &goal);
	chase->mRise = goal.y;
	rate   = chase->mFollowRate;
	goal.x *= rate;
	goal.z *= rate;
	goal.x = chase->mPos.x * (1.0f - rate) + goal.x;
	goal.z = chase->mPos.z * (1.0f - rate) + goal.z;
	if (goal.y - chase->mPos.y < -1.0f) {
		if (camHeadGetRise(chase->mHead) < -0.8f) {
			rate = chase->mDropRate;
		} else {
			rate = chase->mFallRate;
		}
	} else {
		rate = chase->mRiseRate;
	}
	goal.y *= rate;
	rate = 1.0f - rate;
	goal.y = chase->mPos.y * rate + goal.y;

	if ((chase->mFlags & 3) == 0) {
		EggHits hits;
		if (camLineHit(hits.mHits, &chase->mPos, &goal, 0x20) != 0) {
			along  = hits.mHits[0].mPos;
			normal = hits.mHits[0].mNormal;
			tmp    = goal;
			njSubVector(&tmp, &along);
			f32 len = njUnitVector(&tmp);
			if (len < gUnitVectorMin) {
				tmp = gVecZero;
			}
			normal.y = 0.0f;
			flatUnit(&normal);
			gjMulAddVector(-njInnerProduct(&normal, &tmp), &slide, &tmp, &normal);
			gjMulAddVector(len, &goal, &along, &slide);
			if (camLineHit(hits.mHits, &chase->mPos, &goal, 0x20) != 0) {
				goal = hits.mHits[0].mPos;
			}
		}
	}

	sphere.mPos    = goal;
	sphere.mRadius = 25.0f;
	if ((chase->mFlags & 3) != 0) {
		close = 0;
	} else {
		close = queryGatherBall(&list, &sphere);
	}
	if (close != 0) {
		camChasePushUp(chase, 22.5f, &list, &goal);
		camChasePushSide(chase, 22.5f, &list, &goal, &at);
		if (chase->mHits & 0x10) {
			chase->mWallFrames++;
		} else {
			chase->mWallFrames = 0;
		}
	} else {
		chase->mPos        = goal;
		chase->mWallFrames = 0;
	}

	gap = at;
	njSubVector(&gap, &chase->mPos);
	if (chase->mHits & 0x3C) {
		chase->mRot = 10430.38043493439 * (f32)atan2(gap.x, gap.z);
	}
	gap.y        = 0.0f;
	chase->mGap  = -njScalor(&gap);
	chase->mRise = chase->mRise - chase->mPos.y;
}

// 0x8009686C
void camClampUnit(CamChase* chase, f32* pos, f32* vel)
{
	f32 v = *vel;
	f32 p = *pos + v;
	if (p > 1.0f) {
		p = 1.0f;
		v = 0.0f;
	} else if (p < -1.0f) {
		p = -1.0f;
		v = 0.0f;
	}
	*vel = v;
	*pos = p;
}

// 0x800968B0
void camDamp(CamChase* chase, f32* pos, f32* vel, int* count)
{
	*vel *= 0.9f;
	*count = 0;
	camClampUnit(chase, pos, vel);
}

// 0x800968E8
// Pushes pos toward the input's side once it has held for upDelay or downDelay the other way frames
void camSteer(CamChase* chase, f32 input, f32* pos, f32* vel, int* count, int upDelay, int downDelay)
{
	f32 v      = *vel;
	f32 target = 0.0f;
	if (input < 0.05f && input >= -0.05f) {
		if (*count > 0) {
			*count = 0;
		}
		(*count)--;
		if (*count <= -downDelay) {
			v = approachSpeed(v, -*pos, 0.3f, 0.001f, 0.003f, 0.1f, 0.02f);
		} else {
			v *= 0.9f;
		}
	} else {
		if (*count < 0) {
			*count = 0;
		}
		(*count)++;
		if (*count >= upDelay) {
			f32 rate = 0.05f * input;
			if (input >= 0.0f) {
				target = 1.0f - *pos;
			} else if (input <= -0.1f) {
				rate *= -1.0f;
				target = -1.0f - *pos;
			}
			v = approachSpeed(v, target, 0.3f, 1.0f, 1.0f, rate, 0.02f);
		} else {
			v *= 0.9f;
		}
	}
	*vel = v;
	camClampUnit(chase, pos, vel);
}

// 0x80096A54
// Moves the offset's pitch toward max t above 0 or min by t keeping its distance along the ground
void camTilt(CamChase* chase, f32 t, f32* offset, int max, int min)
{
	f32 s;
	f32 c;
	int ang = 10430.38043493439 * (f32)atan2(offset[0], -offset[1]);
	int to;
	if (t > 0.0f) {
		f32 diff = max - ang;
		if (diff < 0.0f) {
			to = ang;
		} else {
			to = (int)(diff * t);
			to += ang;
		}
	} else {
		f32 diff = min - ang;
		if (diff > 0.0f) {
			to = ang;
		} else {
			to = -(int)(diff * t);
			to += ang;
		}
	}
	njSinCos(to, &s, &c);
	offset[0] = -(s * offset[1] / c);
}

// 0x80096B84
void camChaseSteerGround(CamChase* chase, f32 input)
{
	camSteer(chase, input, &chase->mGround.mPos, &chase->mGround.mVel, &chase->mGround.mCount, 0, 0);
}

// 0x80096BB8
// Looks at the ground a little ahead of the head and pitches for it
void camChaseLookGround(CamChase* chase)
{
	EggHits hits;
	Vec3f at;
	Vec3f probe;
	Vec3f dir;
	f32 input;

	if (chase->mView != 0) {
		dir   = chase->mView->mDir;
		dir.y = 0.0f;
		flatUnit(&dir);
	} else {
		dir.z = 0.0f;
		dir.x = 0.0f;
		dir.y = 0.0f;
		dir.z = 1.0f;
	}
	camCurveGetPos(chase->mCurve, &probe);
	camHeadGetPos(chase->mHead, &at);
	probe = at;
	gjMulAddVector(25.0f, &probe, &probe, &dir);
	if (camLineHit(hits.mHits, &at, &probe, 0x20) != 0) {
		input = 0.0f;
	} else if (meshColliProbe(hits.mHits, &probe, 0x20) != 0) {
		f32 dy = hits.mHits[0].mPos.y - at.y;
		if (dy > 10.0f) {
			input = -1.0f;
		} else if (dy < -20.0f) {
			input = 1.0f;
		} else {
			input = 0.0f;
		}
	} else {
		input = 1.0f;
	}
	camChaseSteerGround(chase, input);
}

// 0x80096D3C
void camChaseTiltGround(CamChase* chase)
{
	camTilt(chase, chase->mGround.mPos, chase->mOffset, 0x1900, 0x400);
}

// 0x80096D6C
void camChaseSteerStick(CamChase* chase, f32 input)
{
	camSteer(chase, input, &chase->mStick.mPos, &chase->mStick.mVel, &chase->mStick.mCount, 0, 120);
}

// 0x80096DA0
void camChaseReadStick(CamChase* chase)
{
	f32 y = 0.0f;
	if (chase->mPad != 0) {
		y = camPadY(chase->mPad);
	}
	camChaseSteerStick(chase, y);
}

// 0x80096DE4
void camChaseTiltStick(CamChase* chase)
{
	camTilt(chase, chase->mStick.mPos, chase->mOffset, chase->mPitch, chase->mPitchMin);
}

// 0x80096E14
// Pulls mWall up while a wall is near and lets it go 16 frames after
void camChaseBlendWall(CamChase* chase, f32 close)
{
	f32 wall  = chase->mWall;
	int count = chase->mWallCount;
	if (close > 0.0f) {
		wall += 0.02f;
		if (wall > 1.0f) {
			wall = 1.0f;
		}
		count = 0;
	} else if (count > 15) {
		wall -= 0.03f;
		if (wall < 0.0f) {
			wall = 0.0f;
		}
	} else {
		count++;
	}
	chase->mWall      = wall;
	chase->mWallCount = count;
}

// 0x80096E78
void camChaseCheckWall(CamChase* chase)
{
	f32 close;
	if (chase->mHits & 0xC) {
		close = 1.0f;
	} else {
		close = 0.0f;
	}
	camChaseBlendWall(chase, close);
}

// 0x80096EB0
void camChasePullIn(CamChase* chase, f32 dist)
{
	if (chase->mWall > 0.0f) {
		dist = -dist;
		dist -= chase->mOffset[1];
		dist *= chase->mWall;
		chase->mOffset[1] += dist;
	}
}

// 0x80096EDC
void camChaseSteerSide(CamChase* chase, f32 input)
{
	camSteer(chase, input, &chase->mSide, &chase->mSideVel, &chase->mSideCount, 0, 0);
}

// 0x80096F10
void camChaseDampSide(CamChase* chase)
{
	camDamp(chase, &chase->mSide, &chase->mSideVel, &chase->mSideCount);
}

// 0x80096F3C
void camChaseTiltSide(CamChase* chase)
{
	camTilt(chase, chase->mSide, chase->mOffset, 0x2800, 0);
}

// 0x80096F6C
int camChaseHardHeld(CamChase* chase)
{
	if (chase->mPad != 0) {
		return camPadHardHeld(chase->mPad);
	}
	return 0;
}

// 0x80096FA0
int camChaseStickSide(CamChase* chase)
{
	if (chase->mPad != 0) {
		u8 side = 0;
		if (camPadRight(chase->mPad) != 0 || camPadLeft(chase->mPad) != 0) {
			side = 1;
		}
		return side;
	}
	return 0;
}

// 0x80097004
void camChaseClearFollow(CamChase* chase)
{
	chase->mFollow      = 0;
	chase->mFollowTimer = 0;
	chase->mWallFrames  = 0;
	chase->mGap         = 0.0f;
	chase->mRise        = 0.0f;
	chase->mGapFlags    = 0;
}

// 0x80097028
// Watches how far the eye has been pushed off its place
int camChaseCheckFollow(CamChase* chase)
{
	f32 gap = -(chase->mGap - chase->mOffset[1]);
	f64 absRise = fabs(chase->mRise);
	f32 rise = absRise;
	if (chase->mWallFrames > 0 && (gap > 50.0f || rise >= 50.0f)) {
		chase->mFollow = 3;
		if ((chase->mFlags & 2) == 0) {
			chase->mRot         = camHeadGetRot(chase->mHead);
			chase->mWall        = 1.0f;
			chase->mStick.mPos  = -0.5f;
			chase->mStick.mVel  = 0.0f;
			chase->mSide        = -0.5f;
		}
		chase->mFlags |= 2;
	}
	if (gap >= 20.0f) {
		chase->mGapFlags |= 1;
	} else if (gap < -20.0f) {
		chase->mGapFlags |= 2;
	} else {
		chase->mGapFlags &= ~3;
	}
	switch (chase->mFollow) {
	case 0:
		if (chase->mGapFlags & 1) {
			chase->mFollowTimer = 24;
			chase->mFollow      = 1;
		}
		break;
	case 1:
		chase->mFollowTimer--;
		if (chase->mFollowTimer <= 0) {
			chase->mFollow = 0;
		}
		break;
	case 2:
		break;
	case 3:
		if (gap < 5.0f) {
			chase->mFollow = 0;
			chase->mFlags &= ~2;
		}
		break;
	}
	return chase->mFollow == 0;
}

// 0x800971CC
void camChaseHeadTop(CamChase* chase, Vec3f* out)
{
	camHeadGetPos(chase->mHead, out);
	out->y += 10.0f;
}

// 0x8009720C
void camChaseCheckSight(CamChase* chase)
{
	EggHits hits;
	Vec3f top;

	camChaseHeadTop(chase, &top);
	if (camLineHit(hits.mHits, &chase->mPos, &top, 0x20) != 0) {
		chase->mHits |= 0x40;
	} else {
		chase->mHits &= ~0x40;
	}
}

// 0x80097294
// Whether the head can be seen from the eye raised or lowered by t
int camChaseTryLift(CamChase* chase, f32 t, const f32* offset, int rot, const Vec3f* target)
{
	EggHits hits;
	f32 tilted[2];
	Vec3f eye;

	t *= 0.4f;
	if (t > 0.0f) {
		t += 0.6f;
	} else {
		t -= 0.6f;
	}
	tilted[0] = offset[0];
	tilted[1] = offset[1];
	camTilt(chase, t, tilted, 0x2800, 0);
	eye.x = 0.0f;
	eye.y = tilted[0];
	eye.z = tilted[1];
	vecRotateY(&eye, &eye, rot);
	njAddVector(&eye, target);
	if (camLineHit(hits.mHits, &chase->mPos, &eye, 0x20) == 0) {
		return camLineHit(hits.mHits, &eye, target, 0x20) == 0;
	}
	return 0;
}

// 0x800973C0
// The same with the eye turned by t instead
int camChaseTryTurn(CamChase* chase, f32 t, const f32* offset, int rot, const Vec3f* target)
{
	EggHits hits;
	Vec3f eye;

	t *= 0.9f;
	if (t > 0.0f) {
		t += 0.1f;
	} else {
		t -= 0.1f;
	}
	eye.x = 0.0f;
	eye.y = offset[0];
	eye.z = offset[1];
	rot += (int)(32768.0f * t);
	vecRotateY(&eye, &eye, rot);
	njAddVector(&eye, target);
	if (camLineHit(hits.mHits, &chase->mPos, &eye, 0x20) == 0) {
		return camLineHit(hits.mHits, &eye, target, 0x20) == 0;
	}
	return 0;
}

// 0x800974DC
// Looks for a place the head can be seen from 0 or 1 lifting up or down 2 or 3 turning 4 none
// first and second pick turning 1 or lifting 0 and the order
int camChaseFindView(CamChase* chase, f32 step, int first, int second)
{
	Vec3f top;
	Vec3f gap;
	f32 offset[2];
	int mode;
	int rot;

	camChaseHeadTop(chase, &top);
	gap = chase->mPos;
	njSubVector(&gap, &top);
	rot       = (int)(10430.38043493439 * (f32)atan2(gap.x, gap.z)) + 0x8000;
	offset[0] = chase->mOffset[0];
	offset[1] = chase->mOffset[1];
	mode = first;
	for (int i = 0; i < 2; i++) {
		if (mode == 1) {
			if (camChaseTryTurn(chase, -step, offset, rot, &top) != 0) {
				return 2;
			}
			if (camChaseTryTurn(chase, step, offset, rot, &top) != 0) {
				return 3;
			}
		} else if (mode == 0) {
			if (camChaseTryLift(chase, step, offset, rot, &top) != 0) {
				return 0;
			}
			if (camChaseTryLift(chase, -step, offset, rot, &top) != 0) {
				return 1;
			}
		}
		mode = second;
	}
	return 4;
}

// 0x80097668
void camChaseUpdate(CamChase* chase, CamViewInfo* view, CamPath* pad)
{
	chase->mView = view;
	chase->mPad  = pad;
	if (camHeadIsHidden(chase->mHead) == 0) {
		camChaseCallState(chase, chase->mState, CHASE_CHECK);
		camChaseCallState(chase, chase->mState, CHASE_UPDATE);
		camTrailPush(10.0f, &chase->mTrail, &chase->mPos);
	}
}

// 0x800976D8
void camChaseSetState(CamChase* chase, int state)
{
	camChaseCallState(chase, chase->mState, CHASE_EXIT);
	chase->mState = state;
	camChaseCallState(chase, chase->mState, CHASE_ENTER);
}

// 0x80097724
void camChaseCallState(CamChase* chase, int state, int event)
{
	switch (state) {
	case CHASE_FOLLOW:
		camChaseStateFollow(chase, event);
		break;
	case CHASE_SWING:
		camChaseStateSwing(chase, event);
		break;
	case CHASE_BEHIND:
		camChaseStateBehind(chase, event);
		break;
	case CHASE_LOCK:
		camChaseStateLock(chase, event);
		break;
	case CHASE_IDLE:
		camChaseStateIdle(chase, event);
		break;
	}
}

// 0x800977A4
bool camChaseCheckButtons(CamChase* chase)
{
	if (camChaseCheckLock(chase) != 0) {
		return 1;
	}
	return camChaseCheckBehind(chase) != 0;
}

// 0x800977F0
int camChaseCheckBehind(CamChase* chase)
{
	if (chase->mPad != 0 && camPadHardPressed(chase->mPad) != 0) {
		camChaseSetState(chase, CHASE_BEHIND);
		return 1;
	}
	return 0;
}

// 0x80097848
int camChaseCheckLock(CamChase* chase)
{
	if (camChaseHardHeld(chase) != 0 && camLockA((u16)camHeadGetId(chase->mHead)) != 0) {
		camChaseSetState(chase, CHASE_LOCK);
		return 1;
	}
	return 0;
}

// 0x800978AC
void camChaseStateFollow(CamChase* chase, int event)
{
	switch (event) {
	case CHASE_ENTER:
		chase->mSub     = 0;
		chase->mTimer   = 0;
		chase->mSubFlag = 0;
		chase->mSubStep = 0.05f;
		break;
	case CHASE_CHECK:
		if (camChaseStickSide(chase) != 0) {
			camChaseSetState(chase, CHASE_SWING);
		} else {
			camChaseCheckButtons(chase);
		}
		break;
	case CHASE_UPDATE: {
		if (chase->mHits & 0x40) {
			if (chase->mSub == 0 || chase->mSub == 1 || chase->mSub == 7) {
				int turn = camHeadTurn(chase->mHead, &chase->mView->mDir);
				int found;
				if (chase->mSub == 0) {
					if (turn == 4 || turn == 5) {
						found = camChaseFindView(chase, chase->mSubStep, 0, 1);
					} else {
						found = camChaseFindView(chase, chase->mSubStep, 1, 0);
					}
				} else {
					found = camChaseFindView(chase, chase->mSubStep, 1, -1);
				}
				switch (found) {
				case 4:
				default:
					chase->mSubStep += 0.05f;
					break;
				case 0:
					chase->mSub = 2;
					break;
				case 1:
					chase->mSub = 3;
					break;
				case 2:
					chase->mSub = 5;
					break;
				case 3:
					chase->mSub = 6;
					break;
				}
				chase->mTimer = 0;
			} else {
				if (chase->mSub == 2 || chase->mSub == 3) {
					f32 atSide = chase->mSide >= 0.95f;
					f32 held   = fabs(atSide);
					if (held) {
						chase->mTimer++;
						chase->mTimer = 0;
						chase->mSub   = 1;
					}
				}
				chase->mSubStep = 0.05f;
			}
		} else {
			switch (chase->mSub) {
			case 2:
			case 3:
				if (camHeadGetStill(chase->mHead) <= 0) {
					chase->mTimer++;
				}
				if (chase->mTimer >= 6) {
					chase->mTimer = 30;
					chase->mSub   = 4;
				}
				break;
			case 5:
			case 6:
				chase->mTimer = 30;
				chase->mSub   = 7;
				break;
			case 4:
			case 7:
				chase->mTimer--;
				if (chase->mTimer < 0) {
					chase->mSub = 0;
				}
				break;
			default:
				chase->mSub = 0;
				break;
			}
		}

		f32 dist = -chase->mDist;
		if (chase->mSub == 5 || chase->mSub == 6) {
			if (chase->mSub == 5) {
				chase->mRotVel = -4.0f;
			} else {
				chase->mRotVel = 4.0f;
			}
			camTurn(dist, chase->mRotVel, 1.0f, &chase->mRot);
		} else if (chase->mSub == 7) {
			chase->mRotVel *= 0.9f;
			camTurn(dist, chase->mRotVel, 1.0f, &chase->mRot);
		} else {
			int headRot;
			int ok = 1;
			int angle;
			f32 sign;
			f32 limit;
			f32 vel;
			headRot = camHeadGetRot(chase->mHead);
			sign    = camAngleSign(chase->mRot, headRot, &angle);
			vel     = chase->mRotVel * sign;
			limit   = camAngleToMove(dist, angle);
			if (angle > 0x5000) {
				ok = 0;
			}
			if (camHeadGetStill(chase->mHead) >= 6) {
				ok = 0;
			}
			if (ok) {
				vel = approachSpeed(vel, limit, 0.015f, 0.03f, 0.25f, 1000.0f, 0.01f);
				if (vel > 0.0f) {
					vel *= chase->mSpeed;
				}
				if (limit < vel) {
					chase->mRot = headRot;
					vel         = 0.0f;
				} else {
					camTurn(dist, vel, sign, &chase->mRot);
				}
			} else {
				vel *= 0.75f;
				if ((f32)fabs(vel) < 0.01f) {
					vel = 0.0f;
				} else {
					camTurn(dist, vel, sign, &chase->mRot);
				}
			}
			chase->mRotVel = vel * sign;
		}

		chase->mOffset[0] = chase->mHeight;
		chase->mOffset[1] = chase->mDist;
		if (camHeadGetRise(chase->mHead) < -0.8f) {
			if (chase->mHits & 0x40) {
				chase->mSubFlag = 1;
			}
		} else {
			chase->mSubFlag = 0;
		}
		f32 close;
		if (chase->mSubFlag == 1) {
			close = 1.0f;
		} else {
			chase->mWallCount = 100;
			close              = 0.0f;
		}
			if (chase->mSub == 2 || chase->mSub == 3 || chase->mSub == 4) {
				camChaseCheckFollow(chase);
				camChaseSteerGround(chase, 0.0f);
				camChaseSteerStick(chase, 0.0f);
				if (chase->mSub == 2) {
					camChaseSteerSide(chase, 1.0f);
				} else if (chase->mSub == 3) {
					camChaseSteerSide(chase, -1.0f);
				} else if (chase->mSub == 4) {
					camChaseDampSide(chase);
				}
				camChaseBlendWall(chase, close);
			} else {
				camChaseCheckFollow(chase);
				camChaseLookGround(chase);
				camChaseReadStick(chase);
				camChaseBlendWall(chase, close);
				camChaseSteerSide(chase, 0.0f);
			}
			camChaseTiltGround(chase);
			camChaseTiltStick(chase);
			if (close >= 1.0f) {
				camChasePullIn(chase, 7.0f);
			} else {
				camChasePullIn(chase, 20.0f);
			}
			camChaseTiltSide(chase);
			camChaseMove(chase);
			camChaseCheckSight(chase);
		break;
	}
	case CHASE_EXIT:
		break;
	}
}

// 0x80097EB8
void camChaseStateSwing(CamChase* chase, int event)
{
	switch (event) {
	case CHASE_ENTER:
		chase->mSub     = 0;
		chase->mTimer   = 0;
		chase->mSubFlag = 0;
		break;
	case CHASE_CHECK:
		if (chase->mSubFlag == 2) {
			camChaseSetState(chase, CHASE_FOLLOW);
		} else {
			camChaseCheckButtons(chase);
		}
		break;
	case CHASE_UPDATE: {
		camCurveSetDrifting(chase->mCurve);
		f32 dist = -chase->mOffset[1];
		f32 vel  = chase->mRotVel;
		if (camChaseStickSide(chase) == 0) {
			chase->mSub++;
			if (chase->mSub >= 3) {
				chase->mSubFlag = 2;
			}
			vel *= 0.75f;
		} else {
			chase->mSub = 0;
			f32 x;
			if (chase->mPad != 0) {
				x = camPadX(chase->mPad);
			} else {
				x = 0.0f;
			}
			if ((f32)fabs(x) < 0.1f) {
				vel *= 0.9f;
			} else {
				f32 goal = 1.5f * x;
				if (x < 0.0f) {
					vel *= -1.0f;
					goal *= -1.0f;
				}
				if (vel < 0.0f) {
					vel += 0.3f;
				} else {
					vel += 0.2f;
				}
				if (vel > goal) {
					vel = goal;
				}
				if (x < 0.0f) {
					vel *= -1.0f;
				}
			}
		}
		chase->mRot += camMoveToAngle(dist, vel);
		chase->mRotVel    = vel;
		chase->mOffset[0] = chase->mHeight;
		chase->mOffset[1] = chase->mDist;
		camChaseCheckFollow(chase);
		camChaseLookGround(chase);
		camChaseReadStick(chase);
		camChaseCheckWall(chase);
		camChaseSteerSide(chase, 0.0f);
		camChaseTiltGround(chase);
		camChaseTiltStick(chase);
		camChasePullIn(chase, 20.0f);
		camChaseTiltSide(chase);
		camChaseMove(chase);
		break;
	}
	case CHASE_EXIT:
		chase->mOffset[1] = chase->mDist;
		break;
	}
}

// 0x800980DC
void camChaseStateBehind(CamChase* chase, int event)
{
	switch (event) {
	case CHASE_ENTER:
		chase->mSub        = 0;
		chase->mTimer      = 0;
		chase->mSubRot     = camHeadGetRot(chase->mHead);
		chase->mFollowRate = 1.0f;
		break;
	case CHASE_CHECK:
		if (chase->mSub == 2 || (f32)chase->mTimer >= 24.0f) {
			camChaseSetState(chase, CHASE_FOLLOW);
		} else {
			camChaseCheckLock(chase);
		}
		break;
	case CHASE_UPDATE: {
		camCurveSetDrifting(chase->mCurve);
		if (chase->mHits & 0x3C) {
			chase->mTimer++;
		} else {
			chase->mTimer = 0;
		}
		if (chase->mPad != 0 && camPadHardPressed(chase->mPad) != 0) {
			chase->mSub    = 0;
			chase->mSubRot = camHeadGetRot(chase->mHead);
		}
		int angle;
		f32 vel;
		f32 sign;
		f32 limit;
		f32 dist;
		dist  = -chase->mOffset[1];
		sign  = camAngleSign(chase->mRot, chase->mSubRot, &angle);
		limit = camAngleToMove(dist, angle);
		if (chase->mSub == 0) {
			chase->mRotVel = sign * (0.18f * limit);
			chase->mSub    = 1;
		}
		vel = approachSpeed(chase->mRotVel * sign, limit, 0.15f, 0.09f, 0.18f, 13.0f, 0.2f);
		if (vel != 0.0f) {
			if (limit <= vel) {
				chase->mRot = chase->mSubRot;
				chase->mSub = 2;
				vel         = 0.0f;
			} else {
				camTurn(dist, vel, sign, &chase->mRot);
			}
		} else {
			chase->mSub = 2;
		}
		chase->mRotVel    = vel * sign;
		chase->mOffset[0] = chase->mHeight;
		chase->mOffset[1] = chase->mDist;
		camChaseLookGround(chase);
		camChaseReadStick(chase);
		camChaseCheckWall(chase);
		camChaseSteerSide(chase, 0.0f);
		camChaseTiltGround(chase);
		camChaseTiltStick(chase);
		camChasePullIn(chase, 20.0f);
		camChaseTiltSide(chase);
		camChaseMove(chase);
		break;
	}
	case CHASE_EXIT:
		chase->mFollowRate = 0.5f;
		break;
	}
}

// 0x8009837C
void camChaseStateLock(CamChase* chase, int event)
{
	switch (event) {
	case CHASE_ENTER:
		chase->mSub   = 0;
		chase->mTimer = 18;
		camLockB((u16)camHeadGetId(chase->mHead));
		break;
	case CHASE_CHECK:
		if (camChaseHardHeld(chase) == 0 || chase->mSub == 1) {
			camChaseSetState(chase, CHASE_FOLLOW);
		}
		break;
	case CHASE_UPDATE: {
		camCurveSetDrifting(chase->mCurve);
		u16 id = camHeadGetId(chase->mHead);
		if (chase->mTimer <= 0) {
			if (chase->mPad != 0 && camPadDown(chase->mPad) != 0) {
				camLockD(id);
			}
		} else {
			chase->mTimer--;
		}
		CamLockTarget target;
		camLockGetTarget(id, &target);
		if (findObjById(target.mId) != 0) {
			Vec3f at;
			camHeadGetPos(chase->mHead, &at);
			chase->mTarget   = target.mPos;
			chase->mToTarget = chase->mTarget;
			njSubVector(&chase->mToTarget, &at);
			int rot   = 10430.38043493439 * (f32)atan2(chase->mToTarget.x, chase->mToTarget.z);
			int angle;
			f32 sign;
			f32 dist  = -chase->mOffset[1];
			sign = camAngleSign(chase->mRot, rot, &angle);
			f32 limit = camAngleToMove(dist, angle);
			f32 vel   = chase->mRotVel * sign;
			f32 max = camMaxScaled(limit, 0.09f, 0.3f);
			vel = camAccel(vel, 0.4f, 0.8f, max);
			if (limit < vel) {
				chase->mRot = rot;
				vel         = 0.0f;
			} else {
				camTurn(dist, vel, sign, &chase->mRot);
			}
			chase->mRotVel = vel * sign;
		} else {
			chase->mSub = 1;
		}
		chase->mOffset[0] = chase->mHeight;
		chase->mOffset[1] = chase->mDist;
		camChaseCheckFollow(chase);
		camChaseReadStick(chase);
		camChaseCheckWall(chase);
		camChaseSteerSide(chase, 0.0f);

		Vec3f view;
		view.x = chase->mTarget.x;
		view.y = chase->mTarget.y;
		view.z = chase->mTarget.z;
		njCalcPointEx(&view, &view);
		f32 input = 0.0f;
		if (view.y > input && view.z < input && (-1.0f / view.z) * view.y > chase->mView->mSlope) {
			input = -1.0f;
		}
		camSteer(chase, input, &chase->mGround.mPos, &chase->mGround.mVel, &chase->mGround.mCount, 0, 120);
		camTilt(chase, chase->mGround.mPos, chase->mOffset, 0x1900, 0x200);
		camChaseTiltGround(chase);
		camChaseTiltStick(chase);
		camChasePullIn(chase, 20.0f);
		camChaseTiltSide(chase);
		camChaseMove(chase);
		break;
	}
	case CHASE_EXIT:
		camLockC((u16)camHeadGetId(chase->mHead));
		break;
	}
}

// 0x800986D0
void camChaseStateIdle(CamChase* chase, int event)
{
	switch (event) {
	case CHASE_ENTER:
		chase->mSub     = 0;
		chase->mSubStep = 0.1f;
		break;
	case CHASE_UPDATE:
		switch (chase->mSub) {
		case 0:
				camHeadGetStill(chase->mHead);
			break;
		}
		break;
	case CHASE_CHECK:
		break;
	case CHASE_EXIT:
		break;
	}
}
}
