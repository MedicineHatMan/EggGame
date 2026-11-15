#ifndef _OBJECT_YELLOW_STATUELISTIMPL_H_
#define _OBJECT_YELLOW_STATUELISTIMPL_H_

#include "object/yellow/TObjYellowStatue.h"

StatueList::StatueList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

StatueList::~StatueList() { clear(); }

void StatueList::clear()
{
	StatueNode* node = mHead;
	while (node != 0) {
		StatueNode* next = node->mNext;
		node->mList = 0;
		node->mNext = 0;
		node->mPrev = 0;
		node = next;
	}
}

#endif
