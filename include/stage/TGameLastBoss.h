#ifndef _STAGE_TGAMELASTBOSS_H_
#define _STAGE_TGAMELASTBOSS_H_

#include "core/types.h"

class TGameContext;
class TObject;

extern "C" {
void* createGameLastBoss(TObject*);
void* createGameStateLastBoss(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameLastBossName;
extern const char* gTGameStateLastBossName;
}

#endif
