#include "animal/AniAttacks.h"
#include "animal/AniMount.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "engine/Colli.h"
#include "egg/TEggCreatureTypeE.h"
#include "object/TObjEggRing.h"
#include "sound/Sound3DPlay.h"
#include "Ninja/ninja.h"
#include "engine/GroundQuery.h"
#include "animal/TAniBullBoar.h"
#include "animal/TAniBigTackleBomb.h"
struct Fx;
struct ColliShape;

extern "C" {
MotionEntry gBullBoarMotions[9] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.2f, 0.0f, 0 },  { MOTION_LOOP, 1, -1, 0, 0.0f, 30.0f, 0.25f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 29.0f, 0.6f, 0.0f, 0 },  { MOTION_ONCE, 6, -1, 0, 0.0f, 19.0f, 0.25f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 20.0f, 0.35f, 0.0f, 0 }, { MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 19.0f, 0.8f, 0.0f, 0 },  { MOTION_ONCE, 1, -1, 0, 0.0f, 30.0f, 0.25f, 0.0f, 0 },
	{ MOTION_CHAIN, 7, -1, 6, 0.0f, 9.0f, 0.3f, 0.0f, 0 },
};
ColliShape sBullBoarShape0[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
MotionEntry sBullBoarMotions1[9] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.2f, 0.0f, 0 },  { MOTION_LOOP, 1, -1, 0, 0.0f, 9.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },  { MOTION_ONCE, 1, -1, 0, 0.0f, 9.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 39.0f, 0.45f, 0.0f, 0 }, { MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 19.0f, 0.7f, 0.0f, 0 },  { MOTION_ONCE, 1, -1, 0, 0.0f, 9.0f, 0.3f, 0.0f, 0 },
	{ MOTION_CHAIN, 3, -1, 6, 0.0f, 19.0f, 0.7f, 0.0f, 0 },
};
ColliShape sBullBoarShape1[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
MotionEntry sBullBoarMotions2[9] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.15f, 0.0f, 0 }, { MOTION_LOOP, 1, -1, 0, 0.0f, 9.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 19.0f, 0.6f, 0.0f, 0 },  { MOTION_ONCE, 1, -1, 0, 0.0f, 9.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 99.0f, 0.35f, 0.0f, 0 }, { MOTION_ONCE, 5, -1, 0, 0.0f, 56.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 19.0f, 0.7f, 0.0f, 0 },  { MOTION_ONCE, 1, -1, 0, 0.0f, 9.0f, 0.3f, 0.0f, 0 },
	{ MOTION_CHAIN, 3, -1, 6, 0.0f, 19.0f, 0.7f, 0.0f, 0 },
};
ColliShape sBullBoarShape2[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TAniBullBoar_size_check[(sizeof(TAniBullBoar) == 0x3F4) ? 1 : -1];

inline void TAniBullBoar::killAllFx()
{
	killFx();
	if (mFx2 != 0) {
		fxRemove(mFx2);
		mFx2 = 0;
	}
	if (mFx3 != 0) {
		fxRemove(mFx3);
		mFx3 = 0;
	}
}

extern "C" {
void bullBones0();
void bullBones1();
void bullBones2();
void bullBones3();
void bullBones4();
}

// The bone callbacks for kinds other than 0xB and 0x10
void* const sBullBoarCallbacks[8] = {
	(void*)bullBones0, (void*)bullBones1, (void*)bullBones2, (void*)bullBones3, (void*)bullBones4,
};

const char* gTAniBullBoarName = "TAniBullBoar";

// 0x8019CA74
TAniBullBoar::TAniBullBoar(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00   = (u32)gTAniBullBoarName;
	mSize = 0x3F4;
	if (kind == 0xB) {
		mNodes = sBullBoarMotions1;
		mShape = sBullBoarShape1;
	} else if (kind == 0x10) {
		mNodes = sBullBoarMotions2;
		mShape = sBullBoarShape2;
	} else {
		mNodes = gBullBoarMotions;
		mShape = sBullBoarShape0;
		m270   = sBullBoarCallbacks;
	}
	mColli = (CharaColli*)colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)mObjId);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		colliCopyShapes(mColli, mShape);
		colliPlaceShapes(mColli, &mPos, mShape);
	}
	mSpeed    = 0.0f;
	mTopSpeed = 0.0f;
	mFx2      = 0;
	mFx3      = 0;
	mtxIdentity(mFx2Matrix);
	mtxIdentity(mFx3Matrix);
}

// 0x8019CBA0
TAniBullBoar::~TAniBullBoar()
{
	killAllFx();
	seStop3D(&mSound);
	colliFree(mColli);
	mColli = 0;
}

static inline Vec3f scaledBy(const Vec3f* dir, f32 s)
{
	Vec3f v;
	gjScaleVector(s, &v, dir);
	return v;
}

static inline Vec3f addedTo(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjAddVector(&v, a, b);
	return v;
}

static inline Vec3f addedToRef(const Vec3f* a, const Vec3f& b)
{
	Vec3f v;
	gjAddVector(&v, a, &b);
	return v;
}

