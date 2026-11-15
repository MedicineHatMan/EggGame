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
#include "engine/DrawState.h"
#include "enemy/EnemyLight.h"
#include "camera/TCamera.h"
#include "ui/TSimpleText.h"
#include "engine/Color.h"
#include "stage/TEventCtrl.h"
#include "ui/MessageTable.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "object/ObjShadow.h"
#include "object/TObjHiyoko.h"
#include "object/TObjLauncher.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCamPath.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "core/MathUtil.h"
#include "engine/GroundQuery.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "engine/LightSet.h"
struct SetLink;

#define HIYOKO_CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

/** Message 0x70 a player asks to talk */
struct HiyokoIdMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mId;   // _04
};

/** Message 0x43 asks an object how it is moving */
struct HiyokoVelMsg {
	u16 mType; // _00
	Vec3f mVel; // _04
};

struct HiyokoRGBA {
	u8 r, g, b, a;
};

enum HiyokoState {
	HIYOKO_NONE,     // 0
	HIYOKO_WAIT,     // 1 stands
	HIYOKO_RUN,      // 2 runs to a point near home
	HIYOKO_HOP_UP,   // 3
	HIYOKO_HOP_DOWN, // 4
	HIYOKO_ACT,      // 5 a one-off motion then mNextState
	HIYOKO_6,        // 6
	HIYOKO_7,        // 7
	HIYOKO_TURN,     // 8 turns to the player who asked to talk
	HIYOKO_TALK,     // 9 shows the text until it closes
};

// 0x80246738
// An earlier motion table that nothing reads five motions and an end row then three more rows
extern const MotionEntry sHiyokoOldMotionTable[9];
const MotionEntry sHiyokoOldMotionTable[9] = {
	{ 0, 0, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },
	{ 0, 1, -1, 0, 0.0f, 10.0f, 1.0f, 0.0f, 0 },
	{ 0, 2, -1, 0, 0.0f, 10.0f, 1.0f, 0.0f, 0 },
	{ 0, 3, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },
	{ 0, 4, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },
	{ -1, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
	{ 0, 0, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },
	{ 0, 1, -1, 0, 0.0f, 10.0f, 1.0f, 0.0f, 0 },
	{ 2, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
};

// 0x80246834
extern const char sHiyokoClassName[];
const char sHiyokoClassName[] = "TObjHiyoko";

// 0x80246840
const MotionEntry sHiyokoMotionTable[] = {
	{ 0, 9, -1, 0, 0.0f, 40.0f, 0.4f, 0.0f, 0 },
	{ 0, 6, -1, 0, 0.0f, 20.0f, 0.6f, 0.0f, 0 },
	{ 1, 1, -1, 0, 0.0f, 15.0f, 0.3f, 0.0f, 0 },
	{ 0, 2, -1, 0, 0.0f, 10.0f, 0.3f, 0.0f, 0 },
	{ 1, 3, -1, 0, 0.0f, 15.0f, 0.4f, 0.0f, 0 },
	{ 1, 4, -1, 0, 0.0f, 50.0f, 0.3f, 0.0f, 0 },
	{ 1, 5, -1, 0, 0.0f, 25.0f, 0.5f, 0.0f, 0 },
	{ 0, 7, -1, 0, 0.0f, 40.0f, 0.5f, 0.0f, 0 },
	{ 0, 8, -1, 0, 0.0f, 30.0f, 0.3f, 0.0f, 0 },
};

// Which parts each equip index draws
static const u32 sHiyokoEquip[15] = { 0, 1, 2, 4, 3, 5, 7, 6, 8, 0x10, 9, 0x18, 0x12, 0x14, 0x16 };

const f32 gHiyokoSetRange = 800.0f;

// 0x80282518
// the equip colours
ColorValue sHiyokoColors[6] = {
	ColorValue(0xBF0000FF), ColorValue(0x00BF00FF), ColorValue(0x0000BFFF),
	ColorValue(0xAA32C8FF), ColorValue(0x00BFBFFF), ColorValue(0xFF8000FF),
};

const char* gTObjHiyokoName = sHiyokoClassName; // 0x802C4150
const char* sHiyokoModeNames[2] = { "WAIT", "RUN" }; // 0x802C4154

static int sHiyokoMatCount;       // 0x802C5410 materials hiyokoMatColor has set this draw
static HiyokoRGBA sHiyokoBodyColor; // 0x802C5414 the first material's colour

typedef char TObjHiyoko_size_check[(sizeof(TObjHiyoko) == 0x29C) ? 1 : -1];

#define HIYOKO_RAND() ((1.0f / 32768.0f) * rand())

// 0x800F44AC
extern "C" void initTObjHiyoko() { }

// 0x800F44B0
extern "C" void termTObjHiyoko() { }

// 0x800F44B4
extern "C" TObject* createTObjHiyoko(TObject* parent, SetDesc* desc)
{
	return new TObjHiyoko(parent, desc);
}

// 0x800F4508
void TObjHiyoko::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mMsgId = desc->getInt(0);
	mMsgId = HIYOKO_CLAMP(mMsgId, 0, getMessageCount() - 1);
	int mode = desc->getInt(1);
	mMode    = HIYOKO_CLAMP(mode, 0, 1);
	mEquip   = sHiyokoEquip[HIYOKO_CLAMP(desc->getInt(2), 0, 14)];
	mEquip10State.mMaterial = sHiyokoColors[HIYOKO_CLAMP(desc->getInt(3), 0, 5)].mValue;
	mEquip8State.mMaterial  = mEquip10State.mMaterial;
	mEquip2State.mMaterial  = mEquip8State.mMaterial;
	mEquip1State.mMaterial  = mEquip2State.mMaterial;
	mRange = 50.0f + desc->getFloat(1);
	mSpeed = 0.5f + desc->getFloat(2);
}

// 0x800F471C
void TObjHiyoko::debugPrint(void*)
{
	njPrintC(0xA0013, ":MSG IDX");
	njPrint(0xA0014, ":MODE %s", sHiyokoModeNames[mMode]);
	njPrintC(0xA0015, ":EQUIP");
	njPrintC(0xA0016, ":EQUIP COL");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "+50.0:RANGE");
	njPrintC(0xA0019, "+0.5:SPD");
}

