#include "object/purple/TObjPurpleGear.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "object/ObjScript.h"
#include "Ninja/ninja.h"
#include "object/ObjArchive.h"
#include "object/TObjLauncher.h"
#include "ui/AeArc.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "stage/SetSlot.h"
#include "object/mission/TMsnObjDarkGate.h"
struct NJS_TEXLIST;
struct SetLink;

extern "C" {

// 0x802C41C0
const char* gTObjPurpleGearName = "TObjPurpleGear";
char* sPurpleGearKindNames[2] = { "SMALL", "BIG" };
}

typedef char TObjPurpleGear_size_check[(sizeof(TObjPurpleGear) == 0x110) ? 1 : -1];

// 0x801033F4
extern "C" TObject* createTObjPurpleGear(TObject* parent, SetDesc* desc)
{
	return new TObjPurpleGear(parent, desc);
}

// 0x80103448
TObjPurpleGear::TObjPurpleGear(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00       = (u32)gTObjPurpleGearName;
	mSize     = 0x110;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mAngle    = 0;
	initFromDesc(desc);
	vfn_74();
}

// 0x801034F0
TObjPurpleGear::~TObjPurpleGear()
{
	removeDrawTask(mDrawTask);
}

// 0x80103564
// 1.0f is 1 Kind 0 reads MODEL_GEAR_S kind 1 reads MODEL_GEAR_L
void TObjPurpleGear::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);

	mKind     = CLAMP_RANGE(desc->getInt(0), 0, 1);
	mRotSpeed = desc->getInt(1);
	mScale    = 1.0f + desc->getFloat(0);

	mtxSetTransV(mMatrix, &mPos);
	mtxRotateZYX((f32*)mMatrix, (const f32*)mMatrix, (const int*)&mRot);
	mtxScale(mScale, mScale, mScale, mMatrix, mMatrix);
	mtxRotateX(mSpin, mMatrix, mAngle);

	switch (mKind) {
	case 0:
		mModelData = findGeObjModel("MODEL_GEAR_S");
		mTexList   = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_GEAR_S");
		break;
	case 1:
		mModelData = findGeObjModel("MODEL_GEAR_L");
		mTexList   = (NJS_TEXLIST*)findGeObjData("TLS_MODEL_GEAR_L");
		break;
	}

	mCenter  = *(Vec3f*)&mModelData->model->center;
	mtxMultVec(mSpin, &mCenter, &mCenter);
	mRadius = mModelData->model->r * mScale;

	mModel.setModel(mModelData);
	mModel.mTexList = mTexList;
	mModel.mState   = (DrawState*)getDefaultDrawState();
}

// 0x80103740
// sPurpleGearKindNames is "SMALL" then "BIG"
void TObjPurpleGear::debugPrint(void*)
{
	njPrint(0xA0013, ":TYPE %s", sPurpleGearKindNames[mKind]);
	njPrintC(0xA0014, ":ROT SPD");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "+1.0:SCALE");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x80103800
void TObjPurpleGear::vfn_74()
{
	setLinkFree((SetLink*)&mLink);
}

// 0x80103824
// 900.0f is 900 The editor path still advances the roll and skips the matrix
void TObjPurpleGear::draw()
{
	if (objDebugEnabled(this) != 0) {
		mAngle += mRotSpeed;
	} else {
		if (!isInRange(this, &mPos, 900.0f)) {
			mFlags |= 1;
		} else {
			mAngle += mRotSpeed;
			mtxRotateX(mSpin, mMatrix, mAngle);
		}
	}
}

// 0x801038B4
// 800.0f is 800
void TObjPurpleGear::vfn_10()
{
	if (cameraIsVisible(800.0f, mRadius, &mCenter, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8010391C
void TObjPurpleGear::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mSpin);
	mModel.draw();
	njPopMatrixEx();
}