static inline Vec3f doubled(const Vec3f* v)
{
	return scaledBy(v, 2.0f);
}

static inline Vec3f lookAhead(const Vec3f* from, const Vec3f* vel)
{
	addedTo(from, vel);
	return addedToRef(from, doubled(vel));
}

static inline void scaleVel(Vec3f* vel, f32 s)
{
	Vec3f slow = scaledBy(vel, s);
	*vel       = slow;
}

static inline Vec3f baseScale(TAniBullBoar* self)
{
	Vec3f v;
	Vec3f one;
	one.x = 1.0f;
	one.y = 1.0f;
	one.z = 1.0f;
	gjScaleVector(self->mParams->mBaseScale, &v, &one);
	return v;
}

// 0x8019CC60
void TAniBullBoar::onPhase(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(3, 1, 0.075f);
		eggTypeStopAllFx(this);
		startFollowers();
		mTimer    = 0;
		mAniFlags = 0;
		mStep     = 0x1D;

		u8 shape[0x30];
		f32* values = mParams->mValues;
		if ((int)values[4] == 0) {
			Vec3f centre;
			centre.x = 0.0f;
			centre.y = 5.0f;
			centre.z = 0.0f;
			colliShapeMakeSphere((ColliShape*)shape, &centre, 1, values[5]);
		} else {
			colliShapeMakeCylinder((ColliShape*)shape, &gVecZero, 1, values[5], values[6]);
		}
		colliCopyShapes(mColli, (const ColliShape*)shape);
		colliShapeSetBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0xFF, 0xFF, 0x10);

		if (mKind == 0xB) {
			mtxCopy(mFxMatrix, mNodeMatrix);
			mFx = fxSpawnFollow(1, mFxBank, 3, mFxMatrix);
			mtxCopy(mFx2Matrix, mNodeMatrix);
			mFx2 = fxSpawnFollow(1, mFxBank, 6, mFx2Matrix);
			mtxCopy(mFxMatrix, mNodeMatrix);
			mFx3 = fxSpawnFollow(1, mFxBank, 9, mFx3Matrix);
			sePlay3DHeld(400.0f, &mSound, 0x7C, 0xC8, (const Vec3f*)&mPos);
		} else if (mKind == 8) {
			sePlay3D(0x9E, &mPos);
		}
		mSpeed    = mParams->mValues[8];
		mTopSpeed = mParams->mValues[7];
		mSubMode  = 5;
	} else if (phase == 2) {
		switch (mStep) {
		case 0x1D:
			runPaw();
			break;
		case 0x1E:
			runCharge();
			break;
		case 0x1F:
			runShrink();
			break;
		}
		if (mKind == 0xB) {
			mtxCopy(mFxMatrix, mNodeMatrix);
			mtxTranslate((float*)mFx2Matrix, (const float*)mNodeMatrix, 0.0f, 0.0f, 20.0f);
			mtxCopy(mFx3Matrix, mNodeMatrix);
		}
	} else if (phase == 1) {
		mScale = baseScale(this);
		if (mKind == 0xB) {
			killAllFx();
		}
		mShape = sBullBoarShape0;
		colliCopyShapes(mColli, mShape);
		colliShapeClearBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		eggTypeStopAllFx(this);
		clearExtraFx();
		seStop3D(&mSound);
	}
}

// 0x8019CFC4
void TAniBullBoar::runPaw()
{
	if (mPlayer.isDone() != 0) {
		playMotion(6, 1, 0.075f);
		Vec3f vel = scaledBy(&mDir, mSpeed);
		mVel      = vel;
		mTimer    = (int)(60.0f * mParams->mValues[3]);
		colliShapeSetBit8(mColli, 0);
		if (mKind == 8) {
			sePlay3DHeld(400.0f, &mSound, 0x1D1, 0xC8, (const Vec3f*)&mPos);
		} else if (mKind == 0xB) {
			seStop3D(&mSound);
			sePlay3D(0x203, &mPos);
		}
		mStep = 0x1E;
	}
	mRot[1] = vecAngleXZ(&mDir);
	if (mTimer == 0x28) {
		gjScaleVector(1.5f, &mScale, &mScale);
	} else if (mTimer == 0x32) {
		gjScaleVector(1.2f, &mScale, &mScale);
	} else if (mTimer == 0x37) {
		gjScaleVector(1.1f, &mScale, &mScale);
	}
	mTimer++;
}

