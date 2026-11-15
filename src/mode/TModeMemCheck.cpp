#include "stage/Stage.h"
#include "core/Pad.h"
#include "engine/ShadowTex.h"
#include "io/ResManager.h"
#include "object/TObjLauncher.h"
#include "core/Mem.h"
#include "mode/TModeMemCheck.h"
#include "Ninja/ninja.h"

// 0x800692F0
TModeMemCheck* TModeMemCheck::create(TModeAllocator* allocator)
{
	return new (allocator) TModeMemCheck;
}

// 0x80069328
TModeMemCheck::TModeMemCheck()
{
	mHideHelp = 0;
}

// 0x80069354
void TModeMemCheck::update()
{
	if (padGet(0)->mPressed & 0x1000) {
		_08 = 0;
		return;
	}
	if (padGet(0)->mPressed & 0x200) {
		mHideHelp = mHideHelp == 0;
	}
	if (mHideHelp == 0) {
		njPrintC(5, "MEMORY LEAK CHECK");
		njPrintC(6, "[START] EXIT");
		njPrintC(7, "[B]     DISP THIS HELP");
		printResources();
	}
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	gjBeforeRender();
	frameTickerGet()->update();
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}
