#ifndef _STAGE_TSTAGELASTBOSS_H_
#define _STAGE_TSTAGELASTBOSS_H_
#include "stage/Stage.h"
struct TStageLastBoss;

#include "core/types.h"

class TStage;

extern "C" {
TStage* createLastBossStage();
}

class TStageLastBoss : public TStage {
public:
	TStageLastBoss();
	virtual ~TStageLastBoss();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

#endif
