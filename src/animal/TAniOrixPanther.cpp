#include "animal/RacerMover.h"
#include "animal/AniAttacks.h"
#include "io/ResBank.h"
#include "core/MathUtil.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/FxSystem.h"
#include "engine/Colli.h"
#include "egg/TEggCreatureTypeE.h"
#include "object/TObjEggRing.h"
#include "sound/Sound3DPlay.h"
#include "Ninja/ninja.h"
#include "engine/GroundQuery.h"
#include "animal/TAniOrixPanther.h"
struct Fx;
struct ColliShape;

extern "C" {
MotionEntry gOrixMotions[13] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 15.0f, 0.27f, 0.0f, 0 },
	{ MOTION_BLEND, 6, -1, 12, 0.0f, 8.0f, 0.2f, 0.075f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 29.0f, 1.5f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 16.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 19.0f, 0.2f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 29.0f, 1.5f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 11.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 1.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 0.0f, 11.0f, 0.2f, 0.0f, 0 },
	{ MOTION_ONCE, 10, -1, 0, 0.0f, 9.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 9.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 20.0f, 0.2f, 0.0f, 0 },
};
ColliShape sOrixShapes[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
MotionEntry sOrixMotions1[11] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 10.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 39.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 19.0f, 0.7f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 9.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 19.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 0.0f, 19.0f, 0.35f, 0.0f, 0 },
};
ColliShape sOrixShapes1[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
MotionEntry sOrixMotions2[12] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.15f, 0.0f, 0 },
	{ MOTION_BLEND, 6, -1, 11, 0.0f, 29.0f, 0.5f, 0.075f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 39.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 19.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 24.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 56.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 8, -1, 0, 0.0f, 19.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 29.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 8, -1, 0, 0.0f, 19.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 29.0f, 0.6f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 39.0f, 0.5f, 0.0f, 0 },
};
ColliShape sOrixShapes2[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
MotionEntry sOrixMotions3[11] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 29.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 56.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 29.0f, 1.5f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 1.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 1.0f, 9.0f, 0.3f, 0.0f, 0 },
};
ColliShape sOrixShapes3[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TAniOrixPanther_size_check[(sizeof(TAniOrixPanther) == 0x3F4) ? 1 : -1];

inline void TAniOrixPanther::killAllFx()
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
void orixBones0();
void orixBones1();
void orixBones2();
void orixBones3();
void orixBones4();
}

void* const sOrixPantherCallbacks[13] = {
	(void*)orixBones0, (void*)orixBones1, (void*)orixBones2, (void*)orixBones3, (void*)orixBones4,
};

const char* gTAniOrixPantherName = "TAniOrixPanther";

// 0x8019B7FC
TAniOrixPanther::TAniOrixPanther(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00   = (u32)gTAniOrixPantherName;
	mSize = 0x3F4;
	if (kind == 0xD) {
		mNodes = sOrixMotions1;
		mShape = sOrixShapes1;
		mMotionCount = 1;
	} else if (kind == 0xF) {
		mNodes = sOrixMotions2;
		mShape = sOrixShapes2;
		mMotionCount = 1;
	} else if (kind == 0x12) {
		mNodes = sOrixMotions3;
		mShape = sOrixShapes3;
		mMotionCount = 1;
	} else {
		mNodes = gOrixMotions;
		mShape = sOrixShapes;
		mMotionCount = 2;
		m270   = sOrixPantherCallbacks;
	}
	initColli();
	mDashes = 5;
	mFx2    = 0;
	mFx3    = 0;
	mtxIdentity(mFx2Matrix);
	mtxIdentity(mFx3Matrix);
}

// 0x8019B960
TAniOrixPanther::~TAniOrixPanther()
{
	killAllFx();
	colliFree(mColli);
	mColli = 0;
}

