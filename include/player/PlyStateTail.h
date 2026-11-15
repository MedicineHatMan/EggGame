#ifndef _PLAYER_PLYSTATETAIL_H_
#define _PLAYER_PLYSTATETAIL_H_

#include "player/PlyState.h"

inline TPlyStateVictory::~TPlyStateVictory()
{
}

inline TPlyStateTransform::~TPlyStateTransform()
{
}

// 0x800BC6D8
// for FlagByte The animals use it too
template <class T>
int flagByteTest(T* flags, int bit)
{
	return flags->mBits & (1 << bit);
}

#endif
