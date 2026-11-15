#include "io/MemCard.h"
#include "stl/string.h"

typedef void (*CARDCallback)(s32 chan, s32 result);

// 0x801A0EFC
CardBuf::CardBuf()
{
	mWork = new u8[0xA000];
	for (int i = 0; i < 32; i++) {
		mName[i] = 0;
	}
	mFileNo = 0;
}

// 0x801A0F60
CardBuf::~CardBuf()
{
	if (mWork != 0) {
		delete mWork;
		mWork = 0;
	}
}

// 0x801A0FBC
extern "C" CardWorker* createCardWorker(int chan)
{
	if (chan < 0 || chan >= 2) {
		return 0;
	}
	return new CardWorker(chan);
}

// 0x801A1018
CardWorker::CardWorker(int chan)
{
	CARDInit();
	mBuf = new CardBuf;
	if (chan < 0) {
		chan = 0;
	} else if (chan >= 2) {
		chan = 1;
	}
	mChan      = chan;
	mState     = 3;
	mStep      = 0;
	mStarted   = 0;
	mBusy      = 0;
	mRequest   = 0;
	mResult    = 3;
	mFlags     = 0;
	mSize      = 0;
	mData      = 0;
	mImage     = 0;
	mFreeBytes = 0;
	mFreeFiles = 0;
}

// 0x801A10C0
CardWorker::~CardWorker()
{
	close();
	if (mBuf != 0) {
		delete mBuf;
		mBuf = 0;
	}
}

// 0x801A1124
int CardWorker::getState()
{
	return mState;
}

// 0x801A112C
int CardWorker::isBusy()
{
	return mBusy;
}

// 0x801A1134
int CardWorker::getResult()
{
	return mResult;
}

// 0x801A113C
int CardWorker::hasRoom(int size)
{
	u8 ok = 0;
	if (mFreeBytes >= size && mFreeFiles > 0) {
		ok = 1;
	}
	return ok;
}

// 0x801A1164
int CardWorker::begin(int kind)
{
	if (mBusy == 0 && startStep(3) != 0) {
		mBusy    = 1;
		mRequest = kind;
		mResult  = 1;
		return 1;
	}
	return 0;
}

// 0x801A11C8
int CardWorker::requestFormat()
{
	if (begin(1) != 0) {
		setStepStarted(3);
		return 1;
	}
	return 0;
}

// 0x801A1218
int CardWorker::requestCreate(const char* name, int size, u8* data, CardImage* image)
{
	if (begin(2) != 0) {
		strncpy(mBuf->mName, name, 32);
		mSize  = size;
		mData  = data;
		mImage = image;
		setStepStarted(3);
		return 1;
	}
	return 0;
}

// 0x801A1298
int CardWorker::requestDelete(const char* name)
{
	if (begin(3) != 0) {
		strncpy(mBuf->mName, name, 32);
		setStepStarted(3);
		return 1;
	}
	return 0;
}

// 0x801A1300
int CardWorker::requestRead(const char* name, int size, u8* data)
{
	if (begin(4) != 0) {
		strncpy(mBuf->mName, name, 32);
		mSize = size;
		mData = data;
		setStepStarted(3);
		return 1;
	}
	return 0;
}

// 0x801A1378
int CardWorker::requestWrite(const char* name, int size, u8* data)
{
	if (begin(5) != 0) {
		strncpy(mBuf->mName, name, 32);
		mSize = size;
		mData = data;
		setStepStarted(3);
		return 1;
	}
	return 0;
}

// 0x801A13F0
int CardWorker::requestFree()
{
	if (begin(6) != 0) {
		setStepStarted(3);
		return 1;
	}
	return 0;
}

// 0x801A1440
int CardWorker::startStep(int step)
{
	int ok = 1;
	switch (step) {
	case 0:
	case 1:
	case 2:
		break;
	case 3:
		ok = startMount();
		break;
	case 4:
		ok = startCheck();
		break;
	case 5:
		ok = startOpen();
		break;
	case 6:
		ok = startStatus();
		break;
	case 7:
		ok = startFormat();
		break;
	case 8:
		ok = startCreate();
		break;
	case 9:
		ok = startDelete();
		break;
	case 10:
		ok = startRead();
		break;
	case 11:
		ok = startWrite();
		break;
	case 12:
		ok = startFree();
		break;
	case 13:
		ok = startSetStatus();
		break;
	}
	return ok;
}

// 0x801A1504
void CardWorker::pollStep(int step)
{
	switch (step) {
	case 0:
		pollProbe();
		break;
	case 1:
		pollReady();
		break;
	case 2:
		pollEmpty();
		break;
	case 3:
		pollMount();
		break;
	case 4:
		pollCheck();
		break;
	case 5:
		pollOpen();
		break;
	case 6:
		pollStatus();
		break;
	case 7:
		pollFormat();
		break;
	case 8:
		pollCreate();
		break;
	case 9:
		pollDelete();
		break;
	case 10:
		pollRead();
		break;
	case 11:
		pollWrite();
		break;
	case 12:
		pollFree();
		break;
	case 13:
		pollSetStatus();
		break;
	}
}

