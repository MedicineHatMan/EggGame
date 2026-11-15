#include "ui/TResult.h"
#include "io/LangFile.h"
#include "ui/TextVec.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "ui/TAeLayer.h"
#include "ui/TSimpleText.h"
#include "engine/DrawTask.h"
#include "core/Pad.h"
#include "ui/TFade.h"
#include "ui/AeUtil.h"
#include "Ninja/ninja.h"
#include "stage/TStageScore.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "sound/SoundCtrl.h"
#include "ui/AeObjControl.h"
#include "core/TMainTask.h"
struct ArcFile;
struct NJS_TEXLIST;
struct TArc;

/** What ae_result.arc holds */
struct ResultBlock {
	void* mAe;      // _00 the AE file
	void* mTexlist; // _04
	void* mGvm;     // _08
	u32* mPalettes; // _0C three or NULL
};

ArcFile* sResultArc;         // ae_result.arc
void* sResultAe;             // its AE file
NJS_TEXLIST* sResultTexlist; // its textures
BinFile* sResultMes;         // mes_result.bin
int sResultMesCount;         // the number of messages in it
void** sResultMessages;      // and the messages

static const u32 sBanks[3]       = { 0x1A, 0x1B, 0x1C };                  // the palette banks
static const u32 sScoreSlots[5]  = { 0x141, 0x147, 0x14D, 0x153, 0x161 }; // the first digit of each score then the total
static const u32 sRankIn[5]      = { 0xFD, 0xF3, 0xE8, 0xD9, 0x10D };     // each rank's letter coming in
static const u32 sRankLoop[5]    = { 0xF9, 0xEE, 0xE2, 0xD0, 0x102 };     // and staying
static const u32 sDigitAnims[10] = { 0x21, 0x24, 0x27, 0x2A, 0x2D, 0x30, 0x33, 0x36, 0x39, 0x3C };

typedef char TResult_size_check[(sizeof(TResult) == 0xE8) ? 1 : -1];

