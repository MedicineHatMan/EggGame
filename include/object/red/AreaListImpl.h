#ifndef _OBJECT_RED_AREALISTIMPL_H_
#define _OBJECT_RED_AREALISTIMPL_H_

#include "object/red/TObjRedDHeadInvokeArea.h"

static void detachAreaLinks(AreaList* list);

// 0x800C7070
AreaList::AreaList()
{
	mTail  = 0;
	mHead  = 0;
	mCount = 0;
}

// 0x800C7084
AreaList::~AreaList()
{
	detachAreaLinks(this);
}

// 0x800C70CC
static void detachAreaLinks(AreaList* list)
{
	RedAreaLink* link = list->mTail;
	while (link != 0) {
		RedAreaLink* next = link->mNext;
		link->mList       = 0;
		link->mNext       = 0;
		link->mPrev       = 0;
		link              = next;
	}
}

#endif
