#ifndef _TITEMEQUIPSHOESPEED_H
#define _TITEMEQUIPSHOESPEED_H
struct Fx;

#include "item/TItemEquipment.h"

/**
 * While the owner's slot 0x8C is non-zero, keeps particle 0x57 (bank 0) on node 0x1B,
 * turned by the owner's yaw. vfn_80 frees that particle.
 *
 * @size 0x60
 */
class TItemEquipShoeSpeed : public TItemEquipment {
public:
	TItemEquipShoeSpeed(TObject* parent, const ItemSpec* spec);
	virtual ~TItemEquipShoeSpeed();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void vfn_7C(); // _7C empty
	virtual void vfn_80(); // _80
	virtual void vfn_84(); // _84

	Fx* mFx; // _5C particle Its matrix is at +0x2C
};

extern "C" {
extern u32 gTItemEquipShoeSpeedName;
}

#endif
