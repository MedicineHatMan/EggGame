#include "object/ObjModel.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "stage/SetActor.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "object/ObjCommon.h"
#include "camera/TCamera.h"
#include "object/TObjLauncher.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "Ninja/ninja.h"
#include "object/ObjAction.h"
#include "camera/TCameraCtrl.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "object/TObject.h"
#include "object/map/TObjMapRock01.h"

const char* gTObjMapRock01Name = "TObjMapRock01";
struct SetLink;
struct NJS_QUATERNION;

// 0x80065874
extern "C" TObject* createTObjMapRock01(TObject* parent, SetDesc* desc)
{
	return new TObjMapRock01(parent, desc);
}

// 0x800658C8
// ":BREAK ID" is the first int
void TObjMapRock01::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mKeyBit = desc->getInt(0);
	mScale  = desc->getFloat(3) + 1.0f;
}

// 0x80065938
void TObjMapRock01::debugPrint(void*)
{
	njPrintC(0xA0013, ":BREAK ID");
	njPrintC(0xA001A, "+1.0:SCALE");
}

// 0x8006597C
TObjMapRock01::TObjMapRock01(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjMapRock01Name;
	mSize = 0xF0;
	initFromDesc(desc);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	vfn_74();
}

// 0x80065A24
TObjMapRock01::~TObjMapRock01()
{
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDraw);
}

// 0x80065AA8
void TObjMapRock01::vfn_74()
{
	mAct          = 0;
	mPlace.mFlags = 7;
	mPlace.mPos   = mPos;
	njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
	Vec3f scale;
	scale.z = scale.y = scale.x = 1.0f;
	gjScaleVector(mScale, &scale, &scale);
	mPlace.mScale = scale;
	mPlace.update();
	mModel.setModel(getStgObjModels()[0x24]);
	mModel.mPlace   = &mPlace;
	mModel.mTexList = getStgObjTextures()[0x24];
	mModel.mState   = (DrawState*)getDefaultDrawState();
	setLinkDone((SetLink*)&mLink);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0, 0xD6);
	setSize(25.0f);
}

// 0x80065BBC
void TObjMapRock01::checkTouch()
{
	for (ColliContact* contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactHasBit8(contact) == 0 || colliInGroup(contact->mOther, 0x20) == 0) {
			continue;
		}
		TObjAction* other = findObjById((u16)colliContactOwner(contact));
		if (other != 0) {
			ObjMsg msg;
			msg.mType = 0x47;
			other->sendToHandle(&msg);
			setKeyBit((u16)mKeyBit);
			return;
		}
	}
}

// 0x80065C64
void TObjMapRock01::stateWait()
{
	checkTouch();
	if (keyBitIsSet((u16)mKeyBit) == 1) {
		Vec3f size;
		size.x = size.y = size.z = 0.5f * colliGetShape(mShape, 0)->mSizeX;
		ShatterList pieces;
		pieces.mModels     = getStgObjModels();
		pieces.mTexture    = getStgObjTextures()[0x24];
		pieces.mFirst      = 0x25;
		pieces.mModelCount = 9;
		pieces.mCount      = 10;
		pieces.mScale      = 1.0f;
		pieces.mSpeed      = 3.0f;
		shatterListAt(this, getPos(), &size, &pieces);
		sePlay3D(0x4B, getPos());
		mAct = 1;
	}
}

// 0x80065D54
// Waits for the pieces to go then deletes itself
void TObjMapRock01::stateBroken()
{
	if (objDebugEnabled(this) == 0 && objCountChildren(this) == 0) {
		mFlags |= 1;
	}
}

// 0x80065DA4
void TObjMapRock01::setSize(f32 scale)
{
	Vec3f tmp;
	tmp.x        = 0.0f;
	tmp.y        = 0.0f;
	tmp.z        = 0.0f;
	tmp.y        = 0.7f * scale * mScale;
	Vec3f offset = tmp;
	colliSetSphere((CharaColli*)mShape, &mPos, 0x80, &offset, scale * mScale);
}

// 0x80065E14
void TObjMapRock01::draw()
{
	if (objDebugEnabled(this) != 0) {
		mPlace.mPos = mPos;
		njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
		Vec3f scale;
		scale.z = scale.y = scale.x = 1.0f;
		gjScaleVector(mScale, &scale, &scale);
		mPlace.mScale = scale;
		mPlace.update();
	}
	if (isInRange(this, &mPos, 600.0f) == 0) {
		if (mAct != 1) {
			setLinkFree((SetLink*)&mLink);
		}
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateBroken();
		break;
	}
}

// 0x80065F18
void TObjMapRock01::vfn_10()
{
	ColliShape* shape = colliGetShape(mShape, 0);
	if (cameraIsVisible(500.0f, shape->mSizeX, getPos(), 0) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x80065FA0
void TObjMapRock01::render()
{
	if (mAct != 1) {
		mModel.draw();
	}
}
