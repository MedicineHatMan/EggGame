#include "ui/TAeObjMainMenu.h"
#include "ui/TextVec.h"
#include "ui/MainMenu.h"
#include "io/LangFile.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#define createTMainMenuModel createTMainMenuModel_int
#include "ui/TMainMenuModel.h"
#undef createTMainMenuModel
extern "C" TMainMenuModel* createTMainMenuModel(TObject*, const Vec3f*, const int*, s8, u16);
#include "ui/TSimpleText.h"
#include "ui/TAeObjMsgWindow.h"
#include "ui/Font.h"
#include "engine/ShadowTex.h"
#include "engine/Viewport.h"
#include "engine/DrawTask.h"
#include "sound/SoundCtrl.h"
#include "stage/TStageSelectModel.h"
#include "io/CardManager.h"
#include "object/TObjLauncher.h"
#include "ui/TFade.h"
#include "core/dolphin.h"
#include "camera/TCamera.h"
#include "camera/TCameraCtrl.h"
#include "core/Mem.h"
#include "Dolphin/os.h"
#include "stage/Stage.h"
#include "Ninja/ninja.h"
#include "core/TMainTask.h"
#include "io/SaveData.h"
#include "io/MemCard.h"
#include "core/Pad.h"
struct Vec3f;
struct TMainTask;
struct TCamFix;
struct TCameraCtrl;

// Front-end pages advance runs the current page's update and only then applies setState
// exit the old page enter the new one then draw
// TModeStage numbers update and exit the other way round
// _08 is the result the mode switcher waits on
// The page bodies that read the pad live in updateRoot root updateFile file updateSound sound and the setting at
// padSetVibEnabled and runBox the message box
// mSelect mFileStep mSoundStep or mBoxKind

enum MenuPhase {
	MENU_ENTER  = 0,
	MENU_UPDATE = 1,
	MENU_DRAW   = 2,
	MENU_EXIT   = 3,
};

struct MenuPad {
	u32 mHold; // _00
	u8 _04[4];
	u32 mTrigger; // _08
	u8 _0C[8];
	u32 mRepeat; // _14
};

