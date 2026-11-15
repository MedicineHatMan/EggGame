#include "mode/TModeTvConfig.h"
#include "io/LangFile.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "ui/TSimpleText.h"
#include "ui/TAeObjMsgWindow.h"
#include "core/TMainTask.h"
#include "engine/Viewport.h"
#include "ui/Font.h"
#include "core/Pad.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "camera/TCameraCtrl.h"
#include "engine/DrawTask.h"
#include "object/TObjLauncher.h"
#include "Dolphin/os.h"
#include "stage/Stage.h"
#include "sound/SoundCtrl.h"

static BinFile* sTvConfigFile; // 0x802C5828
static u32 sTvConfigCount;     // 0x802C582C
static char** sTvConfigText;   // 0x802C5830

// 0x8016BC88
TModeTvConfig* TModeTvConfig::create(TModeAllocator* allocator)
{
	return new (allocator) TModeTvConfig;
}

// 0x8016BCC0
void TModeTvConfig::loadFiles()
{
	loadPrd("k_tv_config.prd");
}

// 0x8016BCE8
int TModeTvConfig::init()
{
	Vec3f at;
	Vec3f box;
	Vec3f glyph;
	Vec3f at2;
	Vec3f box2;
	Vec3f glyph2;
	GXSetCopyClear(*(GXColor*)&sModeTvConfigClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	loadMsgWindow();
	objClearLayers(&gMainTask);
	createCameraCtrl();
	camSetDrawView(0);
	setViewport(0);
	fontCreateTask();
	loadText();

	glyph.x = 1.0f;
	glyph.y = 1.0f;
	glyph.z = 1.0f;
	box.x   = 80.0f;
	box.y   = 24.0f;
	box.z   = -2.0f;
	at.x    = 471.0f;
	at.y    = 319.0f;
	at.z    = -2.0f;
	mTitle  = createSimpleTextOnTask(1, -1, &at, &box, &glyph, sTvConfigText[3]);
	if (mTitle != NULL) {
		mTitle->_A8 = 1;
	}

	glyph2.x  = 1.0f;
	glyph2.y  = 1.0f;
	glyph2.z  = 1.0f;
	box2.x    = 80.0f;
	box2.y    = 24.0f;
	box2.z    = -2.0f;
	at2.x     = 471.0f;
	at2.y     = 373.0f;
	at2.z     = -2.0f;
	mSubtitle = createSimpleTextOnTask(1, -1, &at2, &box2, &glyph2, sTvConfigText[4]);
	if (mSubtitle != NULL) {
		mSubtitle->_A8 = 1;
	}

	ask(sTvConfigText[0]);
	return 1;
}

// 0x8016BE60
int TModeTvConfig::shutdown()
{
	if (mTitle != NULL) {
		mTitle->mFlags |= 1;
		mTitle = NULL;
	}
	if (mSubtitle != NULL) {
		mSubtitle->mFlags |= 1;
		mSubtitle = NULL;
	}
	if (mText != NULL) {
		mText->mFlags |= 1;
		mText = NULL;
	}
	if (mWindow != NULL) {
		mWindow->mFlags |= 1;
		mWindow = NULL;
	}
	if (sTvConfigFile != NULL) {
		releaseResource(sTvConfigFile);
		sTvConfigFile = NULL;
	}
	objClearLayers(&gMainTask);
	freeMsgWindow();
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	return 1;
}

// 0x8016BF34
// Turns the message offsets into pointers
void TModeTvConfig::loadText()
{
	u32* data;
	u32 text;
	int i;

	loadSizedResource((const char*)getLangFileName("mes_tv_config.bin"), &sTvConfigFile);
	data = (u32*)sTvConfigFile->mData;
	text = *data;
	text += (u32)data++;
	u32 size = *data++;
	sTvConfigText = (char**)data;
	sTvConfigCount = size >> 2;
	for (i = 0; i < (int)sTvConfigCount; i++) {
		sTvConfigText[i] += text + 8;
	}
}

// 0x8016BFBC
TModeTvConfig::TModeTvConfig()
{
	mState    = 0;
	mTimer    = 600;
	mText     = NULL;
	mSubtitle = NULL;
	mTitle    = NULL;
	mPrompt   = 0;
	mChoice   = 0;
	mWindow   = NULL;
}

// 0x8016C008
TModeTvConfig::~TModeTvConfig() { }

// 0x8016C030
void TModeTvConfig::update()
{
	gMainTask.draw();
	step();
	gjBeforeRender();
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_NONE);
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	runDrawTasks(0x3E);
	runDrawTasks(0x3F);
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}

