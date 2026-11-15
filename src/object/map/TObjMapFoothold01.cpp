#include "stage/SetManager.h"
struct DrawTask;
#include "stage/SetManager.h"
#include "stage/SetManager.h"
#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "object/ObjScript.h"
#include "object/ObjModel.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "camera/TCamera.h"
#include "engine/Colli.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCamPath.h"
#include "stage/SetSlot.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "object/map/TObjMapFoothold01.h"
struct SetLink;
struct CharaColli;
struct NJS_QUATERNION;
struct MeshColli;
struct MeshData;
struct MeshObject;

extern "C" {
const f32 gMapFoothold01SetRange = 800.0f;

// 0x802C3D08
ObjClassName gTObjMapFoothold01Name = { "TObjMapFoothold01", 0 };
}

enum FootholdType {
	FOOTHOLD_STOP,
	FOOTHOLD_ROT_X,      // turns on X then waits
	FOOTHOLD_ROT_Y,      // turns on Y without stopping
	FOOTHOLD_UP_DOWN,    // a sine wave on Y
	FOOTHOLD_LEFT_RIGHT, // a sine wave on X
	FOOTHOLD_ROT_Y2,     // turns on Y around a point away from its centre
	FOOTHOLD_IN_OUT,     // fades in waits and fades out again
	FOOTHOLD_FALL,       // falls when a player stands on it and comes back from -1000
};

typedef char TObjMapFoothold01_size_check[(sizeof(TObjMapFoothold01) == 0x188) ? 1 : -1];

// 0x80051044
extern "C" TObject* createTObjMapFoothold01(TObject* parent, SetDesc* desc)
{
	return new TObjMapFoothold01(parent, desc);
}

// 0x80051098
void TObjMapFoothold01::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mWait   = 0;
	mRange  = 0.0f;
	mOffset = 0.0f;

	mType    = desc->getInt(0);
	int type = mType;
	if (type < 0) {
		type = 0;
	} else if (type > 7) {
		type = 7;
	}
	mType = type;

	mSpeed = desc->getInt(2) + 0x100;
	if (mType == FOOTHOLD_IN_OUT || mType == FOOTHOLD_FALL || mType == FOOTHOLD_ROT_X) {
		mWait = desc->getInt(3) + 60;
	}
	if (mType == FOOTHOLD_UP_DOWN || mType == FOOTHOLD_LEFT_RIGHT) {
		mRange = 10.0f + desc->getFloat(0);
	} else if (mType == FOOTHOLD_ROT_Y2) {
		mOffset = 10.0f + desc->getFloat(0);
	}
	placeOffCentre();
}

// 0x800511E8
void TObjMapFoothold01::debugPrint(void*)
{
	switch (mType) {
	case FOOTHOLD_STOP:
		njPrintC(0xA0013, ":STOP");
		break;
	case FOOTHOLD_ROT_X:
		njPrintC(0xA0013, ":ROT X");
		njPrintC(0xA0016, "+60:TIME LIMIT");
		break;
	case FOOTHOLD_ROT_Y:
		njPrintC(0xA0013, ":ROT Y");
		break;
	case FOOTHOLD_UP_DOWN:
		njPrintC(0xA0013, ":UP DOWN");
		njPrintC(0xA0017, "+10.0:MOVE RANGE");
		break;
	case FOOTHOLD_LEFT_RIGHT:
		njPrintC(0xA0013, ":LEFT RIGHT");
		njPrintC(0xA0017, "+10.0:MOVE RANGE");
		break;
	case FOOTHOLD_ROT_Y2:
		njPrintC(0xA0013, ":ROT Y2");
		njPrintC(0xA0017, "+10.0:RADIUS");
		break;
	case FOOTHOLD_IN_OUT:
		njPrintC(0xA0013, ":IN OUT");
		njPrintC(0xA0016, "+60:TIME LIMIT");
		break;
	case FOOTHOLD_FALL:
		njPrintC(0xA0013, ":FALL");
		njPrintC(0xA0016, "+60:TIME LIMIT");
		break;
	}
	if (mType != FOOTHOLD_STOP) {
		njPrintC(0xA0015, "+0x100:SPEED");
	}
}

