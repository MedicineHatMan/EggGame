#ifndef _STAGE_TGAMESTATERACE_H_
#define _STAGE_TGAMESTATERACE_H_

#include "core/types.h"

class TGameContext;

extern "C" {
void* createGameStateRace(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameStateRaceName;
}

#endif
