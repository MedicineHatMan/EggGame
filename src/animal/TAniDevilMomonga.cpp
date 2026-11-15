#include "animal/RacerMover.h"
#include "animal/AniAttacks.h"
#include "io/ResBank.h"
#include "core/MathUtil.h"
#include "enemy/boss/EneBoss.h"
#include "engine/FxSystem.h"
#include "engine/Colli.h"
#include "camera/TCamPath.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "object/ObjShadow.h"
#include "egg/TEggCreatureTypeE.h"
#include "sound/Sound3DPlay.h"
#include "object/TObjEggRing.h"
#include "engine/GroundQuery.h"
#include "animal/AniMount.h"
#include "animal/TAniDevilMomonga.h"
struct DrawStateSlot;
struct DrawState;
struct Fx;
struct ColliShape;
struct NJS_OBJECT;

extern "C" {
MotionEntry gMomongaMotions[11] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.15f, 0.0f, 0 }, { MOTION_LOOP, 1, -1, 0, 0.0f, 19.0f, 0.31f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 20.0f, 0.31f, 0.0f, 0 }, { MOTION_LOOP, 3, -1, 0, 0.0f, 29.0f, 1.3f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 19.0f, 0.6f, 0.0f, 0 },  { MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 19.0f, 0.31f, 0.0f, 0 }, { MOTION_ONCE, 1, -1, 0, 0.0f, 19.0f, 0.31f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 19.0f, 0.6f, 0.0f, 0 },  { MOTION_BLEND, 6, -1, 10, 0.0f, 19.0f, 0.6f, 0.075f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 29.0f, 1.3f, 0.0f, 0 },
};
ColliShape sMomongaShape0[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
MotionEntry sMomongaMotions1[11] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.2f, 0.0f, 0 },  { MOTION_LOOP, 1, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },  { MOTION_LOOP, 3, -1, 0, 0.0f, 19.0f, 0.7f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 19.0f, 0.25f, 0.0f, 0 }, { MOTION_ONCE, 6, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },  { MOTION_ONCE, 1, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },  { MOTION_BLEND, 7, -1, 10, 0.0f, 19.0f, 0.3f, 0.075f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 19.0f, 0.6f, 0.0f, 0 },
};
ColliShape sMomongaShape1[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
MotionEntry sMomongaMotions2[11] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.15f, 0.0f, 0 }, { MOTION_LOOP, 1, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },  { MOTION_LOOP, 3, -1, 0, 0.0f, 19.0f, 1.5f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },  { MOTION_ONCE, 4, -1, 0, 0.0f, 56.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },  { MOTION_ONCE, 1, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 29.0f, 0.25f, 0.0f, 0 }, { MOTION_BLEND, 5, -1, 10, 0.0f, 29.0f, 0.3f, 0.075f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },
};
ColliShape sMomongaShape2[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TAniDevilMomonga_size_check[(sizeof(TAniDevilMomonga) == 0x408) ? 1 : -1];

inline void TAniDevilMomonga::killAllFx()
{
	killFx();
	if (mFx2 != 0) {
		fxRemove(mFx2);
		mFx2 = 0;
	}
}

extern "C" {
void momongaBones0();
void momongaBones1();
void momongaBones2();
void momongaBones3();
void momongaBones4();
}

void* const sDevilMomongaCallbacks[9] = {
	(void*)momongaBones0, (void*)momongaBones1, (void*)momongaBones2, (void*)momongaBones3, (void*)momongaBones4,
};

const char* gTAniDevilMomongaName = "TAniDevilMomonga";

// 0x801A3EE0
TAniDevilMomonga::TAniDevilMomonga(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00   = (u32)gTAniDevilMomongaName;
	mSize = 0x408;
	if (kind == 0xC) {
		mNodes = sMomongaMotions1;
		mShape = sMomongaShape1;
	} else if (kind == 0x11) {
		mNodes = sMomongaMotions2;
		mShape = sMomongaShape2;
	} else {
		mNodes = gMomongaMotions;
		mShape = sMomongaShape0;
		m270   = sDevilMomongaCallbacks;
	}
	initColli();
	mImageWait = 0;
	mTravelled = 0.0f;
	mSpeed     = 0.0f;
	mTopSpeed  = 0.0f;
	mTurnRate  = 0;
	mBob       = 0.0f;
	mBobAngle  = 0;
	mtxIdentity(mSpinMatrix);
	mSpinAngle = 0;
	mtxIdentity(mFx2Matrix);
	mFx2 = 0;
}

// 0x801A4028
TAniDevilMomonga::~TAniDevilMomonga()
{
	killAllFx();
	seStop3D(&mSound);
	colliFree(mColli);
	mColli = 0;
}

// 0x801A40D0
void TAniDevilMomonga::render()
{
	if (mAsset.mLink.mList == 0) {
		return;
	}
	TEggCreatureTypeE::render();
	if (mKind == 9 && mMode == 8) {
		setDrawAmbient(&mLight, &mDrawState);
		DrawStateSlot* slot = getDrawStateSlot();
		njPushMatrixEx();
		njMultiMatrix(0, mSpinMatrix);
		beginDrawState(slot, (const DrawState*)&mDrawState);
		njSetTexture(mTexList);
		shadowAlphaBegin();
		if (mMotion != 0) {
			gjDrawObject((NJS_OBJECT*)(((void***)mMotion)[0][1]));
		}
		shadowAlphaEnd();
		endDrawState((DrawStateSlot*)slot);
		njPopMatrixEx();
	}
}

// 0x801A4184
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

static inline Vec3f subbedFrom(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjSubVector(&v, a, b);
	return v;
}

static inline Vec3f baseScale(TAniDevilMomonga* self)
{
	Vec3f v;
	Vec3f one;
	one.x = 1.0f;
	one.y = 1.0f;
	one.z = 1.0f;
	gjScaleVector(self->mParams->mBaseScale, &v, &one);
	return v;
}

static inline void flyVel(TAniDevilMomonga* self, Vec3f* out)
{
	Vec3f v;
	gjScaleVector(self->mSpeed, &v, &self->mDir);
	Vec3f along = v;
	*out        = along;
}

void TAniDevilMomonga::onPhase(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(3, 1, 0.075f);
		eggTypeStopAllFx(this);
		startFollowers();
		mImageWait = 0;
		mAniFlags  = 0;
		mStep      = 0x23;
		if (mKind == 0xC) {
			mtxSetTransV(mFxMatrix, &mPos);
			mFx = fxSpawnFollow(1, mFxBank, 3, mFxMatrix);
			mtxSetTransV(mFx2Matrix, &mPos);
			mFx2 = fxSpawnFollow(1, mFxBank, 6, mFx2Matrix);
			sePlay3D(0x9D, &mPos);
		} else if (mKind == 0x11) {
			mtxSetTransV(mFxMatrix, &mPos);
			mFx = fxSpawnFollow(1, mFxBank, 3, mFxMatrix);
			mtxSetTransV(mFx2Matrix, &mPos);
			mFx2 = fxSpawnFollow(1, mFxBank, 6, mFx2Matrix);
			sePlay3D(0x1BD, &mPos);
			sePlay3DHeld(400.0f, &mSound, 0x1EA, 0xC8, (const Vec3f*)&mPos);
		} else {
			mtxSetTransV(mFxMatrix, &mPos);
			mFx = fxSpawnFollow(1, mFxBank, 7, mFxMatrix);
			sePlay3DHeld(400.0f, &mSound, 0x1CF, 0xC8, (const Vec3f*)&mPos);
		}
		makeAttackShape(0, 0);
		mImageWait = 5;
		mTravelled = 0.0f;
		mSpeed     = mParams->mValues[8];
		mTopSpeed  = mParams->mValues[7];
		mTurnRate  = 0;
		mBob       = 0.0f;
		mBobAngle  = 0;
		mSubMode   = 5;
	} else if (phase == 2) {
		if (mHolderRef.get() == 0) {
			setMode(5);
			return;
		}
		switch (mStep) {
		case 0x23:
			runOut();
			break;
		case 0x24:
			runBack();
			break;
		case 0x25:
			runEnd();
			break;
		}
		mtxCopy(mFxMatrix, mNodeMatrix);
		mtxTranslate((float*)mFx2Matrix, (const float*)mNodeMatrix, 0.0f, 0.0f, 20.0f);
		if (mKind == 9) {
			mSpinAngle += 0x1000;
			mSpinAngle &= 0xFFFF;
			mtxRotateY(mSpinMatrix, mNodeMatrix, mSpinAngle);
			mtxTranslate((float*)mSpinMatrix, (const float*)mSpinMatrix, 0.0f, 0.0f, 5.0f);
			if (mImageWait == 0) {
				new TObjFxModel(this, ((void***)mMotion)[0][1], mTexList, mSpinMatrix);
				mImageWait = 5;
			} else {
				mImageWait--;
			}
		}
	} else if (phase == 1) {
		mScale = baseScale(this);
		if (mKind == 0xC) {
			mShape = sMomongaShape1;
		} else {
			mShape = sMomongaShape0;
		}
		colliCopyShapes(mColli, mShape);
		colliShapeClearBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		killAllFx();
		eggTypeStopAllFx(this);
		clearExtraFx();
		seStop3D(&mSound);
	}
}

