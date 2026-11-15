#ifndef _DRAWTASK_H
#define _DRAWTASK_H
struct TSetGenerator;

#include "core/types.h"

// The queue of things to draw this frame
// See "Drawing and collision" in docs/notes.md

struct DrawTaskList;
class TObject;

/**
 * @size 0x18
 */
struct DrawTask {
	DrawTask& operator=(const DrawTask& other);

	DrawTaskList* mList;  // _00
	DrawTask* mPrev;      // _04 the head's mPrev is the tail
	DrawTask* mNext;      // _08 also links the free list
	void* mOwner;         // _0C skipped while its flags have 0x10 set
	void (*mFunc)(void*); // _10
	u16 mFlags;           // _14 1 = skip
	u16 mSortKey;         // _16 sortDrawTasks uses the top six bits
};

/**
 * @size 0x8
 */
struct DrawTaskList {
	DrawTaskList& operator=(const DrawTaskList& other);

	DrawTask* mHead; // _00
	u16 mCount;      // _04
};

extern "C" {
DrawTask* addDrawTask(void* owner, void (*func)(void*), u16 list, u16 sortKey); // 0x80029064 0 when the pool is empty
void drawObjectTask(TObject* obj); // 0x800141E8 the usual task function calls obj->render()
}

extern "C" {
u16 drawTaskDepthKey(f32);
void initDrawTasks();
void linkDrawTask(DrawTaskList*, DrawTask*);
void setDrawTaskDepthRange(f32, f32);
void termDrawTasks();
void unlinkDrawTask(DrawTask*);
}

extern "C" {
void removeDrawTask(DrawTask*);
}

extern "C" {
void runDrawTasks(u16);
void sortDrawTasks(u16);
}

extern "C" {
void moveDrawTask(DrawTask*, u16);
}

extern "C" {
extern DrawTaskList* gDrawTaskLists;
extern DrawTask* gDrawTaskPool;
extern DrawTask* gDrawTaskFree;
extern f32 gDrawTaskDepthFar;
extern f32 gDrawTaskDepthScale;
extern TSetGenerator* gSetGenerator;
}

#define DRAWTASK_LISTS 0x40

extern "C" {
extern DrawTask* sDrawTaskBuckets[DRAWTASK_LISTS];
}

#endif
