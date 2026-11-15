#include "ui/TAeObjPause.h"
struct DrawTask;
#include "ui/TextVec.h"
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "core/Pad.h"
#include "ui/TSimpleText.h"
#include "ui/TAeObjMsgWindow.h"
#include "ui/TAeObjPlayRest.h"
#include "ui/TFade.h"
#include "ui/AeObjControl.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "stage/TStageSelectModel.h"
#include "sound/SoundCtrl.h"
#include "object/orange/TObjOrangeMapMirror.h"
#include "ui/TAeObjCoinNum.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;
struct TObject;
struct Vec3f;

// TAeObjPause class at 0x80124494 Vtable 0x8025E398
// draw() calls 0x80124C90 0x80124D24 0x801251C0 0x8012532C and 0x80125398 in this run

TArc* sPauseArc;
void* sPauseAe;
NJS_TEXLIST* sPauseTexList;
u32 sPausePulse;
s8 sPauseReady;

extern "C" {
const u32 gPauseRow0Anims[4] = { 0x38, 0x36, 0x36, 0x36 };
const u32 gPauseRow1Anims[4] = { 0x3A, 0x3C, 0x3A, 0x3A };
const u32 gPauseRow2Anims[4] = { 0x42, 0x42, 0x44, 0x42 };
const u32 gPauseRow3Anims[4] = { 0x3E, 0x3E, 0x3E, 0x40 };
}

const int gRestCharCells[4] = { 0x74, 0x76, 0x78, 0x7A };
const int gPausePadCells[4] = { 0x4F, 0x51, 0x53, 0x55 };
const int gAeDigitCells[10] = { 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C };

// The three class names come before the archive name in .rodata
const char* gTAeObjPauseName = "TAeObjPause";
const char* gTAeObjPlayRestName = "TAeObjPlayRest";
const char* gTAeObjCoinNumName = "TAeObjCoinNum";
static inline Vec3f vec3(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// 0x80282808
// TAeObjPause's two lines of text their box their glyph scale and where each starts
static TextVec gPauseTextBox   = textVec(480.0f, 18.0f, 0.0f);
static TextVec gPauseTextScale = textVec(0.91f, 0.91f, 0.91f);
static TextVec gPauseText0At   = textVec(512.0f, 320.0f, -2.0f);
static TextVec gPauseText1At   = textVec(512.0f, 373.0f, -2.0f);

typedef char TAeObjPause_size_check[(sizeof(TAeObjPause) == 0x9C) ? 1 : -1];

// 0x80124334
extern "C" void loadPause()
{
	loadArc(getLangFileName("ae_pause.arc"), &sPauseArc);
	if (sPauseArc != 0) {
		AeArcBlock* block = aeArcBlock(sPauseArc);
		sPauseAe = block->mAe;
		relocateAe(sPauseAe);
		sPauseTexList = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, sPauseTexList);
		u8* buf = new u8[0x800];
		if (block->mPalettes != 0) {
			loadE3Palette(block->mPalettes[0], (E3Palette*)&buf, gPauseBanks[0]);
		}
		delete[] buf;
		njSetTexture(sPauseTexList);
		njSetPaletteBankNum(0, gPauseBanks[0]);
		GXInvalidateTexAll();
	}
}

// 0x801243E0
extern "C" void freePause()
{
	if (sPauseArc != 0) {
		releaseTextures((NJS_TEXLIST*)sPauseTexList);
		sPauseTexList = 0;
		releaseResource(sPauseArc);
		sPauseArc = 0;
		sPauseAe = 0;
	}
}

// 0x8012442C
extern "C" TAeObjPause* createPause(TObject* parent)
{
	TAeObjPause* made;

	if (sPauseArc == 0) {
		return 0;
	}
	made = new TAeObjPause(parent, sPauseAe, sPauseTexList);
	return made;
}

static inline void mark(TObject* obj)
{
	if (obj != 0) {
		obj->mFlags |= 0x10;
	}
}

#define PAUSE_PULSE(i)                                       \
	if (mRow[i] != 0) {                                      \
		mRow[i]->update();                                   \
		if (mCursor == i) {                                  \
			mColor[1] = 0.5f * njCos(sPausePulse) + 0.5f;    \
			mColor[1] = 240.0f * mColor[1];                  \
			mColor[1] = mColor[1] / 255.0f;                  \
			mColor[2] = 0.9411765f;                          \
			mColor[3] = 0.15686275f;                         \
		} else {                                             \
			mColor[1] = mColor[2] = mColor[3] = 1.0f;        \
		}                                                    \
		mRow[i]->setColor(mColor);                           \
	}