// 0x8019BA18
void TAniOrixPanther::onPhase(int phase)
{
	if (phase == 0) {
		playMotion(3, 1, 0.075f);
		setMotionSpeed(1.0f);
		eggTypeStopAllFx(this);
		startFollowers();
		mTimer = 0;
		mAniFlags   = 0;
		mStep  = 0x20;

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
		killFx();
		mStartPos = mPos;
		mRot[1]   = vecAngleXZ(&mDir);
		mDashes   = (int)mParams->mValues[8];
		mSubMode  = 5;
	} else if (phase == 2) {
		switch (mStep) {
		case 0x20:
			runStart();
			break;
		case 0x21:
			runDash();
			break;
		case 0x22:
			runEnd();
			break;
		}
		if (mKind == 0xD) {
			mtxCopy(mFxMatrix, mNodeMatrix);
			mtxTranslate((float*)mFx2Matrix, (const float*)mNodeMatrix, 0.0f, 0.0f, 20.0f);
			mtxCopy(mFx3Matrix, mNodeMatrix);
		}
	} else if (phase == 1) {
		if (mKind == 0xD) {
			killAllFx();
		}
		mShape = sOrixShapes;
		colliCopyShapes(mColli, mShape);
		colliShapeClearBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		eggTypeStopAllFx(this);
		clearExtraFx();
		mVel.x = 0.0f;
		mVel.y = 0.0f;
		mVel.z = 0.0f;
	}
}

// 0x8019BCC0
void TAniOrixPanther::runStart()
{
	if (mPlayer.isDone() == 0) {
		return;
	}
	TObjFxModel* model = new TObjFxModel(this, ((void***)mMotion)[0][0], mTexList, mNodeMatrix);
	int cur = mPlayer.mCurrent;
	f32 frame = mPlayer.mFrame;
	fxModelMotion(frame, model, mPlayer.getMotion(cur));
	pickTarget();
	playMotion(8, 1, 0.075f);
	if (mKind == 0xD) {
		sePlay3D(0x1BD, &mPos);
		mtxCopy(mFxMatrix, mNodeMatrix);
		mFx = fxSpawnFollow(1, mFxBank, 3, mFxMatrix);
		mtxCopy(mFx2Matrix, mNodeMatrix);
		mFx2 = fxSpawnFollow(1, mFxBank, 6, mFx2Matrix);
		mtxCopy(mFxMatrix, mNodeMatrix);
		mFx3 = fxSpawnFollow(1, mFxBank, 8, mFx3Matrix);
	} else if (mKind == 0x12) {
		sePlay3D(0x1BD, &mPos);
	} else if (mKind == 7) {
		sePlay3D(0x1D5, &mPos);
	}
	mStep = 0x21;
}

static inline Vec3f orixAdd(const Vec3f* a, const Vec3f* b)
{
	Vec3f sum;
	gjAddVector(&sum, a, b);
	return sum;
}

// 0x8019BE34
void TAniOrixPanther::runDash()
{
	f32 left = distXZ(&mGoal, &mPos);
	if (left > 0.5f) {
		EggHits hits;
		Vec3f from = mPos;
		from.y += 10.0f;
		Vec3f to = orixAdd(&from, &mVel);
		if (meshColliProbeLine((EggHit*)hits.mHits, (const Vec3f*)&from, (const Vec3f*)&to, (unsigned long)0x201) != 0 || touchedTarget() != 0) {
			endDash();
			mStep = 0x22;
			return;
		}
		if (meshColliProbe((EggHit*)hits.mHits, (const Vec3f*)&to, (unsigned long)1) != 0) {
			if ((hits.mMask & 1) != 0 && mPos.y - hits.mHits[0].mPos.y >= 20.0f) {
				endDash();
				mStep = 0x22;
				return;
			}
		} else {
			endDash();
			mStep = 0x22;
			return;
		}
		njAddVector(&mPos, &mVel);
		if (mKind == 0xF) {
			fxSpawnAt(1, mFxBank, 8, mNodeMatrix);
			sePlay3D(0x9D, &mPos);
		}
	} else {
		if (mKind == 0xD) {
			killAllFx();
		}
		if (mDashes == 0) {
			playMotion(9, 1, 0.075f);
			mStep = 0x22;
		} else {
			f32 range = (f32)(mMotionCount + 0xA);
			f32 pick  = (range - 10.0f) * (3.0517578e-5f * rand()) + 10.0f;
			playMotion((s8)(int)pick, 1, 0.075f);
			mDashes--;
			mStep = 0x20;
		}
	}
}

// 0x8019C0E4
void TAniOrixPanther::runEnd()
{
	if (mPlayer.isDone() != 0) {
		endAttack();
	}
}

// 0x8019C124
void TAniOrixPanther::endDash()
{
	if (mKind == 0xD) {
		killAllFx();
	} else if (mKind == 7) {
		fxSpawnAt(1, mFxBank, 2, mNodeMatrix);
		sePlay3D(0x1D6, &mPos);
		new TAniAImageHomingBomb(this, &mPos, mObjId);
	}
	notifyHolder();
	playMotion(9, 1, 0.075f);
}

