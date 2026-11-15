#include "ui/TAeObjStageTitle.h"
struct DrawTask;
#include "camera/TCamera.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "ui/AeArc.h"
#include "ui/TSimpleText.h"
#include "stage/TStageSelectModel.h"
#include "camera/EventCamArc.h"
#include "egg/EggCreature.h"
#include "stage/TEventCtrl.h"
#include "core/TMainTask.h"
#include "core/Pad.h"
struct NJS_TEXLIST;

extern "C" {

}

typedef char TAeObjStageTitle_size_check[(sizeof(TAeObjStageTitle) == 0x54) ? 1 : -1];

// 0x80246028
// The layer each world's banner plays -1 = none
static const s32 sTitleLayers[7] = { 3, 3, 3, 3, 2, 3, 3 };
static const char* sClassName = "TAeObjStageTitle"; // 0x802C4108

TArc* sTitleRes;     // 0x802C5320 the archive
void* sTitleAe;      // 0x802C5324
NJS_TEXLIST* sTitleTex;     // 0x802C5328
s32 sTitleWorld;     // 0x802C532C -1 = none
s32 sTitleMission;   // 0x802C5330 -1 = none

// 0x800E90AC
extern "C" int stageTitleHideObjs()
{
	eventHideObj((u8*)gTL_07);
	return 1;
}

// 0x800E90D4
extern "C" void stageTitleReset()
{
	sTitleRes     = 0;
	sTitleAe      = 0;
	sTitleTex     = 0;
	sTitleWorld   = -1;
	sTitleMission = -1;
}

// 0x800E90F4
extern "C" void stageTitleRelease()
{
	if (sTitleRes != 0) {
		releaseTextures(sTitleTex);
		sTitleTex = 0;
		releaseResource(sTitleRes);
		sTitleRes = 0;
		sTitleAe  = 0;
	}
}

// 0x800E9140
// The "load_stg_title" command the world then the mission
extern "C" int stageTitleLoadCmd(s32* args)
{
	sTitleMission = args[1];
	sTitleWorld   = args[0];
	return stageTitleLoad();
}

// 0x800E9170
extern "C" int stageTitleLoad()
{
	const char* name = 0;
	switch (sTitleWorld) {
	case 0:
		name = "ae_stage_b_title.arc";
		break;
	case 1:
		name = "ae_stage_r_title.arc";
		break;
	case 2:
		name = "ae_stage_p_title.arc";
		break;
	case 3:
		name = "ae_stage_o_title.arc";
		break;
	case 4:
		name = "ae_stage_y_title.arc";
		break;
	case 5:
		name = "ae_stage_g_title.arc";
		break;
	case 6:
		name = "ae_stage_w_title.arc";
		break;
	}
	if (name != 0) {
		loadArc(name, &sTitleRes);
		AeArcBlock* block = aeArcBlock(sTitleRes);
		sTitleAe          = block->mAe;
		relocateAe(sTitleAe);
		sTitleTex = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, (NJS_TEXLIST*)sTitleTex);
	}
	return 1;
}

// 0x800E9230
extern "C" TAeObjStageTitle* createStageTitle()
{
	return new TAeObjStageTitle(gTL_07, sTitleAe, sTitleTex);
}

// 0x800E9278
TAeObjStageTitle::TAeObjStageTitle(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00    = (u32)sClassName;
	mSize  = 0x54;
	mLayer = 0;
	if (sTitleWorld != -1 && sTitleLayers[sTitleWorld] != -1) {
		mLayer = createLayer(sTitleLayers[sTitleWorld], 0x10);
	}
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	mMissionText = 0;
	if (sTitleMission != -1) {
		char* text = getStageSelectMission(sTitleMission);
		if (text != 0) {
			mMissionText = createSimpleTextTimed(text, 0xB4);
		}
	}
	mWait      = 1;
	mEvent     = 0;
	mHintCam   = 0;
	mHintTried = 0;
	mDone      = 0;
}

// 0x800E9364
TAeObjStageTitle::~TAeObjStageTitle()
{
	if (mMissionText != 0) {
		mMissionText->mHold = 1;
		mMissionText        = 0;
	}
	if (mHintCam != 0) {
		mHintCam->mFlags |= 1;
		mHintCam = 0;
	}
	removeDrawTask(mTask);
}

// 0x800E9408
void TAeObjStageTitle::finish()
{
	if (mMissionText != 0) {
		mMissionText->mHold = 1;
		mMissionText        = 0;
	}
	if (mHintCam != 0) {
		mHintCam->mFlags |= 1;
		mHintCam = 0;
	}
	if (mEvent != 0) {
		eventEnd();
		mEvent = 0;
	}
	mDone = 1;
}

// 0x800E948C
void TAeObjStageTitle::draw()
{
	u32* pad = ((u32*)padGet(0));
	if ((pad[2] & 0x100) != 0 || (((u32*)padGet(0))[2] & 0x1000) != 0) {
		finish();
		return;
	}
	if (mDone != 0) {
		return;
	}
	if (mWait != 0) {
		mWait = mWait - 1;
		return;
	}
	if (mEvent == 0) {
		mEvent = eventStartById(0xFFFF, (int (*)(int))stageTitleHideObjs);
		return;
	}
	if (mHintTried == 0 && mHintCam == 0) {
		const char* name = "hint";
		mHintCam = eventCamArcPlayNamed(1.0f, 0.0f, name);
		if (mHintCam != 0) {
			((TCamera*)mHintCam)->setCut();
		}
		mHintTried = 1;
		return;
	}
	if (mLayer != 0) {
		mLayer->update();
	}
	if (mHintCam != 0) {
		if (mHintCam->isDone() != 0) {
			finish();
		}
	} else if (mLayer != 0) {
		if (mLayer->isDone() != 0) {
			finish();
		}
	} else {
		finish();
	}
}

// 0x800E95E0
void TAeObjStageTitle::render()
{
	if (mDone == 0 && mLayer != 0) {
		mLayer->present();
	}
}

// 0x800E9618
// The set's init and term empty
extern "C" void initStageTitleSet()
{
}

// 0x800E961C
extern "C" void termStageTitleSet()
{
}
