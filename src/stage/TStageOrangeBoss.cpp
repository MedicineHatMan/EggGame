#include "stage/Stage.h"
#include "object/ObjAction.h"
#include "enemy/boss/TEneOrangeBossMap.h"
#include "Ninja/ninja.h"
#include "engine/ShadowTex.h"
#include "camera/TCameraCtrl.h"
#include "engine/Viewport.h"
#include "player/Player.h"
#include "engine/DrawTask.h"
#include "object/TObjPlayer.h"
#include "stage/TStageOrangeBoss.h"
struct TObjPlayer;

typedef char TStageOrangeBoss_size_check[(sizeof(TStageOrangeBoss) == 0x34) ? 1 : -1];

TStageOrangeBoss* sOrangeStage; // 0x802C5598

// 0x8011EC14
extern "C" TStage* createOrangeBossStage()
{
	return new TStageOrangeBoss;
}

// 0x8011EC4C
TStageOrangeBoss::TStageOrangeBoss()
{
	sOrangeStage = this;
	mFlags = 0;
}

// 0x8011EC94
TStageOrangeBoss::~TStageOrangeBoss()
{
	sOrangeStage = 0;
}

// 0x8011ECF4
void TStageOrangeBoss::loadAssets()
{
	stageLoadBossConsole(this);
	initOrangeBossMap();
}

// 0x8011ED18
void TStageOrangeBoss::freeAssets()
{
	shutdownOrangeBossMap();
	((void (*)(void*))stageFreeBossConsole)(this);
}

// 0x8011ED4C
void TStageOrangeBoss::vfn_14()
{
	stageCamLockAll();
	mirrorsCreate();
}

// 0x8011ED70
extern "C" void orangeStageMirrorsOn()
{
	if (sOrangeStage == 0) {
		return;
	}
	sOrangeStage->mFlags |= 1;
}

// 0x8011ED8C
extern "C" void orangeStageMirrorsOff()
{
	if (sOrangeStage == 0) {
		return;
	}
	sOrangeStage->mFlags &= ~1;
}

// 0x8011EDA8
void TStageOrangeBoss::run(u32 flags)
{
	if (flags & 1) {
		calc();
	}
	u32 draw = flags & 2;
	if (draw) {
		camSetDrawView(0);
		drawScene();
	}
	if (draw) {
		Vec3f pos = *findPlayer(0)->getPos();
		GXSetCopyClear(sStageOrangeBossClearColor, 0xFFFFFF);
		shadowTexGet()->begin(&pos);
		runDrawTasks(0x1F);
		shadowTexGet()->end();
	}
	if (draw) {
		setViewport(5);
		camSetDrawView(0);
		drawObjects();
		runDrawTasks(0x19);
		if (mFlags & 1) {
			runDrawTasks(0x2C);
			runDrawTasks(0x2D);
		}
		mirrorCopyScreen();
	}
	if (draw) {
		setViewport(0);
		camSetDrawView(0);
		drawObjects();
		drawEffects(!(mFlags & 1));
	}
}
