#ifndef _TPLYEFFSTBLIND_H
#define _TPLYEFFSTBLIND_H
struct TObject;
struct Fx;

#include "player/TPlyEffSt.h"
#include "item/TItemEquipment.h"
#include "engine/DrawTask.h"

/**
 * Blind effect. Item 4 is shown on a draw task, and effect 99 plays at the owner's position, raised by 20.
 *
 * @size 0x108
 */
class TPlyEffStBlind : public TPlyEffSt {
public:
	TPlyEffStBlind(TObject* parent);
	virtual ~TPlyEffStBlind();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C
	virtual void vfn_20(); // _20
	virtual void vfn_24(); // _24

	Fx* mEffect;        // _28
	f32 mMtx[12];       // _2C
	f32 mMtxB[12];      // _5C
	DrawTask* mTask;    // _8C
	ItemHolder mHolder; // _90
	ItemGlow mGlow;     // _A8
};

extern "C" {
TPlyEffStBlind* createTPlyEffStBlind(TObject*);
}

extern "C" {
extern u32 gTPlyEffStBlindName;
}

#endif
