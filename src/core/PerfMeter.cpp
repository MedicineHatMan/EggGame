#include "core/types.h"
#include "Dolphin/GX/GXPerf.h"
#include "Dolphin/GX/GXMisc.h"
#include "Dolphin/OS/OSUtil.h"

#include "core/PerfMeter.h"

extern "C" {

static u32 sPerfStart[PERF_MAX];
static u32 sPerfDrawDone[PERF_MAX];
static u32 sPerfEnd[PERF_MAX];
static u32 sPerfPixelsIn[PERF_MAX];
static u32 sPerfPixelsOut[PERF_MAX];
static u32 sPerfColor[PERF_MAX];
static const char* sPerfName[PERF_MAX];

// 0x8004CB7C
void perfBegin(u32 color, const char* name)
{
	if (sPerfOn != 0) {
		if (sPerfCount >= sPerfFirst && sPerfCount < sPerfLast) {
			sPerfIndex++;
			GXClearGPMetric();
			GXClearPixMetric();
			sPerfStart[sPerfIndex] = OSGetTick();
			sPerfColor[sPerfIndex] = color;
			sPerfName[sPerfIndex]  = name;
		}
		sPerfCount++;
	}
}

// 0x8004CC1C
void perfEnd()
{
	u32 topIn, topOut, botIn, botOut, clrIn, copyClks;

	if (sPerfOn != 0 && sPerfIndex >= 0 && sPerfIndex < PERF_MAX) {
		GXSetDrawSync(sPerfIndex);
		sPerfDrawDone[sPerfIndex] = OSGetTick();
		while (sPerfIndex != GXReadDrawSync()) { }
		GXReadPixMetric(&topIn, &topOut, &botIn, &botOut, &clrIn, &copyClks);
		GXClearPixMetric();
		sPerfPixelsIn[sPerfIndex]  = topIn + botIn;
		sPerfPixelsOut[sPerfIndex] = topOut + botOut;
		sPerfEnd[sPerfIndex]       = OSGetTick();
	}
}
}
