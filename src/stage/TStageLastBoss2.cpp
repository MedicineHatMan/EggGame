#include "stage/Stage.h"
#include "object/last/TObjLastExBoss_BGCtrl.h"
#include "player/SuperBillyRes.h"
#include "stage/TStageLastBoss2.h"

extern "C" {
}

typedef char TStageLastBoss2_size_check[(sizeof(TStageLastBoss2) == 0x30) ? 1 : -1];

// 0x8017BAE0
extern "C" TStage* createLastBoss2Stage()
{
	return new TStageLastBoss2;
}

// 0x8017BB18
TStageLastBoss2::TStageLastBoss2() { }

// 0x8017BB54
void TStageLastBoss2::loadAssets()
{
	stageLoadBossConsole(this);
	clearLastExBg();
	superBillyLoad();
}

// 0x8017BB7C
void TStageLastBoss2::freeAssets()
{
	releaseLastExBg();
	((void (*)(void*))stageFreeBossConsole)(this);
	superBillyFree();
}

// 0x8017BBB4
void TStageLastBoss2::vfn_14()
{
	stageCamLockAll();
	createTObjLastExBoss_BGCtrl();
	spawnPlyDemoLastCtrl();
}

// 0x8017BBDC
TStageLastBoss2::~TStageLastBoss2() { }