static inline TextVec resultVec(f32 x, f32 y, f32 z)
{
	TextVec v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// 0x801C9DEC
// is this file's __sinit each Vec3f is built in a temp and copied in
static TextVec sResultSmallGlyph = resultVec(0.73f, 0.73f, 0.73f);
static TextVec sResultSmallBox   = resultVec(144.0f, 18.0f, 0.0f);
static TextVec sResultGlyph      = resultVec(0.91f, 0.91f, 0.91f);
static TextVec sResultBox        = resultVec(264.0f, 22.0f, 0.0f);

// Where each text goes 0x80286CA8 just after the layout
static TextVec sResultTextAt[13] = {
	resultVec(82.0f, 93.0f, -2.0f),   // [0]
	resultVec(82.0f, 151.0f, -2.0f),  // [1]
	resultVec(82.0f, 209.0f, -2.0f),  // [2]
	resultVec(82.0f, 267.0f, -2.0f),  // [3]
	resultVec(34.0f, 369.0f, -2.0f),  // [4]
	resultVec(34.0f, 411.0f, -2.0f),  // [5]
	resultVec(461.0f, 107.0f, -2.0f), // [6]
	resultVec(461.0f, 134.0f, -2.0f), // [7]
	resultVec(461.0f, 203.0f, -2.0f), // [8]
	resultVec(461.0f, 227.0f, -2.0f), // [9]
	resultVec(461.0f, 251.0f, -2.0f), // [10]
	resultVec(461.0f, 275.0f, -2.0f), // [11]
	resultVec(461.0f, 299.0f, -2.0f), // [12]
};

// 0x801C78F4
void loadResultText()
{
	u32* data;
	u32 text;
	int i;

	sResultArc     = NULL;
	sResultAe      = NULL;
	sResultTexlist = NULL;
	loadSizedResource(getLangFileName("mes_result.bin"), &sResultMes);

	// The table holds offsets from the text after it
	data = (u32*)sResultMes->mData;
	text = *data;
	text += (u32)data++;
	u32 size        = *data++;
	sResultMessages = (void**)data;
	sResultMesCount = size >> 2;
	for (i = 0; i < sResultMesCount; i++) {
		((char**)sResultMessages)[i] += text + 8;
	}
}

// 0x801C798C
void freeResultText()
{
	if (sResultMes != NULL) {
		releaseResource(sResultMes);
		sResultMes = NULL;
	}
}

// 0x801C79C0
void loadResult()
{
	aeLoadArc((const char*)("ae_result.arc"), (TArc**)&sResultArc);
	if (sResultArc != NULL) {
		ResultBlock* block = (ResultBlock*)sResultArc->mData;
		sResultAe          = block->mAe;
		relocateAe(sResultAe);
		sResultTexlist = (NJS_TEXLIST*)block->mTexlist;
		bindGvmTextures((u8*)block->mGvm, sResultTexlist);
		u8* buffer = new u8[0x800];
		if (block->mPalettes != NULL) {
			loadE3Palette(block->mPalettes[0], (E3Palette*)&buffer, sBanks[0]);
			loadE3Palette(block->mPalettes[1], (E3Palette*)&buffer, sBanks[1]);
			loadE3Palette(block->mPalettes[2], (E3Palette*)&buffer, sBanks[2]);
		}
		delete[] buffer;
		njSetTexture(sResultTexlist);
		njSetPaletteBankNum(0, sBanks[0]);
		njSetPaletteBankNum(1, sBanks[1]);
		njSetPaletteBankNum(2, sBanks[2]);
		GXInvalidateTexAll();
	}
}

// 0x801C7AD8
void freeResult()
{
	if (sResultArc != NULL) {
		releaseTextures(sResultTexlist);
		sResultTexlist = NULL;
		aeFreeArc((TArc**)&sResultArc);
		sResultArc = NULL;
		sResultAe  = NULL;
	}
}

// 0x801C7B24
TResult* createResult(TObject* parent)
{
	if (sResultArc == NULL) {
		return NULL;
	}
	return new TResult(parent, sResultAe, sResultTexlist);
}

const char* gTResultName = "TResult";

// 0x801C7B8C
TResult::TResult(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	mTime.mHour   = 0;
	mTime.mMinute = 0;
	mTime.mSecond = 0;
	mTime.mMilli  = 0;
	_00           = (u32)gTResultName;
	mSize         = 0xE8;
	mDrawTask     = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3E, 0);
	stageRecordClear(&mRecord);
	mShown     = 0;
	mRank      = 0;
	mNewBest   = 0;
	mFlags     = 0;
	mBase      = NULL;
	mTitle     = NULL;
	mBest      = NULL;
	mRankLayer = NULL;
	mPrompt    = NULL;
	mDigits[0] = NULL;
	mDigits[1] = NULL;
	mEmblem    = NULL;
	for (int i = 0; i < 13; i++) {
		mTexts[i] = NULL;
	}
	mMsgOpen   = 0;
	mMsgWait   = 0;
	mMsg       = NULL;
	mTensRoll  = 0;
	mWait      = 0;
	mDone      = 0;
	mState     = 0;
	mNextState = 0;
	runState(mState, 0);
}

// 0x801C7C94
TResult::~TResult()
{
	alphaFadeDelete();
	for (int i = 0; i < 13; i++) {
		if (mTexts[i] != NULL) {
			mTexts[i]->mFlags |= 1;
			mTexts[i] = NULL;
		}
	}
	freeResult();
	removeDrawTask(mDrawTask);
}

// 0x801C7D3C
void TResult::draw()
{
	if (mDone == 0) {
		runState(mState, 1);
		if (mNextState != mState) {
			runState(mState, 3);
			mState = mNextState;
			runState(mState, 0);
		}
		runState(mState, 2);
	}
}

// 0x801C7DC0
void TResult::render()
{
	if (mBase != NULL) {
		mBase->present();
	}
}

// 0x801C7DEC
int TResult::pressedA()
{
	return padGet(0)->mPressed & 0x100;
}

// 0x801C7E18
int TResult::pressedAny()
{
	return padGet(0)->mPressed & 0x1F7F;
}

// 0x801C7E44
void TResult::closeMessage()
{
	mMsgOpen = 0;
	if (mMsg != NULL) {
		mMsg->mHold = 1;
		mMsg        = NULL;
	}
}

// 0x801C7E68
void TResult::showMessage(void* text)
{
	closeMessage();
	mMsgOpen = 1;
	mMsgWait = 120;
	if (mMsg == NULL) {
		mMsg = createSimpleTextId2(gTL_07, text);
		if (mMsg != NULL) {
			mMsg->_A8 = 1;
		}
	}
	sePlay(0x2D);
}