// 0x8016C0B8
void TModeTvConfig::step()
{
	switch (mState) {
	case 0:
		if (answered() || mTimer == 0) {
			closePrompt();
			mState = 1;
		}
		if (mTimer != 0) {
			mTimer--;
		}
		break;
	case 1:
		if (closed()) {
			if (mChoice == 0) {
				setProgressive();
				mTimer = 100;
				mState = 2;
			} else {
				OSSetProgressiveMode(0);
				say(sTvConfigText[2]);
				mState = 3;
			}
		}
		break;
	case 2:
		if (mTimer != 0 && --mTimer == 0) {
			say(sTvConfigText[1]);
			mState = 3;
		}
		break;
	case 3:
		if (answered()) {
			_08 = 0;
		}
		break;
	}
}

// 0x8016C1F0
void TModeTvConfig::setProgressive()
{
	OSSetProgressiveMode(1);
	_gj_rmode_ = &GXNtsc480Prog;
	VIConfigure(&GXNtsc480Prog);
	VIFlush();
	VIWaitForRetrace();
	VIWaitForRetrace();
}

// 0x8016C230
void TModeTvConfig::clearPrompt()
{
	mPrompt = 0;
	mChoice = 0;
	if (mText != NULL) {
		mText->mHold = 1;
		mText        = NULL;
	}
	closeWindow();
}

// 0x8016C274
void TModeTvConfig::closeWindow()
{
	TAeObjMsgWindow* window = mWindow;
	if (window != NULL) {
		window->mDieOnClose = TRUE;
		window->close();
		mWindow = NULL;
	}
	hideTitles();
}

// 0x8016C2C4
void TModeTvConfig::closePrompt()
{
	if (mText != NULL) {
		mText->closeText();
	}
	closeWindow();
}

// 0x8016C304
void TModeTvConfig::ask(void* text)
{
	clearPrompt();
	mPrompt = 1;
	if (mText == NULL) {
		mText = createSimpleTextByKind(gTL_07, text, -1);
		if (mText != NULL) {
			mText->_A8 = 1;
		}
	}
	if (mWindow == NULL) {
		mWindow = createMsgWindow(gTL_07, 9);
		if (mWindow != NULL) {
			mWindow->setPrompt(3);
			mWindow->setCursor(mChoice);
		}
	}
	showTitles();
}

// 0x8016C3B4
void TModeTvConfig::say(void* text)
{
	clearPrompt();
	mPrompt = 2;
	if (mText == NULL) {
		mText = createSimpleTextByKind(gTL_07, text, -1);
		if (mText != NULL) {
			mText->_A8 = 1;
		}
	}
}

// 0x8016C420
// Moves the cursor on a question and returns 1 once it is answered or once a message is read
int TModeTvConfig::answered()
{
	int done = 0;
	switch (mPrompt) {
	case 0:
	default:
		done = 1;
		break;
	case 1:
		if (pressedDir(4)) {
			mChoice++;
			{
				int v = mChoice;
				if (v >= 2) {
					v = 0;
				}
				mChoice = v;
			}
			sePlay(0x1B);
		} else if (pressedDir(8)) {
			mChoice--;
			{
				int v = mChoice;
				if (v < 0) {
					v = 1;
				}
				mChoice = v;
			}
			sePlay(0x1B);
		}
		if (mWindow != NULL) {
			mWindow->setCursor(mChoice);
		}
		if (pressedB()) {
			sePlay(0x20);
			done    = 1;
			mChoice = 1;
		} else if (pressedA()) {
			sePlay(0x1C);
			done = 1;
		}
		break;
	case 2:
		if (mText != NULL && mText->mState == 3) {
			done = 1;
		}
		break;
	}
	return done;
}

// 0x8016C568
int TModeTvConfig::closed()
{
	int textDone = mText != NULL ? (mText->mState == 3 ? 1 : 0) : 1;
	int windowDone = mWindow != NULL ? (u8)(mWindow->mState == TAeObjMsgWindow::MSGWIN_CLOSED) : 1;
	if (textDone && windowDone) {
		return 1;
	}
	return 0;
}

// 0x8016C5D0
int TModeTvConfig::pressedA()
{
	return padGet(0)->mPressed & 0x100;
}

// 0x8016C5FC
int TModeTvConfig::pressedB()
{
	return padGet(0)->mPressed & 0x200;
}

// 0x8016C628
// Any one direction takes in both the pad and the stick bit for it
int TModeTvConfig::pressedDir(u32 dir)
{
	if (dir & 0x200004) {
		dir |= 0x200004;
	} else if (dir & 0x100008) {
		dir |= 0x100008;
	} else if (dir & 0x800002) {
		dir |= 0x800002;
	} else if (dir & 0x400001) {
		dir |= 0x400001;
	}
	return dir & padGet(0)->mRepeat;
}

// 0x8016C6CC
void TModeTvConfig::showTitles()
{
	if (mTitle != NULL) {
		mTitle->mFlags &= ~0x10;
	}
	if (mSubtitle != NULL) {
		mSubtitle->mFlags &= ~0x10;
	}
}

// 0x8016C700
void TModeTvConfig::hideTitles()
{
	if (mTitle != NULL) {
		mTitle->mFlags |= 0x10;
	}
	if (mSubtitle != NULL) {
		mSubtitle->mFlags |= 0x10;
	}
}
