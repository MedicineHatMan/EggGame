#ifndef _TITEMEQUIPCOMB_H
#define _TITEMEQUIPCOMB_H

#include "item/TItemEquipment.h"

/**
 * @size 0x78
 */
class TItemEquipComb : public TItemEquipment {
public:
	TItemEquipComb(TObject* parent, const ItemSpec* spec);
	virtual ~TItemEquipComb();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void vfn_7C(); // _7C
	virtual void vfn_80(); // _80
	virtual void vfn_84(); // _84

	ItemNodes mNodes; // _5C
};

extern "C" {
extern const char* gTItemEquipCombName;
}

#endif
