#include "object/purple/TObjPurpleIceGearL.h"
#include "camera/TCamera.h"
#include "engine/DrawTask.h"
#include "camera/EventCamArc.h"
#include "camera/EventCamRef.h"
#include "object/ObjScript.h"
#include "object/ObjCommon.h"
#include "object/ObjArchive.h"
#include "object/purple/TObjPurpleBlizzard.h"
#include "object/TObjLauncher.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "stage/TGameState.h"
#include "stl/math.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "object/TObjSnowman.h"
struct SetLink;

extern "C" {

// 0x802C4218
ObjClassName gTObjPurpleIceGearLName = { "TObjPurpleIceGearL", 0 };
}

typedef char TObjPurpleIceGearL_size_check[(sizeof(TObjPurpleIceGearL) == 0x154) ? 1 : -1];

// 0x8010BC5C
// The set table's init
extern "C" void iceGearSetInit()
{
}

// 0x8010BC60
// The set table's exit
extern "C" void iceGearSetExit()
{
}

// 0x8010BC64
extern "C" TObject* createTObjPurpleIceGearL(TObject* parent, SetDesc* desc)
{
	return new TObjPurpleIceGearL(parent, desc);
}

// 0x8010BCB8
TObjPurpleIceGearL::TObjPurpleIceGearL(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00       = (u32)gTObjPurpleIceGearLName.mName;
	mSize     = 0x154;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mGeo      = findGeObjData("MODEL_ICE_GEAR_L");
	mTex      = findGeObjData("TLS_MODEL_ICE_GEAR_L");
	mAngle    = 0;
	initFromDesc(desc);
	vfn_74();
}

// 0x8010BD98
TObjPurpleIceGearL::~TObjPurpleIceGearL()
{
	seStop3D(&mSound);
	removeDrawTask(mDrawTask);
}

// 0x8010BE20
void TObjPurpleIceGearL::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mSwitch   = (u16)desc->getInt(2);
	mRotSpeed = desc->getInt(0);
	mScale    = 1.0f + desc->getFloat(0);

	mtxSetTransV(mMatrix, &mPos);
	mtxScale(mScale, mScale, mScale, mMatrix, mMatrix);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxRotateX(mSpin, mMatrix, mAngle);

	mCentre        = ((GearNode*)mGeo)->mMesh->mCentre;
	mtxMultVec(mSpin, &mCentre, &mCentre);
	mRadius = ((GearNode*)mGeo)->mMesh->mRadius * mScale;
}

// 0x8010BF38
void TObjPurpleIceGearL::debugPrint(void*)
{
	njPrintC(0xA0013, ":ROT SPD");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, ":SWITCH ID");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "+1.0:SCALE");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x8010BFE4
void TObjPurpleIceGearL::vfn_74()
{
	mModel.setModel(mGeo);
	mModel.mTexList = (NJS_TEXLIST*)mTex;
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mHide1          = 0;
	mHide0          = 0;
	mEvent          = 0;
	if (keyBitIsSet(mSwitch) == 0) {
		sePlay3DHeld(2000.0f, &mSound, 0x235, 0, &mPos);
		mAct = 0;
	} else {
		mAct = 3;
	}
	setLinkFree((SetLink*)&mLink);
}

// 0x8010C07C
// 199 arms the camera The number is the message type
int TObjPurpleIceGearL::sendToHandle(ObjMsg* msg)
{
	if (msg->mType == 199) {
		mCam.set(msg);
		return 1;
	}
	return 0;
}

// 0x8010C0B8
void TObjPurpleIceGearL::draw()
{
	if (objDebugEnabled(this) != 0) {
		mAngle += mRotSpeed;
		if (mHide0 != 0) {
			mHide0->mFlags |= 4;
		}
		if (mHide1 != 0) {
			mHide1->mFlags |= 4;
		}
	} else {
		if (isInRange(this, &mPos, 5000.0f) || mCam.isActive() != 0) {
			switch (mAct) {
			case 0:
				spin();
				break;
			case 1:
				ease();
				break;
			case 2:
				settle();
				break;
			case 3:
				stopped();
				break;
			}
		}
	}
}

// 0x8010C1B0
void TObjPurpleIceGearL::spin()
{
	mAngle += mRotSpeed;
	mtxRotateX(mSpin, mMatrix, mAngle);
	if (mCam.isActive() == 0) {
		return;
	}
	if (mHide0 != 0) {
		mHide0->mFlags |= 1;
		mHide0 = 0;
	}
	if (mHide1 != 0) {
		mHide1->mFlags |= 1;
		mHide1 = 0;
	}
	seStop3D(&mSound);
	mEvent = eventCamArcPlay(1.0f, 0.0f, 0x100);
	((TCamera*)mEvent)->setCut();
	mAct = 1;
}

// 0x8010C274
// 0x50 and TObjPurpleBlizzard::freeEffects are a guess at a debug trigger
void TObjPurpleIceGearL::ease()
{
	mRotSpeed = (int)(0.98f * (f32)mRotSpeed);
	mAngle += mRotSpeed;
	mtxRotateX(mSpin, mMatrix, mAngle);
	if ((int)(f32)floor(mEvent->getFrame()) == 0x50 && gBlizzard != 0) {
		((TObjPurpleBlizzard*)gBlizzard)->freeEffects();
	}
	if (mEvent->isDone() != 0) {
		if (mEvent != 0) {
			mEvent->mFlags |= 1;
			mEvent = 0;
		}
		mCam.release();
		if (gBlizzard != 0) {
			((TObjPurpleBlizzard*)gBlizzard)->freeEffects();
		}
		u16 msg = 0x8E;
		sendGameMsg((ObjMsg*)&msg);
		mAct = 3;
	}
}

// 0x8010C388
// Nothing here stores 2 in mAct
void TObjPurpleIceGearL::settle()
{
	if (mEvent->isDone() != 0) {
		mEvent->mFlags |= 1;
		mEvent = 0;
		mCam.release();
		u16 msg = 0x8E;
		sendGameMsg((ObjMsg*)&msg);
		mAct = 3;
	}
}

// 0x8010C3F8
void TObjPurpleIceGearL::stopped()
{
}

// 0x8010C3FC
void TObjPurpleIceGearL::vfn_10()
{
	if (cameraIsVisible(5000.0f, mRadius, &mCentre, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8010C464
void TObjPurpleIceGearL::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mSpin);
	mModel.draw();
	njPopMatrixEx();
}

// 0x8010C4A8
// The set hook run when the stage loads empty
extern "C" void initPurpleIceGearLSet()
{
}

// 0x8010C4AC
// The set hook run when the stage ends empty
extern "C" void termPurpleIceGearLSet()
{
}
