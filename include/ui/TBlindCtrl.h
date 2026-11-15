#ifndef _UI_TBLINDCTRL_H_
#define _UI_TBLINDCTRL_H_
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct TBlindCtrl;

#include "core/types.h"

extern const char* gTBlindCtrlName; // "TBlindCtrl"

extern "C" {
void blindCreate();
void blindOff(int);
void blindOn(int);
}

extern "C" {
int blindIsOn(int);
}

/**
 * @size 0x30
 */
class TBlindCtrl : public TObject {
public:
	inline TBlindCtrl(TObject* parent)
	    : TObject(parent)
	{
		_00      = (u32)gTBlindCtrlName;
		mSize    = 0x30;
		mOn[3]   = 0;
		mOn[2]   = 0;
		mOn[1]   = 0;
		mOn[0]   = 0;
	}
	virtual ~TBlindCtrl();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	int mOn[4]; // _20
};

extern "C" {
extern TBlindCtrl* sBlindCtrlObj;
}

#endif
