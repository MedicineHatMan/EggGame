#include "core/main.h"
#include "item/TItemManage.h"
#include "stage/Stage.h"
#include "stage/GameFlow.h"
#include "core/dolphin.h"
#include "ui/Font.h"
#include "io/FileReader.h"
#include "core/JoyCarry.h"
#include "mode/TModeMovie.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "object/TObjKey00.h"
#include "io/ResManager.h"
#include "player/Player.h"
#include "ui/ConstPalette.h"
#include "sound/SoundCtrl.h"
#include "core/Vibration.h"
#include "io/SaveData.h"
#include "io/CardManager.h"
#include "enemy/boss/TEneYellowBossGreen.h"
#include "ui/NowLoading.h"
#include "Dolphin/ai.h"
#include "Dolphin/ar.h"
#include "ui/TextTexture.h"
#include "core/Pad.h"
#include "core/Mem.h"
#include "engine/FrameTex.h"
#include "object/TObjLauncher.h"
#include "Dolphin/dvd.h"
#include "Dolphin/vi.h"
#include "Dolphin/gba.h"
#include "core/TMainTask.h"
#include "core/SdkCalls.h"
#include "Ninja/ninja.h"
#include "sound/AudioLib.h"
struct NJS_QUAD_TEXTURE;
struct TObject;

// Not identified yet dtk's default fn_/lbl_ names
// Comments are guesses from call order and from the PC build's hglnj.dll exports

struct GraphicsInitParams {
	u32 _00;       // _00
	u32 _04;       // _04
	u32 mXfb0;     // _08
	u32 mXfb1;     // _0C
	u32 mFifo;     // _10
	u32 mFifoSize; // _14
	u32 mHeap;     // _18
	u32 mHeapSize; // _1C
};

struct ScreenQuad {
	f32 mMinX; // _00
	f32 mMinY; // _04
	f32 mMaxX; // _08
	f32 mMaxY; // _0C
	f32 mMinU; // _10
	f32 mMinV; // _14
	f32 mMaxU; // _18
	f32 mMaxV; // _1C
};

// All zero in .sbss2 No declaration tried gets the compiler to put it there so it stays outside this unit for now

static const char* sDiscMessages[] = {
	"An error has occurred. \nTurn the power off and refer to \nthe Nintendo GameCube Instruction \nBooklet for more information.",
	"The Disc Cover is open. \nIf you want to continue the game, \nplease close the Disc Cover.",
	"Please insert the Billy Hatcher \nand the Giant Egg Game Disc.",
	"This is not the Billy Hatcher \nand the Giant Egg Game Disc. \nPlease insert the Billy Hatcher \nand the Giant Egg Game Disc.",
	"The Game Disc could not be read. \nPlease read the Nintendo GameCube \nInstruction Booklet for more \ninformation.",
};

static void emptyCallback(u32);

/**
 * @note Address: 0x8000EED8
 * @note Size: 0x44
 */
int main(int argc, char** argv)
{
	CMain app(argc, argv);
	app.run();
	return 0;
}

/**
 * @note Address: 0x8000EF1C
 * @note Size: 0xEC
 */
CMain::CMain(int argc, char** argv)
{
	mSSArg           = 0;
	mPrevResetButton = 0;
	mPrevPadReset    = 0;
	_24              = 0;
	mHasDiscError    = 0;
	_2C              = 0;

	initSystem();
	parseArgs(argc, argv);

	if (mSSArg) {
		enableSSMode();
	} else {
		plyGlobalsInit();
	}

	initObjects((TObject*)&gMainTask);
	memDebugInit();
	initResources();
	fontOpen();
	mTextObject = createTextTexture(0x40, 0x10, 0x101E);
	initKeyBits();
	frameTexBoot();
	initItems();
	loadBootPrd("k_boot.prd");
	soundLoadBanks();
	addResPack("amem_boot.nrc");
	mFlow = new TGameFlow;
}

/**
 * @note Address: 0x8000F008
 * @note Size: 0x3C
 */
CMain::~CMain()
{
}

/**
 * @note Address: 0x8000F044
 * @note Size: 0x1F0
 */
