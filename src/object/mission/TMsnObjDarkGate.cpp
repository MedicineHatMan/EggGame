#include "object/purple/TObjPurpleGear.h"
struct DrawTask;
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "camera/EventCamArc.h"
#include "stage/SetActor.h"
#include "camera/EventCamRef.h"
#include "engine/ModelDraw.h"
#include "object/ObjModel.h"
#include "engine/CharaColli.h"
#include "stage/StageObjUtil.h"
#include "engine/Color.h"
#include "stage/TEventCtrl.h"
#include "ui/AeArc.h"
#include "object/TObjLauncher.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/FxSystem.h"
#include "object/TObjEggRing.h"
#include "camera/TCameraCtrl.h"
#include "stage/TGameState.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "object/TObjKey00.h"
#include "core/VecMath.h"
#include "sound/SoundCtrl.h"
#include "object/mission/TMsnObjDarkGate.h"
#include "object/mission/TMsnObjEmblem.h"
#include "Ninja/ninja.h"
#include "object/ObjScript.h"
struct NJS_TEXLIST;
struct CharaColli;
struct ColliShape;
struct Fx;

// 0x80103960
// 30.0f is 30 the radius until loadDarkGateAssets replaces it
extern "C" void resetDarkGateAssets()
{
	sDarkGateArc     = 0;
	gDarkGateModel   = 0;
	gDarkGateTexList = 0;
	gDarkGateRadius  = 30.0f;
}

// 0x8010397C
extern "C" void releaseDarkGateAssets()
{
	if (sDarkGateArc != 0) {
		releaseTextures(gDarkGateTexList);
		releaseResource(sDarkGateArc);
		sDarkGateArc     = 0;
		gDarkGateModel   = 0;
		gDarkGateTexList = 0;
	}
}

// 0x801039C4
extern "C" int prepareDarkGateAssets()
{
	loadDarkGateAssets();
	return 1;
}

// 0x801039E8
// "model" "texlist" and "texture" are the exports
extern "C" void loadDarkGateAssets()
{
	loadArc("geobj_darkgate.arc", &sDarkGateArc);
	gDarkGateModel   = (NJS_OBJECT*)findArcExport(sDarkGateArc, "model");
	gDarkGateTexList = (NJS_TEXLIST*)findArcExport(sDarkGateArc, "texlist");
	void* texture    = findArcExport(sDarkGateArc, "texture");
	bindGvmTextures((u8*)texture, gDarkGateTexList);

	gDarkGateRadius = gDarkGateModel->model->r;
}

enum {
	GATE_HIDDEN = 0, // for the camera
	GATE_OPEN   = 1,
	GATE_APPEAR = 2,
	GATE_DONE   = 3,
};

enum {
	APPEAR_START   = 0,
	APPEAR_FADE    = 1,
	APPEAR_END     = 2,
	APPEAR_LEAVE   = 3,
	APPEAR_LEAVING = 4,
	APPEAR_LEFT    = 5,
};

// Message 0x14 go through the gate to its place
struct GateEnterMsg : public ObjMsg {
	u8 _02[2];  // _02
	Vec3f mPos; // _04
	int mRotY;  // _10
};

typedef char TMsnObjDarkGate_size_check[(sizeof(TMsnObjDarkGate) == 0x130) ? 1 : -1];

// 0x80103A58
extern "C" TObject* createTMsnObjDarkGate(TObject* parent, SetDesc* desc)
{
	return new TMsnObjDarkGate(parent, desc);
}

f32 gDarkGateRadius              = 50.0f;
const char* gTMsnObjDarkGateName = "TMsnObjDarkGate";

// 0x80103AAC
TMsnObjDarkGate::TMsnObjDarkGate(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
    , mGlow(0)
    , mCamera(0)
    , mEventCam(0)
    , mOpenFx(0)
    , mTimer(0)
    , mMusic(-1)
    , mDelay(30)
{
	_00       = (u32)gTMsnObjDarkGateName;
	mSize     = 0x130;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	mShape    = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 255, 255, 254);
	mDraw.setModel(gDarkGateModel);
	mDraw.mDrawState = &mState;
	mDraw.mTexList   = gDarkGateTexList;
	mDraw.mMatrix    = mMatrix;
	mState.mAmbient  = -1;
	mGlow            = fxSpawnFollow(1, 7, 113, mMatrix);
	initFromDesc(desc);
}

