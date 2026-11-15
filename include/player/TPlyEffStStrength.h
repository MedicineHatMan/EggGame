#ifndef _TPLYEFFSTSTRENGTH_H
#define _TPLYEFFSTSTRENGTH_H
struct TObject;
struct Fx;

#include "player/TPlyEffSt.h"

/**
 * Strength effect. Effect 0x52 is placed at the owner's position, and draw follows that position.
 *
 * @size 0x5C
 */
class TPlyEffStStrength : public TPlyEffSt {
public:
	TPlyEffStStrength(TObject* parent);
	virtual ~TPlyEffStStrength();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_20(); // _20
	virtual void vfn_24(); // _24

	Fx* mEffect;  // _28
	f32 mMtx[12]; // _2C
};

extern "C" {
TPlyEffStStrength* createTPlyEffStStrength(TObject*);
}

extern "C" {
extern u32 gTPlyEffStStrengthName;
}

#endif
