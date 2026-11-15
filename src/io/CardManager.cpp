#include "io/CardManager.h"
#include "io/MemCard.h"
#include "io/SaveData.h"
#include "Ninja/ninja.h"
#include "Dolphin/OS/OSUtil.h"
#include "core/SdkCalls.h"

extern "C" {
const char* gCardFileName = "billyhatcher";
CardManager* gCardManager;
}

// 0x8019F80C
CardManager::CardManager()
{
	mFileName = gCardFileName;
	mSaveSize = sizeof(SaveData);
	mWorker   = createCardWorker(0);
	mImage    = createCardImage(0, 0, 1, 0, 2, gCardImages, "Billy Hatcher", "");
	int image = (mImage->getSize() + 0x1F) / 0x20 * 0x20;
	mSize     = mSaveSize + image;
	mSize     = (mSize + 0x1FFF) & ~0x1FFF;
	mData     = new u8[mSize];
	memset(mData, 0, mSize);
	mSave    = (SaveData*)(mData + image);
	mBusy    = 0;
	mResult  = 3;
	mState   = 0;
	mPhase   = 0;
	mWant    = 0;
	mChanged = 0;
	runState(mState, 0);
}

// 0x8019F910
CardManager::~CardManager()
{
	if (mWorker != 0) {
		delete mWorker;
		mWorker = 0;
	}
	if (mData != 0) {
		delete[] mData;
		mData = 0;
	}
	if (mImage != 0) {
		delete mImage;
		mImage = 0;
	}
	if (gCardManager == this) {
		gCardManager = 0;
	}
}

static inline int clampInt(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

// 0x8019F9B8
// The second comment line is the save time
void CardManager::writeComment()
{
	char* line = mImage->getComment();
	OSCalendarTime t;
	OSTicksToCalendarTime(OSGetTime(), &t);
	t.sec  = clampInt(t.sec, 0, 59);
	t.min  = clampInt(t.min, 0, 59);
	t.hour = clampInt(t.hour, 0, 23);
	t.mday = clampInt(t.mday, 1, 31);
	t.mon  = clampInt(t.mon, 0, 11) + 1;
	t.year = clampInt(t.year, 1, 9999);
	sprintf(line, "%02d/%02d/%04d %02d:%02d", t.mon, t.mday, t.year, t.hour, t.min);
}

// 0x8019FB0C
int CardManager::getState()
{
	return mState;
}

// 0x8019FB14
int CardManager::resultIs8()
{
	return mWorker->getResult() == 8;
}

// 0x8019FB44
int CardManager::cardStateIs6()
{
	return mWorker->getState() == 6;
}

// 0x8019FB74
void CardManager::update()
{
	if (mWorker != 0) {
		mWorker->update();
	} else {
		return;
	}
	do {
		syncWorker();
		mChanged = 0;
		runState(mState, 1);
	} while (mChanged != 0);
	if (mWant != 0) {
		handleState(mState, 4);
		mWant = 0;
	}
}

// 0x8019FC04
u8 CardManager::request(int want)
{
	int old = mWant;
	mWant   = want;
	int ok  = handleState(mState, 3);
	switch (ok) {
	default:
		return 1;
	case 0:
		mWant = old;
		return 0;
	}
}

// 0x8019FC58
void CardManager::setState(int state)
{
	if (state != mState) {
		runState(mState, 2);
		mState = state;
		runState(mState, 0);
		mChanged = 1;
	}
}

// 0x8019FCB4
void CardManager::runState(int state, int event)
{
	switch (state) {
	case 0:
		runNoCard(event);
		break;
	case 1:
		runWrongDevice(event);
		break;
	case 2:
		runBroken(event);
		break;
	case 3:
		runError(event);
		break;
	case 4:
		runNoSpace(event);
		break;
	case 5:
		runNoFile(event);
		break;
	case 6:
		runCorrupt(event);
		break;
	case 7:
		runOtherSave(event);
		break;
	case 8:
		runFormat(event);
		break;
	case 9:
		runCreate(event);
		break;
	case 10:
		runDelete(event);
		break;
	case 11:
		runLoad(event);
		break;
	case 12:
		runSave(event);
		break;
	case 14:
		runCrcCheck(event);
		break;
	case 13:
		runFreeCheck(event);
		break;
	case 15:
		runFormatted(event);
		break;
	case 16:
		runCreated(event);
		break;
	case 17:
		runDeleted(event);
		break;
	case 18:
		runReady(event);
		break;
	case 19:
		runSaved(event);
		break;
	}
}

// 0x8019FDDC
int CardManager::handleState(int state, int event)
{
	int ok = 0;
	switch (state) {
	case 2:
		ok = askBroken(event);
		break;
	case 5:
		ok = askNoFile(event);
		break;
	case 6:
		ok = askCorrupt(event);
		break;
	case 15:
		ok = askFormatted(event);
		break;
	case 16:
		ok = askCreated(event);
		break;
	case 17:
		ok = askDeleted(event);
		break;
	case 18:
		ok = askReady(event);
		break;
	case 19:
		ok = askSaved(event);
		break;
	}
	return ok;
}

// 0x8019FE9C
void CardManager::syncWorker()
{
	mBusy   = mWorker->isBusy();
	mResult = mWorker->getResult();
}

// 0x8019FEDC
// 1 while a card is in and ready else moves to the state for what is wrong
int CardManager::checkCard()
{
	int state = mWorker->getState();
	if (state == 0) {
		return 1;
	}
	switch (state) {
	case 3:
		setState(0);
		break;
	case 4:
		setState(1);
		break;
	case 6:
		setState(3);
		break;
	case 2:
	default:
		setState(3);
		break;
	}
	return 0;
}

// 0x8019FF84
// 1 when the last request ended well else moves to the state for the error
int CardManager::checkResult()
{
	if (mResult == 0) {
		return 1;
	}
	switch (mResult) {
	case 3:
		setState(0);
		break;
	case 4:
		setState(1);
		break;
	case 0:
	case 12:
	default:
		setState(3);
		break;
	case 7:
	case 8:
		setState(2);
		break;
	case 9:
		setState(4);
		break;
	case 10:
		setState(6);
		break;
	case 11:
		setState(5);
		break;
	}
	return 0;
}

// 0x801A0028
void CardManager::runNoCard(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		if (checkCard() != 0) {
			setState(11);
		}
		break;
	case 2:
		break;
	}
}

