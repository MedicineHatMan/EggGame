#ifndef _TPLYEYELID_H
#define _TPLYEYELID_H

#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The blink on the player's face. mOpen goes from 1 to 0 and back. State 3 holds it shut.
 *
 * @size 0x2C
 */
class TPlyEyelid : public TObject {
public:
	TPlyEyelid(TObject* parent);

	// 0x80040AA4
	virtual ~TPlyEyelid() { }

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	// Holds the lid shut
	void close();
	// Back to open and the blink timer starts again
	void reset();

	int mState; // _20 0 waiting 1 closing 2 opening 3 held shut
	int mTimer; // _24
	f32 mOpen;  // _28 1 open and 0 shut
};

extern "C" {
extern const char* gTPlyEyelidName;
}

#endif
