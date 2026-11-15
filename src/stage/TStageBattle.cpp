#include "stage/Stage.h"
#include "core/TMainTask.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "object/TObjPlayer.h"
#include "engine/Viewport.h"
#include "camera/TCamEditor.h"
#include "object/ObjShadow.h"
#include "engine/FrameTex.h"
#include "camera/CamArea.h"
#include "camera/TCamera.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "engine/DrawTask.h"
#include "core/TSceneManager.h"
#include "stage/TEventCtrl.h"
#include "stage/TStageBattle.h"
struct SceneRecord;

typedef char TStageBattle_size_check[(sizeof(TStageBattle) == 0x54) ? 1 : -1];

TStageBattle* sBattleStage; // 0x802C4FB8

// 0x8006B574
extern "C" TStage* createBattleStage()
{
	return new TStageBattle;
}

// 0x8006B5AC
TStageBattle::TStageBattle()
{
	mPlayers = 4;
	for (int i = 0; i < 4; i++) {
		mCamera[i] = i;
		mHuman[i]  = 1;
	}
	sBattleStage = this;
}

// 0x8006B620
TStageBattle::~TStageBattle()
{
	sBattleStage = 0;
}

// 0x8006B680
void TStageBattle::setPlayers(int n)
{
	mPlayers = n < 1 ? 1 : (n > 4 ? 4 : n);
}

// 0x8006B6A8
void TStageBattle::setCamera(int slot, int camera)
{
	slot          = slot < 0 ? 0 : (slot > 3 ? 3 : slot);
	mCamera[slot] = camera < 0 ? 0 : (camera > 3 ? 3 : camera);
}

// 0x8006B6F8
void TStageBattle::setHuman(int slot, int human)
{
	slot         = slot < 0 ? 0 : (slot > 3 ? 3 : slot);
	mHuman[slot] = human;
}

// 0x8006B728
extern "C" void battleStageSetHuman(int slot, int human)
{
	if (sBattleStage != 0) {
		sBattleStage->setHuman(slot, human);
	}
}

// 0x8006B764
// For a half of the screen
void TStageBattle::setWideCamera(int camera)
{
	camSetChaseBase(camera, 16.0f, 120.0f);
	camSave(camera, 15.0f, 23.0f, 180.0f);
}

// 0x8006B7B0
// For a quarter of the screen
void TStageBattle::setQuarterCamera(int camera)
{
	camSetChaseBase(camera, 21.0f, 75.0f);
	camClearSaved(camera);
}

// 0x8006B7F0
// Sets the cameras up for the split then puts a computer player in every slot without a human
void TStageBattle::setupCameras()
{
	switch (mPlayers) {
	case 2:
		setWideCamera(mCamera[0]);
		setWideCamera(mCamera[1]);
		break;
	case 3:
		setWideCamera(mCamera[0]);
		setQuarterCamera(mCamera[1]);
		setQuarterCamera(mCamera[2]);
		break;
	case 4:
	default:
		setQuarterCamera(mCamera[0]);
		setQuarterCamera(mCamera[1]);
		setQuarterCamera(mCamera[2]);
		setQuarterCamera(mCamera[3]);
		break;
	}
	int cpu = 0;
	for (int i = 0; i < 4; i++) {
		if (mHuman[i] == 0) {
			Vec3f pos;
			Vec3f rot;
			findCamStart(cpu, &pos, &rot);
			createTCamFix(i, 10000, &pos, &rot);
			cpu++;
		}
	}
}

// 0x8006B8F8
extern "C" void battleStageSetupCameras()
{
	if (sBattleStage != 0) {
		sBattleStage->setupCameras();
	}
}

// 0x8006B924
void TStageBattle::useCamera(int camera)
{
	camSetDrawView(camera);
	playerSetDrawView((u16)camera);
	getSceneRecord()->mCamera = camera;
}

