#include "ui/TAeObjMsgWindow.h"
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "sound/SoundCtrl.h"
#include "ui/AeUtil.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

// TAeObjMsgWindow class at 0x800E9EA4 Vtable 0x8025B7C8

// 0x800E9FC4
// builds the layers 0x800EB0E8 and 0x800D2FD4 run from draw()
// The loader starts at 0x800E9CE4

// The tables are this file's draw() reaches the pulse colours from gMsgWindowBanks the start of its .rodata
extern "C" {
const u32 gMsgWindowBanks[3]   = { 0x10, 0x11, 0x12 };
const u32 gMsgPromptSlots[4]   = { 0x54, 0x53, 0x52, 0x55 };
const u32 gMsgChoice3Slots[3]  = { 0x26, 0x25, 0x24 };
const MsgColor gMsgCursorColor = { 1.0f, 0.94f, 0.94f, 0.16f };
const MsgColor gMsgRowColor    = { 1.0f, 1.0f, 1.0f, 1.0f };
const MsgColor gMsgMarkColor   = { 1.0f, 0.5f, 0.5f, 0.5f };
// What the pulse moves the cursor row's tint between and the small box's second panel
const f32 gMsgPulseWhite[4]   = { 1.0f, 1.0f, 1.0f, 0.0f };
const f32 gMsgPulseMagenta[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
const f32 gMsgPulseOrange[4]  = { 1.0f, 0.88f, 0.45f, 0.0f };
}

// Defined ahead of the loader its string comes before the archive name
const char* gTAeObjMsgWindowName = "TAeObjMsgWindow";

typedef char TAeObjMsgWindow_size_check[(sizeof(TAeObjMsgWindow) == 0x74) ? 1 : -1];

// 0x800E9CE4
extern "C" void loadMsgWindow()
{
	loadArc("ae_msg_window.arc", &sMsgWindowArc);
	if (sMsgWindowArc != 0) {
		AeArcBlock* block = aeArcBlock(sMsgWindowArc);
		sMsgWindowAe = block->mAe;
		relocateAe(sMsgWindowAe);
		sMsgWindowTexList = block->mTexList;
		bindGvmTextures((u8*)block->mGvm, sMsgWindowTexList);
		u8* buf = new u8[0x800];
		if (block->mPalettes != 0) {
			loadE3Palette(block->mPalettes[0], (E3Palette*)&buf, gMsgWindowBanks[0]);
			loadE3Palette(block->mPalettes[1], (E3Palette*)&buf, gMsgWindowBanks[1]);
			loadE3Palette(block->mPalettes[2], (E3Palette*)&buf, gMsgWindowBanks[2]);
		}
		delete[] buf;
		njSetTexture(sMsgWindowTexList);
		njSetPaletteBankNum(0, gMsgWindowBanks[0]);
		njSetPaletteBankNum(1, gMsgWindowBanks[1]);
		njSetPaletteBankNum(2, gMsgWindowBanks[2]);
		GXInvalidateTexAll();
	}
}

// 0x800E9DFC
extern "C" void freeMsgWindow()
{
	if (sMsgWindowArc != 0) {
		releaseTextures((NJS_TEXLIST*)sMsgWindowTexList);
		sMsgWindowTexList = 0;
		releaseResource(sMsgWindowArc);
		sMsgWindowArc = 0;
		sMsgWindowAe = 0;
	}
}

// 0x800E9E48
extern "C" TAeObjMsgWindow* createMsgWindow(TObject* parent, u32 kind)
{
	TAeObjMsgWindow* made;

	made = new TAeObjMsgWindow(parent, sMsgWindowAe, sMsgWindowTexList, kind);
	return made;
}

// 0x800E9EA4
TAeObjMsgWindow::TAeObjMsgWindow(TObject* parent, void* data, void* assets, u32 kind)
    : TAeObject(parent, data, assets)
{
	mKind  = kind;
	_00    = (u32)gTAeObjMsgWindowName;
	mSize  = 0x74;
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3E, 0);
	mWindow = 0;
	mArrow    = 0;
	mChoices    = 0;
	mPanelA    = 0;
	mPanelB    = 0;
	mCursorRow    = 0;
	mRow    = 0;
	mMark    = 0;
	mIcon    = 0;
	mDieOnClose    = 0;
	mArrowState    = 2;
	mPulse    = 0.0f;
	mPulseAngle    = 0;
	build();
}

