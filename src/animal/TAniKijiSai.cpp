#include "egg/EggCreature.h"
#include "stage/Stage.h"
#include "engine/DrawTask.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "object/TObjLauncher.h"
#include "camera/TCamPath.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "object/ObjShadow.h"
#include "egg/TEggCreatureTypeE.h"
#include "sound/Sound3DPlay.h"
#include "engine/GroundQuery.h"
#include "stl/math.h"
#include "object/TObjEggRing.h"
#include "engine/TParticle.h"
#include "camera/TCameraCtrl.h"
#include "engine/Color.h"
#include "animal/TAniKijiSai.h"
#include "animal/TAniSpiralFire.h"
#include "object/ObjClassName.h"
struct DrawStateSlot;
struct DrawState;
struct Fx;
struct DrawTask;
struct CharaColli;
struct ColliShape;

// 0x802C43A0
// the motion rates the static init writes into gKijiMotions
f32 sKijiSaiRate0 = 0.25f;
f32 sKijiSaiRate1 = 0.33f;
f32 sKijiSaiRate2 = 0.85f;
f32 sKijiSaiRate3 = 0.65f;

extern "C" {
MotionEntry gKijiMotions[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 15.0f, sKijiSaiRate0, 0.0f, 0 }, { MOTION_LOOP, 1, -1, 0, 0.0f, 30.0f, sKijiSaiRate1, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 32.0f, sKijiSaiRate2, 0.0f, 0 }, { MOTION_LOOP, 3, -1, 0, 0.0f, 15.0f, sKijiSaiRate3, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 32.0f, sKijiSaiRate2, 0.0f, 0 }, { MOTION_ONCE, 4, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 32.0f, sKijiSaiRate2, 0.0f, 0 }, { MOTION_ONCE, 1, -1, 0, 0.0f, 30.0f, sKijiSaiRate1, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 32.0f, sKijiSaiRate2, 0.0f, 0 }, { MOTION_LOOP, 2, -1, 0, 0.0f, 32.0f, sKijiSaiRate2, 0.0f, 0 },
};
ColliShape sKijiSaiFirstShape[] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TAniKijiSai_size_check[(sizeof(TAniKijiSai) == 0x3D4) ? 1 : -1];

inline void TAniKijiSai::killFx2()
{
	if (mFx2 != 0) {
		fxRemove(mFx2);
		mFx2 = 0;
	}
}

extern "C" void kijiBones0();
extern "C" void kijiBones1();
extern "C" void kijiBones2();
extern "C" void kijiBones3();

// 0x802493D8
// the bone copiers the creature runs in order null ended
void* const sKijiSaiCallbacks[] = {
	(void*)kijiBones0, (void*)kijiBones1, (void*)kijiBones2, (void*)kijiBones3, 0,
};

ObjClassName gTAniKijiSaiName = { "TAniKijiSai", 0 };

// 0x801325B0
TAniKijiSai::TAniKijiSai(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00    = (u32)gTAniKijiSaiName.mName;
	mSize  = 0x3D4;
	mNodes = gKijiMotions;
	mShape = sKijiSaiFirstShape;
	initColli();
	m270       = sKijiSaiCallbacks;
	mSpeed     = 0.0f;
	mSpread    = 0.0f;
	mSpin      = 0;
	mTravelled = 0.0f;
	mFx2       = 0;
	mtxIdentity(mFx2Matrix);
}

// 0x8013269C
TAniKijiSai::~TAniKijiSai()
{
	killFx();
	killFx2();
	colliFree(mColli);
	mColli = 0;
}

/** Draws one copy spread to the side */
#define KIJISAI_DRAW_ONE(spread)                      \
	njPushMatrixEx();                                 \
	gjTranslate(spread, 0.0f, 0.0f);                  \
	gjRotateZ(-mRot[2]);                              \
	if (mActor != 0) {                                \
		gjDrawActor(mActor);                          \
	} else {                                          \
		debugWireSphere(gjGetMatrix(), &gColorBlack); \
	}                                                 \
	njPopMatrixEx();

