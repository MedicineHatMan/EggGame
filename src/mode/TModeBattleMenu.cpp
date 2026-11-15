#include "stage/Stage.h"
#include "core/Pad.h"
#include "core/TArray.h"
#include "core/TMainTask.h"
#include "core/dolphin.h"
#include "object/TObjLauncher.h"
#include "core/Mem.h"
#include "object/TObject.h"
#include "mode/TModeBattleMenu.h"
#include "Ninja/ninja.h"

#define PRINT_AT(x, y) (((y) << 16) | (x))

typedef char TModeBattleMenu_size_check[(sizeof(TModeBattleMenu) == 0x1C) ? 1 : -1];

// 0x80256E58
const char* sBattleRuleNames[3] = { "SURVIVAL", "TIME", "SCORE" };

// 0x8006BF04
TModeBattleMenu* TModeBattleMenu::create(TModeAllocator* allocator)
{
	return new (allocator) TModeBattleMenu;
}

// 0x8006BF3C
TModeBattleMenu::TModeBattleMenu()
{
	mPage  = 0;
	mRule  = 0;
	mStage = 0;
}

// 0x8006BFA0
int TModeBattleMenu::init()
{
	StageDefTable* table = getStageDefTable();
	for (int i = 0; i < table->mCount; i++) {
		const char* name = table->mEntries[i].mName;
		if (strncmp(name, "battle", 6) == 0) {
			mStages.push(name);
		}
	}
	return 1;
}

// 0x8006C020
void TModeBattleMenu::update()
{
	gjBeforeRender();
	njPrintC(PRINT_AT(10, 6), "BATTLE MENU");
	switch (mPage) {
	case 0:
		if ((padGet(0)->mPressed & 8) && mRule != 0) {
			mRule--;
		} else if ((padGet(0)->mPressed & 4) && mRule < 2) {
			mRule++;
		}
		if (padGet(0)->mPressed & 0x100) {
			mPage = 1;
			break;
		}
		if (padGet(0)->mPressed & 0x200) {
			_08 = 1;
			break;
		}
		njPrintC(PRINT_AT(12, 6), "SELECT RULE");
		njPrintC(PRINT_AT(mRule + 14, 6), ">");
		for (int i = 0; i < 3; i++) {
			njPrintC(PRINT_AT(i + 14, 8), sBattleRuleNames[i]);
		}
		break;
	case 1:
		if ((padGet(0)->mPressed & 8) && mStage != 0) {
			mStage--;
		} else if ((padGet(0)->mPressed & 4) && mStage < mStages.mCount - 1) {
			mStage++;
		}
		if (padGet(0)->mPressed & 0x100) {
			_08 = 0;
			break;
		}
		if (padGet(0)->mPressed & 0x200) {
			mPage = 0;
		}
		njPrintC(PRINT_AT(12, 6), "SELECT STAGE");
		njPrintC(PRINT_AT(mStage + 14, 6), ">");
		for (int i = 0; i < mStages.mCount; i++) {
			njPrintC(PRINT_AT(i + 14, 8), mStages.mData[i]);
		}
		break;
	}
	objPrintTree(&gMainTask);
	memDebugDrawBegin();
	memShowStats();
	memDebugDrawEnd();
	gjPrintFlush();
	gjDoneRender();
	endFrameSwap();
}
