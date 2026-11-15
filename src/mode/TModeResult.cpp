#include "mode/TModeResult.h"
#include "ui/TextVec.h"
#include "ui/TSimpleText.h"
#include "ui/TAeObjMsgWindow.h"
#include "core/TMainTask.h"
#include "core/Pad.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCamera.h"
#include "camera/TCameraCtrl.h"
#include "engine/Viewport.h"
#include "engine/DrawTask.h"
#include "object/TObjLauncher.h"
#include "ui/Font.h"
#include "stage/TStageSelectModel.h"
#include "io/CardManager.h"
#include "Ninja/ninja.h"
#include "stage/Stage.h"
#include "io/MemCard.h"
#include "sound/SoundCtrl.h"

enum ResultPhase {
	RESULT_ENTER  = 0,
	RESULT_UPDATE = 1,
	RESULT_DRAW   = 2,
	RESULT_EXIT   = 3,
};

inline Vec3f vec3f(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// Built by the static initializer at 0x801B9EDC
static TextVec sTextGlyph = textVec(0.91f, 0.91f, 0.91f);
static TextVec sTextBox   = textVec(480.0f, 36.0f, 0.0f);
static TextVec sTextAt    = textVec(54.0f, 60.0f, -2.0f);
static TextVec sYesAt     = textVec(512.0f, 320.0f, -2.0f);
static TextVec sNoAt      = textVec(512.0f, 373.0f, -2.0f);

inline void freeObj(TSimpleText*& obj)
{
	if (obj != 0) {
		obj->mFlags |= 1;
		obj = 0;
	}
}

// 0x801B8148
TModeResult* TModeResult::create(TModeAllocator* allocator)
{
	return new (allocator) TModeResult;
}

// 0x801B8180
void TModeResult::loadFiles()
{
	loadPrd("k_result.prd");
}

// 0x801B81A8
int TModeResult::init()
{
	GXSetCopyClear(*(GXColor*)&sModeResultClearColor, 0xFFFFFF);
	camCtrlInit();
	viewportInit();
	initDrawTasks();
	loadMsgWindow();
	loadStageSelectText();
	cardInit();
	objClearLayers(&gMainTask);
	createCameraCtrl();
	camSetDrawView(0);
	setViewport(0);
	cardCreate();
	fontCreateTask();
	buildText();
	runState(mState, RESULT_ENTER);
	return 1;
}

// 0x801B8244
int TModeResult::shutdown()
{
	closeBox();
	freeObj(mOption[0]);
	freeObj(mOption[1]);
	freeObj(mBox);
	objClearLayers(&gMainTask);
	cardShutdown();
	freeStageSelectText();
	freeMsgWindow();
	termDrawTasks();
	viewportTerm();
	camCtrlTerm();
	return 1;
}

// 0x801B82F8
TModeResult::TModeResult()
{
	mState     = 0;
	mBackState = 0;
	mNextState = 0;
	mBoxKind   = 0;
	mChoice    = 0;
	mBoxTimer  = 0;
	_24        = 0;
	mCard      = 0xB;
	mPrevCard  = mCard;
	mFrame     = 0;
	mPressedA  = 0;
	mOption[0] = 0;
	mOption[1] = 0;
	mBox       = 0;
	mWindow    = 0;
}

// Unused and stripped by the linker
static void dropResult(TModeResult* mode)
{
	delete mode;
}

// 0x801B838C
void TModeResult::buildText()
{
	mOption[0] = createSimpleTextOnTask(1, -1, &sTextAt, &sTextBox, &sTextGlyph, getStageSelectWarning(0x27));
	mOption[1] = createSimpleTextOnTask(1, -1, &sTextAt, &sTextBox, &sTextGlyph, getStageSelectWarning(0x28));
	hideOptions();
}

// 0x801B8414
void TModeResult::update()
{
	cardUpdate();
	gMainTask.draw();
	advance();

	gjBeforeRender();
	gjSetRenderMode(3);
	GXSetCullMode(GX_CULL_NONE);
	setViewport(0);
	camSetDrawView(0);
	gMainTask.vfn_10();
	runDrawTasks(0x21);
	runDrawTasks(0x3C);
	runDrawTasks(0x3E);
	runDrawTasks(0x3F);
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}

// 0x801B84B0
void TModeResult::advance()
{
	mPrevCard = mCard;
	mCard     = cardGetState();
	mPressedA = padGet(0)->mPressed & 0x100;

	runState(mState, RESULT_UPDATE);
	if (mNextState != mState) {
		runState(mState, RESULT_EXIT);
		mState = mNextState;
		runState(mState, RESULT_ENTER);
	}
	runState(mState, RESULT_DRAW);
	mFrame++;
}

// 0x801B855C
int TModeResult::pressedA()
{
	if ((int)mPressedA != 0) {
		mPressedA = 0;
		return 1;
	}
	return 0;
}

// 0x801B8580
int TModeResult::pressedB()
{
	return padGet(0)->mPressed & 0x200;
}

// 0x801B85AC
int TModeResult::anyButton()
{
	return padGet(0)->mPressed & 0x1F7F;
}

// 0x801B85D8
// A stick direction counts as its d-pad button too
int TModeResult::repeated(u32 mask)
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
	return mask & padGet(0)->mRepeat;
}

