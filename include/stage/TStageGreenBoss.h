#ifndef _STAGE_TSTAGEGREENBOSS_H_
#define _STAGE_TSTAGEGREENBOSS_H_
#include "stage/Stage.h"
struct TStageGreenBoss;

#include "core/types.h"

class TStage;

extern "C" {
TStage* createGreenBossStage();
}

class TStageGreenBoss : public TStage {
public:
	TStageGreenBoss();
	virtual ~TStageGreenBoss();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

#endif
