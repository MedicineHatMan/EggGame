#ifndef _TPLYDEMOLASTLOSE_H
#define _TPLYDEMOLASTLOSE_H
struct PlyDemoExport;
struct TArc;

#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The camera and the pose for the last demo's lose. draw() calls mStep.
 *
 * @size 0x34
 */
class TCamPath;
class TPlyDemoLastLose : public TObject {
public:
	TPlyDemoLastLose(TObject* parent);
	virtual ~TPlyDemoLastLose();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	// The names of the steps are ours
	void stepIntro();
	void stepHold();
	void stepEnd();

	TCamPath* mEvent;                  // _20
	int mTimer;                        // _24
	void (TPlyDemoLastLose::*mStep)(); // _28
};

extern "C" {
void plyDemoLastLoseBind(TArc*);
}

extern "C" {
extern const char* gTPlyDemoLastLoseName;
extern PlyDemoExport* gLastLoseExports;
}

#endif
