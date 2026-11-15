#include "object/purple/TObjPurpleIceGearS.h"
#include "engine/DrawTask.h"
#include "stage/EventScript.h"
#include "object/ObjScript.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "object/TObjLauncher.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "stage/TGameState.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "object/purple/TObjPurpleIceGearL.h"
struct Vec3f;
struct CharaColli;
struct SetLink;

extern "C" {

// 0x802C4208
ObjClassName gTObjPurpleIceGearSName = { "TObjPurpleIceGearS", 0 };
}

typedef char TObjPurpleIceGearS_size_check[(sizeof(TObjPurpleIceGearS) == 0x144) ? 1 : -1];

// 0x80109BE8
extern "C" TObject* createTObjPurpleIceGearS(TObject* parent, SetDesc* desc)
{
	return new TObjPurpleIceGearS(parent, desc);
}

// 0x80109C3C
TObjPurpleIceGearS::TObjPurpleIceGearS(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00       = (u32)gTObjPurpleIceGearSName.mName;
	mSize     = 0x144;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mGeo      = findGeObjData("MODEL_ICE_GEAR_S");
	mTex      = findGeObjData("TLS_MODEL_ICE_GEAR_S");
	makeShape();
	mAngle = 0;
	initFromDesc(desc);
	vfn_74();
}

// 0x80109D1C
TObjPurpleIceGearS::~TObjPurpleIceGearS()
{
	seStop3D(&mSound);
	freeShape();
	removeDrawTask(mDrawTask);
}

// 0x80109DA0
void TObjPurpleIceGearS::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mEvent    = (u16)desc->getInt(2);
	mSwitch   = (u16)desc->getInt(3);
	mRotSpeed = desc->getInt(0);
	mScale    = 1.0f + desc->getFloat(0);

	mtxSetTransV(mMatrix, &mPos);
	mtxScale(mScale, mScale, mScale, mMatrix, mMatrix);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxRotateX(mSpin, mMatrix, mAngle);

	mCentre        = ((GearNode*)mGeo)->mMesh->mCentre;
	mtxMultVec(mSpin, &mCentre, &mCentre);
	mRadius = ((GearNode*)mGeo)->mMesh->mRadius * mScale;
	placeShape();
}

// 0x80109EDC
void TObjPurpleIceGearS::debugPrint(void*)
{
	njPrintC(0xA0013, ":ROT SPD");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, ":CALL EVENT ID");
	njPrintC(0xA0016, ":SW ID");
	njPrintC(0xA0017, "+1.0:SCALE");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x80109F88
void TObjPurpleIceGearS::vfn_74()
{
	mModel.setModel(mGeo);
	mModel.mTexList = (NJS_TEXLIST*)mTex;
	mModel.mState   = (DrawState*)getDefaultDrawState();
	if (keyBitIsSet(mSwitch) == 1 && objDebugEnabled(this) == 0) {
		mDrawTask->mFlags |= 1;
		mFlags |= 1;
		setLinkForever((SetLink*)&mLink);
	} else {
		mAct = 0;
	}
	setLinkFree((SetLink*)&mLink);
}

// 0x8010A024
void TObjPurpleIceGearS::makeShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xD6, 0xD6);
}

// 0x8010A080
void TObjPurpleIceGearS::placeShape()
{
	ColliShape shape;
	colliShapeMakeSphere(&shape, &mPos, 0, 20.0f * mScale);
	colliCopyShapes(mShape, &shape);
}

// 0x8010A0D0
void TObjPurpleIceGearS::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x8010A108
void TObjPurpleIceGearS::draw()
{
	if (objDebugEnabled(this) != 0) {
		mAngle += mRotSpeed;
	} else {
		if (!isInRange(this, &mPos, 900.0f)) {
			mFlags |= 1;
		} else {
			switch (mAct) {
			case 0:
				spin();
				break;
			case 1:
				shatter();
				break;
			case 2:
				finish();
				break;
			}
		}
	}
}

// 0x8010A1C0
// Group 0x20 is a guess at which contact can break the gear
void TObjPurpleIceGearS::spin()
{
	for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
		if (colliInGroup(hit->mOther, 0x20) == 0) {
			continue;
		}
		TObjAction* other = findObjById((u16)colliContactOwner(hit));
		if (other == 0) {
			continue;
		}
		HitInfo info;
		if (other->onHit(this, 0, (int*)&info) != 0 && (int)info.mFlags.testBit(4) != 0) {
			seStop3D(&mSound);
			mAct = 1;
		}
	}
	mAngle += mRotSpeed;
	mtxRotateX(mSpin, mMatrix, mAngle);
	sePlay3DHeld(500.0f, &mSound, 0x231, 100, &mCentre);
}

// 0x8010A2BC
void TObjPurpleIceGearS::shatter()
{
	ShatterList burst;
	burst.mModels = getGeObjModels();
	burst.mTexture = getGeObjTextures()[0];
	burst.mFirst = 3;
	burst.mModelCount = 1;
	burst.mCount = 10;
	burst.mScale = 1.0f;
	burst.mSpeed = 0.5f;

	Vec3f at = mCentre;
	at.y -= mRadius;
	Vec3f size;
	size.x = mRadius * mScale;
	size.y = size.x;
	size.z = size.x;
	shatterList(0, &at, mRot.y, &at, &size, &burst);
	sePlay3D(0x232, &mCentre);
	mTimer = 0x3C;
	mAct   = 2;
}

// 0x8010A394
void TObjPurpleIceGearS::finish()
{
	if (mTimer == 0) {
		setKeyBit(mSwitch);
		eventScriptRequest(mEvent);
		u16 msg = 0xA0;
		sendGameMsg((ObjMsg*)&msg);
		setLinkForever((SetLink*)&mLink);
		mFlags |= 1;
	}
	mTimer--;
}

// 0x8010A408
void TObjPurpleIceGearS::vfn_10()
{
	if (mAct != 0) {
		mDrawTask->mFlags |= 1;
	} else {
		u8 out[0x10];
		if (cameraIsVisible(800.0f, mRadius, &mCentre, (Vec3f*)out) != 0) {
			mDrawTask->mSortKey = drawTaskDepthKey(*(f32*)(out + 8));
			mDrawTask->mFlags &= ~1;
		} else {
			mDrawTask->mFlags |= 1;
		}
	}
}

// 0x8010A4A0
void TObjPurpleIceGearS::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mSpin);
	mModel.draw();
	njPopMatrixEx();
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
	}
}
