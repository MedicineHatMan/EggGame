#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

u32 syTmrGetCount(void)
{
	return OSGetTime() >> 6;
}

// 0x80216458
u32 syTmrDiffCount(u32 from, u32 to)
{
	return to - from;
}

// 0x80216460
u32 syTmrCountToMicro(u32 count)
{
	return (count << 6) * 8 / 324;
}
