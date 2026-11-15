#ifndef _ENGINE_GENTEX_H_
#define _ENGINE_GENTEX_H_

#include "core/types.h"

extern "C" {
void genTexExit();
void genTexInit();
void genTexLoad(int, int);
void genTexLoadWrap(int, int, s32);
}

#endif