// 0x801C7EE0
// Nonzero once the message has been up for two seconds and is closing
int TResult::messageDone()
{
	int done = 0;
	switch (mMsgOpen) {
	case 0:
	default:
		done = 1;
		break;
	case 1:
		if (mMsgWait == 0) {
			if (mMsg != NULL) {
				mMsg->closeText();
				done = 1;
			}
		} else {
			mMsgWait--;
		}
		break;
	}
	return done;
}

// 0x801C7F48
void TResult::setState(int state)
{
	mNextState = state;
}

// 0x801C7F50
void TResult::runState(int state, int phase)
{
	switch (state) {
	case 0:
		runOpen(phase);
		break;
	case 1:
		runTitle(phase);
		break;
	case 2:
		runShow(phase);
		break;
	case 3:
		runScore0(phase);
		break;
	case 4:
		runScore1(phase);
		break;
	case 5:
		runScore2(phase);
		break;
	case 6:
		runScore3(phase);
		break;
	case 7:
		runTotal(phase);
		break;
	case 8:
		runBest(phase);
		break;
	case 9:
		runRank(phase);
		break;
	case 10:
		runWait(phase);
		break;
	case 11:
		runSkip(phase);
		break;
	case 12:
		runClose(phase);
		break;
	case 13:
		runNext(phase);
		break;
	case 14:
		runEmblem(phase);
		break;
	case 15:
		runMessage(phase);
		break;
	case 16:
		runMessage2(phase);
		break;
	case 17:
		runEnd(phase);
		break;
	}
}

// 0x801C8060
// The frame opens
void TResult::runOpen(int phase)
{
	switch (phase) {
	case 0:
		if (mBase != NULL) {
			mBase->reset(99, 0x10);
			mBase->mBanks = sBanks;
		} else {
			mBase = createLayer(99, 0x10);
			if (mBase != NULL) {
				mBase->mBanks = sBanks;
			}
		}
		break;
	case 1:
		if (mBase->isDone()) {
			setState(1);
		}
		break;
	case 2:
		if (mBase != NULL) {
			mBase->update();
		}
		break;
	case 3:
		break;
	}
}

// 0x801C8138
// The title comes in
void TResult::runTitle(int phase)
{
	switch (phase) {
	case 0:
		alphaFadeDim(60);
		if (mBase != NULL) {
			mBase->reset(0x12D, 4);
			mBase->mBanks = sBanks;
			mBase->_A8    = 3.0f;
			mBase->_AC    = 1;
			mBase->setRate(2.0f);
		}
		mTitle = createLayer(0xAA, 0x10);
		if (mTitle != NULL) {
			mTitle->mBanks = sBanks;
		}
		if (mTitle != NULL) {
			mTitle->setRate(2.0f);
		}
		if (mBase != NULL) {
			mBase->attach(0x17E, mTitle, 0);
		}
		break;
	case 1:
		if (mBase != NULL && mBase->play()) {
			setState(2);
		}
		if (pressedAny()) {
			setState(11);
		}
		break;
	case 2:
		if (mTitle != NULL) {
			mTitle->update();
			if (mTitle->getId() == 0xAA && mTitle->isDone()) {
				mTitle->reset(0x8D, 1);
				mTitle->mBanks = sBanks;
			}
		}
		break;
	case 3:
		break;
	}
}

// 0x801C82F0
// The labels the counts and the time
void TResult::runShow(int phase)
{
	switch (phase) {
	case 0:
		mWait = 30;
		showTexts();
		showCounts();
		showTime();
		break;
	case 1:
		if (pressedAny()) {
			setState(11);
		}
		if (mWait == 0) {
			setState(3);
		} else {
			mWait--;
		}
		break;
	case 2:
		if (mBase != NULL) {
			mBase->play();
		}
		if (mTitle != NULL) {
			mTitle->update();
		}
		break;
	case 3:
		break;
	}
}

