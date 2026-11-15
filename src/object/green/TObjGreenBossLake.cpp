#include "engine/WaterSurface.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "object/green/GreenObjBank.h"
#include "camera/TCamera.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/green/TObjGreenBossLake.h"
#include "Ninja/ninja.h"

typedef char TObjGreenBossLake_size_check[(sizeof(TObjGreenBossLake) == 0x100) ? 1 : -1];

const char* gTObjGreenBossLakeName = "TObjGreenBossLake";

// 0x80192F80
extern "C" TObject* createTObjGreenBossLake(TObject* parent, SetDesc* desc)
{
	return new TObjGreenBossLake(parent, desc);
}

// 0x80192FD4
TObjGreenBossLake::TObjGreenBossLake(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00      = (u32)gTObjGreenBossLakeName;
	mSize    = 0x100;
	mModel   = getGreenObjModels()[2];
	mTexList = getGreenObjTextures()[0];
	mWater.setModel(mModel, mTexList);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	initFromDesc(desc);
	vfn_74();
}

// 0x801930AC
TObjGreenBossLake::~TObjGreenBossLake()
{
	removeDrawTask(mDrawTask);
}

// 0x80193134
void TObjGreenBossLake::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mWater.initFromDesc(desc);
}

// 0x80193170
void TObjGreenBossLake::debugPrint(void*)
{
	njPrintC(0xA0013, ":MODEL IDX");
	mWater.debugPrint();
}

// 0x801931B4
void TObjGreenBossLake::vfn_74()
{
}

// 0x801931B8
void TObjGreenBossLake::draw()
{
	mWater.update();
}

// 0x801931DC
void TObjGreenBossLake::vfn_10()
{
	if (mWater.isVisible() != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80193238
void TObjGreenBossLake::render()
{
	mWater.render();
}