// 0x800F47D0
TObjHiyoko::TObjHiyoko(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjHiyokoName;
	mSize = 0x29C;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	makeShape();
	initFromDesc(desc);
	reset();
}

// 0x800F48BC
TObjHiyoko::~TObjHiyoko()
{
	if (mText != 0) {
		mText->mHold = 1;
	}
	freeShape();
	removeDrawTask(mDraw);
}

// 0x800F4970
void TObjHiyoko::reset()
{
	mModels[0]   = getStgObjModels()[14];
	mModels[1]   = getStgObjModels()[17];
	mModels[2]   = getStgObjModels()[16];
	mModels[3]   = getStgObjModels()[18];
	mModels[4]   = getStgObjModels()[15];
	mModels[5]   = getStgObjModels()[19];
	mTexLists[0] = getStgObjTextures()[13];
	mTexLists[1] = getStgObjTextures()[16];
	mTexLists[2] = getStgObjTextures()[15];
	mTexLists[3] = getStgObjTextures()[17];
	mTexLists[4] = getStgObjTextures()[14];
	mTexLists[5] = getStgObjTextures()[18];
	mScale.x   = 1.0f;
	mScale.y   = 1.0f;
	mScale.z   = 1.0f;
	mHop       = 0.0f;
	_1FC       = 0.0f;
	mSquashVel = 0.0f;
	_204       = 0.0f;
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
	mMotion.init(sHiyokoMotionTable, gStgObjCommon->mSet.mMotions);
	mMotion.play(0);
	mObj.setModelNodes(mModels[0], 0x34);
	mObj.mTexList = mTexLists[0];
	mObj.mState   = (DrawState*)getDefaultDrawState();
	mObj.mMotion  = &mMotion;
	mObj.calcMotion();
	mEquip1State.mFlags = mEquip2State.mFlags = mEquip8State.mFlags = mEquip10State.mFlags = DRAWSTATE_MATERIAL;
	mShadow._30 = 8.0f;
	mShadow._34 = 8.0f;
	calcShadow();
	mLight.reset();
	mLight.setFadeDist(5.0f);
	startWalk();
	setLinkFree((SetLink*)&mLink);
	mLightSet = lightSetFindArea(&mPos);
}

