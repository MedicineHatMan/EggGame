#ifndef _BLOCKPOOL_H
#define _BLOCKPOOL_H

#include "core/types.h"

/**
 * @size 0x14
 */
class BlockPool {
public:
	/** The header ahead of each block */
	struct Block {
		Block* mNext;   // _00 the next free block
		u8 _04[0x1C];   // _04
	};

	BlockPool(u32 size, int count); // 0x8012BFFC
	virtual ~BlockPool();           // 0x8012C094

	void* alloc();           // 0x8012C0EC 0 when none are left
	void release(void* mem); // 0x8012C124

	Block* mBlocks; // _04
	Block* mFree;   // _08
	u32 mSize;      // _0C asked for blocks are rounded up to 0x20 plus the header
	int mCount;     // _10 free blocks
};

#endif
