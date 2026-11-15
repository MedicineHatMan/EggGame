#include "stage/SetManager.h"
struct DrawTask;
#include "stage/SetActor.h"
#include "object/ObjAction.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "object/yellow/TObjYellowFogTrapStatue.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "engine/Colli.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "engine/TFogCtrl.h"
#include "camera/TCameraCtrl.h"
#include "object/yellow/TObjYellowFogTrapCtrl.h"
#include "Ninja/ninja.h"
struct SetLink;
struct ColliContact;
struct ColliShape;

#define sInstance sFogTrapCtrlObj

typedef char TObjYellowFogTrapCtrl_size_check[(sizeof(TObjYellowFogTrapCtrl) == 0xBC) ? 1 : -1];

const f32 gYellowFogTrapCtrlSetRange = 1000.0f;

// 0x8016F314
extern "C" TObject* createTObjYellowFogTrapCtrl(TObject* parent, SetDesc* desc)
{
	if (sInstance == 0) {
		sInstance = new TObjYellowFogTrapCtrl(parent, desc);
		return (TObject*)sInstance;
	}
	return 0;
}

const char* gTObjYellowFogTrapCtrlName = "TObjYellowFogTrapCtrl";

// 0x8016F380
TObjYellowFogTrapCtrl::TObjYellowFogTrapCtrl(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjYellowFogTrapCtrlName;
	mSize = 0xBC;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	createShape();
	initFromDesc(desc);
	vfn_74();
}

// 0x8016F420
TObjYellowFogTrapCtrl::~TObjYellowFogTrapCtrl()
{
	freeShape();
	removeDrawTask(mDrawTask);
	if (this == sInstance) {
		sInstance = 0;
	}
}

// 0x8016F4A0
void TObjYellowFogTrapCtrl::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	int id;
	if (desc->getInt(0) < 0) {
		id = 0;
	} else if (desc->getInt(0) > 0x800) {
		id = 0x800;
	} else {
		id = desc->getInt(0);
	}
	mSwitchId = id;
	mFogId    = desc->getInt(1);
	mWidth    = __fabs(desc->getFloat(0));
	mHeight   = __fabs(desc->getFloat(1));
	mDepth    = __fabs(desc->getFloat(2));
}

// 0x8016F5C4
void TObjYellowFogTrapCtrl::debugPrint(void*)
{
	njPrintC(0xA0013, ":OFF_SWITCH ID");
	njPrintC(0xA0014, ":FOG_PARAM ID");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, ":WIDTH");
	njPrintC(0xA0018, ":HEIGHT");
	njPrintC(0xA0019, ":DEPTH");
	njPrintC(0xA001A, "");
}

// 0x8016F668
void TObjYellowFogTrapCtrl::vfn_74()
{
	mFirstTime  = 1;
	mFogOn      = 0;
	mClearTimer = 0;
	updateMatrix();
	updateShape();
	setLinkFree((SetLink*)&mLink);
	if (keyBitIsSet(mSwitchId) == 1) {
		mAct = 2;
	} else {
		mAct = 0;
	}
}

// 0x8016F6DC
void TObjYellowFogTrapCtrl::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
}

// 0x8016F720
void TObjYellowFogTrapCtrl::createShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 255, 222);
}

// 0x8016F77C
void TObjYellowFogTrapCtrl::updateShape()
{
	colliShapeMakeBox(colliGetShape(mShape, 0), &mPos, mRot.y, mWidth, mHeight, mDepth, 1);
}

// 0x8016F7CC
void TObjYellowFogTrapCtrl::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x8016F804
void TObjYellowFogTrapCtrl::applyFog()
{
	fogRequest(camGetDrawIndex(), mFogId, 2, 100, 1.0f / 60.0f);
}

// 0x8016F844
void TObjYellowFogTrapCtrl::draw()
{
	if (objDebugEnabled(this) != 0) {
		updateMatrix();
		updateShape();
		return;
	}
	if (!isInRange(this, &mPos, 1100.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateOutside();
		break;
	case 1:
		stateInside();
		break;
	case 2:
		stateOff();
		break;
	}
}

// 0x8016F8FC
void TObjYellowFogTrapCtrl::stateOutside()
{
	if (keyBitIsSet(mSwitchId) != 0) {
		mAct = 2;
		return;
	}
	for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
		if (colliContactInGroup1(hit) != 0 && colliContactNoBit10(hit) != 0) {
			if (mFirstTime != 0) {
				triggerFogTrapStatue();
				mFirstTime = 0;
			}
			mAct = 1;
		}
	}
}

// 0x8016F998
void TObjYellowFogTrapCtrl::stateInside()
{
	if (keyBitIsSet(mSwitchId) != 0) {
		mAct = 2;
		return;
	}
	int inside = 0;
	for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
		if (colliContactInGroup1(hit) != 0) {
			if (colliContactNoBit10(hit) != 0) {
				inside = 1;
			}
		} else if (colliContactHasBit8(hit) != 0) {
			TObjAction* other = (TObjAction*)findObjById((u16)colliContactOwner(hit));
			if (other != 0) {
				HitInfo info;
				if (other->onHit(this, 0, (int*)&info) != 0) {
					if (info.mFlags.testBit(8) != 0) {
						mClearTimer = 900;
					}
				}
			}
		}
	}
	switch (inside) {
	default:
		if (mClearTimer == 0) {
			mFogOn = 1;
		} else {
			mFogOn = 0;
		}
		break;
	case 0:
		mClearTimer = 0;
		mAct        = 0;
		return;
	}
	if (mFogOn != 0) {
		applyFog();
	}
	if (mClearTimer == 1) {
		triggerFogTrapStatue();
	}
	if (--mClearTimer < 0) {
		mClearTimer = 0;
	}
}

// 0x8016FB10
void TObjYellowFogTrapCtrl::stateOff()
{
	setLinkForever((SetLink*)&mLink);
	mFlags |= 1;
}

// 0x8016FB4C
void TObjYellowFogTrapCtrl::vfn_10()
{
}

// 0x8016FB50
void TObjYellowFogTrapCtrl::render()
{
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
	}
}
