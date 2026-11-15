#ifndef _TPLYEFFSTINVINCIBLE_H
#define _TPLYEFFSTINVINCIBLE_H
struct TObject;
struct Fx;

#include "player/TPlyEffSt.h"

/**
 * Invincible effect. Effect 0x55 is started on joint 0x1B's matrix. draw keeps mMtx at that joint's position.
 *
 * @size 0x5C
 */
class TPlyEffStInvincible : public TPlyEffSt {
public:
	TPlyEffStInvincible(TObject* parent);
	virtual ~TPlyEffStInvincible();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_20(); // _20
	virtual void vfn_24(); // _24

	Fx* mEffect;  // _28
	f32 mMtx[12]; // _2C
};

extern "C" {
TPlyEffStInvincible* createTPlyEffStInvincible(TObject*);
}

extern "C" {
extern u32 gTPlyEffStInvincibleName;
}

#endif
