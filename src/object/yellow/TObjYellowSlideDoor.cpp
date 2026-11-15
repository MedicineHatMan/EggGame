#include "stage/SetManager.h"
struct DrawTask;
#include "stage/SetActor.h"
#include "object/ObjScript.h"
#include "object/ObjAction.h"
#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "camera/EventCamRef.h"
#include "camera/TCamera.h"
#include "enemy/EnemyLight.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "core/Vibration.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCamPath.h"
#include "core/MathUtil.h"
#include "core/Pad.h"
#include "object/TObjKey00.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "core/VecMath.h"
#include "stage/SetSlot.h"
#include "sound/SoundCtrl.h"
#include "object/yellow/TObjYellowSlideDoor.h"
struct MeshData;
struct SetLink;
struct MeshObject;
struct DrawState;

static inline Vec3f colliNeg(const Vec3f& v)
{
	Vec3f r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

extern "C" {
const f32 gYellowSlideDoorSetRange = 5000.0f;

// 0x802C4348
ObjClassName gTObjYellowSlideDoorName = { "TObjYellowSlideDoor", 0 };

// The event camera handle Its code ends just before this file
}

static inline f32 randSigned()
{
	return 2.0f * (3.0517578e-05f * (f32)rand() - 0.5f);
}

typedef char TObjYellowSlideDoor_size_check[(sizeof(TObjYellowSlideDoor) == 0x168) ? 1 : -1];

// 0x8012D4A0
extern "C" TObject* createTObjYellowSlideDoor(TObject* parent, SetDesc* desc)
{
	return new TObjYellowSlideDoor(parent, desc);
}

// 0x8012D4F4
TObjYellowSlideDoor::TObjYellowSlideDoor(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjYellowSlideDoorName.mName;
	mSize = 0x168;

	mModelData = *getGeObjModels();
	mTexList   = *getGeObjTextures();
	mColliData = (MeshData*)gGeObjCommon->mSet.mMeshes[0];
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	initColli();
	initFromDesc(desc);
	vfn_74();
}

// 0x8012D5E0
TObjYellowSlideDoor::~TObjYellowSlideDoor()
{
	freeColli();
	removeDrawTask(mDrawTask);
}

// 0x8012D670
// Int 0 is the switch int 1 turns the event camera on and int 2 says the floats hold the camera's eye
void TObjYellowSlideDoor::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mSwitchId  = desc->getInt(0);
	mUseCamera = desc->getInt(1);

	int hasOffset;
	if (desc->getInt(2) < 0) {
		hasOffset = 0;
	} else if (desc->getInt(2) > 1) {
		hasOffset = 1;
	} else {
		hasOffset = desc->getInt(2);
	}
	mHasOffset = hasOffset;

	if (mHasOffset != 0) {
		f32 z     = desc->getFloat(2);
		f32 y     = desc->getFloat(1);
		mCamEye.x = desc->getFloat(0);
		mCamEye.y = y;
		mCamEye.z = z;
		vecRotateY(&mCamEye, &mCamEye, mRot.y);
		vecAdd(&mCamEye, &mCamEye, &mPos);
	} else {
		mCamEye = sDoorCamOffset;
		vecRotateY(&mCamEye, &mCamEye, mRot.y);
		vecAdd(&mCamEye, &mCamEye, &mPos);
	}
}

// 0x8012D830
void TObjYellowSlideDoor::debugPrint(void*)
{
	njPrintC(0xA0013, ":SW ID");
	njPrint(0xA0014, ":EVENTCAMERA %s", (mUseCamera != 0) ? "ON" : "OFF");
	if (mUseCamera != 0) {
		njPrintC(0xA0015, ":SET CAM POS");
	}
	njPrintC(0xA0016, "");
	if (mUseCamera != 0 && mHasOffset != 0) {
		njPrintC(0xA0017, "OFFSET-X");
		njPrintC(0xA0018, "OFFSET-Y");
		njPrintC(0xA0019, "OFFSET-Z");
	}
	njPrintC(0xA001A, "");
}

// 0x8012D91C
void TObjYellowSlideDoor::vfn_74()
{
	mFixCam = 0;
	mModel.setModel(mModelData);
	mModel.mTexList = mTexList;
	mModel.mState   = &mState;
	if (keyBitIsSet(mSwitchId) == 1) {
		mLift = 80.0f;
		meshColliRefresh(mColli);
		mAct = 3;
	} else {
		mLift = 0.0f;
		mAct  = 0;
	}
	updateMatrix();
	updateColli();
	setLinkFree((SetLink*)&mLink);
	mLight.mTimer = 0;
}

// 0x8012D9C4
// A box that holds a sphere around center
void TObjYellowSlideDoor::setColliBox(void* colli, const Vec3f* center, f32 radius)
{
	Vec3f size;
	size.x = 1.0f;
	size.y = 1.0f;
	size.z = 1.0f;
	gjScaleVector(radius, &size, &size);

	Vec3f high;
	high.x = size.x;
	high.y = size.y;
	high.z = size.z;
	Vec3f low;
	Vec3f t = colliNeg(size);
	low.x   = t.x;
	low.y   = t.y;
	low.z   = t.z;

	gjPushUnitMatrix();
	njTranslateEx(center);
	njCalcPointEx(&high, &high);
	njCalcPointEx(&low, &low);
	njPopMatrixEx();

	f32 box[6];
	boxAroundLine(box, &low, &high);
	meshColliSetBox((MeshObject*)colli, (const f32*)box);
}

