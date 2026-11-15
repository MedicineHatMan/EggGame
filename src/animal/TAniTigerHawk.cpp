#include "animal/AniAttacks.h"
#include "animal/AniMount.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "engine/Colli.h"
#include "egg/TEggCreatureTypeE.h"
#include "object/TObjEggRing.h"
#include "Ninja/ninja.h"
#include "engine/GroundQuery.h"
#include "sound/Sound3DPlay.h"
#include "animal/TAniTigerHawk.h"
struct ColliShape;
struct Fx;

extern "C" {
MotionEntry gTigerMotions[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.14f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 29.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 29.0f, 0.35f, 0.0f, 0 },
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 19.0f, 0.75f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 29.0f, 0.35f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 29.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 14.0f, 0.45f, 0.0f, 0 },
	{ MOTION_BLEND, 6, -1, 10, 0.0f, 19.0f, 0.5f, 0.075f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 39.0f, 0.52f, 0.0f, 0 },
};
ColliShape sTigerHawkFirstShape[] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TAniTigerHawk_size_check[(sizeof(TAniTigerHawk) == 0x3CC) ? 1 : -1];

extern "C" {
void tigerBones0();
void tigerBones1();
void tigerBones2();
void tigerBones3();
void tigerBones4();
}

void* const sTigerHawkCallbacks[] = {
	(void*)tigerBones0, (void*)tigerBones1, (void*)tigerBones2, (void*)tigerBones3, (void*)tigerBones4, 0, 0, 0,
};

const char* gTAniTigerHawkName = "TAniTigerHawk";

// 0x801BC880
TAniTigerHawk::TAniTigerHawk(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00    = (u32)gTAniTigerHawkName;
	mSize  = 0x3CC;
	mNodes = gTigerMotions;
	mShape = sTigerHawkFirstShape;
	initColli();
	m270       = sTigerHawkCallbacks;
	mTravelled = 0.0f;
	mSpeed     = 0.0f;
	mAccel     = 0.0f;
	_394       = 0;
	mFx2       = 0;
	mtxIdentity(mFx2Matrix);
}

// 0x801BC96C
TAniTigerHawk::~TAniTigerHawk()
{
	killFx();
	if (mFx2 != 0) {
		fxRemove((Fx*)mFx2);
		mFx2 = 0;
	}
	seStop3D(&mSound);
	if (mColli != 0) {
		colliFree(mColli);
		mColli = 0;
	}
}

// 0x801BCA1C
void TAniTigerHawk::onPhase(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(3, 1, 0.075f);
		eggTypeStopAllFx(this);
		startFollowers();
		_394  = 0;
		mAniFlags  = 0;
		mStep = 0x26;
		makeAttackShape(0, 0);
		sePlay3DHeld(400.0f, &mSound, 0x1D3, 0xC8, (const Vec3f*)&mPos);
		mtxCopy(mFxMatrix, mNodeMatrix);
		mFx        = fxSpawnFollow(1, mFxBank, 2, mFxMatrix);
		mFall      = -2.0f;
		mStartPos  = mPos;
		mRot[1]    = vecAngleXZ(&mDir);
		mRot[2]    = 0;
		mTravelled = 0.0f;
		mSpeed     = 0.0f;
		mAccel     = mParams->mValues[8];
		mSubMode   = 5;
	} else if (phase == 2) {
		switch (mStep) {
		case 0x26:
			runFly();
			break;
		case 0x27:
			runStep27();
			break;
		case 0x28:
			runEnd();
			break;
		}
		mtxCopy(mFxMatrix, mNodeMatrix);
	} else if (phase == 1) {
		mShape = sTigerHawkFirstShape;
		colliCopyShapes(mColli, mShape);
		colliShapeClearBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		killFx();
		if (mFx2 != 0) {
			fxRemove(mFx2);
			mFx2 = 0;
		}
		eggTypeStopAllFx(this);
		clearExtraFx();
		seStop3D(&mSound);
	}
}

static inline Vec3f scaledBy(const Vec3f* dir, f32 s)
{
	Vec3f v;
	gjScaleVector(s, &v, dir);
	return v;
}

static inline void flyVel(TAniTigerHawk* self)
{
	Vec3f along = scaledBy(&self->mDir, self->mSpeed);
	self->mVel = along;
}

