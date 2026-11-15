#ifndef _STAGE_TGAMERESCUE_H_
#define _STAGE_TGAMERESCUE_H_

#include "core/types.h"

class TObject;

extern "C" {
void* createGameRescue(TObject*);
}

extern "C" {
extern const char* gTGameRescueName;
}

#endif
