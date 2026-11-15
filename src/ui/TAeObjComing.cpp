#include "ui/TAeObjComing.h"
#include "mode/TModeComing.h"
#include "io/Resource.h"
#include "io/LangFile.h"
#include "core/TMainTask.h"
#include "ui/AeArc.h"
#include "core/Mem.h"
#include "Ninja/ninja.h"

struct NJS_TEXLIST;

const char* gTAeObjComingName = "TAeObjComing";

TArc* sComingArc;            // 0x802C5720
void* sComingAe;             // 0x802C5724
NJS_TEXLIST* sComingTexList; // 0x802C5728

struct ComingArcData {
	void* mAe;       // _00
	NJS_TEXLIST* mTexList;  // _04
	u8* mTextures;   // _08
};

typedef char TAeObjComing_size_check[(sizeof(TAeObjComing) == 0x3C) ? 1 : -1];

// 0x80142BFC
void comingLoad()
{
	loadArc("ae_coming.arc", &sComingArc);
	if (sComingArc != 0) {
		ComingArcData* data = (ComingArcData*)sComingArc->mData;
		sComingAe           = data->mAe;
		relocateAe(sComingAe);
		sComingTexList = data->mTexList;
		bindGvmTextures(data->mTextures, sComingTexList);
	}
}

// 0x80142C5C
void comingFree()
{
	if (sComingArc != 0) {
		releaseTextures((NJS_TEXLIST*)sComingTexList);
		sComingTexList = 0;
		releaseResource(sComingArc);
		sComingArc = 0;
		sComingAe  = 0;
	}
}

// 0x80142CA8
TAeObjComing* comingCreate()
{
	if (sComingArc == 0) {
		return 0;
	}
	return new TAeObjComing(gTL_07, sComingAe, sComingTexList);
}

// 0x80142D04
TAeObjComing::TAeObjComing(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00    = (u32)gTAeObjComingName;
	mSize  = 0x3C;
	mLayer = createLayer(9, 0x10);
	mDone  = 0;
}

// 0x80142D6C
TAeObjComing::~TAeObjComing() { }

// 0x80142DC8
void TAeObjComing::draw()
{
	if (mLayer != 0) {
		if (mLayer->isDone() != 0) {
			mDone = 1;
		} else {
			mLayer->update();
		}
	}
}

// 0x80142E20
void TAeObjComing::vfn_10()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}
