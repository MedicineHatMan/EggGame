#ifndef _ENGINE_FXSETUP_H_
#define _ENGINE_FXSETUP_H_
struct FxGroup;

#include "core/types.h"

extern "C" {
void fxDrawEnd();
}

extern "C" {
void fxDrawBegin(FxGroup*);
}

#endif
