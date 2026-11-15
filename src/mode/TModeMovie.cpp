#include "mode/TModeMovie.h"
#include "core/TMainTask.h"
#include "Ninja/ninja.h"
#include "core/Pad.h"
#include "engine/ShadowTex.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "camera/TCameraCtrl.h"
#include "engine/Viewport.h"
#include "engine/DrawTask.h"
#include "object/TObjLauncher.h"
#include "core/Mem.h"
#include "stage/Stage.h"

TModeMovie* gModeMovie;

extern GXColor sMovieClearColor;

static const char* sMovieNames[MOVIE_COUNT] = {
	"mv_demo_movie.sfd",  "mv_demo_play.sfd",   "mv_opening.sfd",     "mv_ending.sfd",      "mv_boss_clear1.sfd", "mv_boss_clear2.sfd",
	"mv_boss_clear3.sfd", "mv_boss_clear4.sfd", "mv_boss_clear5.sfd", "mv_boss_clear6.sfd", "mv_boss_clear7.sfd", "mv_staffroll.sfd",
};

static inline int clampMovie(int movie)
{
	return (movie < 0) ? 0 : ((movie > MOVIE_COUNT - 1) ? MOVIE_COUNT - 1 : movie);
}

// 0x80151CBC
TModeMovie* TModeMovie::create(TModeAllocator* allocator)
{
	return new (allocator) TModeMovie;
}

// 0x80151CF4
void movieModeCheckDisc()
{
	movieCheckDisc();
}

// 0x80151D14
TModeMovie::TModeMovie()
{
	mMovie     = MOVIE_DEMO_MOVIE;
	mFlags     = 0;
	gModeMovie = this;
}

// 0x80151D48
TModeMovie::~TModeMovie()
{
	gModeMovie = NULL;
}

// 0x80151D78
void TModeMovie::loadFiles()
{
	movieLoad();
}

// 0x80151D98
void TModeMovie::freeFiles()
{
	movieFree();
}

// 0x80151DB8
int TModeMovie::init()
{
	GXSetCopyClear(sMovieClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	movieStart();
	objClearLayers(&gMainTask);
	createEditCameraCtrl();
	camSetDrawView(0);
	setViewport(0);
	int movie = mMovie;
	if (movie < 0) {
		movie = 0;
	} else if (movie > MOVIE_COUNT - 1) {
		movie = MOVIE_COUNT - 1;
	}
	mMovie = movie;
	moviePlay(sMovieNames[mMovie]);
	return 1;
}

// 0x80151E68
int TModeMovie::shutdown()
{
	objClearLayers(&gMainTask);
	movieStop();
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	return 1;
}

// 0x80151EA4
void TModeMovie::setMovie(int movie, int skippable)
{
	mMovie = clampMovie(movie);
	if (skippable) {
		mFlags |= MOVIE_MODE_SKIPPABLE;
	}
}

// 0x80151EE0
const char* movieName(int movie)
{
	return sMovieNames[clampMovie(movie)];
}

// 0x80151F14
void TModeMovie::update()
{
	gjBeforeRender();
	if (mFlags & MOVIE_MODE_SKIPPED) {
		_08 = 1;
		moviePause();
	} else if ((mFlags & MOVIE_MODE_SKIPPABLE) && (padGet(0)->mPressed & PAD_BUTTON_START)) {
		mFlags |= MOVIE_MODE_SKIPPED;
	}
	if (movieIsDone()) {
		_08 = 0;
	}
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_BACK);
	gMainTask.draw();
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();
	movieModeDrawFrame();
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}

// 0x80151FF4
// The quad is at a z of -10 so the positions are -10 times the screen's 0-640 by 16-464
void movieModeDrawFrame()
{
	u8* frame = movieGetFrame();
	if (frame != NULL) {
		movieDecode();
		GXInvalidateTexAll();
		gjStartVertex2D();
		gjStartVertexTexOpaque();
		GXTexObj tex;
		GXInitTexObj(&tex, frame, 640, 448, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GXLoadTexObj(&tex, GX_TEXMAP0);
		f32 left   = 0.0f;
		f32 top    = -160.0f;
		f32 z      = -10.0f;
		f32 right  = -6400.0f;
		f32 one    = 1.0f;
		f32 bottom = -4640.0f;
		GXBegin(GX_QUADS, GX_VTXFMT4, 4);
		GXPosition3f32(left, top, z);
		GX_FIFO_U32 = 0xFFFFFFFF;
		GXTexCoord2f32(left, left);
		GXPosition3f32(right, top, z);
		GX_FIFO_U32 = 0xFFFFFFFF;
		GXTexCoord2f32(one, left);
		GXPosition3f32(right, bottom, z);
		GX_FIFO_U32 = 0xFFFFFFFF;
		GXTexCoord2f32(one, one);
		GXPosition3f32(left, bottom, z);
		GX_FIFO_U32 = 0xFFFFFFFF;
		GXTexCoord2f32(left, one);
	}
}
