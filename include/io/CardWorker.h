#ifndef _IO_CARDWORKER_H_
#define _IO_CARDWORKER_H_

#include "core/types.h"

struct CardWorker;

extern "C" {
CardWorker* createCardWorker(int);
}

#endif