// States 3 to 6 count one score up 500 a frame then go on to the next
inline void TResult::runScore(int phase, int index)
{
	switch (phase) {
	case 0:
		mWait                 = 30;
		mRecord.mScore[index] = mRecord.mScore[index] > 999999L ? 999999L : mRecord.mScore[index];
		mShown                = 0;
		showText(index);
		break;
	case 1:
		if (pressedAny()) {
			setState(11);
		}
		if (mWait != 0) {
			mWait--;
		} else if (mShown == mRecord.mScore[index]) {
			setState(index + 4);
		}
		break;
	case 2:
		if (mWait == 0) {
			if (mBase != NULL) {
				mBase->play();
			}
			if (mTitle != NULL) {
				mTitle->update();
			}
			mShown += 500;
			if (mShown > mRecord.mScore[index]) {
				mShown = mRecord.mScore[index];
			}
			showNumber(sScoreSlots[index], mShown);
		}
		break;
	case 3:
		break;
	}
}

// 0x801C83B8
void TResult::runScore0(int phase)
{
	runScore(phase, 0);
}

// 0x801C84F4
void TResult::runScore1(int phase)
{
	runScore(phase, 1);
}

// 0x801C8630
void TResult::runScore2(int phase)
{
	runScore(phase, 2);
}

// 0x801C876C
void TResult::runScore3(int phase)
{
	runScore(phase, 3);
}

// 0x801C88A8
// The total counts up then a new best or straight to the rank
void TResult::runTotal(int phase)
{
	switch (phase) {
	case 0:
		mWait  = 30;
		mShown = 0;
		showText(4);
		break;
	case 1:
		if (mWait != 0) {
			mWait--;
		} else {
			int done  = 0;
			u32 total = stageRecordTotal(&mRecord) > 999999 ? 999999 : stageRecordTotal(&mRecord);
			if (mShown == total) {
				done = 1;
			}
			if (pressedAny()) {
				setState(11);
			}
			if (done) {
				if (mNewBest != 0) {
					setState(8);
				} else {
					setState(9);
				}
			}
		}
		break;
	case 2:
		if (mWait == 0) {
			if (mBase != NULL) {
				mBase->play();
			}
			if (mTitle != NULL) {
				mTitle->update();
			}
			mShown += 500;
			u32 total = stageRecordTotal(&mRecord) > 999999 ? 999999 : stageRecordTotal(&mRecord);
			if (mShown > total) {
				mShown = total;
			}
			showNumber(sScoreSlots[4], mShown);
		}
		break;
	case 3:
		break;
	}
}

// 0x801C8A38
// "New record"
void TResult::runBest(int phase)
{
	switch (phase) {
	case 0:
		sePlay(0x2B);
		mBest = createLayer(0xBB, 0);
		if (mBest != NULL) {
			mBest->mBanks = sBanks;
		}
		if (mBase != NULL && mBest != NULL) {
			mBest->setRate(2.0f);
			mBase->attach(0x17D, mBest, 0);
		}
		mWait = 60;
		break;
	case 1:
		if (pressedAny()) {
			setState(11);
		}
		if (mWait == 0) {
			setState(9);
		} else {
			mWait--;
		}
		break;
	case 2:
		if (mBase != NULL) {
			mBase->play();
		}
		if (mTitle != NULL) {
			mTitle->update();
		}
		if (mBest != NULL) {
			mBest->update();
		}
		break;
	case 3:
		break;
	}
}

// Plays the rank's letter and once it is in keeps it there
inline void TResult::updateRank()
{
	if (mRankLayer != NULL) {
		mRankLayer->update();
		if (sRankIn[mRank] == mRankLayer->getId() && mRankLayer->isDone()) {
			mRankLayer->reset(sRankLoop[mRank], 1);
			mRankLayer->mBanks = sBanks;
		}
	}
}

// 0x801C8B78
// The rank
void TResult::runRank(int phase)
{
	switch (phase) {
	case 0:
		sePlay(0x26);
		mRankLayer = createLayer(sRankIn[mRank], 0);
		if (mRankLayer != NULL) {
			mRankLayer->mBanks = sBanks;
		}
		if (mBase != NULL && mRankLayer != NULL) {
			mRankLayer->setRate(2.0f);
			mBase->attach(0x17C, mRankLayer, 0);
		}
		mWait = 120;
		break;
	case 1:
		if (mWait == 0) {
			setState(10);
		} else {
			mWait--;
		}
		break;
	case 2:
		if (mBase != NULL) {
			mBase->play();
		}
		if (mTitle != NULL) {
			mTitle->update();
		}
		if (mBest != NULL) {
			mBest->update();
		}
		updateRank();
		break;
	case 3:
		break;
	}
}

