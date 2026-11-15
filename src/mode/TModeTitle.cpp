#include "mode/TModeMovie.h"
#include "mode/TModeTitle.h"
#include "camera/TCamera.h"
#include "engine/Landscape.h"
#include "core/Pad.h"
#include "ui/TAeObjTitle.h"
#include "core/TMainTask.h"
#include "object/TObjTitleHelios.h"
#include "engine/ShadowTex.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "ui/TFade.h"
#include "io/NinjaFile.h"
#include "ui/NowLoading.h"
#include "ui/TitleObjArc.h"
#include "stage/TStageTitle.h"
#include "sound/SoundCtrl.h"
#include "camera/TCameraCtrl.h"
#include "engine/Viewport.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "stage/Stage.h"
#include "object/TObject.h"

s8 gTitleScene;
int gTitleMovie; // 0x802C4C5C which attract movie plays next

typedef char TModeTitle_size_check[(sizeof(TModeTitle) == 0x2C) ? 1 : -1];

// 0x80014AA4
TModeTitle* TModeTitle::create(TModeAllocator* allocator)
{
	return new (allocator) TModeTitle;
}

// 0x80014ADC
TModeTitle::TModeTitle()
{
	mStage   = 0;
	mInMovie = 0;
	mFlags   = 0;
}

// 0x80014B10
TModeTitle::~TModeTitle()
{
}

// 0x80014B38
void TModeTitle::nextScene()
{
	u8 scene    = gTitleScene + 1;
	gTitleScene = scene;
	int value   = (s8)scene;
	if (value >= 5 || value < 0) {
		gTitleScene = 0;
	}
}

// 0x80014B68
void TModeTitle::createModel()
{
	void* object = *(void**)getTitleObjScene();
	mModel       = createCamPath(0, 3, object, *getTitleObjMotion(0), 0, 1.0f, 1.0f);
	if (mModel != 0) {
		((TCamera*)mModel)->setCut();
	}
}

// 0x80014BD4
// Start during the first 600 frames jumps the scene to frame 600
void TModeTitle::skipIntro()
{
	if (mTimer > 1200) {
		int skipped = mTimer - 1200;
		if (mModel != 0) {
			mModel->setFrame(600.0f);
		}
		if (mHelios != 0) {
			mHelios->setPace(skipped);
		}
		f32 blend = skipped / 1800;
		switch (gTitleScene) {
		case 2:
			setLandscapeBlend(blend, 0);
			setLandscapeBlend(1.0f, mTimer);
			break;
		case 3:
			break;
		case 4:
			setLandscapeBlend(1.0f - blend, 0);
			setLandscapeBlend(0.0f, mTimer);
			break;
		}
		if (mLogo != 0) {
			mLogo->showPrompt();
		}
		mTimer = 1200;
	}
}

// 0x80014CD0
void TModeTitle::loadFiles()
{
	nowLoadingDisable();
	loadPrd("k_stage_common.prd");
	loadPrd("k_title.prd");
	movieLoad();
}

// 0x80014D20
void TModeTitle::freeFiles()
{
	movieFree();
	TMode::freeFiles();
}

// 0x80014D54
int TModeTitle::init()
{
	GXSetCopyClear(*(GXColor*)&sModeTitleClearColor, 0xFFFFFF);
	movieStart();
	const char* name = "title";
	mStage           = createTitleStage();
	mStage->load(name);
	mStage->loadAssets();
	loadAeTitle();
	return startTitle();
}

// 0x80014DD0
void TStage::loadAssets()
{
}

// 0x80014DD4
int TModeTitle::shutdown()
{
	int result;
	if (mInMovie == 0) {
		result = endTitle();
	} else {
		result = endMovie();
	}
	freeAeTitle();
	mStage->freeAssets();
	mStage->unload();
	delete mStage;
	movieStop();
	return result;
}

// 0x80014E60
void TStage::freeAssets()
{
}

