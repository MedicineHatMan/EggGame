#ifndef _STAGE_TSTAGEBLUE_H_
#define _STAGE_TSTAGEBLUE_H_
#include "stage/Stage.h"
struct TStageBlue;

#include "core/types.h"

class TStage;

extern "C" {
TStage* createBlueStage();
}

class TStageBlue : public TStage {
public:
	TStageBlue();
	virtual ~TStageBlue();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

#endif