// 0x80103BF0
TMsnObjDarkGate::~TMsnObjDarkGate()
{
	fxStop((Fx*)mGlow);
	if (mEventCam != 0) {
		mEventCam->mFlags |= 1;
		mEventCam = 0;
	}
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x80103CB4
// Int 0 hides the gate until switch int 1 0..0x7FF
void TMsnObjDarkGate::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mHidden = CLAMP_RANGE(desc->getInt(0), 0, 1);
	mSwitch = CLAMP_RANGE(desc->getInt(1), 0, 0x7FF);
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	colliShapeMakeSphere(colliGetShapes(mShape), &mPos, 0, 30.0f);
	if (mHidden == 1 && keyBitIsSet(mSwitch) == 0) {
		moveDrawTask(mDrawTask, 0x22);
		colliDisable(mShape);
		mState.mFlags = DRAWSTATE_AMBIENT;
		setColorAlpha(&mState.mAmbient, 0);
		if (mGlow != 0) {
			fxPause(mGlow);
		}
		mGate   = GATE_HIDDEN;
		mAppear = APPEAR_START;
	} else {
		mState.mFlags = 0;
		if (mGlow != 0) {
			fxResume(mGlow);
		}
		mGate   = GATE_OPEN;
		mAppear = APPEAR_LEAVE;
	}

	mSetCam = CLAMP_RANGE(desc->getInt(2), 0, 1);
	if (mSetCam != 0) {
		f32 z     = desc->getFloat(2);
		f32 y     = desc->getFloat(1);
		mCamEye.x = desc->getFloat(0);
		mCamEye.y = y;
		mCamEye.z = z;
		vecRotateY(&mCamEye, &mCamEye, mRot.y);
		vecAdd(&mCamEye, &mCamEye, &mPos);
		mCamAt = mPos;
		mCamAt.y += desc->getFloat(3);
	} else {
		mCamAt = mPos;
		vecFromAngleXZ(200.0f, &mCamEye, mRot.y);
		mCamEye.y += 80.0f;
		vecAdd(&mCamEye, &mCamEye, &mPos);
	}
}

// 0x80104018
// The lines are the set-file labels
void TMsnObjDarkGate::debugPrint(void*)
{
	njPrintC(0xA0013, mHidden == 0 ? "TYPE NORMAL" : "TYPE HIDDEN");
	if (mHidden == 1) {
		njPrintC(0xA0014, "SWITCH ID");
	}
	njPrintC(0xA0015, ":SET CAM POS");
	if (mSetCam != 0) {
		njPrintC(0xA0017, ":OFFSET-X");
		njPrintC(0xA0018, ":OFFSET-Y");
		njPrintC(0xA0019, ":OFFSET-Z");
		njPrintC(0xA001A, ":TGT-OFFSET-Y");
	}
}

// 0x801040E0
int TMsnObjDarkGate::takeCamera(ObjMsg* msg)
{
	mCamRef.set(msg);
	return 1;
}

// 0x80104108
void TMsnObjDarkGate::draw()
{
	if (isInRange(this, &mPos, 500.0f) || mCamRef.isActive()) {
		switch (mGate) {
		case GATE_HIDDEN:
			waitCamera();
			break;
		case GATE_OPEN:
			waitUser();
			break;
		case GATE_APPEAR:
			appear();
			break;
		case GATE_DONE:
			done();
			break;
		}
	}
}

// 0x801041AC
void TMsnObjDarkGate::waitCamera()
{
	if (mCamRef.isActive()) {
		mGate = GATE_APPEAR;
	}
}

// 0x801041EC
// After a short delay the shape comes on
void TMsnObjDarkGate::waitUser()
{
	if (mDelay > -1 && (mDelay = mDelay - 1, mDelay == 0)) {
		colliEnable(mShape);
		mDelay = 0;
		return;
	}
	for (ColliContact* contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliContactNoBit10(contact) == 0) {
			continue;
		}
		TObjAction* other = findObjById((u16)colliContactOwner(contact));
		if (other == 0) {
			continue;
		}
		GateEnterMsg msg;
		msg.mType = 0x14;
		msg.mPos  = mPos;
		msg.mRotY = mRot.y;
		if (other->sendToHandle(&msg) != 0) {
			mUser.setId(colliContactOwnerU16(contact));
			mGate = GATE_APPEAR;
		}
	}
}

// 0x801042E8
void TMsnObjDarkGate::appear()
{
	switch (mAppear) {
	case APPEAR_START:
		appearStart();
		break;
	case APPEAR_FADE:
		appearFade();
		break;
	case APPEAR_END:
		appearEnd();
		break;
	case APPEAR_LEAVE:
		leave();
		break;
	case APPEAR_LEAVING:
		leaving();
		break;
	case APPEAR_LEFT:
		left();
		break;
	}
}

