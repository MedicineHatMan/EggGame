#include "engine/SpringBody.h"
#include "engine/FlagCloth.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"

static inline Vec3f scaled(const Vec3f& v, f32 scale)
{
	Vec3f out;
	gjScaleVector(scale, &out, &v);
	return out;
}

// 0x800A8FA4
SpringBody::SpringBody(u16 count, u16 pins, int flags, f32 spring, f32 loss, f32 param)
{
	init(count, pins, flags, spring, loss, param);
}

// 0x800A8FE0
void SpringBody::init(u16 count, u16 pins, int flags, f32 spring, f32 loss, f32 param)
{
	mCapacity     = count;
	mPoints       = allocPoints(mCapacity);
	mPinCapacity  = pins;
	mPins         = allocPins(mPinCapacity);
	mCount        = 0;
	mPinCount     = 0;
	mWind.x       = 0.0f;
	mWind.y       = 0.0f;
	mWind.z       = 0.0f;
	mReact        = 0;
	mSpherePos.x  = 0.0f;
	mSpherePos.y  = 0.0f;
	mSpherePos.z  = 0.0f;
	mSphereRadius = 0.0f;
	mSphereVel.x  = 0.0f;
	mSphereVel.y  = 0.0f;
	mSphereVel.z  = 0.0f;
	mSpring       = spring;
	mLoss         = loss;
	mParam        = param;
	mHit          = flags;
}

// 0x800A90B8
SpringBody::~SpringBody()
{
	freeArray(mPins);
	freeArray(mPoints);
}

// 0x800A9118
SpringPoint* SpringBody::allocPoints(int count)
{
	return (SpringPoint*)memCalloc(sizeof(SpringPoint), count);
}

// 0x800A9140
SpringPin* SpringBody::allocPins(int count)
{
	return (SpringPin*)memCalloc(sizeof(SpringPin), count);
}

// 0x800A9168
void SpringBody::freeArray(void* array)
{
	memFree(array);
}

// 0x800A9188
void SpringBody::step()
{
	addSprings();
	move();
	holdPins();
	mReact = 0;
	if (mHit & 1) {
		collide();
	}
}

// 0x800A91EC
void SpringBody::addSprings()
{
	u16 j;
	u16 i;
	SpringPoint* point;
	for (i = 0; i < mCount; i++) {
		point           = &mPoints[i];
		point->mForce.x = 0.0f;
		point->mForce.y = 0.0f;
		point->mForce.z = 0.0f;
		for (j = 0; j < point->mLinkCount; j++) {
			f32 spring = mSpring;
			f32 rest   = point->mRest[j];
			Vec3f pull;
			vecSub(&pull, &point->mLink[j]->mPos, &point->mPos);
			f32 length = vecLength(&pull);
			if (length < 5e-6f) {
				pull.x = 0.0f;
				pull.y = 0.0f;
				pull.z = 0.0f;
			} else {
				vecScale(spring * (1.0f - rest / length), &pull, &pull);
			}
			vecAdd(&point->mForce, &point->mForce, &pull);
		}
	}
}

// 0x800A9320
void SpringBody::move()
{
	SpringPoint* point;
	u16 i;
	for (i = 0; i < mCount; i++) {
		point = &mPoints[i];
		Vec3f force;
		getForce(i, &force);
		vecAdd(&force, &force, &mWind);
		vecAdd(&force, &force, &point->mForce);
		vecAdd(&point->mVel, &point->mVel, &force);
		vecAdd(&point->mPos, &point->mPos, &point->mVel);
		vecScale(mLoss, &point->mVel, &point->mVel);
	}
}

// 0x800A93E0
void SpringBody::getForce(u16 index, Vec3f* out)
{
	out->x = 0.0f;
	out->y = 0.0f;
	out->z = 0.0f;
}

// 0x800A93F4
void SpringBody::holdPins()
{
	SpringPin* pin;
	u16 i;
	for (i = 0; i < mPinCount; i++) {
		pin               = &mPins[i];
		pin->mPoint->mPos = pin->mPos;
	}
}

