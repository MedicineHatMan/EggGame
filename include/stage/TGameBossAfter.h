#ifndef _STAGE_TGAMEBOSSAFTER_H_
#define _STAGE_TGAMEBOSSAFTER_H_

#include "core/types.h"

class TGameContext;
class TObject;

extern "C" {
void* createGameBossAfter(TObject*);
void* createGameStateBossAfter(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameBossAfterName;
extern const char* gTGameStateBossAfterName;
}

#endif
