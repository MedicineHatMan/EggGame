#include "Ninja/ninja.h"
#include "stage/GameSelect.h"
#include "core/Pad.h"
#include "engine/ShadowTex.h"
#include "core/TMainTask.h"
#include "engine/Viewport.h"
#include "ui/TAeObjComing.h"
#include "io/ResManager.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "camera/TCameraCtrl.h"
#include "engine/DrawTask.h"
#include "mode/TModeComing.h"
#include "ui/TAeObject.h"
#include "ui/AeArc.h"
#include "core/Mem.h"
#include "stage/Stage.h"
struct NJS_TEXLIST;

// 0x801429F8
TModeComing* TModeComing::create(TModeAllocator* allocator)
{
	return new (allocator) TModeComing;
}

// 0x80142A30
TModeComing::TModeComing() { }

// 0x80142A54
TModeComing::~TModeComing() { }

// 0x80142A7C
int TModeComing::init()
{
	GXSetCopyClear(sModeComingClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	comingLoad();
	objClearLayers(&gMainTask);
	createEditCameraCtrl();
	camSetDrawView(0);
	setViewport(0);
	mComing = comingCreate();
	if (mComing != 0) {
		return 1;
	}
	return 0;
}

// 0x80142B00
int TModeComing::shutdown()
{
	objClearLayers(&gMainTask);
	comingFree();
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	return 1;
}

// 0x80142B3C
void TModeComing::update()
{
	gjBeforeRender();
	if (padGet(0)->mPressed & 0x100) {
		_08 = 0;
		return;
	}
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_NONE);
	if ((int)mComing->mDone != 0) {
		_08 = 0;
		return;
	}
	gMainTask.draw();
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();
	gjPrintFlush();
	gjDoneRender();
	gjSwapBuffers();
}
