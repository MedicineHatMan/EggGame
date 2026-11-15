#ifndef _OBJECT_RED_REDHEADLISTIMPL_H_
#define _OBJECT_RED_REDHEADLISTIMPL_H_

#include "object/red/TObjRedDinoHead.h"

static void detachRedHeadLinks(RedHeadList* list);

// 0x800C6668
RedHeadList::RedHeadList()
{
	mTail  = 0;
	mHead  = 0;
	mCount = 0;
}

// 0x800C667C
RedHeadList::~RedHeadList()
{
	detachRedHeadLinks(this);
}

// 0x800C66C4
static void detachRedHeadLinks(RedHeadList* list)
{
	RedHeadLink* link = list->mTail;
	while (link != 0) {
		RedHeadLink* next = link->mNext;
		link->mList       = 0;
		link->mNext       = 0;
		link->mPrev       = 0;
		link              = next;
	}
}

// 0x800C66F0
void initRedDinoHeadSet()
{
}

// 0x800C66F4
void termRedDinoHeadSet()
{
}

#endif
