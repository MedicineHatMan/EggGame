#include "player/TPlyEffStSleep.h"
#include "egg/EggCreature.h"
#include "engine/TParticle.h"

struct SleepPart {
	u8 _00[0x58];
	f32* mFollow; // _58
};

typedef char TPlyEffSt_size_check[(sizeof(TPlyEffSt) == 0x28) ? 1 : -1];
typedef char TPlyEffStSleep_size_check[(sizeof(TPlyEffStSleep) == 0x58) ? 1 : -1];

// TChrPlayer::vfn_A8
static inline void copyJointMtx(TChrPlayer* player, int joint, f32* dst)
{
	player->vfn_A8(joint, dst);
}

// 0x801BBB14
TPlyEffStSleep::TPlyEffStSleep(TObject* parent)
    : TPlyEffSt(parent, 5)
{
	_00   = gTPlyEffStSleepName;
	mSize = 0x58;
}

// 0x801BBBD0
void TPlyEffStSleep::vfn_20()
{
	SleepPart* part;

	copyJointMtx(mOwner, 0x1B, mMtx);
	part = (SleepPart*)particleGenById(this, &gVecZero, 0, 0x51);
	if (part != 0) {
		part->mFollow = mMtx;
	}
}

// 0x801BBC3C
void TPlyEffStSleep::draw()
{
	copyJointMtx(mOwner, 0x1B, mMtx);
}

// 0x801BBC78
TPlyEffStSleep::~TPlyEffStSleep()
{
}

// 0x801BBCE4
void TPlyEffSt::vfn_24()
{
}
