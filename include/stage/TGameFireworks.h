#ifndef _STAGE_TGAMEFIREWORKS_H_
#define _STAGE_TGAMEFIREWORKS_H_

#include "core/types.h"

class TGameContext;
class TObject;

extern "C" {
void* createGameFireworks(TObject*);
void* createGameStateFireworks(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameFireworksName;
extern const char* gTGameStateFireworksName;
}

#endif
