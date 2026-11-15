#include "ui/TAeObjMsgWindow.h"
struct DrawTask;
#include "ui/TSimpleText.h"
#include "engine/DrawTask.h"
#include "ui/Font.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "core/Mem.h"
#include "Ninja/ninja.h"
#include "sound/SoundCtrl.h"
#include "core/TMainTask.h"
#include "core/Pad.h"

extern "C" {

struct SpeakPad {
	u8 _00[8];
	u32 mButtons; // _08
};

}

static inline Vec3f textVec(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

Vec3f sTextAtSmall    = textVec(48.0f, 336.0f, -1.0f);
Vec3f sTextBoxSmall   = textVec(550.0f, 74.0f, 0.0f);
Vec3f sTextGlyphSmall = textVec(1.0f, 1.0f, 1.0f);
Vec3f sTextAt         = textVec(153.0f, 190.0f, -1.0f);
Vec3f sTextBox        = textVec(330.0f, 132.0f, 0.0f);
Vec3f sTextGlyph      = textVec(0.91f, 0.91f, 0.91f);

static inline Vec3f* textAt() { return &sTextAt; }
static inline Vec3f* textBox() { return &sTextBox; }
static inline Vec3f* textGlyph() { return &sTextGlyph; }
static inline Vec3f* textAtSmall() { return &sTextAtSmall; }
static inline Vec3f* textBoxSmall() { return &sTextBoxSmall; }
static inline Vec3f* textGlyphSmall() { return &sTextGlyphSmall; }

// 0x8010F770
extern "C" TSimpleText* createSimpleTextId2(TObject* parent, void* desc)
{
	return new TSimpleText(parent, 2, 2, &sTextAt, &sTextBox, &sTextGlyph, desc);
}

// 0x8010F8D4
extern "C" TSimpleText* createSimpleTextId4(TObject* parent, void* desc)
{
	return new TSimpleText(parent, 2, 4, &sTextAt, &sTextBox, &sTextGlyph, desc);
}

// 0x8010F944
extern "C" TSimpleText* createSimpleTextId5(TObject* parent, void* desc)
{
	return new TSimpleText(parent, 2, 5, &sTextAt, &sTextBox, &sTextGlyph, desc);
}

// 0x8010F9B4
extern "C" TSimpleText* createSimpleTextId6(TObject* parent, void* desc)
{
	return new TSimpleText(parent, 2, 6, &sTextAt, &sTextBox, &sTextGlyph, desc);
}

// 0x8010F7E0
// Kind 0 1 and 2 pick a fixed id
extern "C" TSimpleText* createSimpleTextByKind(TObject* parent, void* desc, s8 kind)
{
	switch (kind) {
	case 0:
		return createSimpleTextId4(parent, desc);
	case 1:
		return createSimpleTextId5(parent, desc);
	case 2:
		return createSimpleTextId6(parent, desc);
	case 3:
		return new TSimpleText(parent, 1, 3, &sTextAt, &sTextBox, &sTextGlyph, desc);
	default:
		return new TSimpleText(parent, 2, 3, &sTextAt, &sTextBox, &sTextGlyph, desc);
	}
}

// 0x8010FA24
extern "C" TSimpleText* createSimpleTextPlain(void* desc)
{
	return new TSimpleText(gTL_07, 0, 0, &sTextAtSmall, &sTextBoxSmall, &sTextGlyphSmall, desc);
}

// 0x8010FA90
extern "C" TSimpleText* createSimpleTextTimed(void* desc, int timer)
{
	TSimpleText* text = new TSimpleText(gTL_07, 4, 1, &sTextAtSmall, &sTextBoxSmall, &sTextGlyphSmall, desc);
	if (text != 0) {
		text->mTimer = timer;
	}
	return text;
}

// 0x8010FB0C
extern "C" TSimpleText* createSimpleText(TObject* parent, int kind, int id, const Vec3f* at, const Vec3f* box, const Vec3f* glyph, void* desc)
{
	return new TSimpleText(parent, kind, id, at, box, glyph, desc);
}

// 0x8010FB88
extern "C" TSimpleText* createSimpleTextOnTask(int kind, int id, const Vec3f* at, const Vec3f* box, const Vec3f* glyph, void* desc)
{
	return new TSimpleText(gTL_07, kind, id, at, box, glyph, desc);
}

typedef char TSimpleText_size_check[(sizeof(TSimpleText) == 0xAC) ? 1 : -1];

const char* gTSimpleTextName = "TSimpleText";

// 0x8010FC00
TSimpleText::TSimpleText(TObject* parent, int kind, int id, const Vec3f* at, const Vec3f* box, const Vec3f* glyph, void* desc)
    : TObject(parent)
    , mAt(*at)
    , mBox(*box)
    , mId(id)
    , mGlyph(*glyph)
    , mKind(kind)
{
	_00   = (u32)gTSimpleTextName;
	mSize = 0xAC;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3F, 0);
	mWindow = 0;
	if (mId != -1) {
		mWindow  = createMsgWindow(this, id);
		mState = 0;
	} else {
		mState = 1;
	}
	mBlock.start(desc);
	buildLines();
	switch (mKind) {
	case 0:
		break;
	case 1:
		mBits |= 1;
		break;
	case 2:
		mBits |= 1;
		break;
	}
}

