#ifndef _STAGE_TGAMEGOLDENEGG_H_
#define _STAGE_TGAMEGOLDENEGG_H_

#include "core/types.h"

class TObject;

extern "C" {
void* createGameGoldenEgg(TObject*);
}

extern "C" {
extern const char* gTGameGoldenEggName;
}

#endif
