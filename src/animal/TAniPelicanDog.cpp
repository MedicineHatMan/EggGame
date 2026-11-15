#include "egg/EggCreature.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "camera/TCamPath.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "object/ObjShadow.h"
#include "egg/TEggCreatureTypeE.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "engine/GroundQuery.h"
#include "animal/TAniPelicanDog.h"
struct DrawStateSlot;
struct DrawState;
struct ColliShape;
struct DrawTask;
struct NJS_OBJECT;

extern "C" {
f32 sPelicanDogRate0 = 0.15f; // 0x802C4438 the table's rates
f32 sPelicanDogRate1 = 0.11f;
f32 sPelicanDogRate2 = 0.55f;
f32 sPelicanDogRate3 = 0.6f;

MotionEntry gPelicanMotions[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 15.0f, sPelicanDogRate0, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 33.0f, sPelicanDogRate1, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, sPelicanDogRate2, 0.0f, 0 },
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 15.0f, sPelicanDogRate3, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 15.0f, sPelicanDogRate3, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, sPelicanDogRate2, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 33.0f, sPelicanDogRate1, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 30.0f, sPelicanDogRate2, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, sPelicanDogRate2, 0.0f, 0 },
};
ColliShape sPelicanDogFirstShape[] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

static void drawExtraTask(void* me);

typedef char TAniPelicanDog_size_check[(sizeof(TAniPelicanDog) == 0x3A0) ? 1 : -1];

extern "C" void pelicanBones0();
extern "C" void pelicanBones1();
extern "C" void pelicanBones2();
extern "C" void pelicanBones3();

// 0x80249818
// The bone callbacks the actor runs one per motion group
extern "C" void* const sPelicanDogCallbacks[7] = {
	(void*)pelicanBones0, (void*)pelicanBones1, (void*)pelicanBones2, (void*)pelicanBones3, 0, 0, 0,
};

const char* gTAniPelicanDogName = "TAniPelicanDog";

// 0x8013CD38
TAniPelicanDog::TAniPelicanDog(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00        = (u32)gTAniPelicanDogName;
	mSize      = 0x3A0;
	mExtraTask = addDrawTask(this, (void (*)(void*))drawExtraTask, 0x22, 0);
	mNodes     = gPelicanMotions;
	mShape     = sPelicanDogFirstShape;
	initColli();
	m270       = sPelicanDogCallbacks;
	mShowExtra = 0;
	mExtraFrame = 0.0f;
	mTravelled = 0.0f;
	mSpeed     = 0.0f;
	mSpin      = 0;
	mNodeKind  = -1;
}

// 0x8013CE40
TAniPelicanDog::~TAniPelicanDog()
{
	killFx();
	colliFree(mColli);
	mColli = 0;
	removeDrawTask((DrawTask*)mExtraTask);
}

// 0x8013CED0
void TAniPelicanDog::draw()
{
	TEggCreatureTypeE::draw();
	const MotionEntry* table = (const MotionEntry*)mNodes;
	int kind = table[mPlayer.mCurrent].mMotion;
	if (kind != mNodeKind) {
		if (kind == 1) {
			mShowExtra  = 1;
			mExtraFrame = 0.0f;
		}
		if (mNodeKind == 1) {
			mShowExtra = 0;
			mtxIdentity(sPelicanBoneMtx);
		}
		mNodeKind = kind;
	}
	if (kind == 1) {
		mExtraFrame += sPelicanDogRate1;
		u32 frames = *(u32*)((u8*)((void***)mMotion)[1][6] + 4);
		if (mExtraFrame > (f32)frames) {
			mExtraFrame -= (f32)frames;
		}
	}
}

// 0x8013CFBC
void TAniPelicanDog::vfn_10()
{
	Vec3f view;
	f32 radius = 2.0f * (mShape)->mSizeX;
	if (mLight.isVisible(mLightTask, 1000.0f, &mPos, radius, (int)&view) != 0) {
		mLightTask->mFlags &= ~1;
		((DrawTask*)mExtraTask)->mSortKey = drawTaskDepthKey(view.z);
		((DrawTask*)mExtraTask)->mFlags &= ~1;
	} else {
		mLightTask->mFlags |= 1;
		((DrawTask*)mExtraTask)->mFlags |= 1;
	}
}

// 0x8013D068
void TAniPelicanDog::renderExtra()
{
	if (mShowExtra == 0) {
		return;
	}
	setDrawAmbient(&mLight, &mDrawState);
	DrawStateSlot* slot = getDrawStateSlot();
	njPushMatrixEx();
	njMultiMatrix(0, mNodeMatrix);
	njMultiMatrix(0, sPelicanBoneMtx);
	beginDrawState(slot, (const DrawState*)&mDrawState);
	njSetTexture(mTexList);
	shadowAlphaBegin();
	void*** assets = (void***)mMotion;
	if (assets != 0) {
		gjDrawMotion((NJS_OBJECT*)assets[0][1], assets[1][6], mExtraFrame);
	}
	shadowAlphaEnd();
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
}

// 0x8013D11C
void TAniPelicanDog::onPhase(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(3, 1, 0.075f);
		eggTypeStopAllFx(this);
		startFollowers();
		mTimer = 0x3C;
		mAniFlags   = 0;
		mStep  = 0x17;
		makeAttackShape(1, 1);
		mtxSetTransV(mFxMatrix, &mPos);
		mFx       = fxSpawnFollow(1, mFxBank, 1, mFxMatrix);
		mStartPos = mPos;
		mRot[1]   = vecAngleXZ(&mDir) - 0x8000;
		mTravelled = 0.0f;
		mSpeed    = 0.0f;
		mSpin     = 0x580;
		mSubMode  = 5;
	} else if (phase == 2) {
		switch (mStep) {
		case 0x17:
			runWait();
			break;
		case 0x18:
			runFly();
			break;
		case 0x19:
			runShrink();
			break;
		}
	} else if (phase == 1) {
		mShape = sPelicanDogFirstShape;
		colliCopyShapes(mColli, mShape);
		colliShapeClearBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		killFx();
		eggTypeStopAllFx(this);
		clearExtraFx();
	}
}

