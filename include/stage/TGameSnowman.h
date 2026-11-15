#ifndef _STAGE_TGAMESNOWMAN_H_
#define _STAGE_TGAMESNOWMAN_H_

#include "core/types.h"

class TGameContext;
class TObject;

extern "C" {
void* createGameSnowman(TObject*);
void* createGameStateSnowman(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameSnowmanName;
extern const char* gTGameStateSnowmanName;
}

#endif
