#include "ui/TextVec.h"
struct DrawTask;
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "camera/EventCamArc.h"
#include "stage/EventScript.h"
#include "sound/SoundHandle.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "stage/SetManager.h"
#include "engine/DrawTask.h"
#include "camera/EventCamRef.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "engine/ModelDraw.h"
#include "object/TObjLauncher.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "stage/SetSlot.h"
#include "object/TObjKey00.h"
#include "enemy/boss/TEneYellowBossGreen.h"
#include "sound/SoundCtrl.h"
#include "core/VecMath.h"
#include "stage/TEventCtrl.h"
#include "engine/TFrameBlur.h"
#include "object/yellow/TObjYellowGateSet.h"
#include "object/ObjScript.h"

static const f32 sGateSetExtent[3] = { 1.807f, 906.5f, -2100.7f };

// Where the six pedestals and the centre stand before the set's yaw
const Vec3f sGateSetOffsets[7] = {
	{ -89.307f, 0.0f, 154.4f }, { -186.907f, 0.0f, -1.8f }, { -91.037f, 0.0f, -156.0f }, { 87.423f, 0.0f, -156.0f },
	{ 185.1f, 0.0f, -1.8f },    { 87.5f, 0.0f, 154.4f },    { 0.0f, 0.0f, 0.0f },
};

// Nothing in the unit reads these two
const char sGatePartNames[6][12] = {
	"gate_green", "gate_blue", "gate_red", "gate_purple", "gate_orange", "gate_yellow",
};
const u32 sGatePartBits[7] = { 0x100, 0x200, 0x400, 0x800, 0x40, 0x20, 0 };
struct SetLink;
struct ColliContact;
struct ColliShape;
struct CharaColli;
struct Fx;

extern "C" {

// The event camera handle See TObjYellowSlideDoor.cpp
}

/** The message that moves the player to the gate */
struct GateMoveMsg {
	u16 mId;
	u16 _02;
	Vec3f mPos; // _04
	int mRotY;  // _10
};

typedef char TObjWidgetGate_size_check[(sizeof(TObjWidgetGate) == 0xD4) ? 1 : -1];

typedef char TObjYellowGateSet_size_check[(sizeof(TObjYellowGateSet) == 0xE8) ? 1 : -1];

const char* gTObjWidgetGateName  = "TObjWidgetGate";
const f32 gYellowGateSetSetRange = 10000.0f;

// 0x80191A9C
TObjWidgetGate::TObjWidgetGate(TObject* parent, int index, NJS_OBJECT* model, NJS_TEXLIST* texList, int effect)
    : TObjAction(parent)
    , mIndex(index)
    , mModelData(model)
    , mTexList(texList)
    , mEffectId(effect)
    , mEffect(0)
    , _D0(0)
{
	_00   = (u32)gTObjWidgetGateName;
	mSize = 0xD4;
	mtxIdentity(mMatrix);
	mModel.setModel(mModelData);
	mModel.mDrawState = getDefaultDrawState();
	mModel.mTexList   = mTexList;
	mModel.mMatrix    = mMatrix;
	mDrawTask         = addDrawTask(this, (void (*)(void*))drawObjectTask, 34, 0);
	mEffect           = fxSpawnFollow(1, 8, mEffectId, mMatrix);
	sePlay3DHeld(500.0f, &mSound, 0x266, 100, &gVecZero);
}

// 0x80191BA4
TObjWidgetGate::~TObjWidgetGate()
{
	seStop3D(&mSound);
	fxStop((Fx*)mEffect);
	removeDrawTask(mDrawTask);
}

// 0x80191C28
void TObjWidgetGate::setPlace(const Vec3f* pos, int rotY)
{
	mPos  = *pos;
	mRotY = rotY;
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRotY);
	mSound.mPos = mPos;
}

// 0x80191CA0
void TObjWidgetGate::startEffect()
{
	if (mEffect != 0) {
		fxResume(mEffect);
	}
}

// 0x80191CCC
void TObjWidgetGate::stopEffect()
{
	if (mEffect != 0) {
		fxPause(mEffect);
	}
}

// 0x80191CF8
void TObjWidgetGate::draw()
{
}

