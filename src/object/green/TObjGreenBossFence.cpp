#include "stage/SetManager.h"

const char* gTObjGreenBossFenceName = "TObjGreenBossFence";
struct DrawTask;
#include "enemy/TEneAm02.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "object/green/GreenObjBank.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "object/green/TObjGreenBossFence.h"

typedef char TObjGreenBossFence_size_check[(sizeof(TObjGreenBossFence) == 0xD4) ? 1 : -1];

// 0x80190294
extern "C" TObject* createTObjGreenBossFence(TObject* parent, SetDesc* desc)
{
	return new TObjGreenBossFence(parent, desc);
}

// 0x801902E8
TObjGreenBossFence::TObjGreenBossFence(TObject* parent, SetDesc* desc)
    : TSetObj(parent, desc)
{
	_00       = (u32)gTObjGreenBossFenceName;
	mSize     = 0xD4;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mLight.reset();
	mLight.setFadeDist(50.0f);
}

// 0x8019036C
TObjGreenBossFence::~TObjGreenBossFence()
{
	removeDrawTask(mDrawTask);
	mDrawTask = 0;
}

// 0x801903E8
void TObjGreenBossFence::draw()
{
	if (!isInRange(this, &mTrans.mPos, 2050.0f)) {
		mFlags |= 1;
	}
}

// 0x80190430
void TObjGreenBossFence::vfn_10()
{
	if (mLight.isVisible(mDrawTask, 2500.0f, &mTrans.mPos, 100.0f, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x801904A0
void TObjGreenBossFence::render()
{
	NJS_OBJECT** models = getGreenObjModels();

	njSetTexture(getGreenObjTextures()[0]);
	njPushMatrixEx();
	njTranslateEx(&mTrans.mPos);
	gjRotateY(mTrans.mRot.y);
	mLight.drawObject(models[1]);
	njPopMatrixEx();
}