// 0x801A15AC
void CardWorker::update()
{
	if (mStarted == 0) {
		if (startStep(mStep) != 0) {
			mStarted = 1;
			pollStep(mStep);
		}
	} else {
		pollStep(mStep);
	}
	isBusy();
}

// 0x801A1618
// Retries while the card is busy up to 1000 times
s32 CardWorker::probe(s32* sectorSize)
{
	s32 memSize;
	s32 result;
	int i = 0;
	while ((result = CARDProbeEx(mChan, &memSize, sectorSize)) == CARD_RESULT_BUSY) {
		if (++i >= 1000) {
			break;
		}
	}
	return result;
}

// 0x801A1670
void CardWorker::close()
{
	if (mFlags & 1) {
		CARDUnmount(mChan);
	}
	if (mFlags & 2) {
		CARDClose(&mBuf->mInfo);
	}
	mFlags = 0;
}

// 0x801A16CC
void CardWorker::goStep(int step)
{
	if (startStep(step) != 0) {
		setStepStarted(step);
	} else {
		setStepPending(step);
	}
}

// 0x801A1720
void CardWorker::setStepPending(int step)
{
	mStarted = 0;
	mStep    = step;
}

// 0x801A1730
void CardWorker::setStepStarted(int step)
{
	mStarted = 1;
	mStep    = step;
}

// 0x801A1740
int CardWorker::probeState(s32 result, s32 sectorSize)
{
	switch (result) {
	case CARD_RESULT_FATAL_ERROR:
	case CARD_RESULT_NOFILE:
	default:
		return 2;
	case CARD_RESULT_BUSY:
	case CARD_RESULT_NOCARD:
		return 3;
	case CARD_RESULT_WRONGDEVICE:
		return 4;
	case CARD_RESULT_IOERROR:
		return 5;
	case CARD_RESULT_READY:
		if (sectorSize == 0x2000) {
			break;
		}
		return 6;
	}
	return 0;
}

// 0x801A17B4
int CardWorker::errorKind(s32 result)
{
	switch (result) {
	case CARD_RESULT_NOPERM:
	case CARD_RESULT_FATAL_ERROR:
		return 2;
	case CARD_RESULT_WRONGDEVICE:
		return 4;
	case CARD_RESULT_NOCARD:
		return 3;
	case CARD_RESULT_IOERROR:
	case CARD_RESULT_BROKEN:
		return 5;
	case CARD_RESULT_ENCODING:
		return 8;
	case CARD_RESULT_NOFILE:
		return 11;
	}
	return 2;
}

// 0x801A183C
s32 CardWorker::probeCard()
{
	s32 sectorSize;
	s32 result = probe(&sectorSize);
	mState     = probeState(result, sectorSize);
	return result;
}

// 0x801A1888
void CardWorker::finishOk()
{
	mBusy    = 0;
	mRequest = 0;
	mResult  = 0;
	close();
	goStep(1);
}

// 0x801A18D0
void CardWorker::finishFail()
{
	mBusy    = 0;
	mRequest = 0;
	close();
	goStep(2);
}

// 0x801A1914
void CardWorker::fail(s32 result)
{
	mResult = errorKind(result);
	finishFail();
}

// 0x801A194C
// The card was pulled end the request with the probe's state
int CardWorker::cardGone()
{
	if (probeCard() != 0) {
		mResult = mState;
		finishFail();
		return 1;
	}
	return 0;
}

// 0x801A199C
void CardWorker::pollProbe()
{
	s32 result = probeCard();
	if (result == 0) {
		goStep(1);
	} else if (result == CARD_RESULT_BUSY) {
		goStep(2);
	}
}

// 0x801A19F4
void CardWorker::pollReady()
{
	if (probeCard() != 0) {
		goStep(2);
	}
}

// 0x801A1A34
void CardWorker::pollEmpty()
{
	if (probeCard() == 0) {
		goStep(1);
	}
}

// 0x801A1A74
int CardWorker::startMount()
{
	return CARDMountAsync(mChan, (CARDMemoryCard*)mBuf->mWork, 0, 0) != CARD_RESULT_BUSY;
}

