#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "object/ObjScript.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"
#include "sound/SoundHandle.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "camera/TCamera.h"
#include "core/Pad.h"
#include "object/ObjAction.h"
#include "engine/Viewport.h"
#include "Ninja/ninja.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjShadow.h"
#include "camera/TCamPath.h"
#include "core/MathUtil.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "engine/TFrameBlur.h"
#include "object/TObjEggCannon.h"
struct SetLink;
struct DrawState;

extern "C" {
const f32 gEggCannonSetRange = 800.0f;

// 0x802C40C8
ObjClassName gTObjEggCannonName = { "TObjEggCannon", 0 };
}

/** Message 0x16 to an egg get in this cannon */
struct CannonEnterMsg {
	u16 mType;    // _00
	u8 _02[2];
	u16 mCannon;  // _04 the cannon's id
	u16 _06;      // _06
	u8 _08[8];
};

/** Message 0x23 to an egg fly off at this velocity */
struct CannonShotMsg {
	u16 mType;  // _00
	Vec3f mVel; // _04
};

/** One corner of the scope's shade a Ninja 2D polygon vertex */
struct ScopeVertex {
	f32 x, y, z;
	u32 mColor; // ARGB
};

// 0x80280570
// The shade around the scope a strip from a ring of radius 200 to one of 400 round the screen's middle then two closing
// corners
static ScopeVertex sScopeShade[66];

typedef char TObjEggCannon_size_check[(sizeof(TObjEggCannon) == 0x20C) ? 1 : -1];

// 0x800D3B5C
extern "C" void initTObjEggCannon()
{
	int angle = 0;
	int i     = 0;
	for (; i < 64; i += 2) {
		f32 s = njSin(angle);
		f32 c = njCos(angle);
		ScopeVertex* v = &sScopeShade[i];
		int grey    = (u8)(int)(32.0f * s + 32.5f);
		v[0].x      = 400.0f * c + 320.0f;
		v[0].y      = 400.0f * s + 240.0f;
		v[0].z      = 0.5f;
		v[0].mColor = 0xFF000000;
		v[1].x      = 200.0f * c + 320.0f;
		v[1].y      = 200.0f * s + 240.0f;
		v[1].z      = 0.5f;
		v[1].mColor = 0xFF000000 | (grey << 16) | (grey << 8) | grey;
		angle += 0x800;
	}
	sScopeShade[i].x          = 720.0f;
	sScopeShade[i].y          = 240.0f;
	sScopeShade[i].z          = 0.5f;
	sScopeShade[i].mColor     = 0xFF000000;
	sScopeShade[i + 1].x      = 520.0f;
	sScopeShade[i + 1].y      = 240.0f;
	sScopeShade[i + 1].z      = 0.5f;
	sScopeShade[i + 1].mColor = 0xFF202020;
}

// 0x800D3D04
extern "C" void termTObjEggCannon() { }

// 0x800D3D08
extern "C" TObject* createTObjEggCannon(TObject* parent, SetDesc* desc)
{
	return new TObjEggCannon(parent, desc);
}

// 0x800D3D5C
void TObjEggCannon::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mPower     = (f32)desc->getInt(0);
	mFixTime   = desc->getInt(1);
	mLightPos  = mPos;
	mLightPos.y += 20.0f;
	mLightPos.z += -10.0f;
	mPitchMin  = (int)(182.04445f * desc->getFloat(0));
	mYawMin    = (int)(182.04445f * desc->getFloat(2));
	mRollMin   = 0;
	mPitchMax  = (int)(182.04445f * desc->getFloat(1));
	mYawMax    = (int)(182.04445f * desc->getFloat(3));
	mRollMax   = 0;
	mPitch     = mPitchMin;
	mYaw       = (mYawMin + mYawMax) / 2;
	mRoll      = 0;
}

// 0x800D3F0C
void TObjEggCannon::debugPrint(void*)
{
	njPrintC(0xA0013, ":POWER");
	njPrintC(0xA0014, ":FIX TIME");
	njPrintC(0xA0017, "DEG:PITCH MIN");
	njPrintC(0xA0018, "DEG:PITCH MAX");
	njPrintC(0xA0019, "DEG:YAW MIN");
	njPrintC(0xA001A, "DEG:YAW MAX");
}

// 0x800D3F98
TObjEggCannon::TObjEggCannon(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjEggCannonName.mName;
	mSize = 0x20C;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	makeShape();
	initFromDesc(desc);
	init();
}

// 0x800D4074
TObjEggCannon::~TObjEggCannon()
{
	seStop3D(&mSnd);
	freeShape();
	removeDrawTask(mDraw);
}

// 0x800D4110
void TObjEggCannon::init()
{
	mCamera = 0;
	mTimer  = 0;
	mState  = CANNON_WAIT;
	mFlags2 = 0;
	mFlags2 |= CANNON_DRAW_MODEL;
	shadowSetGround(&mShadow, &mPos, 0);
	mShadow._30 = 40.0f;
	mShadow._34 = 40.0f;
	shadowEnable(&mShadow);
	setLinkFree((SetLink*)&mLink);
	mLight.mTimer = 0;
	mModel.setModel(getStgObjModels()[0x23]);
	mModel.mTexList = getStgObjTextures()[0x22];
	mModel.mState   = &mDrawState;
	updateMatrix();
	placeSpheres();
}

