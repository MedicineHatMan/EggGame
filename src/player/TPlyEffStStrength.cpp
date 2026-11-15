#include "player/TPlyEffStStrength.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
struct Fx;

typedef char TPlyEffStStrength_size_check[(sizeof(TPlyEffStStrength) == 0x5C) ? 1 : -1];

// 0x801BBF18
extern "C" TPlyEffStStrength* createTPlyEffStStrength(TObject* parent)
{
	return new TPlyEffStStrength(parent);
}

// 0x801BBF64
TPlyEffStStrength::TPlyEffStStrength(TObject* parent)
    : TPlyEffSt(parent, 0xC)
{
	mEffect = 0;
	_00     = gTPlyEffStStrengthName;
	mSize   = 0x5C;
}

// 0x801BBFCC
TPlyEffStStrength::~TPlyEffStStrength()
{
	if (mEffect != 0) {
		fxStop((Fx*)mEffect);
	}
}

// 0x801BC050
void TPlyEffStStrength::vfn_20()
{
	Vec3f pos;

	pos = *mOwner->getPos();
	mtxSetTransV(mMtx, &pos);
	mEffect = fxSpawnFollow(1, 0, 0x52, mMtx);
}

// 0x801BC0C8
void TPlyEffStStrength::vfn_24()
{
	if (mEffect != 0) {
		fxStop(mEffect);
		mEffect = 0;
	}
}

// 0x801BC108
void TPlyEffStStrength::draw()
{
	Vec3f pos;

	pos = *mOwner->getPos();
	mtxSetTransV(mMtx, &pos);
}
