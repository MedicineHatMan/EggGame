#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "object/ObjAction.h"
#include "object/ObjArchive.h"
#include "egg/ObjEgg.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"
#include "camera/TCamera.h"
#include "ui/TSimpleText.h"
#include "io/ResBank.h"
#include "engine/Color.h"
#include "stage/TEventCtrl.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "object/ObjShadow.h"
#include "ui/MessageTable.h"
#include "object/last/TObjLastAnimus.h"
#include "object/TObjLauncher.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "core/MathUtil.h"
#include "engine/GroundQuery.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "engine/LightSet.h"
struct SetLink;

/** Message 0x70 a player asks to talk */
struct AnimusIdMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mId; // _04
};

/** Message 0x43 asks an object how it is moving */
struct AnimusVelMsg {
	u16 mType;  // _00
	Vec3f mVel; // _04
};

enum AnimusState {
	ANIMUS_NONE, // 0
	ANIMUS_WAIT, // 1 stands
	ANIMUS_RUN,  // 2 walks to a point near home
	ANIMUS_ACT,  // 3 a one-off motion then mNextState
	ANIMUS_TURN, // 4 turns to the player who asked to talk
	ANIMUS_TALK, // 5 shows the text until it closes
};

typedef char TObjLastAnimus_size_check[(sizeof(TObjLastAnimus) == 0x1E0) ? 1 : -1];

#define ANIMUS_RAND()           ((1.0f / 32768.0f) * rand())
#define ANIMUS_CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

// ObjScript.h's static tables come first in .rodata 0x8024B968 though nothing here reads them
#include "object/ObjScript.h"

// 0x8024BA64
const MotionEntry sLastAnimusMotionTable[] = {
	{ MOTION_LOOP, 6, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 }, { MOTION_LOOP, 3, -1, 0, 0.0f, 30.0f, 0.6f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 50.0f, 0.3f, 0.0f, 0 }, { MOTION_ONCE, 2, -1, 0, 0.0f, 25.0f, 0.6f, 0.0f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 40.0f, 0.6f, 0.0f, 0 }, { MOTION_LOOP, 5, -1, 0, 0.0f, 30.0f, 0.3f, 0.0f, 0 },
};

const f32 gLastAnimusSetRange = 800.0f;

// 0x802C4638
// The two names are short enough to sit in .sdata2 after the range
const char* sLastAnimusModeNames[2] = { "WAIT", "RUN" };

// 0x8018ED58
extern "C" TObject* createTObjLastAnimus(TObject* parent, SetDesc* desc)
{
	return new TObjLastAnimus(parent, desc);
}

// 0x8018EDAC
void TObjLastAnimus::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mMsgId   = desc->getInt(0);
	mMsgId   = ANIMUS_CLAMP(mMsgId, 0, getMessageCount() - 1);
	int mode = desc->getInt(1);
	mMode    = ANIMUS_CLAMP(mode, 0, 1);
	mRange   = 50.0f + desc->getFloat(1);
	mSpeed   = 0.5f + desc->getFloat(2);
}

// 0x8018EEB0
void TObjLastAnimus::debugPrint(void*)
{
	njPrintC(0xA0013, ":MSG IDX");
	njPrint(0xA0014, ":MODE %s", sLastAnimusModeNames[mMode]);
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "+50.0:RANGE");
	njPrintC(0xA0019, "+0.5:SPD");
}

const char* gTObjLastAnimusName = "TObjLastAnimus";

// 0x8018EF64
TObjLastAnimus::TObjLastAnimus(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjLastAnimusName;
	mSize = 0x1E0;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	makeShape();
	initFromDesc(desc);
	reset();
}

// 0x8018F02C
TObjLastAnimus::~TObjLastAnimus()
{
	if (mText != 0) {
		mText->mHold = 1;
	}
	freeShape();
	removeDrawTask(mDraw);
}

