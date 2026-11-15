#include "stage/SetManager.h"
struct DrawTask;
#include "stage/SetActor.h"
#include "camera/TCamera.h"
#include "camera/EventCamArc.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "stage/TEventCtrl.h"
#include "object/TObjLauncher.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "object/yellow/TObjYellowFogTrapStatue.h"
struct SetLink;
struct ColliShape;
struct CharaColli;
struct Fx;

const char* gTObjYellowFogTrapStatueName = "TObjYellowFogTrapStatue";

extern "C" {
extern const MotionEntry sYellowFogTrapStatueMotions[] = {
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 10.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 10.0f, 20.0f, 0.3f, 0.0f, 0 },
};
}

#define sInstance sFogTrapStatueObj

/** The offset of the camera's start from the statue */
struct FogTrapCamOffset : public Vec3f {
	FogTrapCamOffset(f32 ox, f32 oy, f32 oz)
	{
		x = ox;
		y = oy;
		z = oz;
	}
};

static FogTrapCamOffset sCamOffset(-120.0f, -10.0f, 0.0f);

typedef char TObjYellowFogTrapStatue_size_check[(sizeof(TObjYellowFogTrapStatue) == 0x160) ? 1 : -1];

const f32 gYellowFogTrapStatueSetRange = 1000.0f;

// 0x80179EE0
extern "C" TObject* createTObjYellowFogTrapStatue(TObject* parent, SetDesc* desc)
{
	if (sInstance == 0) {
		sInstance = new TObjYellowFogTrapStatue(parent, desc);
		return (TObject*)sInstance;
	}
	return 0;
}

// 0x80179F4C
extern "C" int triggerFogTrapStatue()
{
	TObjYellowFogTrapStatue* statue = (TObjYellowFogTrapStatue*)sInstance;
	if (statue == 0) {
		return 0;
	}
	if (statue->mFirstTime != 0) {
		return statue->startEvent();
	}
	return statue->startCamera();
}

// 0x80179F94
TObjYellowFogTrapStatue::TObjYellowFogTrapStatue(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjYellowFogTrapStatueName;
	mSize = 0x160;

	mModelData = getGeObjModels()[7];
	mTexList   = getGeObjTextures()[7];
	mMotion.init((const MotionEntry*)sYellowFogTrapStatueMotions, gGeObjCommon->mSet.mMotions);
	mMotion.play(0);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);

	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 255, 246, 246);

	initFromDesc(desc);
	vfn_74();
}

// 0x8017A0AC
TObjYellowFogTrapStatue::~TObjYellowFogTrapStatue()
{
	stopBreath();
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
	if (this == sInstance) {
		sInstance = 0;
	}
}

// 0x8017A148
void TObjYellowFogTrapStatue::initFromDesc(SetDesc* desc)
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
}

