#include "animal/AniAttacks.h"
#include "engine/Colli.h"
#include "egg/TEggCreatureTypeE.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "Ninja/ninja.h"
#include "engine/GroundQuery.h"
#include "object/TObject.h"
#include "animal/TAniRabitOwl.h"
struct ColliShape;

extern "C" {
MotionEntry gRacerMotions[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.14f, 0.0f, 0 }, { MOTION_LOOP, 1, -1, 0, 0.0f, 15.0f, 0.17f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 14.0f, 0.33f, 0.0f, 0 }, { MOTION_LOOP, 3, -1, 0, 0.0f, 14.0f, 0.77f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 14.0f, 0.18f, 0.0f, 0 }, { MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 14.0f, 0.33f, 0.0f, 0 }, { MOTION_ONCE, 1, -1, 0, 0.0f, 14.0f, 0.17f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 14.0f, 0.33f, 0.0f, 0 }, { MOTION_BLEND, 6, -1, 10, 0.0f, 14.0f, 0.3f, 0.075f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 39.0f, 0.38f, 0.0f, 0 },
};
ColliShape sRabitOwlFirstShape[] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TAniRabitOwl_size_check[(sizeof(TAniRabitOwl) == 0x3B8) ? 1 : -1];

const char* gTAniRabitOwlName = "TAniRabitOwl";

// 0x8019ABFC
TAniRabitOwl::TAniRabitOwl(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00    = (u32)gTAniRabitOwlName;
	mSize  = 0x3B8;
	mNodes = gRacerMotions;
	mShape = sRabitOwlFirstShape;
	initColli();
	m270       = sRabitOwlCallbacks;
	mWait      = 0;
	mTravelled = 0.0f;
	mSpeed     = 0.0f;
	mAccel     = 0.0f;
	mGrowth    = 0.0f;
	for (int i = 0; i < 8; i++) {
		mBombs[i] = 0;
	}
}

// 0x8019ACFC
TAniRabitOwl::~TAniRabitOwl()
{
	for (int i = 0; i < 8; i++) {
		if (mBombs[i] != 0) {
			mBombs[i]->killFx();
		}
	}
	killFx();
	if (mColli != 0) {
		colliFree(mColli);
		mColli = 0;
	}
}

// 0x8019ADB8
void TAniRabitOwl::onPhase(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(3, 1, 0.075f);
		eggTypeStopAllFx(this);
		startFollowers();
		mWait     = 0;
		mAniFlags = 0;
		mStep     = 0x1A;

		u8 shape[0x30];
		f32* values = mParams->mValues;
		if ((int)values[4] == 0) {
			Vec3f centre;
			centre.x = 0.0f;
			centre.y = 5.0f;
			centre.z = 0.0f;
			colliShapeMakeSphere((ColliShape*)shape, &centre, 0, values[5]);
		} else {
			colliShapeMakeCylinder((ColliShape*)shape, &gVecZero, 0, values[5], values[6]);
		}
		colliCopyShapes(mColli, (const ColliShape*)shape);
		colliShapeSetBit8(mColli, 0);
		colliGetShapes(mColli)->mSizeX = mParams->mValues[4];
		colliSetMasks(mColli, 0x20, 0xFF, 0xFF, 0x10);
		mtxSetTransV(mFxMatrix, &mPos);
		mFx = fxSpawnFollow(1, mFxBank, 1, mFxMatrix);
		sePlay3D(0x1BE, &mPos);
		mStartPos  = mPos;
		mRot[1]    = vecAngleXZ(&mDir);
		mTravelled = 0.0f;
		mSpeed     = 0.0f;
		mAccel     = 0.25f;
		mGrowth    = mParams->mValues[8];
		mSubMode   = 5;
	} else if (phase == 2) {
		switch (mStep) {
		case 0x1A:
			runFly();
			break;
		case 0x1B:
			runWait();
			break;
		case 0x1C:
			runBurst();
			break;
		}
		for (int i = 0; i < 8; i++) {
			if (mBombs[i] != 0 && mBombs[i]->mDone != 0) {
				mBombs[i]->killFx();
				if (mBombs[i] != 0) {
					mBombs[i]->mFlags |= 1;
					mBombs[i] = 0;
				}
			}
		}
	} else if (phase == 1) {
		mShape = sRabitOwlFirstShape;
		colliCopyShapes(mColli, mShape);
		colliShapeClearBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		killFx();
		eggTypeStopAllFx(this);
		clearExtraFx();
	}
}