// 0x801C8D20
// A button was pressed while counting everything at once
void TResult::runSkip(int phase)
{
	switch (phase) {
	case 0:
		if (mBase != NULL) {
			mBase->mTime = 3.0f;
		}
		if (mTitle != NULL) {
			mTitle->reset(0x8D, 1);
			mTitle->mBanks = sBanks;
			mTitle->setRate(2.0f);
		} else {
			mTitle = createLayer(0x8D, 0);
			if (mTitle != NULL) {
				mTitle->mBanks = sBanks;
			}
			if (mTitle != NULL) {
				mTitle->setRate(2.0f);
			}
		}
		if (mNewBest != 0) {
			if (mBest == NULL) {
				mBest = createLayer(0xBB, 0);
				if (mBest != NULL) {
					mBest->mBanks = sBanks;
				}
			}
			if (mBase != NULL && mBest != NULL) {
				mBest->setRate(2.0f);
				mBase->attach(0x17D, mBest, 0);
			}
		}
		if (mRankLayer != NULL) {
			mRankLayer->reset(sRankIn[mRank], 1);
			mRankLayer->mBanks = sBanks;
		} else {
			mRankLayer = createLayer(sRankIn[mRank], 0);
			if (mRankLayer != NULL) {
				mRankLayer->mBanks = sBanks;
			}
		}
		if (mBase != NULL && mRankLayer != NULL) {
			mRankLayer->setRate(2.0f);
			mBase->attach(0x17C, mRankLayer, 0);
		}
		showTexts();
		showText(0);
		showText(1);
		showText(2);
		showText(3);
		showText(4);
		showCounts();
		showTime();
		showNumber(sScoreSlots[0], mRecord.mScore[0]);
		showNumber(sScoreSlots[1], mRecord.mScore[1]);
		showNumber(sScoreSlots[2], mRecord.mScore[2]);
		showNumber(sScoreSlots[3], mRecord.mScore[3]);
		showNumber(sScoreSlots[4], stageRecordTotal(&mRecord));
		sePlay(0x26);
		break;
	case 1:
		setState(10);
		break;
	case 2:
		break;
	case 3:
		break;
	}
}

// 0x801C9008
// Waits for A
void TResult::runWait(int phase)
{
	switch (phase) {
	case 0:
		mPrompt = createLayer(0x87, 0);
		if (mPrompt != NULL) {
			mPrompt->mBanks = sBanks;
		}
		if (mBase != NULL && mPrompt != NULL) {
			mPrompt->setRate(2.0f);
			mBase->attach(0x140, mPrompt, 0);
		}
		break;
	case 1:
		if (pressedA()) {
			sePlay(0x1C);
			setState(12);
		}
		break;
	case 2:
		if (mBase != NULL) {
			mBase->play();
		}
		if (mTitle != NULL) {
			mTitle->update();
		}
		if (mBest != NULL) {
			mBest->update();
		}
		updateRank();
		if (mPrompt != NULL) {
			mPrompt->update();
		}
		break;
	case 3:
		break;
	}
}

// 0x801C91A0
// The frame closes
void TResult::runClose(int phase)
{
	switch (phase) {
	case 0:
		if (mPrompt != NULL) {
			mPrompt->reset(0x89, 0x10);
			mPrompt->mBanks = sBanks;
			mPrompt->setRate(2.0f);
		}
		if (mBase != NULL) {
			mBase->_A8 = 4.0f;
		}
		hideTexts();
		break;
	case 1:
		if (mBase != NULL && mBase->play()) {
			setState(13);
		}
		break;
	case 2:
		if (mTitle != NULL) {
			mTitle->update();
		}
		if (mBest != NULL) {
			mBest->update();
		}
		if (mRankLayer != NULL) {
			mRankLayer->update();
		}
		if (mPrompt != NULL) {
			mPrompt->update();
		}
		break;
	case 3:
		break;
	}
}

// 0x801C92AC
void TResult::runNext(int phase)
{
	switch (phase) {
	case 0:
		break;
	case 1:
		if (mFlags & 1) {
			setState(14);
		} else if (mFlags & 0x1FE) {
			setState(15);
		} else if (mFlags & 0x1E00) {
			setState(16);
		} else {
			setState(17);
		}
		break;
	case 2:
		break;
	case 3:
		break;
	}
}

