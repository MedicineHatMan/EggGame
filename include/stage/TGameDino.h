#ifndef _STAGE_TGAMEDINO_H_
#define _STAGE_TGAMEDINO_H_

#include "core/types.h"

class TObject;

extern "C" {
void* createGameDino(TObject*);
}

extern "C" {
extern const char* gTGameDinoName;
}

#endif
