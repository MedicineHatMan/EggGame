#include "stage/Stage.h"

#include "stage/TStageGreenBoss.h"

typedef char TStageGreenBoss_size_check[(sizeof(TStageGreenBoss) == 0x30) ? 1 : -1];

// 0x8019AAD0
extern "C" TStage* createGreenBossStage()
{
	return new TStageGreenBoss;
}

TStageGreenBoss::TStageGreenBoss()
{
}

void TStageGreenBoss::loadAssets()
{
	stageLoadBossConsole(this);
}

void TStageGreenBoss::freeAssets()
{
	stageFreeBossConsole();
}

void TStageGreenBoss::vfn_14()
{
	stageCamLockAll();
}

TStageGreenBoss::~TStageGreenBoss()
{
}
