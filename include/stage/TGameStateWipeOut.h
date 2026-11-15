#ifndef _STAGE_TGAMESTATEWIPEOUT_H_
#define _STAGE_TGAMESTATEWIPEOUT_H_

#include "core/types.h"

class TGameContext;

extern "C" {
void* createGameStateWipeOut(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameStateWipeOutName;
}

#endif