// 0x801BCC8C
void TAniTigerHawk::runFly()
{
	f32 height;
	f32 ground;
	mSpeed += mAccel;
	if (mSpeed > mParams->mValues[7]) {
		mSpeed = mParams->mValues[7];
	}
	flyVel(this);
	mVel.y = mFall;
	EggHits hits;

	if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&mPos, (unsigned long)1) != 0 && hits.mHits[0].mNormal.y > 0.7f) {
		ground = hits.mHits[0].mPos.y;
		height = mPos.y - ground;
		if (!(height < 0.0f)) {
			if (height < 5.0f) {
				mPos.y = 5.0f + ground;
				mVel.y = 0.0f;
			} else if (height < 15.0f) {
				mFall *= 0.75f;
			}
		}
	}

	Vec3f to = mVel;
	to.y     = 0.0f;
	njAddVector(&to, &mPos);
	if (meshColliProbeLine((EggHit*)hits.mHits, (const Vec3f*)&mPos, (const Vec3f*)&to, (unsigned long)0x201) != 0 || touchedTarget() != 0 || mTravelled >= mParams->mValues[3]) {
		mAniFlags |= 0x2000;
		fxSpawnAt(1, mFxBank, 7, mNodeMatrix);
		new TAniLowFlyingBomb(this, &mPos, mObjId);
		seStop3D(&mSound);
		sePlay3D(0x1D4, &mPos);
		notifyHolder();
		mStep = 0x28;
	} else {
		njAddVector(&mPos, &mVel);
		mSound.mPos = mPos;
		mTravelled += mSpeed;
		if (height < 10.0f) {
			Vec3f at = mPos;
			at.y     = 0.5f + ground;
			mtxSetTransV(mFx2Matrix, &at);
			mtxRotateY(mFx2Matrix, mFx2Matrix, mRot[1]);
			if (mFx2 == 0) {
				mFx2 = fxSpawnFollow(1, mFxBank, 5, mFx2Matrix);
			}
		} else if (mFx2 != 0) {
			fxRemove(mFx2);
			mFx2 = 0;
		}
	}
}

// 0x801BCF7C
void TAniTigerHawk::runStep27()
{
}

// 0x801BCF80
void TAniTigerHawk::runEnd()
{
	endAttack();
}

// 0x801BCFA0
int TAniTigerHawk::onHit(const TObjAction* other, int, int* result)
{
	return fillHitParam(other, result, 2);
}

static int sBone; // 0x802C5B20

// 0x801BD01C
void TAniTigerHawk::resetBones()
{
	sBone = 0;
}

// 0x801BD028
extern "C" void tigerBones0()
{
	copyAnimalBones(10, 0, sBone);
	sBone++;
}

// 0x801BD0C0
extern "C" void tigerBones1()
{
	copyAnimalBones(10, 1, sBone);
	sBone++;
}

// 0x801BD158
extern "C" void tigerBones2()
{
	copyAnimalBones(10, 2, sBone);
	sBone++;
}

// 0x801BD1F0
extern "C" void tigerBones3()
{
	copyAnimalBones(10, 3, sBone);
	sBone++;
}

// 0x801BD288
extern "C" void tigerBones4()
{
	copyAnimalBones(10, 4, sBone);
	sBone++;
}

#include "animal/AniAttacks.h"
#include "engine/Colli.h"
#include "animal/TAniLowFlyingBomb.h"
struct CharaColli;

const char* gTAniLowFlyingBombName = "TAniLowFlyingBomb";

const ColliShape sLowFlyingBombShape = { 0 };

// 0x801BD320
TAniLowFlyingBomb::TAniLowFlyingBomb(TObject* owner, const Vec3f* pos, u16 ownerId)
    : TObject(owner)
    , mPos(*pos)
{
	_00     = (u32)gTAniLowFlyingBombName;
	mSize   = 0x40;
	mRadius = 10.0f;
	mGrowth = 0.0f;
	mTimer  = 0x5A;
	ColliShape shape = sLowFlyingBombShape;
	shape.mSizeX     = mRadius;
	mColli = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)ownerId);
		colliSetMasks((CharaColli*)mColli, 0x20, 0xFF, 0xFF, 0x10);
		colliCopyShapes((CharaColli*)mColli, &shape);
		colliPlaceShapes((CharaColli*)mColli, &mPos, &shape);
		colliShapeSetBit8((CharaColli*)mColli, 0);
	}
}

// 0x801BD43C
TAniLowFlyingBomb::~TAniLowFlyingBomb()
{
	colliFree((CharaColli*)mColli);
}

// 0x801BD4A4
void TAniLowFlyingBomb::draw()
{
	mGrowth += 0.15f;
	mRadius += mGrowth;
	f32 r = mRadius;
	f32 size = r < 0.0f ? 0.0f : (r > 70.0f ? 70.0f : r);
	colliGetShapes(mColli)->mSizeX = size;
	if (mTimer == 0) {
		mFlags |= 1;
	} else {
		mTimer--;
	}
}

// 0x801BD554
void TAniLowFlyingBomb::render()
{
}

// 0x801BD558
int TAniLowFlyingBomb::vfn_20()
{
	return 4;
}

// 0x801BD560
const Vec3f* TAniLowFlyingBomb::vfn_24()
{
	return &mPos;
}