// 0x8013273C
void TAniKijiSai::render()
{
	if (mMode != 8) {
		TEggCreatureTypeE::render();
		return;
	}
	setDrawAmbient(&mLight, &mDrawState);
	DrawStateSlot* slot = getDrawStateSlot();
	njPushMatrixEx();
	beginDrawState(slot, (const DrawState*)&mDrawState);
	njSetTexture(mTexList);
	shadowAlphaBegin();
	njTranslateEx(&mPos);
	gjRotateY(mRot[1]);
	gjRotateZ(mRot[2]);
	KIJISAI_DRAW_ONE(mSpread)
	KIJISAI_DRAW_ONE(-mSpread)
	shadowAlphaEnd();
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
}

// 0x8013285C
void TAniKijiSai::onPhase(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(3, 1, 0.075f);
		eggTypeStopAllFx(this);
		startFollowers();
		mTimer    = 0;
		mAniFlags = 0;
		mStep     = 0xC;

		u8 shape[0x30];
		if ((int)mParams->mValues[4] == 0) {
			colliShapeMakeSphere((ColliShape*)shape, &gVecZero, 1, 5.0f);
		} else {
			colliShapeMakeCylinder((ColliShape*)shape, &gVecZero, 1, 5.0f, 5.0f);
		}
		colliCopyShapes(mColli, (const ColliShape*)shape);
		colliShapeSetBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0xFF, 0xFF, 0x10);

		sePlay3D(0x1B3, &mPos);
		mtxSetTransV(mFxMatrix, &mPos);
		mFx = fxSpawnFollow(1, mFxBank, 2, mFxMatrix);
		mtxSetTransV(mFx2Matrix, &mPos);
		mFx2       = fxSpawnFollow(1, mFxBank, 2, mFx2Matrix);
		mFlyDir    = mDir;
		mSpeed     = 0.0f;
		mSpread    = 0.0f;
		mSpin      = 0x800;
		mTravelled = 0.0f;
		mSubMode   = 5;
	} else if (phase == 2) {
		switch (mStep) {
		case 0xC:
			runFly();
			break;
		case 0xD:
			runEnd();
			break;
		}
	} else if (phase == 1) {
		mFlags &= ~0x10;
		mRot[2] = 0;
		mShape  = sKijiSaiFirstShape;
		colliCopyShapes(mColli, mShape);
		colliShapeClearBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		killFx();
		killFx2();
		eggTypeStopAllFx(this);
		clearExtraFx();
	}
}

/** a plus b returned by value */
static inline Vec3f kijiAdd(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjAddVector(&v, a, b);
	return v;
}

// 0x80132AB0
void TAniKijiSai::runFly()
{
	f32 spread = mSpread;
	f32 most   = mParams->mValues[5];
	if (spread < most) {
		spread += 0.5f;
		mSpread = spread;
	} else {
		mSpread = most;
	}
	colliGetShapes(mColli)->mSizeX = mParams->mValues[5];

	mVel = mFlyDir;
	if (njUnitVector(&mVel) < gUnitVectorMin) {
		mVel = gVecZero;
	}
	gjScaleVector(mSpeed, &mVel, &mVel);
	mSpeed += mParams->mValues[8];
	if (mSpeed > mParams->mValues[7]) {
		mSpeed = mParams->mValues[7];
	}

	EggHits hits;
	EggProbe probe;
	Vec3f to = kijiAdd(&mPos, &mVel);
	if (meshColliProbeLine((EggHit*)&hits, (const Vec3f*)&mPos, (const Vec3f*)&to, (unsigned long)((-1))) != 0
	    || mTravelled >= mParams->mValues[3] || touchedTarget() != 0) {
		eggTypeStopAllFx(this);
		killFx();
		killFx2();
		fxSpawnAt(1, mFxBank, 7, mNodeMatrix);
		new TAniSpiralFire(this, &mPos, mObjId);
		notifyHolder();
		sePlay3D(0x1B4, &mPos);
		mFlags |= 0x10;
		mTimer = 0x78;
		mStep  = 0xD;
	} else {
		njAddVector(&mPos, &mVel);
		if (njScalor(&mVel) > 0.0f) {
			mRot[1] = (int)(10430.38043493439 * (f32)atan2(mVel.x, mVel.z));
		}
		int rz = mRot[2];
		rz += mSpin;
		mRot[2] = rz;
		mtxSetTransV(mFxMatrix, &mPos);
		mtxRotateY(mFxMatrix, mFxMatrix, mRot[1]);
		mtxCopy(mFx2Matrix, mFxMatrix);
		mtxRotateZ(mFxMatrix, mFxMatrix, mRot[2]);
		mtxRotateZ(mFx2Matrix, mFx2Matrix, mRot[2]);
		mtxTranslate((float*)mFxMatrix, (const float*)mFxMatrix, mSpread, 0.0f, 0.0f);
		mtxTranslate((float*)mFx2Matrix, (const float*)mFx2Matrix, -mSpread, 0.0f, 0.0f);
		mTravelled += mSpeed;
	}
}

