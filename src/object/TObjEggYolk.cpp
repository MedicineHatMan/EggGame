#include "stage/Stage.h"
#include "ui/TextVec.h"
#include "engine/MotionPlayer.h"
#include "engine/DrawTask.h"
#include "engine/DrawState.h"
#include "egg/ObjEgg.h"
#include "core/Pad.h"
#include "core/TArray.h"
#include "core/TMainTask.h"
#include "ui/TSimpleText.h"
#include "camera/TCamera.h"
#include "engine/Viewport.h"
#include "engine/ShadowTex.h"
#include "ui/AeArc.h"
#include "ui/TAeObjStageSelectE3.h"
#include "stage/TStageSelectModelE3.h"
#include "object/ObjAction.h"
#include "object/TObjLauncher.h"
#include "io/Arc.h"
#include "camera/TCameraCtrl.h"
#include "ui/TAeObjMsgWindow.h"
#include "sound/SoundCtrl.h"
#include "ui/TFade.h"
#include "core/dolphin.h"
#include "object/TObjEggYolk.h"
#include "ui/TAeObject.h"
#include "core/Mem.h"
#include "engine/GroundQuery.h"
#include "Ninja/ninja.h"
struct NJS_TEXLIST;
struct GjActor;

class TArc;

inline Vec3f vec3f(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// 0x80282560
// built by the static initializer at 0x801015AC the title's place box and glyph size then the rows'
static TextVec sE3TitlePos   = textVec(90.0f, 40.0f, -2.0f);
static TextVec sE3TitleBox   = textVec(494.0f, 54.0f, 0.0f);
static TextVec sE3TitleScale = textVec(1.0f, 1.0f, 1.0f);
static TextVec sE3RowPos     = textVec(160.0f, 372.0f, -2.0f);
static TextVec sE3RowBox     = textVec(607.0f, 148.0f, 0.0f);
static TextVec sE3RowScale   = textVec(1.0f, 1.0f, 1.0f);

// 0x802C4198
const char* sE3Title = "Select a Stage To Play";

// 0x8025CC88
// one per row
const char* sE3StageNames[4] = {
	"Mission\tbRecover Patriarch Chicken, Uli-Uli!",
	"Mission\tbCaptured Chick!",
	"Mission\tbAfter Heavy Snowfall...",
	"Boss Battle\tbBeat Down CAPTAIN GLER!!",
};

class TModeStageSelectE3 : public TMode {
public:
	enum State {
		E3_SELECT  = 0,
		E3_LEAVING = 1, // the fade out then the mode ends
	};

	TModeStageSelectE3();

	virtual ~TModeStageSelectE3();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void select();
	void leave();
	void choose();

	int mState;                  // _0C State
	int mTimer;                  // _10
	int mWorld;                  // _14 of the stage chosen
	int _18;                     // _18
	TArray<const char*> mStages; // _1C the stages chosen
	int mCursor;                 // _28 -1 = none
	TSimpleText* mRows[4];       // _2C
	TSimpleText* mTitle;         // _3C
	TObject* mCamera;            // _40
	TStageSelectModelE3* mModel; // _44
	u32 _48;                     // _48
};

typedef char TModeStageSelectE3_size_check[(sizeof(TModeStageSelectE3) == 0x4C) ? 1 : -1];

// 0x80100D14
extern "C" TMode* createModeStageSelectE3(TModeAllocator* allocator)
{
	return new (allocator) TModeStageSelectE3;
}

// 0x80100D4C
int TModeStageSelectE3::init()
{
	GXSetCopyClear(sEggYolkClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	loadMsgWindow();
	loadStageSelectE3();
	loadE3ModelArc();
	gjSetAmbient(0.6f, 0.65f, 0.6f);
	gjSetLightSwitch(0, 1);
	gjSetLightVector(0, 0.5f, -1.0f, 0.0f);
	gjSetLightColor(0, 1.0f, 1.0f, 1.0f);
	gjLoadLight(0);
	objClearLayers(&gMainTask);
	fontCreateTask();
	createEditCameraCtrl();
	camSetDrawView(0);
	Vec3f eye;
	Vec3f at;
	at.x    = 0.0f;
	at.y    = 25.0f;
	at.z    = 100.0f;
	eye.x   = 0.0f;
	eye.y   = 0.0f;
	eye.z   = 0.0f;
	mCamera = createTCamFix(0, 2, &eye, &at);
	setViewport(0);
	createStageSelectE3();
	mTitle = createSimpleTextOnTask(1, -1, &sE3TitlePos, &sE3TitleBox, &sE3TitleScale, (void*)sE3Title);
	mTitle->render();
	mTitle->mFlags |= 0x10;
	for (int i = 0; i < 4; i++) {
		mRows[i] = createSimpleTextOnTask(1, -1, &sE3RowPos, &sE3RowBox, &sE3RowScale,
		                                  (void*)sE3StageNames[i]);
		mRows[i]->mFlags |= 0x10;
	}
	Vec3f pos;
	pos.x  = -28.5f;
	pos.y  = 7.5f;
	pos.z  = 30.0f;
	mModel = createTStageSelectModelE3(gTL_05, &pos, 0, 0);
	bgmPlay(1);
	return 1;
}

// 0x80100F30
int TModeStageSelectE3::shutdown()
{
	if (mCamera != 0) {
		mCamera->mFlags |= 1;
		mCamera = 0;
	}
	mTitle->mFlags |= 1;
	mRows[0]->mFlags |= 1;
	mRows[1]->mFlags |= 1;
	mRows[2]->mFlags |= 1;
	mRows[3]->mFlags |= 1;
	mRows[3] = 0;
	mRows[2] = 0;
	mRows[1] = 0;
	mRows[0] = 0;
	mTitle   = 0;
	objClearLayers(&gMainTask);
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	freeMsgWindow();
	freeStageSelectE3();
	freeE3ModelArc();
	bgmStop();
	return 1;
}

// 0x80101004
TModeStageSelectE3::TModeStageSelectE3()
{
	mState         = E3_SELECT;
	mTimer         = 30;
	mCursor        = 0;
	mWorld         = 0;
	_18            = 0;
	mStages.clear();
	_48            = 0;
	mTitle         = 0;
	mRows[3]       = 0;
	mRows[2]       = 0;
	mRows[1]       = 0;
	mRows[0]       = 0;
	mCamera        = 0;
	mModel         = 0;
}

// 0x801010A0
TModeStageSelectE3::~TModeStageSelectE3() { }

// 0x801010FC
void TModeStageSelectE3::update()
{
	gMainTask.draw();
	gjBeforeRender();
	switch (mState) {
	case E3_SELECT:
		select();
		break;
	case E3_LEAVING:
		leave();
		break;
	}
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_NONE);
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	runDrawTasks(0x21);
	runDrawTasks(0x3E);
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();
	gjPrintFlush();
	gjDoneRender();
	gjSwapBuffers();
}

// 0x801011C0
void TModeStageSelectE3::select()
{
	if ((padGet(0)->mRepeat & 0x200000) || (padGet(0)->mRepeat & 8)) {
		if (mCursor > 0) {
			mCursor--;
		} else {
			mCursor = 3;
		}
		showStageSelectE3(mCursor);
		if (mModel != 0) {
			mModel->mIndex = mCursor;
		}
		sePlay(0x1B);
	} else if ((padGet(0)->mRepeat & 0x100000) || (padGet(0)->mRepeat & 4)) {
		if (mCursor < 3) {
			mCursor++;
		} else {
			mCursor = 0;
		}
		showStageSelectE3(mCursor);
		if (mModel != 0) {
			mModel->mIndex = mCursor;
		}
		sePlay(0x1B);
	}
	if (padGet(0)->mPressed & 0x100) {
		if (mCursor == -1) {
			return;
		}
		choose();
		mState = E3_LEAVING;
		mTimer = 60;
		fadeCircleClose();
		sePlay(0x1C);
	} else if (padGet(0)->mPressed & 0x200) {
		_08 = 1;
		sePlay(0x20);
	}
	if (mTimer == 0) {
		mTitle->mFlags &= ~0x10;
		if (mCursor != -1) {
			mRows[0]->mFlags |= 0x10;
			mRows[1]->mFlags |= 0x10;
			mRows[2]->mFlags |= 0x10;
			mRows[3]->mFlags |= 0x10;
			mRows[mCursor]->mFlags &= ~0x10;
		}
	} else {
		mTimer--;
	}
}

// 0x801013C0
void TModeStageSelectE3::leave()
{
	if (mTimer == 0) {
		_08 = 0;
	} else {
		mTimer--;
	}
}

// 0x801013E4
// Keeps the stages of the row's world
void TModeStageSelectE3::choose()
{
	StageDefTable* table = getStageDefTable();
	switch (mCursor) {
	case 0:
		for (int i = 0; i < table->mCount; i++) {
			const char* name = table->mEntries[i].mName;
			if (strncmp(name, "blue", 4) == 0) {
				mStages.push(name);
			}
		}
		mWorld = 0;
		break;
	case 1:
		for (int i = 0; i < table->mCount; i++) {
			const char* name = table->mEntries[i].mName;
			if (strncmp(name, "red", 3) == 0) {
				mStages.push(name);
			}
		}
		mWorld = 2;
		break;
	case 2:
		for (int i = 0; i < table->mCount; i++) {
			const char* name = table->mEntries[i].mName;
			if (strncmp(name, "purple", 6) == 0) {
				mStages.push(name);
			}
		}
		mWorld = 3;
		break;
	case 3:
		for (int i = 0; i < table->mCount; i++) {
			const char* name = table->mEntries[i].mName;
			if (strcmp(name, "blue_boss") == 0) {
				mStages.push(name);
			}
		}
		mWorld = 0;
		break;
	}
}

