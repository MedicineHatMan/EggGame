#include "egg/EggCreature.h"
#include "engine/DrawTask.h"
#include "stage/Stage.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "engine/GroundQuery.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "engine/Colli.h"
#include "camera/TCamPath.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "io/ResBank.h"
#include "object/ObjShadow.h"
#include "egg/TEggCreatureTypeE.h"
#include "sound/Sound3DPlay.h"
#include "engine/Color.h"
#include "animal/TAniSaruKoumori.h"
#include "animal/TAnimalRacer.h"
struct DrawStateSlot;
struct DrawState;
struct ColliShape;
struct DrawTask;
struct Fx;
struct CharaColli;

extern "C" {
// 0x8025EB70
MotionEntry gSaruKoumoriMotions[10] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 15.0f, gSaruRate20, 0.0f, 0 }, { MOTION_LOOP, 1, -1, 0, 0.0f, 30.0f, gSaruRate30, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, gSaruRate55, 0.0f, 0 }, { MOTION_LOOP, 3, -1, 0, 0.0f, 15.0f, gSaruRate80, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 30.0f, gSaruRate55, 0.0f, 0 }, { MOTION_ONCE, 4, -1, 0, 0.0f, 55.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, gSaruRate55, 0.0f, 0 }, { MOTION_ONCE, 1, -1, 0, 0.0f, 30.0f, gSaruRate30, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 30.0f, gSaruRate55, 0.0f, 0 }, { MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, gSaruRate40, 0.0f, 0 },
};
ColliShape sSaruKoumoriFirstShape[] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 5.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
ColliShape sSaruAttackEndShape[] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 20.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

typedef char TAniSaruKoumori_size_check[(sizeof(TAniSaruKoumori) == 0x3C0) ? 1 : -1];

extern "C" {
void saruBones0();
void saruBones1();
void saruBones2();
void saruBones3();

// 0x80249360
// called as the model walks each of its four sections
void* const sSaruKoumoriCallbacks[5] = { (void*)saruBones0, (void*)saruBones1, (void*)saruBones2, (void*)saruBones3, 0 };

// 0x802C4370
// the motion rates update writes into gSaruKoumoriMotions
f32 gSaruRate20 = 0.2f;
f32 gSaruRate30 = 0.3f;
f32 gSaruRate55 = 0.55f;
f32 gSaruRate80 = 0.8f;
f32 gSaruRate40 = 0.4f;
}

const char* gTAniSaruKoumoriName = "TAniSaruKoumori";

// 0x8013076C
TAniSaruKoumori::TAniSaruKoumori(TObject* parent, int player, int kind, const Vec3f* pos)
    : TEggCreatureTypeE(parent, player, kind, pos)
{
	_00    = (u32)gTAniSaruKoumoriName;
	mSize  = 0x3C0;
	mNodes = gSaruKoumoriMotions;
	mShape = sSaruKoumoriFirstShape;
	mColli = (CharaColli*)colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)mObjId);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		colliCopyShapes(mColli, mShape);
		colliPlaceShapes(mColli, &mPos, mShape);
	}
	m270 = sSaruKoumoriCallbacks;

	mStart.x       = 0.0f;
	mStart.y       = 0.0f;
	mStart.z       = 0.0f;
	mSwing         = 0.0f;
	mSwingSpeed    = 0.0f;
	mStartRot[0]   = 0;
	mStartRot[1]   = 0;
	mStartRot[2]   = 0;
	mSpeed         = 0.0f;
	mTravelled     = 0.0f;
	_3AC           = 0.0f;
	mElectric      = 0;
	mElectricPos.x = 0.0f;
	mElectricPos.y = 0.0f;
	mElectricPos.z = 0.0f;
}

inline void TAniSaruKoumori::killElectric()
{
	if (mElectric != 0) {
		mElectric->mFlags |= 1;
		mElectric = 0;
	}
}

// 0x80130878
TAniSaruKoumori::~TAniSaruKoumori()
{
	killElectric();
	if (mFx != 0) {
		fxRemove(mFx);
		mFx = 0;
	}
	colliFree(mColli);
	mColli = 0;
}