// 0x8017A1E4
// The first lines give where the model's centre should go for the record's position to be right
void TObjYellowFogTrapStatue::debugPrint(void*)
{
	Vec3f pos;
	Vec3f center;
	const Vec3f* c = (const Vec3f*)&mModelData->model->center;
	center.set(c->x, c->y, c->z);
	vecSub(&pos, &center, &mPos);
	njPrint(0xA0005, ":%f", pos.x);
	njPrint(0xA0006, ":%f", pos.y);
	njPrint(0xA0007, ":%f", pos.z);
	njPrintC(0xA0013, ":OFF SW ID");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x8017A300
void TObjYellowFogTrapStatue::vfn_74()
{
	mBreath    = 0;
	mFrame     = 0;
	mStep      = 0;
	mFirstTime = 1;
	mEvent     = 0;
	mFullEvent = 0;
	mCamera    = 0;
	mModel.setModelNodes(mModelData, 5);
	mModel.mTexList = mTexList;
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mModel.mMotion  = &mMotion;
	mModel.calcMotion();
	updatePlace();
	if (keyBitIsSet(mSwitchId) == 1) {
		mAct = 3;
	} else {
		mAct = 0;
	}
	setLinkFree((SetLink*)&mLink);
}

// 0x8017A3B0
// The record's position is where the model's centre goes so that is subtracted first
void TObjYellowFogTrapStatue::updatePlace()
{
	Vec3f base;
	Vec3f breath;
	Vec3f center;
	NJS_OBJECT* object = mModelData;
	center.set(object->model->center.x, object->model->center.y, object->model->center.z);
	vecSub(&base, &mPos, &center);
	mtxSetTransV(mMatrix, &base);

	// The breath comes out of the mouth a node two levels down
	object            = mModelData;
	NJS_OBJECT* mouth = object->child->sibling;
	breath.x          = mouth->pos[0] - object->model->center.x;
	breath.y          = mouth->pos[1] - object->model->center.y;
	breath.z          = mouth->pos[2] - object->model->center.z;
	vecAdd(&breath, &mPos, &breath);
	mtxSetTransV(mBreathMtx, &breath);
	mtxRotateY(mBreathMtx, mBreathMtx, 0x4000);

	GjModel* body = mModelData->model;
	mCenter       = *(Vec3f*)&body->center;
	mtxMultVec(mMatrix, &mCenter, &mCenter);
	mRadius = mModelData->model->r;
	colliShapeMakeSphere(colliGetShape(mShape, 0), &mCenter, 0, mRadius);
}

// 0x8017A4DC
int TObjYellowFogTrapStatue::startEvent()
{
	if (mAct != 0) {
		return 0;
	}
	if (eventStartById(mObjId, 0) == 0) {
		return 0;
	}
	mEvent = mFullEvent = eventCamArcPlay(1.0f, 0.0f, 0x100);
	if (mEvent == 0) {
		eventEnd();
		return 0;
	}
	((TCamera*)mEvent)->setCut();
	mFrame     = 0;
	mStep      = 0;
	mAct       = 1;
	mFirstTime = 0;
	return 1;
}

// 0x8017A584
int TObjYellowFogTrapStatue::startCamera()
{
	if (mAct != 0) {
		return 0;
	}
	if (eventStartById(mObjId, 0) == 0) {
		return 0;
	}
	Vec3f target;
	target = mCenter;
	target.y += 50.0f;

	Vec3f from;
	vecAdd(&from, &mCenter, &sCamOffset);
	from.z += 50.0f;

	mEvent = mCamera = createTCamFix(camGetDrawIndex(), 3, &target, &from);
	if (mEvent == 0) {
		eventEnd();
		return 0;
	}
	((TCamera*)mEvent)->setCut();
	mFrame = 0;
	mStep  = 0;
	mAct   = 2;
	return 1;
}

// 0x8017A674
void TObjYellowFogTrapStatue::startBreath()
{
	if (mBreath == 0) {
		mBreath = fxSpawnFollow(1, 8, 0x10, mBreathMtx);
		Vec3f pos;
		mtxGetPos(mBreathMtx, &pos);
		sePlay3D(0x25D, &pos);
	}
}

// 0x8017A6D8
void TObjYellowFogTrapStatue::stopBreath()
{
	fxStop(mBreath);
	mBreath = 0;
}

// 0x8017A710
void TObjYellowFogTrapStatue::draw()
{
	if (objDebugEnabled(this) != 0) {
		mMotion.update();
		mModel.calcMotion();
		updatePlace();
		return;
	}
	if (!isInRange(this, &mPos, 1100.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		statePlay();
		break;
	case 2:
		statePlay();
		break;
	case 3:
		stateOff();
		break;
	}
}

// 0x8017A7E0
void TObjYellowFogTrapStatue::stateWait()
{
	if (keyBitIsSet(mSwitchId) == 1) {
		mAct = 3;
	}
}

// 0x8017A820
void TObjYellowFogTrapStatue::statePlay()
{
	switch (mStep) {
	case 0:
		mMotion.play(0);
		mStep = 1;
		break;
	case 1:
		if (mMotion.isDone()) {
			startBreath();
			mStep = 2;
		}
		mMotion.update();
		break;
	case 2:
		if (mFrame > 240) {
			stopBreath();
			mMotion.play(1);
			mMotion.update();
			mStep = 3;
		}
		break;
	case 3:
		if (mMotion.isDone()) {
			if (mFullEvent != 0 && mFullEvent->isDone() == 0) {
				break;
			}
			mStep = 4;
		}
		mMotion.update();
		break;
	case 4:
		if (mEvent != 0) {
			mEvent->mFlags |= 1;
			mEvent = 0;
		}
		mFullEvent = 0;
		mCamera    = 0;
		eventEnd();
		mAct = 0;
		break;
	}
	mFrame++;
}

// 0x8017A96C
void TObjYellowFogTrapStatue::stateOff()
{
}

// 0x8017A970
void TObjYellowFogTrapStatue::vfn_10()
{
	if (cameraIsVisible(1000.0f, mRadius, &mCenter, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8017A9D8
// Debug drawing marks where the breath comes out
void TObjYellowFogTrapStatue::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
	if (objDebugEnabled(this) != 0) {
		f32 mtx[12];
		mtxConcat(mtx, gjGetMatrix(), mBreathMtx);
		mtxScale(2.0f, 2.0f, 2.0f, mtx, mtx);
		u32 color = 0xFFFF00FF;
		debugDrawSphere(mtx, &color);
	}
}
