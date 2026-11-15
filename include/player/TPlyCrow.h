#ifndef _TPLYCROW_H
#define _TPLYCROW_H

#include "stage/Stage.h"
#include "engine/Light.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A placed light. The mode is the first word and the vtable is at +4.
 * Vtable 0x80255530. Its other slot, 0x8004341C, is outside this unit. The name is ours.
 *
 * @size 0x7C
 */
/**
 * The crow that flies off the player. It lives 0x32 frames, then sets the delete flag.
 *
 * @size 0xDC
 */
class TPlyCrow : public TObject {
public:
	TPlyCrow(TObject* parent, u16 id, const f32* mtx);
	virtual ~TPlyCrow();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	s8 mState;          // _20 0 until the light and the particle are started
	u8 _21;             // _21
	u16 mId;            // _22
	u16 mLife;          // _24 counts down from 0x32
	u8 _26[2];          // _26
	f32 mMtx[12];       // _28
	void* mColli;       // _58
	PointLight mLight; // _5C
	u16 mPulse;         // _D8
	u16 mLit;           // _DA counts down from 0x14 then the light is removed
};

extern "C" {
extern const char* gTPlyCrowName;
}

#endif
