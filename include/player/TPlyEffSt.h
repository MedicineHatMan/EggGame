#ifndef _TPLYEFFST_H
#define _TPLYEFFST_H

#include "player/Player.h"

extern "C" void objHeapFree(void*, void*);

/**
 * Base of the player status effects. The name is ours. mKind is stored by the constructor.
 * mOwner is not. Vtable 0x802683F8.
 *
 * @size 0x28
 */
class TPlyEffSt : public TObject {
public:
	TPlyEffSt(TObject* parent, int kind)
	    : TObject(parent)
	    , mKind(kind)
	{
	}
	virtual ~TPlyEffSt() { }

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	// Empty Its bytes are in Player.cpp
	// 0x8003EE1C
	virtual void vfn_20(); // _20
	virtual void vfn_24(); // _24

	int mKind;          // _20 ours
	TChrPlayer* mOwner; // _24 ours
};

#endif