// 0x80191CFC
void TObjWidgetGate::vfn_10()
{
	Vec3f screen;
	f32 radius = mModelData->model->r;
	if (cameraIsVisible(800.0f, radius, &mPos, &screen) != 0) {
		mDrawTask->mSortKey = drawTaskDepthKey(screen.z);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80191D7C
void TObjWidgetGate::render()
{
	mModel.draw();
}

// 0x80191DA0
extern "C" TObject* createTObjYellowGateSet(TObject* parent, SetDesc* desc)
{
	return new TObjYellowGateSet(parent, desc);
}

const char* gTObjYellowGateSetName = "TObjYellowGateSet";

// 0x80191DF4
TObjYellowGateSet::TObjYellowGateSet(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
    , mEvent(0)
    , mSound(-1)
{
	_00   = (u32)gTObjYellowGateSetName;
	mSize = 0xE8;
	setObjId(this, desc->mObjId);
	mLink.get()->attach((SetDesc*)desc);

	mModelData = getGeObjModels()[10];
	mTexList   = getGeObjTextures()[10];
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 34, 0);

	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 255, 255, 254);

	mParts[0] = 0;
	mStep     = 0;
	initFromDesc((SetDesc*)desc);
}

// 0x80191F18
TObjYellowGateSet::~TObjYellowGateSet()
{
	colliRetire((CharaColli*)mShape);
	removeDrawTask(mDrawTask);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	releaseObjId(this);
}

// 0x80191FC0
// The part offsets are added as they are the turned copy of each one is overwritten straight away
static inline void rotateOffset(const Vec3f& from, Vec3f* out, int angle)
{
	Vec3f offset;
	offset = from;
	vecRotateY(&offset, out, angle);
}

static inline void addOffset(Vec3f* out, const Vec3f* base, const Vec3f& from)
{
	Vec3f offset;
	offset = from;
	vecAdd(out, base, &offset);
}

void TObjYellowGateSet::initFromDesc(SetDesc* desc)
{
	desc->getPos(&mPos);
	desc->getRot(&mRot);
	updateMatrix();

	f32 radius = 0.5f * mModelData->model->r;
	colliShapeMakeSphere(colliGetShape(mShape, 0), &mPos, 0, radius);

	mSwitchId    = desc->getInt(0);
	mEventBefore = desc->getInt(1);
	mEventAfter  = desc->getInt(2);
	mEventFailed = desc->getInt(3);

	if (keyBitIsSet(mSwitchId) == 0) {
		createParts(0);
		mAct = 0;
	} else {
		createParts(1);
		mAct = 4;
	}

	for (int i = 0; i < 7; i++) {
		Vec3f pos;
		rotateOffset(sGateSetOffsets[i], &pos, mRot.y);
		addOffset(&pos, &mPos, sGateSetOffsets[i]);
		mParts[i]->setPlace(&pos, mRot.y);
	}
}

// 0x80192194
void TObjYellowGateSet::debugPrint(void*)
{
	njPrint(0xA0005, "=%f", sGateSetExtent[0]);
	njPrint(0xA0006, "=%f", sGateSetExtent[1]);
	njPrint(0xA0007, "=%f", sGateSetExtent[2]);
	njPrintC(0xA0013, ":SWITCH ID");
	njPrintC(0xA0014, ":EVID ANTE");
	njPrintC(0xA0015, ":EVID POST");
	njPrintC(0xA0016, ":EVID FAILED");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x80192290
void TObjYellowGateSet::createParts(int opened)
{
	if (mParts[0] != 0) {
		return;
	}
	NJS_OBJECT* centre     = getGeObjModels()[10];
	NJS_OBJECT* pedestal   = getGeObjModels()[11];
	NJS_TEXLIST** textures = getGeObjTextures();
	int effect             = (opened != 0) ? 0x5C : 0x42;

	mParts[6] = new TObjWidgetGate(this, 6, centre, textures[10], effect);
	mParts[0] = new TObjWidgetGate(this, 0, pedestal, textures[11], 0x2F);
	mParts[1] = new TObjWidgetGate(this, 1, pedestal, textures[12], 0x1F);
	mParts[2] = new TObjWidgetGate(this, 2, pedestal, textures[13], 0x12);
	mParts[3] = new TObjWidgetGate(this, 3, pedestal, textures[14], 0x37);
	mParts[4] = new TObjWidgetGate(this, 4, pedestal, textures[15], 0x3F);
	mParts[5] = new TObjWidgetGate(this, 5, pedestal, textures[16], 0x27);
	if (opened == 0) {
		mParts[6]->stopEffect();
	}
}

// 0x8019246C
void TObjYellowGateSet::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
}

// 0x801924B0
int TObjYellowGateSet::onCameraMessage(ObjMsg* msg)
{
	mCam.set(msg);
	return 1;
}

// 0x801924D8
void TObjYellowGateSet::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 500.0f) && (mAct == 0 || mAct == 4)) {
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateCamera();
		break;
	case 2:
		stateEvent();
		break;
	case 3:
		stateFinish();
		break;
	case 4:
		stateDone();
		break;
	}
}

// 0x801925A4
void TObjYellowGateSet::stateWait()
{
	for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
		if (colliContactNoBit10(hit) != 0) {
			if (storySeqEval(0x30) != 0) {
				if (eventScriptRequest(mEventBefore) != 0) {
					mToucher.setId(colliContactOwnerU16(hit));
					mSound = sePlayFor(0x2BA, 0);
					mAct   = 1;
				}
			} else if (eventScriptRequest(mEventFailed) != 0) {
				mAct = 4;
			}
		}
	}
}

