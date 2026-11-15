#ifndef _STAGE_TGAMEBOSS_H_
#define _STAGE_TGAMEBOSS_H_

#include "core/types.h"

class TGameContext;
class TObject;

extern "C" {
void* createGameBoss(TObject*);
void* createGameStateBoss(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameBossName;
extern const char* gTGameStateBossName;
}

#endif
