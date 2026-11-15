#include <Dolphin/ax.h>
#include "Dolphin/__ax.h"
#define NULL 0

static long __AXBufferAuxA[3][480] ATTRIBUTE_ALIGN(32); // 0x80287D20
static long __AXBufferAuxB[3][480] ATTRIBUTE_ALIGN(32); // 0x802893A0

static void (*__AXCallbackAuxA)(void*, void*);
static void (*__AXCallbackAuxB)(void*, void*);
static void* __AXContextAuxA;
static void* __AXContextAuxB;
static long* __AXAuxADspWrite;
static long* __AXAuxADspRead;
static long* __AXAuxBDspWrite;
static long* __AXAuxBDspRead;
static u32 __AXAuxDspWritePosition;
static u32 __AXAuxDspReadPosition;
static u32 __AXAuxDspWritePositionDPL2;
static u32 __AXAuxDspReadPositionDPL2;
static u32 __AXAuxCpuReadWritePosition;

// 0x801E5D20
void __AXAuxInit(void)
{
	int i;
	long* pA;
	long* pB;

	__AXCallbackAuxA            = NULL;
	__AXCallbackAuxB            = NULL;
	__AXContextAuxA             = 0;
	__AXContextAuxB             = 0;
	__AXAuxDspWritePosition     = 0;
	__AXAuxDspReadPosition      = 1;
	__AXAuxDspWritePositionDPL2 = 0;
	__AXAuxDspReadPositionDPL2  = 1;
	__AXAuxCpuReadWritePosition = 2;
	pA = (long*)&__AXBufferAuxA;
	pB = (long*)&__AXBufferAuxB;
	for (i = 0; i < 0x1E0; i++) {
		*(pA) = 0;
		pA += 1;
		*(pB) = 0;
		pB += 1;
	}
}

// 0x801E5E04
void __AXGetAuxAInput(u32* p)
{
	if (__AXCallbackAuxA) {
		*p = (u32)&__AXBufferAuxA[__AXAuxDspWritePosition][0];
		return;
	}
	*p = 0;
}

// 0x801E5E38
void __AXGetAuxAInputDpl2(u32* p)
{
	*p = (u32)&__AXBufferAuxB[__AXAuxDspWritePosition][320];
}

// 0x801E5E58
void __AXGetAuxAOutput(u32* p)
{
	*p = (u32)&__AXBufferAuxA[__AXAuxDspReadPosition][0];
}

// 0x801E5E74
void __AXGetAuxAOutputDpl2R(u32* p)
{
	*p = (u32)&__AXBufferAuxA[__AXAuxDspReadPosition][160];
}

// 0x801E5E94
void __AXGetAuxAOutputDpl2Ls(u32* p)
{
	*p = (u32)&__AXBufferAuxA[__AXAuxDspReadPosition][320];
}

// 0x801E5EB4
void __AXGetAuxAOutputDpl2Rs(u32* p)
{
	*p = (u32)&__AXBufferAuxB[__AXAuxDspReadPosition][320];
}

// 0x801E5ED4
void __AXGetAuxBInput(u32* p)
{
	if (__AXCallbackAuxB) {
		*p = (u32)&__AXBufferAuxB[__AXAuxDspWritePosition][0];
		return;
	}
	*p = 0;
}

// 0x801E5F08
void __AXGetAuxBOutput(u32* p)
{
	*p = (u32)&__AXBufferAuxB[__AXAuxDspReadPosition][0];
}

// 0x801E5F24
void __AXGetAuxBForDPL2(u32* p)
{
	*p = (u32)&__AXBufferAuxB[__AXAuxDspWritePositionDPL2][0];
}

// 0x801E5F40
void __AXGetAuxBOutputDPL2(u32* p)
{
	*p = (u32)&__AXBufferAuxB[__AXAuxDspReadPositionDPL2][0];
}

// 0x801E5F5C
void __AXProcessAux(void)
{
	AX_AUX_DATA_DPL2 dpl2Data;
	AX_AUX_DATA auxAData;
	AX_AUX_DATA auxBData;

	__AXAuxADspWrite = &__AXBufferAuxA[__AXAuxDspWritePosition][0];
	__AXAuxADspRead  = &__AXBufferAuxA[__AXAuxDspReadPosition][0];
	__AXAuxBDspWrite = &__AXBufferAuxB[__AXAuxDspWritePosition][0];
	__AXAuxBDspRead  = &__AXBufferAuxB[__AXAuxDspReadPosition][0];
	if (__AXCallbackAuxA) {
		if (__AXClMode == AX_MODE_DPL2) {
			dpl2Data.l  = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][0];
			dpl2Data.r  = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][160];
			dpl2Data.ls = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][320];
			dpl2Data.rs = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][320];
			DCInvalidateRange(dpl2Data.l, 0x780);
			DCInvalidateRange(dpl2Data.rs, 0x280);
			__AXCallbackAuxA(&dpl2Data.l, __AXContextAuxA);
			DCFlushRangeNoSync(dpl2Data.l, 0x780);
			DCFlushRangeNoSync(dpl2Data.rs, 0x280);
		} else {
			auxAData.l = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][0];
			auxAData.r = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][160];
			auxAData.s = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][320];
			DCInvalidateRange(auxAData.l, 0x780);
			__AXCallbackAuxA(&auxAData.l, __AXContextAuxA);
			DCFlushRangeNoSync(auxAData.l, 0x780);
		}
	}
	if (__AXCallbackAuxB && __AXClMode != AX_MODE_DPL2) {
		auxBData.l = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][0];
		auxBData.r = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][160];
		auxBData.s = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][320];
		DCInvalidateRange(auxBData.l, 0x780);
		__AXCallbackAuxB(&auxBData.l, __AXContextAuxB);
		DCFlushRangeNoSync(auxBData.l, 0x780);
	}
	__AXAuxDspWritePosition++;
	__AXAuxDspWritePosition %= 3;
	__AXAuxDspReadPosition++;
	__AXAuxDspReadPosition %= 3;
	__AXAuxDspWritePositionDPL2++;
	__AXAuxDspWritePositionDPL2 %= 2;
	__AXAuxDspReadPositionDPL2++;
	__AXAuxDspReadPositionDPL2 %= 2;
	__AXAuxCpuReadWritePosition++;
	__AXAuxCpuReadWritePosition %= 3;
}

// 0x801E6198
void AXRegisterAuxACallback(void (*callback)(void*, void*), void* context)
{
	__AXCallbackAuxA = callback;
	__AXContextAuxA  = context;
}

// 0x801E61A4
void AXRegisterAuxBCallback(void (*callback)(void*, void*), void* context)
{
	__AXCallbackAuxB = callback;
	__AXContextAuxB  = context;
}
