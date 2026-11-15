#include "core/BlockPool.h"
#include "core/Mem.h"

// 0x8012BFFC
BlockPool::BlockPool(u32 size, int count)
{
	u32 stride = ((size + 0x1F) & ~0x1F) + sizeof(Block);
	mBlocks    = (Block*)memAlloc(stride * count);
	u8* block  = (u8*)mBlocks;
	if (block != 0) {
		for (int i = count - 1; i > 0; i--) {
			((Block*)block)->mNext = (Block*)(block + stride);
			block += stride;
		}
		((Block*)block)->mNext = 0;
	}
	mFree  = mBlocks;
	mCount = count;
	mSize  = size;
}

// 0x8012C094
BlockPool::~BlockPool()
{
	memFree(mBlocks);
}

// 0x8012C0EC
void* BlockPool::alloc()
{
	Block* block = mFree;
	if (block == 0) {
		return 0;
	}
	mFree        = block->mNext;
	block->mNext = 0;
	mCount--;
	return block + 1;
}

// 0x8012C124
void BlockPool::release(void* mem)
{
	if (mem == 0) {
		return;
	}
	Block* block = (Block*)mem - 1;
	block->mNext = mFree;
	mFree        = block;
	mCount++;
}
