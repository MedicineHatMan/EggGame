#include "stage/Stage.h"

#include "stage/TStageTitle.h"

typedef char TStageTitle_size_check[(sizeof(TStageTitle) == 0x30) ? 1 : -1];

// 0x801AE264
extern "C" TStage* createTitleStage()
{
	return new TStageTitle;
}

// 0x801AE29C
TStageTitle::TStageTitle()
{
}

// 0x801AE2D8
void TStageTitle::loadAssets()
{
}

// 0x801AE2DC
void TStageTitle::freeAssets()
{
}

// 0x801AE2E0
void TStageTitle::vfn_14()
{
}

// 0x801AE2E4
TStageTitle::~TStageTitle()
{
}
