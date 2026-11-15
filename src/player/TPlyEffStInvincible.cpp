#include "player/TPlyEffStInvincible.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"

const char* gTPlyEffStInvincibleName = "TPlyEffStInvincible";
struct Fx;

typedef char TPlyEffStInvincible_size_check[(sizeof(TPlyEffStInvincible) == 0x5C) ? 1 : -1];

// TChrPlayer::vfn_A8
static inline void copyJointMtx(TChrPlayer* player, int joint, f32* dst)
{
	player->vfn_A8(joint, dst);
}

// 0x801BBCE8
extern "C" TPlyEffStInvincible* createTPlyEffStInvincible(TObject* parent)
{
	return new TPlyEffStInvincible(parent);
}

// 0x801BBD34
TPlyEffStInvincible::TPlyEffStInvincible(TObject* parent)
    : TPlyEffSt(parent, 3)
{
	mEffect = 0;
	_00     = (u32)gTPlyEffStInvincibleName;
	mSize   = 0x5C;
}

// 0x801BBD9C
TPlyEffStInvincible::~TPlyEffStInvincible()
{
	if (mEffect != 0) {
		fxStop((Fx*)mEffect);
	}
}

// 0x801BBE20
void TPlyEffStInvincible::vfn_20()
{
	copyJointMtx(mOwner, 0x1B, mMtx);
	mEffect = fxSpawnFollow(1, 0, 0x55, mMtx);
}

// 0x801BBE7C
void TPlyEffStInvincible::vfn_24()
{
	if (mEffect != 0) {
		fxStop(mEffect);
		mEffect = 0;
	}
}

// 0x801BBEBC
void TPlyEffStInvincible::draw()
{
	f32 joint[12];
	Vec3f pos;

	copyJointMtx(mOwner, 0x1B, joint);
	mtxGetPos(joint, &pos);
	mtxSetTransV(mMtx, &pos);
}
