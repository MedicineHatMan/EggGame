#ifndef _STAGE_STAGETIME_H_
#define _STAGE_STAGETIME_H_
struct StageTime;

#include "core/types.h"

extern "C" {
void stageTimerTick(u32*);
StageTime* stageTimeAdd(StageTime*, const StageTime*);
StageTime* stageTimeSub(StageTime*, const StageTime*);
}

extern "C" {
void stageTimeSet(StageTime*, u32);
}

#endif
