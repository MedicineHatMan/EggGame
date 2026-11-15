#include "ui/TAeObjTitle.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "ui/AeSprite.h"
#include "egg/EggCreature.h"
#include "io/NinjaFile.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

typedef char TAeObjTitle_size_check[(sizeof(TAeObjTitle) == 0x48) ? 1 : -1];

const char* gTAeObjTitleName = "TAeObjTitle";

// 0x800A5E9C
extern "C" void loadAeTitle()
{
	loadArc(getLangFileName("ae_egg_title.arc"), &sAeTitleArc);
	if (sAeTitleArc != 0) {
		AeArcBlock* block = aeArcBlock(sAeTitleArc);
		sAeTitleAe        = block->mAe;
		relocateAe(sAeTitleAe);
		sAeTitleTexList = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, sAeTitleTexList);
	}
}

extern const NJS_QUAD_TEXTURE_EX sAeZeroQuad = { 0 };

// 0x800A5F00
extern "C" void freeAeTitle()
{
	if (sAeTitleArc != 0) {
		releaseTextures((NJS_TEXLIST*)sAeTitleTexList);
		sAeTitleTexList = 0;
		releaseResource(sAeTitleArc);
		sAeTitleArc = 0;
		sAeTitleAe  = 0;
	}
}

// 0x800A5F4C
extern "C" TAeObjTitle* createAeTitle()
{
	TAeObjTitle* made;

	if (sAeTitleArc == 0) {
		return 0;
	}
	made = new TAeObjTitle(gTL_07, sAeTitleAe, sAeTitleTexList);
	return made;
}

// 0x800A5FA8
TAeObjTitle::TAeObjTitle(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00     = (u32)gTAeObjTitleName;
	mSize   = 0x48;
	mLayer  = createLayer(0xF9, 0x10);
	mLayer2 = 0;
	_40     = 0;
	mTask   = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3E, 0);
	mState  = 0;
}

// 0x800A6038
TAeObjTitle::~TAeObjTitle()
{
	removeDrawTask(mTask);
}

// 0x800A60A0
// Once the logo is in holds it and starts the press-start layer bound to it
void TAeObjTitle::showPrompt()
{
	if (mLayer != 0) {
		mLayer2 = createLayer(0xB9, 1);
		mLayer->reset(0xFB, 1);
		mLayer->bind(0xFC, mLayer2);
	}
	mState = 1;
}

// 0x800A6124
// Start was pressed flashes the prompt
void TAeObjTitle::pressed()
{
	if (mLayer2 != 0) {
		mLayer2->reset(0xCE, 0x10);
	}
}

// 0x800A615C
void TAeObjTitle::draw()
{
	if (debugEditActive() != 0) {
		return;
	}
	if (mLayer != 0) {
		mLayer->update();
		switch (mState) {
		case 0:
			if (mLayer->isDone() != 0) {
				showPrompt();
			}
			break;
		case 1:
			break;
		}
	}
	if (mLayer2 != 0) {
		mLayer2->update();
	}
}

// 0x800A61EC
void TAeObjTitle::vfn_10()
{
}

// 0x800A61F0
void TAeObjTitle::render()
{
	if (debugEditActive() == 0 && mLayer != 0) {
		mLayer->present();
	}
}
