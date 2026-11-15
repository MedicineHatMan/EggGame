#include "mode/TModeStageSelect.h"
#include "core/Pad.h"
#include "core/dolphin.h"
#include "object/TObjLauncher.h"
#include "stage/Stage.h"
#include "Ninja/ninja.h"

#define PRINT_LOC(x, y) ((x) << 16 | (y))

// 0x8007BD9C
TModeStageSelect* TModeStageSelect::create(TModeAllocator* allocator)
{
	return new (allocator) TModeStageSelect;
}

// 0x8007BDD4
TModeStageSelect::TModeStageSelect()
{
	mCursor = 0;
	mPlayer = 0;
}

// 0x8007BE34
const char* TModeStageSelect::getStageName()
{
	return mStages.mData[mCursor];
}

// 0x8007BE48
int TModeStageSelect::init()
{
	StageDefTable* table = getStageDefTable();
	for (int i = 0; i < table->mCount; i++) {
		const char* name = table->mEntries[i].mName;
		if (strncmp(name, "battle", 6) != 0) {
			mStages.push(name);
		}
	}
	return 1;
}

// 0x8007BEC8
void TModeStageSelect::update()
{
	gjBeforeRender();

	if (padGet(0)->mRepeat & PAD_BUTTON_UP) {
		if (mCursor != 0) {
			mCursor--;
		} else {
			mCursor = mStages.mCount - 1;
		}
	} else if (padGet(0)->mRepeat & PAD_BUTTON_DOWN) {
		if (mCursor < mStages.mCount - 1) {
			mCursor++;
		} else {
			mCursor = 0;
		}
	} else if (padGet(0)->mRepeat & PAD_BUTTON_RIGHT) {
		if (++mPlayer >= 5) {
			mPlayer = 0;
		}
	} else if (padGet(0)->mRepeat & PAD_BUTTON_LEFT) {
		if (--mPlayer < 0) {
			mPlayer = 4;
		}
	}

	if (padGet(0)->mPressed & PAD_BUTTON_A) {
		_08 = 0;
	}
	if (padGet(0)->mPressed & PAD_BUTTON_B) {
		_08 = 1;
	}

	njPrintC(PRINT_LOC(6, 5), "SELECT STAGE");
	njPrintC(PRINT_LOC(6, 6), " [A] ... START GAME");
	njPrintC(PRINT_LOC(6, 7), " [B] ... BACK TO SELECT GAME");
	njPrintC(9, "PLAYER:  1 2 3 4 5");
	njPrintC(PRINT_LOC(mPlayer * 2 + 8, 9), "[ ]");

	int first = 0;
	if (mCursor > 10) {
		first = mCursor - 10;
	}
	for (int i = first; i < mStages.mCount; i++) {
		njPrintC(PRINT_LOC(8, i + 12 - first), mStages.mData[i]);
	}
	njPrintC(PRINT_LOC(6, mCursor + 12 - first), ">");

	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}

// 0x8007C0D8
TModeStageSelect::~TModeStageSelect()
{
}
