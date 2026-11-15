#include "item/TItemImm.h"
#include "item/TItemManage.h"

typedef char TItemImm_size_check[(sizeof(TItemImm) == 0x40) ? 1 : -1];

const char* gTItemImmName = "TItemImm";

// 0x801BDECC
TItemImm::TItemImm(TObject* parent, const ItemSpec* spec)
    : TItem(parent)
{
	struct SpecWords { u32 a, b; };
	*(SpecWords*)&mSpec = *(const SpecWords*)spec;
	_00   = (u32)gTItemImmName;
	mSize = 0x40;
}

// 0x801BDF2C
TItemImm::~TItemImm() { }

// 0x801BDF88
void TItemImm::vfn_6C()
{
	apply(&mSpec, getOwner());
}
