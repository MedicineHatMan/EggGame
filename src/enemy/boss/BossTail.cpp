#include "enemy/boss/TEneBoss02.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "animal/RacerMover.h"

typedef f32 TailMtx[12];

typedef char BossTail_size_check[(sizeof(BossTail) == 0x2C) ? 1 : -1];

// 0x800766EC
BossTail::BossTail(int count)
{
	mCount    = count;
	mMatrices = (f32*)new TailMtx[count];
	mExtra    = new HitVec[count];
	mPoints   = new HitVec[count];
	mPrev     = new HitVec[count];
	mVel      = new HitVec[count];
	mLengths  = new f32[count];
	for (int i = 0; i < count; i++) {
		njUnitMatrix(&mMatrices[i * 12]);
		mExtra[i].set(0.0f, 0.0f, 0.0f);
		mPoints[i].set(0.0f, 0.0f, 0.0f);
		mPrev[i].set(0.0f, 0.0f, 0.0f);
		mVel[i].set(0.0f, 0.0f, 0.0f);
		mLengths[i]  = 0.0f;
	}
	mBend = 0.0004f;
	mDrop = 1.0f;
	mSpin = 530.0f;
	mDamp = 0.99f;
}

// 0x80076898
BossTail::~BossTail()
{
	if (mLengths != 0) {
		delete[] mLengths;
		mLengths = 0;
	}
	if (mVel != 0) {
		delete[] mVel;
		mVel = 0;
	}
	if (mPrev != 0) {
		delete[] mPrev;
		mPrev = 0;
	}
	if (mPoints != 0) {
		delete[] mPoints;
		mPoints = 0;
	}
	if (mExtra != 0) {
		delete[] mExtra;
		mExtra = 0;
	}
	if (mMatrices != 0) {
		delete[] mMatrices;
		mMatrices = 0;
	}
}

// 0x8007698C
void BossTail::reset()
{
	for (int i = 0; i < mCount; i++) {
		mVel[i].set(0.0f, 0.0f, 0.0f);
	}
	savePrev();
}

// 0x800769E4
void BossTail::savePrev()
{
	for (int i = 0; i < mCount; i++) {
		mPrev[i] = mPoints[i];
	}
}

// 0x80076A30
// What each point moved this frame damped
void BossTail::updateVel()
{
	for (int i = 0; i < mCount; i++) {
		mVel[i] = mPoints[i];
		njSubVector(&mVel[i], &mPrev[i]);
		gjScaleVector(mDamp, &mVel[i], &mVel[i]);
	}
}

// 0x80076AC8
// Hangs point index from the one before
void BossTail::step(int index, Vec3f* dir)
{
	Vec3f pull;
	Vec3f link;
	Vec3f side;
	Vec3f bend;
	f32 sn;
	f32 cs;

	link = mPoints[index];
	njSubVector(&link, &mPrev[index - 1]);
	if (njUnitVector(&link) < gUnitVectorMin) {
		link = gVecZero;
	}

	pull = mPrev[index - 1];
	njSubVector(&pull, &mPoints[index - 1]);
	pull.y -= mDrop;
	njAddVector(&pull, &mVel[index]);

	int angle = 10430.38043493439 * (f32)acos(njInnerProduct(dir, &link));
	bend = *dir;
	njSubVector(&bend, &link);
	if (njUnitVector(&bend) < gUnitVectorMin) {
		bend = gVecZero;
	}
	gjScaleVector(angle * mBend, &bend, &bend);
	njAddVector(&pull, &bend);

	f32 along = njInnerProduct(&link, &pull);
	along *= -1.0f;
	gjMulAddVector(along, &side, &pull, &link);
	f32 len = njUnitVector(&side);
	if (len < gUnitVectorMin) {
		side = gVecZero;
	}
	njSinCos(len * mSpin, &sn, &cs);
	gjScaleVector(cs, &link, &link);
	gjMulAddVector(sn, &link, &link, &side);
	gjMulAddVector(mLengths[index], &mPoints[index], &mPoints[index - 1], &link);

	*dir = mPoints[index];
	njSubVector(dir, &mPoints[index - 1]);
	if (njUnitVector(dir) < gUnitVectorMin) {
		*dir = gVecZero;
	}
}

// 0x80076D84
void BossTail::update(const Vec3f* root, const Vec3f* dir)
{
	savePrev();
	mPoints[0] = *root;
	HitVec d;
	d = *dir;
	if (njUnitVector(&d) < gUnitVectorMin) {
		d = gVecZero;
	}
	for (int i = 1; i < mCount; i++) {
		step(i, &d);
	}
	updateVel();
}
