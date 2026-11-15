#ifndef _TITEM_H
#define _TITEM_H

#include "object/ObjAction.h"

struct ItemSpec;

/**
 * The base of the things a player carries and uses, like the sensor bomb. Constructor at 0x80186CF0, vtable 0x80262B08.
 * The code is in TItemManage.cpp. giveItem makes one from an item id and sets mKind and mOwnerId.
 *
 * @size 0x38
 */
class TItem : public TObjAction {
public:
	TItem(TObject* parent);
	virtual ~TItem();
	virtual int getKind() { return 3; } // _2C
	virtual void vfn_6C();                // _6C
	virtual void vfn_70();                // _70
	virtual void vfn_74();                // _74
	virtual void place(const Vec3f* pos); // _78

	TObjAction* getOwner() const;

	void apply(const ItemSpec* spec, TObjAction* owner);
	void playFx(int fx);

	// What an item with mode 3 does by spec->mParam
	void effectOthers(const ItemSpec* spec, TObjAction* owner, int status);
	void effectAll(const ItemSpec* spec, TObjAction* owner, int status);
	void effectEggMsg(const ItemSpec* spec, TObjAction* owner);
	void effectShatterEggs(const ItemSpec* spec, TObjAction* owner);
	void effectBreakEggs(const ItemSpec* spec, TObjAction* owner);
	void effectSteal(const ItemSpec* spec, TObjAction* owner);
	void effectStatus(const ItemSpec* spec, TObjAction* owner);
	void effectTime(const ItemSpec* spec, TObjAction* owner);
	void effectBonus(const ItemSpec* spec, TObjAction* owner);

	u16 mOwnerId; // _30
	u8 _32[2];    // _32
	int mKind;    // _34 the item id
};

#endif
