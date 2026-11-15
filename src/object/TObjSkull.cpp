#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "object/TObjSkull.h"
#include "camera/TCamera.h"
#include "Ninja/ninja.h"
#include "object/TObjLauncher.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "engine/TParticle.h"
#include "stage/SetSlot.h"
struct NJS_QUATERNION;

extern "C" {
MotionEntry sSkullMotions[] = {
	{ 4, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 1.0f, 0.0f, 0 },
};

}

#define sSkulls sSkullListObj

typedef char TObjSkull_size_check[(sizeof(TObjSkull) == 0x148) ? 1 : -1];

// 0x80054918
extern "C" void initSkulls()
{
	sSkulls = new SkullList;
}

// 0x80054950
extern "C" void termSkulls()
{
	delete sSkulls;
}

// 0x80054978
extern "C" TObject* createTObjSkull(TObject* parent, SetDesc* desc)
{
	return new TObjSkull(parent, desc);
}

// 0x800549CC
void TObjSkull::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mSkullId = desc->getInt(0);
	mScale   = 1.0f + desc->getFloat(0);
}

const char* gTObjSkullName = "TObjSkull";

// 0x80054A3C
void TObjSkull::debugPrint(void*)
{
	njPrintC(0xA0013, ":DOKURO ID");
	njPrintC(0xA0017, "+1.0:SCALE");
}

// 0x80054A80
TObjSkull::TObjSkull(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjSkullName;
	mSize = 0x148;
	initFromDesc(desc);
	mNode.mObj = this;
	sSkulls->add(&mNode);
	setLinkTimed((SetLink*)&mLink, 600);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	vfn_74();
}

// 0x80054B5C
TObjSkull::~TObjSkull()
{
	sSkulls->remove(&mNode);
	removeDrawTask(mDrawTask);
}

// 0x80054BDC
extern "C" TObjSkull* findSkull(u16 id)
{
	for (SkullNode* node = sSkulls->mHead; node != 0; node = node->mNext) {
		if (id == node->mObj->mSkullId) {
			return node->mObj;
		}
	}
	return 0;
}

// 0x80054C10
void TObjSkull::vfn_74()
{
	mAct      = 0;
	mMotionId = 0;
	mLitId    = -1;
	mTimer    = 0;

	mPlace.mFlags = 7;
	mPlace.mPos   = mPos;
	njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
	mPlace.mScale.z = mPlace.mScale.y = mPlace.mScale.x = mScale;
	mPlace.update();

	mMotion.init((const MotionEntry*)sSkullMotions, gGeObjCommon->mSet.mMotions);
	mMotion.play((s16)mMotionId);
	mModel.setModel(getGeObjModels()[1]);
	mModel.mPlace   = &mPlace;
	mModel.mTexList = getGeObjTextures()[1];
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mModel.mMotion  = &mMotion;
}

// 0x80054CF4
void TObjSkull::stateIdle()
{
}

// 0x80054CF8
void TObjSkull::stateLight()
{
	mTimer++;
}

// 0x80054D08
void TObjSkull::stateOpening()
{
}

// 0x80054D0C
// Back to idle once the opening motion is over
void TObjSkull::stateOpen()
{
	if (mMotionId == 1 && mMotion.isDone()) {
		mAct      = 0;
		mMotionId = 0;
		mMotion.play((s16)mMotionId);
	}
}

// 0x80054D6C
// Takes several calls returns 1 when the eyes have been lit for this id
int TObjSkull::lightEyes(u16 id)
{
	if (mLitId == id) {
		return 1;
	}
	if (mAct == 1) {
		if (mTimer == 0) {
			Vec3f left;
			Vec3f right;
			right = mPos;
			left = mPos;
			Vec3f leftEye;
			leftEye.x = -9.5f;
			leftEye.y = 0.8f;
			leftEye.z = 15.0f;
			Vec3f rightEye;
			rightEye.x = 11.4f;
			rightEye.y = 0.8f;
			rightEye.z = 15.0f;
			njUnitMatrix(0);
			gjRotateY(mRot.y);
			njCalcPointEx(&leftEye, &leftEye);
			njCalcPointEx(&rightEye, &rightEye);
			njAddVector(&left, &leftEye);
			njAddVector(&right, &rightEye);
			particleOnceById(this, &left, 0, 0x165);
			particleOnceById(this, &right, 0, 0x165);
		} else if (mTimer == 15) {
			mTimer = 0;
			mLitId = id;
			mAct   = 0;
			return 1;
		}
	} else {
		mAct = 1;
	}
	return 0;
}

// 0x80054EA8
int TObjSkull::openJaw()
{
	if (mAct == 0) {
		mAct      = 2;
		mMotionId = 1;
		mMotion.play((s16)mMotionId);
		return 1;
	}
	return 0;
}

// 0x80054F00
// True from frame 5 of the opening motion
int TObjSkull::isJawOpen()
{
	int open = 0;
	if (mMotionId == 1 && mMotion.mFrame > 5.0f) {
		mAct = 3;
		open = 1;
	}
	return open;
}

// 0x80054F34
void TObjSkull::draw()
{
	if (!isInRange(this, &mPos, 1100.0f)) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this) != 0) {
		mPlace.mPos = mPos;
		njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
		mPlace.update();
		return;
	}
	switch (mAct) {
	case 0:
		stateIdle();
		break;
	case 1:
		stateLight();
		break;
	case 2:
		stateOpening();
		break;
	case 3:
		stateOpen();
		break;
	}
	mMotion.update();
}

// 0x80055020
void TObjSkull::vfn_10()
{
	if (cameraIsVisible(1000.0f, 35.0f, getPos(), 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80055094
void TObjSkull::render()
{
	mModel.draw();
}

// 0x800550B8
SkullList::SkullList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

// 0x800550CC
SkullList::~SkullList()
{
	clear();
}

// 0x80055114
void SkullList::add(SkullNode* node)
{
	if (mHead == 0) {
		mTail = node;
		mHead = node;
	} else {
		node->mPrev  = mTail;
		mTail->mNext = node;
		mTail        = node;
	}
	node->mList = this;
	mCount++;
}

// 0x80055154
void SkullList::remove(SkullNode* node)
{
	if (node->mPrev != 0) {
		node->mPrev->mNext = node->mNext;
	} else {
		mHead = node->mNext;
	}
	if (node->mNext != 0) {
		node->mNext->mPrev = node->mPrev;
	} else {
		mTail = node->mPrev;
	}
	node->mNext = 0;
	node->mPrev = 0;
	node->mList = 0;
	mCount--;
}

// 0x800551B4
void SkullList::clear()
{
	SkullNode* node = mHead;
	while (node != 0) {
		SkullNode* next = node->mNext;
		node->mList = 0;
		node->mNext = 0;
		node->mPrev = 0;
		node = next;
	}
}
