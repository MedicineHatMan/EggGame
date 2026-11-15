#ifndef _TPLYDEMOLASTCTRL_H
#define _TPLYDEMOLASTCTRL_H

#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

/**
 * Chooses the lose demo or the super demo. draw() calls mStep.
 *
 * @size 0x30
 */
class TPlyDemoLastCtrl : public TObject {
public:
	TPlyDemoLastCtrl(TObject* parent);
	virtual ~TPlyDemoLastCtrl();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	// The names of the steps are ours
	void stepLose();
	void stepAfterLose();
	void stepSuper();
	void stepAfterSuper();
	void stepDone();

	int mTimer;                        // _20
	void (TPlyDemoLastCtrl::*mStep)(); // _24
};

extern "C" {
extern u32 gTPlyDemoLastCtrlName;
}

#endif