// 0x801B867C
void TModeResult::closeBox()
{
	mBoxKind = 0;
	mChoice  = 0;
	_24      = 0;
	if (mBox != 0) {
		mBox->mHold = 1;
		mBox        = 0;
	}
	closeChoice();
}

// 0x801B86C4
void TModeResult::closeChoice()
{
	TAeObjMsgWindow* window = mWindow;
	if (window != 0) {
		window->mDieOnClose = TRUE;
		window->close();
		mWindow = 0;
	}
	hideOptions();
}

// 0x801B8714
void TModeResult::openChoice(void* message)
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
	if (mWindow == 0) {
		mWindow = createMsgWindow(gTL_07, 9);
		if (mWindow != 0) {
			mWindow->setPrompt(3);
			mWindow->setCursor(mChoice);
		}
	}
	showOptions();
}

// 0x801B87CC
void TModeResult::openMessage(void* message)
{
	closeBox();
	mBoxKind  = 2;
	mBoxTimer = 0;
	if (mBox == 0) {
		mBox = createSimpleTextByKind(gTL_07, message, -1);
		if (mBox != 0) {
			mBox->_A8 = 1;
		}
	}
}

// 0x801B8840
void TModeResult::openNotice(void* message)
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
}

// 0x801B88B0
void TModeResult::openTimed(void* message)
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
}

// 0x801B8920
// 1 once the box is answered or done
int TModeResult::runBox()
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
				int choice = mChoice;
				if (choice >= 2) {
					choice = 0;
				}
				mChoice = choice;
			}
			sePlay(0x1B);
		} else if (repeated(8) != 0) {
			mChoice--;
			{
				int choice = mChoice;
				if (choice < 0) {
					choice = 1;
				}
				mChoice = choice;
			}
			sePlay(0x1B);
		}
		if (mWindow != 0) {
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
	}
	return done;
}

// 0x801B8AEC
int TModeResult::choseYes()
{
	return mChoice == 0;
}

// 0x801B8AFC
void TModeResult::setState(int state)
{
	mNextState = state;
}

// 0x801B8B04
// Moves to the page for the card's state
int TModeResult::checkCard()
{
	int next = mState;
	switch (mCard) {
	case 0:
		next = 1;
		break;
	case 1:
		next = 2;
		break;
	case 3:
		if (cardStateIs6() != 0) {
			next = 4;
		} else {
			next = 3;
		}
		break;
	case 4:
		next = 5;
		break;
	case 2:
		if (cardResultIs8() != 0) {
			next = 0xC;
		} else {
			next = 0xA;
		}
		break;
	case 5:
		next = 0xD;
		break;
	case 6:
		next = 8;
		break;
	case 7:
		next = 7;
		break;
	}
	if (next != mState) {
		setState(next);
		return 1;
	}
	return 0;
}

static void stateIdle(TModeResult* mode, int phase);
static void stateNotice(TModeResult* mode, int phase);
static void stateCard2(TModeResult* mode, int phase);
static void stateCard3(TModeResult* mode, int phase);
static void stateCard4(TModeResult* mode, int phase);
static void stateCard5(TModeResult* mode, int phase);
static void stateCard6(TModeResult* mode, int phase);
static void stateCard7(TModeResult* mode, int phase);
static void stateCard8(TModeResult* mode, int phase);
static void stateAskSave(TModeResult* mode, int phase);
static void stateAsk10(TModeResult* mode, int phase);
static void stateAsk11(TModeResult* mode, int phase);
static void stateAsk12(TModeResult* mode, int phase);
static void stateAsk14(TModeResult* mode, int phase);
static void stateAsk13(TModeResult* mode, int phase);
static void stateAskSkip(TModeResult* mode, int phase);
static void stateWork16(TModeResult* mode, int phase);
static void stateWork17(TModeResult* mode, int phase);
static void stateWork18(TModeResult* mode, int phase);
static void stateSaving(TModeResult* mode, int phase);
static void stateDone20(TModeResult* mode, int phase);
static void stateDone21(TModeResult* mode, int phase);
static void stateDone22(TModeResult* mode, int phase);
static void stateSaved(TModeResult* mode, int phase);
static void stateError24(TModeResult* mode, int phase);
static void stateError25(TModeResult* mode, int phase);
static void stateFailed(TModeResult* mode, int phase);
static void stateNoSave(TModeResult* mode, int phase);

