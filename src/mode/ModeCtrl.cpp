#include "stage/Stage.h"
#include "core/dolphin.h"
#include "io/FileReader.h"
#include "ui/NowLoading.h"
#include "object/TObjLauncher.h"
#include "core/Mem.h"
#include "Ninja/ninja.h"

struct ModeBlock {
	u8 _00[4];
	ModeBlock* mNext; // _04
};

// While a load is outstanding pump the disc reader until it reports idle
static inline void waitForReader()
{
	FileReader::get()->cancelAll();
	while (FileReader::get()->getState() == 0) {
		FileReader::get()->update();
	}
}

/**
 * @note Address: 0x800116E4
 * @note Size: 0x14
 */
TModeCtrl::TModeCtrl()
{
	mMode  = 0;
	mPhase = MODE_PHASE_RUN;
}

// Nothing calls this so the linker strips it
static void dropMode(TMode* mode)
{
	delete mode;
}

/**
 * @note Address: 0x80011710
 * @note Size: 0x8
 */
int TMode::shutdown()
{
	return 1;
}

/**
 * @note Address: 0x80011718
 * @note Size: 0xBC
 */
TMode* TModeCtrl::setMode(TMode* mode)
{
	TMode* old = mMode;
	if (old != 0) {
		if (mPhase == MODE_PHASE_RUN) {
			old->shutdown();
			old->freeFiles();
		}
		delete old;
	}

	nowLoadingReset();
	if (mode != 0) {
		mode->loadFiles();
	}
	mMode  = mode;
	mPhase = MODE_PHASE_LOAD;
	return old;
}

/**
 * @note Address: 0x800117D4
 * @note Size: 0x4
 */
void TMode::loadFiles()
{
}

/**
 * @note Address: 0x800117D8
 * @note Size: 0x1C
 */
int TModeCtrl::getState()
{
	if (mMode != 0) {
		return mMode->_08;
	}
	return 8;
}

/**
 * @note Address: 0x800117F4
 * @note Size: 0xD8
 */
// 0x800117F4
void TModeCtrl::release()
{
	if (mMode == 0) {
		return;
	}
	switch (mPhase) {
	case MODE_PHASE_LOAD:
		waitForReader();
		mMode->freeFiles();
		break;
	case MODE_PHASE_INIT:
		waitForReader();
		mMode->shutdown();
		mMode->freeFiles();
		break;
	}
}

// One frame while a mode is still loading
// 0x800118CC
static void presentFrame()
{
	if (nowLoadingIsDisabled() != 0) {
		gjBeforeRender();
		nowLoadingUpdate();
		nowLoadingDraw();
		gjDoneRender();
		endFrameSwap();
	} else {
		VIWaitForRetrace();
	}
}

/**
 * @note Address: 0x80011910
 * @note Size: 0xC4
 */
// 0x80011910
void TModeCtrl::update()
{
	switch (mPhase) {
	case MODE_PHASE_RUN:
		if (mMode != 0) {
			mMode->update();
		}
		break;
	case MODE_PHASE_LOAD:
		if (FileReader::get()->getState() == 1) {
			if (mMode != 0) {
				mMode->init();
			}
			mPhase = MODE_PHASE_INIT;
		}
		presentFrame();
		break;
	case MODE_PHASE_INIT:
		if (FileReader::get()->getState() == 1) {
			mPhase = MODE_PHASE_RUN;
		}
		presentFrame();
		break;
	}
}

/**
 * @note Address: 0x800119D4
 * @note Size: 0x8
 */
int TMode::init()
{
	return 1;
}

/**
 * @note Address: 0x800119DC
 * @note Size: 0x4
 */
void TMode::update()
{
}

/**
 * @note Address: 0x800119E0
 * @note Size: 0x68
 */
// 0x800119E0
TModeAllocator::TModeAllocator(u32 slotSize)
{
	mSlotSize = slotSize;
	mBuffer   = memAlloc(slotSize << 1);
	if (mBuffer != 0) {
		ModeBlock* first = (ModeBlock*)mBuffer;
		first->mNext = (ModeBlock*)((u8*)first + slotSize);
		first->mNext->mNext = 0;
	}
	mFree = (ModeBlock*)mBuffer;
}

/**
 * @note Address: 0x80011A48
 * @note Size: 0x6C
 */
void* TModeAllocator::alloc(u32 size)
{
	if (size > mSlotSize) {
		// Original source line The check is the only use of size a slot is always mSlotSize bytes
		OSPanic("ModeCtrl.cpp", 167, "oversize allocation in TModeAllocator\n");
	}

	ModeBlock* block = (ModeBlock*)mFree;
	if (block != 0) {
		mFree       = block->mNext;
		block->mNext = 0;
	}
	return block;
}

/**
 * @note Address: 0x80011AB4
 * @note Size: 0x18
 */
void TModeAllocator::free(void* block)
{
	if (block == 0) {
		return;
	}
	((ModeBlock*)block)->mNext = (ModeBlock*)mFree;
	mFree                      = (ModeBlock*)block;
}

/**
 * @note Address: 0x80011ACC
 * @note Size: 0x20
 */
void TMode::freeFiles()
{
	freeModePrds();
}

/**
 * @note Address: 0x80011AEC
 * @note Size: 0x24
 */
void TMode::loadPrd(const char* name)
{
	loadModePrd(name);
}
