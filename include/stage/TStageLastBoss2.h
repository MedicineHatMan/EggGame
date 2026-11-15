#ifndef _STAGE_TSTAGELASTBOSS2_H_
#define _STAGE_TSTAGELASTBOSS2_H_
#include "stage/Stage.h"
struct TStageLastBoss2;

#include "core/types.h"

class TStage;

extern "C" {
TStage* createLastBoss2Stage();
}

class TStageLastBoss2 : public TStage {
public:
	TStageLastBoss2();
	virtual ~TStageLastBoss2();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

#endif
