#ifndef _IO_RESTOC_H_
#define _IO_RESTOC_H_

#include "core/types.h"

struct ResToc;

extern "C" {
int resTocFind(const ResToc*, const char*);
u32 resTocOffset(ResToc*, int);
u32 resTocSize(ResToc*, int);
}

#endif
