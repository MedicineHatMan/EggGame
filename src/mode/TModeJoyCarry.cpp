#include "core/JoyCarry.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "core/Pad.h"
#include "core/TMainTask.h"
#include "object/TObjLauncher.h"
#include "core/Mem.h"
#include "stage/Stage.h"
#include "object/TObject.h"
#include "Ninja/ninja.h"
struct TObject;

// The Japanese build's programs Nothing reads this table but its strings stay in the pool ahead of the others
static const char* sFileNamesJP[5] = {
	"Chu2ChallengeJP.bin", "puyojp.gba", "NightsUS.gba", "BillyAGB.bin", "BillyAGB.bin",
};

static const char* sFileNames[5] = {
	"Chu2ChallengeUS.bin", "puyous.gba", "NightsUS.gba", "BillyAGB.bin", "BillyAGB.bin",
};

static const char* sMenuNames[5] = {
	"CHUCHU", "PUYOPUYO", "NIGHTS", "BILLY HATCHER EASY", "BILLY HATCHER",
};

// 0x801AF068
TModeJoyCarry* TModeJoyCarry::create(TModeAllocator* allocator)
{
	return new (allocator) TModeJoyCarry;
}

// 0x801AF0A0
TModeJoyCarry::TModeJoyCarry()
{
	mState  = 4;
	mCursor = 0;
}

// 0x801AF0D4
int TModeJoyCarry::init()
{
	for (int i = 0; i < 5; i++) {
		loadSizedResource(sFileNames[i], &mFiles[i]);
	}
	mState = 0;
	return 1;
}

// 0x801AF138
int TModeJoyCarry::shutdown()
{
	for (int i = 0; i < 5; i++) {
		releaseResource(mFiles[i]);
	}
	return 1;
}

// 0x801AF188
void TModeJoyCarry::update()
{
	gjBeforeRender();

	if (mState == 0) {
		if ((padGet(0)->mPressed & 8) && mCursor > 0) {
			mCursor--;
		} else if ((padGet(0)->mPressed & 4) && mCursor < 4) {
			mCursor++;
		}
		if (padGet(0)->mPressed & 0x100) {
			u32 size;
			BinFile* file = mFiles[mCursor];
			size          = file->mSize;
			JoyCarry* jc  = JoyCarry::get();
			if (jc->boot(file->mData, size) == 0) {
				mState = 2;
				mTimer = 180;
			} else {
				mState = 1;
			}
		} else if (padGet(0)->mPressed & 0x200) {
			_08 = 0;
		}
	} else if (mState == 1) {
		u8 percent;
		if (JoyCarry::get()->getPercent(&percent)) {
			njPrint(0xA0014, "DOWNLOAD ... %3d / 100", percent);
		}
		int result = JoyCarry::get()->getResult();
		if (result != 1) {
			if (result == 0) {
				mTimer = 180;
				mState = 3;
			} else {
				mTimer = 180;
				mState = 2;
			}
		}
	} else if (mState == 2) {
		if (--mTimer == 0) {
			mState = 0;
		}
		njPrintC(0xA0014, "CONNECT ERROR");
	} else if (mState == 3) {
		if (--mTimer == 0) {
			mState = 0;
		}
		njPrintC(0xA0014, "COMPLETE");
	}

	for (int i = 0; i < 5; i++) {
		njPrintC((i + 10) | 0xA0000, sMenuNames[i]);
	}
	njPrintC((mCursor + 10) | 0x80000, ">");
	objPrintTree(&gMainTask);
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}
