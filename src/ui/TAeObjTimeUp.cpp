#include "ui/TAeObjTimeUp.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "ui/AeObjControl.h"
#include "core/TMainTask.h"
struct TArc;

// 0x801D039C
// loads ae_timeup.arc and 0x801D0408 releases it
// Same layer id as TAeObjGoal

typedef char TAeObjTimeUp_size_check[(sizeof(TAeObjTimeUp) == 0x3C) ? 1 : -1];

const char* gTAeObjTimeUpName = "TAeObjTimeUp";

// 0x801D039C
int loadTimeUp()
{
	u32* header;

	aeLoadArc((const char*)("ae_timeup.arc"), (TArc**)&sAeTimeUpArc);
	if (sAeTimeUpArc != 0) {
		header = *(u32**)(sAeTimeUpArc + 0x4C);
		relocateAe((void*)(sAeTimeUpData = header[0]));
		sAeTimeUpTex = header[1];
		bindGvmTextures((u8*)header[2], (NJS_TEXLIST*)sAeTimeUpTex);
		return 1;
	}
	return 0;
}

// 0x801D0408
void aeTimeUpFree()
{
	if (sAeTimeUpArc != 0) {
		releaseTextures((NJS_TEXLIST*)sAeTimeUpTex);
		sAeTimeUpTex = 0;
		aeFreeArc((TArc**)&sAeTimeUpArc);
		sAeTimeUpArc  = 0;
		sAeTimeUpData = 0;
	}
}

// 0x801D0454
// Not a method Refuses to build one when the archive slot is empty
TAeObjTimeUp* createTimeUp()
{
	if (sAeTimeUpArc == 0) {
		return 0;
	}
	return new TAeObjTimeUp((TObject*)gTL_07, (void*)sAeTimeUpData, (void*)sAeTimeUpTex);
}

// 0x801D04B0
TAeObjTimeUp::TAeObjTimeUp(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00    = (u32)gTAeObjTimeUpName;
	mSize  = 0x3C;
	mLayer = createLayer(0xB, 0x10);
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	_30    = 0;
}

// 0x801D0534
TAeObjTimeUp::~TAeObjTimeUp()
{
	aeTimeUpFree();
	removeDrawTask(mTask);
}

// 0x801D05A0
void TAeObjTimeUp::draw()
{
	if (mLayer != 0) {
		if (mLayer->isDone() != 0) {
			_30 = 1;
			mFlags |= 1;
		} else {
			mLayer->update();
		}
	}
}

// 0x801D0604
void TAeObjTimeUp::render()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}
