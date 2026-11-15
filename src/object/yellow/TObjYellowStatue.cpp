#include "stage/SetActor.h"
struct DrawTask;
#include "camera/TCamera.h"
#include "camera/EventCamArc.h"
#include "stage/EventScript.h"
#include "object/ObjScript.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "object/ObjAction.h"
#include "stl/math.h"
#include "stage/TEventCtrl.h"
#include "object/TObjLauncher.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "sound/SoundCtrl.h"
#include "object/yellow/TObjYellowStatue.h"
#include "object/yellow/StatueListImpl.h"
struct SetLink;
struct TObjAction;

extern "C" {
// 0x802C44D0
ObjClassName gTObjYellowStatueName              = { "TObjYellowStatue", 0 };
extern const MotionEntry sYellowStatueMotions[] = { { 1, 0, -1, 0, 0.0f, 45.0f, 0.2f, 0.0f, 0 } };
}

class TObjYellowStatue;

static StatueList sStatues;

typedef char TObjYellowStatue_size_check[(sizeof(TObjYellowStatue) == 0x130) ? 1 : -1];

// 0x80154008
extern "C" TObject* createTObjYellowStatue(TObject* parent, SetDesc* desc)
{
	return new TObjYellowStatue(parent, desc);
}

// 0x8015405C
TObjYellowStatue::TObjYellowStatue(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	mTimer     = 0x30;
	mEvent     = 0;
	_00        = (u32)gTObjYellowStatueName.mName;
	mSize      = 0x130;
	mNode.mObj = this;
	sStatues.add(&mNode);

	mModelData = getGeObjModels()[2];
	mTexList   = getGeObjTextures()[2];
	mMotion.init((const MotionEntry*)sYellowStatueMotions, gGeObjCommon->mSet.mMotions);
	mMotion.play(0);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	initFromDesc(desc);
	vfn_74();
}

// 0x80154174
TObjYellowStatue::~TObjYellowStatue()
{
	removeDrawTask(mDrawTask);
	sStatues.remove(&mNode);
}

static inline int clampSwitch(SetDesc* desc, int index)
{
	if (desc->getInt(index) < 0) {
		return 0;
	}
	if (desc->getInt(index) > 0x800) {
		return 0x800;
	}
	return desc->getInt(index);
}

// 0x801541F8
void TObjYellowStatue::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mMirror     = desc->getInt(0);
	mSwitchTurn = clampSwitch(desc, 1);
	mSwitchOff  = clampSwitch(desc, 2);
	mCheckId    = desc->getInt(3);
}

// 0x80154338
void TObjYellowStatue::debugPrint(void*)
{
	njPrint(0xA0013, ":TYPE %s", (mMirror != 0) ? "B" : "A");
	njPrintC(0xA0014, ":ROT SW ID");
	njPrintC(0xA0015, ":SW ID");
	njPrintC(0xA0016, ":CALL EVENT ID");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x801543FC
void TObjYellowStatue::vfn_74()
{
	mModel.setModel(mModelData);
	mModel.mTexList = mTexList;
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mModel.mMotion  = &mMotion;
	if (keyBitIsSet(mSwitchOff) == 1) {
		mAngle = 0x8000;
		mAct   = 5;
	} else {
		mAngle = 0x4000;
		mAct   = 0;
	}
	updateMatrix();
	setLinkFree((SetLink*)&mLink);
}

// 0x80154490
void TObjYellowStatue::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	if (mMirror == 0) {
		mtxRotateY(mMatrix, mMatrix, mAngle + mRot.y);
	} else {
		mtxRotateY(mMatrix, mMatrix, mRot.y - mAngle);
	}
	GjModel* model = mModelData->model;
	mCenter        = *(Vec3f*)&model->center;
	mtxMultVec(mMatrix, &mCenter, &mCenter);
	mRadius = mModelData->model->r;
}

// 0x80154544
int TObjYellowStatue::othersDone()
{
	StatueNode* node;
	node = sStatues.mHead;
	while (node != 0) {
		if ((node->mObj != this) && (node->mObj->mAct != 5)) {
			return 0;
		} else {
			node = node->mNext;
		}
	}
	return 1;
}