inline Vec3f vec3f(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

static TextVec sTextGlyph = textVec(0.91f, 0.91f, 0.91f);
static TextVec sTextBox   = textVec(396.0f, 44.0f, 0.0f);
static TextVec sTextAt    = textVec(44.0f, 41.0f, -2.0f);
// The three captions of the file page one row per caption one column per file
static TextVec sCaptionAt0[3] = { textVec(159.0f, 168.0f, -2.0f), textVec(318.0f, 168.0f, -2.0f), textVec(477.0f, 168.0f, -2.0f) };
static TextVec sCaptionAt1[3] = { textVec(159.0f, 208.0f, -2.0f), textVec(318.0f, 208.0f, -2.0f), textVec(477.0f, 208.0f, -2.0f) };
static TextVec sCaptionAt2[3] = { textVec(159.0f, 248.0f, -2.0f), textVec(318.0f, 248.0f, -2.0f), textVec(477.0f, 248.0f, -2.0f) };
// "Yes" and "No" mText[4] mText[5] under each file then in the message box
static TextVec sYesAt[4] = {
	textVec(159.0f, 181.0f, -2.0f),
	textVec(318.0f, 181.0f, -2.0f),
	textVec(477.0f, 181.0f, -2.0f),
	textVec(512.0f, 320.0f, -2.0f),
};
static TextVec sNoAt[4] = {
	textVec(159.0f, 234.0f, -2.0f),
	textVec(318.0f, 234.0f, -2.0f),
	textVec(477.0f, 234.0f, -2.0f),
	textVec(512.0f, 373.0f, -2.0f),
};
// The three lines of the sound page
static TextVec sLineAt0    = textVec(127.0f, 271.0f, -2.0f);
static TextVec sLineAt1    = textVec(319.0f, 271.0f, -2.0f);
static TextVec sLineAt2    = textVec(511.0f, 271.0f, -2.0f);
static TextVec sLeftAt     = textVec(210.0f, 273.0f, -2.0f);
static TextVec sRightAt    = textVec(402.0f, 273.0f, -2.0f);
static TextVec sTabAt      = textVec(570.0f, 36.0f, -2.0f);
static TextVec sTab2At     = textVec(570.0f, 73.0f, -2.0f);
static TextVec sHelpAt     = textVec(57.0f, 341.0f, -2.0f);
static TextVec sHelpBox    = textVec(528.0f, 66.0f, 0.0f);
static TextVec sIconPos[3] = { textVec(-20.0f, -2.0f, -60.0f), textVec(0.0f, -2.0f, -60.0f), textVec(20.0f, -2.0f, -60.0f) };
static TextRot sIconRot[3] = { textRot(0, 0xC00, 0), textRot(0, 0, 0), textRot(0, 0xF400, 0) };
static TextVec sCenterPos  = textVec(0.0f, 0.0f, 20.0f);
static TextRot sCenterRot  = textRot(0, 0, 0);

// One string record in mes_main_menu.bin
// Sets the object's retire bit and drops the pointer
template <typename T>
inline void freeObj(T*& obj)
{
	if (obj != 0) {
		((TObject*)obj)->mFlags |= 1;
		obj = 0;
	}
}

// 0x8016FE50
TModeMainMenu* TModeMainMenu::create(TModeAllocator* allocator)
{
	return new (allocator) TModeMainMenu;
}

// 0x8016FE88
void TModeMainMenu::loadFiles()
{
	loadPrd("k_main_menu.prd");
}

// 0x8016FEB0
int TModeMainMenu::init()
{
	GXSetCopyClear(*(GXColor*)&sMainMenuClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	loadMsgWindow();
	loadAeMainMenu();
	loadMainMenuModel();
	loadStageSelectText();
	cardInit();
	gjSetAmbient(0.5f, 0.5f, 0.6f);
	gjSetLightSwitch(0, 1);
	gjSetLightVector(0, 1.0f, -1.0f, -1.0f);
	gjSetLightColor(0, 1.0f, 1.0f, 1.0f);
	gjLoadLight(0);
	objClearLayers(&gMainTask);
	createCameraCtrl();

	f32 pos[3];
	f32 dir[3];
	dir[0] = 0.0f;
	dir[1] = 0.0f;
	dir[2] = 0.0f;
	pos[0] = 0.0f;
	pos[1] = 0.0f;
	pos[2] = -100.0f;
	mLight = createTCamFix(0, 2, (const Vec3f*)pos, (const Vec3f*)dir);

	camSetDrawView(0);
	setViewport(0);
	cardCreate();
	fontCreateTask();
	createAeMainMenu();
	buildText();
	bgmPlay(1);

	switch ((s8)mOpened) {
	case 0:
		setState(1);
		break;
	case 1:
		setState(4);
		break;
	}
	return 1;
}

// 0x80170010
int TModeMainMenu::shutdown()
{
	if (sMainMenuFile != 0) {
		releaseResource(sMainMenuFile);
		sMainMenuFile = 0;
	}

	freeObj(mFade);
	freeObj(mText[0]);
	freeObj(mText[1]);
	freeObj(mText[2]);
	freeObj(mText[3]);
	freeObj(mText[4]);
	freeObj(mText[5]);
	for (int i = 0; i < 3; i++) {
		freeObj(mLine[i]);
		freeObj(mCaption[i]);
	}
	freeObj(mBox);
	for (int i = 3; i >= 0; i--) {
		freeObj(mIcon[i]);
	}
	freeObj(mLight);
	freeObj(mChoiceWindow);
	freeObj(mYesNoWindow);

	objClearLayers(&gMainTask);
	fontReset();
	cardShutdown();
	freeStageSelectText();
	freeMainMenuModel();
	freeAeMainMenu();
	freeMsgWindow();
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	bgmStop();
	return 1;
}

// 0x8017027C
TModeMainMenu::TModeMainMenu()
{
	mOpened     = 0;
	mFlow       = 0;
	mPage       = 0;
	mState      = 0;
	mBackState  = 0;
	mNextState  = 0;
	mSelect     = 0;
	mCopyTo     = 0;
	mFile       = 0;
	_30         = 0;
	_34         = 0;
	mFileStep   = 0;
	mSlot       = 0;
	mSoundStep  = 0;
	mSoundMode  = 0;
	_48         = 0;
	_4C         = 0;
	mBoxKind    = 0;
	mChoice     = 0;
	mBoxTimer   = 0;
	_5C         = 0;
	mCursor     = 0;
	mPrevCursor = mCursor;
	mFrame      = 0;
	mPressedA   = 0;
	mFade       = 0;
	mFadeTimer  = 0;

	sMainMenuCount   = 0;
	sMainMenuEntries = 0;

	mText[0] = 0;
	mText[1] = 0;
	mText[2] = 0;
	mText[3] = 0;
	mText[4] = 0;
	mText[5] = 0;
	for (int i = 0; i < 3; i++) {
		mLine[i]    = 0;
		mCaption[i] = 0;
	}
	mBox          = 0;
	mChoiceWindow = 0;
	mYesNoWindow  = 0;
	for (int i = 0; i < 4; i++) {
		mIcon[i] = 0;
	}
	mLight = 0;
}

// 0x80170388
TModeMainMenu::~TModeMainMenu()
{
}

// 0x801703B0
void TModeMainMenu::buildText()
{
	u32* data;
	u32 text;
	int i;

	loadSizedResource(getLangFileName("mes_main_menu.bin"), &sMainMenuFile);
	data = (u32*)sMainMenuFile->mData;
	text = *data;
	text += (u32)data++;
	u32 size         = *data++;
	sMainMenuEntries = (int*)data;
	sMainMenuCount   = size >> 2;
	for (i = 0; i < sMainMenuCount; i++) {
		sMainMenuEntries[i] += text + 8;
	}

	mText[0]      = createSimpleTextOnTask(1, -1, &sTextAt, &sTextBox, &sTextGlyph, (void*)sMainMenuEntries[0]);
	mText[0]->_A9 = 1;
	mText[1]      = createSimpleTextOnTask(1, -1, &sHelpAt, &sHelpBox, &sTextGlyph, (void*)sMainMenuEntries[0]);
	mText[2]      = createSimpleTextOnTask(1, -1, &sTabAt, &sTextBox, &sTextGlyph, (void*)sMainMenuEntries[9]);
	mText[3]      = createSimpleTextOnTask(1, -1, &sTab2At, &sTextBox, &sTextGlyph, (void*)sMainMenuEntries[10]);
	mText[4]      = createSimpleTextOnTask(1, -1, &sTextAt, &sTextBox, &sTextGlyph, getStageSelectWarning(0x27));
	mText[5]      = createSimpleTextOnTask(1, -1, &sTextAt, &sTextBox, &sTextGlyph, getStageSelectWarning(0x28));

	for (int i = 0; i < 3; i++) {
		mLine[i] = createSimpleTextOnTask(1, -1, &sTextAt, &sTextBox, &sTextGlyph, (void*)sMainMenuEntries[0x10 + i]);
	}
	for (int i = 0; i < 3; i++) {
		mCaption[i] = createSimpleTextOnTask(1, -1, &sTextAt, &sTextBox, &sTextGlyph, (void*)sMainMenuEntries[6 + i]);
	}

	hideAll();
	for (int i = 0; i < 3; i++) {
		mIcon[i] = createTMainMenuModel(gTL_05, (const Vec3f*)&sIconPos[i], (const int*)&sIconRot[i], i, 0x22);
	}
	mIcon[3] = createTMainMenuModel(gTL_05, (const Vec3f*)&sCenterPos, (const int*)&sCenterRot, 3, 0x21);
	hideIcons();
}

// 0x80170638
void TModeMainMenu::update()
{
	cardUpdate();
	gMainTask.draw();
	advance();
	if (mIcon[3] != 0) {
		mIcon[3]->mAngle.v[1] += 1;
	}

	gjBeforeRender();
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_NONE);
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	runDrawTasks(0x21);
	runDrawTasks(0x3C);
	runDrawTasks(0x22);
	runDrawTasks(0x3E);
	runDrawTasks(0x3F);
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}

// 0x80170708
void TModeMainMenu::advance()
{
	mPrevCursor = mCursor;
	mCursor     = cardGetState();
	mPressedA   = ((MenuPad*)padGet(0))->mTrigger & 0x100;

	runState(mState, MENU_UPDATE);
	if (mNextState != mState) {
		runState(mState, MENU_EXIT);
		mState = mNextState;
		runState(mState, MENU_ENTER);
	}
	runState(mState, MENU_DRAW);
	mFrame++;
}

// 0x801707B4
int TModeMainMenu::pressedA()
{
	if ((int)mPressedA != 0) {
		mPressedA = 0;
		return 1;
	}
	return 0;
}

// 0x801707D8
int TModeMainMenu::pressedB()
{
	return ((MenuPad*)padGet(0))->mTrigger & 0x200;
}

// 0x80170804
int TModeMainMenu::anyButton()
{
	return ((MenuPad*)padGet(0))->mTrigger & 0x1F7F;
}

// 0x80170830
int TModeMainMenu::repeated(u32 mask)
{
	if (mask & 0x200004) {
		mask |= 0x200004;
	} else if (mask & 0x100008) {
		mask |= 0x100008;
	} else if (mask & 0x800002) {
		mask |= 0x800002;
	} else if (mask & 0x400001) {
		mask |= 0x400001;
	}
	return mask & ((MenuPad*)padGet(0))->mRepeat;
}

// 0x801708D4
void TModeMainMenu::resetRoot()
{
	mSelect = 0;
	mCopyTo = 0;
	mFile   = 0;
	_30     = 0;
	_34     = 0;
}

// 0x801708F0
int TModeMainMenu::updateRoot()
{
	switch (mSelect) {
	case 0:
		if (aeMainMenuIsReady() != 0) {
			showTitle(0);
			readFileInfos();
			mSelect = 1;
		}
		break;
	case 1:
		if (pressedA()) {
			sePlay(0x1C);
			openFileChoice(mFile);
			aeMainMenuHidePanels();
			mSelect = 2;
		} else if (pressedB()) {
			sePlay(0x20);
			mSelect = 4;
			return 1;
		}
		break;
	case 2:
		if (runBox() != 0) {
			switch (mChoice) {
			case 0:
				saveSelectSlot(mFile);
				if (OSGetSoundMode() == 0) {
					soundSetMode(0);
				} else if (saveGetSoundMode() == 1) {
					soundSetMode(2);
				} else {
					soundSetMode(1);
				}
				if (saveGetRumbleOff() == 0) {
					padSetVibEnabled(1);
				} else {
					padSetVibEnabled(0);
				}
				fadeOutRoot();
				aeMainMenuCloseRoot();
				closeChoiceWindow();
				hideAll();
				mSelect = 3;
				break;
			case 1: {
				mCopyTo++;
				int file = mCopyTo;
				if (file == mFile) {
					file++;
				}
				mCopyTo = file;
				mCopyTo = mCopyTo >= 3 ? (mFile == 0 ? 1 : 0) : mCopyTo;
				showTitle(1);
				mSelect = 5;
				break;
			}
			case 2:
				closeChoiceWindow();
				openCardChoice(mFile);
				showTitle(3);
				mSelect = 7;
				break;
			}
		} else if (pressedB()) {
			sePlay(0x20);
			closeBox();
			mSelect = 1;
		}
		break;
	case 3:
		if (fadeDone() != 0) {
			return 1;
		}
		break;
	case 4:
		break;
	case 5:
		if (pressedA()) {
			sePlay(0x1C);
			openCardChoice(mCopyTo);
			showTitle(2);
			aeMainMenuHidePanels();
			mSelect = 6;
		} else if (pressedB()) {
			sePlay(0x20);
			saveSelectSlot(mFile);
			aeMainMenuHidePanels();
			showTitle(0);
			mBoxKind = 5;
			mSelect  = 2;
		}
		break;
	case 6:
		if (runBox() != 0) {
			if (choseYes() != 0) {
				saveCopySlot(mFile, mCopyTo);
				return 1;
			}
			closeYesNoWindow();
			showTitle(1);
			mSelect = 5;
		}
		break;
	case 7:
		if (runBox() != 0) {
			if (choseYes() != 0) {
				saveClearSlot(mFile);
				return 1;
			}
			openFileChoice(mFile);
			showTitle(0);
			mSelect = 2;
		}
		break;
	}
	return 0;
}

// 0x80170C70
void TModeMainMenu::chooseFiles()
{
	FileInfo info;
	info.mWorld   = -1;
	info.mEmblems = 0;
	info.mLives   = 0;

	switch (mSelect) {
	case 0:
		if (pressedA()) {
			aeMainMenuSnapRoot();
		}
		break;
	case 1:
		if (repeated(2) != 0) {
			mFile++;
			{
				int v = mFile;
				if (v >= 3) {
					v = 0;
				}
				mFile = v;
			}
			sePlay(0x1B);
		} else if (repeated(1) != 0) {
			mFile--;
			{
				int v = mFile;
				if (v < 0) {
					v = 2;
				}
				mFile = v;
			}
			sePlay(0x1B);
		}
		readFileInfo(&info, mFile);
		aeMainMenuShowFile(mFile, &info, 0);
		break;
	case 5:
		if (repeated(2) != 0) {
			mCopyTo++;
			int file = mCopyTo;
			if (file == mFile) {
				file++;
			}
			mCopyTo = file;
			mCopyTo = mCopyTo >= 3 ? (mFile == 0 ? 1 : 0) : mCopyTo;
			sePlay(0x1B);
		} else if (repeated(1) != 0) {
			mCopyTo--;
			int file = mCopyTo;
			if (file == mFile) {
				file--;
			}
			mCopyTo = file;
			mCopyTo = mCopyTo < 0 ? (mFile == 2 ? 1 : 2) : mCopyTo;
			sePlay(0x1B);
		}
		readFileInfo(&info, mCopyTo);
		aeMainMenuShowFile(mCopyTo, &info, 0);
		break;
	case 6:
	case 7:
		break;
	}
}

// 0x80170E70
void TModeMainMenu::readFileInfos()
{
	FileInfo info;
	for (s8 i = 0; i < 3; i++) {
		readFileInfo(&info, i);
		aeMainMenuSetFileBar(i, &info);
	}
}

// 0x80170ECC
void TModeMainMenu::readFileInfo(FileInfo* info, s8 slot)
{
	saveSelectSlot(slot);
	if (saveSlotUsed(slot) != 0) {
		info->mLives = saveSlotLives(slot);
		if (info->mLives < 5) {
			info->mLives = 5;
		}
		if (saveGetKeyBit(0x31) != 0) {
			info->mWorld = 6;
		} else if (saveGetKeyBit(0x29) != 0) {
			info->mWorld = 5;
		} else if (saveGetKeyBit(0x21) != 0) {
			info->mWorld = 4;
		} else if (saveGetKeyBit(0x19) != 0) {
			info->mWorld = 3;
		} else if (saveGetKeyBit(0x11) != 0) {
			info->mWorld = 2;
		} else if (saveGetKeyBit(9) != 0) {
			info->mWorld = 1;
		} else if (saveGetKeyBit(1) != 0) {
			info->mWorld = 0;
		}

		s16 count = 0;
		for (s16 i = 0x3C; i < 0x78; i++) {
			if (saveGetKeyBit(i) != 0) {
				count++;
			}
		}
		info->mEmblems = count;
	}
}

// 0x80171028
void TModeMainMenu::resetFile()
{
	mFileStep = 0;
	mSlot     = 0;
}

// 0x80171038
int TModeMainMenu::updateFile()
{
	switch (mFileStep) {
	case 0:
		if (aeMainMenuIsReady() != 0) {
			aeMainMenuSetFileCursor(mSlot);
			showTitle(4);
			showLines();
			showFileHelp(mSlot);
			for (int i = 0; i < 3; i++) {
				if (i == mSlot) {
					mIcon[i]->setState1();
				} else {
					mIcon[i]->setIdle();
				}
			}
			showIcons();
			mFileStep = 1;
		}
		break;
	case 1: {
		int prev = mSlot;
		if (repeated(2) != 0) {
			mSlot++;
			{
				int v = mSlot;
				if (v >= 3) {
					v = 0;
				}
				mSlot = v;
			}
			sePlay(0x1B);
		} else if (repeated(1) != 0) {
			mSlot--;
			{
				int v = mSlot;
				if (v < 0) {
					v = 2;
				}
				mSlot = v;
			}
			sePlay(0x1B);
		}
		if (mSlot != prev) {
			aeMainMenuSetFileCursor(mSlot);
			showFileHelp(mSlot);
			for (int i = 0; i < 3; i++) {
				if (i == mSlot) {
					mIcon[i]->setState1();
				} else {
					mIcon[i]->setIdle();
				}
			}
		}
		if (pressedA()) {
			sePlay(0x1C);
			aeMainMenuFilesToRoot();
			aeMainMenuHideFileCursor();
			hideAll();
			mIcon[mSlot]->setState2();
			switch (mSlot) {
			case 0:
				mFileStep = 3;
				break;
			case 1:
				mFileStep = 3;
				break;
			case 2:
				aeMainMenuFilesToRoot();
				hideAll();
				mFileStep = 2;
				break;
			}
		} else if (pressedB()) {
			sePlay(0x20);
			aeMainMenuFilesToRoot();
			aeMainMenuHideFileCursor();
			hideAll();
			fadeOutFileBack();
			mSlot     = -1;
			mFileStep = 4;
		}
		break;
	}
	case 2:
		if (aeMainMenuIsReady() != 0) {
			hideIcons();
			return 1;
		}
		break;
	case 3:
		if (mIcon[mSlot]->mMotion.passedFrame(10.0f)) {
			if (mSlot == 0) {
				fadeOutFile0();
			} else {
				fadeOutFile1();
			}
			mFileStep = 4;
		}
		break;
	case 4:
		if (fadeDone() != 0) {
			if (mSlot == 0) {
				return 1;
			}
			if (mSlot == 1) {
				return 1;
			}
			_08 = 1;
		}
		break;
	}
	return 0;
}

// 0x80171370
void TModeMainMenu::resetSound()
{
	mSoundStep = 0;
	mSoundMode = 0;
	_48        = 0;
	_4C        = 0;
}

// 0x80171388
int TModeMainMenu::updateSound()
{
	switch (mSoundStep) {
	case 0:
		if (aeMainMenuIsReady() != 0) {
			aeMainMenuSetSoundRow(mSoundMode);
			showTitle(5);
			showLinePair();
			showSoundHelp(mSoundMode);
			mSoundStep = 1;
		}
		break;
	case 1: {
		int prev = mSoundMode;
		if (repeated(2) != 0) {
			mSoundMode++;
			{
				int v = mSoundMode;
				if (v >= 2) {
					v = 0;
				}
				mSoundMode = v;
			}
			sePlay(0x1B);
		} else if (repeated(1) != 0) {
			mSoundMode--;
			{
				int v = mSoundMode;
				if (v < 0) {
					v = 1;
				}
				mSoundMode = v;
			}
			sePlay(0x1B);
		}
		if (mSoundMode != prev) {
			aeMainMenuSetSoundRow(mSoundMode);
			showSoundHelp(mSoundMode);
		}
		if (pressedA()) {
			if (mSoundMode == 0) {
				openSoundChoice();
				mSoundStep = 2;
			} else {
				openVibration();
				mSoundStep = 3;
			}
		} else if (pressedB()) {
			aeMainMenuHideSound();
			aeMainMenuClearSoundRows();
			hideAll();
			mSoundStep = 4;
		}
		break;
	}
	case 2:
		if (runBox() != 0) {
			int mono = 0;
			if (mChoice == 0) {
				OSSetSoundMode(0);
				soundSetMode(0);
			} else if (mChoice == 1) {
				OSSetSoundMode(1);
				soundSetMode(1);
			} else if (mChoice == 2) {
				OSSetSoundMode(1);
				soundSetMode(2);
				mono = 1;
			}
			saveSetSoundMode(mono);
			closeBox();
			showLinePair();
			mSoundStep = 1;
		} else if (pressedB()) {
			closeBox();
			showLinePair();
			mSoundStep = 1;
		}
		break;
	case 3:
		if (runBox() != 0) {
			if (mChoice == 0) {
				padSetVibEnabled(1);
			} else {
				padSetVibEnabled(0);
			}
			saveSetRumbleOff(mChoice);
			closeBox();
			showLinePair();
			mSoundStep = 1;
		} else if (pressedB()) {
			closeBox();
			showLinePair();
			mSoundStep = 1;
		}
		break;
	case 4:
		if (fadeDone() != 0) {
			setState(6);
		}
		break;
	}
	return 0;
}

// 0x80171678
void TModeMainMenu::closeBox()
{
	mBoxKind = 0;
	mChoice  = 0;
	_5C      = 0;
	if (mBox != 0) {
		mBox->mHold = 1;
		mBox        = 0;
	}
	closeYesNoWindow();
	closeChoiceWindow();
	alphaFadeClear();
}

// 0x801716DC
void TModeMainMenu::closeYesNoWindow()
{
	TAeObjMsgWindow* window = mYesNoWindow;
	if (window != 0) {
		window->mDieOnClose = TRUE;
		window->close();
		mYesNoWindow = 0;
	}
	hideCards();
}

// 0x8017172C
void TModeMainMenu::closeChoiceWindow()
{
	TAeObjMsgWindow* window = mChoiceWindow;
	if (window != 0) {
		window->mDieOnClose = TRUE;
		window->close();
		mChoiceWindow = 0;
	}
	hideCaptions();
}

// 0x8017177C
void TModeMainMenu::openYesNo(void* message)
{
	closeBox();
	mBoxKind  = 1;
	mBoxTimer = 0;
	if (mBox == 0) {
		mBox = createSimpleTextByKind(gTL_07, message, -1);
		if (mBox != 0) {
			mBox->_A8 = 1;
		}
	}
	if (mYesNoWindow == 0) {
		mYesNoWindow = createMsgWindow(gTL_07, 9);
		if (mYesNoWindow != 0) {
			mYesNoWindow->setPrompt(3);
			mYesNoWindow->setCursor(mChoice);
		}
	}
	showCards(3);
	alphaFadeDim(0x3C);
}

// 0x80171840
void TModeMainMenu::openMessage(void* message, s8 kind)
{
	closeBox();
	mBoxKind  = 2;
	mBoxTimer = 0;
	if (mBox == 0) {
		mBox = createSimpleTextByKind(gTL_07, message, kind);
		if (mBox != 0) {
			mBox->_A8 = 1;
		}
	}
	alphaFadeDim(0x3C);
}

// 0x801718C0
void TModeMainMenu::openNotice(void* message)
{
	closeBox();
	mBoxKind  = 3;
	mBoxTimer = 0;
	if (mBox == 0) {
		mBox = createSimpleTextId2(gTL_07, message);
		if (mBox != 0) {
			mBox->_A8 = 1;
		}
	}
	alphaFadeDim(0x3C);
}

// 0x80171938
void TModeMainMenu::openTimed(void* message)
{
	closeBox();
	mBoxKind  = 4;
	mBoxTimer = 0x1E;
	if (mBox == 0) {
		mBox = createSimpleTextId2(gTL_07, message);
		if (mBox != 0) {
			mBox->_A8 = 1;
		}
	}
	alphaFadeDim(0x3C);
}

// 0x801719B0
void TModeMainMenu::openFileChoice(s8 kind)
{
	closeBox();
	mBoxKind  = 5;
	mBoxTimer = 0;
	if (mChoiceWindow == 0) {
		mChoiceWindow = createMsgWindow(gTL_07, 8);
		if (mChoiceWindow != 0) {
			mChoiceWindow->setPrompt(kind);
			mChoiceWindow->setCursor(mChoice);
			if (saveGetKeyBit(1) == 0) {
				mChoiceWindow->setMark(1);
				mChoiceWindow->setMark(2);
			}
		}
	}
	showCaptions(kind);
}

// 0x80171A60
void TModeMainMenu::openCardChoice(s8 kind)
{
	mChoice   = 1;
	_5C       = 0;
	mBoxKind  = 6;
	mBoxTimer = 0;
	if (mYesNoWindow == 0) {
		mYesNoWindow = createMsgWindow(gTL_07, 7);
		if (mYesNoWindow != 0) {
			mYesNoWindow->setPrompt(kind);
			mYesNoWindow->setCursor(mChoice);
		}
	}
	showCards(kind);
}

// 0x80171AF0
void TModeMainMenu::openSoundChoice()
{
	mChoice   = 0;
	_5C       = 0;
	mBoxKind  = 7;
	mBoxTimer = 0;
	switch (OSGetSoundMode()) {
	case 0:
		mChoice = 0;
		break;
	case 1:
		if (saveGetSoundMode() == 0) {
			mChoice = 1;
		} else {
			mChoice = 2;
		}
		break;
	}
	if (mChoiceWindow == 0) {
		mChoiceWindow = createMsgWindow(gTL_07, 8);
		if (mChoiceWindow != 0) {
			mChoiceWindow->setPrompt(1);
			mChoiceWindow->setCursor(mChoice);
		}
	}
	showCaptionRow();
	hideLinePair();
}

// 0x80171BC4
void TModeMainMenu::openVibration()
{
	mChoice   = 0;
	_5C       = 0;
	mBoxKind  = 8;
	mBoxTimer = 0;
	if (padGetVibEnabled() != 0) {
		mChoice = 0;
	} else {
		mChoice = 1;
	}
	if (mYesNoWindow == 0) {
		mYesNoWindow = createMsgWindow(gTL_07, 7);
		if (mYesNoWindow != 0) {
			mYesNoWindow->setPrompt(1);
			mYesNoWindow->setCursor(mChoice);
		}
	}
	showCaptionPair();
	hideLinePair();
}

// 0x80171C70
int TModeMainMenu::runBox()
{
	int done = 0;
	switch (mBoxKind) {
	case 0:
	default:
		done = 1;
		break;
	case 1:
		if (repeated(4) != 0) {
			mChoice++;
			{
				int v = mChoice;
				if (v >= 2) {
					v = 0;
				}
				mChoice = v;
			}
			sePlay(0x1B);
		} else if (repeated(8) != 0) {
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
		if (mYesNoWindow != 0) {
			mYesNoWindow->setCursor(mChoice);
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
		if (mBox != 0 && mBox->mState == 3) {
			done = 1;
		}
		break;
	case 3:
		if (mBox != 0 && mBox->mState == 3) {
			done = 1;
		} else if (anyButton() && mBox != 0) {
			mBox->closeText();
		}
		break;
	case 4:
		if (mBoxTimer == 0) {
			if (mBox != 0) {
				mBox->closeText();
				done = 1;
			}
		} else {
			mBoxTimer--;
		}
		break;
	case 5:
		if (saveGetKeyBit(1) != 0) {
			if (repeated(4) != 0) {
				mChoice++;
				{
					int v = mChoice;
					if (v >= 3) {
						v = 0;
					}
					mChoice = v;
				}
				sePlay(0x1B);
			} else if (repeated(8) != 0) {
				mChoice--;
				{
					int v = mChoice;
					if (v < 0) {
						v = 2;
					}
					mChoice = v;
				}
				sePlay(0x1B);
			}
			if (mChoiceWindow != 0) {
				mChoiceWindow->setCursor(mChoice);
			}
		}
		if (pressedA()) {
			sePlay(0x1C);
			done = 1;
		}
		break;
	case 6:
		if (repeated(4) != 0) {
			mChoice++;
			{
				int v = mChoice;
				if (v >= 2) {
					v = 0;
				}
				mChoice = v;
			}
			sePlay(0x1B);
		} else if (repeated(8) != 0) {
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
		if (mYesNoWindow != 0) {
			mYesNoWindow->setCursor(mChoice);
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
	case 7:
		if (repeated(4) != 0) {
			mChoice++;
			{
				int v = mChoice;
				if (v >= 3) {
					v = 0;
				}
				mChoice = v;
			}
			sePlay(0x1B);
		} else if (repeated(8) != 0) {
			mChoice--;
			{
				int v = mChoice;
				if (v < 0) {
					v = 2;
				}
				mChoice = v;
			}
			sePlay(0x1B);
		}
		if (mChoiceWindow != 0) {
			mChoiceWindow->setCursor(mChoice);
		}
		if (pressedA()) {
			sePlay(0x1C);
			done = 1;
		}
		break;
	case 8:
		if (repeated(4) != 0) {
			mChoice++;
			{
				int v = mChoice;
				if (v >= 2) {
					v = 0;
				}
				mChoice = v;
			}
			sePlay(0x1B);
		} else if (repeated(8) != 0) {
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
		if (mYesNoWindow != 0) {
			mYesNoWindow->setCursor(mChoice);
		}
		if (pressedA()) {
			sePlay(0x1C);
			done = 1;
		}
		break;
	}
	return done;
}

// 0x8017211C
int TModeMainMenu::choseYes()
{
	return mChoice == 0;
}

// 0x8017212C
void TModeMainMenu::freeFade()
{
	freeObj(mFade);
	mFadeTimer = 0;
}

// 0x80172158
void TModeMainMenu::fadeOutBack()
{
	freeFade();
	mFade      = new TWhiteFadeInOut(gTL_01, 1);
	mFadeTimer = 0x3C;
}

// 0x801721B4
void TModeMainMenu::fadeOutRoot()
{
	freeFade();
	mFade      = new TWhiteFadeInOut(gTL_01, 1);
	mFadeTimer = 0x3C;
}

// 0x80172210
void TModeMainMenu::fadeOutFileBack()
{
	freeFade();
	mFade      = new TWhiteFadeInOut(gTL_01, 1);
	mFadeTimer = 0x3C;
}

// 0x8017226C
void TModeMainMenu::fadeOutFile0()
{
	freeFade();
	mFade      = new TWhiteFadeInOut(gTL_01, 1);
	mFadeTimer = 0x3C;
}

// 0x801722C8
void TModeMainMenu::fadeOutFile1()
{
	freeFade();
	mFade      = new TWhiteFadeInOut(gTL_01, 1);
	mFadeTimer = 0x3C;
}

// 0x80172324
int TModeMainMenu::fadeDone()
{
	if (mFadeTimer == 0) {
		return 1;
	}
	mFadeTimer--;
	return 0;
}

// 0x80172348
void TModeMainMenu::setState(int state)
{
	mNextState = state;
}

// 0x80172350
int TModeMainMenu::pickState()
{
	int next = mState;
	switch (mCursor) {
	case 0:
		next = 2;
		break;
	case 1:
		next = 8;
		break;
	case 3:
		if (cardStateIs6() != 0) {
			next = 0xA;
		} else {
			next = 9;
		}
		break;
	case 4:
		next = 0xB;
		break;
	case 2:
		if (cardResultIs8() != 0) {
			next = 0x11;
		} else {
			next = 0xF;
		}
		break;
	case 5:
		next = 0x12;
		break;
	case 6:
		next = 0xE;
		break;
	case 7:
		next = 0xD;
		break;
	}
	if (next != mState) {
		setState(next);
		return 1;
	}
	return 0;
}

static void stateIdle(TModeMainMenu* menu, int phase);
static void stateNotice(TModeMainMenu* menu, int phase);
static void stateRoot(TModeMainMenu* menu, int phase);
static void stateFile(TModeMainMenu* menu, int phase);
static void statePage3(TModeMainMenu* menu, int phase);
static void state6(TModeMainMenu* menu, int phase);
static void stateBack(TModeMainMenu* menu, int phase);
static void state8(TModeMainMenu* menu, int phase);
static void state9(TModeMainMenu* menu, int phase);
static void state10(TModeMainMenu* menu, int phase);
static void state11(TModeMainMenu* menu, int phase);
static void state12(TModeMainMenu* menu, int phase);
static void state13(TModeMainMenu* menu, int phase);
static void state14(TModeMainMenu* menu, int phase);
static void state15(TModeMainMenu* menu, int phase);
static void state16(TModeMainMenu* menu, int phase);
static void state17(TModeMainMenu* menu, int phase);
static void state18(TModeMainMenu* menu, int phase);
static void state19(TModeMainMenu* menu, int phase);
static void state20(TModeMainMenu* menu, int phase);
static void state21(TModeMainMenu* menu, int phase);
static void state22(TModeMainMenu* menu, int phase);
static void state23(TModeMainMenu* menu, int phase);
static void state24(TModeMainMenu* menu, int phase);
static void state25(TModeMainMenu* menu, int phase);
static void state26(TModeMainMenu* menu, int phase);
static void state27(TModeMainMenu* menu, int phase);
static void state28(TModeMainMenu* menu, int phase);
static void state30(TModeMainMenu* menu, int phase);
static void state29(TModeMainMenu* menu, int phase);
static void state31(TModeMainMenu* menu, int phase);
static void state32(TModeMainMenu* menu, int phase);
static void state33(TModeMainMenu* menu, int phase);
static void state34(TModeMainMenu* menu, int phase);
static void state35(TModeMainMenu* menu, int phase);

// 0x80172424
void TModeMainMenu::runState(int state, int phase)
{
	switch (state) {
	case 1:
		stateIdle(this, phase);
		break;
	case 2:
		stateNotice(this, phase);
		break;
	case 3:
		stateRoot(this, phase);
		break;
	case 4:
		stateFile(this, phase);
		break;
	case 5:
		statePage3(this, phase);
		break;
	case 6:
		state6(this, phase);
		break;
	case 7:
		stateBack(this, phase);
		break;
	case 8:
		state8(this, phase);
		break;
	case 9:
		state9(this, phase);
		break;
	case 10:
		state10(this, phase);
		break;
	case 11:
		state11(this, phase);
		break;
	case 12:
		state12(this, phase);
		break;
	case 13:
		state13(this, phase);
		break;
	case 14:
		state14(this, phase);
		break;
	case 15:
		state15(this, phase);
		break;
	case 16:
		state16(this, phase);
		break;
	case 17:
		state17(this, phase);
		break;
	case 20:
		state20(this, phase);
		break;
	case 19:
		state19(this, phase);
		break;
	case 18:
		state18(this, phase);
		break;
	case 21:
		state21(this, phase);
		break;
	case 22:
		state22(this, phase);
		break;
	case 23:
		state23(this, phase);
		break;
	case 25:
		state25(this, phase);
		break;
	case 26:
		state26(this, phase);
		break;
	case 24:
		state24(this, phase);
		break;
	case 27:
		state27(this, phase);
		break;
	case 28:
		state28(this, phase);
		break;
	case 29:
		state29(this, phase);
		break;
	case 30:
		state30(this, phase);
		break;
	case 31:
		state31(this, phase);
		break;
	case 32:
		state32(this, phase);
		break;
	case 33:
		state33(this, phase);
		break;
	case 34:
		state34(this, phase);
		break;
	case 35:
		state35(this, phase);
		break;
	}
}

// 0x80172600
static void stateIdle(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->mCursor     = 0;
		menu->mPrevCursor = menu->mCursor;
		menu->closeBox();
		saveReset();
		break;
	case MENU_UPDATE:
		if (menu->pickState() == 0 && menu->mCursor == 0x12) {
			cardLoadSave();
			menu->setState(3);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172688
static void stateNotice(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(7), -1);
		break;
	case MENU_UPDATE:
		if (menu->mCursor != 0) {
			if (menu->mPage == 3) {
				menu->setState(6);
			} else {
				menu->setState(1);
			}
		} else if (menu->runBox() != 0) {
			menu->mBackState = menu->mState;
			menu->setState(0x14);
		}
		break;
	case MENU_EXIT:
		menu->resetRoot();
		break;
	case MENU_DRAW:
		break;
	}
}

// 0x8017274C
static void stateRoot(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->mPage = 1;
		menu->freeFade();
		menu->closeBox();
		menu->resetRoot();
		aeMainMenuShowRoot();
		for (int i = 0; i < 3; i++) {
			aeMainMenuClearFileBar(i);
		}
		break;
	case MENU_UPDATE:
		if (menu->pickState() != 0) {
			saveReset();
		} else if (menu->updateRoot() != 0) {
			if (menu->mSelect == 3) {
				menu->setState(4);
			} else if (menu->mSelect == 4) {
				menu->setState(7);
			} else if (menu->mSelect == 6) {
				menu->setState(0x19);
			} else if (menu->mSelect == 7) {
				menu->setState(0x1A);
			}
		}
		break;
	case MENU_DRAW:
		menu->chooseFiles();
		break;
	case MENU_EXIT:
		aeMainMenuCloseRoot();
		menu->closeChoiceWindow();
		menu->hideAll();
		break;
	}
}

// 0x8017287C
static void stateFile(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->mPage = 2;
		menu->freeFade();
		menu->closeBox();
		menu->resetFile();
		aeMainMenuShowFiles();
		menu->freeFade();
		break;
	case MENU_UPDATE:
		if (menu->updateFile() != 0) {
			switch (menu->mSlot) {
			case 0:
			case 1:
				menu->_08 = 0;
				break;
			case 2:
				menu->setState(5);
				break;
			}
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x8017292C
static void statePage3(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->mPage = 3;
		menu->freeFade();
		menu->closeBox();
		menu->resetSound();
		aeMainMenuShowSound();
		menu->freeFade();
		break;
	case MENU_UPDATE:
		menu->updateSound();
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x801729A0
static void state6(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->closeBox();
		break;
	case MENU_UPDATE:
		if (menu->pickState() == 0 && menu->mCursor == 0x12) {
			menu->setState(0x18);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172A10
static void stateBack(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->fadeOutBack();
		aeMainMenuCloseRoot();
		menu->closeChoiceWindow();
		menu->hideAll();
		break;
	case MENU_UPDATE:
		if (menu->fadeDone() != 0) {
			menu->_08 = 1;
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172A84
static void state8(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(8), -1);
		break;
	case MENU_UPDATE:
		if (menu->pickState() == 0 && menu->runBox() != 0) {
			menu->mBackState = menu->mState;
			menu->setState(0x14);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172B14
static void state9(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(1), -1);
		break;
	case MENU_UPDATE:
		if (menu->pickState() == 0 && menu->runBox() != 0) {
			menu->mBackState = menu->mState;
			menu->setState(0x14);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172BA4
static void state10(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(0x2B), -1);
		break;
	case MENU_UPDATE:
		if (menu->pickState() == 0 && menu->runBox() != 0) {
			menu->mBackState = menu->mState;
			menu->setState(0x14);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172C34
static void state11(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(0x12), -1);
		break;
	case MENU_UPDATE:
		if (menu->pickState() == 0 && menu->runBox() != 0) {
			menu->mBackState = menu->mState;
			menu->setState(0x14);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172CC4
static void state12(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(3), -1);
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			menu->setState(0x14);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172D54
static void state13(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(0x19), -1);
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			if (menu->mPage == 3) {
				menu->mBackState = menu->mState;
				menu->setState(0x14);
			}
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172DF8
static void state14(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(0xE), -1);
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			menu->setState(0x13);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172E88
static void state15(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openYesNo(getStageSelectWarning(9));
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			if (menu->choseYes() != 0) {
				menu->mBackState = menu->mState;
				menu->setState(0x10);
			} else {
				menu->mBackState = menu->mState;
				menu->setState(0x14);
			}
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172F44
static void state16(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openYesNo(getStageSelectWarning(10));
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			if (menu->choseYes() != 0) {
				menu->setState(0x15);
			} else {
				menu->setState(menu->mBackState);
			}
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80172FF0
static void state17(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openYesNo(getStageSelectWarning(0x2C));
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			if (menu->choseYes() != 0) {
				menu->mBackState = menu->mState;
				menu->setState(0x10);
			} else {
				menu->mBackState = menu->mState;
				menu->setState(0x14);
			}
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x801730AC
static void state18(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openYesNo(getStageSelectWarning(2));
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			if (menu->choseYes() != 0) {
				menu->setState(0x16);
			} else {
				menu->mBackState = menu->mState;
				menu->setState(0xC);
			}
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80173160
static void state19(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openYesNo(getStageSelectWarning(0xF));
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			if (menu->choseYes() != 0) {
				menu->setState(0x17);
			} else {
				menu->mBackState = 0xE;
				menu->setState(0x14);
			}
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80173214
static void state20(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openYesNo(getStageSelectWarning(4));
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
			if (menu->mCursor == 0xB) {
				if (menu->mPage == 3) {
					menu->setState(6);
				} else {
					menu->setState(1);
				}
			}
		} else if (menu->runBox() != 0) {
			if (menu->choseYes() != 0) {
				if (menu->mPage == 3) {
					menu->setState(0x23);
				} else {
					menu->setState(4);
				}
			} else {
				menu->setState(menu->mBackState);
			}
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80173310
static void state21(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openTimed(getStageSelectWarning(0xB));
		cardRequestFormat();
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		}
		if (menu->mCursor == 0xF && menu->runBox() != 0) {
			menu->setState(0x1B);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x801733AC
static void state22(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openTimed(getStageSelectWarning(5));
		cardRequestCreate();
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		}
		if (menu->mCursor == 0x10 && menu->runBox() != 0) {
			menu->setState(0x1C);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80173448
static void state23(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openTimed(getStageSelectWarning(0x10));
		cardRequestDelete();
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		}
		if (menu->mCursor == 0x11 && menu->runBox() != 0) {
			menu->setState(0x1D);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x801734E4
static void state24(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openTimed(getStageSelectWarning(0x15));
		cardRequestSave();
		break;
	case MENU_UPDATE:
		if (menu->mCursor == 0) {
			menu->setState(0x22);
		} else if (menu->pickState() == 0 && menu->mCursor == 0x13 && menu->runBox() != 0) {
			menu->setState(0x1E);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80173590
static void state25(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openTimed(getStageSelectWarning(0x21));
		cardRequestSave();
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		}
		if (menu->mCursor == 0x13 && menu->runBox() != 0) {
			menu->setState(0x1F);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x8017362C
static void state26(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openTimed(getStageSelectWarning(0x24));
		cardRequestSave();
		break;
	case MENU_UPDATE:
		if (menu->mCursor != menu->mPrevCursor) {
			menu->pickState();
		}
		if (menu->mCursor == 0x13 && menu->runBox() != 0) {
			menu->setState(0x20);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x801736C8
static void state27(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openNotice(getStageSelectWarning(0xC));
		break;
	case MENU_UPDATE:
		if (menu->mCursor == 0) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			if (menu->pickState() != 0) {
				menu->mBackState = menu->mNextState;
				menu->setState(0x21);
			} else {
				menu->setState(1);
				cardConfirm();
			}
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x8017377C
static void state28(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openNotice(getStageSelectWarning(6));
		break;
	case MENU_UPDATE:
		if (menu->mCursor == 0) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			menu->setState(1);
			cardConfirm2();
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80173808
static void state30(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		cardConfirm4();
		break;
	case MENU_UPDATE:
		if (menu->mCursor == 0x12) {
			menu->setState(4);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x8017385C
static void state29(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openNotice(getStageSelectWarning(0x11));
		break;
	case MENU_UPDATE:
		if (menu->mCursor == 0) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			cardConfirm3();
			menu->setState(1);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x801738E8
static void state31(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openNotice(getStageSelectWarning(0x22));
		break;
	case MENU_UPDATE:
		if (menu->mCursor == 0) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			cardConfirm4();
			menu->setState(3);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80173974
static void state32(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openNotice(getStageSelectWarning(0x25));
		break;
	case MENU_UPDATE:
		if (menu->mCursor == 0) {
			menu->pickState();
		} else if (menu->runBox() != 0) {
			cardConfirm4();
			menu->setState(3);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80173A00
static void state33(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(0xD), -1);
		break;
	case MENU_UPDATE:
		if (menu->pickState() == 0 && menu->runBox() != 0) {
			menu->setState(menu->mBackState);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80173A88
static void state34(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(0x1A), -1);
		break;
	case MENU_UPDATE:
		if (menu->pickState() == 0 && menu->runBox() != 0) {
			menu->setState(6);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// 0x80173B10
static void state35(TModeMainMenu* menu, int phase)
{
	switch (phase) {
	case MENU_ENTER:
		menu->openMessage(getStageSelectWarning(0x16), -1);
		break;
	case MENU_UPDATE:
		if (menu->runBox() != 0) {
			menu->setState(4);
		}
		break;
	case MENU_DRAW:
		break;
	case MENU_EXIT:
		break;
	}
}

// Text pages Every shown text gets this box
static inline Vec3f menuBox()
{
	return vec3f(0.0f, 24.0f, 0.0f);
}

static inline void hideText(TSimpleText* text)
{
	if (text != 0) {
		text->mFlags |= 0x10;
	}
}

#define MENU_STRING(i) ((void*)sMainMenuEntries[i])

// 0x80173B88
void TModeMainMenu::hideAll()
{
	hideTitle();
	hideHelp();
	hideCards();
	hideCaptions();
	hideTabs();
	hideLines();
	hideLinePair();
	hideCaptionRow();
	hideCaptionPair();
}

// 0x80173BF4
void TModeMainMenu::showCards(int slot)
{
	Vec3f box = menuBox();
	if (mText[4] != 0) {
		mText[4]->mFlags &= ~0x10;
		mText[4]->mAt  = sYesAt[slot];
		mText[4]->mBox = box;
		mText[4]->_A8  = 1;
	}
	if (mText[5] != 0) {
		mText[5]->mFlags &= ~0x10;
		mText[5]->mAt  = sNoAt[slot];
		mText[5]->mBox = box;
		mText[5]->_A8  = 1;
	}
}

// 0x80173D04
void TModeMainMenu::hideCards()
{
	hideText(mText[4]);
	hideText(mText[5]);
}

// 0x80173D38
void TModeMainMenu::showCaptions(int slot)
{
	Vec3f box = menuBox();
	if (mCaption[0] != 0) {
		mCaption[0]->mFlags &= ~0x10;
		mCaption[0]->resetPage(MENU_STRING(6));
		mCaption[0]->mAt  = sCaptionAt0[slot];
		mCaption[0]->mBox = box;
		mCaption[0]->_A8  = 1;
	}
	if (mCaption[1] != 0) {
		mCaption[1]->mFlags &= ~0x10;
		mCaption[1]->resetPage(MENU_STRING(7));
		mCaption[1]->mAt  = sCaptionAt1[slot];
		mCaption[1]->mBox = box;
		mCaption[1]->_A8  = 1;
	}
	if (mCaption[2] != 0) {
		mCaption[2]->mFlags &= ~0x10;
		mCaption[2]->resetPage(MENU_STRING(8));
		mCaption[2]->mAt  = sCaptionAt2[slot];
		mCaption[2]->mBox = box;
		mCaption[2]->_A8  = 1;
	}
}

// 0x80173F00
void TModeMainMenu::hideCaptions()
{
	hideText(mCaption[0]);
	hideText(mCaption[1]);
	hideText(mCaption[2]);
}

// 0x80173F4C
void TModeMainMenu::showTitle(int msg)
{
	if (mText[0] != 0) {
		mText[0]->mFlags &= ~0x10;
		mText[0]->resetPage(MENU_STRING(msg));
	}
	showTabs();
}

// 0x80173FA8
void TModeMainMenu::hideTitle()
{
	hideText(mText[0]);
}

// 0x80173FC4
void TModeMainMenu::showFileHelp(int slot)
{
	if (mText[1] != 0) {
		mText[1]->mFlags &= ~0x10;
		mText[1]->resetPage(MENU_STRING(11 + slot));
		mText[1]->mAt = sHelpAt;
	}
	showTabs();
}

// 0x80174044
void TModeMainMenu::hideHelp()
{
	hideText(mText[1]);
}

// 0x80174060
void TModeMainMenu::showTabs()
{
	Vec3f box = menuBox();
	if (mText[2] != 0) {
		mText[2]->mFlags &= ~0x10;
		mText[2]->mBox = box;
		mText[2]->_A8  = 1;
	}
	if (mText[3] != 0) {
		mText[3]->mFlags &= ~0x10;
		mText[3]->mBox = box;
		mText[3]->_A8  = 1;
	}
}

// 0x80174118
void TModeMainMenu::hideTabs()
{
	hideText(mText[2]);
	hideText(mText[3]);
}

// 0x8017414C
void TModeMainMenu::showLines()
{
	Vec3f box = menuBox();
	if (mLine[0] != 0) {
		mLine[0]->mFlags &= ~0x10;
		mLine[0]->resetPage(MENU_STRING(16));
		mLine[0]->mAt  = sLineAt0;
		mLine[0]->mBox = box;
		mLine[0]->_A8  = 1;
	}
	if (mLine[1] != 0) {
		mLine[1]->mFlags &= ~0x10;
		mLine[1]->resetPage(MENU_STRING(17));
		mLine[1]->mAt  = sLineAt1;
		mLine[1]->mBox = box;
		mLine[1]->_A8  = 1;
	}
	if (mLine[2] != 0) {
		mLine[2]->mFlags &= ~0x10;
		mLine[2]->resetPage(MENU_STRING(18));
		mLine[2]->mAt  = sLineAt2;
		mLine[2]->mBox = box;
		mLine[2]->_A8  = 1;
	}
}

// 0x801742F8
void TModeMainMenu::hideLines()
{
	hideText(mLine[0]);
	hideText(mLine[1]);
	hideText(mLine[2]);
}

// 0x80174344
void TModeMainMenu::showSoundHelp(int mode)
{
	if (mText[1] != 0) {
		mText[1]->mFlags &= ~0x10;
		mText[1]->resetPage(MENU_STRING(14 + mode));
		mText[1]->mAt = sHelpAt;
	}
	showTabs();
}

// 0x801743C4
void TModeMainMenu::showLinePair()
{
	Vec3f box = menuBox();
	if (mLine[0] != 0) {
		mLine[0]->mFlags &= ~0x10;
		mLine[0]->resetPage(MENU_STRING(19));
		mLine[0]->mAt  = sLeftAt;
		mLine[0]->mBox = box;
		mLine[0]->_A8  = 1;
	}
	if (mLine[1] != 0) {
		mLine[1]->mFlags &= ~0x10;
		mLine[1]->resetPage(MENU_STRING(20));
		mLine[1]->mAt  = sRightAt;
		mLine[1]->mBox = box;
		mLine[1]->_A8  = 1;
	}
	hideText(mLine[2]);
}

// 0x80174510
void TModeMainMenu::hideLinePair()
{
	hideLines();
}

// 0x80174530
void TModeMainMenu::showCaptionRow()
{
	Vec3f box = menuBox();
	if (mCaption[0] != 0) {
		mCaption[0]->mFlags &= ~0x10;
		mCaption[0]->resetPage(MENU_STRING(21));
		mCaption[0]->mAt  = sCaptionAt0[1];
		mCaption[0]->mBox = box;
		mCaption[0]->_A8  = 1;
	}
	if (mCaption[1] != 0) {
		mCaption[1]->mFlags &= ~0x10;
		mCaption[1]->resetPage(MENU_STRING(22));
		mCaption[1]->mAt  = sCaptionAt1[1];
		mCaption[1]->mBox = box;
		mCaption[1]->_A8  = 1;
	}
	if (mCaption[2] != 0) {
		mCaption[2]->mFlags &= ~0x10;
		mCaption[2]->resetPage(MENU_STRING(23));
		mCaption[2]->mAt  = sCaptionAt2[1];
		mCaption[2]->mBox = box;
		mCaption[2]->_A8  = 1;
	}
}

// 0x801746DC
void TModeMainMenu::hideCaptionRow()
{
	hideCaptions();
}

// 0x801746FC
void TModeMainMenu::showCaptionPair()
{
	Vec3f box = menuBox();
	if (mCaption[0] != 0) {
		mCaption[0]->mFlags &= ~0x10;
		mCaption[0]->resetPage(MENU_STRING(24));
		mCaption[0]->mAt  = sYesAt[1];
		mCaption[0]->mBox = box;
		mCaption[0]->_A8  = 1;
	}
	if (mCaption[1] != 0) {
		mCaption[1]->mFlags &= ~0x10;
		mCaption[1]->resetPage(MENU_STRING(25));
		mCaption[1]->mAt  = sNoAt[1];
		mCaption[1]->mBox = box;
		mCaption[1]->_A8  = 1;
	}
	hideText(mCaption[2]);
}

// 0x80174850
void TModeMainMenu::hideCaptionPair()
{
	hideCaptions();
}

// 0x80174870
void TModeMainMenu::showIcons()
{
	for (int i = 0; i < 3; i++) {
		if (mIcon[i] != 0) {
			mIcon[i]->mFlags &= ~0x10;
		}
	}
}

// 0x801748A4
void TModeMainMenu::hideIcons()
{
	for (int i = 0; i < 3; i++) {
		if (mIcon[i] != 0) {
			mIcon[i]->mFlags |= 0x10;
		}
	}
}