// 0x801B8BD8
void TModeResult::runState(int state, int phase)
{
	switch (state) {
	case 0:
		stateIdle(this, phase);
		break;
	case 1:
		stateNotice(this, phase);
		break;
	case 2:
		stateCard2(this, phase);
		break;
	case 3:
		stateCard3(this, phase);
		break;
	case 4:
		stateCard4(this, phase);
		break;
	case 5:
		stateCard5(this, phase);
		break;
	case 6:
		stateCard6(this, phase);
		break;
	case 7:
		stateCard7(this, phase);
		break;
	case 8:
		stateCard8(this, phase);
		break;
	case 9:
		stateAskSave(this, phase);
		break;
	case 10:
		stateAsk10(this, phase);
		break;
	case 11:
		stateAsk11(this, phase);
		break;
	case 12:
		stateAsk12(this, phase);
		break;
	case 14:
		stateAsk14(this, phase);
		break;
	case 13:
		stateAsk13(this, phase);
		break;
	case 15:
		stateAskSkip(this, phase);
		break;
	case 16:
		stateWork16(this, phase);
		break;
	case 17:
		stateWork17(this, phase);
		break;
	case 18:
		stateWork18(this, phase);
		break;
	case 19:
		stateSaving(this, phase);
		break;
	case 20:
		stateDone20(this, phase);
		break;
	case 21:
		stateDone21(this, phase);
		break;
	case 22:
		stateDone22(this, phase);
		break;
	case 23:
		stateSaved(this, phase);
		break;
	case 24:
		stateError24(this, phase);
		break;
	case 25:
		stateError25(this, phase);
		break;
	case 26:
		stateFailed(this, phase);
		break;
	case 27:
		stateNoSave(this, phase);
		break;
	}
}

