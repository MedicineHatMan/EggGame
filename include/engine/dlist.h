#ifndef _DLIST_H
#define _DLIST_H

#include "core/types.h"

extern "C" void OSPanic(const char* file, int line, const char* msg, ...);

template <typename T>
class dlist {
public:
	/** @size 0xC */
	struct node {
		// No constructor but an assignment operator new node[] then keeps its array cookie and passes __construct_new_array
		// no constructor as the DOL does
		node& operator=(const node& o)
		{
			mData = o.mData;
			return *this;
		}

		node* mPrev; // _00 the head's points at the last node
		node* mNext; // _04 NULL at the end
		T mData;     // _08
	};

	struct iterator {
		iterator() { }
		iterator(node* n) { mNode = n; }

		T& operator*() const { return mNode->mData; }
		void operator++() { mNode = mNode->mNext; }
		bool valid() const { return mNode != NULL; }

		node* mNode;
	};

	dlist(int count);
	~dlist();

	iterator begin() { return iterator(mHead); }
	iterator end() { return iterator(NULL); }

	void push_back(const T& value);
	iterator insert(iterator pos, const T& value);
	void erase(iterator it);

	void makePool(int count);
	node* alloc();
	void unlink(node* n);

	node* mPool; // _00
	node* mFree; // _04
	node* mHead; // _08
	int mCount;  // _0C
	int mSize;   // _10
};

// The out-of-line parts "dlist.cp" in the panic message
#ifdef DLIST_DEFINE

template <typename T>
dlist<T>::dlist(int count)
{
	makePool(count);
	mFree  = mPool;
	mHead  = NULL;
	mCount = 0;
}

template <typename T>
dlist<T>::~dlist()
{
	if (mPool != NULL) {
		delete[] mPool;
	}
}

template <typename T>
void dlist<T>::erase(iterator it)
{
	unlink(it.mNode);
}

template <typename T>
void dlist<T>::push_back(const T& value)
{
	insert(end(), value);
}

template <typename T>
void dlist<T>::makePool(int count)
{
	mPool = new node[count];
	for (int i = 0; i < count - 1; i++) {
		mPool[i].mNext = &mPool[i + 1];
	}
	mPool[count - 1].mNext = NULL;
	mSize                  = count;
}

template <typename T>
typename dlist<T>::node* dlist<T>::alloc()
{
	node* n = mFree;
	if (mFree != NULL) {
		mFree = mFree->mNext;
		mCount++;
	} else {
		OSPanic("dlist.cp", 225, "dlist alloc error");
	}
	return n;
}

template <typename T>
void dlist<T>::unlink(node* n)
{
	if (n->mPrev == n) {
		mHead = NULL;
	} else if (mHead == n) {
		mHead           = n->mNext;
		n->mPrev->mNext = NULL;
		if (n->mNext != NULL) {
			n->mNext->mPrev = n->mPrev;
		}
	} else {
		n->mPrev->mNext = n->mNext;
		if (n->mNext != NULL) {
			n->mNext->mPrev = n->mPrev;
		} else {
			mHead->mPrev = n->mPrev;
		}
	}
	n->mNext = mFree;
	mFree    = n;
	mCount--;
}

template <typename T>
typename dlist<T>::iterator dlist<T>::insert(iterator pos, const T& value)
{
	node* at = pos.mNode;
	node* n  = alloc();
	if (at == NULL) {
		if (mHead == NULL) {
			n->mPrev = n;
			n->mNext = NULL;
			mHead    = n;
		} else {
			n->mPrev            = mHead->mPrev;
			n->mNext            = NULL;
			mHead->mPrev->mNext = n;
			mHead->mPrev        = n;
		}
	} else {
		if (mHead == at) {
			mHead = n;
		} else {
			at->mPrev->mNext = n;
		}
		n->mPrev  = at->mPrev;
		n->mNext  = at;
		at->mPrev = n;
	}
	n->mData = value;
	return iterator(n);
}

#endif

#endif
