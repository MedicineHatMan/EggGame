#ifndef _STAGE_TGAMESTATEGEAR_H_
#define _STAGE_TGAMESTATEGEAR_H_

#include "core/types.h"

class TGameContext;

extern "C" {
void* createGameStateGear(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameStateGearName;
}

#endif