// 0x800D41C4
void TObjEggCannon::makeShape()
{
	mShape = colliCreate(2, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 0xF6, 0xF6);
}

// 0x800D4220
// Sphere 0 is the mouth an egg goes in by
void TObjEggCannon::placeSpheres()
{
	Vec3f pos;
	pos.x = 0.0f;
	pos.y = 15.0f;
	pos.z = 10.0f;
	vecRotateY(&pos, &pos, mRot.y);
	vecAdd(&pos, &pos, &mPos);
	colliShapeMakeSphere(colliGetShape(mShape, 0), &pos, 0, 12.0f);
	colliGetShape(mShape, 0)->mFlags |= 0x10;
	pos.x = 0.0f;
	pos.y = 15.0f;
	pos.z = -10.0f;
	vecRotateY(&pos, &pos, mRot.y);
	vecAdd(&pos, &pos, &mPos);
	colliShapeMakeSphere(colliGetShape(mShape, 1), &pos, 0, 25.0f);
}

// 0x800D4308
void TObjEggCannon::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x800D4340
void TObjEggCannon::draw()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 850.0f) == 0 && mState == CANNON_WAIT) {
		mFlags |= 1;
		return;
	}
	switch (mState) {
	case CANNON_WAIT:
		stateWait();
		break;
	case CANNON_AIM:
		stateAim();
		break;
	case CANNON_FIRED:
		stateFired();
		break;
	}
}

// 0x800D43E0
void TObjEggCannon::setState(int state)
{
	mState = state;
}

// 0x800D43E8
// An egg touching the mouth is asked in
void TObjEggCannon::stateWait()
{
	for (ColliContact* contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (contact->mShape != 0 || (colliContactShape(contact)->mFlags & 0x400) == 0) {
			continue;
		}
		mEggId = colliContactOwner(contact);
		mEgg   = findObjById(mEggId);
		if (mEgg == 0) {
			continue;
		}
		CannonEnterMsg msg;
		msg.mType   = 0x16;
		msg.mCannon = 0xFFFF;
		msg.mCannon = mObjId;
		msg._06     = 0;
		if (mEgg->sendToHandle((ObjMsg*)&msg) == 0) {
			mEgg   = 0;
			mEggId = 0xFFFF;
			continue;
		}
		colliDisable(mShape);
		mFlags2 &= ~CANNON_DRAW_MODEL;
		mFlags2 |= CANNON_DRAW_SCOPE;
		startCamera();
		setState(CANNON_AIM);
		break;
	}
	if (objDebugEnabled(this) != 0) {
		updateMatrix();
	}
}

// 0x800D4514
void TObjEggCannon::stateAim()
{
	if (checkEgg() == 0) {
		endCamera();
		setState(CANNON_FIRED);
		return;
	}
	if ((padGet(0)->mPressed & 0x100) && fire() != 0) {
		frameBlurStart(15, 0.8f);
		seStop3D(&mSnd);
		padVibStart(0, (int)this, 0xFF, 0, 0x21);
		setState(CANNON_FIRED);
		return;
	}
	PadState* pad = padGet(0);
	int angle     = (int)(f32)pad->mAngle;
	f32 len       = pad->mLength;
	int pitch     = (int)(256.0f * (len * njSin(angle)));
	int yaw       = (int)(-256.0f * (len * njCos(angle)));
	mPitch += pitch;
	int pitching = pitch != 0;
	mYaw += yaw;
	int yawing = yaw != 0;
	if (mPitch > mPitchMax) {
		mPitch   = mPitchMax;
		pitching = 0;
	} else if (mPitch < mPitchMin) {
		mPitch   = mPitchMin;
		pitching = 0;
	}
	if (mYaw > mYawMax) {
		mYaw   = mYawMax;
		yawing = 0;
	} else if (mYaw < mYawMin) {
		mYaw   = mYawMin;
		yawing = 0;
	}
	updateCamera();
	if (pitching || yawing) {
		sePlay3DHeld(500.0f, &mSnd, 0x207, 0, &mPos);
	} else {
		seStop3D(&mSnd);
	}
}

// 0x800D4714
// After mFixTime frames the cannon is empty again aimed at its rest
void TObjEggCannon::stateFired()
{
	mTimer++;
	if (mTimer > mFixTime) {
		endCamera();
		mEgg    = 0;
		mEggId  = 0xFFFF;
		mFlags2 = 0;
		mFlags2 |= CANNON_DRAW_MODEL;
		mTimer = 0;
		colliEnable(mShape);
		mPitch = mPitchMin;
		mYaw   = (mYawMin + mYawMax) / 2;
		setState(CANNON_WAIT);
	}
}

