#include "egg/EggCreature.h"
#include "engine/Colli.h"
#include "io/ResBank.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "egg/TEggCreatureTypeE.h"
#include "sound/Sound3DPlay.h"
#include "engine/GroundQuery.h"
#include "engine/FxSystem.h"
#include "animal/TAniKumaKame.h"
struct ColliShape;

extern "C" {
// 0x802C4388
// the motion rates the static init writes into gKumaMotions
f32 sKumaKameRate0 = 0.18f;
f32 sKumaKameRate1 = 0.27f;
f32 sKumaKameRate2 = 0.8f;
f32 sKumaKameRate3 = 0.4f;
f32 sKumaKameRate4 = 0.32f;

MotionEntry gKumaMotions[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 15.0f, sKumaKameRate0, 0.0f, 0 }, { MOTION_LOOP, 1, -1, 0, 0.0f, 30.0f, sKumaKameRate1, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, sKumaKameRate2, 0.0f, 0 }, { MOTION_ONCE, 3, -1, 0, 0.0f, 30.0f, sKumaKameRate3, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 20.0f, sKumaKameRate4, 0.0f, 0 }, { MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, sKumaKameRate2, 0.0f, 0 }, { MOTION_ONCE, 1, -1, 0, 0.0f, 30.0f, sKumaKameRate1, 0.0f, 0 },
};
ColliShape sKumaKameFirstShape[] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
extern const f32 sKumaHopHeights[] = {
	50.0f,
	50.0f,
	100.0f,
}; // the height of each hop
extern const int sKumaHopWaits[] = {
	0,
	0,
	15,
}; // the frames to wait after each hop
extern const int sKumaHopEffects[] = {
	4,
	2,
	6,
}; // the effect for each landing
}

typedef char TAniKumaKame_size_check[(sizeof(TAniKumaKame) == 0x38C) ? 1 : -1];

const char* gTAniKumaKameName = "TAniKumaKame";

// 0x801314F0
TAniKumaKame::TAniKumaKame(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00    = (u32)gTAniKumaKameName;
	mSize  = 0x38C;
	mNodes = gKumaMotions;
	mShape = sKumaKameFirstShape;
	initColli();
	m270      = sKumaKameCallbacks;
	mHops     = 0;
	mDistance = 0.0f;
}

// 0x801315C8
TAniKumaKame::~TAniKumaKame()
{
	colliFree(mColli);
	mColli = 0;
}

/** The params' base scale on all three axes returned by value */
static inline Vec3f baseScale(TAniKumaKame* self)
{
	Vec3f v;
	Vec3f one;
	one.x = 1.0f;
	one.y = 1.0f;
	one.z = 1.0f;
	gjScaleVector(self->mParams->mBaseScale, &v, &one);
	return v;
}

/** The offset from b to a returned by value */
static inline Vec3f subtracted(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjSubVector(&r, a, b);
	return r;
}

// 0x80131638
void TAniKumaKame::onPhase(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(3, 1, 0.075f);
		mTimer    = 0;
		mAniFlags = 0;
		mStep     = 0x14;
		makeAttackShape(1, 1);
		mDistance = 0.0f;

		u32 id = findEggAhead(getPos(), getRotY(), mHolderId, 0x2000, 200.0f);
		if ((u16)id != 0xFFFF) {
			TObjAction* target = findObjById(id);
			mDir               = subtracted(target->getPos(), getPos());
			f32 length         = njUnitVector(&mDir);
			if (length < 1e-6f) {
				mDir = gVecZero;
			}
			mDistance = length;
		}
		mRot[1]   = vecAngleXZ(&mDir);
		mAniFlags = 0;
		mHops     = 0;
		mSubMode  = 5;
	} else if (phase == 2) {
		switch (mStep) {
		case 0x13:
			runWait();
			break;
		case 0x14:
			runRise();
			break;
		case 0x15:
			runFall();
			break;
		case 0x16:
			runShrink();
			break;
		}
	} else if (phase == 1) {
		mScale = baseScale(this);
		mShape = sKumaKameFirstShape;
		colliCopyShapes(mColli, mShape);
		colliShapeClearBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		eggTypeStopAllFx(this);
		clearExtraFx();
	}
}

// 0x80131940
void TAniKumaKame::runWait()
{
	if (mTimer == 0) {
		if (mPlayer.isDone() == 0) {
			return;
		}
		if (mHops == 3) {
			mStep = 0x16;
			return;
		}
		mPlayer.play(3);
		mAniFlags &= ~0x1000;
		mVel.x                         = 0.0f;
		mVel.y                         = 0.0f;
		mVel.z                         = 0.0f;
		colliGetShapes(mColli)->mSizeX = mParams->mValues[5];
		sePlay3D(0x1BB, &mPos);
		mStep = 0x14;
	} else {
		mTimer--;
	}
}

