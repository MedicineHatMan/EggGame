#include "stage/Stage.h"
#include "core/TMainTask.h"
#include "core/Mem.h"
#include "Ninja/ninja.h"

// 0x8000C1D0
void* memAlloc(u32 size)
{
	return syMalloc(size);
}

// 0x8000C1F0
void* memCalloc(u32 size, u32 count)
{
	return syCalloc(size, count);
}

// 0x8000C210
void memFree(void* mem)
{
	syFree(mem);
}

// 0x8000C230
void* operator new(unsigned long size)
{
	return syMalloc(size);
}

// 0x8000C250
void operator delete(void* mem)
{
	syFree(mem);
}

// 0x8000C270
void* operator new[](unsigned long size)
{
	return syMalloc(size);
}

// 0x8000C290
void operator delete[](void* mem)
{
	syFree(mem);
}

// 0x8000C2B0
void memDebugDrawBegin()
{
}

// 0x8000C2B4
void memDebugInit()
{
}

// 0x8000C2B8
void memDebugDrawEnd()
{
}

// 0x8000C2BC
void objDumpPrintf(const char*, ...)
{
}

// 0x8000C30C
void memDumpPrintf(const char*, ...)
{
}

// 0x8000C35C
void memDebugObject(TObject* obj)
{
}

// 0x8000C360
void memShowStats()
{
	if (sMemShowHeap != 0) {
		u32 free, max;
		syMallocStat((s32*)&free, (s32*)&max);
		njPrint(0x140004, "%06X : Heap free size", free);
		njPrint(0x140005, "%06X : Heap Max size", max);
		njPrint(0x140006, "%06d : Exec Time", gMainTask._1C);
		njPrint(0x140007, "%06d : Disp Time", gMainTask.mSize);
	}
}

/**
 * A heap block shown by the debug print: its address, size and owner. One is built at start-up. Our name.
 * @size 0x2C
 */
class TMemBlock : public TObject {
public:
	TMemBlock()
	    : TObject(0)
	{
	}
	virtual ~TMemBlock() { }

	virtual void vfn_14() { memDumpPrintf("Adr %08X Sz %06X Cl %08X", mAddr, mBytes, mOwner); }

	u32 mAddr;  // _20
	u32 mBytes; // _24
	u32 mOwner; // _28
};

// 0x8000C414
// is the static initializer the inline destructor and print follow it
TMemBlock gMemBlock;
