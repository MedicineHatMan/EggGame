#include "stage/Stage.h"
#include "object/TObjHelios.h"

#include "stage/TStageLast.h"
#include "core/TMainTask.h"

typedef char TStageLast_size_check[(sizeof(TStageLast) == 0x30) ? 1 : -1];

// 0x8019A9BC
extern "C" TStage* createLastStage()
{
	return new TStageLast;
}

TStageLast::TStageLast()
{
}

void TStageLast::loadAssets()
{
}

void TStageLast::freeAssets()
{
}

void TStageLast::vfn_14()
{
	Vec3f pos;
	pos.x = -4300.0f;
	pos.y = 1430.0f;
	pos.z = 3450.0f;
	createTObjHelios(gTL_02, &pos);
}

TStageLast::~TStageLast()
{
}
