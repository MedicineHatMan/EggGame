#ifndef _STAGE_TGAMESTATEGOLDENEGG_H_
#define _STAGE_TGAMESTATEGOLDENEGG_H_

#include "core/types.h"

class TGameContext;
class TGameStateGoldenEggLost;

extern "C" {
void* createGameStateGoldenEgg(void*, TGameContext*);
void* createGameStateGoldenEggLost(void*, TGameContext*);
int goldenEggLostFound(TGameStateGoldenEggLost*);
}

extern "C" {
extern const char* gTGameStateGoldenEggName;
extern const char* gTGameStateGoldenEggLostName;
}

#endif