// 0x800E9F5C
TAeObjMsgWindow::~TAeObjMsgWindow() { removeDrawTask(mTask); }

// The window's slot for the choice panel by the prompt picked with setPrompt
// The choice panel's slot for each row of a three-way choice
// The choice panel's slot for each row of a yes/no choice

static inline void make(TAeObjMsgWindow* self, TAeLayer*& slot, int id, u32 flags)
{
	slot = self->createLayer(id, flags);
	if (slot != 0) {
		slot->mBanks = gMsgWindowBanks;
	}
}

// A macro over the caller's colour buffer every tint in a function uses the same one
#define tint(buf, layer, color)                                                                                \
	if ((layer) != 0) {                                                                                     \
		buf[0] = (color).v[0];                                                                            \
		buf[1] = (color).v[1];                                                                            \
		buf[2] = (color).v[2];                                                                            \
		buf[3] = (color).v[3];                                                                            \
		(layer)->setColor(buf);                                                                           \
	}

// 0x800E9FC4
// Builds the layers for the kind of window
void TAeObjMsgWindow::build()
{
	switch (mKind) {
	case MSG_TALK_SE:
		sePlay(0x1E);
	case MSG_TALK:
		makeTalk();
		break;
	case MSG_NOTICE_ICON:
		makeNotice();
		make(this, mIcon, 2, 0);
		if (mWindow != 0 && mIcon != 0) {
			mWindow->bind(0x4F, mIcon, 0);
		}
		break;
	case MSG_NOTICE:
		makeNotice();
		if (mWindow != 0) {
			mWindow->setCell(0x4F, 0);
		}
		break;
	case MSG_NOTICE_5:
		makeNotice();
		if (mWindow != 0) {
			mWindow->setCell(0x4F, 5);
		}
		break;
	case MSG_NOTICE_7:
		makeNotice();
		if (mWindow != 0) {
			mWindow->setCell(0x4F, 7);
		}
		break;
	case MSG_NOTICE_9:
		makeNotice();
		if (mWindow != 0) {
			mWindow->setCell(0x4F, 9);
		}
		break;
	case MSG_YES_NO:
		makeYesNo();
		break;
	case MSG_CHOICE3:
		makeChoice3();
		break;
	case MSG_YES_NO_SMALL:
		makeYesNoSmall();
		break;
	}
}

// 0x800EA12C
// The talk box and its next-page arrow
void TAeObjMsgWindow::makeTalk()
{
	make(this, mWindow, 0x57, 0x10);
	make(this, mArrow, 0x42, 0);
	mState  = MSGWIN_OPENING;
}

// 0x800EA1B0
void TAeObjMsgWindow::makeNotice()
{
	sePlay(0x1E);
	make(this, mWindow, 0x4E, 0x10);
	make(this, mArrow, 0xB, 0);
	mState  = MSGWIN_OPENING;
}

// 0x800EA240
// The choice box and the layers its rows are made of
void TAeObjMsgWindow::makeChoices()
{
	f32 color[4];

	sePlay(0x23);
	make(this, mWindow, 0x52, 0x10);
	make(this, mChoices, 0x20, 0);
	make(this, mPanelA, 0x28, 0x10);
	make(this, mPanelB, 0x36, 0x10);
	make(this, mCursorRow, 0x30, 0x10);
	tint(color, mCursorRow, gMsgCursorColor);
	make(this, mRow, 0x30, 0x10);
	tint(color, mRow, gMsgRowColor);
	make(this, mMark, 0x30, 0x10);
	tint(color, mMark, gMsgMarkColor);
}

// 0x800EA444
void TAeObjMsgWindow::makeYesNo()
{
	makeChoices();
	if (mChoices != 0) {
		mChoices->bind(0x20, mPanelA, 0);
		mChoices->bind(0x21, mPanelB, 0);
		mChoices->bind(0x23, mRow, 0);
		mChoices->bind(0x22, mRow, 0);
	}
	mState = MSGWIN_OPENING;
}

