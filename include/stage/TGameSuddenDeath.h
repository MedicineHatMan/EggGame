#ifndef _STAGE_TGAMESUDDENDEATH_H_
#define _STAGE_TGAMESUDDENDEATH_H_

#include "core/types.h"

class TGameContextBattle;

extern "C" {
void* createGameStateSuddenDeath(void*, TGameContextBattle*);
}

extern "C" {
void* createGameSuddenDeath();
}

extern "C" {
extern const char* gTGameSuddenDeathName;
extern const char* gTGameStateSuddenDeathName;
}

#endif
