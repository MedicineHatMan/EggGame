#ifndef _STAGE_TSTAGEYELLOWBOSS_H_
#define _STAGE_TSTAGEYELLOWBOSS_H_
#include "stage/Stage.h"
struct TStageYellowBoss;

#include "core/types.h"

class TStage;

extern "C" {
TStage* createYellowBossStage();
}

class TStageYellowBoss : public TStage {
public:
	TStageYellowBoss();
	virtual ~TStageYellowBoss();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

#endif
