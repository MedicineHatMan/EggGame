#ifndef _CORE_PERFMETER_H_
#define _CORE_PERFMETER_H_

#include "core/types.h"

extern "C" {
void perfEnd();
}

extern "C" {
void perfBegin(u32, const char*);
}

#define PERF_MAX 40

extern "C" {
extern int sPerfOn;
extern int sPerfFirst;
extern int sPerfLast;
extern int sPerfCount;
extern int sPerfIndex;
}

#endif