// 0x8012DAB0
void TObjYellowSlideDoor::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxTranslate((float*)mMatrix, (const float*)mMatrix, mLift, 0.0f, 0.0f);

	GjModel* model = mModelData->model;
	mCenter        = *(Vec3f*)&model->center;
	mtxMultVec(mMatrix, &mCenter, &mCenter);
	mRadius = mModelData->model->r;
}

// 0x8012DB4C
void TObjYellowSlideDoor::initColli()
{
	mColli = (MeshObject*)meshColliCreate();
	meshColliSetData(mColli, mColliData);
}

// 0x8012DB88
void TObjYellowSlideDoor::updateColli()
{
	setColliBox(mColli, &mCenter, 10.0f + mRadius);
	meshColliSetMatrix(mColli, mMatrix);
}

// 0x8012DBD4
void TObjYellowSlideDoor::freeColli()
{
	meshColliFree(mColli);
}

// 0x8012DBF8
void TObjYellowSlideDoor::startEventCamera()
{
	padVibStart(0, (int)this, 10, 0, 0x20);
}

// 0x8012DC2C
void TObjYellowSlideDoor::endEventCamera()
{
	vibStopOwner(0, this);
}

// 0x8012DC54
// Message 0xC7 hands the door an event camera and closes it
int TObjYellowSlideDoor::onCameraMessage(ObjMsg* msg)
{
	mCam.set(msg);
	mLift = 0.0f;
	updateMatrix();
	mAct = 0;
	return 1;
}

// 0x8012DCA0
void TObjYellowSlideDoor::draw()
{
	if (objDebugEnabled(this) != 0) {
		updateMatrix();
		updateColli();
		return;
	}
	if (!isInRange(this, &mPos, 5050.0f) && mCam.isActive() == 0) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateOpen();
		break;
	case 2:
		stateOpenAtEye();
		break;
	case 3:
		stateDone();
		break;
	}
}

// 0x8012DD78
void TObjYellowSlideDoor::stateWait()
{
	if (mUseCamera != 0) {
		if (mCam.isActive() != 0) {
			mFixCam = createTCamFix(0, 3, &mPos, &mCamEye);
			sePlay(0x263);
			startEventCamera();
			int act = 1;
			if (mHasOffset != 0) {
				act = 2;
			}
			mAct = act;
		}
		return;
	}
	if (mCam.isActive() != 0) {
		mCam.release();
	}
	if (keyBitIsSet(mSwitchId) != 0) {
		sePlay3D(0x263, &mPos);
		startEventCamera();
		mAct = 1;
	}
}

// 0x8012DE44
void TObjYellowSlideDoor::stateOpen()
{
	mLift += 0.5f;

	if (mUseCamera != 0 && mFixCam != 0) {
		f32 shake = randSigned();
		Vec3f jitter;
		jitter.set(1.5f * randSigned(), 0.0f, 1.5f * shake);

		mCamEye = sDoorCamOffset;
		vecScale(0.5f + 0.5f * (1.0f - mLift / 80.0f), &mCamEye, &mCamEye);
		vecRotateY(&mCamEye, &mCamEye, mRot.y);
		vecAdd(&mCamEye, &mCamEye, &mPos);
		vecAdd(&mCamEye, &mCamEye, &jitter);
		mFixCam->setEye(&mCamEye);

		Vec3f target;
		vecAdd(&target, &mPos, &jitter);
		mFixCam->setAt(&target);
	}

	if (80.0f < mLift) {
		if (mUseCamera != 0) {
			if (mFixCam != 0) {
				mFixCam->mFlags |= 1;
				mFixCam = 0;
			}
			mCam.release();
		}
		mLift = 80.0f;
		meshColliRefresh(mColli);
		endEventCamera();
		mAct = 3;
	}
	updateMatrix();
	updateColli();
}

// 0x8012E050
// The same with the camera held at the eye from the record
void TObjYellowSlideDoor::stateOpenAtEye()
{
	mLift += 0.5f;

	if (mFixCam != 0) {
		Vec3f pos;
		Vec3f target;
		Vec3f jitter;
		f32 shake = randSigned();
		jitter.set(1.5f * randSigned(), 0.0f, 1.5f * shake);

		vecAdd(&pos, &mCamEye, &jitter);
		vecAdd(&target, &mPos, &jitter);
		mFixCam->setEye(&mCamEye);
		mFixCam->setAt(&target);
	}

	if (80.0f < mLift) {
		if (mFixCam != 0) {
			mFixCam->mFlags |= 1;
			mFixCam = 0;
		}
		mCam.release();
		mLift = 80.0f;
		meshColliRefresh(mColli);
		endEventCamera();
		mAct = 3;
	}
	updateMatrix();
	updateColli();
}

// 0x8012E1E0
void TObjYellowSlideDoor::stateDone()
{
}

// 0x8012E1E4
void TObjYellowSlideDoor::vfn_10()
{
	if (mCam.isActive() != 0) {
		mState.mFlags = 0;
		moveDrawTask(mDrawTask, 33);
		mDrawTask->mFlags &= ~1;
	} else if (mLight.isVisible(mDrawTask, 1000.0f, &mCenter, mRadius, 0) != 0) {
		setDrawAmbient(&mLight, &mState);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8012E298
void TObjYellowSlideDoor::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
	if (objDebugEnabled(this) != 0) {
		meshDebugDraw((MeshColli*)mColli, (MeshData*)mColliData);
	}
}

// 0x8012E2F8
int TObjYellowSlideDoor::sendToHandle(ObjMsg* msg)
{
	switch (*(u16*)msg) {
	case 0xC7:
		return onCameraMessage(msg);
	default:
		return 0;
	}
}

// 0x802828A0
// set by the static initializer at 0x8012E330
InitVec sDoorCamOffset(0.0f, 400.0f, 200.0f);
