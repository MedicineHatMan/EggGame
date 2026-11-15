#ifndef _STAGE_TGAMECOLLECT_H_
#define _STAGE_TGAMECOLLECT_H_

#include "core/types.h"

class TObject;

extern "C" {
void* createGameCollect(TObject*);
}

extern "C" {
extern const char* gTGameCollectName;
}

#endif