// 0x801A1AB8
void CardWorker::pollMount()
{
	if (cardGone() != 0) {
		return;
	}
	s32 result = CARDGetResultCode(mChan);
	if (result == CARD_RESULT_BUSY) {
		return;
	}
	switch (result) {
	case CARD_RESULT_READY:
	case CARD_RESULT_BROKEN:
	case CARD_RESULT_ENCODING:
		mFlags |= 1;
		break;
	default:
		mFlags &= ~1;
		break;
	}
	if (result == CARD_RESULT_READY || result == CARD_RESULT_BROKEN) {
		goStep(4);
	} else if (mRequest == 1 && result == CARD_RESULT_ENCODING) {
		goStep(7);
	} else {
		fail(result);
	}
}

// 0x801A1B8C
int CardWorker::startCheck()
{
	return CARDCheckAsync(mChan, 0) != CARD_RESULT_BUSY;
}

// 0x801A1BC4
void CardWorker::pollCheck()
{
	s32 result;
	if (cardGone() == 0 && (result = CARDGetResultCode(mChan)) != CARD_RESULT_BUSY) {
		if (result == CARD_RESULT_READY) {
			switch (mRequest) {
			case 0:
				mResult = 2;
				finishFail();
				break;
			case 1:
				goStep(7);
				break;
			case 6:
				goStep(12);
				break;
			case 4:
			case 5:
				goStep(5);
				break;
			case 2:
				goStep(8);
				break;
			case 3:
				goStep(9);
				break;
			}
		} else if (result == CARD_RESULT_BROKEN) {
			if (mRequest == 1) {
				goStep(7);
			} else {
				mResult = 7;
				finishFail();
			}
		} else {
			fail(result);
		}
	}
}

// 0x801A1CF0
int CardWorker::startOpen()
{
	s32 result = CARDOpen(mChan, mBuf->mName, &mBuf->mInfo);
	if (result == CARD_RESULT_READY) {
		mFlags |= 2;
	} else {
		mFlags &= ~2;
	}
	return result != CARD_RESULT_BUSY;
}

// 0x801A1D60
void CardWorker::pollOpen()
{
	if (cardGone() != 0) {
		return;
	}
	s32 result;
	if ((mFlags & 2) != 0) {
		result = CARDGetResultCode(mChan);
	} else {
		result = CARDOpen(mChan, mBuf->mName, &mBuf->mInfo);
	}
	if (result != CARD_RESULT_BUSY) {
		if (result == CARD_RESULT_READY) {
			mFlags |= 2;
			mBuf->mFileNo = mBuf->mInfo.fileNo;
			switch (mRequest) {
			case 0:
			case 1:
			case 3:
			case 6:
			default:
				mResult = 2;
				finishFail();
				break;
			case 4:
			case 5:
				goStep(6);
				break;
			}
		} else {
			fail(result);
		}
	}
}

// 0x801A1E48
int CardWorker::startStatus()
{
	s32 result = CARDGetStatus(mChan, mBuf->mFileNo, &mBuf->mStat);
	if (result == CARD_RESULT_READY) {
		mFlags |= 4;
	} else {
		mFlags &= ~4;
	}
	return result != CARD_RESULT_BUSY;
}

// 0x801A1EB8
void CardWorker::pollStatus()
{
	if (cardGone() != 0) {
		return;
	}
	s32 result;
	if ((mFlags & 4) != 0) {
		result = CARDGetResultCode(mChan);
	} else {
		result = CARDGetStatus(mChan, mBuf->mFileNo, &mBuf->mStat);
	}
	if (result != CARD_RESULT_BUSY) {
		if (result == CARD_RESULT_READY) {
			mFlags |= 4;
			switch (mRequest) {
			case 3:
			case 6:
			default:
				mResult = 2;
				finishFail();
				break;
			case 2:
				goStep(11);
				break;
			case 4:
			case 5:
				if (mBuf->mStat.iconAddr == 0xFFFFFFFF) {
					mResult = 10;
					finishFail();
					break;
				}
				if (mRequest == 4) {
					goStep(10);
					break;
				}
				goStep(11);
				break;
			}
		} else {
			fail(result);
		}
	}
}

// 0x801A1FE4
int CardWorker::startFormat()
{
	return CARDFormatAsync(mChan, 0) != CARD_RESULT_BUSY;
}

// 0x801A201C
void CardWorker::pollFormat()
{
	s32 result;
	if (cardGone() == 0 && (result = CARDGetResultCode(mChan)) != CARD_RESULT_BUSY) {
		if (result == CARD_RESULT_READY) {
			if (mRequest == 1) {
				finishOk();
			} else {
				mResult = 2;
				finishFail();
			}
		} else {
			fail(result);
		}
	}
}

// 0x801A20A0
int CardWorker::startCreate()
{
	return CARDCreateAsync(mChan, mBuf->mName, mSize, &mBuf->mInfo, 0) != CARD_RESULT_BUSY;
}

