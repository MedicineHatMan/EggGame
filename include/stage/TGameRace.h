#ifndef _STAGE_TGAMERACE_H_
#define _STAGE_TGAMERACE_H_

#include "core/types.h"

class TObject;

extern "C" {
void* createGameRace(TObject*);
}

extern "C" {
extern const char* gTGameRaceName;
}

#endif