// 0x80124494
TAeObjPause::TAeObjPause(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	f32 pos[3];
	f32 scale[3];
	f32 color[3];
	int i;

	_00     = (u32)gTAeObjPauseName;
	mSize   = 0x9C;
	mBase  = createLayer(0x83, 0x10);
	if (mBase != 0) {
		mBase->mBanks = gPauseBanks;
	}
	mTitle = createLayer(0x32, 0x10);
	if (mTitle != 0) {
		mTitle->mBanks = gPauseBanks;
	}
	mRow[0] = createLayer(0x36, 0);
	if (mRow[0] != 0) {
		mRow[0]->mBanks = gPauseBanks;
	}
	mRow[1] = createLayer(0x3A, 0);
	if (mRow[1] != 0) {
		mRow[1]->mBanks = gPauseBanks;
	}
	mRow[2] = createLayer(0x42, 0);
	if (mRow[2] != 0) {
		mRow[2]->mBanks = gPauseBanks;
	}
	mRow[3] = createLayer(0x3E, 0);
	if (mRow[3] != 0) {
		mRow[3]->mBanks = gPauseBanks;
	}
	if (mBase != 0) {
		mBase->attach(0x48, mTitle, 0);
		mBase->attach(0x4C, mRow[0], 0);
		mBase->attach(0x4B, mRow[1], 0);
		mBase->attach(0x4A, mRow[2], 0);
		mBase->attach(0x49, mRow[3], 0);
	}
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3E, 0);
	mState   = 0;
	mCursor   = 0;
	mResult   = -1;
	mDone   = 0;
	mColor[0]   = 1.0f;
	mColor[1]   = 0.9411765f;
	mColor[2]   = 0.9411765f;
	mColor[3]   = 0.15686275f;
	mHint   = 0;
	for (i = 0; i < 2; i++) {
		mText[i] = 0;
	}
	mWindow = 0;
	mChoice = 1;
	for (i = 0; i < PAUSE_ROWS; i++) {
		mDisabled[i] = 0;
	}
	mPad = 0;
	mCoins = 0;
	alphaFadeDim(0x3C);
	mText[0] = createSimpleText(this, 1, -1, &gPauseText0At, &gPauseTextBox, &gPauseTextScale, getStageSelectWarning(0x27));
	mark(mText[0]);
	mText[1] = createSimpleText(this, 1, -1, &gPauseText1At, &gPauseTextBox, &gPauseTextScale, getStageSelectWarning(0x28));
	mark(mText[1]);
	color[0] = 0.91f;
	color[1] = 0.91f;
	color[2] = 0.91f;
	scale[0] = 330.0f;
	scale[1] = 132.0f;
	scale[2] = 0.0f;
	pos[0]   = 153.0f;
	pos[1]   = 190.0f;
	pos[2]   = -1.0f;
	mHint = createSimpleText((TObject*)this, 1, -1, (const Vec3f*)pos, (const Vec3f*)scale, (const Vec3f*)color, (void*)getStageSelectWarning(0x2A));
	mark(mHint);
}

// 0x80124850
TAeObjPause::~TAeObjPause()
{
	alphaFadeDelete();
	for (int i = 0; i < 2; i++) {
		if (mText[i] != 0) {
			mText[i]->mFlags |= 1;
			mText[i]                   = 0;
		}
	}
	if (mWindow != 0) {
		mWindow->mFlags |= 1;
		mWindow                   = 0;
	}
	removeDrawTask(mTask);
}

