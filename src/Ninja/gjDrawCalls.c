#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

// The original name of this file is unknown. The DOL keeps these in .bss and .sdata blocks of their own
// between njMotionObj.c's and njPrint.c's
GjDrawCalls gjDrawCallsDefault;
GjDrawCalls* gjDrawCalls = &gjDrawCallsDefault;
