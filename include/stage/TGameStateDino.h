#ifndef _STAGE_TGAMESTATEDINO_H_
#define _STAGE_TGAMESTATEDINO_H_

#include "core/types.h"

class TGameContext;

extern "C" {
void* createGameStateDino(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameStateDinoName;
}

#endif
