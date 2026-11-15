#ifndef _TAEOBJCOMING_H
#define _TAEOBJCOMING_H

#include "ui/TAeLayer.h"

/**
 * @size 0x3C
 */
class TAeObjComing : public TAeObject {
public:
	TAeObjComing(TObject* parent, void* data, void* assets);
	virtual ~TAeObjComing();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10

	TAeLayer* mLayer; // _34
	u32 mDone;        // _38 set when the clip finishes
};

extern "C" {
extern const char* gTAeObjComingName;
}

#endif
