#include "egg/TEggShockWave.h"
#include "engine/CharaColli.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "animal/AniMount.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
struct ColliShape;
struct CharaColli;

extern "C" {

extern const ColliShape sEggShockWaveShape[] = {
	{ { 0.0f, 2.0f, 0.0f }, 0x0, 0, 0x0, 10.0f, 2.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};
}

typedef char TEggShockWave_size_check[(sizeof(TEggShockWave) == 0x54) ? 1 : -1];

const char* gTEggShockWaveName = "TEggShockWave";

// 0x80029410
TEggShockWave::TEggShockWave(f32 radius, TObject* parent, u16 source, void* tag)
    : TObjAction(parent), mSource(source)
{
	ColliShape* shape;

	mRadius = radius;
	mTag    = tag;
	_00     = (u32)gTEggShockWaveName;
	mSize   = 0x54;
	setObjId(this, -1);
	mTimer = 0;
	mPower = 0;
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 8, 0, 0xFF, 0x10);
	colliShapeSetCylinder(colliGetShape((CharaColli*)mShape, 0));
	colliGetShape((CharaColli*)mShape, 0)->mSizeX = mRadius;
	colliGetShape((CharaColli*)mShape, 0)->mSizeY = mRadius;
	shape         = colliGetShape((CharaColli*)mShape, 0);
	shape->mFlags |= 8;
	shape         = colliGetShape((CharaColli*)mShape, 0);
	shape->mFlags |= 0x10;
}

// 0x80029544
TEggShockWave::~TEggShockWave()
{
	releaseObjId(this);
	colliFree((CharaColli*)mShape);
}

// 0x800295B0
int TEggShockWave::vfn_48(TObjAction* other)
{
	if (mSource == other->mObjId || (u16)(s16)other->mObjId == mHolderId) {
		return 0;
	}
	return mPower;
}

// 0x800295E0
int TEggShockWave::onHit(const TObjAction* other, int, int* result)
{
	HitInfo* hit;

	if (mSource == other->mObjId || (u16)(s16)other->mObjId == mHolderId) {
		return 0;
	}
	hit = (HitInfo*)result;
	hit->mDamage = mPower;
	hit->mFlags.setBit(1);
	hit->mFlags.orWith(mHitFlags);
	hit->_14 = (u32)mTag;
	return 1;
}

// 0x80029660
void TEggShockWave::setPos(const Vec3f* pos)
{
	mPos = *pos;
	colliPlaceShapes(mShape, &mPos, (const ColliShape*)sEggShockWaveShape);
}

// 0x800296A8
void TEggShockWave::draw()
{
	f32 mtx[12];
	f32 scaled[12];
	f32 scale;

	if (mTimer == 0) {
		mtxSetTransV(mtx, &mPos);
		mtxRotateX(mtx, mtx, -0x4000);
		scale = 0.6f * (0.24f * mRadius);
		mtxScale(scale, scale, scale, scaled, mtx);
		fxSpawnAt(1, 0, 8, scaled);
	} else if (mTimer == 0x14) {
		mFlags |= 1;
	}
	mTimer += 1;
}

// 0x80029758
int TEggShockWave::getKind()
{
	return 3;
}

// 0x80029760
Vec3f* TEggShockWave::getPos()
{
	return &mPos;
}