// 0x800EA510
void TAeObjMsgWindow::makeChoice3()
{
	makeChoices();
	if (mChoices != 0) {
		mChoices->bind(0x20, mPanelA, 0);
		mChoices->bind(0x21, mPanelB, 0);
		mChoices->bind(0x26, mRow, 0);
		mChoices->bind(0x25, mRow, 0);
		mChoices->bind(0x24, mRow, 0);
	}
	mState = MSGWIN_OPENING;
}

// 0x800EA600
// A yes/no box without the opening sound or the mark
void TAeObjMsgWindow::makeYesNoSmall()
{
	f32 color[4];

	make(this, mWindow, 0x52, 0x10);
	make(this, mChoices, 0x20, 0);
	make(this, mPanelA, 0x2C, 0x10);
	make(this, mPanelB, 0x36, 0x10);
	make(this, mCursorRow, 0x30, 0x10);
	tint(color, mCursorRow, gMsgCursorColor);
	make(this, mRow, 0x30, 0x10);
	tint(color, mRow, gMsgRowColor);
	if (mChoices != 0) {
		mChoices->bind(0x20, mPanelA, 0);
		mChoices->bind(0x21, mPanelB, 0);
		mChoices->bind(0x23, mRow, 0);
		mChoices->bind(0x22, mRow, 0);
	}
	mState = MSGWIN_OPENING;
}

// 0x800EA834
// Puts the choice panel in the window's slot for a prompt
void TAeObjMsgWindow::setPrompt(int prompt)
{
	if ((mKind == MSG_YES_NO || mKind == MSG_CHOICE3 || mKind == MSG_YES_NO_SMALL) && mWindow != 0) {
		mWindow->bind(gMsgPromptSlots[prompt], mChoices, 0);
	}
}

// 0x800EA8A4
// Lights the row under the cursor
void TAeObjMsgWindow::setCursor(int cursor)
{
	int i;

	switch (mKind) {
	case MSG_YES_NO:
		for (i = 0; i < 2; i++) {
			if (i == cursor) {
				mChoices->bind(gMsgYesNoSlots[i], mCursorRow, 0);
			} else {
				mChoices->bind(gMsgYesNoSlots[i], mRow, 0);
			}
		}
		break;
	case MSG_CHOICE3:
		for (i = 0; i < 3; i++) {
			if (i == cursor) {
				mChoices->bind(gMsgChoice3Slots[i], mCursorRow, 0);
			} else {
				mChoices->bind(gMsgChoice3Slots[i], mRow, 0);
			}
		}
		break;
	case MSG_YES_NO_SMALL:
		for (i = 0; i < 2; i++) {
			if (i == cursor) {
				mChoices->bind(gMsgYesNoSlots[i], mCursorRow, 0);
			} else {
				mChoices->bind(gMsgYesNoSlots[i], mRow, 0);
			}
		}
		break;
	}
}

// 0x800EAA64
// Marks a row for one already picked
void TAeObjMsgWindow::setMark(int row)
{
	switch (mKind) {
	case MSG_YES_NO:
		mChoices->bind(gMsgYesNoSlots[row], mMark, 0);
		break;
	case MSG_CHOICE3:
		mChoices->bind(gMsgChoice3Slots[row], mMark, 0);
		break;
	case MSG_YES_NO_SMALL:
		mChoices->bind(gMsgYesNoSlots[row], mMark, 0);
		break;
	}
}

// A macro close() reads the member again for each step
#define playOut(layer)                                                                                         \
	if ((layer) != 0) {                                                                                     \
		(layer)->setMode(0x20);                                                                           \
		(layer)->mPlayFlags &= ~0x500;                                                                    \
		(layer)->wake();                                                                                  \
	}

// 0x800EAB38
// Plays every layer out mState goes on to MSGWIN_CLOSED once the window has
BOOL TAeObjMsgWindow::close()
{
	switch (mKind) {
	case MSG_TALK_SE:
	case MSG_NOTICE_ICON:
	case MSG_NOTICE:
	case MSG_NOTICE_5:
	case MSG_NOTICE_7:
	case MSG_NOTICE_9:
		sePlay(0x1F);
		break;
	case MSG_YES_NO:
	case MSG_CHOICE3:
	case MSG_YES_NO_SMALL:
		sePlay(0x24);
		break;
	}
	if (mWindow != 0) {
		mWindow->setMode(0x20);
		mWindow->mPlayFlags &= ~0x500;
		mWindow->clearSlots();
		mWindow->wake();
	}
	playOut(mChoices);
	playOut(mPanelA);
	playOut(mPanelB);
	playOut(mCursorRow);
	playOut(mRow);
	mState = MSGWIN_CLOSING;
	return TRUE;
}