// 0x8019C20C
int TAniOrixPanther::onHit(const TObjAction* other, int, int* result)
{
	return fillHitParam(other, result, 2);
}

// 0x8019C288
static inline Vec3f orixSub(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjSubVector(&v, a, b);
	return v;
}

static inline Vec3f orixScale(const Vec3f* dir, f32 s)
{
	Vec3f v;
	gjScaleVector(s, &v, dir);
	return v;
}

void TAniOrixPanther::pickTarget()
{
	int rot = vecAngleXZ(&mDir);
	u32 id  = findEggAhead(&mPos, rot, mHolderId, 0x8000, 100.0f);
	if ((u16)id != 0xFFFF) {
		TObjAction* target = findObjById(id);
		if (target != 0) {
			mGoal = *target->getPos();
			Vec3f dir  = orixSub(&mGoal, &mPos);
			f32 length = njUnitVector(&dir);
			if (length < gUnitVectorMin) {
				dir = gVecZero;
			}
			mVel    = orixScale(&dir, length * mParams->mValues[7]);
			mRot[1] = vecAngleXZ(&dir);
		} else {
			pickPoint();
		}
	} else {
		pickPoint();
	}
}

// 0x8019C3DC
void TAniOrixPanther::pickPoint()
{
	int rot   = vecAngleXZ(&mDir);
	int angle = (int)((f32)rot + (16384.0f * (3.0517578e-5f * rand()) + -8192.0f));
	Vec3f ahead;
	vecFromAngleXZ(50.0f * (3.0517578e-5f * rand()) + 50.0f, &ahead, angle);
	mGoal = orixAdd(&mPos, &ahead);
	mVel  = orixScale(&ahead, 0.25f);
	mRot[1] = angle;
}

static int sBone; // 0x802C59D8

// 0x8019C528
void TAniOrixPanther::resetBones()
{
	sBone = 0;
}

// 0x8019C534
extern "C" void orixBones0()
{
	copyAnimalBones(7, 0, sBone);
	sBone++;
}

// 0x8019C5CC
extern "C" void orixBones1()
{
	copyAnimalBones(7, 1, sBone);
	sBone++;
}

// 0x8019C664
extern "C" void orixBones2()
{
	copyAnimalBones(7, 2, sBone);
	sBone++;
}

// 0x8019C6FC
extern "C" void orixBones3()
{
	copyAnimalBones(7, 3, sBone);
	sBone++;
}

// 0x8019C794
extern "C" void orixBones4()
{
	copyAnimalBones(7, 4, sBone);
	sBone++;
}

#include "animal/AniAttacks.h"
#include "engine/Colli.h"
#include "animal/TAniAImageHomingBomb.h"
struct CharaColli;

const char* gTAniAImageHomingBombName = "TAniAImageHomingBomb";

const ColliShape sAImageHomingBombShape = { { 0.0f, 30.0f, 0.0f }, 0x0, 0, 0x0, 0.0f, 30.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 };

// 0x8019C82C
TAniAImageHomingBomb::TAniAImageHomingBomb(TObject* owner, const Vec3f* pos, u16 ownerId)
    : TObject(owner)
    , mPos(*pos)
{
	_00     = (u32)gTAniAImageHomingBombName;
	mSize   = 0x40;
	mRadius = 10.0f;
	mGrowth = 0.0f;
	mTimer  = 0x48;
	ColliShape shape = sAImageHomingBombShape;
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

// 0x8019C948
TAniAImageHomingBomb::~TAniAImageHomingBomb()
{
	colliFree((CharaColli*)mColli);
}

// 0x8019C9B0
void TAniAImageHomingBomb::draw()
{
	mGrowth += 0.15f;
	mRadius += mGrowth;
	f32 r = mRadius;
	f32 size = r < 0.0f ? 0.0f : (r > 60.0f ? 60.0f : r);
	colliGetShapes(mColli)->mSizeX = size;
	if (mTimer == 0) {
		mFlags |= 1;
	} else {
		mTimer--;
	}
}

// 0x8019CA60
void TAniAImageHomingBomb::render()
{
}

// 0x8019CA64
int TAniAImageHomingBomb::vfn_20()
{
	return 4;
}

// 0x8019CA6C
const Vec3f* TAniAImageHomingBomb::vfn_24()
{
	return &mPos;
}