// 0x8019265C
void TObjYellowGateSet::stateCamera()
{
	if (mCam.isActive() != 0) {
		mAct = 2;
	}
}

// 0x8019269C
void TObjYellowGateSet::stateEvent()
{
	switch (mStep) {
	case 0:
		startEvent();
		// fall through
	case 1:
		updateEvent();
		return;
	case 2:
		endEvent();
		break;
	}
	mCam.release();
	mStep = 0;
	mAct  = 3;
}

// 0x80192718
void TObjYellowGateSet::stateFinish()
{
	if (eventScriptRequest(mEventAfter) != 0) {
		if (mToucher.get() != 0) {
			u16 msg = 0x79;
			mToucher.get()->sendToHandle((ObjMsg*)&msg);
		}
		mToucher.setId(0xFFFF);
		mAct = 4;
	}
}

// 0x8019279C
void TObjYellowGateSet::stateDone()
{
}

// 0x801927A0
void TObjYellowGateSet::startEvent()
{
	mEvent = eventCamArcPlay(1.0f, 0.0f, 0x100);
	if (mEvent != 0) {
		((TCamera*)mEvent)->setCut();
		if (mToucher.get() != 0) {
			GateMoveMsg msg;
			msg.mId   = 0x7D;
			msg.mPos  = mPos;
			msg.mRotY = mRot.y + 0x8000;
			mToucher.get()->sendToHandle((ObjMsg*)&msg);
			eventHideIdU16(mToucher.getId());
		}
		mStep = 1;
	} else {
		mStep = 2;
	}
}

static inline TextVec makeVec(f32 x, f32 y, f32 z)
{
	TextVec v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// 0x80192868
// The five places flashes go as world positions a function static set up on first use
static void spawnGateFlash(int index)
{
	static TextVec sFlashPos[5] = {
		makeVec(-1678.94f, 679.5463f, -1687.62f), makeVec(-1272.71f, 679.5463f, -1455.49f), makeVec(-1388.78f, 679.5463f, -1751.45f),
		makeVec(-1301.73f, 679.5463f, -1281.39f), makeVec(-1725.36f, 679.5463f, -1536.73f),
	};

	f32 mtx[12];
	int i = (index < 0) ? 0 : ((index > 4) ? 4 : index);
	mtxSetTransV(mtx, &sFlashPos[i]);
	fxSpawnAt(1, 8, 0x5A, mtx);
}

// 0x801929D8
void TObjYellowGateSet::updateEvent()
{
	switch ((int)(0.5f + mEvent->getFrame())) {
	case 360:
		spawnGateFlash(0);
		spawnGateFlash(1);
		spawnGateFlash(2);
		spawnGateFlash(3);
		spawnGateFlash(4);
		break;
	case 380:
		spawnGateFlash(2);
		spawnGateFlash(3);
		break;
	case 400:
		spawnGateFlash(4);
		spawnGateFlash(1);
		break;
	case 420:
		spawnGateFlash(2);
		spawnGateFlash(3);
		break;
	case 440:
		spawnGateFlash(4);
		spawnGateFlash(1);
		break;
	case 481:
		mParts[6]->startEffect();
		break;
	case 700:
		frameBlurStart(0x46, 0.8f);
		break;
	}
	if (mEvent->isDone() != 0) {
		mStep = 2;
	}
}

// 0x80192B1C
void TObjYellowGateSet::endEvent()
{
	seStop(mSound);
	if (mEvent != 0) {
		mEvent->mFlags |= 1;
		mEvent = 0;
	}
}

// 0x80192B6C
void TObjYellowGateSet::vfn_10()
{
	Vec3f screen;
	f32 radius = mModelData->model->r;
	if (cameraIsVisible(800.0f, radius, &mPos, &screen) != 0) {
		mDrawTask->mSortKey = drawTaskDepthKey(screen.z);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80192BEC
void TObjYellowGateSet::render()
{
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
	}
}

// 0x80192C28
int TObjYellowGateSet::getKind()
{
	return 3;
}

// 0x80192C30
int TObjYellowGateSet::getRotY()
{
	return mRot.y;
}

// 0x80192C38
Vec3f* TObjYellowGateSet::getPos()
{
	return &mPos;
}

// 0x80192C40
void TObjYellowGateSet::setPos(const Vec3f* pos)
{
	mPos = *pos;
}

// 0x80192C5C
SetLink* TObjYellowGateSet::getLink()
{
	return mLink.get();
}

// 0x80192C64
int TObjYellowGateSet::sendToHandle(ObjMsg* msg)
{
	switch (*(u16*)msg) {
	case 0xC7:
		return onCameraMessage(msg);
	default:
		return 0;
	}
}
