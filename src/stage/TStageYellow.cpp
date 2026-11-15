#include "stage/Stage.h"
#include "object/yellow/TObjYellowDoubleFog.h"
#include "object/yellow/TObjYellowTornadoScene.h"
#include "core/TMainTask.h"

class TStageYellow : public TStage {
public:
	TStageYellow();
	virtual ~TStageYellow();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

typedef char TStageYellow_size_check[(sizeof(TStageYellow) == 0x30) ? 1 : -1];

// 0x8017D830
extern "C" TStage* createYellowStage()
{
	return new TStageYellow;
}

// 0x8017D868
TStageYellow::TStageYellow()
{
}

// 0x8017D8A4
void TStageYellow::loadAssets()
{
	initYellowDoubleFog();
}

// 0x8017D8C4
void TStageYellow::freeAssets()
{
	termYellowDoubleFog();
}

// 0x8017D8E4
void TStageYellow::vfn_14()
{
	createYellowDoubleFog();
	createTObjYellowTornadoScene(gTL_02, 0, 0);
	createTObjYellowTornadoScene(gTL_02, 1, 0);
	createTObjYellowTornadoScene(gTL_02, 2, 0);
}

// 0x8017D934
TStageYellow::~TStageYellow()
{
}