// Shoves the points inside the sphere out to its skin and gives them its speed
// 0x800A943C
void SpringBody::collide()
{
	for (u16 i = 0; i < mCount; i++) {
		Vec3f gap;
		vecSub(&gap, &mPoints[i].mPos, &mSpherePos);
		if (vecLength(&gap) < mSphereRadius) {
			vecNormalize(&gap, &gap);
			vecScale(0.5f + mSphereRadius, &gap, &gap);
			vecAdd(&mPoints[i].mPos, &mSpherePos, &gap);
			mPoints[i].mVel = mSphereVel;
			mReact          = 1;
		}
	}
}

// 0x800A9514
void SpringBody::reset()
{
	syMemSet(mPoints, 0, mCount * sizeof(SpringPoint));
	mCount = 0;
}

// 0x800A9558
int SpringBody::addPin(SpringPoint* point)
{
	if (mPinCount < mPinCapacity) {
		mPins[mPinCount].mPoint = point;
		mPins[mPinCount].mPos   = mPins[mPinCount].mPoint->mPos;
		mPinCount++;
		return 1;
	}
	return 0;
}

// 0x800A95C0
void SpringBody::setSphere(const Vec3f* pos, Vec3f* vel, f32 radius)
{
	mSpherePos    = *pos;
	mSphereRadius = radius;
	mSphereVel.x  = vel->x;
	mSphereVel.y  = vel->y;
	mSphereVel.z  = vel->z;
}

// 0x800A95F8
SpringRope::SpringRope(Vec3f* from, const Vec3f* to, u16 segments, f32 spring, f32 loss, f32 param)
    : SpringBody(segments + 1, 8, 0, spring, loss, param)
{
	mCount = segments + 1;
	Vec3f step;
	vecSub(&step, to, from);
	gjScaleVector(1.0f / segments, &step, &step);
	f32 length = njScalor(&step);
	Vec3f pos  = *from;
	for (u16 i = 0; i < mCount; i++) {
		mPoints[i].mPos = pos;
		vecAdd(&pos, &pos, &step);
	}

	SpringPoint* end = mPoints;
	end->mLink[0]    = &mPoints[1];
	end->mRest[0]    = length;
	end->mLinkCount  = 1;
	end              = &mPoints[mCount - 1];
	end->mLink[0]    = &mPoints[mCount - 2];
	end->mRest[0]    = length;
	end->mLinkCount  = 1;
	for (u16 i = 1; i < mCount - 1; i++) {
		SpringPoint* point = &mPoints[i];
		point->mLink[0]    = &mPoints[i - 1];
		point->mRest[0]    = length;
		point->mLink[1]    = &mPoints[i + 1];
		point->mRest[1]    = length;
		point->mLinkCount  = 2;
	}
	addPin(mPoints);
	addPin(&mPoints[mCount - 1]);
}

// 0x800A97F0
SpringRope::~SpringRope()
{
}

// 0x800A9848
void SpringRope::step()
{
	SpringBody::step();
}

// 0x800A9868
void SpringRope::reset()
{
	SpringBody::reset();
}

// 0x800A9888
void SpringRope::getForce(u16 index, Vec3f* out)
{
	out->x = 0.0f;
	out->y = 0.0f;
	out->z = 0.0f;
	*out   = mForces[index];
}

// 0x800A98C4
FlagCloth::FlagCloth(ClothMatrix base, u16 cols, u16 rows, f32 width, f32 height, f32 spring, f32 loss, f32 param)
    : SpringBody((cols + 1) * (rows + 1), 16, 0, spring, loss, param)
{
	mWindDir.x = 0.0f;
	mWindDir.y = 0.0f;
	mWindDir.z = 0.0f;
	mGust.x    = 0.0f;
	mGust.y    = 0.0f;
	mGust.z    = 0.0f;
	mSway.x    = 0.0f;
	mSway.y    = 0.0f;
	mSway.z    = 0.0f;
	mSwaySpeed = 0x1000;
	mSwayStep  = mSwaySpeed;
	mSwayAngle = 0;
	build(&base, cols, rows, width, height);
}

