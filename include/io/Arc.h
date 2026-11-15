#ifndef _IO_ARC_H_
#define _IO_ARC_H_

#include "core/types.h"

class TArc;

extern "C" {
void* findArcExport(TArc*, const char*);
void relocateArc(u8*, u8*, u32);
}

#endif
