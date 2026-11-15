#ifndef _TITEMEQUIPWING_H
#define _TITEMEQUIPWING_H

#include "item/TItemEquipment.h"
#include "engine/DrawTask.h"

/**
 * @size 0x10C
 */
class TItemEquipWing : public TItemEquipment {
public:
	TItemEquipWing(TObject* parent, const ItemSpec* spec);
	virtual ~TItemEquipWing();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C
	virtual void vfn_7C(); // _7C
	virtual void vfn_84(); // _84

	f32 mMtx[12];     // _5C
	DrawTask* mDraw;  // _8C
	ItemGlow mGlow;   // _90
	ItemNodes mNodes; // _F0
};

extern "C" {
extern const char* gTItemEquipWingName;
}

#endif
