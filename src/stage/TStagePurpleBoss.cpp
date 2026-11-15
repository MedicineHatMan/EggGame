#include "stage/TStagePurple.h"
#include "object/purple/TObjPurpleBlizzard.h"
#include "io/Resource.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjWindColli.h"
#include "stage/Stage.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "stl/math.h"
#include "core/VecMath.h"
#include "object/TObjSnowman.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;
struct Fx;

// 0x800FC4E0
extern "C" TStage* createPurpleBossStage()
{
	return new TStagePurpleBoss;
}

// 0x800FC518
TStagePurpleBoss::TStagePurpleBoss()
{
}

// 0x800FC554
// stageLoadBossConsole calls bossConsoleLoad(mName)
void TStagePurpleBoss::loadAssets()
{
	stageLoadBossConsole(this);
}

// 0x800FC574
// stageFreeBossConsole calls bossConsoleFree
void TStagePurpleBoss::freeAssets()
{
	stageFreeBossConsole();
}

// 0x800FC594
// stageCamLockAll calls camLockAll
void TStagePurpleBoss::vfn_14()
{
	stageCamLockAll();
}

// 0x800FC5B4
TStagePurpleBoss::~TStagePurpleBoss()
{
}
