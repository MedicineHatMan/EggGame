#ifndef _STAGE_TGAMESTATECOLLECT_H_
#define _STAGE_TGAMESTATECOLLECT_H_

#include "core/types.h"

class TGameContext;

extern "C" {
void* createGameStateCollect(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameStateCollectName;
}

#endif
