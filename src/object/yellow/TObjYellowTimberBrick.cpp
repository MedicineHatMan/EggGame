#include "stage/SetManager.h"
struct DrawTask;
#include "object/yellow/TObjYellowTimberBrick.h"
#include "engine/DrawTask.h"
#include "camera/EventCamArc.h"
#include "camera/EventCamRef.h"
#include "object/ObjScript.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "camera/TCamera.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "core/Vibration.h"
#include "object/yellow/TObjYellowSandfall.h"
#include "object/TObjLauncher.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "core/Pad.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "stage/TStageSelectModelE3.h"
#include "stage/SetActor.h"
#include "engine/ScrollModel.h"
#include "object/ObjAction.h"
#include "engine/CharaColli.h"
#include "core/PlacementNew.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "object/yellow/TObjYellowSandFloor.h"
struct NJS_OBJECT;
struct CharaColli;
struct SetLink;
struct MeshColli;
struct MeshData;
struct MeshObject;
struct CamView;

extern "C" {
const f32 gYellowTimberBrickSetRange = 2000.0f;
const f32 gYellowSandFloorSetRange = 2000.0f;

// 0x802C44D8
const char* gTObjYellowTimberBrickName = "TObjYellowTimberBrick";
const char* gTObjYellowSandFloorName = "TObjYellowSandFloor";

}

#define sBrick gTimberBrickObj
#define sFloor gSandFloorObj

// The two points camGetDrawView returns
struct ViewPair {
	Vec3f mPos; // _00
	Vec3f mAt;  // _0C
};

// Message 0xC7 mLevel is 0 for the first rise and 1 for the second
struct RiseMsg {
	u16 mType;  // _00
	u16 _02;    // _02
	u32 _04;    // _04
	int mLevel; // _08
};

typedef char TObjYellowTimberBrick_size_check[(sizeof(TObjYellowTimberBrick) == 0x130) ? 1 : -1];

// 0x80154C8C
// The yellow set's init
extern "C" void timberBrickSetInit()
{
	sBrick = 0;
}

// 0x80154C98
// The yellow set's exit
extern "C" void timberBrickSetExit()
{
}

// 0x80154C9C
extern "C" TObject* createTObjYellowTimberBrick(TObject* parent, SetDesc* desc)
{
	if (sBrick == 0) {
		sBrick = (u8*)new TObjYellowTimberBrick(parent, desc);
		return (TObjYellowTimberBrick*)sBrick;
	}
	return 0;
}

// 0x80154D08
TObjYellowTimberBrick::TObjYellowTimberBrick(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjYellowTimberBrickName;
	mSize = 0x130;

	mModelData = getGeObjModels()[3];
	mTexList   = getGeObjTextures()[3];
	mColliData = gGeObjCommon->mSet.mMeshes[1];
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	mColli     = (MeshObject*)meshColliCreate();
	meshColliSetData((MeshObject*)mColli, (MeshData*)mColliData);
	meshSetFlag2((MeshObject*)mColli, 1);

	initFromDesc(desc);
	vfn_74();
}

// 0x80154E08
TObjYellowTimberBrick::~TObjYellowTimberBrick()
{
	seStop3D(&mSound);
	meshColliFree(mColli);
	removeDrawTask(mDrawTask);
	if ((u8*)this == sBrick) {
		sBrick = 0;
	}
}

// 0x80154EAC
void TObjYellowTimberBrick::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	updatePlace();
	mSwitch1 = desc->getInt(0) < 0 ? 0 : (desc->getInt(0) > 0x800 ? 0x800 : desc->getInt(0));
	mSwitch2 = desc->getInt(1) < 0 ? 0 : (desc->getInt(1) > 0x800 ? 0x800 : desc->getInt(1));
	mEventId = desc->getInt(2);
}

