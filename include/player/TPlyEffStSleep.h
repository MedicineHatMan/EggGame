#ifndef _TPLYEFFSTSLEEP_H
#define _TPLYEFFSTSLEEP_H

#include "player/TPlyEffSt.h"

/**
 * @size 0x58
 */
class TPlyEffStSleep : public TPlyEffSt {
public:
	TPlyEffStSleep(TObject* parent);
	virtual ~TPlyEffStSleep();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_20(); // _20

	f32 mMtx[12]; // _28
};

extern "C" {
extern u32 gTPlyEffStSleepName;
}

#endif
