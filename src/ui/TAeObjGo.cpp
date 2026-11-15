#include "ui/TAeObjGo.h"
struct DrawTask;
struct NJS_TEXLIST;
#include "ui/AeArc.h"
#include "ui/AeObjControl.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "egg/EggCreature.h"
#include "enemy/boss/TEneYellowBossBlue.h"
#include "egg/TEggCreatureTypeE.h"
#include "animal/TAniManager.h"
#include "core/TMainTask.h"

typedef char TAeObjGo_size_check[(sizeof(TAeObjGo) == 0x3C) ? 1 : -1];

const char* gTAeObjGoName = "TAeObjGo";

// What ae_go.arc holds
struct AeGoArc {
	void* mAe;             // _00
	NJS_TEXLIST* mTexList; // _04
	void* mGvm;            // _08
};

extern "C" {
TArc* gAeGoArc;            // 0x802C5770
void* gAeGoData;           // 0x802C5774
NJS_TEXLIST* gAeGoTexList; // 0x802C5778
}

// 0x80152BE0
extern "C" void initAeGo()
{
	gAeGoArc     = 0;
	gAeGoData    = 0;
	gAeGoTexList = 0;
}

// 0x80152BF4
extern "C" void termAeGo()
{
}

// 0x80152BF8
extern "C" int loadAeGo()
{
	AeGoArc* contents;

	aeLoadArc("ae_go.arc", &gAeGoArc);
	if (gAeGoArc != 0) {
		void* ae;
		NJS_TEXLIST* texList;

		contents  = (AeGoArc*)gAeGoArc->mData;
		ae        = contents->mAe;
		gAeGoData = ae;
		relocateAe(ae);
		texList      = contents->mTexList;
		gAeGoTexList = texList;
		bindGvmTextures((u8*)contents->mGvm, texList);
		return 1;
	}
	return 0;
}

// 0x80152C64
extern "C" void releaseAeGo()
{
	if (gAeGoArc != 0) {
		releaseTextures(gAeGoTexList);
		gAeGoTexList = 0;
		aeFreeArc(&gAeGoArc);
		gAeGoArc  = 0;
		gAeGoData = 0;
	}
}

// 0x80152CB0
extern "C" TAeObjGo* createAeGo()
{
	if (gAeGoArc == 0) {
		return 0;
	}
	return new TAeObjGo(gTL_08, gAeGoData, gAeGoTexList);
}

// 0x80152D0C
TAeObjGo::TAeObjGo(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00    = (u32)gTAeObjGoName;
	mSize  = 0x3C;
	mLayer = createLayer(7, 0x10);
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
}

// 0x80152D88
TAeObjGo::~TAeObjGo()
{
	releaseAeGo();
	removeDrawTask(mTask);
}

// 0x80152DF4
void TAeObjGo::draw()
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

// 0x80152E58
void TAeObjGo::render()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}