// 0x800F4B60
void TObjHiyoko::makeShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xF6, 0xF6);
}

// 0x800F4BBC
void TObjHiyoko::makeSphere()
{
	ColliShape shape;
	colliShapeMakeSphere(&shape, &mPos, 1, 8.0f);
	shape.mFlags |= 0x200;
	colliCopyShapes(mShape, &shape);
}

// 0x800F4C10
void TObjHiyoko::moveShape()
{
	Vec3f pos = mPos;
	pos.y += 8.0f;
	colliShapeSetPos(colliGetShape(mShape, 0), &pos);
}

// 0x800F4C68
void TObjHiyoko::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x800F4CA0
// a player landing on it squashes it and bounces them up
int TObjHiyoko::checkTouch()
{
	HiyokoVelMsg ask;
	HiyokoVelMsg bounce;
	HiyokoIdMsg touched;
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
				touched.mId   = mObjId;
				other->sendToHandle((ObjMsg*)&touched);
			}
		}
		ask.mType = 0x43;
		if (obj->sendToHandle((ObjMsg*)&ask) && ask.mVel.y < 0.0f) {
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

// 0x800F4E0C
void TObjHiyoko::calcMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxTranslate(mMatrix, mMatrix, 0.0f, mHop, 0.0f);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxScaleV(mMatrix, mMatrix, &mScale);
}

// 0x800F4E78
void TObjHiyoko::calcShadow()
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

// 0x800F4F28
void TObjHiyoko::setShadow(EggHits* hits)
{
	shadowSetFromContact(&mShadow, (u8*)hits);
}

// 0x800F4F4C
void TObjHiyoko::startConversation()
{
	if (mConversation == 0) {
		mConversation = createTCamConversation(0, 2, &mPos, 0xE38, 0x1555);
	}
}

// 0x800F4F9C
void TObjHiyoko::endConversation()
{
	if (mConversation != 0) {
		mConversation->mFlags |= 1;
		mConversation = 0;
	}
}

// 0x800F4FC0
void TObjHiyoko::startMode()
{
	switch (mMode) {
	case 0:
		setState(HIYOKO_WAIT);
		break;
	case 1:
		setState(HIYOKO_RUN);
		break;
	}
}