// 0x80130920
// The powered up kind 0x14 draws with an extra TEV stage while it is out
void TAniSaruKoumori::render()
{
	if (mAsset.mLink.mList == 0) {
		return;
	}
	if (mMotion == 0) {
		return;
	}
	if (mKind != 0x14) {
		TEggCreatureTypeE::render();
		return;
	}
	if (mMode == 1 && mStep == 0) {
		TEggCreatureTypeE::render();
		return;
	}

	setDrawAmbient(&mLight, &mDrawState);
	DrawStateSlot* slot = getDrawStateSlot();
	njPushMatrixEx();
	njMultiMatrix(0, mNodeMatrix);
	beginDrawState(slot, (const DrawState*)&mDrawState);
	njSetTexture(*(void**)(*(u8**)(gAnimalCommonModel + 4) + 8));
	gjLoadTextureNum(0, 0, 0, 1);

	f32 mtx[12];
	PSMTXCopy(_gj_matrix_, mtx);
	mtx[3]  = 0.0f;
	mtx[7]  = 0.0f;
	mtx[11] = 0.0f;
	GXLoadTexMtxImm(mtx, GX_TEXMTX5, GX_MTX3x4);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)modelPostTevAdd);
	gjSetPostTexGenCallback((void*)modelPostTexGenNormal);
	_nj_control_3d_flag_ |= 0x80000;
	njSetTexture(mTexList);
	beginTexAnim();
	shadowAlphaBegin();
	if (mActor != 0) {
		gjDrawActor(mActor);
	} else {
		debugWireSphere(gjGetMatrix(), &gColorBlack);
	}
	shadowAlphaEnd();
	endTexAnim();
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= ~0x80000;
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
}

// The DOL copies the start rotation as a block paired loads as for a struct holding an array
struct SaruRotBlock {
	int v[3];
};

// 0x80130AB0
void TAniSaruKoumori::onPhase(int phase)
{
	if (phase == 0) {
		setMotionSpeed(1.0f);
		playMotion(3, 1, 0.075f);
		eggTypeStopAllFx(this);
		startFollowers();
		mTimer                     = 90;
		*(int*)((u8*)this + 0x380) = 0;
		mStep                      = 0x11;

		f32* values = mParams->mValues;
		u8 shape[0x30];
		if ((int)values[4] == 0) {
			colliShapeMakeSphere((ColliShape*)shape, &gVecZero, 1, values[5]);
		} else {
			colliShapeMakeCylinder((ColliShape*)shape, &gVecZero, 0, values[5], values[6]);
		}
		colliCopyShapes(mColli, (const ColliShape*)shape);
		colliShapeSetBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0xFF, 0xFF, 0x10);

		mElectricPos           = mPos;
		TAniElectric* electric = new TAniElectric(this);
		mElectric              = electric;
		mElectric->mPosRef     = &mElectricPos;

		mtxSetTransV(mFxMatrix, &mPos);
		if (mKind == 0x14) {
			mFx = fxSpawnFollow(1, mFxBank, 1, mFxMatrix);
		} else {
			mFx = fxSpawnFollow(1, mFxBank, 1, mFxMatrix);
		}
		sePlay3D(0x1B7, &mPos);

		mStart                     = mPos;
		mSwing                     = 0.0f;
		mSwingSpeed                = mParams->mValues[8];
		mRot[0]                    = 0;
		mRot[1]                    = 0;
		mRot[2]                    = 0;
		mRot[1]                    = vecAngleXZ(&mDir);
		*(SaruRotBlock*)&mStartRot = *(SaruRotBlock*)&mRot;
		mSpeed                     = mParams->mValues[7];
		mTravelled                 = 0.0f;
		_3AC                       = 0.0f;
		mSubMode                   = 5;
	} else if (phase == 2) {
		switch (mStep) {
		case 0x11:
			runAttack();
			break;
		case 0x12:
			runEnd();
			break;
		}
	} else if (phase == 1) {
		mShape = sSaruKoumoriFirstShape;
		colliCopyShapes(mColli, mShape);
		colliShapeClearBit8(mColli, 0);
		colliSetMasks(mColli, 0x20, 0, 0, 0x10);
		killElectric();
		if (mFx != 0) {
			fxRemove(mFx);
			mFx = 0;
		}
		eggTypeStopAllFx(this);
		clearExtraFx();
	}
}

/** v scaled by s returned by value */
static inline Vec3f saruScale(f32 s, const Vec3f* v)
{
	Vec3f r;
	gjScaleVector(s, &r, v);
	return r;
}

/** a plus b returned by value */
static inline Vec3f saruAdd(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjAddVector(&v, a, b);
	return v;
}

/** Adds v scaled by s to out */
static inline void saruAddScaled(Vec3f* out, f32 s, const Vec3f* v)
{
	Vec3f r = saruScale(s, v);
	njAddVector(out, &r);
}

/** a plus b through one more inline */
static inline Vec3f saruAhead(const Vec3f* a, const Vec3f* b)
{
	return saruAdd(a, b);
}

