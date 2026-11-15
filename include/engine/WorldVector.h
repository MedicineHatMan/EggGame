#ifndef _WORLDVECTOR_H
#define _WORLDVECTOR_H

#include "core/types.h"

template <typename T>
struct WorldVecIter {
	WorldVecIter(T* p) { mPtr = p; }

	T* mPtr;
};

// A growable array The instances live in WorldMesh.cpp 0x800779E4 on
template <typename T>
struct WorldVector {
	inline WorldVecIter<T> begin() { return WorldVecIter<T>(mData); }
	inline WorldVecIter<T> end() { return WorldVecIter<T>(mData + mSize); }

	T* mData;  // _00
	int mSize; // _04
	int mCap;  // _08
};

#endif
