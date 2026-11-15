#ifndef _TITEMEQUIPMENT_H
#define _TITEMEQUIPMENT_H

#include "item/TItem.h"
#include "object/ObjCommon.h"
#include "item/ItemParts.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One row of the item table in TItemManage.cpp, which findItemSpec looks up by id. Field names are ours.
 * The low nibble of mFlags picks what TItem::apply does: 3 runs effect mParam, 4 sets key bit 0x400 + mParam,
 * 5 sends message 0x9A, anything else hands the spec to the owner. 0x10 makes an equipment item (mType picks which),
 * 0x20 a bomb, 0x40 a TItemImm, and none of them a TItemOnce.
 *
 * @size 0x8
 */
struct ItemSpec {
	u8 mKind;    // _00 the item id
	u8 mFlags;   // _01
	u16 mType;   // _02 for equipment 0 comb 1 wing 2 shoes
	u16 mParam;  // _04
	u16 mTime;   // _06 seconds for the status effects
} __attribute__((aligned(4)));

/**
 * An item the player puts on. vfn_70 turns it on and calls vfn_7C, vfn_74 turns it off and calls vfn_80,
 * and draw calls vfn_84. The two spec words are handed to the owner's slots 0x90 and 0x94.
 *
 * @size 0x5C
 */
class TItemEquipment : public TItem {
public:
	TItemEquipment(TObject* parent, const ItemSpec* spec);
	virtual ~TItemEquipment();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_70(); // _70
	virtual void vfn_74(); // _74
	virtual void vfn_7C(); // _7C equip hook Empty here
	virtual void vfn_80(); // _80 unequip hook Empty here
	virtual void vfn_84(); // _84 called from draw Empty here

	int mOn;            // _38 1 while equipped
	ItemHolder mHolder; // _3C
	ItemSpec mSpec;     // _54
};

extern "C" {
extern const char* gTItemEquipmentName;
}

#endif
