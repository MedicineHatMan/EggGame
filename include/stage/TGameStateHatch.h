#ifndef _STAGE_TGAMESTATEHATCH_H_
#define _STAGE_TGAMESTATEHATCH_H_

#include "core/types.h"

class TGameContextBattle;

extern "C" {
void* createGameStateHatch(void*, TGameContextBattle*);
}

extern "C" {
extern const char* gTGameStateHatchName;
}

#endif