// 0x800A99B4
FlagCloth::~FlagCloth()
{
}

// Lays the points out on the matrix and links each to its neighbours across up and down at their spacing and corner to
// corner at the diagonal
// 0x800A9A0C
void FlagCloth::build(const ClothMatrix* base, u16 cols, u16 rows, f32 width, f32 height)
{
	f32 diag;
	f32 across;
	f32 down;
	u32 y;
	u32 x;
	u32 i;

	mBase   = *base;
	mWidth  = width;
	mHeight = height;
	mCols   = cols + 1;
	mRows   = rows + 1;
	across  = width / cols;
	down    = height / rows;
	mCount  = mCols * mRows;
	diag    = across * across + down * down;
	if (diag > 0.0f) {
		f64 guess         = __frsqrte((f64)diag);
		guess             = .5 * guess * (3.0 - guess * guess * diag);
		guess             = .5 * guess * (3.0 - guess * guess * diag);
		guess             = .5 * guess * (3.0 - guess * guess * diag);
		volatile f32 root = (f32)(diag * guess);
		diag              = root;
	}

	for (i = 0; i < mCount; i++) {
		SpringPoint* point;
		u32 n         = mCols;
		y             = i / n;
		x             = i - y * n;
		point         = &mPoints[i];
		point->mPos.x = across * x;
		point->mPos.y = down * y;
		point->mPos.z = 0.0f;
		mtxMultVec(base->m, &mPoints[i].mPos, &mPoints[i].mPos);

		int left;
		int top;
		int right;
		int bottom;
		bottom = 0;
		right  = 0;
		top    = 0;
		left   = 0;
		if (x != 0) {
			addLink(i, i - 1, across);
			left = 1;
		}
		if (y != 0) {
			addLink(i, i - mCols, down);
			top = 1;
		}
		if (x < mCols - 1) {
			addLink(i, i + 1, across);
			right = 1;
		}
		if (y < mRows - 1) {
			addLink(i, i + mCols, down);
			bottom = 1;
		}
		if (left && top) {
			addLink(i, i - mCols - 1, diag);
		}
		if (right && bottom) {
			addLink(i, i + mCols + 1, diag);
		}
		if (left && bottom) {
			addLink(i, i + mCols - 1, diag);
		}
		if (top && right) {
			addLink(i, i - mCols + 1, diag);
		}
	}
}

// 0x800A9F30
void FlagCloth::reset()
{
	rebuild(&mBase);
}

// 0x800A9F54
void FlagCloth::rebuild(const ClothMatrix* base)
{
	SpringBody::reset();
	build(base, mCols - 1, mRows - 1, mWidth, mHeight);
}

// The gust is a fifth of the wind plus the rest of it turned a quarter round and swung by a sine
// 0x800A9FB0
void FlagCloth::step()
{
	f32 m[12];
	mSway = mWindDir;
	mGust = mSway;
	gjScaleVector(0.2f, &mGust, &mGust);
	gjScaleVector(0.8f, &mSway, &mSway);
	mtxSetRotY(m, 0x4000);
	mtxMultVec(m, &mSway, &mSway);
	gjScaleVector(njSin(mSwayAngle), &mSway, &mSway);
	vecAdd(&mGust, &mGust, &mSway);
	mSwayAngle += mSwayStep;
	SpringBody::step();
}

// 0x800AA080
void FlagCloth::getForce(u16 index, Vec3f* out)
{
	*out = mGust;
}

// 0x800AA09C
void FlagCloth::collide()
{
	SpringBody::collide();
}

// 0x800AA0BC
void FlagCloth::pin(u16 x, u16 y)
{
	addPin(point(x, y));
}

// 0x800AA0F4
void FlagCloth::setWind(const Vec3f* dir, s16 speed, f32 strength)
{
	mWindDir   = scaled(scaled(scaled(*dir, strength), 1.0f / 60.0f), 10.0f);
	mSwaySpeed = mSwayStep = speed;
}

// 0x800AA1B0
SpringPoint* FlagCloth::point(u16 x, u16 y)
{
	return &mPoints[x + y * mCols];
}