// 0x80154FD8
void TObjYellowTimberBrick::debugPrint(void*)
{
	njPrint(0xA0005, ":%f", mModelData->pos[0]);
	njPrint(0xA0006, ":%f", mModelData->pos[1]);
	njPrint(0xA0007, ":%f", mModelData->pos[2]);
	njPrintC(0xA0013, ":SW_ID LV1");
	njPrintC(0xA0014, ":SW_ID LV2");
	njPrintC(0xA0015, ":CALL EVENT ID");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x801550DC
int TObjYellowTimberBrick::sendToHandle(ObjMsg* msg)
{
	RiseMsg* rise = (RiseMsg*)msg;
	if (rise->mType == 0xC7) {
		mCam.set(msg);
		mRequest = rise->mLevel;
		return 1;
	}
	return 0;
}

// 0x80155130
void TObjYellowTimberBrick::vfn_74()
{
	mRequest = -1;
	mEvent   = 0;
	mTimer   = 0;
	mModel.setModel(mModelData);
	mModel.mTexList = mTexList;
	mModel.mState   = (DrawState*)getDefaultDrawState();

	if (keyBitIsSet(mSwitch1) == 1) {
		if (keyBitIsSet(mSwitch2) == 1) {
			mLevel  = 2;
			mHeight = 200.0f;
		} else {
			mLevel  = 1;
			mHeight = 100.0f;
		}
	} else {
		mLevel  = 0;
		mHeight = 0.0f;
	}
	updatePlace();
	setLinkFree((SetLink*)&mLink);
	mAct = 0;
}

// 0x801551F8
void TObjYellowTimberBrick::updatePlace()
{
	Vec3f centre;
	centre.set(mModelData->model->center.x, mModelData->model->center.y, mModelData->model->center.z);
	vecAdd(&mCenter, &mPos, &centre);
	mCenter.y += mHeight;
	mRadius = mModelData->model->r;

	mtxSetTrans((float*)mMatrix, mPos.x, mPos.y + mHeight, mPos.z);
	updateColli();

	NJS_OBJECT* obj = mModelData;
	mtxTranslate((float*)mMatrix, (const float*)mMatrix, -obj->pos[0], -obj->pos[1], -obj->pos[2]);
}

// 0x801552BC
void TObjYellowTimberBrick::updateColli()
{
	f32 size = 10.0f + mRadius;
	Vec3f low;
	Vec3f high;
	high.x = size;
	high.y = size;
	low.x  = -size;
	low.y  = -size;
	low.z  = -size;
	high.z = size;
	mtxMultVec(mMatrix, &low, &low);
	mtxMultVec(mMatrix, &high, &high);

	f32 box[6];
	boxAroundLine(box, &low, &high);
	meshColliSetBox(mColli, (const f32*)box);
	meshColliSetMatrix((MeshColli*)mColli, (const f32*)mMatrix);
}

// 0x80155354
void TObjYellowTimberBrick::startEvent()
{
	if (mEvent != 0) {
		mEvent->mFlags |= 1;
		mEvent = 0;
	}
	mEvent = eventCamArcPlay(1.0f, 0.0f, 0x101);
}

// 0x801553B0
void TObjYellowTimberBrick::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 2100.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateEvent();
		break;
	case 2:
		stateRise();
		break;
	}
}

// 0x80155454
void TObjYellowTimberBrick::stateWait()
{
	if (mCam.isActive() == 0) {
		return;
	}
	int rise = 0;
	if (mLevel == 0 && mRequest == 0) {
		rise    = 1;
		mTarget = 100.0f;
	} else if (mLevel == 1 && mRequest == 1) {
		rise    = 1;
		mTarget = 200.0f;
	}
	if (rise != 0) {
		pauseEventSandfalls();
		startEventSandfalls();
		startEvent();
		padVibStart(0, (int)this, 10, 0, 0x1E);
		mTimer = 0xB4;
		mAct   = 1;
	} else {
		mCam.release();
	}
}

// 0x80155524
void TObjYellowTimberBrick::stateEvent()
{
	if (mEvent == 0 || mEvent->isDone() != 0) {
		if (mEvent != 0) {
			mEvent->mFlags |= 1;
			mEvent = 0;
		}
		mCam.release();
		if (sFloor != 0) {
			((TObject*)sFloor)->mFlags &= ~8;
		}
		Vec3f pos;
		mtxGetPos(mMatrix, &pos);
		sePlay3DHeld(1000.0f, &mSound, 0x25F, 100, &pos);
		mSound.mFar = 0x7F;
	}
	if (mEvent != 0) {
		return;
	}
	if (mTimer == 0) {
		mAct = 2;
	} else {
		mTimer--;
	}
}

