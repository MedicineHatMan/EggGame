#include "Dolphin/ax.h"
#include "Dolphin/__ax.h"

const char* __AXVersion = "<< Dolphin SDK - AX\trelease build: Apr 17 2003 12:33:57 (0x2301) >>";

void AXInitEx(u32 outputBufferMode);

void AXInit(void)
{
	AXInitEx(0);
}

void AXInitEx(u32 outputBufferMode)
{
	OSRegisterVersion(__AXVersion);
	__AXAllocInit();
	__AXVPBInit();
	__AXSPBInit();
	__AXAuxInit();
	__AXClInit();
	__AXOutInit(outputBufferMode);
}
