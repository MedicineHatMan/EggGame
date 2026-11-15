#ifndef _STAGE_TSTAGEORANGEBOSS_H_
#define _STAGE_TSTAGEORANGEBOSS_H_
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct TStageOrangeBoss;

#include "core/types.h"

class TStage;

extern "C" {
TStage* createOrangeBossStage();
void orangeStageMirrorsOff();
void orangeStageMirrorsOn();
}

class TStageOrangeBoss : public TStage {
public:
	TStageOrangeBoss();
	virtual ~TStageOrangeBoss();
	virtual void loadAssets();
	virtual void freeAssets();
	virtual void vfn_14();
	virtual void run(u32 flags);

	u32 mFlags; // _30 bit 0 = draw the mirror passes
};

#include "Dolphin/GX/GXTypes.h"
extern "C" {
extern const GXColor sStageOrangeBossClearColor;
}

#endif
