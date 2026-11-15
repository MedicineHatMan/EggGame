#include "animal/AniAttacks.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "animal/TAniFlushFlowerBomb.h"
struct CharaColli;

const char* gTAniFlushFlowerBombName = "TAniFlushFlowerBomb";

// 0x801A2D60
TAniFlushFlowerBomb::TAniFlushFlowerBomb(TObject* owner, u16 ownerId, const Vec3f* pos, int fxBank, ColliShape* shape)
    : TObject(owner)
    , mPos(*pos)
{
	mFxBank = fxBank;
	_00     = (u32)gTAniFlushFlowerBombName;
	mSize   = 0xC0;
	mtxSetTransV(mFxMatrix, &mPos);
	mFx                           = fxSpawnFollow(1, mFxBank, 1, mFxMatrix);
	*(ColliShapeCopy*)&mShapeData = *(ColliShapeCopy*)shape;
	mColli                        = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)ownerId);
		colliSetMasks((CharaColli*)mColli, 0x20, 0xFF, 0xFF, 0x10);
		colliCopyShapes((CharaColli*)mColli, &mShapeData);
		colliPlaceShapes((CharaColli*)mColli, &mPos, &mShapeData);
		colliShapeSetBit8((CharaColli*)mColli, 0);
	}
	mVel.x  = 0.0f;
	mVel.y  = 0.0f;
	mVel.z  = 0.0f;
	mDrag   = -0.1f;
	_3C     = 15.0f;
	mGrowth = 5.0f;
	mTimer  = 0x3C;
	mState  = 0;
	mDone   = 0;
}

// 0x801A2EFC
TAniFlushFlowerBomb::~TAniFlushFlowerBomb()
{
	killFx();
	colliFree((CharaColli*)mColli);
}

// 0x801A2F68
void TAniFlushFlowerBomb::draw()
{
	if (mDone == 0) {
		switch (mState) {
		case 0:
			runFly();
			break;
		case 1:
			runGrow();
			break;
		}
	}
}

// 0x801A2FB8
void TAniFlushFlowerBomb::runFly()
{
	if (mTimer == 0) {
		if (mFx != 0) {
			fxRemove(mFx);
			mFx = 0;
		}
		fxSpawnAt(1, mFxBank, 6, mFxMatrix);
		mTimer = 0x3C;
		mState = 1;
		return;
	}
	mTimer--;
	Vec3f slow = mVel;
	if (njUnitVector(&slow) < gUnitVectorMin) {
		slow = gVecZero;
	}
	gjScaleVector(mDrag, &slow, &slow);
	njAddVector(&mVel, &slow);
	njAddVector(&mPos, &mVel);
	Vec3f at = mPos;
	at.y += 5.0f;
	mtxSetTransV(mFxMatrix, &at);
	if (mColli != 0) {
		colliPlaceShapes(mColli, &mPos, &mShapeData);
	}
}

// 0x801A30F0
void TAniFlushFlowerBomb::runGrow()
{
	if (mTimer == 0) {
		mDone = 1;
		return;
	}
	mTimer--;
	if (mColli != 0) {
		colliGetShapes(mColli)->mSizeX += mGrowth;
	}
	mGrowth *= 0.8f;
}

// 0x801A3168
void TAniFlushFlowerBomb::vfn_10()
{
}

// 0x801A316C
void TAniFlushFlowerBomb::render()
{
}

// 0x801A3170
void TAniFlushFlowerBomb::killFx()
{
	if (mFx != 0) {
		fxRemove(mFx);
		mFx = 0;
	}
}

// 0x801A31B0
int TAniFlushFlowerBomb::vfn_20()
{
	return 4;
}

// 0x801A31B8
const Vec3f* TAniFlushFlowerBomb::vfn_24()
{
	return &mPos;
}
