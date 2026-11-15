#ifndef _ITEMLIST_H
#define _ITEMLIST_H

#include "core/types.h"

class TArc;
struct ItemHolder;

/** A list node that carries its data. @size 8 + sizeof(T) */
template <typename T>
struct TListNode {
	TListNode* mNext; // _00
	TListNode* mPrev; // _04
	T mData;          // _08
};

/** A list of TListNodes. @size 0xC */
template <typename T>
struct TList {
	typedef TListNode<T> Node;

	TList()
	    : mHead(NULL)
	    , mTail(NULL)
	    , mCount(0)
	{
	}

	void append(Node* node);
	void remove(Node* node);

	Node* mHead; // _00
	Node* mTail; // _04
	int mCount;  // _08
};

/** A fixed set of TListNodes made up front, chained through mNext while free. @size 0xC */
template <typename T>
struct TNodePool {
	typedef TListNode<T> Node;

	TNodePool();
	~TNodePool();

	void create(int count);
	Node* alloc();
	void free(Node* node);

	Node* mNodes; // _00
	Node* mFree;  // _04
	int mCount;   // _08
};

/** A list of objects that each hold their own node, which points back at the list. @size 0xC */
template <typename T>
struct TRefList {
	/** @size 0x10 */
	struct Node {
		Node(T* obj)
		{
			mNext = NULL;
			mPrev = NULL;
			mList = NULL;
			mObj  = obj;
		}

		Node* mNext;     // _00
		Node* mPrev;     // _04
		T* mObj;         // _08
		TRefList* mList; // _0C NULL while in no list
	};

	TRefList();
	~TRefList();

	void append(Node* node);
	void remove(Node* node);
	void clear();
	void unlinkAll();

	Node* mTail; // _00
	Node* mHead; // _04
	int mCount;  // _08
};

/** A fixed array filled from the front. @size N * sizeof(T) + 4 */
template <typename T, int N>
struct TFixedArray {
	TFixedArray();

	void push_back(const T& value);
	T* begin() { return mItems; }
	T* end() { return &mItems[mCount]; }

	T mItems[N]; // _00
	int mCount;  // _(N * sizeof(T))
};

/**
 * One item archive the manager has loaded, or is loading, into a block of its own, with the holders that use it.
 * The name is ours.
 *
 * @size 0x1C
 */
struct ItemArc {
	~ItemArc() { }

	int mKind;                     // _00 the item id
	u32 mLoadId;                   // _04 from requestDiscLoad
	void* mBlock;                  // _08 from the manager's BlockPool
	TArc* mArc;                    // _0C set once the load is done
	TRefList<ItemHolder> mHolders; // _10
};

template <typename T>
void TList<T>::append(Node* node)
{
	if (mHead == NULL) {
		mTail = node;
		mHead = node;
	} else {
		node->mPrev  = mTail;
		mTail->mNext = node;
		mTail        = node;
	}
	mCount++;
}

template <typename T>
void TList<T>::remove(Node* node)
{
	if (node->mPrev != NULL) {
		node->mPrev->mNext = node->mNext;
	} else {
		mHead = node->mNext;
	}
	if (node->mNext != NULL) {
		node->mNext->mPrev = node->mPrev;
	} else {
		mTail = node->mPrev;
	}
	node->mNext = NULL;
	node->mPrev = NULL;
	mCount--;
}

template <typename T>
TNodePool<T>::TNodePool()
{
	mFree  = NULL;
	mNodes = NULL;
	mCount = 0;
}

template <typename T>
TNodePool<T>::~TNodePool()
{
	if (mNodes != NULL) {
		delete[] mNodes;
	}
}

template <typename T>
void TNodePool<T>::create(int count)
{
	mNodes = new Node[count];
	for (int i = 0; i < count - 1; i++) {
		mNodes[i].mNext = &mNodes[i + 1];
	}
	mNodes[count - 1].mNext = NULL;
	mFree                   = mNodes;
	mCount                  = count;
}

template <typename T>
typename TNodePool<T>::Node* TNodePool<T>::alloc()
{
	Node* node = mFree;
	if (node != NULL) {
		mFree       = node->mNext;
		node->mNext = NULL;
		node->mPrev = NULL;
	}
	return node;
}

template <typename T>
void TNodePool<T>::free(Node* node)
{
	if (node != NULL) {
		node->mNext = mFree;
		mFree       = node;
	}
}

template <typename T>
TRefList<T>::TRefList()
{
	mHead  = NULL;
	mTail  = NULL;
	mCount = 0;
}

template <typename T>
TRefList<T>::~TRefList()
{
	unlinkAll();
}

template <typename T>
void TRefList<T>::append(Node* node)
{
	if (mHead == NULL) {
		mTail = node;
		mHead = node;
	} else {
		node->mPrev  = mTail;
		mTail->mNext = node;
		mTail        = node;
	}
	node->mList = this;
	mCount++;
}

template <typename T>
void TRefList<T>::remove(Node* node)
{
	if (node->mPrev != NULL) {
		node->mPrev->mNext = node->mNext;
	} else {
		mHead = node->mNext;
	}
	if (node->mNext != NULL) {
		node->mNext->mPrev = node->mPrev;
	} else {
		mTail = node->mPrev;
	}
	node->mNext = NULL;
	node->mPrev = NULL;
	node->mList = NULL;
	mCount--;
}

template <typename T>
void TRefList<T>::clear()
{
	unlinkAll();
	mHead  = NULL;
	mTail  = NULL;
	mCount = 0;
}

template <typename T>
void TRefList<T>::unlinkAll()
{
	Node* node = mHead;
	while (node != NULL) {
		Node* next  = node->mNext;
		node->mList = NULL;
		node->mNext = NULL;
		node->mPrev = NULL;
		node        = next;
	}
}

template <typename T, int N>
TFixedArray<T, N>::TFixedArray()
{
	mCount = 0;
}

template <typename T, int N>
void TFixedArray<T, N>::push_back(const T& value)
{
	mItems[mCount] = value;
	mCount++;
}

#endif
