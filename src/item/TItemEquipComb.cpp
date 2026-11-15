#include "item/TItemEquipComb.h"
#include "player/Player.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"

const char* gTItemEquipCombName = "TItemEquipComb";

typedef char TItemEquipComb_size_check[(sizeof(TItemEquipComb) == 0x78) ? 1 : -1];

// 0x801AF5F0
TItemEquipComb::TItemEquipComb(TObject* parent, const ItemSpec* spec)
    : TItemEquipment(parent, spec)
{
	_00   = (u32)gTItemEquipCombName;
	mSize = 0x78;
}

// 0x801AF644
void TItemEquipComb::vfn_7C()
{
	void* data = mHolder.mNode.mList != 0 ? mHolder.mEntry->mData.mArc : 0;
	ItemModelRec* rec;
	TChrPlayer* owner;

	if (data == 0) {
		return;
	}
	rec   = *(ItemModelRec**)((u8*)data + 0x4C);
	owner = (TChrPlayer*)getOwner();
	owner->vfn_A4(*(void**)rec->root, rec->arg);
	mNodes.init(rec);
}

// 0x801AF6C4
void TItemEquipComb::vfn_80()
{
	TChrPlayer* owner = (TChrPlayer*)getOwner();

	owner->vfn_A4(0, 0);
}

// 0x801AF6FC
void TItemEquipComb::vfn_84()
{
	TChrPlayer* owner = (TChrPlayer*)getOwner();
	f32 mtx[12];
	int i;

	for (i = 0; i < 0x15; i++) {
		if (mNodes.has(i) != 0) {
			owner->vfn_AC(i, mtx);
			mNodes.set(i, mtx);
		}
	}
	mtxIdentity(mtx);
	mNodes.calc(mtx);
}

// 0x801AF790
TItemEquipComb::~TItemEquipComb()
{
}
