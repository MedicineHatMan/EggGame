#ifndef _STAGE_TGAMESTATESURVIVAL_H_
#define _STAGE_TGAMESTATESURVIVAL_H_

#include "core/types.h"

class TGameContextBattle;

extern "C" {
void* createGameStateSurvival(void*, TGameContextBattle*);
}

extern "C" {
extern const char* gTGameStateSurvivalName;
}

#endif