// 0x80124914
void TAeObjPause::draw()
{
	if (sPauseReady == 0) {
		sPausePulse = 0;
		sPauseReady = 1;
	}
	if (mTitle != 0) {
		mTitle->update();
	}
	switch (mState) {
	case 0:
		stateOpen();
		break;
	case 1:
		stateSelect();
		PAUSE_PULSE(0);
		PAUSE_PULSE(1);
		PAUSE_PULSE(2);
		PAUSE_PULSE(3);
		sPausePulse += 0x200;
		sPausePulse &= 0xFFFF;
		break;
	case 2:
		stateConfirm();
		break;
	case 3:
		stateWait();
		break;
	case 4:
		stateClose();
		break;
	}
	mColor[1] = 0.5f;
	mColor[2] = 0.5f;
	mColor[3] = 0.5f;
	if (mDisabled[0] != 0) {
		mRow[0]->setColor(mColor);
	}
	if (mDisabled[1] != 0) {
		mRow[1]->setColor(mColor);
	}
	if (mDisabled[2] != 0) {
		mRow[2]->setColor(mColor);
	}
	if (mDisabled[3] != 0) {
		mRow[3]->setColor(mColor);
	}
	if (mState != 4) {
		aeShowPlayRest();
		aeSetCoinNum(mCoins & 0xFFFF);
	}
}

// The animation each row plays for each cursor position lit on its own dim on the others
// The backdrop's cell for each player's pad

#define PAUSE_SHOW_CURSOR()                                   \
	sePlay(0x1B);                                             \
	mRow[0]->reset(gPauseRow0Anims[mCursor], 1);                     \
	mRow[1]->reset(gPauseRow1Anims[mCursor], 1);                     \
	mRow[2]->reset(gPauseRow2Anims[mCursor], 1);                     \
	mRow[3]->reset(gPauseRow3Anims[mCursor], 1);

// 0x80124C90
void TAeObjPause::stateOpen()
{
	if (mBase != 0) {
		if (mBase->isDone()) {
			mRow[0]->reset(0x38, 1);
			mRow[1]->reset(0x3A, 1);
			mRow[3]->reset(0x3E, 1);
			mState = PAUSE_SELECT;
		} else {
			mBase->update();
		}
	}
}

// 0x80124D24
// Up and down skip the disabled rows and wrap
void TAeObjPause::stateSelect()
{
	if (mResult == -1) {
		u32 pad = mPad;
		s8 i;

		if ((padGet(pad)->mRepeat & 0x100000) || (padGet(pad)->mRepeat & 0x8)) {
			for (i = mCursor - 1; i >= -1; i--) {
				if (i == -1) {
					for (i = 3; i > mCursor; i--) {
						if (mDisabled[i] == 0) {
							mCursor = (s8)i;
							break;
						}
					}
					break;
				}
				if (mDisabled[i] == 0) {
					mCursor = i;
					break;
				}
			}
			PAUSE_SHOW_CURSOR();
		} else if ((padGet(pad)->mRepeat & 0x200000) || (padGet(pad)->mRepeat & 0x4)) {
			for (i = mCursor + 1; i <= 4; i++) {
				if (i == 4) {
					for (i = 0; i < mCursor; i++) {
						if (mDisabled[i] == 0) {
							mCursor = (s8)i;
							break;
						}
					}
					break;
				}
				if (mDisabled[i] == 0) {
					mCursor = i;
					break;
				}
			}
			PAUSE_SHOW_CURSOR();
		} else if (padGet(pad)->mPressed & 0x100) {
			if (mCursor == PAUSE_QUIT) {
				if (mHint != 0) {
					mHint->mFlags |= 1;
					mHint = 0;
				}
				if (mHint == 0) {
					mHint = createSimpleTextByKind(this, getStageSelectWarning(0x2A), 3);
					if (mHint != 0) {
						mHint->_A8 = 1;
					}
				}
				Vec3f size = vec3(0.0f, 24.0f, 0.0f);
				if (mWindow == 0) {
					mWindow = createMsgWindow(this, 9);
					if (mWindow != 0) {
						mWindow->setPrompt(3);
						mWindow->setCursor(mChoice);
					}
				}
				if (mText[0] != 0) {
					mText[0]->mFlags &= ~0x10;
					mText[0]->mBox = size;
					mText[0]->_A8  = 1;
				}
				if (mText[1] != 0) {
					mText[1]->mFlags &= ~0x10;
					mText[1]->mBox = size;
					mText[1]->_A8  = 1;
				}
				mTask->mFlags |= 1;
				mChoice = 1;
				mWindow->setCursor(mChoice);
				mState = PAUSE_CONFIRM;
			} else {
				mResult = mCursor;
				sePlay(0x1C);
				close();
			}
		} else if ((padGet(pad)->mPressed & 0x200) || (padGet(pad)->mPressed & 0x1000)) {
			mResult = 0;
			sePlay(0x20);
			close();
		}
	}
}