void CMain::run()
{
	int state = 0;

	while (true) {
		s32 driveStatus = DVDGetDriveStatus();

		switch (state) {
		case 0: {
			if (driveStatus == DVD_STATE_FATAL_ERROR) {
				showFatalError();
				break;
			}

			FileReader::get()->update();
			padUpdate();
			int reset = checkReset();

			if (checkDisc()) {
				movieModeCheckDisc();
				if (reset) {
					restart();
				}
				break;
			}

			if (FileReader::get()->getState() == 1) {
				initGame();
				VIWaitForRetrace();
				if (isProgressiveRequested()) {
					mFlow->startProgressive();
				} else {
					mFlow->start();
				}
				state = 2;
			}
			break;
		}

		case 2: {
			if (driveStatus == DVD_STATE_FATAL_ERROR) {
				showFatalError();
				movieModeCheckDisc();
				break;
			}

			FileReader::get()->update();
			padUpdate();
			JoyCarry::get()->update();
			int reset = checkReset();

			if (checkDisc()) {
				movieModeCheckDisc();
				if (reset) {
					restart();
				}
				break;
			}

			vibUpdate();
			if (reset) {
				if (sNoRestart == 0) {
					restart();
				} else if (DVDCheckDisk() == 0) {
					hotReset(0);
				} else {
					padRecalibrateAll();
					vibStopAll();
					state = 3;
				}
				break;
			}

			updateResources();
			if (mSSArg == 0) {
				plyGlobalsEmpty();
			}
			mFlow->update();
			break;
		}

		case 3:
			padUpdate();
			vibUpdate();
			if (cardIsWriting()) {
				cardUpdate();
			} else if (mFlow->reset() == 0) {
				restart();
			} else {
				gjSwapBuffers();
				state = 2;
			}
			break;
		}
	}
}

/**
 * @note Address: 0x8000F234
 * @note Size: 0xAC
 */
int CMain::checkReset()
{
	int button         = OSGetResetButtonState();
	int buttonReleased = (mPrevResetButton != 0 && button == 0);
	mPrevResetButton   = button;

	PadState* pad = padGet(0);
	int padReset  = 0;
	if (pad->mStatus != -2) {
		int padCombo  = padResetHeld();
		padReset      = (mPrevPadReset == 0 && padCombo != 0);
		mPrevPadReset = padCombo;
	}

	return buttonReleased || padReset;
}

/**
 * @note Address: 0x8000F2E0
 * @note Size: 0x90
 */
void CMain::parseArgs(int argc, char** argv)
{
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-ss") == 0) {
			mSSArg = 1;
		} else {
			OSPanic("main.cpp", 474, "unkown args");
		}
	}
}

/**
 * @note Address: 0x8000F370
 * @note Size: 0x1CC
 */
void CMain::initSystem()
{
	OSInit();
	DVDInit();
	VIInit();
	PADInit();
	GBAInit();
	njInitDevice();
	ARInit(sARStack, 4);
	ARQInit();
	AIInit(NULL);
	AXInit();
	MIXInit();
	AXRegisterCallback(gcaxSndCallback);
	ADXT_SetupDvdFs(0);
	ADXM_SetupThrd(0);
	ADXT_Init();
	gcaxSndInit(0, 0, 0);

	void* arenaLo = OSGetArenaLo();
	void* arenaHi = OSGetArenaHi();

	GraphicsInitParams params;
	params._00       = 0;
	u32 xfbSize      = gjCalcFrameBufferSize(0);
	params.mXfb0     = ((u32)arenaLo + 0x1F) & ~0x1F;
	params.mFifoSize = 0x40000;
	params.mXfb1     = params.mXfb0 + xfbSize;
	params.mFifo     = params.mXfb1 + xfbSize;
	params.mHeap     = (u32)OSInitAlloc((void*)(params.mFifo + 0x40000), arenaHi, 2);
	params.mHeapSize = 0x180000;

	u32 pool = params.mHeap + 0x180000;
	syMallocInit((void*)pool, (u32)arenaHi - pool);
	gjInitSystem((GjSysParams*)&params);
	OSSetArenaLo(arenaHi);

	mBuffer3600 = syMalloc(0x3600);
	mBuffer5A00 = syMalloc(0x5A00);
	njInitTextureEx(mBuffer3600, 0x240, mBuffer5A00, 0x240);
	njMiscInit(0, 0);

	mBuffer600 = syMalloc(0x600);
	gjInitMatrix((s32)mBuffer600, 0x20);
	syPadInit(0, 0, 0);

	mBuffer1800 = syMalloc(0x1800);
	njInitPrint((GjPrintEntry*)mBuffer1800, 0x80, 0x1000);
	njPrintColor(-1);
	njPrintSize(0x10);

	GXSetCopyClear(smainClearColor, 0xFFFFFF);
	VISetPostRetraceCallback(emptyCallback);
	setCullMode(0x10000000);
}

/**
 * @note Address: 0x8000F53C
 * @note Size: 0x60
 */