// 0x801A460C
void TAniDevilMomonga::runOut()
{
	EggHits hits;
	if (mSpeed < mTopSpeed) {
		mSpeed += 0.2f;
	} else {
		mSpeed = mTopSpeed;
	}
	Vec3f vel;
	flyVel(this, &vel);
	Vec3f from = mPos;
	from.y += 10.0f;
	Vec3f to = addedTo(&from, &vel);

	if (meshColliProbeLine((EggHit*)hits.mHits, (const Vec3f*)&from, (const Vec3f*)&to, (unsigned long)0x201) != 0
	    || touchedTarget() != 0) {
		mAniFlags |= 0x2000;
		if (mKind == 9) {
			fxSpawnAt(1, mFxBank, 2, mFxMatrix);
			new TAniBoomerangCurveBomb(this, &mPos, mObjId);
			sePlay3D(0x1D0, &mPos);
		}
		notifyHolder();
		if (njUnitVector(&vel) < gUnitVectorMin) {
			vel = gVecZero;
		}
		vecReflect(&vel, &hits.mHits[0].mNormal, &vel);
		turnBack(vecAngleXZ(&vel));
	} else if (mTravelled >= mParams->mValues[3] || touchedTarget() != 0) {
		turnBack(0x8000);
	}

	mVel = vel;
	mTravelled += mSpeed;
	njAddVector(&mPos, &mVel);
	int a   = vecAngleXZ(&mVel);
	mRot[1] = angleApproach(mRot[1], a, 0x400);
}

