#include "stage/Stage.h"
#include "enemy/boss/TEneBoss03Mole.h"

#include "stage/TStageRedBoss.h"

typedef char TStageRedBoss_size_check[(sizeof(TStageRedBoss) == 0x30) ? 1 : -1];

// 0x80088098
extern "C" TStage* createRedBossStage()
{
	return new TStageRedBoss;
}

TStageRedBoss::TStageRedBoss() { }

void TStageRedBoss::loadAssets()
{
	stageLoadBossConsole(this);
}

void TStageRedBoss::freeAssets()
{
	stageFreeBossConsole();
}

void TStageRedBoss::vfn_14()
{
	stageCamLockAll();
	createBoss03Magma();
}

TStageRedBoss::~TStageRedBoss() { }
