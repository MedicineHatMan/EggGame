#include "item/TItemEquipShoeHiJump.h"
#include "player/Player.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"

typedef char TItemEquipShoeHiJump_size_check[(sizeof(TItemEquipShoeHiJump) == 0x60) ? 1 : -1];

// 0x801D12C0
TItemEquipShoeHiJump::TItemEquipShoeHiJump(TObject* parent, const ItemSpec* spec)
    : TItemEquipment(parent, spec)
    , mFx(0)
{
	_00   = gTItemEquipShoeHiJumpName;
	mSize = 0x60;
}

// 0x801D1314
void TItemEquipShoeHiJump::vfn_7C() { }

// 0x801D1318
void TItemEquipShoeHiJump::vfn_80()
{
	fxStop(mFx);
}

// 0x801D133C
void TItemEquipShoeHiJump::vfn_84()
{
	TChrPlayer* owner = (TChrPlayer*)getOwner();
	int on            = owner->vfn_88();

	if (on == 0) {
		if (mFx != 0) {
			fxStop(mFx);
			mFx = 0;
		}
	} else if (mFx == 0) {
		mFx = fxSpawn(1, 0, 0x4A);
	}
	if (mFx != 0) {
		Vec3f pos;
		pos = *owner->getPos();
		mtxSetTransV(mFx->mMatrix, &pos);
	}
}

// 0x801D1408
TItemEquipShoeHiJump::~TItemEquipShoeHiJump() { }
