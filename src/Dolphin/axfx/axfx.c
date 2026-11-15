#include "types.h"
#include <Dolphin/axfx.h>
#include "Dolphin/os.h"

static void* __AXFXAllocFunction(u32 bytes)
{
	return OSAllocFromHeap(__OSCurrHeap, bytes);
}

static void __AXFXFreeFunction(void* p)
{
	OSFreeToHeap(__OSCurrHeap, p);
}

void* (*__AXFXAlloc)(u32) = __AXFXAllocFunction;
void (*__AXFXFree)(void*) = __AXFXFreeFunction;

void AXFXSetHooks(void* (*alloc)(u32), void (*free)(void*))
{
	__AXFXAlloc = alloc;
	__AXFXFree  = free;
}