// 0x8018F0E0
void TObjLastAnimus::reset()
{
	mModel     = getGeObjModels()[2];
	mTexList   = getGeObjTextures()[2];
	mScale.x   = 1.0f;
	mScale.y   = 1.0f;
	mScale.z   = 1.0f;
	mHop       = 0.0f;
	_144       = 0.0f;
	mSquashVel = 0.0f;
	_14C       = 0.0f;
	mTalker.setId(0xFFFF);
	mTarget.x     = mPos.x;
	mTarget.y     = mPos.y;
	mTarget.z     = mPos.z;
	mTargetAngle  = mRot.y;
	mVel.x        = 0.0f;
	mVel.y        = 0.0f;
	mVel.z        = 0.0f;
	mConversation = 0;
	mTimer        = 0;
	mWaitTime     = 0;
	mNextState    = 0;
	mText         = 0;
	makeSphere();
	calcMatrix();
	mMotion.init(sLastAnimusMotionTable, gGeObjCommon->mSet.mMotions);
	mMotion.play(0);
	mObj.setModel(mModel);
	mObj.mTexList = mTexList;
	mObj.mState   = (DrawState*)getDefaultDrawState();
	mObj.mMotion  = &mMotion;
	mObj.calcMotion();
	mShadow._30 = 10.0f;
	mShadow._34 = 10.0f;
	shadowEnable(&mShadow);
	calcShadow();
	mLight.reset();
	mLight.setFadeDist(5.0f);
	startWalk();
	setLinkFree((SetLink*)&mLink);
	mLightSet = lightSetFindArea(&mPos);
}

// 0x8018F248
void TObjLastAnimus::makeShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xF6, 0xF6);
}

// 0x8018F2A4
void TObjLastAnimus::makeSphere()
{
	ColliShape shape;
	colliShapeMakeSphere(&shape, &mPos, 1, 8.0f);
	shape.mFlags |= 0x200;
	colliCopyShapes(mShape, &shape);
}

// 0x8018F2F8
void TObjLastAnimus::moveShape()
{
	Vec3f pos = mPos;
	pos.y += 8.0f;
	colliShapeSetPos(colliGetShape(mShape, 0), &pos);
}

// 0x8018F350
void TObjLastAnimus::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x8018F388
// a player landing on it squashes it and bounces them up
int TObjLastAnimus::checkTouch()
{
	AnimusVelMsg ask;
	AnimusVelMsg bounce;
	AnimusIdMsg touched;
	TObjAction* obj;
	ColliContact* contact;

	if (mSquashVel != 0.0f) {
		return 0;
	}
	for (contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		ColliShape* shape = colliGetShape(contact->mOther, contact->mOtherShape);
		if (shape->mFlags & 0x10) {
			continue;
		}
		obj = findObjById(colliContactOwner(contact));
		if (obj == 0) {
			continue;
		}
		if (!(shape->mFlags & 0x400)) {
			continue;
		}
		if (colliContactInGroup8(contact)) {
			TObjAction* other = findObjById(colliContactOwner(contact));
			if (other != 0) {
				touched.mType = 0x2A;
				other->sendToHandle((ObjMsg*)&touched);
			}
		}
		ask.mType = 0x43;
		obj->sendToHandle((ObjMsg*)&ask);
		if (ask.mVel.y < 0.0f) {
			mScale.y      = 0.05f;
			bounce.mType  = 1;
			bounce.mVel.x = 0.0f;
			bounce.mVel.z = 0.0f;
			bounce.mVel.y = 1.5f * (60.0f * -ask.mVel.y);
			obj->sendToHandle((ObjMsg*)&bounce);
		}
	}
	return 0;
}

// 0x8018F4E4
void TObjLastAnimus::calcMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxTranslate(mMatrix, mMatrix, 0.0f, mHop, 0.0f);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxScaleV(mMatrix, mMatrix, &mScale);
}

// 0x8018F550
void TObjLastAnimus::calcShadow()
{
	EggHits hits;
	Vec3f probe = mPos;
	probe.y += 10.0f;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&probe, 1) != 0 && (hits.mMask & 1)) {
		setShadow(&hits);
	} else {
		shadowSetGround(&mShadow, &mPos, 0);
	}
}

// 0x8018F600
void TObjLastAnimus::setShadow(EggHits* hits)
{
	shadowSetFromContact(&mShadow, (u8*)hits);
}

// 0x8018F624
void TObjLastAnimus::startConversation()
{
	if (mConversation == 0) {
		mConversation = createTCamConversation(0, 2, &mPos, 0, 0x1555);
	}
}

