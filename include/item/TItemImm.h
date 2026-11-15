#ifndef _TITEMIMM_H
#define _TITEMIMM_H

#include "item/TItemEquipment.h"

/**
 * @size 0x40
 */
class TItemImm : public TItem {
public:
	TItemImm(TObject* parent, const ItemSpec* spec);
	virtual ~TItemImm();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void vfn_6C(); // _6C

	ItemSpec mSpec; // _38
};

extern "C" {
extern const char* gTItemImmName;
}

#endif
