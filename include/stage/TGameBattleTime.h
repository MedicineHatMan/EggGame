#ifndef _STAGE_TGAMEBATTLETIME_H_
#define _STAGE_TGAMEBATTLETIME_H_

#include "core/types.h"

class TGameContextBattle;

extern "C" {
void* createGameStateTime(void*, TGameContextBattle*);
}

extern "C" {
void* createGameBattleTime();
}

extern "C" {
extern const char* gTGameBattleTimeName;
extern const char* gTGameStateTimeName;
}

#endif