// 0x801C9324
// An emblem the count goes up by one
void TResult::runEmblem(int phase)
{
	switch (phase) {
	case 0:
		if (mBase != NULL) {
			mBase->reset(0x65, 4);
			mBase->mBanks = sBanks;
			mBase->_A8    = 0.83f;
			mBase->_AC    = 1;
		}
		showEmblems();
		mFlags &= ~1;
		mCleared++;
		mWait = 180;
		break;
	case 1:
		if (mWait == 0) {
			setState(13);
		} else {
			mWait--;
		}
		break;
	case 2:
		if (mBase != NULL) {
			mBase->play();
		}
		if (mDigits[0] != NULL) {
			mDigits[0]->update();
			if (mWait == 120) {
				mDigits[0]->wake();
			}
		}
		if (mDigits[1] != NULL) {
			mDigits[1]->update();
			if (mTensRoll != 0 && mWait == 120) {
				mDigits[1]->wake();
			}
		}
		if (mEmblem != NULL) {
			mEmblem->update();
			if (mWait == 120) {
				mEmblem->reset(0x1D, 0x10);
				mEmblem->mBanks = sBanks;
			}
		}
		if (mWait == 105) {
			sePlay(0x72);
		}
		break;
	case 3:
		break;
	}
}

// 0x801C94C0
// Messages 13-20 one for each of bits 1-8 lowest first
void TResult::runMessage(int phase)
{
	switch (phase) {
	case 0:
		int msg;
		if (mFlags & 2) {
			msg = 13;
			mFlags &= ~2;
		} else if (mFlags & 4) {
			msg = 14;
			mFlags &= ~4;
		} else if (mFlags & 8) {
			msg = 15;
			mFlags &= ~8;
		} else if (mFlags & 0x10) {
			msg = 16;
			mFlags &= ~0x10;
		} else if (mFlags & 0x20) {
			msg = 17;
			mFlags &= ~0x20;
		} else if (mFlags & 0x40) {
			msg = 18;
			mFlags &= ~0x40;
		} else if (mFlags & 0x80) {
			msg = 19;
			mFlags &= ~0x80;
		} else if (mFlags & 0x100) {
			msg = 20;
			mFlags &= ~0x100;
		}
		showMessage(sResultMessages[msg]);
		break;
	case 1:
		if (messageDone()) {
			setState(13);
		}
		break;
	case 2:
		break;
	case 3:
		break;
	}
}

// 0x801C95F4
// Messages 21-24 for bits 9-12
void TResult::runMessage2(int phase)
{
	switch (phase) {
	case 0:
		int msg;
		if (mFlags & 0x200) {
			msg = 21;
			mFlags &= ~0x200;
		} else if (mFlags & 0x400) {
			msg = 22;
			mFlags &= ~0x400;
		} else if (mFlags & 0x800) {
			msg = 23;
			mFlags &= ~0x800;
		} else if (mFlags & 0x1000) {
			msg = 24;
			mFlags &= ~0x1000;
		}
		showMessage(sResultMessages[msg]);
		break;
	case 1:
		if (messageDone()) {
			setState(14);
		}
		break;
	case 2:
		break;
	case 3:
		break;
	}
}

// 0x801C96C8
void TResult::runEnd(int phase)
{
	switch (phase) {
	case 0:
		mWait = 120;
		fadeModel01();
		break;
	case 1:
		if (mWait == 0) {
			mDone = 1;
		} else {
			mWait--;
		}
		break;
	case 2:
		break;
	case 3:
		break;
	}
}

// 0x801C9730
// Text i is message i 0-4 the score names 5-12 the labels
void TResult::showText(int index)
{
	if (mTexts[index] == NULL) {
		Vec3f glyph = sResultGlyph;
		Vec3f box   = sResultBox;
		if (index >= 6) {
			glyph = sResultSmallGlyph;
			box   = sResultSmallBox;
		}
		mTexts[index] = createSimpleTextOnTask(1, -1, &sResultTextAt[index], &box, &glyph, sResultMessages[index]);
		if (index >= 6) {
			styleSmallText(index);
		}
	}
}

// 0x801C9828
void TResult::showTexts()
{
	showText(6);
	showText(7);
	showText(8);
	showText(9);
	showText(10);
	showText(11);
	showText(12);
	showText(5);
}

