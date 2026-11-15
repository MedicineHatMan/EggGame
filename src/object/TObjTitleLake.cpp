#include "object/TObjTitleLake.h"
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "ui/TitleObjArc.h"
#include "Ninja/ninja.h"
struct NJS_TEXLIST;

typedef char TObjTitleLake_size_check[(sizeof(TObjTitleLake) == 0x100) ? 1 : -1];

// 0x801B0C1C
extern "C" TObject* createTObjTitleLake(TObject* parent, SetDesc* desc)
{
	return new TObjTitleLake(parent, desc);
}

// 0x801B0C70
TObjTitleLake::TObjTitleLake(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00                 = gTObjTitleLakeName;
	mSize               = 0x100;
	NJS_OBJECT** models = getTitleObjModels();
	mModel              = models[8];
	void** textures     = (void**)getTitleObjTexLists();
	mTexList            = (NJS_TEXLIST*)textures[0];
	mWater.setModel(mModel, mTexList);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	initFromDesc(desc);
	vfn_74();
}

// 0x801B0D38
TObjTitleLake::~TObjTitleLake()
{
	removeDrawTask(mDraw);
}

// 0x801B0DAC
void TObjTitleLake::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mWater.initFromDesc(desc);
}

// 0x801B0DE8
void TObjTitleLake::debugPrint(void*)
{
	njPrintC(0xA0013, ":MODEL IDX");
	mWater.debugPrint();
}

// 0x801B0E2C
void TObjTitleLake::vfn_74()
{
}

// 0x801B0E30
void TObjTitleLake::draw()
{
	mWater.update();
}

// 0x801B0E54
void TObjTitleLake::vfn_10()
{
	if (mWater.isVisible() != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x801B0EB0
void TObjTitleLake::render()
{
	mWater.render();
}
