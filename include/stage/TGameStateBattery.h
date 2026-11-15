#ifndef _STAGE_TGAMESTATEBATTERY_H_
#define _STAGE_TGAMESTATEBATTERY_H_

#include "core/types.h"

class TGameContext;

extern "C" {
void* createGameStateBattery(void*, TGameContext*);
}

extern "C" {
extern const char* gTGameStateBatteryName;
}

#endif