// 0x800D47B8
void TObjEggCannon::startCamera()
{
	if (mCamera == 0) {
		Vec3f zero = gVecZero;
		Vec3f at   = gVecZero;
		Vec3f eye  = gVecZero;
		at.z       = 1.0f;
		mCamera    = createTCamFix(0, 3, &at, &eye);
		updateCamera();
	}
}

// 0x800D484C
void TObjEggCannon::endCamera()
{
	if (mCamera != 0) {
		mCamera->mFlags |= 1;
		mCamera = 0;
	}
}

// 0x800D4870
// The eye sits behind the barrel and looks along it
void TObjEggCannon::updateCamera()
{
	if (mCamera != 0) {
		Vec3f pos;
		pos.x = 0.0f;
		pos.y = 0.0f;
		pos.z = -100.0f;
		f32 mtx[12];
		mtxSetTransV(mtx, &mPos);
		mtxTranslate((float*)mtx, (const float*)mtx, 0.0f, 15.0f, 0.0f);
		mtxRotateY(mtx, mtx, mYaw + mRot.y);
		mtxRotateX(mtx, mtx, mPitch);
		mtxTranslate((float*)mtx, (const float*)mtx, 0.0f, 0.0f, -60.0f);
		mtxMultVec(mtx, &pos, &pos);
		mCamera->setAt(&pos);
		pos.x = 0.0f;
		pos.y = 0.0f;
		pos.z = -30.0f;
		mtxMultVec(mtx, &pos, &pos);
		mCamera->setEye(&pos);
	}
}

// 0x800D4980
// Forgets the egg once it has gone
BOOL TObjEggCannon::checkEgg()
{
	TObjAction* egg = findObjById(mEggId);
	if (egg == 0 || egg != mEgg) {
		mEggId = 0xFFFF;
		mEgg   = 0;
		return FALSE;
	}
	return TRUE;
}

// 0x800D49E4
BOOL TObjEggCannon::fire()
{
	if (mEgg == 0) {
		return FALSE;
	}
	CannonShotMsg msg;
	msg.mType   = 0x23;
	msg.mVel.set(0.0f, 0.0f, -mPower);
	f32 mtx[12];
	mtxSetRotY(mtx, mYaw + mRot.y);
	mtxRotateX(mtx, mtx, mPitch);
	mtxMultVec(mtx, &msg.mVel, &msg.mVel);
	if (mEgg->sendToHandle((ObjMsg*)&msg) != 0) {
		sePlay3D(0x208, &mPos);
		return TRUE;
	}
	return FALSE;
}

// 0x800D4AB0
void TObjEggCannon::updateMatrix()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, mRot.y);
}

// 0x800D4AF4
// Node 0 is the barrel's mouth raised by the egg's radius
void TObjEggCannon::getNodeMatrix(int node, f32* mtx)
{
	if (node != 0 || checkEgg() == 0) {
		return;
	}
	mtxSetTransV(mtx, &mPos);
	mtxTranslate(mtx, (const float*)mtx, 0.0f, 15.0f + mEgg->getRadiusV(), 0.0f);
	mtxRotateY(mtx, mtx, mYaw + mRot.y);
	mtxRotateX(mtx, mtx, mPitch);
}

// 0x800D4B9C
int TObjEggCannon::sendToHandle(ObjMsg*)
{
	return 0;
}

// 0x800D4BA4
void TObjEggCannon::vfn_10()
{
	if (mLight.isVisible(mDraw, 700.0f, &mLightPos, 50.0f, 0) != 0) {
		setDrawAmbient(&mLight, &mDrawState);
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x800D4C20
void TObjEggCannon::render()
{
	if (mFlags2 & CANNON_DRAW_MODEL) {
		shadowSetAngle(&mShadow, 0);
		njPushMatrixEx();
		njMultiMatrix(0, mMtx);
		mModel.draw();
		njPopMatrixEx();
	}
	if (mFlags2 & CANNON_DRAW_SCOPE) {
		drawScope();
	}
}

// 0x800D4C90
// The shade then a black cross at the middle of the view
void TObjEggCannon::drawScope()
{
	f32 x;
	f32 y;
	f32 w;
	f32 h;
	getViewport(&x, &y, &w, &h);
	f32 cx = 0.5f * w + x;
	f32 cy = 0.5f * h + y;
	njPolygonCullingMode(0);
	njDrawPolygon2DExStart(0);
	njDrawPolygon2DExSetData((const NJS_POLYGON_VTX*)sScopeShade, (s32)66);
	njDrawPolygon2DExEnd();
	njPolygonCullingMode(0x10000000);
	f32 cross[4][2] = {
		{ cx - 32.0f, cy },
		{ 32.0f + cx, cy },
		{ cx, cy - 32.0f },
		{ cx, 32.0f + cy },
	};
	njDrawLineExStart(1.0f, 0xFF000000, 0);
	njDrawLine2DExSetList((const f32*)cross, 1.0f, 4);
	njDrawLineExEnd();
}
