#include "stage/Stage.h"
#include "object/blue/TObjBlueSea.h"
#include "object/blue/TObjBlueFlag.h"

#include "stage/TStageBlue.h"

typedef char TStageBlue_size_check[(sizeof(TStageBlue) == 0x30) ? 1 : -1];

// 0x8006BDD0
extern "C" TStage* createBlueStage()
{
	return new TStageBlue;
}

// 0x8006BE08
TStageBlue::TStageBlue()
{
}

// 0x8006BE44
void TStageBlue::loadAssets()
{
	initBlueSea();
	initBlueFlag();
}

// 0x8006BE68
void TStageBlue::freeAssets()
{
	termBlueFlag();
	termBlueSea();
}

// 0x8006BE8C
void TStageBlue::vfn_14()
{
	createTObjBlueSea();
}

// 0x8006BEAC
TStageBlue::~TStageBlue()
{
}