// 0x80130D98
void TAniSaruKoumori::runAttack()
{
	mSwing += mSwingSpeed;
	if (mSwingSpeed > 0.0f) {
		if (mSwing > 40.0f) {
			mSwing      = 40.0f;
			mSwingSpeed = -mSwingSpeed;
		}
	} else if (mSwingSpeed < 0.0f) {
		if (mSwing < -40.0f) {
			mSwing      = -40.0f;
			mSwingSpeed = -mSwingSpeed;
		}
	}

	Vec3f side;
	side.x = 1.0f;
	side.y = 0.0f;
	side.z = 0.0f;

	f32 mtx[12];
	mtxIdentity(mtx);
	mtxRotateY(mtx, mtx, mStartRot[1]);
	mtxMultVecSR(mtx, &side, &side);
	gjScaleVector(mSwing, &side, &side);

	saruAddScaled(&mStart, mSpeed, &mDir);

	Vec3f next = saruAhead(&mStart, &side);
	mtxSetTransV(mFxMatrix, &mPos);

	EggHits hits;
	EggProbe probe;
	if (meshColliProbeLine((EggHit*)&hits, (const Vec3f*)&mPos, (const Vec3f*)&next, (unsigned long)((-1))) != 0 || touchedTarget() != 0
	    || mTravelled >= mParams->mValues[3]) {
		sePlay3D(0x1B8, &mPos);
		fxRemove(mFx);
		mtxSetTransV(mFxMatrix, &mPos);
		if (mKind == 0x14) {
			mFx = fxSpawnAt(1, mFxBank, 5, mFxMatrix);
		} else {
			mFx = fxSpawnAt(1, mFxBank, 5, mFxMatrix);
		}
		notifyHolder();
		mShape = sSaruAttackEndShape;
		colliCopyShapes(mColli, mShape);
		colliShapeSetBit8(mColli, 0);
		mStep = 0x12;
		killElectric();
	} else {
		mPos    = next;
		mRot[1] = (mRot[1] + 0x1000) & 0xFFFF;
		mtxSetTransV(mtx, &mPos);
		mtxRotateY(mtx, mtx, mRot[1]);
		mtxConcat((f32*)mtx, (const f32*)mtx, gAnimalExtraMatrices[0]);
		mtxGetPos(mtx, &mElectricPos);
		mTravelled += mSpeed;

		next         = mElectricPos;
		Vec3f ground = mElectric->mGround;
		f32 height   = next.y - ground.y;
		colliShapeSetPos(colliGetShapes(mColli), &next);
		colliGetShapes(mColli)->mSizeY = height;
	}
}

// 0x801310F0
// The shape grows while the timer runs out
void TAniSaruKoumori::runEnd()
{
	if (mTimer == 0) {
		if (mFx != 0) {
			fxRemove(mFx);
			mFx = 0;
		}
		endAttack();
	} else {
		mTimer--;
		if (colliGetShapes(mColli)->mSizeX < 60.0f) {
			colliGetShapes(mColli)->mSizeX += 5.0f;
		}
	}
}

// 0x80131180
int TAniSaruKoumori::onHit(const TObjAction* other, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	if (other->mObjId == mHolderId) {
		return 0;
	}
	resetHitInfo(hit);
	hit->mDamage = ((int*)mParams->mValues)[1];
	hit->mFlags.setBit(6);
	hit->_14 = mHitValue;
	return 1;
}

static int sBoneCount; // 0x802C5670

// 0x801311F4
void TAniSaruKoumori::resetBones()
{
	sBoneCount = 0;
}

// The model calls these as it walks each of its four sections
// 0x80131200
extern "C" void saruBones0()
{
	copyAnimalBones(3, 0, sBoneCount);
	sBoneCount++;
}

// 0x80131298
extern "C" void saruBones1()
{
	copyAnimalBones(3, 1, sBoneCount);
	sBoneCount++;
}

// 0x80131330
extern "C" void saruBones2()
{
	copyAnimalBones(3, 2, sBoneCount);
	sBoneCount++;
}

// 0x801313C8
extern "C" void saruBones3()
{
	copyAnimalBones(3, 3, sBoneCount);
	switch (sBoneCount) {
	case 0xE:
		njGetMatrix(gAnimalExtraMatrices[0]);
		njGetMatrix(gAnimalExtraMatrices[1]);
		break;
	case 2:
		njGetMatrix(gAnimalExtraMatrices[2]);
		break;
	}
	sBoneCount++;
}