// 0x80014E64
void TModeTitle::update()
{
	if (mFlags & 1) {
		mFlags &= ~1;
		switchPart();
	}
	if (mInMovie == 0) {
		updateTitle();
	} else {
		updateMovie();
	}
}

// 0x80014EC4
void TModeTitle::requestSwitch()
{
	mFlags |= 1;
}

// 0x80014ED4
void TModeTitle::switchPart()
{
	if (mInMovie == 0) {
		endTitle();
		mInMovie = 1;
		startMovie();
	} else {
		endMovie();
		mInMovie = 0;
		startTitle();
	}
}

// 0x80014F34
int TModeTitle::startTitle()
{
	mTimer = 1800;
	mState = 0;
	objClearAllLayers(&gMainTask);
	mStage->start(1);
	mStage->vfn_14();
	mLogo = createAeTitle();
	createModel();
	mHelios = createTObjTitleHelios(gTL_07);
	switch (gTitleScene) {
	case 0:
	case 1:
		setLandscapeBlend(0.0f, 0);
		break;
	case 2:
		setLandscapeBlend(0.0f, 0);
		setLandscapeBlend(1.0f, 1800);
		break;
	case 3:
		setLandscapeBlend(1.0f, 0);
		break;
	case 4:
		setLandscapeBlend(1.0f, 0);
		setLandscapeBlend(0.0f, 1800);
		break;
	}
	if (mFlags & 2) {
		skipIntro();
	}
	return 1;
}

// 0x80015054
void TStage::vfn_14()
{
}

// Deletes and clears The callers check for null first as well which leaves the doubled branch in the DOL
template <typename T>
inline void deleteObj(T*& obj)
{
	if (obj != 0) {
		delete obj;
		obj = 0;
	}
}

// 0x80015058
int TModeTitle::endTitle()
{
	bgmStop();
	if (mHelios != 0) {
		deleteObj(mHelios);
	}
	if (mModel != 0) {
		deleteObj(mModel);
	}
	if (mLogo != 0) {
		deleteObj(mLogo);
	}
	nextScene();
	objClearAllLayers(&gMainTask);
	return 1;
}

// 0x8001512C
void TModeTitle::updateTitle()
{
	if (debugEditActive() == 0) {
		switch (mState) {
		case 0:
			bgmPlay(0);
			mState = 1;
		case 1:
			if (mTimer == 0) {
				requestSwitch();
				return;
			}
			mTimer--;
			if (padGet(0)->mPressed & 0x1000) {
				if (mTimer > 1200) {
					skipIntro();
				} else {
					sePlay(0x1C);
					if (mLogo != 0) {
						mLogo->pressed();
					}
					mState = 2;
					mTimer = 0;
				}
			}
			break;
		case 2:
			mTimer++;
			if (mTimer == 30) {
				fadeToWhite();
			} else if (mTimer >= 90) {
				bgmStop();
				_08 = 0;
			}
			break;
		}
	}
	gjBeforeRender();
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_NONE);
	mStage->run(3);
	objPrintTree(&gMainTask);
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}

// 0x800152A0
int TModeTitle::startMovie()
{
	objClearLayers(&gMainTask);
	createEditCameraCtrl();
	camSetDrawView(0);
	setViewport(0);
	mFlags &= ~2;
	int movie;
	if (gTitleMovie == 0) {
		gTitleMovie++;
		movie = MOVIE_DEMO_MOVIE;
	} else {
		gTitleMovie = 0;
		movie       = MOVIE_DEMO_PLAY;
	}
	moviePlay(movieName(movie));
	return 1;
}

// 0x80015328
int TModeTitle::endMovie()
{
	objClearLayers(&gMainTask);
	moviePause();
	return 1;
}

// 0x80015358
void TModeTitle::updateMovie()
{
	gjBeforeRender();
	if (padGet(0)->mPressed & 0x1000) {
		moviePause();
		mFlags |= 2;
		requestSwitch();
	}
	if (movieIsDone()) {
		requestSwitch();
	}
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_BACK);
	gMainTask.draw();
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	objPrintTree(&gMainTask);
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();
	movieModeDrawFrame();
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}