// 0x801A007C
void CardManager::runWrongDevice(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		if (checkCard() != 0) {
			setState(11);
		}
		break;
	case 2:
		break;
	}
}

// 0x801A00D0
void CardManager::runBroken(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A0104
int CardManager::askBroken(int event)
{
	int ok = 0;
	if (mWant == 1) {
		ok = 1;
		if (event == 4) {
			setState(8);
		}
	}
	return ok;
}

// 0x801A0150
void CardManager::runError(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A0184
void CardManager::runNoSpace(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A01B8
void CardManager::runNoFile(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A01EC
int CardManager::askNoFile(int event)
{
	int ok = 0;
	if (mWant == 2) {
		ok = 1;
		if (event == 4) {
			setState(9);
		}
	}
	return ok;
}

// 0x801A0238
void CardManager::runCorrupt(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A026C
int CardManager::askCorrupt(int event)
{
	int ok = 0;
	if (mWant == 3) {
		ok = 1;
		if (event == 4) {
			setState(10);
		}
	}
	return ok;
}

// 0x801A02B8
void CardManager::runOtherSave(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A02EC
// After a load found no file is there room to make one
void CardManager::runFreeCheck(int event)
{
	switch (event) {
	case 0:
		mPhase = 0;
		break;
	case 1:
		if (checkCard() != 0) {
			switch (mPhase) {
			case 0:
				if (mWorker->requestFree() != 0) {
					mPhase = 1;
				}
				break;
			case 1:
				switch (mBusy) {
				case 0:
					if (checkResult() != 0) {
						if (mWorker->hasRoom(mSize) != 0) {
							setState(5);
						} else {
							setState(4);
						}
					}
					break;
				case 1:
					break;
				}
				break;
			}
		}
		break;
	case 2:
		break;
	}
}

// 0x801A03DC
void CardManager::runCrcCheck(int event)
{
	switch (event) {
	case 0:
		mPhase = 0;
		break;
	case 1:
		if (checkCard() != 0) {
			if (mSave->checkCrc() == 0) {
				setState(6);
			} else {
				setState(18);
			}
		}
		break;
	case 2:
		break;
	}
}

// 0x801A0464
void CardManager::runFormat(int event)
{
	switch (event) {
	case 0:
		mPhase = 0;
		break;
	case 1:
		if (checkCard() != 0) {
			switch (mPhase) {
			case 0:
				if (mWorker->requestFormat() != 0) {
					mPhase = 1;
				}
				break;
			case 1:
				if (mBusy == 0 && checkResult() != 0) {
					setState(15);
				}
				break;
			}
		}
		break;
	case 2:
		break;
	}
}

// 0x801A0520
void CardManager::runCreate(int event)
{
	switch (event) {
	case 0:
		mPhase = 0;
		memset(mData, 0, mSize);
		writeComment();
		mImage->build(mData);
		mSave->clear();
		break;
	case 1:
		if (checkCard() != 0) {
			switch (mPhase) {
			case 0:
				if (mWorker->requestCreate(mFileName, mSize, mData, mImage) != 0) {
					mPhase = 1;
				}
				break;
			case 1:
				if (mBusy == 0 && checkResult() != 0) {
					setState(16);
				}
				break;
			}
		}
		break;
	case 2:
		break;
	}
}

// 0x801A0618
void CardManager::runDelete(int event)
{
	switch (event) {
	case 0:
		mPhase = 0;
		break;
	case 1:
		if (checkCard() != 0) {
			switch (mPhase) {
			case 0:
				if (mWorker->requestDelete(mFileName) != 0) {
					mPhase = 1;
				}
				break;
			case 1:
				if (mBusy == 0 && checkResult() != 0) {
					setState(17);
				}
				break;
			}
		}
		break;
	case 2:
		break;
	}
}

// 0x801A06D8
void CardManager::runLoad(int event)
{
	switch (event) {
	case 0:
		mPhase = 0;
		break;
	case 1:
		if (checkCard() != 0) {
			switch (mPhase) {
			case 0:
				if (mWorker->requestRead(mFileName, mSize, mData) != 0) {
					mPhase = 1;
				}
				break;
			case 1:
				if (mBusy == 0) {
					if (mResult == 11) {
						setState(13);
					} else if (checkResult() != 0) {
						setState(14);
					}
				}
				break;
			}
		}
		break;
	case 2:
		break;
	}
}

// 0x801A07BC
// Only writes over the save it loaded the times must agree else goes to state 7
void CardManager::runSave(int event)
{
	switch (event) {
	case 0:
		mPhase = 0;
		if (saveTimeIs(mSave->mTime) == 0) {
			mPhase = 2;
		} else {
			memset(mData, 0, mSize);
			writeComment();
			mImage->build(mData);
			saveSeal();
			saveWrite(mSave);
		}
		break;
	case 1:
		if (checkCard() != 0) {
			switch (mPhase) {
			case 0:
				if (mWorker->requestWrite(mFileName, mSize, mData) != 0) {
					mPhase = 1;
				}
				break;
			case 1:
				if (mBusy == 0 && checkResult() != 0) {
					setState(19);
				}
				break;
			case 2:
				setState(7);
				break;
			}
		}
		break;
	case 2:
		break;
	}
}

// 0x801A08F0
void CardManager::runFormatted(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A0924
int CardManager::askFormatted(int event)
{
	int ok = 0;
	if (mWant == 5) {
		ok = 1;
		if (event == 4) {
			setState(11);
		}
	}
	return ok;
}

// 0x801A0970
void CardManager::runCreated(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A09A4
int CardManager::askCreated(int event)
{
	int ok = 0;
	if (mWant == 5) {
		ok = 1;
		if (event == 4) {
			setState(18);
		}
	}
	return ok;
}

// 0x801A09F0
void CardManager::runDeleted(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A0A24
int CardManager::askDeleted(int event)
{
	int ok = 0;
	if (mWant == 5) {
		ok = 1;
		if (event == 4) {
			setState(11);
		}
	}
	return ok;
}

// 0x801A0A70
void CardManager::runReady(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A0AA4
int CardManager::askReady(int event)
{
	int ok = 0;
	if (mWant == 4) {
		ok = 1;
		if (event == 4) {
			setState(12);
		}
	}
	return ok;
}

// 0x801A0AF0
void CardManager::runSaved(int event)
{
	switch (event) {
	case 0:
		break;
	case 1:
		checkCard();
		break;
	case 2:
		break;
	}
}

// 0x801A0B24
int CardManager::askSaved(int event)
{
	int ok = 0;
	if (mWant == 5) {
		ok = 1;
		if (event == 4) {
			setState(18);
		}
	}
	return ok;
}

// 0x801A0B70
extern "C" void cardInit()
{
	gCardManager = 0;
}

// 0x801A0B7C
extern "C" void cardShutdown()
{
	if (gCardManager != 0) {
		delete gCardManager;
		gCardManager = 0;
	}
}

// 0x801A0BB4
extern "C" void cardCreate()
{
	if (gCardManager == 0) {
		gCardManager = new CardManager;
	}
}

// 0x801A0BF8
extern "C" void cardUpdate()
{
	if (gCardManager != 0) {
		gCardManager->update();
	}
}

// 0x801A0C24
extern "C" int cardGetState()
{
	if (gCardManager != 0) {
		return gCardManager->getState();
	}
	return 0;
}

// 0x801A0C58
extern "C" int cardResultIs8()
{
	if (gCardManager != 0) {
		return gCardManager->resultIs8();
	}
	return 0;
}

// 0x801A0C8C
extern "C" int cardStateIs6()
{
	if (gCardManager != 0) {
		return gCardManager->cardStateIs6();
	}
	return 0;
}

// 0x801A0CC0
extern "C" int cardIsWriting()
{
	switch (cardGetState()) {
	case 8:
	case 9:
	case 10:
	case 12:
		return 1;
	}
	return 0;
}

// 0x801A0D0C
extern "C" u8 cardRequestCreate()
{
	if (gCardManager != 0) {
		return gCardManager->request(2);
	}
	return 0;
}

// 0x801A0D44
extern "C" u8 cardRequestFormat()
{
	if (gCardManager != 0) {
		return gCardManager->request(1);
	}
	return 0;
}

// 0x801A0D7C
extern "C" u8 cardRequestDelete()
{
	if (gCardManager != 0) {
		return gCardManager->request(3);
	}
	return 0;
}

// 0x801A0DB4
extern "C" u8 cardConfirm()
{
	if (gCardManager != 0) {
		return gCardManager->request(5);
	}
	return 0;
}

// 0x801A0DEC
extern "C" u8 cardConfirm2()
{
	if (gCardManager != 0) {
		return gCardManager->request(5);
	}
	return 0;
}

// 0x801A0E24
extern "C" u8 cardConfirm3()
{
	if (gCardManager != 0) {
		return gCardManager->request(5);
	}
	return 0;
}

// 0x801A0E5C
extern "C" u8 cardRequestSave()
{
	if (gCardManager != 0) {
		return gCardManager->request(4);
	}
	return 0;
}

// 0x801A0E94
extern "C" u8 cardConfirm4()
{
	if (gCardManager != 0) {
		return gCardManager->request(5);
	}
	return 0;
}

// 0x801A0ECC
extern "C" void cardLoadSave()
{
	if (gCardManager != 0) {
		saveRead(gCardManager->mSave);
	}
}

typedef char CardManager_size_check[(sizeof(CardManager) == 0x34) ? 1 : -1];
