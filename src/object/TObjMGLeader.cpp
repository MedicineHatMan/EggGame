#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "camera/TCamera.h"
#include "ui/TSimpleText.h"
#include "ui/TAeObjGameOver.h"
#include "stage/EventScript.h"
#include "engine/Color.h"
#include "object/TObjLauncher.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "object/ObjShadow.h"
#include "io/Arc.h"
#include "ui/AeArc.h"
#include "ui/TAeObject.h"
#include "object/TObjMGLeader.h"
#include "object/ObjAction.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "engine/GroundQuery.h"
#include "stage/SetSlot.h"
#include "ui/MessageTable.h"
#include "sound/SoundCtrl.h"
#include "stage/TEventCtrl.h"
#include "ui/AeObjControl.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;
struct SetLink;

extern "C" {

// geobj_mg_leader.arc and what it holds
TArc* gMGLeaderArc;      // 0x802C5548
NJS_OBJECT* gMGLeaderModel;    // 0x802C554C
NJS_TEXLIST* gMGLeaderTexList;  // 0x802C5550
NJS_MOTION** gMGLeaderMotions;  // 0x802C5554
}

// What the 0x70 message a player talking carries
struct LeaderTalkMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mId;   // _04 the talker
};

// 0x1003 the line to say or below zero for none
struct LeaderLineMsg {
	u16 mType;   // _00
	u8 _02[2];
	s16 mMsgId;  // _04
};

// 0x1004 whether it takes talk
struct LeaderReadyMsg {
	u16 mType;   // _00
	u8 _02[2];
	int mReady;  // _04
};

// 0x8A asks for the height a talker looks at
struct LeaderHeightMsg {
	u16 mType;   // _00
	u8 _02[2];
	f32 mHeight; // _04
};

enum {
	LEADER_NONE = 0,
	LEADER_RESET = 1, // a new idle time then back to WAIT
	LEADER_WAIT  = 2,
	LEADER_TURN  = 3, // turns to the talker
	LEADER_TALK  = 4,
};

typedef char TObjMGLeader_size_check[(sizeof(TObjMGLeader) == 0x16C) ? 1 : -1];

const char* gTObjMGLeaderName = "TObjMGLeader";

static const MotionEntry sMGLeaderMotions[3] = {
	{ 0, 0, -1, 0, 0.0f, 30.0f, 0.4f, 0.0f, 0 },
	{ 0, 1, -1, 0, 0.0f, 30.0f, 0.4f, 0.0f, 0 },
	{ 1, 2, -1, 0, 0.0f, 45.0f, 0.4f, 0.0f, 0 },
};

#define LEADER_RAND() (3.0517578125e-05f * (f32)(int)rand())

// 0x8011292C
extern "C" void initMGLeader()
{
	gMGLeaderArc     = 0;
	gMGLeaderModel   = 0;
	gMGLeaderTexList = 0;
	gMGLeaderMotions = 0;
}

// 0x80112944
extern "C" void releaseMGLeader()
{
	if (gMGLeaderArc != 0) {
		releaseTextures(gMGLeaderTexList);
		releaseResource(gMGLeaderArc);
	}
	gMGLeaderArc     = 0;
	gMGLeaderModel   = 0;
	gMGLeaderTexList = 0;
	gMGLeaderMotions = 0;
}

// 0x80112990
// the event script's load_mgleader
extern "C" int scriptLoadMGLeader(void*)
{
	loadMGLeader();
	return 1;
}

// 0x801129B4
extern "C" void loadMGLeader()
{
	loadArc("geobj_mg_leader.arc", &gMGLeaderArc);
	gMGLeaderModel   = *(NJS_OBJECT**)findArcExport(gMGLeaderArc, "models");
	gMGLeaderTexList = (NJS_TEXLIST*)(*(void**)findArcExport(gMGLeaderArc, "texlists"));
	bindGvmTextures((u8*)findArcExport(gMGLeaderArc, "texture"), (NJS_TEXLIST*)gMGLeaderTexList);
	gMGLeaderMotions = (NJS_MOTION**)findArcExport(gMGLeaderArc, "motions");
}

// 0x80112A30
extern "C" TObject* createTObjMGLeader(TObject* parent, SetDesc* desc)
{
	return new TObjMGLeader(parent, desc);
}

// 0x80112A84
void TObjMGLeader::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mHold    = 0;
	mEventId = desc->getInt(2);
}

