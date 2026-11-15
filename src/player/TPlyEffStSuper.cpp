#include "player/TPlyEffStSuper.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
struct Fx;

typedef char TPlyEffStSuper_size_check[(sizeof(TPlyEffStSuper) == 0x5C) ? 1 : -1];

// 0x801C1EC4
extern "C" TPlyEffStSuper* createTPlyEffStSuper(TObject* parent)
{
	return new TPlyEffStSuper(parent);
}

// 0x801C1F10
TPlyEffStSuper::TPlyEffStSuper(TObject* parent)
    : TPlyEffSt(parent, 0xD)
{
	mEffect = 0;
	_00     = gTPlyEffStSuperName;
	mSize   = 0x5C;
}

// 0x801C1F78
TPlyEffStSuper::~TPlyEffStSuper()
{
	if (mEffect != 0) {
		fxStop((Fx*)mEffect);
	}
}

// 0x801C1FFC
void TPlyEffStSuper::vfn_20()
{
	Vec3f pos;

	pos     = *mOwner->getPos();
	mtxSetTransV(mMtx, &pos);
	mEffect = fxSpawnFollow(1, 0, 0x8D, mMtx);
}

// 0x801C2074
void TPlyEffStSuper::vfn_24()
{
	if (mEffect != 0) {
		fxStop(mEffect);
		mEffect = 0;
	}
}

// 0x801C20B4
void TPlyEffStSuper::draw()
{
	Vec3f pos;

	pos = *mOwner->getPos();
	mtxSetTransV(mMtx, &pos);
}
