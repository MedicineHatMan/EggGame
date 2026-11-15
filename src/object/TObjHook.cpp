#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "object/ObjCommon.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "stage/StageObjUtil.h"
#include "animal/TAnimalRacer.h"
#include "object/TObjLauncher.h"
#include "camera/TCamera.h"
#include "object/ObjArchive.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjAction.h"
#include "camera/TCameraCtrl.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "stage/SetSlot.h"
#include "object/TObjHook.h"
#include "Ninja/ninja.h"
struct SetLink;
struct NJS_QUATERNION;

/** What the hook sends to the object that touches it */
struct HookMsg : public ObjMsg {
	u16 _02;   // _02
	u16 mFrom; // _04 the hook's id
	u16 _06;   // _06
};

typedef char TObjHook_size_check[(sizeof(TObjHook) == 0xF4) ? 1 : -1];

// 0x80072468
extern "C" TObject* createTObjHook(TObject* parent, SetDesc* desc)
{
	return new TObjHook(parent, desc);
}

// 0x800724BC
void TObjHook::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	setShape(12.5f);
}

// 0x800724F4
void TObjHook::debugPrint(void*)
{
}

// 0x800724F8
TObjHook::TObjHook(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjHookName;
	mSize = 0xF4;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 254, 254);
	initFromDesc(desc);
	vfn_74();
}

// 0x800725D4
TObjHook::~TObjHook()
{
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDraw);
}

// 0x80072658
void TObjHook::vfn_74()
{
	_E8     = 0.0f;
	_F0     = 0;
	mHooked = 0;
	mPlace.mFlags = 7;
	mPlace.mPos.x = mPos.x;
	mPlace.mPos.y = mPos.y;
	mPlace.mPos.z = mPos.z;
	njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
	mPlace.mScale.z = mPlace.mScale.y = mPlace.mScale.x = 1.0f;
	mPlace.update();
	mModel.setModel(getStgObjModels()[0x22]);
	mModel.mPlace   = &mPlace;
	mModel.mTexList = getStgObjTextures()[0x21];
	mModel.mState   = getDefaultDrawState();
	setLinkFree((SetLink*)&mLink);
}

// 0x80072710
int TObjHook::checkHit()
{
	ContactQuery query;
	if (query.findGroup((CharaColli*)mShape, 1) != 0) {
		ColliShape* shape = colliContactShape(query.mContact);
		TObjAction* obj   = findObjById(query.mOwner);
		if (obj == 0 || mHooked != 0) {
			return 0;
		}
		if ((shape->mFlags & 0x10) == 0) {
			HookMsg msg;
			msg.mType = 0x1B;
			msg.mFrom = mObjId;
			msg._06   = 0;
			obj->sendToHandle(&msg);
			mHooked = 1;
			return 1;
		}
	} else {
		mHooked = 0;
	}
	return 0;
}

// 0x80072800
void TObjHook::setShape(f32 radius)
{
	Vec3f offset = gVecZero;
	colliSetSphere((CharaColli*)mShape, &mPos, 0, &offset, radius);
}

// 0x80072850
void TObjHook::draw()
{
	if (isInRange(this, &mPos, 900.0f) == 0) {
		mFlags |= 1;
	} else {
		if (objDebugEnabled(this) != 0) {
			mPlace.mPos.x = mPos.x;
			mPlace.mPos.y = mPos.y;
			mPlace.mPos.z = mPos.z;
			njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
			mPlace.update();
		}
		checkHit();
	}
}

// 0x800728E0
void TObjHook::vfn_10()
{
	if (cameraIsVisible(800.0f, 12.5f, &mPos, 0) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x80072948
void TObjHook::render()
{
	mModel.draw();
	objDebugEnabled(this);
}

// 0x80072980
void TObjHook::getNodeMatrix(int node, f32* mtx)
{
	Vec3f pos;
	f32 m[12];
	if (node == 0) {
		mtxSetTransV(m, &mPos);
		mtxRotateZYX(m, m, (const int*)&mRot);
		mtxTranslate(m, m, 0.0f, 0.0f, 7.0f);
		mtxGetPos(m, &pos);
		mtxSetTransV(mtx, &pos);
		mtxRotateY(mtx, mtx, mRot.y + 0x8000);
	}
}