// 0x8018F674
void TObjLastAnimus::endConversation()
{
	if (mConversation != 0) {
		mConversation->mFlags |= 1;
		mConversation = 0;
	}
}

// 0x8018F698
void TObjLastAnimus::startMode()
{
	switch (mMode) {
	case 0:
		setState(ANIMUS_WAIT);
		break;
	case 1:
		setState(ANIMUS_RUN);
		break;
	}
}

// 0x8018F6E4
void TObjLastAnimus::draw()
{
	if (objDebugEnabled(this)) {
		mMotion.update();
		mObj.calcMotion();
		calcMatrix();
		calcShadow();
		moveShape();
	} else if (!isInRange(this, &mPos, 850.0f)) {
		mFlags |= 1;
	} else if (isInRange(this, &mPos, 500.0f)) {
		switch (mMode) {
		case 0:
			updateWait();
			break;
		case 1:
			updateRun();
			break;
		}
		mObj.calcMotion();
	}
}

// 0x8018F7BC
void TObjLastAnimus::updateWait()
{
	updateState();
	calcMatrix();
}

// 0x8018F7F0
void TObjLastAnimus::updateRun()
{
	updateState();
	calcMatrix();
	moveShape();
}

// 0x8018F82C
int TObjLastAnimus::setState(int state)
{
	switch (state) {
	case ANIMUS_NONE:
		return 0;
	case ANIMUS_WAIT:
		mMotion.playBlend(0, 0.05f);
		break;
	case ANIMUS_RUN:
		mMotion.playBlend(1, 0.05f);
		break;
	case ANIMUS_ACT:
		mMotion.playBlend(2, 0.05f);
		break;
	case ANIMUS_TURN:
		mMotion.playBlend(3, 0.05f);
		break;
	case ANIMUS_TALK: {
		mText = createSimpleTextPlain(getMessage(mMsgId));
		if (mText == 0) {
			return 0;
		}
		startConversation();
		ColliShape* shape = colliGetShape(mShape, 0);
		Vec3f at;
		at.set(shape->mPos.x, shape->mPos.y, shape->mPos.z);
		sePlay3D(0x65, &at);
		if (ANIMUS_RAND() < 0.8f) {
			mMotion.playBlend(4, 0.05f);
		} else {
			mMotion.playBlend(5, 0.05f);
		}
		break;
	}
	}
	mState = state;
	mTimer = mWaitTime;
	return 1;
}

// 0x8018F9A8
int TObjLastAnimus::nextState()
{
	int ok = setState(mNextState);
	if (ok) {
		mNextState = 0;
	}
	return ok;
}

// 0x8018F9E8
// back to its mode with a new wait of 3 to 6 seconds
void TObjLastAnimus::startWalk()
{
	mTalker.setId(0xFFFF);
	mTarget.x    = mPos.x;
	mTarget.y    = mPos.y;
	mTarget.z    = mPos.z;
	mTargetAngle = mRot.y;
	mNextState   = 0;
	mWaitTime    = ((int)(3.0f * ANIMUS_RAND() + 0.5f) + 3) * 60;
	startMode();
}

// 0x8018FA9C
void TObjLastAnimus::updateState()
{
	switch (mState) {
	case ANIMUS_WAIT:
		stateWait();
		break;
	case ANIMUS_RUN:
		stateRun();
		break;
	case ANIMUS_ACT:
		stateAct();
		break;
	case ANIMUS_TURN:
		stateTurn();
		break;
	case ANIMUS_TALK:
		stateTalk();
		break;
	}
	mSquashVel += 0.04f * (1.0f - mScale.y);
	if ((f32)__fabs(mSquashVel) < 0.0004f) {
		mScale.y   = 1.0f;
		mSquashVel = 0.0f;
	} else {
		mScale.y += mSquashVel;
		mSquashVel *= 0.97f;
	}
	mMotion.update();
}

// 0x8018FB80
void TObjLastAnimus::stateWait()
{
	checkTouch();
	if (mTimer < 0) {
		mNextState = ANIMUS_WAIT;
		setState(ANIMUS_ACT);
	}
	mTimer--;
}