// 0x8019D138
void TAniBullBoar::runCharge()
{
	if (mTimer == 0) {
		if (mKind == 8) {
			fxSpawnAt(1, mFxBank, 2, mNodeMatrix);
			new TAniBigTackleBomb(this, &mPos, mObjId);
			sePlay3D(0x1D2, &mPos);
		}
		notifyHolder();
		mStep = 0x1F;
		return;
	}
	mTimer--;
	if (mKind == 0x10 && mTimer % 5 == 0) {
		fxSpawnAt(1, mFxBank, 8, mNodeMatrix);
	}

	EggHits hits;
	if (mSpeed < mTopSpeed) {
		mSpeed += 0.2f;
	} else {
		mSpeed = mTopSpeed;
	}
	Vec3f vel;
	vel        = scaledBy(&mDir, mSpeed);
	vel.y      = 0.0f;
	f32 rise   = mVel.y;
	Vec3f from = mPos;
	from.y += 10.0f;
	Vec3f to = lookAhead(&from, &vel);
	rise += -0.1f;

	if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&from, (unsigned long)1) != 0 && hits.mHits[0].mNormal.y > 0.5f) {
		f32 ground = hits.mHits[0].mPos.y;
		if (!(ground - mPos.y < rise)) {
			rise   = 0.0f;
			mPos.y = ground;
		}
	}

	if (meshColliProbeLine((EggHit*)hits.mHits, (const Vec3f*)&from, (const Vec3f*)&to, (unsigned long)0x2201) != 0) {
		if (njUnitVector(&vel) < gUnitVectorMin) {
			vel = gVecZero;
		}
		vecReflect(&vel, &hits.mHits[0].mNormal, &vel);
		vel.y = 0.0f;
		if (njUnitVector(&vel) < gUnitVectorMin) {
			vel = gVecZero;
		}
		scaleVel(&vel, 0.8f * mSpeed);
		playMotion(8, 1, 1.0f);
	} else if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&to, (unsigned long)1) != 0) {
		if ((hits.mMask & 1) != 0 && mPos.y - hits.mHits[0].mPos.y >= 30.0f) {
			gjScaleVector(-0.8f, &vel, &vel);
			vel.y = 0.0f;
			playMotion(8, 1, 1.0f);
		}
	} else {
		gjScaleVector(-0.8f, &vel, &vel);
		vel.y = 0.0f;
		playMotion(8, 1, 1.0f);
	}

	mVel   = vel;
	mVel.y = rise;
	mDir   = vel;
	if (njUnitVector(&mDir) < gUnitVectorMin) {
		mDir = gVecZero;
	}
	njAddVector(&mPos, &mVel);
	mRot[1] = vecAngleXZ(&mVel);
}

// 0x8019D58C
void TAniBullBoar::runShrink()
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

// 0x8019D5F8
int TAniBullBoar::onHit(const TObjAction* other, int, int* result)
{
	return fillHitParam(other, result, 2);
}

static int sBone; // 0x802C59E0

// 0x8019D674
void TAniBullBoar::resetBones()
{
	sBone = 0;
}

// 0x8019D680
extern "C" void bullBones0()
{
	copyAnimalBones(8, 0, sBone);
	sBone++;
}

// 0x8019D718
extern "C" void bullBones1()
{
	copyAnimalBones(8, 1, sBone);
	sBone++;
}

// 0x8019D7B0
extern "C" void bullBones2()
{
	copyAnimalBones(8, 2, sBone);
	sBone++;
}

// 0x8019D848
extern "C" void bullBones3()
{
	copyAnimalBones(8, 3, sBone);
	sBone++;
}

// 0x8019D8E0
extern "C" void bullBones4()
{
	copyAnimalBones(8, 4, sBone);
	sBone++;
}

struct CharaColli;

const char* gTAniBigTackleBombName = "TAniBigTackleBomb";

// 0x8019D978
TAniBigTackleBomb::TAniBigTackleBomb(TObject* owner, const Vec3f* pos, u16 ownerId)
    : TObject(owner)
    , mPos(*pos)
{
	_00              = (u32)gTAniBigTackleBombName;
	mSize            = 0x40;
	mRadius          = 10.0f;
	mGrowth          = 0.0f;
	mTimer           = 0x3C;
	ColliShape shape = sBigTackleBombShape;
	shape.mSizeX     = mRadius;
	mColli           = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)ownerId);
		colliSetMasks((CharaColli*)mColli, 0x20, 0xFF, 0xFF, 0x10);
		colliCopyShapes((CharaColli*)mColli, &shape);
		colliPlaceShapes((CharaColli*)mColli, &mPos, &shape);
		colliShapeSetBit8((CharaColli*)mColli, 0);
	}
}

// 0x8019DA94
TAniBigTackleBomb::~TAniBigTackleBomb()
{
	colliFree((CharaColli*)mColli);
}

// 0x8019DAFC
void TAniBigTackleBomb::draw()
{
	mGrowth += 0.15f;
	mRadius += mGrowth;
	f32 r                          = mRadius;
	f32 size                       = r < 0.0f ? 0.0f : (r > 50.0f ? 50.0f : r);
	colliGetShapes(mColli)->mSizeX = size;
	if (mTimer == 0) {
		mFlags |= 1;
	} else {
		mTimer--;
	}
}

// 0x8019DBAC
void TAniBigTackleBomb::render()
{
}

// 0x8019DBB0
int TAniBigTackleBomb::vfn_20()
{
	return 4;
}

// 0x8019DBB8
const Vec3f* TAniBigTackleBomb::vfn_24()
{
	return &mPos;
}