// 0x80051354
void TObjMapFoothold01::setup()
{
	mState = *(DrawState*)getDefaultDrawState();
	if (mType == FOOTHOLD_IN_OUT) {
		mState.mFlags |= DRAWSTATE_AMBIENT;
	}

	mPlace.mFlags = 3;
	mPlace.mPos   = mPos;
	njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
	mPlace.update();

	mModel.setModel(findGeObjData("MODEL_STAND"));
	mModel.mPlace   = &mPlace;
	mModel.mTexList = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_STAND");
	mModel.mState   = &mState;

	mStep    = 0;
	mPhase   = 0;
	mVel.x   = 0.0f;
	mVel.y   = 0.0f;
	mVel.z   = 0.0f;
	mTimer   = 0;
	mVisible = 1;

	MeshData* data = (MeshData*)gStgObjCommon->mSet.mMeshes[1];
	mColli         = (MeshObject*)meshColliCreate();
	meshColliSetData(mColli, data);
	meshColliSetMatrix(mColli, mPlace.mMatrix);
	updateColliBox();

	if (mType != FOOTHOLD_STOP && mType != FOOTHOLD_IN_OUT) {
		meshSetFlag2(mColli, 1);
	}
	if (mType == FOOTHOLD_ROT_X) {
		meshSetState100(mColli, 1);
	}
	if (mType == FOOTHOLD_FALL) {
		mShape = colliCreate(1, 1);
		colliSetOwner(mShape, (u16)mObjId);
		colliSetMasks(mShape, 4, 0xFF, 0xFF, 0xFE);
		updateTouchShape();
	}
}

// 0x80051528
void TObjMapFoothold01::updateColliBox()
{
	Vec3f low;
	low.x = -20.0f;
	low.y = -25.0f;
	low.z = -20.0f;
	Vec3f high;
	high.x = 20.0f;
	high.y = 10.0f;
	high.z = 20.0f;
	f32 box[6];
	boxAroundBox(box, mPlace.mMatrix, &low, &high);
	meshColliSetBox(mColli, box);
}

// 0x80051598
// ROT Y2 orbits its home at mOffset on the rotation's X axis
void TObjMapFoothold01::placeOffCentre()
{
	Vec3f origin;
	origin.x = 0.0f;
	origin.y = 0.0f;
	origin.z = 0.0f;
	gjPushUnitMatrix();
	njTranslateEx(&mHome);
	gjRotateY(mRot.y);
	gjRotateX(mRot.x);
	gjRotateZ(mRot.z);
	gjTranslate(mOffset, 0.0f, 0.0f);
	njCalcPointEx(&origin, &mPos);
	njPopMatrixEx();
}

// 0x80051614
TObjMapFoothold01::TObjMapFoothold01(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjMapFoothold01Name.mName;
	mSize = 0x188;
	initFromDesc(desc);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	setup();
}

// 0x800516C4
TObjMapFoothold01::~TObjMapFoothold01()
{
	if (mShape != 0) {
		colliRetire(mShape);
		mShape = 0;
	}
	meshColliFree(mColli);
	removeDrawTask(mDrawTask);
}

// 0x80051764
void TObjMapFoothold01::stepWaiting()
{
	if (mVisible != 0) {
		waitThen(3);
	} else {
		waitThen(2);
	}
}

// 0x800517A0
void TObjMapFoothold01::stepFadeIn()
{
	mVisible = 1;
	meshClearState400(mColli);
	u8 alpha = mState.mAmbient;
	if (alpha < 0xFF) {
		u32 next = alpha + 4;
		if ((u16)next > 0xFF) {
			next = 0xFF;
		}
		mState.mAmbient = (u8)next;
		mModel.mState   = &mState;
	} else {
		moveDrawTask(mDrawTask, 0x21);
		mStep = 0;
	}
}

// 0x80051824
void TObjMapFoothold01::stepFadeOut()
{
	moveDrawTask(mDrawTask, 0x22);
	int alpha = (u8)mState.mAmbient;
	if (alpha > 0) {
		alpha -= 4;
		if ((s16)alpha < 0) {
			alpha = 0;
		}
		mState.mAmbient = (u8)alpha;
		mModel.mState   = &mState;
	} else {
		meshColliRefresh(mColli);
		mVisible = 0;
		mStep    = 0;
	}
}

// 0x8005189C
void TObjMapFoothold01::stepWaitTouch()
{
	if (isTouched() != 0) {
		mStep = 4;
	}
}