// 0x8018FBD8
void TObjLastAnimus::stateRun()
{
	checkTouch();
	if (mTargetAngle != mRot.y) {
		mRot.y = (u16)angleApproach(mRot.y, mTargetAngle, 0x800);
		return;
	}
	if (mTimer < 0) {
		mNextState = ANIMUS_RUN;
		setState(ANIMUS_ACT);
	}
	mTimer--;
	Vec3f step;
	vecSub(&step, &mTarget, &mPos);
	step.y   = 0.0f;
	f32 dist = njScalor(&step);
	if (dist < mSpeed) {
		mTarget.x    = mHome.x + mRange * (2.0f * (ANIMUS_RAND() - 0.5f));
		mTarget.z    = mHome.z + mRange * (2.0f * (ANIMUS_RAND() - 0.5f));
		mTargetAngle = (u16)angleToXZ(&mPos, &mTarget);
	} else {
		gjScaleVector(mSpeed / dist, &step, &step);
		vecAdd(&mPos, &mPos, &step);
	}
	fall();
}

// 0x8018FD48
void TObjLastAnimus::stateAct()
{
	checkTouch();
	if (mMotion.isDone() && !nextState()) {
		startWalk();
	}
}

// 0x8018FD9C
void TObjLastAnimus::stateTurn()
{
	int angle = angleApproach(mRot.y, mTargetAngle, 0x800);
	if (mMotion.isDone() && angle == mTargetAngle) {
		setState(ANIMUS_TALK);
	}
	mRot.y = angle;
}

// 0x8018FE04
void TObjLastAnimus::stateTalk()
{
	ObjMsg done;

	if (mText != 0 && mText->mState == 3) {
		mText->mHold       = 1;
		mText              = 0;
		TObjAction* talker = mTalker.get();
		if (talker != 0) {
			done.mType = 0x30;
			talker->sendToHandle(&done);
		}
		endConversation();
		startWalk();
		eventEnd();
	}
}

// 0x8018FE94
// gravity and the ground Returns 1 on landing
int TObjLastAnimus::fall()
{
	mVel.y += -0.1f;
	vecAdd(&mPos, &mPos, &mVel);
	EggHits hits;
	Vec3f probe = mPos;
	probe.y += 15.0f;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&probe, 1) != 0 && (hits.mMask & 1)) {
		EggHit* hit = hits.mHits;
		setShadow((EggHits*)hit);
		if (mPos.y < hit->mPos.y) {
			mVel.y = 0.0f;
			mPos.y = hit->mPos.y;
			return 1;
		}
		return 0;
	}
	shadowSetGround(&mShadow, &mPos, 0);
	return 0;
}

// 0x8018FF94
int TObjLastAnimus::sendToHandle(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x40:
		return 0x40;
	case 0x70: {
		u16 from        = ((AnimusIdMsg*)msg)->mId;
		TObjAction* obj = findObjById(from);
		if (obj != 0) {
			Vec3f* at    = obj->getPos();
			mTargetAngle = (u16)angleToXZ(&mPos, at);
		} else {
			return 0;
		}
		if (eventStartByIds(mObjId, from) == 0) {
			return 0;
		}
		mTalker.setId(from);
		setState(ANIMUS_TURN);
		return 1;
	}
	}
	return 0;
}

// 0x80190068
// hides it when its light says it is out of view
void TObjLastAnimus::vfn_10()
{
	ColliShape* shape = colliGetShape(mShape, 0);
	Vec3f center;
	center.set(shape->mPos.x, shape->mPos.y, shape->mPos.z);
	if (mLight.isVisible(mDraw, 700.0f, &center, colliGetShape(mShape, 0)->mSizeX, 0)) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x8019010C
void TObjLastAnimus::render()
{
	f32 mtx[12];

	shadowAlphaBegin();
	lightSetSelect(mLightSet);
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	njSetTexture(mTexList);
	mLight.drawActor(mObj.mActor);
	njPopMatrixEx();
	shadowAlphaEnd();
	if (mMode != 0 && objDebugEnabled(this)) {
		mtxConcat(mtx, gjGetMatrix(), mMatrix);
		mtxScale(mRange, 1.0f, mRange, mtx, mtx);
		mtxRotateX(mtx, mtx, 0x4000);
		debugDrawCylinder(mtx, &gColorGreen);
	}
}
