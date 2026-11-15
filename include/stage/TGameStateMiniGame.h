#ifndef _STAGE_TGAMESTATEMINIGAME_H_
#define _STAGE_TGAMESTATEMINIGAME_H_

#include "core/types.h"

class TGameContext;

extern "C" {
void* createGameStateMiniGame(void*, TGameContext*);
void* createGameStateMiniGameWait(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameStateMiniGameWaitName;
extern const char* gTGameStateMiniGameName;
}

#endif