// 0x80112AD8
void TObjMGLeader::debugPrint(void*)
{
	njPrintC(0xA0013, "");
	njPrint(0xA0014, "");
	njPrintC(0xA0015, ":EVENT ID");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x80112B7C
TObjMGLeader::TObjMGLeader(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjMGLeaderName;
	mSize = 0x16C;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	makeShape();
	initFromDesc(desc);
	reset();
}

// 0x80112C3C
TObjMGLeader::~TObjMGLeader()
{
	if (mText != 0) {
		mText->mHold = 1;
	}
	freeShape();
	removeDrawTask(mDraw);
}

// 0x80112CE4
void TObjMGLeader::reset()
{
	mTalker.setId(0xFFFF);
	mStart.x   = mPos.x;
	mStart.y   = mPos.y;
	mStart.z   = mPos.z;
	mAim       = mRot.y;
	mCam       = 0;
	mTimer     = 0;
	mIdleTime  = 0;
	_164       = 0;
	mText      = 0;
	mReady     = 1;
	makeCylinder();
	calcMatrix();
	if (gMGLeaderArc != 0) {
		mMotion.init(sMGLeaderMotions, gMGLeaderMotions);
		mMotion.play(0);
		mObj.setModel(gMGLeaderModel);
		mObj.mTexList = gMGLeaderTexList;
		mObj.mState   = getDefaultDrawState();
		mObj.mMotion  = &mMotion;
		mObj.calcMotion();
		mShadow._30 = 10.0f;
		mShadow._34 = 10.0f;
		calcShadow();
	} else {
		shadowDisable(&mShadow);
	}
	startIdle();
	setLinkFree((SetLink*)&mLink);
}

// 0x80112DF4
void TObjMGLeader::makeShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xF6, 0xF6);
}

// 0x80112E50
void TObjMGLeader::makeCylinder()
{
	ColliShape shape;
	colliShapeMakeCylinder(&shape, &mPos, 1, 10.0f, 10.0f);
	shape.mFlags |= 0x200;
	colliCopyShapes(mShape, &shape);
}

// 0x80112EA8
void TObjMGLeader::moveShape()
{
	Vec3f pos = mPos;
	pos.y += 10.0f;
	colliShapeSetPos(colliGetShape(mShape, 0), &pos);
}

// 0x80112F00
void TObjMGLeader::freeShape()
{
	colliRetire(mShape);
	mShape = 0;
}

// 0x80112F38
void TObjMGLeader::calcMatrix()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, mRot.y);
}

// 0x80112F7C
void TObjMGLeader::calcShadow()
{
	EggHits hits;
	Vec3f probe = mPos;
	probe.y += 10.0f;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&probe, -1) != 0 && (hits.mMask & 1)) {
		setShadow(&hits);
	} else {
		shadowSetGround(&mShadow, &mPos, 0);
	}
}

// 0x8011302C
void TObjMGLeader::setShadow(EggHits* hits)
{
	shadowSetFromContact(&mShadow, (u8*)hits);
}

// 0x80113050
void TObjMGLeader::startConversation()
{
	if (mCam == 0) {
		Vec3f focus = mPos;
		focus.y += 20.0f;
		mCam = createTCamConversation(0, 2, &focus, 0xAAA, 0x2000);
	}
}

// 0x801130C8
void TObjMGLeader::endConversation()
{
	if (mCam != 0) {
		mCam->mFlags |= 1;
		mCam = 0;
	}
}

// 0x801130EC
void TObjMGLeader::startMode()
{
	switch (mHold) {
	case 0:
		setAction(LEADER_WAIT);
		break;
	}
}

// 0x80113120
void TObjMGLeader::draw()
{
	if (gMGLeaderArc == 0) {
		calcMatrix();
		moveShape();
		calcShadow();
		return;
	}
	if (objDebugEnabled(this)) {
		mMotion.update();
		mObj.calcMotion();
		calcMatrix();
		moveShape();
		calcShadow();
		return;
	}
	if (isInRange(this, &mPos, 600.0f)) {
		switch (mHold) {
		case 0:
			think();
			break;
		}
		mObj.calcMotion();
	}
}

// 0x801131DC
void TObjMGLeader::think()
{
	updateAction();
}

// 0x801131FC
int TObjMGLeader::setAction(int action)
{
	switch (action) {
	case LEADER_NONE:
		return 0;
	case LEADER_RESET:
		startIdle();
		return 1;
	case LEADER_WAIT:
		mMotion.playBlend(0, 0.05f);
		break;
	case LEADER_TURN:
		mMotion.playBlend(2, 0.05f);
		break;
	case LEADER_TALK:
		mText = createSimpleTextPlain(getMessage(mMsgId));
		if (mText == 0) {
			return 0;
		}
		mMotion.playBlend(1, 0.05f);
		startConversation();
		break;
	}
	mAction = action;
	mTimer  = mIdleTime;
	return 1;
}