// 0x8006B964
void TStageBattle::viewTop()
{
	setViewport(3);
	frameTexSelect(2);
	frameTexSetRect(2, 0, 0, 640, 240);
	frameTexSetDest(2, 0, 120);
}

// 0x8006B9B8
void TStageBattle::viewBottom()
{
	setViewport(4);
	frameTexSelect(2);
	frameTexSetRect(2, 0, 240, 640, 240);
	frameTexSetDest(2, 0, 120);
}

// 0x8006BA0C
void TStageBattle::viewTopLeft()
{
	setViewport(5);
	frameTexSelect(0);
	frameTexSetOrigin(0, 0, 0);
}

// 0x8006BA48
void TStageBattle::viewTopRight()
{
	setViewport(6);
	frameTexSelect(0);
	frameTexSetOrigin(0, 320, 0);
}

// 0x8006BA84
void TStageBattle::viewBottomLeft()
{
	setViewport(7);
	frameTexSelect(0);
	frameTexSetOrigin(0, 0, 240);
}

// 0x8006BAC0
void TStageBattle::viewBottomRight()
{
	setViewport(8);
	frameTexSelect(0);
	frameTexSetOrigin(0, 320, 240);
}

// 0x8006BAFC
// The sun its colour and the ambient colour
void TStageBattle::loadAssets()
{
	Vec3f dir;
	u32 sun;
	u32 ambient;

	SceneLights* scene = mLights;
	dir.x              = 0.0f;
	dir.y              = -1.0f;
	dir.z              = 0.0f;
	sceneLightsSetSunDir(scene, &dir);
	sun = 0xFFFFFFFF;
	sceneLightsSetSunColor(scene, &sun);
	ambient = 0x808080FF;
	sceneLightsSetAmbient(scene, &ambient);
}

// 0x8006BB70
void TStageBattle::run(u32 flags)
{
	if (flags & 1) {
		calc();
	}
	if (flags & 2) {
		switch (mPlayers) {
		case 2:
			useCamera(mCamera[0]);
			viewTop();
			drawView();
			useCamera(mCamera[1]);
			viewBottom();
			drawView();
			break;
		case 3:
			useCamera(mCamera[0]);
			viewTop();
			drawView();
			useCamera(mCamera[1]);
			viewBottomLeft();
			drawView();
			useCamera(mCamera[2]);
			viewBottomRight();
			drawView();
			break;
		case 4:
		default:
			useCamera(mCamera[0]);
			viewTopLeft();
			drawView();
			useCamera(mCamera[1]);
			viewTopRight();
			drawView();
			useCamera(mCamera[2]);
			viewBottomLeft();
			drawView();
			useCamera(mCamera[3]);
			viewBottomRight();
			drawView();
			break;
		}
		setViewport(0);
		runDrawTasks(0x3C);
		runDrawTasks(0x3E);
		runDrawTasks(0x3F);
	}
}

// 0x8006BCF4
// One view TStage::drawEffects without the cull mode the extras and the last three lists
void TStageBattle::drawView()
{
	gMainTask.vfn_10();
	drawObjects();
	runDrawTasks(0x20);
	runDrawTasks(0x2C);
	runDrawTasks(0x21);
	shadowAlphaOff();
	runDrawTasks(0x2D);
	sortDrawTasks(0x22);
	runDrawTasks(0x22);
	shadowAlphaOn();
	runDrawTasks(0x23);
	shadowDraw();
	frameTexRequestCur();
	frameTexUpdateCur();
	shadowDarkenScreen();
	runDrawTasks(0x2A);
	GXSetCullMode(GX_CULL_NONE);
	runDrawTasks(0x32);
	spriteDrawAll();
	GXSetCullMode(GX_CULL_NONE);
	runDrawTasks(0x3A);
	runDrawTasks(0x3B);
	if (eventIsBusy() == 0) {
		runDrawTasks(0x3D);
	}
	GXSetCullMode(GX_CULL_NONE);
}