// 0x801A487C
void TAniDevilMomonga::runBack()
{
	TObjAction* holder = mHolderRef.get();
	if (holder == 0) {
		setMode(5);
		return;
	}
	Vec3f goal = *holder->getPos();
	f32 dist   = njDistanceP2P(&mPos, &goal);
	f32 flat   = distXZ(&mPos, &goal);
	if (dist < 50.0f) {
		mStep = 0x25;
	} else if (mTurnRate > 0x800) {
		mTurnRate = 0x800;
	} else {
		mTurnRate += 8;
	}
	mSpeed = 0.1f * dist;
	if (mSpeed > mTopSpeed) {
		mSpeed = mTopSpeed;
	}

	int before = mRot[1];
	int target;
	target     = angleToXZ(&mPos, &goal);
	int yaw    = angleApproach(before, target & 0xFFFF, mTurnRate);
	mRot[1]    = yaw;
	int turned = yaw - before;
	int roll;
	if (turned > 0x40) {
		roll = angleApproach(mRot[2], -0x1000, 0x80);
	} else if (turned < -0x40) {
		roll = angleApproach(mRot[2], 0x1000, 0x80);
	} else {
		roll = angleApproach(mRot[2], 0, 0x80);
	}
	mRot[2] = roll;

	Vec3f dir = subbedFrom(&goal, &mPos);
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 rise = dir.y;
	vecFromAngleXZ(mSpeed, &dir, yaw);
	dir.y = rise;
	if (flat > 0.25f * mTravelled) {
		mBobAngle += 0x200;
		if (mBobAngle > 0x10000) {
			mBobAngle = 0x10000;
		}
		mBob  = 2.0f * njSin(mBobAngle);
		dir.y = mBob;
	}
	mVel = dir;
	njAddVector(&mPos, &mVel);
}