// 0x800518D8
void TObjMapFoothold01::stepFallDelay()
{
	waitThen(5);
}

// 0x800518FC
void TObjMapFoothold01::stepFall()
{
	if (mPos.y < -1000.0f) {
		mPos   = mHome;
		mVel.x = 0.0f;
		mVel.y = 0.0f;
		mVel.z = 0.0f;
		mStep  = 0;
	}
	mVel.y += -0.08166667f;
	njAddVector(&mPos, &mVel);
}

// 0x80051978
void TObjMapFoothold01::waitThen(int step)
{
	if (mTimer++ > mWait) {
		mTimer = 0;
		mStep  = step;
	}
}

// 0x800519A0
// Something from group 1 a player is on it
int TObjMapFoothold01::isTouched()
{
	for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
		if (colliInGroup(hit->mOther, 1) != 0) {
			return 1;
		}
	}
	return 0;
}

// 0x800519FC
void TObjMapFoothold01::updateTouchShape()
{
	ColliShape shape;
	colliShapeInit(&shape);
	shape.mType  = 1;
	shape.mSizeX = 20.0f;
	shape.mSizeY = 7.5f;
	shape.mSizeZ = 20.0f;
	shape.mPos.y = 15.0f;
	shape.mFlags |= 0x10;
	colliCopyShapes(mShape, &shape);
	colliPlaceShapesRotY(mShape, &mPos, mRot.y, &shape);
	colliRefreshBounds(mShape);
}

// 0x80051A84
void TObjMapFoothold01::draw()
{
	if (!isInRange(this, &mPos, 850.0f)) {
		setLinkFree((SetLink*)&mLink);
		mFlags |= 1;
		return;
	}
	if (mType != FOOTHOLD_ROT_X) {
		mPhase += mSpeed;
	}

	int moved = 0;
	switch (mType) {
	case FOOTHOLD_ROT_Y:
		mRot.y = mPhase;
		moved  = 1;
		break;
	case FOOTHOLD_ROT_X:
		switch (mStep) {
		case 0:
			mPhase += mSpeed;
			if (mPhase > 0xFFFF) {
				mPhase = 0;
				mStep  = 1;
			}
			mRot.x = mPhase;
			moved  = 1;
			break;
		case 1:
			waitThen(0);
			break;
		}
		break;
	case FOOTHOLD_UP_DOWN:
		mOffset = mRange * njSin(mPhase);
		mPos.y  = mHome.y + mOffset;
		moved   = 1;
		break;
	case FOOTHOLD_LEFT_RIGHT:
		mOffset = mRange * njSin(mPhase);
		mPos.x  = mHome.x + mOffset;
		moved   = 1;
		break;
	case FOOTHOLD_ROT_Y2:
		mRot.y = mPhase;
		placeOffCentre();
		moved = 1;
		break;
	case FOOTHOLD_IN_OUT:
		switch (mStep) {
		case 0:
			stepWaiting();
			break;
		case 2:
			stepFadeIn();
			break;
		case 3:
			stepFadeOut();
			break;
		}
		break;
	case FOOTHOLD_FALL:
		switch (mStep) {
		case 0:
			stepWaitTouch();
			break;
		case 4:
			stepFallDelay();
			break;
		case 5:
			stepFall();
			moved = 1;
			break;
		}
		updateTouchShape();
		break;
	}

	if (moved != 0) {
		mPlace.mPos = mPos;
		njXYZAngleToQuaternion((const s32*)&mRot, (NJS_QUATERNION*)mPlace.mRot);
		mPlace.update();
		meshColliSetMatrix(mColli, mPlace.mMatrix);
		updateColliBox();
	}
}

// 0x80051CF4
void TObjMapFoothold01::vfn_10()
{
	if (mLight.isVisible(mDrawTask, 700.0f, &mPos, 40.0f, 0) != 0) {
		setDrawAmbient(&mLight, &mState);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80051D70
void TObjMapFoothold01::render()
{
	if (mVisible != 0) {
		mModel.draw();
	}
}

// 0x80051DA0
int TObjMapFoothold01::getKind()
{
	return 3;
}

// 0x80051DA8
SetLink* TObjMapFoothold01::getLink()
{
	return mLink.get();
}