void CMain::initGame()
{
	tlutInit();
	fontInit();
	constPaletteInit();
	loadStageDefs();
	storySeqInit();
	padInit();
	saveCreate();
	nowLoadingInit();
	soundInit();

	if (OSGetSoundMode() == OS_SOUND_MODE_STEREO) {
		soundSetMode(1);
	} else {
		soundSetMode(0);
	}
}

/**
 * @note Address: 0x8000F59C
 * @note Size: 0x68
 */
int CMain::isProgressiveRequested()
{
	if (VIGetDTVStatus() == 0) {
		return 0;
	}

	padUpdate();
	return OSGetProgressiveMode() != 0 || (padGet(0)->mHold & PAD_BUTTON_B) != 0;
}

/**
 * @note Address: 0x8000F604
 * @note Size: 0x5C
 */
void CMain::hotReset(int forceMenu)
{
	VISetBlack(TRUE);
	VIFlush();
	VIWaitForRetrace();

	while (cardIsWriting()) {
		cardUpdate();
	}

	OSResetSystem(OS_RESET_HOTRESET, 0, forceMenu);
}

/**
 * @note Address: 0x8000F660
 * @note Size: 0x50
 */
void CMain::restart()
{
	VISetBlack(TRUE);
	VIFlush();
	VIWaitForRetrace();

	while (cardIsWriting()) {
		cardUpdate();
	}

	OSResetSystem(OS_RESET_RESTART, 0, FALSE);
}

/**
 * @note Address: 0x8000F6B0
 * @note Size: 0xF0
 */
int CMain::checkDisc()
{
	int prevError = mHasDiscError;
	mHasDiscError = 0;

	s32 driveStatus = DVDGetDriveStatus();
	if (driveStatus == DVD_STATE_COVER_OPEN) {
		showCoverOpen();
		mHasDiscError = 1;
	} else if (driveStatus == DVD_STATE_NO_DISK) {
		showNoDisc();
		mHasDiscError = 1;
	} else if (driveStatus == DVD_STATE_WRONG_DISK) {
		showWrongDisc();
		mHasDiscError = 1;
	} else if (driveStatus == DVD_STATE_RETRY) {
		showReadError();
		mHasDiscError = 1;
	}

	if ((mHasDiscError ^ prevError) != 0 && mHasDiscError != 0) {
		u32 motorCommands[4] = { 0, 0, 0, 0 };
		PADControlAllMotors(motorCommands);
		seSilenceAll();
	}

	return mHasDiscError;
}

/**
 * @note Address: 0x8000F7A0
 * @note Size: 0x2C
 */
void CMain::showFatalError()
{
	drawMessage(sDiscMessages[0]);
}

/**
 * @note Address: 0x8000F7CC
 * @note Size: 0x2C
 */
void CMain::showCoverOpen()
{
	drawMessage(sDiscMessages[1]);
}

/**
 * @note Address: 0x8000F7F8
 * @note Size: 0x2C
 */
void CMain::showNoDisc()
{
	drawMessage(sDiscMessages[2]);
}

/**
 * @note Address: 0x8000F824
 * @note Size: 0x2C
 */
void CMain::showWrongDisc()
{
	drawMessage(sDiscMessages[3]);
}

/**
 * @note Address: 0x8000F850
 * @note Size: 0x2C
 */
void CMain::showReadError()
{
	drawMessage(sDiscMessages[4]);
}

/**
 * @note Address: 0x8000F87C
 * @note Size: 0xE8
 */
void CMain::drawMessage(const char* message)
{
	gjBeforeRender();
	pushCullMode(0);
	gjSetPerspective(0x31C7, 1.3333334f, 1.0f, 10000.0f);
	njInitCameraEx();

	mTextObject->clearAll();
	mTextObject->print(0, 0, message);
	mTextObject->setTexture();

	ScreenQuad quad;
	quad.mMinX = 50.0f;
	quad.mMinY = 100.0f;
	quad.mMaxX = 512.0f + quad.mMinX;
	quad.mMaxY = 128.0f + quad.mMinY;
	quad.mMinU = 0.0f;
	quad.mMinV = 0.0f;
	quad.mMaxU = 1.0f;
	quad.mMaxV = 1.0f;

	njQuadTextureStart(1);
	njSetQuadTexture(0, -1);
	njDrawQuadTexture((const NJS_QUAD_TEXTURE*)&quad, 0.45f);
	njQuadTextureEnd();
	popCullMode();
	gjPrintFlush();
	gjDoneRender();
	gjSwapBuffers();
}

/**
 * @note Address: 0x8000F964
 * @note Size: 0x4
 */
static void emptyCallback(u32)
{
}
