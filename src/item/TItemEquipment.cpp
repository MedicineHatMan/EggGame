#include "item/TItemEquipment.h"
#include "item/TItemManage.h"
#include "player/Player.h"

typedef char TItemEquipment_size_check[(sizeof(TItemEquipment) == 0x5C) ? 1 : -1];

const char* gTItemEquipmentName = "TItemEquipment";

// 0x801AF41C
TItemEquipment::TItemEquipment(TObject* parent, const ItemSpec* spec)
    : TItem(parent)
    , mOn(0)
    , mHolder()
{
	struct SpecWords { u32 a, b; };
	*(SpecWords*)&mSpec = *(const SpecWords*)spec;
	_00   = (u32)gTItemEquipmentName;
	mSize = 0x5C;
	itemHolderAttach(spec->mKind, &mHolder);
}

// 0x801AF498
TItemEquipment::~TItemEquipment()
{
	itemHolderRelease(&mHolder);
}

// 0x801AF500
void TItemEquipment::draw()
{
	vfn_84();
}

// 0x801AF52C
void TItemEquipment::vfn_70()
{
	mOn = 1;
	TChrPlayer* owner = (TChrPlayer*)getOwner();

	owner->vfn_90(&mSpec);
	vfn_7C();
}

// 0x801AF588
void TItemEquipment::vfn_74()
{
	mOn = 0;
	TChrPlayer* owner = (TChrPlayer*)getOwner();

	owner->vfn_94(&mSpec);
	vfn_80();
}

// 0x801AF5E4
void TItemEquipment::vfn_84() { }

// 0x801AF5E8
void TItemEquipment::vfn_7C() { }

// 0x801AF5EC
void TItemEquipment::vfn_80() { }
