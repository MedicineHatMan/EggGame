#ifndef _TPLYITEM_H
#define _TPLYITEM_H

#include "item/TItemEquipment.h"
#include "engine/DrawTask.h"

/**
 * @size 0xA0
 */
class TPlyItem : public TObject {
public:
	TPlyItem(TObject* parent, int kind, f32* mtx);
	virtual ~TPlyItem();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	DrawTask* mTask;    // _20
	ItemHolder mHolder; // _24
	ItemGlow mGlow;     // _3C
	f32* mMtx;          // _9C
};

extern "C" {
extern u32 gTPlyItemName;
}

#endif