// 0x801B8D60
static void stateIdle(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->closeBox();
		break;
	case RESULT_UPDATE:
		if (mode->checkCard() == 0 && mode->mCard == 0x12) {
			mode->setState(9);
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B8DD0
static void stateNotice(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openMessage(getStageSelectWarning(7));
		break;
	case RESULT_UPDATE:
		if (mode->mCard != 0) {
			mode->setState(0);
		} else if (mode->runBox() != 0) {
			mode->setState(0xF);
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// A card message that stays until the card changes
#define CARD_MESSAGE(name, id)                                   \
	static void name(TModeResult* mode, int phase)               \
	{                                                            \
		switch (phase) {                                         \
		case RESULT_ENTER:                                       \
			mode->openMessage(getStageSelectWarning(id));        \
			break;                                               \
		case RESULT_UPDATE:                                      \
			if (mode->checkCard() == 0 && mode->runBox() != 0) { \
				mode->setState(0xF);                             \
			}                                                    \
			break;                                               \
		case RESULT_DRAW:                                        \
			break;                                               \
		case RESULT_EXIT:                                        \
			break;                                               \
		}                                                        \
	}

// 0x801B8E5C
CARD_MESSAGE(stateCard2, 8)
// 0x801B8EE0
CARD_MESSAGE(stateCard3, 1)

// 0x801B8F64
static void stateCard4(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openMessage(getStageSelectWarning(0x2B));
		break;
	case RESULT_UPDATE:
		if (mode->checkCard() == 0 && mode->runBox() != 0) {
			mode->mBackState = mode->mState;
			mode->setState(0xF);
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B8FF0
CARD_MESSAGE(stateCard5, 0x12)

// A card message for a card that is in
#define CARD_WAIT(name, id, next)                         \
	static void name(TModeResult* mode, int phase)        \
	{                                                     \
		switch (phase) {                                  \
		case RESULT_ENTER:                                \
			mode->openMessage(getStageSelectWarning(id)); \
			break;                                        \
		case RESULT_UPDATE:                               \
			if (mode->mCard != mode->mPrevCard) {         \
				mode->checkCard();                        \
			} else if (mode->runBox() != 0) {             \
				mode->setState(next);                     \
			}                                             \
			break;                                        \
		case RESULT_DRAW:                                 \
			break;                                        \
		case RESULT_EXIT:                                 \
			break;                                        \
		}                                                 \
	}

// 0x801B9074
CARD_WAIT(stateCard6, 3, 0xF)
// 0x801B9100
CARD_WAIT(stateCard7, 0x19, 0xF)
// 0x801B918C
CARD_WAIT(stateCard8, 0xE, 0xE)

// 0x801B9218
// Save
static void stateAskSave(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openChoice(getStageSelectWarning(0x14));
		break;
	case RESULT_UPDATE:
		if (mode->mCard != mode->mPrevCard) {
			mode->checkCard();
		} else if (mode->runBox() != 0) {
			if (mode->choseYes()) {
				mode->setState(0x13);
			} else {
				mode->setState(0x1A);
			}
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B92C4
static void stateAsk10(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openChoice(getStageSelectWarning(9));
		break;
	case RESULT_UPDATE:
		if (mode->mCard != mode->mPrevCard) {
			mode->checkCard();
		} else if (mode->runBox() != 0) {
			if (mode->choseYes()) {
				mode->mBackState = mode->mState;
				mode->setState(0xB);
			} else {
				mode->mBackState = mode->mState;
				mode->setState(0xF);
			}
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B9380
static void stateAsk11(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openChoice(getStageSelectWarning(0xA));
		break;
	case RESULT_UPDATE:
		if (mode->mCard != mode->mPrevCard) {
			mode->checkCard();
		} else if (mode->runBox() != 0) {
			if (mode->choseYes()) {
				mode->setState(0x10);
			} else {
				mode->setState(mode->mBackState);
			}
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B942C
static void stateAsk12(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openChoice(getStageSelectWarning(0x2C));
		break;
	case RESULT_UPDATE:
		if (mode->mCard != mode->mPrevCard) {
			mode->checkCard();
		} else if (mode->runBox() != 0) {
			if (mode->choseYes()) {
				mode->mBackState = mode->mState;
				mode->setState(0xB);
			} else {
				mode->mBackState = mode->mState;
				mode->setState(0xF);
			}
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B94E8
static void stateAsk13(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openChoice(getStageSelectWarning(2));
		break;
	case RESULT_UPDATE:
		if (mode->mCard != mode->mPrevCard) {
			mode->checkCard();
		} else if (mode->runBox() != 0) {
			if (mode->choseYes()) {
				mode->setState(0x11);
			} else {
				mode->mBackState = mode->mState;
				mode->setState(0xF);
			}
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B959C
static void stateAsk14(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openChoice(getStageSelectWarning(0xF));
		break;
	case RESULT_UPDATE:
		if (mode->mCard != mode->mPrevCard) {
			mode->checkCard();
		} else if (mode->runBox() != 0) {
			if (mode->choseYes()) {
				mode->setState(0x12);
			} else {
				mode->mBackState = 0xE;
				mode->setState(0xF);
			}
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B9650
// Continue without saving
static void stateAskSkip(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openChoice(getStageSelectWarning(0x13));
		break;
	case RESULT_UPDATE:
		if (mode->mCard != mode->mPrevCard) {
			mode->checkCard();
			if (mode->mCard == 0xB) {
				mode->setState(0);
			}
		} else if (mode->runBox() != 0) {
			if (mode->choseYes()) {
				mode->setState(0x1A);
			} else {
				mode->setState(0);
			}
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// A card operation start it show its message and wait for the card to report it done
#define CARD_WORK(name, id, start, busy, done, next)    \
	static void name(TModeResult* mode, int phase)      \
	{                                                   \
		switch (phase) {                                \
		case RESULT_ENTER:                              \
			mode->openTimed(getStageSelectWarning(id)); \
			start();                                    \
			break;                                      \
		case RESULT_UPDATE:                             \
			if (mode->mCard == done) {                  \
				if (mode->runBox() != 0) {              \
					mode->setState(next);               \
				}                                       \
			} else if (mode->mCard != busy) {           \
				mode->checkCard();                      \
			}                                           \
			break;                                      \
		case RESULT_DRAW:                               \
			break;                                      \
		case RESULT_EXIT:                               \
			break;                                      \
		}                                               \
	}

// 0x801B9714
CARD_WORK(stateWork16, 0xB, cardRequestFormat, 8, 0xF, 0x14)
// 0x801B97A8
CARD_WORK(stateWork17, 5, cardRequestCreate, 9, 0x10, 0x15)
// 0x801B983C
CARD_WORK(stateWork18, 0x10, cardRequestDelete, 0xA, 0x11, 0x16)

// 0x801B98D0
// Saving
static void stateSaving(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openTimed(getStageSelectWarning(0x15));
		cardRequestSave();
		break;
	case RESULT_UPDATE:
		if (mode->mCard == 0) {
			if (mode->runBox() != 0) {
				mode->setState(0x19);
			}
		} else if (mode->checkCard() == 0 && mode->mCard == 0x13 && mode->runBox() != 0) {
			mode->setState(0x17);
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B998C
static void stateDone20(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openNotice(getStageSelectWarning(0xC));
		break;
	case RESULT_UPDATE:
		if (mode->mCard == 0) {
			mode->checkCard();
		} else if (mode->runBox() != 0) {
			if (mode->checkCard() != 0) {
				mode->mBackState = mode->mNextState;
				mode->setState(0x18);
			} else {
				mode->setState(0);
				cardConfirm();
			}
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B9A40
static void stateDone21(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openNotice(getStageSelectWarning(6));
		break;
	case RESULT_UPDATE:
		if (mode->mCard == 0) {
			mode->checkCard();
		} else if (mode->runBox() != 0) {
			mode->setState(0);
			cardConfirm2();
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B9ACC
static void stateDone22(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openNotice(getStageSelectWarning(0x11));
		break;
	case RESULT_UPDATE:
		if (mode->mCard == 0) {
			mode->checkCard();
		} else if (mode->runBox() != 0) {
			cardConfirm3();
			mode->setState(0);
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B9B58
// Saved the mode ends
static void stateSaved(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openNotice(getStageSelectWarning(0x17));
		break;
	case RESULT_UPDATE:
		if (mode->runBox() != 0) {
			mode->_08 = 0;
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// A card error that goes back to the page that showed it once read
#define CARD_ERROR(name, id)                                     \
	static void name(TModeResult* mode, int phase)               \
	{                                                            \
		switch (phase) {                                         \
		case RESULT_ENTER:                                       \
			mode->openMessage(getStageSelectWarning(id));        \
			break;                                               \
		case RESULT_UPDATE:                                      \
			if (mode->checkCard() == 0 && mode->runBox() != 0) { \
				mode->setState(mode->mBackState);                \
			}                                                    \
			break;                                               \
		case RESULT_DRAW:                                        \
			break;                                               \
		case RESULT_EXIT:                                        \
			break;                                               \
		}                                                        \
	}

// 0x801B9BC8
CARD_ERROR(stateError24, 0xD)
// 0x801B9C4C
CARD_ERROR(stateError25, 0x1A)

// 0x801B9CD0
// The save failed the mode ends
static void stateFailed(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openNotice(getStageSelectWarning(0x18));
		break;
	case RESULT_UPDATE:
		if (mode->runBox() != 0) {
			mode->_08 = 0;
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B9D40
// Not saving the mode ends
static void stateNoSave(TModeResult* mode, int phase)
{
	switch (phase) {
	case RESULT_ENTER:
		mode->openMessage(getStageSelectWarning(0x16));
		break;
	case RESULT_UPDATE:
		if (mode->runBox() != 0) {
			mode->_08 = 0;
		}
		break;
	case RESULT_DRAW:
		break;
	case RESULT_EXIT:
		break;
	}
}

// 0x801B9DB0
void TModeResult::showOptions()
{
	Vec3f box = vec3f(0.0f, 24.0f, 0.0f);
	if (mOption[0] != 0) {
		mOption[0]->mFlags &= ~0x10;
		mOption[0]->mAt  = sYesAt;
		mOption[0]->mBox = box;
		mOption[0]->_A8  = 1;
	}
	if (mOption[1] != 0) {
		mOption[1]->mFlags &= ~0x10;
		mOption[1]->mAt  = sNoAt;
		mOption[1]->mBox = box;
		mOption[1]->_A8  = 1;
	}
}

// 0x801B9EA8
void TModeResult::hideOptions()
{
	if (mOption[0] != 0) {
		mOption[0]->mFlags |= 0x10;
	}
	if (mOption[1] != 0) {
		mOption[1]->mFlags |= 0x10;
	}
}
