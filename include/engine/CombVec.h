#ifndef _ENGINE_COMBVEC_H_
#define _ENGINE_COMBVEC_H_

#include "core/types.h"
#include "engine/WorldVector.h"

template <typename T>
struct CombVec {
	T* mData;   // _00
	int mCount; // _04
	int mCap;   // _08

	CombVec(int count);
	~CombVec();
	static void fill(WorldVecIter<T> first, WorldVecIter<T> last, const int& value);
	void reserve(int count);
};

template <typename T>
CombVec<T>::CombVec(int count)
{
	reserve(count);
	mCount = count;
}

template <typename T>
CombVec<T>::~CombVec()
{
	if (mData != 0) {
		delete[] mData;
	}
}

// SceneMeshes.cpp's fill of the seen bytes 0x8000EE70
template <typename T>
void CombVec<T>::fill(WorldVecIter<T> first, WorldVecIter<T> last, const int& value)
{
	T* it = first.mPtr;
	while (it != last.mPtr) {
		*it++ = value;
	}
}

template <typename T>
void CombVec<T>::reserve(int count)
{
	mData = new T[count];
	mCap  = count;
}

#endif