// 0x80104368
void TMsnObjDarkGate::done()
{
}

// 0x8010436C
void TMsnObjDarkGate::appearStart()
{
	mCamera = createTCamFix(0, 2, &mCamAt, &mCamEye);
	if (mCamera == 0) {
		mAppear = APPEAR_END;
	} else {
		((TCamera*)mCamera)->setCut();
		mOpenFx = fxSpawnFollow(1, 7, 0x6E, mMatrix);
		sePlay(0x5E);
		mTimer  = 0;
		mAppear = APPEAR_FADE;
	}
}

// A macro the DOL reads the faded value twice with no local for it
#define clampHigh(x, max) ((x) > (max) ? (max) : (x))

// 0x801043F8
// Fades in from frame 20 over 40 frames
void TMsnObjDarkGate::appearFade()
{
	if (mTimer >= 20) {
		setColorAlpha(&mState.mAmbient, (int)clampHigh(255.0f * ((f32)(mTimer - 20) / 40.0f), 255.0f));
	}
	if (mTimer == 240) {
		mAppear = APPEAR_END;
	}
	mTimer++;
}

// 0x801044A0
void TMsnObjDarkGate::appearEnd()
{
	fxStop(mOpenFx);
	if (mCamera != 0) {
		((TObject*)mCamera)->mFlags |= 1;
		mCamera = 0;
	}
	mCamRef.release();
	moveDrawTask(mDrawTask, 0x21);
	if (mGlow != 0) {
		fxResume(mGlow);
	}
	mAppear = APPEAR_LEAVE;
	mGate   = GATE_OPEN;
}

// 0x80104524
// The leaving event's hide callback hides the one going through and starts the music
static int hideUser(u16 id)
{
	TMsnObjDarkGate* gate = (TMsnObjDarkGate*)findObjById((u16)id);
	eventHideIdU16(gate->mUser.getId());
	bgmFadeTo(0.0f, 0xB4);
	gate->mMusic = sePlayFor(0x2B9, 0);
	return 1;
}

// 0x80104580
void TMsnObjDarkGate::leave()
{
	if (eventStartById((u16)mObjId, (int (*)(int))hideUser) != 0) {
		mEventCam = eventCamArcPlay(1.0f, 0.0f, 10);
		if (mEventCam == 0) {
			mAppear = APPEAR_LEFT;
		} else {
			((TCamera*)mEventCam)->setCut();
			mEventCam->attachToMtx();
			mtxCopy(mEventCam->getMtx(), mMatrix);
			if (mGlow != 0) {
				fxPause(mGlow);
			}
			moveDrawTask(mDrawTask, 0x22);
			mAppear = APPEAR_LEAVING;
		}
	}
}

// 0x8010462C
void TMsnObjDarkGate::leaving()
{
	int frame = (int)(0.5f + mEventCam->getFrame());
	switch (frame) {
	case 130:
		f32 mtx[12];
		mtxTranslate(mtx, mMatrix, 0.0f, 50.0f, 0.0f);
		fxSpawnAt(1, 7, 0x5C, mtx);
		break;
	case 232:
		break;
	}
	if (mEventCam->isDone() != 0) {
		mAppear = APPEAR_LEFT;
	}
}

// 0x801046CC
void TMsnObjDarkGate::left()
{
	seStop(mMusic);
	eventEnd();
	mUser.setId(0xFFFF);
	ObjMsg msg;
	msg.mType = 0x97;
	sendGameMsg(&msg);
	mGate = GATE_DONE;
}

// 0x80104728
void TMsnObjDarkGate::vfn_10()
{
	Vec3f view;
	if ((u8)mState.mAmbient == 0) {
		mDrawTask->mFlags |= 1;
	} else if (cameraIsVisible(1000.0f, gDarkGateRadius, &mPos, &view) != 0) {
		if (mState.mFlags & DRAWSTATE_AMBIENT) {
			mDrawTask->mSortKey = drawTaskDepthKey(view.z);
		}
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x801047CC
// Without the model it draws a black box
void TMsnObjDarkGate::render()
{
	if (gDarkGateModel == 0) {
		f32 mtx[12];
		mtxConcat(mtx, (f32*)gjGetMatrix(), mMatrix);
		mtxScale(30.0f, 2.0f, 30.0f, mtx, mtx);
		debugWireCylinder(mtx, &gColorBlack);
	} else {
		mDraw.draw();
	}
}

// 0x80104844
int TMsnObjDarkGate::sendToHandle(ObjMsg* msg)
{
	switch (msg->mType) {
	case 199:
		return takeCamera(msg);
		break;
	}
	return 0;
}