// 0x8010FD60
TSimpleText::~TSimpleText()
{
	if (mWindow != 0) {
		mWindow->mFlags |= 1;
		mWindow = 0;
	}
	if (mExtra != 0) {
		delete mExtra;
		mExtra = 0;
	}
	for (int i = 0; i < mRows; i++) {
		if (mLines[i] != 0) {
			delete mLines[i];
			mLines[i] = 0;
		}
	}
	memFree(mLines);
	removeDrawTask(mDraw);
}

// 0x8010FE50
void TSimpleText::buildLines()
{
	mBits  = 0;
	_7C    = 0;
	_80    = 0;
	_94    = 0;
	_98    = 0;
	mExtra = 0;
	_9C    = 0;
	_A4    = 0;
	_A8    = 0;
	_A9    = 0;
	mTimer = 0;
	mHold  = 0;
	_68    = 0;
	_6C    = 0;

	f32 rows = (mBox.y + gTextGapY) / (gTextGlyphH * mGlyph.y + gTextGapY);
	f32 width = (mBox.x + gTextGapX) / (gTextGlyphW * mGlyph.x + gTextGapX);
	int cols = (int)width * 3;
	mRows = (int)rows;
	if (mRows < 1) {
		mRows = 1;
	}
	mLines = (TextLine**)memAlloc((u32)mRows << 2);
	for (int i = 0; i < mRows; i++) {
		mLines[i] = new TextLine(cols, (int)mBox.x);
	}
}

// 0x8010FF84
void TSimpleText::draw()
{
	if (mState != 2 && mHold != 0) {
		if (mWindow != 0) {
			if (mWindow->close() != 0) {
				mState = 2;
			}
		} else {
			mState = 4;
		}
	}
	switch (mState) {
	case 0:
		waitReady();
		break;
	case 1:
		readInput();
		break;
	case 2:
		finishLine();
		break;
	case 4:
		retire();
		break;
	}
}

// 0x80110048
// Sets bit 0 of the draw task's halfword at +0x14 while waiting and clears it after
void TSimpleText::vfn_10()
{
	if (mState == 0) {
		mDraw->mFlags |= 1;
	} else {
		mDraw->mFlags &= 0xFFFE;
	}
}

// The signed-int truncation the binary does with the 0x43300000 double
static inline f32 textSpan(int count, f32 step, f32 gap)
{
	return (f32)(int)((f32)(count + 1) * step + gap * (f32)count);
}