// 0x8013D338
void TAniPelicanDog::runWait()
{
	if (mTimer == 0) {
		sePlay3D(0x1B9, &mPos);
		mStep = 0x18;
	} else {
		mTimer--;
	}
}

/** v scaled by s returned by value */
static inline Vec3f pelicanScale(f32 s, const Vec3f* v)
{
	Vec3f r;
	gjScaleVector(s, &r, v);
	return r;
}

/** a plus b returned by value */
static inline Vec3f pelicanAdd(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjAddVector(&v, a, b);
	return v;
}

/** Sets out to v scaled by s */
static inline void pelicanSetScaled(Vec3f* out, f32 s, const Vec3f* v)
{
	Vec3f r = pelicanScale(s, v);
	*out = r;
}

/** Adds v scaled by s to out */
static inline void pelicanAddScaled(Vec3f* out, f32 s, const Vec3f* v)
{
	Vec3f r = pelicanScale(s, v);
	njAddVector(out, &r);
}

/** Where the body ends up after one step of vel from pos */
static inline Vec3f pelicanAhead(const Vec3f* pos, const Vec3f* vel)
{
	return pelicanAdd(pos, vel);
}

// 0x8013D38C
void TAniPelicanDog::runFly()
{
	if (mScale.x >= 2.5f) {
		mScale.x = 2.5f;
		mScale.y = 2.5f;
		mScale.z = 2.5f;
	} else {
		mScale.x += 0.01f;
		mScale.y += 0.01f;
		mScale.z += 0.01f;
	}
	colliGetShapes(mColli)->mSizeX = mParams->mValues[5] * mScale.x;
	colliGetShapes(mColli)->mSizeY = mParams->mValues[6] * mScale.y;

	mSpeed += mParams->mValues[8];
	if (mSpeed > mParams->mValues[7]) {
		mSpeed = mParams->mValues[7];
	}

	int rot = mRot[1];
	rot += mSpin;
	rot = (u16)rot;
	mRot[1] = rot;
	Vec3f forward;
	forward.x = 0.0f;
	forward.y = 0.0f;
	forward.z = 1.0f;
	vecRotateY(&forward, &forward, rot);
	pelicanSetScaled(&mVel, mSpeed, &forward);

	f32 drift = 0.75f;
	drift = 0.5f * mSpeed > drift ? drift : 0.5f * mSpeed;
	pelicanAddScaled(&mVel, drift, &mDir);
	mTravelled += drift;

	EggHits hits;
	EggProbe probe;
	Vec3f to = pelicanAhead(&mPos, &mVel);
	if (meshColliProbeLine((EggHit*)&hits, (const Vec3f*)&mPos, (const Vec3f*)&to, (unsigned long)((-1))) != 0 || mTravelled >= mParams->mValues[3] || touchedTarget() != 0) {
		notifyHolder();
		sePlay3D(0x1BA, &mPos);
		killFx();
		mtxTranslate((float*)mFxMatrix, (const float*)mNodeMatrix, 0.0f, 10.0f, 0.0f);
		fxSpawnAt(1, mFxBank, 6, mFxMatrix);
		mStep = 0x19;
	} else {
		njAddVector(&mPos, &mVel);
		mtxSetTransV(mFxMatrix, &mPos);
	}
}

// 0x8013D678
void TAniPelicanDog::runShrink()
{
	colliGetShapes(mColli)->mSizeX = 3.5f * mParams->mValues[5];
	colliGetShapes(mColli)->mSizeY = 3.5f * mParams->mValues[6];
	f32 base = mParams->mBaseScale;
	if (mScale.x <= base) {
		mScale.x = base;
		mScale.y = base;
		mScale.z = base;
		endAttack();
	} else {
		mScale.x -= 0.02f;
		mScale.y -= 0.02f;
		mScale.z -= 0.02f;
	}
}

// 0x8013D744
int TAniPelicanDog::onHit(const TObjAction* other, int, int* result)
{
	return fillHit(other, result, 8);
}

static int sBone; // 0x802C56A8

// 0x8013D7B8
void TAniPelicanDog::resetBones()
{
	sBone = 0;
}

// 0x8013D7C4
extern "C" void pelicanBones0()
{
	copyAnimalBones(5, 0, sBone);
	sBone++;
}

// 0x8013D85C
extern "C" void pelicanBones1()
{
	copyAnimalBones(5, 1, sBone);
	if (sBone == 0x1E) {
		Vec3f at;
		njGetTranslation(0, &at);
		mtxSetTransV((f32*)sPelicanBoneMtx, &at);
	}
	sBone++;
}

// 0x8013D91C
extern "C" void pelicanBones2()
{
	copyAnimalBones(5, 2, sBone);
	sBone++;
}

// 0x8013D9B4
extern "C" void pelicanBones3()
{
	copyAnimalBones(5, 3, sBone);
	sBone++;
}

// 0x8013DA4C
static void drawExtraTask(void* me)
{
	((TAniPelicanDog*)me)->renderExtra();
}

// 0x8013DA6C
// is the static initializer that stores the rates into gPelicanMotions