// 0x801132DC
// a new idle time of 3 to 6 seconds
void TObjMGLeader::startIdle()
{
	_164      = 0;
	mIdleTime = ((int)(3.0f * LEADER_RAND() + 0.5f) + 3) * 60;
	startMode();
}

// 0x80113360
void TObjMGLeader::updateAction()
{
	switch (mAction) {
	case LEADER_WAIT:
		actWait();
		break;
	case LEADER_TURN:
		actTurn();
		break;
	case LEADER_TALK:
		actTalk();
		break;
	}
	mMotion.update();
}

// 0x801133CC
void TObjMGLeader::actWait() { }

// 0x801133D0
void TObjMGLeader::actTurn()
{
	int angle;

	if ((int)(10.0f * mMotion.mFrame + 0.5f) == 120) {
		sePlay(0x71);
	}
	angle = angleApproach(mRot.y, mAim, 0x400);
	if (angle == mAim && mMotion.isDone()) {
		setAction(LEADER_TALK);
	}
	mRot.y = angle;
	calcMatrix();
}

// 0x8011346C
void TObjMGLeader::actTalk()
{
	ObjMsg done;

	if ((mText->mState == 3) ? 1 : 0) {
		mText->mHold = 1;
		mText        = 0;
		endConversation();
		eventScriptSignal();
		TObjAction* talker = mTalker.get();
		if (talker != 0) {
			done.mType = 0x30;
			talker->sendToHandle(&done);
		}
		setAction(LEADER_WAIT);
	}
}

// 0x801134FC
int TObjMGLeader::sendToHandle(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x1004: {
		LeaderReadyMsg ready = *(LeaderReadyMsg*)msg;
		mReady = ready.mReady;
		return 1;
	}
	case 0x40:
		return mReady != 0 ? 0x40 : 0;
	case 0x70: {
		if (mReady == 0) {
			return 0;
		}
		u16 from = ((LeaderTalkMsg*)msg)->mId;
		TObjAction* obj = findObjById(from);
		if (obj != 0) {
			Vec3f* at = obj->getPos();
			mAim      = (u16)angleToXZ(&mPos, at);
		} else {
			return 0;
		}
		mTalker.setId(((LeaderTalkMsg*)msg)->mId);
		return eventScriptRequest(mEventId, (u16)mObjId);
	}
	case 0x8A:
		if (mReady == 0) {
			return 0;
		}
		((LeaderHeightMsg*)msg)->mHeight = 20.0f;
		return 1;
	case 0x1003: {
		s16 line = ((LeaderLineMsg*)msg)->mMsgId;
		if (line < 0 || mReady == 0) {
			ObjMsg done;
			eventScriptSignal();
			if (mTalker.get() != 0) {
				done.mType = 0x30;
				mTalker.get()->sendToHandle(&done);
			}
			return 0;
		}
		mMsgId = line;
		eventHideIdU16(mTalker.getId());
		setAction(LEADER_TURN);
		return 1;
	}
	}
	return 0;
}

// 0x801136B4
// hidden with its shadow while the camera can't see it
void TObjMGLeader::vfn_10()
{
	ColliShape* shape = colliGetShape(mShape, 0);
	Vec3f center;
	center.set(shape->mPos.x, shape->mPos.y, shape->mPos.z);
	if (cameraIsVisible(500.0f, 1.5f * colliGetShape(mShape, 0)->mSizeY, &center, 0)) {
		mDraw->mFlags &= ~1;
		shadowEnable(&mShadow);
	} else {
		mDraw->mFlags |= 1;
		shadowDisable(&mShadow);
	}
}

// 0x80113764
void TObjMGLeader::render()
{
	f32 mtx[12];

	if (gMGLeaderArc == 0) {
		mtxConcat(mtx, gjGetMatrix(), mMtx);
		mtxScale(10.0f, 10.0f, 10.0f, mtx, mtx);
		debugWireSphere(mtx, &gColorBlack);
		return;
	}
	shadowAlphaBegin();
	njSetTexture(gMGLeaderTexList);
	njPushMatrixEx();
	njMultiMatrix(0, mMtx);
	mObj.draw();
	njPopMatrixEx();
	shadowAlphaEnd();
}