// 0x80154588
// Called when the event system can take this event
static int startStatueEvent(u32 id)
{
	TObjYellowStatue* self = (TObjYellowStatue*)findObjById((u16)id);
	if (self == 0) {
		return 0;
	}
	for (StatueNode* node = sStatues.mHead; node != 0; node = node->mNext) {
		if (node->mObj != self) {
			eventHideObj((u8*)node->mObj);
		}
	}
	self->mEvent = eventCamArcPlay(1.0f, 0.0f, 0x101);
	if (self->mEvent != 0) {
		((TCamera*)self->mEvent)->setCut();
	}
	self->mAct = 3;
	return 1;
}

// 0x80154620
void TObjYellowStatue::dimOthers()
{
	TObjYellowStatue* other;
	for (StatueNode* node = sStatues.mHead; node != 0; node = node->mNext) {
		other = node->mObj;
		if (other != this) {
			other->mAct = 2;
		}
	}
}

// 0x80154654
void TObjYellowStatue::draw()
{
	if (objDebugEnabled(this) != 0) {
		updateMatrix();
		return;
	}
	if (!isInRange(this, &mPos, 4100.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateTurn();
		break;
	case 1:
		stateWait();
		break;
	case 2:
		stateMotion();
		break;
	case 3:
		stateEvent();
		break;
	case 4:
		stateCheck();
		break;
	case 5:
		stateDone();
		break;
	}
}

// 0x80154738
// Turns one step whenever the switch is on or to finish a step it has started
void TObjYellowStatue::stateTurn()
{
	if (keyBitIsSet(mSwitchTurn) == 1 || (mAngle & 0x1FFF) != 0) {
		if (mTimer != 0) {
			if (--mTimer == 0) {
				sePlay(0x25C);
			}
		} else {
			mAngle = (f32)mAngle - 128.0f;
			updateMatrix();
			if ((mAngle & 0x1FFF) == 0) {
				if (keyBitIsSet(mSwitchTurn) == 0 && mAngle != 0x8000) {
					sePlay(0x4E);
				}
				mTimer = 0x30;
			}
		}
	} else if (mAngle == 0x8000) {
		sePlay(0x4E);
		mTimer = 60;
		mAct   = 1;
	} else {
		if (mTimer != 0x30) {
			sePlay(0x4E);
		}
		mTimer = 0x30;
	}
}

// 0x8015485C
void TObjYellowStatue::stateWait()
{
	if (mTimer != 0) {
		mTimer--;
	} else if (othersDone() != 0) {
		if (eventStartById(mObjId, (int (*)(int))startStatueEvent) != 0) {
			mAct = 5;
		}
	} else {
		setKeyBit(mSwitchOff);
		mAct = 5;
	}
}

// 0x801548DC
void TObjYellowStatue::stateMotion()
{
	if (mMotion.isDone()) {
		mAct = 5;
	} else {
		mMotion.update();
	}
}

// 0x80154928
// The other statues dim at frame 60 of the event
void TObjYellowStatue::stateEvent()
{
	int frame = 0;
	if (mEvent != 0) {
		frame = (f32)ceil(mEvent->getFrame());
	}
	if (frame < 60) {
		return;
	}
	if (frame == 60) {
		sePlayFor(0x265, 0xB4);
		dimOthers();
	}
	if (mMotion.isDone()) {
		if (mEvent != 0) {
			if (mEvent->isDone() == 0) {
				return;
			}
			if (mEvent != 0) {
				mEvent->mFlags |= 1;
				mEvent = 0;
			}
		}
		eventEnd();
		setKeyBit(mSwitchOff);
		mAct = 4;
	} else {
		mMotion.update();
	}
}

// 0x80154A00
void TObjYellowStatue::stateCheck()
{
	if (eventScriptRequest(mCheckId) != 0) {
		mAct = 5;
	}
}

// 0x80154A40
void TObjYellowStatue::stateDone()
{
}

// 0x80154A44
void TObjYellowStatue::vfn_10()
{
	if (cameraIsVisible(4000.0f, mRadius, &mCenter, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80154AAC
void TObjYellowStatue::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	if (mMirror == 0) {
		mModel.draw();
	} else {
		pushCullMode(0x18000000);
		njScale(0, -1.0f, 1.0f, 1.0f);
		mModel.draw();
		popCullMode();
	}
	njPopMatrixEx();
}

// 0x80154B28
void StatueList::add(StatueNode* node)
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

// 0x80154B68
void StatueList::remove(StatueNode* node)
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

#pragma dont_inline on
