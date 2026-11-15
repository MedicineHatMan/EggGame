#ifndef _PLAYER_PLYPOOL_H_
#define _PLAYER_PLYPOOL_H_
struct PlyPoolBlock;
struct PlyPool;

#include "core/types.h"

extern "C" {
void* allocPlyState(u32, void*);
void* initPlyPool(void*, int);
void releasePlyState(void*, void*);
}

extern "C" {
void* freePlyPool(void*, int);
void* plyPoolTake(PlyPool*, u32);
}

/** A free-list block */
struct PlyPoolBlock {
	PlyPoolBlock* mNext; // _00
};

/**
 * Two blocks of mBlockSize. The name is ours.
 * @size 0xC
 */
struct PlyPool {
	int mBlockSize;      // _00
	u8* mMem;            // _04
	PlyPoolBlock* mFree; // _08
};

extern "C" {
extern const char* gTActionStateName;
}

#endif
