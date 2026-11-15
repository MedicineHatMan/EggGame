#include "player/PlyState.h"
#include "core/Mem.h"
#include "player/PlyPool.h"

// 0x80078B24
TActionState::TActionState()
{
	mName = gTActionStateName;
	mSize = 0x14;
}

// 0x80078B44
TActionState::~TActionState()
{
}

// 0x80078B8C
extern "C" void* allocPlyState(u32 size, void* pool)
{
	return plyPoolTake((PlyPool*)pool, size);
}

// 0x80078BB8
extern "C" void* initPlyPool(void* mem, int blockSize)
{
	u8* m                       = (u8*)memAlloc(blockSize * 2);
	PlyPoolBlock* blocks        = (PlyPoolBlock*)m;
	blocks->mNext               = (PlyPoolBlock*)((u8*)blocks + blockSize);
	blocks->mNext->mNext        = 0;
	((PlyPool*)mem)->mMem       = m;
	((PlyPool*)mem)->mFree      = (PlyPoolBlock*)((PlyPool*)mem)->mMem;
	((PlyPool*)mem)->mBlockSize = blockSize;
	return mem;
}

// 0x80078C18
extern "C" void* freePlyPool(void* mem, int flag)
{
	if (mem != 0) {
		memFree(((PlyPool*)mem)->mMem);
		if ((s16)flag > 0) {
			operator delete(mem);
		}
	}
	return mem;
}

// 0x80078C64
// The size is not checked
extern "C" void* plyPoolTake(PlyPool* pool, u32)
{
	PlyPoolBlock* block = pool->mFree;
	if (block != 0) {
		pool->mFree  = block->mNext;
		block->mNext = 0;
	}
	return block;
}

// 0x80078C88
extern "C" void releasePlyState(void* mem, void* state)
{
	PlyPool* pool = (PlyPool*)mem;
	if (state == 0) {
		return;
	}
	((PlyPoolBlock*)state)->mNext = pool->mFree;
	pool->mFree                   = (PlyPoolBlock*)state;
}