// 0x801A4AE4
void TAniDevilMomonga::runEnd()
{
	endAttack();
}

// 0x801A4B04
void TAniDevilMomonga::turnBack(int angle)
{
	TObjAction* holder = mHolderRef.get();
	if (holder == 0) {
		setMode(5);
		return;
	}
	Vec3f vel;
	Vec3f goal;
	goal    = *holder->getPos();
	int yaw = (u16)(angleToXZ(&mPos, &goal) - angle);
	vecFromAngleXZ(mSpeed, &vel, yaw);
	mVel    = vel;
	mRot[1] = yaw;
	playMotion(10, 1, 1.0f);
	mStep = 0x24;
}

// 0x801A4BD4
int TAniDevilMomonga::onHit(const TObjAction* other, int, int* result)
{
	return fillHitParam(other, result, 2);
}

static int sBone; // 0x802C5A20

// 0x801A4C50
void TAniDevilMomonga::resetBones()
{
	sBone = 0;
}

// 0x801A4C5C
extern "C" void momongaBones0()
{
	copyAnimalBones(9, 0, sBone);
	sBone++;
}

// 0x801A4CF4
extern "C" void momongaBones1()
{
	copyAnimalBones(9, 1, sBone);
	sBone++;
}

// 0x801A4D8C
extern "C" void momongaBones2()
{
	copyAnimalBones(9, 2, sBone);
	sBone++;
}

// 0x801A4E24
extern "C" void momongaBones3()
{
	copyAnimalBones(9, 3, sBone);
	sBone++;
}

// 0x801A4EBC
extern "C" void momongaBones4()
{
	copyAnimalBones(9, 4, sBone);
	sBone++;
}

#include "animal/AniAttacks.h"
#include "engine/Colli.h"
#include "animal/TAniBoomerangCurveBomb.h"
struct CharaColli;

const char* gTAniBoomerangCurveBombName = "TAniBoomerangCurveBomb";

const ColliShape sBoomerangCurveBombShape = { 0 };

// 0x801A4F54
TAniBoomerangCurveBomb::TAniBoomerangCurveBomb(TObject* owner, const Vec3f* pos, u16 ownerId)
    : TObject(owner)
    , mPos(*pos)
{
	_00              = (u32)gTAniBoomerangCurveBombName;
	mSize            = 0x40;
	mRadius          = 10.0f;
	mGrowth          = 0.0f;
	mTimer           = 0x78;
	ColliShape shape = sBoomerangCurveBombShape;
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

// 0x801A5070
TAniBoomerangCurveBomb::~TAniBoomerangCurveBomb()
{
	colliFree((CharaColli*)mColli);
}

// 0x801A50D8
void TAniBoomerangCurveBomb::draw()
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

// 0x801A5188
void TAniBoomerangCurveBomb::render()
{
}

// 0x801A518C
int TAniBoomerangCurveBomb::vfn_20()
{
	return 4;
}

// 0x801A5194
const Vec3f* TAniBoomerangCurveBomb::vfn_24()
{
	return &mPos;
}