// 0x801A20EC
void CardWorker::pollCreate()
{
	s32 result;
	if (cardGone() == 0 && (result = CARDGetResultCode(mChan)) != CARD_RESULT_BUSY) {
		if (result == CARD_RESULT_READY) {
			goStep(6);
			mFlags |= 2;
			mBuf->mFileNo = mBuf->mInfo.fileNo;
			return;
		}
		switch (result) {
	
		case CARD_RESULT_EXIST:
			mResult = 12;
			finishFail();
			return;
		case CARD_RESULT_NOENT:
		case CARD_RESULT_INSSPACE:
			mResult = 9;
			finishFail();
			return;
		case CARD_RESULT_NAMETOOLONG:
			mResult = 2;
			finishFail();
			return;
		}
		fail(result);
	}
}

// 0x801A21CC
int CardWorker::startDelete()
{
	return CARDDeleteAsync(mChan, mBuf->mName, 0) != CARD_RESULT_BUSY;
}

// 0x801A220C
void CardWorker::pollDelete()
{
	s32 result;
	if (cardGone() == 0 && (result = CARDGetResultCode(mChan)) != CARD_RESULT_BUSY) {
		if (result == CARD_RESULT_READY) {
			if (mRequest == 3) {
				finishOk();
			} else {
				mResult = 2;
				finishFail();
			}
		} else {
			fail(result);
		}
	}
}

// 0x801A2290
int CardWorker::startRead()
{
	return CARDReadAsync(&mBuf->mInfo, mData, mSize, 0, 0) != CARD_RESULT_BUSY;
}

// 0x801A22D8
void CardWorker::pollRead()
{
	s32 result;
	if (cardGone() == 0 && (result = CARDGetResultCode(mChan)) != CARD_RESULT_BUSY) {
		if (result == CARD_RESULT_READY) {
			if (mRequest == 4) {
				finishOk();
			} else {
				mResult = 2;
				finishFail();
			}
		} else {
			fail(result);
		}
	}
}

// 0x801A235C
int CardWorker::startWrite()
{
	return CARDWriteAsync(&mBuf->mInfo, mData, mSize, 0, 0) != CARD_RESULT_BUSY;
}

// 0x801A23A4
// A new file then gets its comment banner and icon fields set
void CardWorker::pollWrite()
{
	s32 result;
	if (cardGone() == 0 && (result = CARDGetResultCode(mChan)) != CARD_RESULT_BUSY) {
		if (result == CARD_RESULT_READY) {
			if (mRequest == 2) {
				CARDStat* stat     = &mBuf->mStat;
				stat->commentAddr  = mImage->mCommentAddr;
				stat->bannerFormat = (stat->bannerFormat & ~3) | mImage->bannerFormat();
				stat->iconAddr     = mImage->mIconAddr;
				int speed          = mImage->iconSpeed();
				int i;
				for (i = 0; i < mImage->mIconCount; i++) {
					stat->iconFormat = (stat->iconFormat & ~(3 << (i * 2))) | (mImage->iconFormat() << (i * 2));
					stat->iconSpeed  = (stat->iconSpeed & ~(3 << (i * 2))) | (speed << (i * 2));
				}
				stat->iconSpeed    = stat->iconSpeed & ~(3 << (mImage->mIconCount * 2));
				stat->bannerFormat = (stat->bannerFormat & ~4) | mImage->animMode();
				goStep(13);
			} else if (mRequest == 5) {
				finishOk();
			} else {
				mResult = 2;
				finishFail();
			}
		} else {
			fail(result);
		}
	}
}

// 0x801A250C
int CardWorker::startFree()
{
	return 1;
}

// 0x801A2514
void CardWorker::pollFree()
{
	s32 bytes;
	s32 files;
	s32 result;
	if (cardGone() == 0 && (result = CARDFreeBlocks(mChan, &bytes, &files)) != CARD_RESULT_BUSY) {
		if (result == CARD_RESULT_READY) {
			mFreeBytes = bytes;
			mFreeFiles = files;
			if (mRequest == 6) {
				finishOk();
			} else {
				mResult = 2;
				finishFail();
			}
		} else {
			fail(result);
		}
	}
}

// 0x801A25B0
int CardWorker::startSetStatus()
{
	return CARDSetStatusAsync(mChan, mBuf->mFileNo, &mBuf->mStat, 0) != CARD_RESULT_BUSY;
}

// 0x801A25F4
void CardWorker::pollSetStatus()
{
	s32 result;
	if (cardGone() == 0 && (result = CARDGetResultCode(mChan)) != CARD_RESULT_BUSY) {
		if (result == CARD_RESULT_READY) {
			if (mRequest == 2) {
				finishOk();
			} else {
				mResult = 2;
				finishFail();
			}
		} else {
			fail(result);
		}
	}
}

typedef char CardBuf_size_check[(sizeof(CardBuf) == 0xA8) ? 1 : -1];
typedef char CardWorker_size_check[(sizeof(CardWorker) == 0x38) ? 1 : -1];
