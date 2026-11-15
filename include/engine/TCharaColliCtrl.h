#ifndef _TCHARACOLLICTRL_H
#define _TCHARACOLLICTRL_H

#include "stage/Stage.h"

/**
 * @size 0x20
 */
class TCharaColliCtrl : public TObject {
public:
	TCharaColliCtrl(TObject* parent);
	virtual ~TCharaColliCtrl();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
};

extern "C" {
void charaColliClear();
}

extern "C" {
extern const char* gTCharaColliCtrlName;
}

#endif