// 0x801C98AC
void TResult::styleSmallText(int index)
{
	if (mTexts[index] != NULL) {
		mTexts[index]->mBox.set(0.0f, 24.0f, 0.0f);
		mTexts[index]->_A8 = 1;
	}
}

// 0x801C98E4
void TResult::hideTexts()
{
	for (int i = 0; i < 13; i++) {
		mTexts[i]->mFlags |= 0x10;
	}
}

// 0x801C9910
void TResult::showCounts()
{
	if (mBase != NULL) {
		aeClipSetDigits3(mBase, mRecord.mCount[0], 0x5A, 0x16E, 0x16D, 0x16C);
		aeClipSetDigits3(mBase, mRecord.mCount[1], 0x5A, 0x171, 0x170, 0x16F);
		aeClipSetDigits2(mBase, mRecord.mCount[4], 0x5A, 0x173, 0x172);
		aeClipSetDigits2(mBase, mRecord.mCount[3], 0x5A, 0x175, 0x174);
		aeClipSetDigits2(mBase, mRecord.mCount[2], 0x5A, 0x177, 0x176);
		aeClipSetDigits2(mBase, mRecord.mCount[5], 0x5A, 0x179, 0x178);
		aeClipSetDigits2(mBase, mRecord.mCount[6], 0x5A, 0x17B, 0x17A);
	}
}

// 0x801C99F0
// Six digits from slot on lowest first without the leading zeros
void TResult::showNumber(u32 slot, int value)
{
	if (mBase != NULL) {
		int digits[6];
		int i;
		int n;
		int count;
		int started;
		int div;
		started = 0;
		n       = value < 0 ? 0 : (value > 999999 ? 999999 : value);
		div     = 100000;
		count   = 6;
		for (i = 5; i >= 0; i--) {
			int* d = &digits[i];
			*d     = n / div;
			n -= div * *d;
			div /= 10;
			if (!started) {
				if (digits[i] == 0) {
					count = i;
				} else {
					started = 1;
				}
			}
		}
		int shown = count < 1 ? 1 : count;
		for (int i = 0; i < shown; i++, slot++) {
			mBase->setFrame(slot, digits[i] + 0x4F);
		}
		sePlay(0x25);
	}
}

// 0x801C9B0C
void TResult::showTime()
{
	if (mBase != NULL) {
		aeClipSetDigits2(mBase, mTime.mMinute, 0x4F, 0x15E, 0x15D);
		aeClipSetDigits2(mBase, (u16)mTime.mSecond, 0x4F, 0x15C, 0x15B);
		aeClipSetDigits2(mBase, 0.1f * mTime.mMilli, 0x4F, 0x15A, 0x159);
	}
}

// 0x801C9BB0
// The emblem count before this one ready to turn over
void TResult::showEmblems()
{
	int count = mCleared;
	if (count < 0) {
		count = 0;
	} else if (count > 60) {
		count = 60;
	}
	int slots[2] = { 0x66, 0x65 };
	int digits[2];
	digits[1] = count / 10;
	digits[0] = count % 10;
	for (int i = 0; i < 2; i++) {
		if (mDigits[i] != NULL) {
			mDigits[i]->reset(sDigitAnims[digits[i]], 0x10);
			mDigits[i]->mBanks = sBanks;
		} else {
			mDigits[i] = createLayer(sDigitAnims[digits[i]], 0x10);
			if (mDigits[i] != NULL) {
				mDigits[i]->mBanks = sBanks;
			}
		}
		if (mBase != NULL && mDigits[i] != NULL) {
			u32 slot = slots[i];
			mBase->clearSlot(slot);
			mBase->attach(slot, mDigits[i], 0);
			TAeClip* clip = mDigits[i];
			clip->flipPause();
		}
		if (i > 0 && digits[i - 1] == 9) {
			mTensRoll = 1;
		}
	}
	if (mEmblem != NULL) {
		mEmblem->reset(0x1B, 1);
		mEmblem->mBanks = sBanks;
	} else {
		mEmblem = createLayer(0x1B, 0);
		if (mEmblem != NULL) {
			mEmblem->mBanks = sBanks;
		}
	}
	if (mBase != NULL && mEmblem != NULL) {
		mBase->clearSlot(0x68);
		mBase->attach(0x68, mEmblem, 0);
	}
}