// 0x80131A0C
void TAniKumaKame::runRise()
{
	if (mPlayer.passedFrame(3.0f) != 0 && (mAniFlags & 1) == 0) {
		sePlay3D(0x1BB, &mPos);
		Vec3f vel;
		gjScaleVector(mParams->mValues[7], &vel, &mDir);
		mVel   = vel;
		mVel.y = 0.0653333f * (sKumaHopHeights[mHops] / mParams->mValues[7]);
		mAniFlags &= ~4;
		mAniFlags |= 1;
	}
	if (mPlayer.passedFrame(26.0f) != 0) {
		mAniFlags |= 2;
	}
	if ((mAniFlags & 1) != 0 && (mAniFlags & 0x800) == 0) {
		mVel.y += -0.0653333f;
		if (mVel.y < 0.0f) {
			mVel.x = 0.0f;
			mVel.y = 0.0f;
			mVel.z = 0.0f;
			mTimer = sKumaHopWaits[mHops];
			mAniFlags |= 0x800;
		}
	}

	EggHits hits;
	EggProbe hit;
	Vec3f from = mPos;
	from.y += 10.0f;
	Vec3f sum;
	gjAddVector(&sum, &from, &mVel);
	Vec3f to = sum;
	mAniFlags &= ~4;
	if (meshColliProbeLine((EggHit*)&hits, (const Vec3f*)&from, (const Vec3f*)&to, (unsigned long)0x200) != 0) {
		hit      = *(EggProbe*)&hits.mHits[0];
		f32 dist = *(f32*)hit._3C;
		f32 size = (mShape)->mSizeX;
		if (dist < size) {
			Vec3f push;
			gjScaleVector(size - dist, &push, &hit.mNormal);
			mVel = push;
		}
	}

	Vec3f ahead;
	gjScaleVector(20.0f, &ahead, &mDir);
	Vec3f at;
	Vec3f step = ahead;
	gjAddVector(&at, &mPos, &step);
	from = at;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&from, (unsigned long)1) == 0) {
		mVel.z = 0.0f;
		mVel.x = 0.0f;
	}
	if ((mAniFlags & 0x800) != 0 && (mAniFlags & 2) != 0) {
		mHops++;
		mStep = 0x15;
	}
	njAddVector(&mPos, &mVel);
}

// 0x80131DF4
void TAniKumaKame::runFall()
{
	if ((mAniFlags & 0x800) != 0) {
		if (mHops == 3) {
			if (mScale.x >= 2.5f) {
				mScale.x = 2.5f;
				mScale.y = 2.5f;
				mScale.z = 2.5f;
				if (mTimer-- == 0) {
					mAniFlags &= ~0x800;
				}
			} else {
				gjScaleVector(1.1f, &mScale, &mScale);
			}
			colliGetShapes(mColli)->mSizeX = mParams->mValues[5] * mScale.x;
			colliGetShapes(mColli)->mSizeY = mParams->mValues[6] * mScale.y;
		} else if (mTimer-- == 0) {
			mAniFlags &= ~0x800;
		}
		return;
	}

	if (mPlayer.passedFrame(26.0f) != 0) {
		mAniFlags |= 2;
	}
	mVel.y += -0.326667f;

	EggHits hits;
	EggProbe hit;
	Vec3f from = mPos;
	from.y += 10.0f;
	mAniFlags &= ~4;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&from, (unsigned long)1) != 0) {
		hit      = *(EggProbe*)&hits.mHits[0];
		f32 drop = hit.mPos.y - mPos.y;
		if (drop >= mVel.y) {
			mVel.y = drop;
			mAniFlags &= ~1;
			mAniFlags &= ~2;
			mAniFlags |= 4;
			colliGetShapes(mColli)->mSizeX *= 2.0f;
			if ((mAniFlags & 0x1000) == 0) {
				Vec3f landed;
				gjAddVector(&landed, &mPos, &mVel);
				Vec3f at = landed;
				at.y += 0.5f;
				mtxSetTransV(mFxMatrix, &at);
				fxSpawnAt(1, mFxBank, sKumaHopEffects[mHops - 1], mFxMatrix);
				mAniFlags |= 0x1000;
			}
			if (mHops == 3) {
				notifyHolder();
			}
			sePlay3D(0x1BC, &mPos);
			mTimer = 6;
			mStep  = 0x13;
		}
	}
	mRot[1] = vecAngleXZ(&mDir);
	njAddVector(&mPos, &mVel);
}

// 0x80132190
void TAniKumaKame::runShrink()
{
	f32 base = mParams->mBaseScale;
	if (mScale.x <= base) {
		mScale.x = base;
		mScale.y = base;
		mScale.z = base;
		endAttack();
	} else {
		mScale.x -= 0.1f;
		mScale.y -= 0.1f;
		mScale.z -= 0.1f;
	}
}

// 0x801321FC
int TAniKumaKame::onHit(const TObjAction* other, int, int* result)
{
	return fillHit(other, result, 9);
}

static int sBone; // 0x802C5678

// 0x80132270
void TAniKumaKame::resetBones()
{
	sBone = 0;
}

// 0x8013227C
extern "C" void kumaBones0()
{
	copyAnimalBones(4, 0, sBone);
	sBone++;
}

// 0x80132314
extern "C" void kumaBones1()
{
	copyAnimalBones(4, 1, sBone);
	sBone++;
}

// 0x801323AC
extern "C" void kumaBones2()
{
	copyAnimalBones(4, 2, sBone);
	sBone++;
}

// 0x80132444
extern "C" void kumaBones3()
{
	copyAnimalBones(4, 3, sBone);
	sBone++;
}

// 0x801324DC
extern "C" void kumaBones4()
{
	copyAnimalBones(4, 4, sBone);
	sBone++;
}
