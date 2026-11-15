#ifndef _OBJECT_YELLOW_SANDFALLLISTIMPL_H_
#define _OBJECT_YELLOW_SANDFALLLISTIMPL_H_

#include "object/yellow/TObjYellowSandfall.h"

// 0x8016BC00
SandfallList::SandfallList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

// 0x8016BC14
SandfallList::~SandfallList()
{
	clear();
}

// 0x8016BC5C
void SandfallList::clear()
{
	SandfallNode* node = mHead;
	while (node != 0) {
		SandfallNode* next = node->mNext;
		node->mList        = 0;
		node->mNext        = 0;
		node->mPrev        = 0;
		node               = next;
	}
}

#endif
