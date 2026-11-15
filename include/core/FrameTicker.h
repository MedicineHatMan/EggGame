#ifndef _CORE_FRAMETICKER_H_
#define _CORE_FRAMETICKER_H_

#include "core/types.h"

struct FrameTicker;

extern "C" {
FrameTicker* frameTickerGet();
}

#endif
