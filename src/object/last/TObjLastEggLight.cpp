#include "stage/SetManager.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "Ninja/ninja.h"
#include "engine/ScrollModel.h"
#include "sound/SeHolder.h"
#include "object/ObjAction.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "object/last/TObjLastEggLight.h"
#include "io/ResBank.h"

extern "C" {
NJS_OBJECT** gLastEggLightModels;
NJS_TEXLIST** gLastEggLightTexLists;
}

ResBank gLastEggLightBank; // 0x802861D8

typedef char TObjLastEggLight_size_check[(sizeof(TObjLastEggLight) == 0xF4) ? 1 : -1];

const char* gTObjLastEggLightName = "TObjLastEggLight";

// 0x8015ED54
// The set hook run when the stage loads
extern "C" void initLastEggLightSet()
{
	resBankLoad(&gLastEggLightBank, "ar_obj_last_egglight.arc");
	resBankAddTextures(&gLastEggLightBank, 0, "obj_last_egglight.gvm");
	gLastEggLightModels   = resBankGetModels(&gLastEggLightBank);
	gLastEggLightTexLists = resBankGetTextures(&gLastEggLightBank);
}

// 0x8015EDBC
// The set hook run when the stage ends
extern "C" void termLastEggLightSet()
{
	resBankRemoveTextures(&gLastEggLightBank, 0);
	resBankRelease(&gLastEggLightBank);
	gLastEggLightModels   = 0;
	gLastEggLightTexLists = 0;
}

// 0x8015EE00
extern "C" TObject* createTObjLastEggLight(TObject* parent, SetDesc* desc)
{
	return new TObjLastEggLight(parent, desc);
}

// 0x8015EE54
TObjLastEggLight::TObjLastEggLight(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00   = (u32)gTObjLastEggLightName;
	mSize = 0xF4;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	mSpin = 0;
	mSurface.reset();
	seHolderStop(&mTone);
	mTone.setRange(1300.0f);
	initFromDesc(desc);
}

// 0x8015EF0C
TObjLastEggLight::~TObjLastEggLight()
{
	seHolderStop(&mTone);
	removeDrawTask(mDraw);
	mDraw = 0;
}

// 0x8015EF9C
void TObjLastEggLight::initFromDesc(SetDesc* desc)
{
	int yaw = mTrans.mRot.y;
	readDesc(desc);
	mTrans.mRot.y = yaw;

	int value = desc->getInt(0);
	int kind  = value < 0 ? 0 : (value > 1 ? 1 : value);
	mModel    = gLastEggLightModels[kind];
	GjModel* model;
	if (mModel != 0 && (model = mModel->model) != 0) {
		mRadius = 200.0f + model->r;
	} else {
		mRadius = 200.0f;
	}
	mSpin = desc->getInt(2);
	mSurface.readDesc(desc);
	mSurface.bindModel(mModel->model, &mTrans);
	if (kind == 1) {
		seHolderPlay(&mTone, 0x28E, &mTrans.mPos);
	} else {
		seHolderStop(&mTone);
	}
}

// 0x8015F0BC
void TObjLastEggLight::draw()
{
	if (isInRange((void*)this, (const Vec3f*)&mTrans.mPos, 1000020.0f) == 0) {
		mFlags |= 1;
	} else {
		mTrans.mRot.y += mSpin;
		mSurface.update();
	}
}

// 0x8015F120
void TObjLastEggLight::debugPrint(void*)
{
	njPrintC(0xA0013, ":MDL NO");
	njPrint(0xA0015, ":ROT_SPD %6d", mSpin);
	mSurface.debugPrint();
	mSurface.debugPrintTrans(&mTrans);
}

// 0x8015F188
void TObjLastEggLight::vfn_10()
{
	if (cameraIsVisible(1000000.0f, mRadius, &mTrans.mPos, 0) != 0) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x8015F1E8
void TObjLastEggLight::render()
{
	mSurface.draw(&mTrans, mModel, *gLastEggLightTexLists);
}
