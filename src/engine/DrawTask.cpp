#include "engine/DrawTask.h"
#include "Ninja/ninja.h"

#define DRAWTASK_POOL 0x400

typedef char DrawTask_size_check[(sizeof(DrawTask) == 0x18) ? 1 : -1];

/*
 * --INFO--
 * Address:	80028ED0
 * Size:	000054
 */
// 0x80028ED0
extern "C" void moveDrawTask(DrawTask* task, u16 list)
{
	if (task->mList != &gDrawTaskLists[list]) {
		unlinkDrawTask(task);
		linkDrawTask(&gDrawTaskLists[list], task);
	}
}

/*
 * --INFO--
 * Address:	80028F24
 * Size:	0000E0
 */
// 0x80028F24
extern "C" void initDrawTasks()
{
	int i;
	gDrawTaskLists = new DrawTaskList[DRAWTASK_LISTS];
	for (i = 0; i < DRAWTASK_LISTS; i++) {
		gDrawTaskLists[i].mHead  = 0;
		gDrawTaskLists[i].mCount = 0;
	}
	gDrawTaskPool = new DrawTask[DRAWTASK_POOL];
	for (i = 0; i < DRAWTASK_POOL - 1; i++) {
		gDrawTaskPool[i].mNext = &gDrawTaskPool[i + 1];
	}
	gDrawTaskPool[DRAWTASK_POOL - 1].mNext = 0;
	gDrawTaskFree                          = gDrawTaskPool;
	setDrawTaskDepthRange(5000.0f, -5000.0f);
}

/*
 * --INFO--
 * Address:	80029004
 * Size:	000060
 */
// 0x80029004
extern "C" void termDrawTasks()
{
	if (gDrawTaskPool != 0) {
		delete[] gDrawTaskLists;
		delete[] gDrawTaskPool;
	}
	gDrawTaskLists = 0;
	gDrawTaskPool  = 0;
	gDrawTaskFree  = 0;
}

/*
 * --INFO--
 * Address:	80029064
 * Size:	000080
 *
 * Takes a task from the pool and adds it to a list. Gives 0 when the pool is empty.
 */
// 0x80029064
extern "C" DrawTask* addDrawTask(void* owner, void (*func)(void*), u16 list, u16 sortKey)
{
	DrawTask* task = gDrawTaskFree;
	if (task != 0) {
		gDrawTaskFree = task->mNext;
	} else {
		return 0;
	}
	linkDrawTask(&gDrawTaskLists[list], task);
	task->mOwner   = owner;
	task->mFunc    = func;
	task->mSortKey = sortKey;
	task->mFlags   = 0;
	return task;
}

/*
 * --INFO--
 * Address:	800290E4
 * Size:	000038
 */
extern "C" void removeDrawTask(DrawTask* task)
{
	unlinkDrawTask(task);
	task->mNext   = gDrawTaskFree;
	gDrawTaskFree = task;
}

/*
 * --INFO--
 * Address:	8002911C
 * Size:	000080
 */
// 0x8002911C
extern "C" void runDrawTasks(u16 list)
{
	for (DrawTask* task = gDrawTaskLists[list].mHead; task != 0; task = task->mNext) {
		if (task->mFlags & 1) {
			continue;
		}
		if (task->mOwner != 0) {
			if ((*(u16*)((u8*)task->mOwner + 4) & 0x10) == 0) {
				task->mFunc(task->mOwner);
			}
		} else {
			task->mFunc(task->mOwner);
		}
	}
}

/*
 * --INFO--
 * Address:	8002919C
 * Size:	000104
 *
 * A bucket sort on the top six bits of the key, keeping the order within each bucket.
 */
// 0x8002919C
extern "C" void sortDrawTasks(u16 list)
{
	DrawTaskList* target = &gDrawTaskLists[list];
	if (target->mCount < 2) {
		return;
	}
	syMemSet(sDrawTaskBuckets, 0, sizeof(sDrawTaskBuckets));

	DrawTask* next;
	DrawTask* task = target->mHead;
	while (task != 0) {
		next            = task->mNext;
		u32 key         = (task->mSortKey >> 10) & 0x3F;
		DrawTask* first = sDrawTaskBuckets[key];
		if (first != 0) {
			task->mPrev         = first->mPrev;
			first->mPrev->mNext = task;
			first->mPrev        = task;
		} else {
			task->mPrev           = task;
			sDrawTaskBuckets[key] = task;
		}
		task->mNext = 0;
		task        = next;
	}

	DrawTask* head = 0;
	for (int i = 0; i < DRAWTASK_LISTS; i++) {
		DrawTask* bucket = sDrawTaskBuckets[i];
		if (bucket != 0) {
			if (head != 0) {
				DrawTask* tail     = bucket->mPrev;
				head->mPrev->mNext = bucket;
				bucket->mPrev      = head->mPrev;
				head->mPrev        = tail;
			} else {
				head = bucket;
			}
		}
	}
	target->mHead = head;
}

/*
 * --INFO--
 * Address:	800292A0
 * Size:	000050
 */
// 0x800292A0
extern "C" void linkDrawTask(DrawTaskList* list, DrawTask* task)
{
	DrawTask* head = list->mHead;
	if (head != 0) {
		task->mPrev        = head->mPrev;
		task->mNext        = 0;
		head->mPrev->mNext = task;
		head->mPrev        = task;
	} else {
		task->mPrev = task;
		list->mHead = task;
		task->mNext = 0;
	}
	list->mCount++;
	task->mList = list;
}

/*
 * --INFO--
 * Address:	800292F0
 * Size:	0000A0
 */
// 0x800292F0
extern "C" void unlinkDrawTask(DrawTask* task)
{
	DrawTaskList* list = task->mList;
	if (list->mHead != 0) {
		if (task->mPrev == task) {
			list->mHead = 0;
		} else if (list->mHead == task) {
			list->mHead        = task->mNext;
			task->mPrev->mNext = 0;
			if (task->mNext != 0) {
				task->mNext->mPrev = task->mPrev;
			}
		} else {
			task->mPrev->mNext = task->mNext;
			if (task->mNext != 0) {
				task->mNext->mPrev = task->mPrev;
			} else {
				list->mHead->mPrev = task->mPrev;
			}
		}
	}
	list->mCount--;
	task->mList = 0;
}

/*
 * --INFO--
 * Address:	80029390
 * Size:	000068
 *
 * Turns a depth into a sort key: the far end of the range gives 0xFFFF and the near end gives 0.
 */
// 0x80029390
extern "C" u16 drawTaskDepthKey(f32 depth)
{
	int key = 65536.0f * (gDrawTaskDepthScale * (gDrawTaskDepthFar - depth));
	if (key < 0) {
		key = 0;
	} else if (key > 0xFFFF) {
		key = 0xFFFF;
	}
	return 0xFFFF - (u16)key;
}

/*
 * --INFO--
 * Address:	800293F8
 * Size:	000018
 */
extern "C" void setDrawTaskDepthRange(f32 from, f32 to)
{
	gDrawTaskDepthFar   = from;
	gDrawTaskDepthScale = 1.0f / (from - to);
}