// 0x8011007C
// Draws one line per row while mState is 1
void TSimpleText::render()
{
	if (mState != 1) {
		return;
	}
	f32 z = mAt.z;
	fontSetTexture();
	njSetTextureNum(0);
	njSetPaletteBankNum(0, 0);
	njZBufferMode(0);
	textDrawBegin(1);
	textDrawSet(z, -1);

	f32 x;
	f32 y;
	switch ((s8)_A9) {
	case 0:
		y = mAt.y;
		break;
	case 1:
		y = mBox.y * 0.5f + mAt.y;
		y -= 0.5f * textSpan(_80, gTextGlyphH * mGlyph.y, gTextGapY);
		break;
	case 2:
		y = mAt.y + mBox.y;
		y -= textSpan(_80, gTextGlyphH * mGlyph.y, gTextGapY);
		break;
	}

	for (int i = 0; i < mRows; i++) {
		switch ((s8)_A8) {
		case 0:
			x = mAt.x + (f32)(int)_68;
			break;
		case 1:
			x = (mBox.x * 0.5f + mAt.x) - 0.5f * (f32)(int)mLines[i]->mWidth;
			break;
		case 2:
			x = (mAt.x + mBox.x) - (f32)(int)mLines[i]->mWidth;
			break;
		}
		mLines[i]->draw(x, y, gTextGapX, &mGlyph);
		y += gTextGlyphH * mGlyph.y + gTextGapY;
	}
	textDrawEnd();
	njZBufferMode(1);
}

// 0x80110390
void TSimpleText::waitReady()
{
	if (*(int*)((u8*)mWindow + 0x60) != 1) {
		return;
	}
	mState = 1;
}

// 0x801103AC
// Pad bit 0x100 confirms bit 0x200 skips
void TSimpleText::readInput()
{
	mBits = 0;
	switch (mKind) {
	case 0:
		if (mId != 2) {
			if (mId == 3) {
				SpeakPad* pad = ((SpeakPad*)padGet(0));
				if ((pad->mButtons & 0x100) != 0) {
					mBits |= 1;
					mKind = 2;
					if (mWindow != 0) {
						mWindow->playArrow();
					}
				}
			} else {
				SpeakPad* pad = ((SpeakPad*)padGet(0));
				if ((pad->mButtons & 0x100) != 0) {
					mBits |= 1;
					mKind = 2;
				}
				pad = ((SpeakPad*)padGet(0));
				if ((pad->mButtons & 0x200) != 0) {
					mBits |= 2;
					if (mWindow != 0) {
						mWindow->playArrow();
					}
				}
			}
		}
		break;
	case 1:
		mBits |= 1;
		break;
	case 2:
		mBits |= 1;
		if (mId != 2) {
			if (mId == 3) {
				SpeakPad* pad = ((SpeakPad*)padGet(0));
				if ((pad->mButtons & 0x100) != 0) {
					mBits |= 2;
					if (mWindow != 0) {
						mWindow->playArrow();
					}
				}
			} else {
				SpeakPad* pad = ((SpeakPad*)padGet(0));
				if ((pad->mButtons & 0x200) != 0) {
					mBits |= 2;
					if (mWindow != 0) {
						mWindow->playArrow();
					}
				}
			}
		}
		break;
	case 4:
		mBits |= 1;
		mTimer -= 1;
		if (mTimer == 0) {
			if (mWindow != 0) {
				if (mWindow->close() != 0) {
					mState = 2;
				}
			} else {
				mState = 3;
			}
		}
		break;
	}
	if (advancePage() != 0) {
		applyPage();
		if (mWindow != 0) {
			if (mWindow->close() != 0) {
				mState = 2;
			}
		} else {
			mState = 3;
		}
	}
	if (mId != 2 && mWindow != 0) {
		if (hasMoreText() != 0) {
			mWindow->showArrow();
		} else {
			mWindow->hideArrow();
		}
	}
}

// 0x8011061C
void TSimpleText::finishLine()
{
	if (mWindow == 0) {
		if (mHold != 0) {
			mState = 4;
		} else {
			mState = 3;
		}
		return;
	}
	if (*(int*)((u8*)mWindow + 0x60) != 3) {
		return;
	}
	mWindow->mFlags |= 1;
	mWindow  = 0;
	if (mHold != 0) {
		mState = 4;
	} else {
		mState = 3;
	}
}

// 0x80110690
void TSimpleText::retire()
{
	mFlags |= 1;
}

