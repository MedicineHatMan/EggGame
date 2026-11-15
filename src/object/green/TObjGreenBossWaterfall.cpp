#include "engine/WaterSurface.h"

const char* gTObjGreenBossWaterfallName = "TObjGreenBossWaterfall";
struct DrawTask;
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "object/green/GreenObjBank.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/green/TObjGreenBossWaterfall.h"
#include "Ninja/ninja.h"

typedef char TObjGreenBossWaterfall_size_check[(sizeof(TObjGreenBossWaterfall) == 0x100) ? 1 : -1];

// 0x80192C9C
extern "C" TObject* createTObjGreenBossWaterfall(TObject* parent, SetDesc* desc)
{
	return new TObjGreenBossWaterfall(parent, desc);
}

// 0x80192CF0
TObjGreenBossWaterfall::TObjGreenBossWaterfall(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00      = (u32)gTObjGreenBossWaterfallName;
	mSize    = 0x100;
	mModel   = getGreenObjModels()[0];
	mTexList = getGreenObjTextures()[0];
	mWater.setModel(mModel, mTexList);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	initFromDesc(desc);
	vfn_74();
}

// 0x80192DC8
TObjGreenBossWaterfall::~TObjGreenBossWaterfall()
{
	removeDrawTask(mDrawTask);
}

// 0x80192E50
void TObjGreenBossWaterfall::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mWater.initFromDesc(desc);
}

// 0x80192E8C
void TObjGreenBossWaterfall::vfn_74()
{
}

// 0x80192E90
void TObjGreenBossWaterfall::debugPrint(void*)
{
	njPrintC(0xA0013, "");
	mWater.debugPrint();
}

// 0x80192ED0
void TObjGreenBossWaterfall::draw()
{
	mWater.update();
}

// 0x80192EF4
void TObjGreenBossWaterfall::vfn_10()
{
	if (mWater.isVisible() != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80192F50
void TObjGreenBossWaterfall::render()
{
	mWater.render();
}