// 0x801555FC
void TObjYellowTimberBrick::stateRise()
{
	mHeight += 0.15f;
	if (mHeight >= mTarget) {
		mHeight = mTarget;
		updatePlace();
		TObject* floor = (TObject*)sFloor;
		if (floor != 0) {
			floor->draw();
			floor->mFlags |= 8;
		}
		stopEventSandfalls();
		mLevel++;
		seStop3D(&mSound);
		vibStopOwner(0, this);
		mAct = 0;
		return;
	}

	ViewPair* view = (ViewPair*)camGetDrawView();
	if (view != 0) {
		Vec3f shake;
		f32 z = 2.0f * (3.0517578e-5f * (f32)rand() - 0.5f);
		f32 y = 2.0f * (3.0517578e-5f * (f32)rand() - 0.5f);
		f32 x = 2.0f * (3.0517578e-5f * (f32)rand() - 0.5f);
		shake.set(0.5f * x, 0.5f * y, 0.5f * z);
		vecAdd(&view->mPos, &view->mPos, &shake);
		vecAdd(&view->mAt, &view->mAt, &shake);
	}
	updatePlace();
	Vec3f pos;
	mtxGetPos(mMatrix, &pos);
	mSound.mPos = pos;
}

// 0x801557D8
void TObjYellowTimberBrick::vfn_10()
{
	if (cameraIsVisible(2000.0f, mRadius, &mCenter, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80155840
void TObjYellowTimberBrick::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
}

/** Half the size of the sand's collision box */
struct SandFloorBox {
	SandFloorBox(f32 sx, f32 sy, f32 sz)
	{
		x = sx;
		y = sy;
		z = sz;
	}

	f32 x, y, z;
};

static SandFloorBox sColliBox(500.0f, 100.0f, 580.0f);

struct FloorMsg {
	u16 mId;
};

typedef char TObjYellowSandFloor_size_check[(sizeof(TObjYellowSandFloor) == 0x12C) ? 1 : -1];

// 0x80155884
extern "C" TObject* createTObjYellowSandFloor(TObject* parent, SetDesc* desc)
{
	if (sFloor == 0 && sBrick != 0) {
		sFloor = new TObjYellowSandFloor(parent, desc);
		return (TObject*)sFloor;
	}
	return 0;
}

// 0x801558FC
TObjYellowSandFloor::TObjYellowSandFloor(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjYellowSandFloorName;
	mSize = 0x12C;

	mModelData  = getGeObjModels()[8];
	mTexList    = getGeObjTextures()[8];
	mColliData  = gGeObjCommon->mSet.mMeshes[3];
	mTopModel   = getGeObjModels()[9];
	mTopTexList = getGeObjTextures()[9];

	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 255, 255, 254);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);

	mColli = (MeshColli*)meshColliCreate();
	meshColliSetData((MeshObject*)mColli, (MeshData*)mColliData);
	meshSetFlag2((MeshObject*)mColli, 1);
	mSurface.reset();

	initFromDesc(desc);
	vfn_74();
}

// 0x80155A40
TObjYellowSandFloor::~TObjYellowSandFloor()
{
	meshColliFree(mColli);
	removeDrawTask(mDrawTask);
	colliRetire(mShape);
	mShape = 0;
	if (this == sFloor) {
		sFloor = 0;
	}
}

// 0x80155B04
void TObjYellowSandFloor::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	colliShapeMakeBox(colliGetShapes(mShape), &mPos, 0, sColliBox.x, sColliBox.y, sColliBox.z, 0);
	((ColliShape*)colliGetShapes(mShape))->mPos.y -= sColliBox.y;
	updatePlace();
	mSurface.readDesc(desc);
	mSurface.bindModel(*(void**)((u8*)mModelData + 4), &mTrans);
}

