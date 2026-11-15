#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

typedef struct SyMallocFuncs {
	void* (*malloc)(u32 size);
	void (*free)(void* mem);
	void* (*calloc)(u32 count, u32 size);
	void* (*realloc)(void* mem, u32 size);
} SyMallocFuncs;

extern const SyMallocFuncs syMallocFuncs;
OSHeapHandle syHeap; // the heap
const SyMallocFuncs* syMallocPtr;

typedef struct SyPad {
	u8 _00[0x30]; // _00
	u8* buf;      // _30
} SyPad;

u8 syPadBufs[0x1E0];
SyPad syPads[4];
u8 syPadStatus[0x30];

// 0x80215B4C
// Hand written with lbzu and bdnz
// 0x80215B4C
asm void syMemCopy(void* dst, const void* src, s32 size)
{
	nofralloc
	cmpwi r5, 0x0
	beqlr
	subi r4, r4, 0x1
	subi r3, r3, 0x1
	mtctr r5
L_80215B60:
	lbzu r0, 0x1(r4)
	stbu r0, 0x1(r3)
	bdnz L_80215B60
	blr
}

// 0x80215B70
// 0x80215B70
asm void syMemCopy2(void* dst, const void* src, s32 count)
{
	nofralloc
	cmpwi r5, 0x0
	beqlr
	subi r4, r4, 0x2
	subi r3, r3, 0x2
	mtctr r5
L_80215B84:
	lhzu r0, 0x2(r4)
	sthu r0, 0x2(r3)
	bdnz L_80215B84
	blr
}

// 0x80215B94
void syMemSet(void* dst, u8 value, u32 size)
{
	memset(dst, value, size);
}

// 0x80215BB8
// Eight words at a time then what is left
// 0x80215BB8
asm void syMemSet4(void* dst, u32 value, s32 count)
{
	nofralloc
	cmpwi r5, 0x0
	blelr
	srwi r0, r5, 3
	cmplwi r0, 0x0
	mtctr r0
	beq L_80215C00
L_80215BD0:
	stw r4, 0x0(r3)
	stw r4, 0x4(r3)
	stw r4, 0x8(r3)
	stw r4, 0xc(r3)
	stw r4, 0x10(r3)
	stw r4, 0x14(r3)
	stw r4, 0x18(r3)
	stw r4, 0x1c(r3)
	addi r3, r3, 0x20
	bdnz L_80215BD0
	andi. r5, r5, 0x7
	beqlr
L_80215C00:
	mtctr r5
L_80215C04:
	stw r4, 0x0(r3)
	addi r3, r3, 0x4
	bdnz L_80215C04
	blr
}

// 0x80215C14
void syMallocInit(void* mem, u32 size)
{
	syHeap = OSCreateHeap(mem, (u8*)mem + size);
	syMallocPtr = &syMallocFuncs;
}

// 0x80215C48
void* syMalloc(u32 size)
{
	return syMallocPtr->malloc(size);
}

// 0x80215C74
void* syCalloc(u32 count, u32 size)
{
	return syMallocPtr->calloc(count, size);
}

// 0x80215CA0
void syFree(void* mem)
{
	syMallocPtr->free(mem);
}

// 0x80215CCC
void syMallocStat(s32* used, s32* blocks)
{
	*used = OSCheckHeap(syHeap);
	*blocks = 0;
}

// 0x80215D14
// Slot 0
void* syHeapAlloc(u32 size)
{
	return OSAllocFromHeap(syHeap, size);
}

// 0x80215D3C
// Slot 1
void syHeapFree(void* mem)
{
	OSFreeToHeap(syHeap, mem);
}

// 0x80215D64
// Slot 2
void* syHeapCalloc(u32 count, u32 size)
{
	int total = (int)count * (int)size;
	void* mem = OSAllocFromHeap(syHeap, total);
	syMemSet(mem, 0, total);
	return mem;
}

// 0x80215DB8
// Slot 3 Copies the new size whatever the old size was
void* syHeapRealloc(void* mem, u32 size)
{
	void* out = OSAllocFromHeap(syHeap, size);
	syMemCopy(out, mem, size);
	OSFreeToHeap(syHeap, mem);
	return out;
}

// 0x80215E1C
// Points each of the four pads at its buffer 0x78 bytes each and reads them once
void syPadInit(void)
{
	syPads[0].buf = syPadBufs;
	syPads[1].buf = syPadBufs + 0x78;
	syPads[2].buf = syPadBufs + 0xF0;
	syPads[3].buf = syPadBufs + 0x168;
	PADRead(syPadStatus);
}

/* The DOL has this table in .rodata */
__declspec(section ".rodata") const SyMallocFuncs syMallocFuncs = { syHeapAlloc, syHeapFree, syHeapCalloc, syHeapRealloc };
