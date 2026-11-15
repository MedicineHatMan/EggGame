#include "ui/StaffRoll.h"
#include "mode/TModeMovie.h"
#include "core/TMainTask.h"
#include "Ninja/ninja.h"
#include "ui/Font.h"
#include "engine/ShadowTex.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "camera/TCameraCtrl.h"
#include "engine/Viewport.h"
#include "object/TObjLauncher.h"
#include "engine/DrawTask.h"
#include "core/Mem.h"
#include "stage/Stage.h"
#include "mode/TModeStaffRoll.h"
struct NJS_QUAD_TEXTURE;

/** A screen rectangle with texture coordinates for njDrawQuadTexture */
struct NjQuad {
	f32 x1, y1, x2, y2; // _00
	f32 u1, v1, u2, v2; // _10
};

// 0x801C75FC
TModeStaffRoll* TModeStaffRoll::create(TModeAllocator* allocator)
{
	return new (allocator) TModeStaffRoll;
}

// 0x801C7634
TModeStaffRoll::TModeStaffRoll()
{
}

// 0x801C7658
TModeStaffRoll::~TModeStaffRoll()
{
}

// 0x801C7680
void TModeStaffRoll::loadFiles()
{
	loadPrd("k_staff_roll.prd");
	movieLoad();
}

// 0x801C76AC
void TModeStaffRoll::freeFiles()
{
	movieFree();
	TMode::freeFiles();
}

// 0x801C76E0
int TModeStaffRoll::init()
{
	GXSetCopyClear(*(GXColor*)&sModeStaffRollClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	movieStart();
	staffRollLoad();
	objClearLayers(&gMainTask);
	fontReset();
	fontCreateTask();
	createEditCameraCtrl();
	camSetDrawView(0);
	setViewport(0);
	moviePlay(movieName(MOVIE_STAFFROLL));
	staffRollCreate();
	return 1;
}

// 0x801C7764
int TModeStaffRoll::shutdown()
{
	objClearLayers(&gMainTask);
	staffRollFree();
	movieStop();
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	return 1;
}

// 0x801C77A4
void TModeStaffRoll::update()
{
	gjBeforeRender();
	if (movieIsDone()) {
		_08 = 0;
	}
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_BACK);
	gMainTask.draw();
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	runDrawTasks(0x20);
	runDrawTasks(0x21);
	sortDrawTasks(0x22);
	runDrawTasks(0x22);
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();

	// The movie frame full screen behind everything
	u8* frame = movieGetFrame();
	if (frame != NULL) {
		movieDecode();
		GXInvalidateTexAll();
		GXTexObj tex;
		GXInitTexObj(&tex, frame, 640, 448, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GXLoadTexObj(&tex, GX_TEXMAP0);
		njQuadTextureStart(0);
		njSetQuadTextureColor(0xFFFFFFFF);
		NjQuad quad;
		quad.x1 = 0.0f;
		quad.x2 = 640.0f;
		quad.y1 = 16.0f;
		quad.y2 = 464.0f;
		quad.u1 = 0.0f;
		quad.v1 = 0.0f;
		quad.u2 = 1.0f;
		quad.v2 = 1.0f;
		njDrawQuadTexture((const NJS_QUAD_TEXTURE*)&quad, 0.05f);
		njQuadTextureEnd();
	}

	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}
