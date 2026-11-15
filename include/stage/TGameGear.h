#ifndef _STAGE_TGAMEGEAR_H_
#define _STAGE_TGAMEGEAR_H_

#include "core/types.h"

class TObject;

extern "C" {
void* createGameGear(TObject*);
}

extern "C" {
extern const char* gTGameGearName;
}

#endif
