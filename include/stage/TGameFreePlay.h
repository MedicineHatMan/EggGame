#ifndef _STAGE_TGAMEFREEPLAY_H_
#define _STAGE_TGAMEFREEPLAY_H_

#include "core/types.h"

class TGameContext;
class TObject;

extern "C" {
void* createGameFreePlay(TObject*);
void* createGameStateFreePlay(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameFreePlayName;
extern const char* gTGameStateFreePlayName;
}

#endif