// 0x800F500C
void TObjHiyoko::draw()
{
	if (objDebugEnabled(this)) {
		mMotion.update();
		mObj.calcMotion();
		calcMatrix();
		calcShadow();
		makeSphere();
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

// 0x800F50E4
void TObjHiyoko::updateWait()
{
	updateState();
	calcMatrix();
}

// 0x800F5118
void TObjHiyoko::updateRun()
{
	updateState();
	calcMatrix();
	moveShape();
}

// 0x800F5154
int TObjHiyoko::setState(int state)
{
	switch (state) {
	case HIYOKO_NONE:
		return 0;
	case HIYOKO_WAIT:
		mMotion.playBlend(0, 0.05f);
		break;
	case HIYOKO_RUN:
		mMotion.playBlend(1, 0.05f);
		break;
	case HIYOKO_HOP_UP:
		mMotion.playBlend(2, 0.05f);
		break;
	case HIYOKO_HOP_DOWN:
		mMotion.playBlend(4, 0.05f);
		break;
	case HIYOKO_ACT:
		mMotion.playBlend(5, 0.05f);
		break;
	case HIYOKO_6:
	case HIYOKO_7:
		break;
	case HIYOKO_TURN:
		mMotion.playBlend(6, 0.05f);
		break;
	case HIYOKO_TALK: {
		mText = createSimpleTextPlain(getMessage(mMsgId));
		if (mText == 0) {
			return 0;
		}
		startConversation();
		ColliShape* shape = colliGetShape(mShape, 0);
		Vec3f at;		at.set(shape->mPos.x, shape->mPos.y, shape->mPos.z);
		sePlay3D(0x65, &at);
		if (HIYOKO_RAND() < 0.8f) {
			mMotion.playBlend(7, 0.05f);
		} else {
			mMotion.playBlend(8, 0.05f);
		}
		break;
	}
	}
	mState = state;
	mTimer = mWaitTime;
	return 1;
}

// 0x800F52E4
int TObjHiyoko::nextState()
{
	int ok = setState(mNextState);
	if (ok) {
		mNextState = 0;
	}
	return ok;
}

// 0x800F5324
// back to its mode with a new wait of 3 to 6 seconds
void TObjHiyoko::startWalk()
{
	mTalker.setId(0xFFFF);
	mTarget.x    = mPos.x;
	mTarget.y    = mPos.y;
	mTarget.z    = mPos.z;
	mTargetAngle = mRot.y;
	mNextState   = 0;
	mWaitTime    = ((int)(3.0f * HIYOKO_RAND() + 0.5f) + 3) * 60;
	startMode();
}

// 0x800F53D8
void TObjHiyoko::updateState()
{
	switch (mState) {
	case HIYOKO_WAIT:
		stateWait();
		break;
	case HIYOKO_RUN:
		stateRun();
		break;
	case HIYOKO_HOP_UP:
		stateHopUp();
		break;
	case HIYOKO_HOP_DOWN:
		stateHopDown();
		break;
	case HIYOKO_ACT:
		stateAct();
		break;
	case HIYOKO_TURN:
		stateTurn();
		break;
	case HIYOKO_TALK:
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

// 0x800F54C0
void TObjHiyoko::stateWait()
{
	checkTouch();
	if (mTimer < 0) {
		mNextState = HIYOKO_WAIT;
		setState(HIYOKO_ACT);
	}
	mTimer--;
}

// 0x800F5518
void TObjHiyoko::stateRun()
{
	checkTouch();
	if (mTargetAngle != mRot.y) {
		mRot.y = (u16)angleApproach(mRot.y, mTargetAngle, 0x800);
		return;
	}
	if (mTimer < 0) {
		mNextState = HIYOKO_RUN;
		setState(HIYOKO_ACT);
	}
	mTimer--;
	Vec3f step;
	vecSub(&step, &mTarget, &mPos);
	step.y   = 0.0f;
	f32 dist = njScalor(&step);
	if (dist < mSpeed) {
		mTarget.x    = mHome.x + mRange * (2.0f * (HIYOKO_RAND() - 0.5f));
		mTarget.z    = mHome.z + mRange * (2.0f * (HIYOKO_RAND() - 0.5f));
		mTargetAngle = (u16)angleToXZ(&mPos, &mTarget);
	} else {
		gjScaleVector(mSpeed / dist, &step, &step);
		vecAdd(&mPos, &mPos, &step);
	}
	fall();
}

// 0x800F5688
void TObjHiyoko::stateHopUp()
{
	if (mMotion.mCurrent != 2) {
		mHop += 0.25f;
	}
	if (mHop >= 20.0f) {
		mHop = 20.0f;
		nextState();
	}
	if (mMotion.isDone()) {
		mMotion.playBlend(3, 0.05f);
	}
}

// 0x800F570C
void TObjHiyoko::stateHopDown()
{
	mHop -= 0.5f;
	if (mHop <= 0.0f) {
		mHop = 0.0f;
		nextState();
	}
}

// 0x800F5754
void TObjHiyoko::stateAct()
{
	checkTouch();
	if (mMotion.isDone() && !nextState()) {
		startWalk();
	}
}

// 0x800F57A8
void TObjHiyoko::stateTurn()
{
	int angle = angleApproach(mRot.y, mTargetAngle, 0x800);
	if (mMotion.isDone() && angle == mTargetAngle) {
		setState(HIYOKO_TALK);
	}
	mRot.y = angle;
}

// 0x800F5810
void TObjHiyoko::stateTalk()
{
	ObjMsg done;

	if (mText != 0 && mText->mState == 3) {
		mText->mHold = 1;
		mText        = 0;
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

// 0x800F58A0
// gravity and the ground Returns 1 on landing
int TObjHiyoko::fall()
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

// 0x800F59A0
int TObjHiyoko::sendToHandle(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x40:
		return 0x40;
	case 0x70: {
		u16 from = ((HiyokoIdMsg*)msg)->mId;
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
		setState(HIYOKO_TURN);
		return 1;
	}
	}
	return 0;
}

// 0x800F5A74
// hides it when its light says it is out of view
void TObjHiyoko::vfn_10()
{
	ColliShape* shape = colliGetShape(mShape, 0);
	Vec3f center;	center.set(shape->mPos.x, shape->mPos.y, shape->mPos.z);
	if (mLight.isVisible(mDraw, 700.0f, &center, colliGetShape(mShape, 0)->mSizeX, 0)) {
		setDrawAmbient(&mLight, &mEquip1State);
		setDrawAmbient(&mLight, &mEquip2State);
		setDrawAmbient(&mLight, &mEquip8State);
		setDrawAmbient(&mLight, &mEquip10State);
		mDraw->mFlags &= ~1;
		shadowEnable(&mShadow);
	} else {
		mDraw->mFlags |= 1;
		shadowDisable(&mShadow);
	}
}

// 0x800F5B58
// the body's material colour the equip colour for the first material the model's own after
extern "C" void hiyokoMatColor(int chan, const u32* color)
{

	if (sHiyokoMatCount == 0) {
		GXSetChanMatColor(chan, *(GXColor*)&sHiyokoBodyColor);
	} else {
		GXSetChanMatColor(chan, *(GXColor*)&*color);
	}
	sHiyokoMatCount++;
}

// 0x800F5BB0
void TObjHiyoko::render()
{
	void* funcs[8];
	f32 mtx[12];

	shadowAlphaBegin();
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	funcs[1] = (void*)hiyokoMatColor;
	gjChangeDrawObjIF(funcs, 2);
	sHiyokoMatCount = 0;
	u32 color = mEquip2State.mMaterial;
	sHiyokoBodyColor.r = color >> 24;
	sHiyokoBodyColor.g = color >> 16;
	sHiyokoBodyColor.b = color >> 8;
	sHiyokoBodyColor.a = color;
	lightSetSelect(mLightSet);
	njSetTexture(mTexLists[0]);
	mLight.drawActor(mObj.mActor);
	funcs[1] = 0;
	gjChangeDrawObjIF(funcs, 2);

	ObjPlace* nodes;
	DrawStateSlot* slot;
	slot  = getDrawStateSlot();
	nodes = mObj.mNodes;
	if (mEquip & 4) {
		lightSetSelect(mLightSet);
		njSetTexture(mTexLists[3]);
		njPushMatrixEx();
		njMultiMatrix(0, nodes[14].mMatrix);
		mLight.drawObject(mModels[3]);
		njPopMatrixEx();
	}
	if (mEquip & 1) {
		lightSetSelect(mLightSet);
		beginDrawState(slot, &mEquip1State);
		njSetTexture(mTexLists[1]);
		njPushMatrixEx();
		njMultiMatrix(0, nodes[8].mMatrix);
		gjDrawObject(mModels[1]);
		njPopMatrixEx();
		endDrawState(slot);
	}
	if (mEquip & 2) {
		lightSetSelect(mLightSet);
		beginDrawState(slot, &mEquip2State);
		njSetTexture(mTexLists[2]);
		njPushMatrixEx();
		njMultiMatrix(0, nodes[14].mMatrix);
		gjDrawObject(mModels[2]);
		njPopMatrixEx();
		endDrawState(slot);
	}
	if (mEquip & 8) {
		lightSetSelect(mLightSet);
		beginDrawState(slot, &mEquip8State);
		njSetTexture(mTexLists[4]);
		njPushMatrixEx();
		njMultiMatrix(0, nodes[14].mMatrix);
		gjDrawObject(mModels[4]);
		njPopMatrixEx();
		endDrawState(slot);
	}
	if (mEquip & 0x10) {
		lightSetSelect(mLightSet);
		beginDrawState(slot, &mEquip10State);
		njSetTexture(mTexLists[5]);
		njPushMatrixEx();
		njMultiMatrix(0, nodes[8].mMatrix);
		gjDrawObject(mModels[5]);
		njPopMatrixEx();
		endDrawState(slot);
	}
	njPopMatrixEx();
	shadowAlphaEnd();
	if (mMode != 0 && objDebugEnabled(this)) {
		mtxConcat(mtx, gjGetMatrix(), mMatrix);
		mtxScale(mRange, 1.0f, mRange, mtx, mtx);
		mtxRotateX(mtx, mtx, 0x4000);
		debugDrawCylinder(mtx, &gColorGreen);
	}
}