/** a plus b returned by value */
static inline Vec3f owlAdd(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjAddVector(&v, a, b);
	return v;
}

/** v scaled by s returned by value */
static inline Vec3f owlScale(f32 s, const Vec3f* v)
{
	Vec3f r;
	gjScaleVector(s, &r, v);
	return r;
}

/** Adds v scaled by s to out */
static inline void owlAddScaled(Vec3f* out, f32 s, const Vec3f* v)
{
	Vec3f r = owlScale(s, v);
	njAddVector(out, &r);
}

/** a plus b through one more inline */
static inline Vec3f owlAhead(const Vec3f* a, const Vec3f* b)
{
	return owlAdd(a, b);
}

// 0x8019B078
void TAniRabitOwl::runFly()
{
	EggHits hits;
	Vec3f from = mPos;
	from.y += 10.0f;
	Vec3f to = owlAhead(&from, &mVel);
	if (meshColliProbeLine((EggHit*)&hits, (const Vec3f*)&from, (const Vec3f*)&to, (unsigned long)0x201) != 0 || touchedTarget() != 0
	    || mTravelled >= mParams->mValues[3]) {
		throwBombs();
		mWait = 0x3C;
		mStep = 0x1B;
		return;
	}
	mSpeed += mAccel;
	if (mSpeed > mParams->mValues[7]) {
		mSpeed = mParams->mValues[7];
	}
	owlAddScaled(&mPos, mSpeed, &mDir);
	Vec3f at = mPos;
	at.y += 5.0f;
	mtxSetTransV(mFxMatrix, &at);
	mTravelled += mSpeed;
}

// 0x8019B214
void TAniRabitOwl::runWait()
{
	if (mWait == 0) {
		killFx();
		fxSpawnAt(1, mFxBank, 6, mFxMatrix);
		notifyHolder();
		sePlay3D(0x1BE, &mPos);
		mWait = 0x3C;
		mStep = 0x1C;
	} else {
		mWait--;
	}
}

// 0x8019B2A4
void TAniRabitOwl::runBurst()
{
	if (mWait == 0) {
		if (objCountChildren(this) == 0) {
			endAttack();
		}
		return;
	}
	mWait--;
	if (mColli != 0) {
		colliGetShapes(mColli)->mSizeX += mGrowth;
	}
	mGrowth *= 0.8f;
}

// 0x8019B328
void TAniRabitOwl::throwBombs()
{
	Vec3f forward;
	forward.x = 0.0f;
	forward.y = 0.0f;
	forward.z = 1.0f;
	gjScaleVector(mParams->mValues[8], &forward, &forward);

	Vec3f vel;
	u8 shape[0x30];
	f32* values = mParams->mValues;
	if ((int)values[4] == 0) {
		Vec3f centre;
		centre.x = 0.0f;
		centre.y = 5.0f;
		centre.z = 0.0f;
		colliShapeMakeSphere((ColliShape*)shape, &centre, 0, values[5]);
	} else {
		colliShapeMakeCylinder((ColliShape*)shape, &gVecZero, 0, values[5], values[6]);
	}

	int angle = 0;
	int i     = 0;
	while (i < 8) {
		vecRotateY(&forward, &vel, angle);
		TAniFlushFlowerBomb* bomb = new TAniFlushFlowerBomb(this, mObjId, &mPos, mFxBank, (ColliShape*)shape);
		mBombs[i]                 = bomb;
		if (mBombs[i] != 0) {
			mBombs[i]->mVel    = vel;
			mBombs[i]->mGrowth = mGrowth;
		}
		i++;
		angle += 0x2000;
	}
}

// 0x8019B47C
int TAniRabitOwl::onHit(const TObjAction* other, int, int* result)
{
	return fillHitParam(other, result, 2);
}

static int sBone; // 0x802C59D0

// 0x8019B4F8
void TAniRabitOwl::resetBones()
{
	sBone = 0;
}

// 0x8019B504
extern "C" void rabitBones0()
{
	copyAnimalBones(6, 0, sBone);
	sBone++;
}

// 0x8019B59C
extern "C" void rabitBones1()
{
	copyAnimalBones(6, 1, sBone);
	sBone++;
}

// 0x8019B634
extern "C" void rabitBones2()
{
	copyAnimalBones(6, 2, sBone);
	sBone++;
}

// 0x8019B6CC
extern "C" void rabitBones3()
{
	copyAnimalBones(6, 3, sBone);
	sBone++;
}

// 0x8019B764
extern "C" void rabitBones4()
{
	copyAnimalBones(6, 4, sBone);
	sBone++;
}