// 0x801251C0
void TAeObjPause::stateConfirm()
{
	u32 pad = mPad;

	if ((padGet(pad)->mRepeat & 0x100000) || (padGet(pad)->mRepeat & 0x8)) {
		sePlay(0x1B);
		if (mChoice > 0) {
			mChoice--;
		} else {
			mChoice = 1;
		}
		if (mWindow != 0) {
			mWindow->setCursor(mChoice);
		}
	} else if ((padGet(pad)->mRepeat & 0x200000) || (padGet(pad)->mRepeat & 0x4)) {
		sePlay(0x1B);
		if (mChoice < 1) {
			mChoice++;
		} else {
			mChoice = 0;
		}
		if (mWindow != 0) {
			mWindow->setCursor(mChoice);
		}
	}
	if (padGet(pad)->mPressed & 0x100) {
		if (mChoice == 0) {
			sePlay(0x1C);
			closeConfirm();
			mResult = mCursor;
		} else {
			sePlay(0x20);
			closeConfirm();
		}
	} else if (padGet(pad)->mPressed & 0x200) {
		sePlay(0x20);
		closeConfirm();
	}
}

// 0x8012532C
void TAeObjPause::stateWait()
{
	if (mWindow != 0 && mWindow->mState == TAeObjMsgWindow::MSGWIN_CLOSED) {
		if (mWindow != 0) {
			mWindow->mFlags |= 1;
			mWindow = 0;
		}
		if (mResult == PAUSE_QUIT) {
			close();
		} else {
			mState = PAUSE_SELECT;
		}
	}
}

// 0x80125398
void TAeObjPause::stateClose()
{
	if (mBase != 0) {
		if (mBase->isDone()) {
			mDone = TRUE;
		} else {
			mBase->update();
		}
	}
}

// 0x801253F0
void TAeObjPause::close()
{
	alphaFadeClear();
	aeHidePlayRest();
	aeHideCoinNum();
	if (mBase != 0) {
		mBase->mPlayFlags &= ~0x500;
		mBase->setMode(0x20);
	}
	if (mTitle != 0) {
		mTitle->mPlayFlags &= ~0x500;
		mTitle->setMode(0x20);
	}
	mColor[0] = 1.0f;
	mColor[1] = 1.0f;
	mColor[2] = 1.0f;
	mColor[3] = 1.0f;
	if (mRow[0] != 0) {
		mRow[0]->setColor(mColor);
	}
	if (mRow[1] != 0) {
		mRow[1]->setColor(mColor);
	}
	if (mRow[2] != 0) {
		mRow[2]->setColor(mColor);
	}
	if (mRow[3] != 0) {
		mRow[3]->setColor(mColor);
	}
	mState = PAUSE_CLOSE;
}

// 0x801254F8
void TAeObjPause::closeConfirm()
{
	if (mHint != 0) {
		mHint->mHold = 1;
		mHint        = 0;
	}
	if (mWindow != 0) {
		mWindow->close();
	}
	if (mText[0] != 0) {
		mText[0]->mFlags |= 0x10;
	}
	if (mText[1] != 0) {
		mText[1]->mFlags |= 0x10;
	}
	mTask->mFlags &= ~1;
	mState = PAUSE_WAIT;
}

// 0x80125594
void TAeObjPause::render()
{
	if (mBase != 0) {
		mBase->present();
	}
}

// 0x801255C0
void TAeObjPause::setPad(int pad)
{
	mPad = pad;
	if (mBase != 0) {
		mBase->setCell(0x4D, gPausePadCells[pad]);
	}
}

// 0x80125608
// The play-rest counter is in the pause archive
extern "C" TAeObjPlayRest* createPlayRest(TObject* parent)
{
	if (sPauseArc == 0) {
		return 0;
	}
	return new TAeObjPlayRest(parent, sPauseAe, sPauseTexList);
}

#include "ui/TAeObjPlayRest.h"
#include "engine/DrawTask.h"
#include "ui/TAeObjCoinNum.h"
#include "egg/EggCreature.h"
#include "ui/AeObjControl.h"
#include "object/orange/TObjOrangeMapMirror.h"
#include "ui/TAeObjPause.h"

typedef char TAeObjPlayRest_size_check[(sizeof(TAeObjPlayRest) == 0x44) ? 1 : -1];

