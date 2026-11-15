#include "Ninja/ninja.h"
#include "stage/GameSelect.h"
#include "core/Pad.h"
#include "engine/ShadowTex.h"
#include "io/ResManager.h"
#include "core/Mem.h"
#include "stage/Stage.h"
#include "stl/string.h"

// 0x800BEC0C
TModeGameSelect* TModeGameSelect::create(TModeAllocator* allocator)
{
	return new (allocator) TModeGameSelect;
}

// 0x800BEC44
TModeGameSelect::TModeGameSelect()
{
	mCursor = 0;
	mItems  = (GameSelectItem*)syMalloc(0x20 * sizeof(GameSelectItem));
	mCount  = 0;
	mFrame  = 0;
}

// 0x800BECB0
TModeGameSelect::~TModeGameSelect()
{
	syFree(mItems);
}

// 0x800BED08
int TModeGameSelect::init()
{
	GXSetCopyClear(sGameSelectClearColor, 0xFFFFFF);
	return 1;
}

// 0x800BED40
int TModeGameSelect::shutdown()
{
	return 1;
}

// 0x800BED48
void TModeGameSelect::addItem(int scene, const char* name)
{
	if (mCount == 0x20) {
		OSPanic("GameSelect.cpp", 0x54, "can't regist more item!");
	}
	mItems[mCount].mScene = scene;
	strncpy(mItems[mCount].mName, name, 0x1F);
	mCount++;
}

// 0x800BEDDC
void TModeGameSelect::update()
{
	gjBeforeRender();
	if (padGet(0)->mPressed & 8) {
		if (mCursor > 0) {
			mCursor--;
		} else {
			mCursor = mCount - 1;
		}
	} else if (padGet(0)->mPressed & 4) {
		if (mCursor < mCount - 1) {
			mCursor++;
		} else {
			mCursor = 0;
		}
	}
	if (padGet(0)->mPressed & 0x100) {
		_08 = 0;
	}
	if (padGet(0)->mPressed & 0x200) {
		_08 = 1;
	}
	njPrintC(0x40005, "SELECT GAME MODE");
	njPrintC(0x40006, " [A] ... SELECT MODE");
	njPrintC(0x40007, " [B] ... BACK TO TITLE");
	for (int i = 0; i < mCount; i++) {
		njPrintC((i + 10) | 0x50000, mItems[i].mName);
	}
	if (mFrame & 0x20) {
		njPrintC((mCursor + 10) | 0x30000, ">");
	}
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	frameTickerGet()->update();
	printResources();
	gjPrintFlush();
	mFrame++;
	gjDoneRender();
	gjSwapBuffers();
}
