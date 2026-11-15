#include "stage/Stage.h"

#include "stage/TStageRed.h"

typedef char TStageRed_size_check[(sizeof(TStageRed) == 0x30) ? 1 : -1];

// 0x800F43D4
extern "C" TStage* createRedStage()
{
	return new TStageRed;
}

TStageRed::TStageRed() { }

void TStageRed::loadAssets()
{

}

void TStageRed::freeAssets()
{

}

void TStageRed::vfn_14()
{

}

TStageRed::~TStageRed() { }