// 0x80125670
TAeObjPlayRest::TAeObjPlayRest(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00    = (u32)gTAeObjPlayRestName;
	mSize  = 0x44;
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3D, 0);
	mLayer = createLayer(0x85, 0x10);
	if (mLayer != 0) {
		mLayer->mBanks = gPauseBanks;
	}
}

// 0x80125700
TAeObjPlayRest::~TAeObjPlayRest()
{
	aeForgetPlayRest(this);
	removeDrawTask(mTask);
}

// 0x8012576C
void TAeObjPlayRest::draw()
{
	if (mTimer == 0) {
		hide();
	} else {
		mTimer = mTimer - 1;
	}
	if (mLayer != 0) {
		mLayer->play();
	}
}

// 0x801257C0
void TAeObjPlayRest::render()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}

// 0x801257EC
int TAeObjPlayRest::onRest(s16* code)
{
	PlayRestMsg* msg = (PlayRestMsg*)code;
	if (msg->mPlayer >= 4) {
		return 0;
	}
	setRest(msg->mRest, msg->mChar);
	return 1;
}

// 0x80125830
void TAeObjPlayRest::setRest(u16 rest, u16 chara)
{
	if (mLayer != 0) {
		mTimer = 300;
		mChar  = chara > 3 ? 3 : chara;
		mRest  = rest > 99 ? 99 : rest;
		if (mLayer != 0) {
			int tens = mRest / 10;
			int ones = mRest % 10;
			mLayer->setCell(0x7E, gAeDigitCells[tens]);
			mLayer->setCell(0x7F, gAeDigitCells[ones]);
			mLayer->setCell(0x7C, gRestCharCells[mChar]);
			show();
		}
	}
}

// 0x80125924
void TAeObjPlayRest::show()
{
	if (mLayer != 0) {
		mTimer        = 300;
		mLayer->_A8   = 1.0f;
		mLayer->wake();
	}
}

// 0x8012596C
void TAeObjPlayRest::hide()
{
	if (mLayer != 0) {
		mTimer      = 0;
		mLayer->_A8 = 0.0f;
	}
}

// 0x80125990
// The coin counter from the pause screen's archive
extern "C" TAeObjCoinNum* createCoinNum(TObject* parent)
{
	if (sPauseArc == 0) {
		return 0;
	}
	return new TAeObjCoinNum(parent, sPauseAe, sPauseTexList);
}

#include "ui/TAeObjCoinNum.h"
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "ui/AeObjControl.h"
#include "object/orange/TObjOrangeMapMirror.h"
#include "ui/TAeObjPlayRest.h"
struct DrawTask;

typedef char TAeObjCoinNum_size_check[(sizeof(TAeObjCoinNum) == 0x40) ? 1 : -1];

// 0x801259F8
TAeObjCoinNum::TAeObjCoinNum(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00    = (u32)gTAeObjCoinNumName;
	mSize  = 0x40;
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3D, 0);
	mLayer = 0;
}

// 0x80125A68
TAeObjCoinNum::~TAeObjCoinNum()
{
	aeForgetCoinNum(this);
	removeDrawTask(mTask);
}

// 0x80125AD4
void TAeObjCoinNum::draw()
{
	if (mTimer == 0) {
		hide();
	} else {
		mTimer = mTimer - 1;
	}
	if (mLayer != 0) {
		mLayer->play();
	}
}

// 0x80125B28
void TAeObjCoinNum::render()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}

// 0x80125B54
void TAeObjCoinNum::setCount(u32 count)
{
	if (mLayer != 0) {
		mTimer = 120;
		u16 shown = (u16)count > 99 ? 99 : (u16)count;
		if (mLayer != 0) {
			mLayer->setCell(0x17, gAeDigitCells[shown / 10]);
			mLayer->setCell(0x18, gAeDigitCells[shown % 10]);
			mLayer->setCell(0x15, 2);
		}
	}
}

// 0x80125C1C
void TAeObjCoinNum::show(u32 count)
{
	if (mLayer == 0) {
		mLayer = createLayer(0x81, 0x10);
		if (mLayer != 0) {
			mLayer->mBanks = gPauseBanks;
		}
	}
	setCount(count);
	mTimer      = 120;
	mLayer->_A8 = 1.0f;
	mLayer->wake();
}

// 0x80125CA4
void TAeObjCoinNum::hide()
{
	if (mLayer != 0) {
		mTimer      = 0;
		mLayer->_A8 = 0.0f;
	}
}

// 0x80125CC8
// is the static init
