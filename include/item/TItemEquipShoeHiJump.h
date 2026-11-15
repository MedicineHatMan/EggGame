#ifndef _TITEMEQUIPSHOEHIJUMP_H
#define _TITEMEQUIPSHOEHIJUMP_H
struct Fx;

#include "item/TItemEquipment.h"

/**
 * While the owner's slot 0x88 is non-zero, keeps particle 0x4A (bank 0) on the player's position.
 * vfn_80 frees that particle.
 *
 * @size 0x60
 */
class TItemEquipShoeHiJump : public TItemEquipment {
public:
	TItemEquipShoeHiJump(TObject* parent, const ItemSpec* spec);
	virtual ~TItemEquipShoeHiJump();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void vfn_7C(); // _7C empty
	virtual void vfn_80(); // _80
	virtual void vfn_84(); // _84

	Fx* mFx; // _5C particle Its matrix is at +0x2C
};

extern "C" {
extern const char* gTItemEquipShoeHiJumpName;
}

#endif
