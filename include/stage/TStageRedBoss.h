#ifndef _STAGE_TSTAGEREDBOSS_H_
#define _STAGE_TSTAGEREDBOSS_H_
#include "stage/Stage.h"
struct TStageRedBoss;

#include "core/types.h"

class TStage;

extern "C" {
TStage* createRedBossStage();
}

class TStageRedBoss : public TStage {
public:
	TStageRedBoss();
	virtual ~TStageRedBoss();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

#endif
