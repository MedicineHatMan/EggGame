#ifndef _STAGE_TSTAGETITLE_H_
#define _STAGE_TSTAGETITLE_H_
#include "stage/Stage.h"
struct TStageTitle;

#include "core/types.h"

class TStage;

extern "C" {
TStage* createTitleStage();
}

class TStageTitle : public TStage {
public:
	TStageTitle();
	virtual ~TStageTitle();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

#endif