// 0x800EAD08
// Starts the next-page arrow's bounce
void TAeObjMsgWindow::playArrow()
{
	if (mArrowState != ARROW_HIDDEN && mArrow != 0) {
		if (mKind == MSG_TALK_SE) {
			mArrow->reset(0x44, 0x10);
		} else if ((u32)(mKind - MSG_NOTICE) <= 2 || mKind == MSG_NOTICE_9) {
			mArrow->reset(0xD, 0x10);
		}
		mArrow->mBanks = gMsgWindowBanks;
		mArrowState    = ARROW_PLAYING;
	}
}

// 0x800EADA4
// Puts the next-page arrow in the window
void TAeObjMsgWindow::showArrow()
{
	if (mArrowState == ARROW_HIDDEN && mWindow != 0 && mArrow != 0) {
		if (mKind == MSG_TALK_SE) {
			mArrow->reset(0x42, 1);
			mArrow->mBanks = gMsgWindowBanks;
			mWindow->bind(0x58, mArrow, 0);
		} else if ((u32)(mKind - MSG_NOTICE) <= 2 || mKind == MSG_NOTICE_9) {
			mArrow->reset(0xB, 1);
			mArrow->mBanks = gMsgWindowBanks;
			mWindow->bind(0x50, mArrow, 0);
		}
		mArrowState = ARROW_SHOWN;
	}
}

// 0x800EAEA4
void TAeObjMsgWindow::hideArrow()
{
	if (mArrowState != ARROW_PLAYING && mWindow != 0) {
		mWindow->clearSlot(0x58);
		mArrowState = ARROW_HIDDEN;
	}
}

// 0x800EAEF8
void TAeObjMsgWindow::draw()
{
	f32 boxA[4];
	f32 boxB[4];

	pulse(0x300);
	lerp4f(mPulse, boxA, gMsgPulseWhite, gMsgPulseMagenta);
	lerp4f(mPulse, boxB, gMsgPulseOrange, gMsgPulseWhite);
	if (mWindow != 0) {
		mWindow->update();
		if (mWindow->isDone() != 0) {
			if (mState == MSGWIN_OPENING) {
				mState = MSGWIN_OPEN;
			} else if (mState == MSGWIN_CLOSING) {
				mState = MSGWIN_CLOSED;
				if (mDieOnClose != 0) {
					mFlags = (u16)(mFlags | 1);
				}
			}
		}
	}
	if (mChoices != 0) {
		mChoices->update();
	}
	if (mPanelA != 0) {
		mPanelA->update();
	}
	if (mPanelB != 0) {
		if (mKind == MSG_YES_NO_SMALL) {
			mPanelB->setColor(boxB);
		}
		mPanelB->update();
	}
	if (mCursorRow != 0) {
		mCursorRow->setColor(boxA);
		mCursorRow->update();
	}
	if (mRow != 0) {
		mRow->update();
	}
	if (mMark != 0) {
		mMark->update();
	}
	if (mIcon != 0) {
		mIcon->update();
	}
	if (mArrow != 0) {
		if (mArrow->getId() == 0x44 || mArrow->getId() == 0xD) {
			if (mArrow->isDone() != 0 && mArrowState == 0) {
				mArrowState = 2;
			}
		}
		mArrow->update();
	}
}

// 0x800EB0B8
void TAeObjMsgWindow::vfn_10() { }

// 0x800EB0BC
void TAeObjMsgWindow::render()
{
	if (mWindow != 0) {
		mWindow->present();
	}
}

// 0x800EB0E8
// Steps the cursor's pulse 0 to 1
void TAeObjMsgWindow::pulse(int step)
{
	mPulseAngle += step;
	mPulseAngle = (u16)mPulseAngle;
	f32 c       = njCos(mPulseAngle);
	mPulse      = sMsgWindowHalf * c + sMsgWindowHalf;
}
