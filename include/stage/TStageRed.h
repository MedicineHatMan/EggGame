#ifndef _STAGE_TSTAGERED_H_
#define _STAGE_TSTAGERED_H_
#include "stage/Stage.h"
struct TStageRed;

#include "core/types.h"

class TStage;

extern "C" {
TStage* createRedStage();
}

class TStageRed : public TStage {
public:
	TStageRed();
	virtual ~TStageRed();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

#endif
