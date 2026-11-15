#ifndef _CORE_MEM_H_
#define _CORE_MEM_H_
struct TObject;

#include "core/types.h"

extern "C" {
void memDebugDrawBegin();
void memDebugInit();
void memDebugDrawEnd();
void objDumpPrintf(const char*, ...);
void memDumpPrintf(const char*, ...);
void memShowStats();
void memFree(void*);
}

extern "C" {
void* memAlloc(u32);
}

extern "C" {
void* memCalloc(u32, u32);
void memDebugObject(TObject*);
}

extern "C" {
extern int sMemShowHeap;
}

#endif