// 0x80155BA0
void TObjYellowSandFloor::debugPrint(void*)
{
	mSurface.debugPrint();
	mSurface.debugPrintTrans(&mTrans);
}

// 0x80155BDC
int TObjYellowSandFloor::sendToHandle(ObjMsg*)
{
	return 0;
}

// 0x80155BE4
void TObjYellowSandFloor::vfn_74()
{
	setLinkFree((SetLink*)&mLink);
	mAct = 0;
}

// 0x80155C1C
void TObjYellowSandFloor::updatePlace()
{
	mCenter = mPos;
	GjModel* model = mModelData->model;
	mRadius  = model->r;

	u8* brick = (u8*)sBrick;
	if (brick != 0) {
		mCenter.y = *(f32*)(brick + 0x40) + *(f32*)(brick + 0xCC);
		mtxSetTrans((float*)mMatrix, mPos.x, *(f32*)(brick + 0x40) + *(f32*)(brick + 0xCC), mPos.z);
	} else {
		mtxSetTransV(mMatrix, &mPos);
	}

	model = mModelData->model;
	Vec3f offset;
	offset.set(((Vec3f*)&model->center)->x, ((Vec3f*)&model->center)->y, ((Vec3f*)&model->center)->z);
	gjScaleVector(-1.0f, &offset, &offset);
	updateColliBox();
	mtxTranslateV(mMatrix, mMatrix, &offset);
	meshColliSetMatrix(mColli, (const f32*)mMatrix);

	f32 y = mCenter.y - sColliBox.y;
	((ColliShape*)colliGetShapes(mShape))->mPos.y = y;
	colliRefreshBounds(mShape);
}

// 0x80155D40
void TObjYellowSandFloor::updateColliBox()
{
	Vec3f low;
	Vec3f high;
	f32 size = 10.0f + mRadius;
	high.x = size;
	low.x  = -size;
	high.y = size;
	low.y  = -size;
	low.z  = -size;
	high.z = size;
	mtxMultVec(mMatrix, &low, &low);
	mtxMultVec(mMatrix, &high, &high);

	f32 box[6];
	boxAroundLine(box, &low, &high);
	meshColliSetBox((MeshObject*)mColli, (const f32*)box);
}

// 0x80155DCC
void TObjYellowSandFloor::draw()
{
	if (objDebugEnabled(this) != 0) {
		mSurface.update();
		return;
	}
	if (!isInRange(this, &mPos, 2100.0f) || sBrick == 0) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateRun();
		break;
	}
}

// 0x80155E70
void TObjYellowSandFloor::stateWait()
{
	if (sBrick != 0) {
		updatePlace();
		mAct = 1;
		mFlags |= 8;
	}
}

// 0x80155EBC
void TObjYellowSandFloor::stateRun()
{
	if (colliIsDisabled(mShape) == 0) {
		for (ColliContact* hit = mShape->mContacts; hit != 0; hit = hit->mNext) {
			if (colliContactNoBit10(hit) != 0) {
				TObjAction* obj = (TObjAction*)findObjById((u16)colliContactOwner(hit));
				if (obj != 0 && obj->getKind() == 0) {
					FloorMsg msg;
					msg.mId = 0xF;
					if (obj->sendToHandle((ObjMsg*)&msg) != 0) {
						colliDisable(mShape);
						break;
					}
				}
			}
		}
	}
	mSurface.update();
	updatePlace();
}

// 0x80155F90
void TObjYellowSandFloor::vfn_10()
{
	if (cameraIsVisible(2000.0f, mRadius, &mCenter, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80155FF8
void TObjYellowSandFloor::render()
{
	f32 mtx[12];
	mtxConcat(mtx, gjGetMatrix(), mMatrix);
	njPushMatrix(mtx);

	mSurface.begin();
	njSetTexture(mTexList);
	gjDrawObject(mModelData);
	mSurface.end();

	njSetTexture(mTopTexList);
	gjDrawObject(mTopModel);
	njPopMatrixEx();
}