// 0x801106A0
int TSimpleText::bit0()
{
	return (int)(mBits & 1);
}

// 0x801106AC
int TSimpleText::bit1()
{
	return (int)(mBits & 2);
}

// 0x801106B8
// 1 when _80 has reached the last row
int TSimpleText::rowFull()
{
	return (u8)((int)_80 >= mRows - 1);
}

// 0x801106DC
void TSimpleText::clearCount()
{
	_80 = 0;
}

// 0x801106E8
// Advances the window Once it is full the top line moves to the bottom
void TSimpleText::scrollLine()
{
	_80 += 1;
	if ((int)_7C >= mRows - 1) {
		TextLine* first = mLines[0];
		first->clear();
		for (int i = 1; i < mRows; i++) {
			mLines[i - 1] = mLines[i];
		}
		mLines[mRows - 1] = first;
	} else {
		_7C += 1;
	}
}

// 0x8011078C
void TSimpleText::applyPage()
{
	_80 = 0;
	_7C = 0;
	for (int i = 0; i < mRows; i++) {
		mLines[i]->clear();
	}
}

// 0x801107E8
// Reads one control word out of mBlock and places it on the current line
// Returns 1 when _98 lands on 4
int TSimpleText::advancePage()
{
	do {
		switch (_98) {
		case 0: {
			u16 code;
			FontGlyph info;
			switch (mBlock.next(&code)) {
			case 2:
				if (rowFull() != 0) {
					_98 = 2;
				} else {
					scrollLine();
				}
				break;
			case 3:
				_98 = 3;
				break;
			case 4:
				if ((_9C & 1) != 0) {
					_98 = 3;
				} else {
					_98 = 4;
				}
				break;
			case 0:
			case 1: {
				fontGetGlyph(code, &info);
				TextGlyph* glyph = new TextGlyph(&info, mBlock.mColor);
				_98 = 1;
				_94 = mBlock.mWait;
				if (mLines[_7C]->add(mGlyph.x, glyph) == 0) {
					if (rowFull() != 0) {
						mExtra = glyph;
						_98    = 2;
					} else {
						scrollLine();
						mLines[_7C]->add(mGlyph.x, glyph);
					}
				}
				break;
			}
			}
			_9C &= ~1;
			_9C = 0;
			break;
		}
		case 1:
			if (bit0() != 0) {
				_94 = 0;
			} else {
				_94 -= 1;
			}
			if (_94 <= 0) {
				_98 = 0;
				_9C |= 1;
			}
			break;
		case 2:
			mBits &= ~1;
			_9C |= 2;
			if (bit1() != 0) {
				scrollLine();
				clearCount();
				if (mExtra != 0) {
					mLines[_7C]->add(mGlyph.x, mExtra);
					mExtra = 0;
				}
				_98 = 0;
				_9C &= ~2;
			}
			break;
		case 3:
			mBits &= ~1;
			_9C |= 2;
			if (bit1() != 0) {
				applyPage();
				_98 = 0;
				_9C &= ~2;
				sePlay(0x1E);
			}
			break;
		case 4:
			mBits &= ~1;
			break;
		}
		if ((_9C & 2) != 0) {
			_A4 += 1;
		} else {
			_A4 = 0;
		}
		for (int i = 0; i < mRows; i++) {
			mLines[i]->update();
		}
	} while (bit0() != 0);
	return (u8)((4 - _98) == 0);
}

// 0x80110B20
int TSimpleText::hasMoreText()
{
	return (int)(_9C & 2);
}

// 0x80110B2C
void TSimpleText::resetPage(const void* text)
{
	mBlock.start(text);
	applyPage();
	_98 = 0;
	_9C &= 0xFFFFFFFD;
}

// 0x80110B78
void TSimpleText::closeText()
{
	if (mWindow == 0) {
		return;
	}
	if (*(int*)((u8*)mWindow + 0x60) == 3) {
		mWindow->mFlags |= 1;
		mWindow  = 0;
		mState = 3;
	} else if (mWindow->close() != 0) {
		mState = 2;
	}
}