// 0x80132DE8
void TAniKijiSai::runEnd()
{
	if (mTimer == 0) {
		endAttack();
	} else {
		mTimer--;
	}
}

// 0x80132E20
// In the attack the base's version would not know about the spread so this one places everything on both copies
void TAniKijiSai::vfn_A0()
{
	if (mMode == 8) {
		f32 root[12];
		f32 mtx[12];
		Vec3f at;
		mtxSetTransV(root, &mPos);
		mtxRotateY(root, root, mRot[1]);
		mtxRotateZ(root, root, mRot[2]);

		if (mFollowers[0] != 0) {
			mtxTranslate((float*)mtx, (const float*)root, mSpread, 0.0f, 0.0f);
			mtxConcat((f32*)mtx, (const f32*)mtx, gAnimalExtraMatrices[0]);
			mtxGetPos(mtx, &at);
			mFollowers[0]->mPos = at;
		}
		if (mFollowers[1] != 0) {
			mtxTranslate((float*)mtx, (const float*)root, -mSpread, 0.0f, 0.0f);
			mtxConcat((f32*)mtx, (const f32*)mtx, gAnimalExtraMatrices[0]);
			mtxGetPos(mtx, &at);
			mFollowers[1]->mPos = at;
		}
		if (mEvents != 0) {
			for (s16 i = 0; i < mEventCount; i++) {
				if (mPlayer.passedFrame((f32)mEvents[i].mFrame) == 0) {
					continue;
				}
				if (mEvents[i].mBone != -1) {
					mtxTranslate((float*)mtx, (const float*)root, mSpread, 0.0f, 0.0f);
					mtxConcat((f32*)mtx, (const f32*)root, (const f32*)(gAnimalBoneMatrices + mEvents[i].mBone * 0x30));
					mtxGetPos(mtx, &at);
					particleOnceById(this, &at, 0, mEvents[i].mEffect);
					mtxTranslate((float*)mtx, (const float*)root, -mSpread, 0.0f, 0.0f);
					mtxConcat((f32*)mtx, (const f32*)root, (const f32*)(gAnimalBoneMatrices + mEvents[i].mBone * 0x30));
					mtxGetPos(mtx, &at);
					particleOnceById(this, &at, 0, mEvents[i].mEffect);
				} else {
					mtxGetPos(root, &at);
					particleOnceById(this, &at, 0, mEvents[i].mEffect);
				}
			}
		}
	} else {
		TEggCreatureTypeE::vfn_A0();
	}
}

// 0x801330D4
int TAniKijiSai::onHit(const TObjAction* other, int, int* result)
{
	return fillHit(other, result, 4);
}

static int sBone; // 0x802C5680

// 0x80133148
void TAniKijiSai::resetBones()
{
	sBone = 0;
}

// 0x80133154
extern "C" void kijiBones0()
{
	copyAnimalBones(1, 0, sBone);
	sBone++;
}

// 0x801331EC
extern "C" void kijiBones1()
{
	copyAnimalBones(1, 1, sBone);
	sBone++;
}

// 0x80133284
extern "C" void kijiBones2()
{
	copyAnimalBones(1, 2, sBone);
	sBone++;
}

// 0x8013331C
// Bone 0x2B also keeps the two matrices vfn_A0 places mFollowers[0] and mFollowers[1] at
extern "C" void kijiBones3()
{
	copyAnimalBones(1, 3, sBone);
	switch (sBone) {
	case 0x2B:
		njGetMatrix(gAnimalExtraMatrices[0]);
		njGetMatrix(gAnimalExtraMatrices[1]);
		break;
	}
	sBone++;
}
