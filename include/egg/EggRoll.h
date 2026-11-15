#ifndef _EGG_EGGROLL_H_
#define _EGG_EGGROLL_H_
struct EggColModel;
struct EggHitInfo;

#include "core/types.h"

extern "C" {
void eggNoteFace(EggColModel*, u32, void*, const f32*, EggHitInfo*);
}

#endif
