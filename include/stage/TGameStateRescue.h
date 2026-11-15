#ifndef _STAGE_TGAMESTATERESCUE_H_
#define _STAGE_TGAMESTATERESCUE_H_

#include "core/types.h"

class TGameContext;

extern "C" {
void* createGameStateRescue(void*, TGameContext*);
void* createGameStateRescueErase(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameStateRescueName;
extern const char* gTGameStateRescueEraseName;
}

#endif
