#ifndef _TARRAY_H
#define _TARRAY_H

#include "core/types.h"

/**
 * @size 0xC
 */
template <typename T>
class TArray {
public:
	TArray();
	~TArray();

	void push(const T& value);
	void clear();
	void reserve(int capacity);
	void alloc(int capacity);
	void copy(T* dst, const T* src, int n);

	T* mData;      // _00
	int mCount;    // _04
	int mCapacity; // _08
};

template <typename T>
TArray<T>::TArray()
{
	mData     = 0;
	mCount    = 0;
	mCapacity = 0;
}

template <typename T>
TArray<T>::~TArray()
{
	if (mData != 0) {
		delete[] mData;
	}
}

template <typename T>
void TArray<T>::clear()
{
	mCount = 0;
}

// Grows to twice the count and two more
template <typename T>
void TArray<T>::push(const T& value)
{
	if (mCount + 1 > mCapacity) {
		reserve(mCount + mCount + 2);
	}
	mData[mCount] = value;
	mCount++;
}

template <typename T>
void TArray<T>::reserve(int capacity)
{
	if (capacity > mCapacity) {
		T* old = mData;
		alloc(capacity);
		if (old != 0) {
			copy(mData, old, mCount);
			delete[] old;
		}
	}
}

template <typename T>
void TArray<T>::alloc(int capacity)
{
	mData     = new T[capacity];
	mCapacity = capacity;
}

template <typename T>
void TArray<T>::copy(T* dst, const T* src, int n)
{
	for (int i = 0; i < n; i++) {
		dst[i] = src[i];
	}
}

#endif
