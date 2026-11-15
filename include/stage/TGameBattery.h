#ifndef _STAGE_TGAMEBATTERY_H_
#define _STAGE_TGAMEBATTERY_H_

#include "core/types.h"

class TObject;

extern "C" {
void* createGameBattery(TObject*);
}

extern "C" {
extern const char* gTGameBatteryName;
}

#endif
