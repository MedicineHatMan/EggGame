#include "mode/TModeLicense.h"
#include "io/LangFile.h"
#include "io/Resource.h"
#include "ui/TAeObjLicense.h"
#include "core/TMainTask.h"
#include "engine/Viewport.h"
#include "Ninja/ninja.h"
#include "engine/ShadowTex.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "camera/TCameraCtrl.h"
#include "engine/DrawTask.h"
#include "core/Mem.h"
#include "ui/AeArc.h"
#include "stage/Stage.h"
struct NJS_TEXLIST;

// 0x80140600
TModeLicense* TModeLicense::create(TModeAllocator* allocator)
{
	return new (allocator) TModeLicense;
}

// 0x80140638
TModeLicense::TModeLicense()
{
}

// 0x8014065C
TModeLicense::~TModeLicense()
{
}

// 0x80140684
void TModeLicense::loadFiles()
{
	loadPrd("k_license.prd");
}

// 0x801406AC
int TModeLicense::init()
{
	GXSetCopyClear(*(GXColor*)&sModeLicenseClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	licenseLoad();
	objClearLayers(&gMainTask);
	createEditCameraCtrl();
	camSetDrawView(0);
	setViewport(0);
	mLicense = licenseCreate();
	if (mLicense != 0) {
		return 1;
	}
	return 0;
}

// 0x80140730
int TModeLicense::shutdown()
{
	objClearLayers(&gMainTask);
	licenseFree();
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	return 1;
}

// 0x8014076C
void TModeLicense::update()
{
	gjBeforeRender();
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_NONE);
	if (mLicense->mDone != 0) {
		_08 = 0;
	} else {
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
}
