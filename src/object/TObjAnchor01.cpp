#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "egg/ObjEgg.h"
#include "object/ObjScript.h"
#include "object/ObjCommon.h"
#include "engine/DrawTask.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "enemy/EnemyLight.h"
#include "stage/StageObjUtil.h"
#include "object/ObjAction.h"
#include "engine/Colli.h"
#include "camera/TCamera.h"
#include "Ninja/ninja.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "sound/Sound3DPlay.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "engine/GroundQuery.h"
#include "object/TObjAnchor01.h"

class TSetGenerator;

extern "C" {
const f32 gAnchor01SetRange = 800.0f;

// 0x802C3C30
ObjClassName gTObjAnchor01Name = { "TObjAnchor01", 0 };
}

class TObjAnchor01;

typedef char TObjAnchor01_size_check[(sizeof(TObjAnchor01) == 0x10C) ? 1 : -1];

// 0x8002AAB0
extern "C" TObject* createTObjAnchor01(TObject* parent, SetDesc* desc)
{
	return new TObjAnchor01(parent, desc);
}

// 0x8002AB04
void TObjAnchor01::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mSpeed = desc->getInt(0) + 500;
	mStep  = (int)((f32)mSpeed / 4.4f);
	mRange = desc->getInt(1) + 10000;
	mScale = 1.0f + desc->getFloat(0);
	mRate  = 100.0f;
}

// 0x8002ABD8
void TObjAnchor01::debugPrint(void*)
{
	njPrintC(0xA0013, "+500:SPEED");
	njPrintC(0xA0014, "+10000:RANGE");
	njPrintC(0xA0017, "+1.0:SCALE");
}

// 0x8002AC34
TObjAnchor01::TObjAnchor01(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjAnchor01Name.mName;
	mSize = 0x10C;
	initFromDesc(desc);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	vfn_74();
}

// 0x8002ACDC
TObjAnchor01::~TObjAnchor01()
{
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDraw);
}

// 0x8002AD6C
void TObjAnchor01::vfn_74()
{
	mModel   = getGeObjModels()[2];
	mTexture = getGeObjTextures()[2];
	mShape   = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 9, 0xF7, 0xF6);
	fitShape();
	mLeft    = mSpeed;
	mLive    = 1;
	mGroundY = 0.0f;
	placeShadow();
	mLight.mTimer = 0;
}

// 0x8002AE10
void TObjAnchor01::showHits()
{
	ContactQuery query;
	query.findGroup((CharaColli*)mShape, 8);
}

// 0x8002AE5C
// Swings the anchor by mRange and rings each time it crosses the middle
void TObjAnchor01::swing()
{
	f32 now  = njSin(mPhase);
	f32 next = njSin(mPhase + mLeft * 20);
	mRot.z   = (u16)(int)((f32)mRange * now);
	if (now >= 0.0f && next < 0.0f && mLive == 0) {
		mLive = 1;
		sePlay3D(0x20E, &mPos);
	} else if (now < 0.0f && next >= 0.0f && mLive == 1) {
		mLive = 0;
		sePlay3D(0x20E, &mPos);
	}
	mPhase += mLeft;
	mPhase   = (u16)mPhase;
	int slow = mStep;
	int fast = mSpeed;
	mLeft    = (int)((f32)(fast - slow) / 2.0f * njCos(mPhase << 1) + (f32)(fast + slow) / 2.0f);
	mLeft    = (u16)mLeft;
}

// 0x8002AFF4
void TObjAnchor01::draw()
{
	Vec3f at;
	Vec3f size;
	ColliShape* shape;

	if (!isInRange(this, &mPos, 850.0f)) {
		mFlags |= 1;
		return;
	}
	swing();
	fitShape();
	if (shadowIsEnabled(&mShadow) != 0) {
		shape  = colliGetShapes(mShape);
		at.x   = shape->mPos.x;
		at.y   = mGroundY;
		shape  = colliGetShapes(mShape);
		at.z   = shape->mPos.z;
		size.x = 0.0f;
		size.y = 1.0f;
		size.z = 0.0f;
		shadowSetGround(&mShadow, &at, &size);
	}
	if (objDebugEnabled(this) == 0) {
		showHits();
	}
}

// 0x8002B0C4
void TObjAnchor01::vfn_10()
{
	if (mLight.isVisible(mDraw, 750.0f, getPos(), 120.0f * mScale, 0) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x8002B14C
void TObjAnchor01::render()
{
	u32* piece;

	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY(mRot.y);
	njPushMatrixEx();
	njScale(0, mScale, mScale, mScale);
	((NJS_OBJECT*)mModel)->child->evalflags &= ~2u;
	((NJS_OBJECT*)mModel)->child->ang[2] = mRot.z;
	njSetTexture(mTexture);
	mLight.drawObject(mModel);
	njPopMatrixEx();
	njPopMatrixEx();
	shadowSetAngle(&mShadow, mRot.y);
}

// 0x8002B1EC
void TObjAnchor01::fitShape()
{
	ColliShape shape;

	colliShapeInit(&shape);
	shape.mType  = 1;
	shape.mSizeX = 30.0f * mScale;
	shape.mSizeY = 18.0f * mScale;
	shape.mSizeZ = 6.0f * mScale;
	shape.mPos.y = -(f32)mRate * mScale;
	shape.mFlags |= 8;
	njPushMatrixEx();
	njUnitMatrix(0);
	njTranslateEx(&mPos);
	njRotateY(0, mRot.y);
	njRotateZ(0, mRot.z);
	njCalcPointEx(&shape.mPos, &shape.mPos);
	shape.mRotY = mRot.y;
	njPopMatrixEx();
	colliCopyShapes(mShape, &shape);
	colliRefreshBounds(mShape);
}

// 0x8002B2C0
static inline void shadowSetSize(ObjShadow* shadow, f32 w, f32 h)
{
	shadow->_30 = w;
	shadow->_34 = h;
}

void TObjAnchor01::placeShadow()
{
	EggHits query;

	if (meshColliProbe((EggHit*)&query, (const Vec3f*)&mPos, 0xD) != 0 && (query.mMask & 1) != 0) {
		shadowSetFromContact(&mShadow, (u8*)&query);
		shadowSetSize(&mShadow, 60.0f * mScale, 12.0f * mScale);
		mGroundY = query.mHits[0].mPos.y;
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
}

// 0x8002B36C
int TObjAnchor01::vfn_48(TObjAction* other)
{
	if (other->getKind() == 0) {
		return 0x32;
	}
	return 0;
}

// 0x8002B3AC
int TObjAnchor01::onHit(const TObjAction* other, int shape, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	resetHitInfo(hit);
	hit->mDamage = 0x32;
	hit->mFlags.setBit(0xD);
	return 1;
}

// 0x8002B3F4
// In the stage's init table with the one after it
extern "C" void initSetGenerator()
{
	gSetGenerator = 0;
}

// 0x8002B400
extern "C" void exitSetGenerator()
{
	gSetGenerator = 0;
}
