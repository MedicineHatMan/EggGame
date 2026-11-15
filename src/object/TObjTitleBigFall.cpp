#include "object/TObjTitleBigFall.h"
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "ui/TitleObjArc.h"
#include "Ninja/ninja.h"

const char* gTObjTitleBigFallName = "TObjTitleBigFall";
struct NJS_TEXLIST;

typedef char TObjTitleBigFall_size_check[(sizeof(TObjTitleBigFall) == 0x100) ? 1 : -1];

// 0x801B0EE0
extern "C" TObject* createTObjTitleBigFall(TObject* parent, SetDesc* desc)
{
	return new TObjTitleBigFall(parent, desc);
}

// 0x801B0F34
TObjTitleBigFall::TObjTitleBigFall(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00                 = (u32)gTObjTitleBigFallName;
	mSize               = 0x100;
	NJS_OBJECT** models = getTitleObjModels();
	mModel              = models[7];
	void** textures     = (void**)getTitleObjTexLists();
	mTexList            = (NJS_TEXLIST*)textures[0];
	mWater.setModel(mModel, mTexList);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x2A, 0);
	initFromDesc(desc);
	vfn_74();
}

// 0x801B0FFC
TObjTitleBigFall::~TObjTitleBigFall()
{
	removeDrawTask(mDraw);
}

// 0x801B1070
void TObjTitleBigFall::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mWater.initFromDesc(desc);
}

// 0x801B10AC
void TObjTitleBigFall::vfn_74()
{
}

// 0x801B10B0
void TObjTitleBigFall::debugPrint(void*)
{
	njPrintC(0xA0013, "");
	mWater.debugPrint();
}

// 0x801B10F0
void TObjTitleBigFall::draw()
{
	mWater.update();
}

// 0x801B1114
void TObjTitleBigFall::vfn_10()
{
	if (mWater.isVisible() != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x801B1170
void TObjTitleBigFall::render()
{
	mWater.render();
}
