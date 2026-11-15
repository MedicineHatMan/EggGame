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

// 0x800FC408
extern "C" TStage* createPurpleStage()
{
	return new TStagePurple;
}

// 0x800FC440
TStagePurple::TStagePurple()
{
}

// 0x800FC47C
void TStagePurple::loadAssets()
{
}

// 0x800FC480
void TStagePurple::freeAssets()
{
}

// 0x800FC484
void TStagePurple::vfn_14()
{
}

// 0x800FC488
TStagePurple::~TStagePurple()
{
}
