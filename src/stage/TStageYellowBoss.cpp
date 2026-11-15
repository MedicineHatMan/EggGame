#include "stage/Stage.h"
#include "object/yellow/TObjYellowTornadoScene.h"
#include "stage/TStageYellowBoss.h"
#include "core/TMainTask.h"
struct TObjYellowTornadoScene;

typedef char TStageYellowBoss_size_check[(sizeof(TStageYellowBoss) == 0x30) ? 1 : -1];

// 0x80191950
extern "C" TStage* createYellowBossStage()
{
	return new TStageYellowBoss;
}

TStageYellowBoss::TStageYellowBoss() { }

void TStageYellowBoss::loadAssets()
{
	stageLoadBossConsole(this);
}

void TStageYellowBoss::freeAssets()
{
	stageFreeBossConsole();
}

void TStageYellowBoss::vfn_14()
{
	stageCamLockAll();
	createTObjYellowTornadoScene(gTL_02, 3, 1);
	createTObjYellowTornadoScene(gTL_02, 4, 1);
}

TStageYellowBoss::~TStageYellowBoss() { }
