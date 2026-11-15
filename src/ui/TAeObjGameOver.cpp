#include "ui/TAeObjGameOver.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "object/TObjMGLeader.h"
#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "engine/CharaColli.h"
#include "io/Resource.h"
#include "camera/TCamera.h"
#include "ui/TSimpleText.h"
#include "stage/EventScript.h"
#include "engine/Color.h"
#include "object/TObjLauncher.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "object/ObjShadow.h"
#include "io/Arc.h"
#include "ui/AeArc.h"
#include "ui/TAeObject.h"
#include "object/ObjAction.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "engine/GroundQuery.h"
#include "stage/SetSlot.h"
#include "ui/MessageTable.h"
#include "sound/SoundCtrl.h"
#include "stage/TEventCtrl.h"
#include "ui/AeObjControl.h"
#include "core/TMainTask.h"

typedef char TAeObjGameOver_size_check[(sizeof(TAeObjGameOver) == 0x3C) ? 1 : -1];

const char* gTAeObjGameOverName = "TAeObjGameOver";

extern "C" {
// ae_game_over.arc and what it holds
TArc* gAeGameOverArc;            // 0x802C5558
void* gAeGameOverData;           // 0x802C555C
NJS_TEXLIST* gAeGameOverTexList; // 0x802C5560
}

// What ae_game_over.arc holds
struct AeGameOverArc {
	void* mAe;             // _00
	NJS_TEXLIST* mTexList; // _04
	void* mGvm;            // _08
};

// 0x80113800
extern "C" void initAeGameOver()
{
	gAeGameOverArc     = 0;
	gAeGameOverData    = 0;
	gAeGameOverTexList = 0;
}

// 0x80113814
extern "C" void termAeGameOver()
{
}

// 0x80113818
extern "C" int loadAeGameOver()
{
	AeGameOverArc* contents;

	aeLoadArc("ae_game_over.arc", &gAeGameOverArc);
	if (gAeGameOverArc != 0) {
		void* ae;
		NJS_TEXLIST* texList;

		contents        = (AeGameOverArc*)gAeGameOverArc->mData;
		ae              = contents->mAe;
		gAeGameOverData = ae;
		relocateAe(ae);
		texList            = contents->mTexList;
		gAeGameOverTexList = texList;
		bindGvmTextures((u8*)contents->mGvm, texList);
		return 1;
	}
	return 0;
}

// 0x80113884
extern "C" void releaseAeGameOver()
{
	if (gAeGameOverArc != 0) {
		releaseTextures(gAeGameOverTexList);
		gAeGameOverTexList = 0;
		aeFreeArc(&gAeGameOverArc);
		gAeGameOverArc  = 0;
		gAeGameOverData = 0;
	}
}

// 0x801138D0
extern "C" TAeObjGameOver* createAeGameOver()
{
	if (gAeGameOverArc == 0) {
		return 0;
	}
	return new TAeObjGameOver(gTL_07, gAeGameOverData, gAeGameOverTexList);
}

// 0x8011392C
TAeObjGameOver::TAeObjGameOver(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00    = (u32)gTAeObjGameOverName;
	mSize  = 0x3C;
	mLayer = createLayer(9, 0x10);
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	_30    = 0;
}

// 0x801139B0
TAeObjGameOver::~TAeObjGameOver()
{
	releaseAeGameOver();
	removeDrawTask(mTask);
}

// 0x80113A1C
void TAeObjGameOver::draw()
{
	if (mLayer != 0) {
		if (mLayer->isDone() != 0) {
			_30 = 1;
		} else {
			mLayer->update();
		}
	}
}

// 0x80113A74
void TAeObjGameOver::render()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}
