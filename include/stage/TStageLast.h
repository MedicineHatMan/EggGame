#ifndef _STAGE_TSTAGELAST_H_
#define _STAGE_TSTAGELAST_H_
#include "stage/Stage.h"
struct TStageLast;

#include "core/types.h"

class TStage;

extern "C" {
TStage* createLastStage();
}

class TStageLast : public TStage {
public:
	TStageLast();
	virtual ~TStageLast();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
};

#endif
