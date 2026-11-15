#include <gcax/gcax.h>

#define GCAX_BLK_FREE 0x4D46 /* "MF" */
#define GCAX_BLK_BUSY 0x4D42 /* "MB" */

// 0x802306F4
void gcaxSndHeapReset(void)
{
	GCAX_MEMBLK* top;
	GCAX_MEMBLK* end;
	GCAX_MEMBLK** endp;
	BOOL level;

	level = OSDisableInterrupts();
	top = gcax_work->heap_top;
	endp = &end;
	(void)endp;
	end = (GCAX_MEMBLK*)((u8*)gcax_work->heap_top + gcax_work->heap_size - sizeof(GCAX_MEMBLK));
	top->magic = GCAX_BLK_FREE;
	top->pad = 0;
	top->next = end;
	end->magic = GCAX_BLK_BUSY;
	end->pad = 0;
	end->next = 0;
	OSRestoreInterrupts(level);
}

// 0x8023079C
void gcaxSndHeapInit(void* base, u32 size)
{
	BOOL level;

	level = OSDisableInterrupts();
	gcax_work->heap_top = (GCAX_MEMBLK*)(((u32)base + 0x1F) & ~0x1F);
	size -= (0x20 - ((u32)base & 0x1F)) & 0x1F;
	size &= ~3;
	gcax_work->heap_size = size;
	gcaxSndHeapReset();
	OSRestoreInterrupts(level);
}

// 0x80230818
void* gcaxSndMalloc(u32 size)
{
	GCAX_MEMBLK* blk;
	GCAX_MEMBLK* rest;
	u32 len;
	u32 room;
	BOOL level;
	GCAX_MEMBLK* next;

	blk = gcax_work->heap_top;
	level = OSDisableInterrupts();
	len = (size + 0x1F) & ~0x1F;
	while (blk->next != 0) {
		if (blk->magic == GCAX_BLK_FREE) {
			room = (u32)blk->next - (u32)blk - sizeof(GCAX_MEMBLK);
			if (room >= len) {
				goto found;
			}
		}
		blk = blk->next;
	}
	OSReport("AX SDLIB: gacxSndMalloc(%d) failed (should not happen, rest=%d\n", size, gcaxSndHeapRest());
	OSRestoreInterrupts(level);
	return 0;
found:
	if (room > len + sizeof(GCAX_MEMBLK)) {
		next = blk->next;
		rest = (GCAX_MEMBLK*)((u8*)blk + len);
		rest++;
		rest->magic = GCAX_BLK_FREE;
		rest->pad = 0;
		rest->next = next;
		blk->next = rest;
	}
	blk->magic = GCAX_BLK_BUSY;
	if (gcax_work->debug & 1) {
	}
	OSRestoreInterrupts(level);
	return blk + 1;
}

/* Finds the block before blk */
// 0x80230914
GCAX_MEMBLK* gcaxSndHeapPrev(GCAX_MEMBLK* blk)
{
	GCAX_MEMBLK* cur;
	GCAX_MEMBLK* prev;
	GCAX_MEMBLK* last;
	GCAX_MEMBLK* next;

	cur = gcax_work->heap_top;
	last = prev = 0;
	(void)last;
	while (cur->next != 0) {
		if (cur == blk) {
			return prev;
		}
		prev = cur;
		next = cur = cur->next;
		(void)next;
	}
	return 0;
}

// 0x80230980
void gcaxSndFree(void* p)
{
	GCAX_MEMBLK* blk;
	GCAX_MEMBLK* prev;
	BOOL level;

	level = OSDisableInterrupts();
	blk = (GCAX_MEMBLK*)p - 1;
	blk->magic = GCAX_BLK_FREE;
	if (blk->next != 0) {
		prev = blk->next;
		if (prev->magic == GCAX_BLK_FREE) {
			blk->next = prev->next;
		}
	}
	prev = gcaxSndHeapPrev(blk);
	if (prev != 0 && prev->magic == GCAX_BLK_FREE) {
		prev->next = blk->next;
	}
	OSRestoreInterrupts(level);
}

/* Total free bytes in the heap */
// 0x80230A24
u32 gcaxSndHeapRest(void)
{
	GCAX_MEMBLK* blk;
	BOOL level;
	u32 rest;

	rest = 0;
	level = OSDisableInterrupts();
	if (gcax_work != 0 && gcax_work->heap_top != 0) {
		for (blk = gcax_work->heap_top; blk->next != 0; blk = blk->next) {
			if (blk->magic == GCAX_BLK_FREE) {
				rest += (u32)blk->next - (u32)blk - sizeof(GCAX_MEMBLK);
			}
		}
	}
	OSRestoreInterrupts(level);
	return rest;
}

// 0x80230AC4
void gcaxArqCallback(u32 req)
{
	int i;

	for (i = 0; i < GCAX_ARQ_MAX; i++) {
		if ((u32)&gcax_work->arq[i] == req) {
			goto found;
		}
	}
	OSReport("AX SDLIB: Panic: arq not found at %s:%s\n", "gcaxSndMemory.c", 174);
	for (;;) {
	}
found:
	gcax_work->arq_busy[i] = 0;
	if (gcax_work->arq_cb[i] != 0) {
		gcax_work->arq_cb[i](gcax_work->arq_arg[i]);
	}
}

/* Posts a main RAM to ARAM transfer and returns its slot */
// 0x80230B8C
int gcaxArqPost(u32 src, u32 dst, u32 len, GCAX_CALLBACK cb, void* arg)
{
	int i;

	for (i = 0; i < GCAX_ARQ_MAX; i++) {
		if (gcax_work->arq_busy[i] == 0) {
			goto found;
		}
	}
	OSReport("AX SDLIB: Panic: no more at %s:%s\n", "gcaxSndMemory.c", 194);
	for (;;) {
	}
found:
	gcax_work->arq_busy[i] = 1;
	gcax_work->arq_cb[i] = cb;
	gcax_work->arq_arg[i] = arg;
	ARQPostRequest(&gcax_work->arq[i], gcax_work->arq_owner, 0, 1, src, dst, len, gcaxArqCallback);
	return i;
}

// 0x80230C80
void gcaxArqPostWait(u32 src, u32 dst, u32 len, GCAX_CALLBACK cb, void* arg)
{
	int i;
	u8* busy;

	i = gcaxArqPost(src, dst, len, cb, arg);
	busy = &gcax_work->arq_busy[i];
	while (*busy != 0) {
	}
}

// 0x80230CF4
int gcaxArqPostEx(u32 src, u32 dst, u32 len, GCAX_CALLBACK cb, void* arg, int nowait)
{
	int i;
	u8* busy;

	i = gcaxArqPost(src, dst, len, cb, arg);
	busy = &gcax_work->arq_busy[i];
	if (nowait == 0) {
		while (*busy != 0) {
		}
	}
	return i;
}
