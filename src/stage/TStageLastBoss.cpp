#include "stage/Stage.h"

#include "stage/TStageLastBoss.h"

typedef char TStageLastBoss_size_check[(sizeof(TStageLastBoss) == 0x30) ? 1 : -1];

// 0x801AEB60
extern "C" TStage* createLastBossStage()
{
	return new TStageLastBoss;
}

// 0x801AEB98
TStageLastBoss::TStageLastBoss()
{
}

// 0x801AEBD4
void TStageLastBoss::loadAssets()
{
	stageLoadBossConsole(this);
}

// 0x801AEBF4
void TStageLastBoss::freeAssets()
{
	stageFreeBossConsole();
}

// 0x801AEC14
void TStageLastBoss::vfn_14()
{
	stageCamLockAll();
}

// 0x801AEC34
TStageLastBoss::~TStageLastBoss()
{
}
