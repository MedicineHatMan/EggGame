#ifndef _TPLYEFFSTSUPER_H
#define _TPLYEFFSTSUPER_H
struct TObject;
struct Fx;

#include "player/TPlyEffSt.h"

/**
 * Super effect. Effect 0x8D is placed at the owner's position, and draw follows that position.
 *
 * @size 0x5C
 */
class TPlyEffStSuper : public TPlyEffSt {
public:
	TPlyEffStSuper(TObject* parent);
	virtual ~TPlyEffStSuper();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_20(); // _20
	virtual void vfn_24(); // _24

	Fx* mEffect;  // _28
	f32 mMtx[12]; // _2C
};

extern "C" {
TPlyEffStSuper* createTPlyEffStSuper(TObject*);
}

extern "C" {
extern const char* gTPlyEffStSuperName;
}

#endif
