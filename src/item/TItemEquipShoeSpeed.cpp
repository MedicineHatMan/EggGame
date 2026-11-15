#include "item/TItemEquipShoeSpeed.h"
#include "player/Player.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"

typedef char TItemEquipShoeSpeed_size_check[(sizeof(TItemEquipShoeSpeed) == 0x60) ? 1 : -1];

// 0x801D1464
TItemEquipShoeSpeed::TItemEquipShoeSpeed(TObject* parent, const ItemSpec* spec)
    : TItemEquipment(parent, spec)
    , mFx(0)
{
	_00   = gTItemEquipShoeSpeedName;
	mSize = 0x60;
}

// 0x801D14B8
void TItemEquipShoeSpeed::vfn_7C()
{
}

// 0x801D14BC
void TItemEquipShoeSpeed::vfn_80()
{
	fxStop(mFx);
}

// 0x801D14E0
void TItemEquipShoeSpeed::vfn_84()
{
	TChrPlayer* owner = (TChrPlayer*)getOwner();
	int on            = owner->vfn_8C();
	f32 mtx[12];
	Vec3f pos;

	if (on == 0) {
		if (mFx != 0) {
			fxStop(mFx);
			mFx = 0;
		}
	} else if (mFx == 0) {
		mFx = fxSpawn(1, 0, 0x57);
	}
	if (mFx != 0) {
		owner->vfn_A8(0x1B, mtx);
		mtxGetPos(mtx, &pos);
		mtxSetTransV(mtx, &pos);
		mtxRotateY(mFx->mMatrix, mtx, owner->getRotY());
	}
}

// 0x801D15CC
TItemEquipShoeSpeed::~TItemEquipShoeSpeed()
{
}
