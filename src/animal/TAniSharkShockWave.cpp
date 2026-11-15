#include "animal/AniMount.h"
#include "animal/AniAttacks.h"
#include "engine/Colli.h"
#include "animal/TAniSharkShockWave.h"
struct CharaColli;

const char* gTAniSharkShockWaveName = "TAniSharkShockWave";

// 0x801CD4C0
TAniSharkShockWave::TAniSharkShockWave(TObject* parent, u16 ownerId, const Vec3f* pos, f32 radius, f32 finalRadius)
    : TObject(parent)
    , mOwnerId(ownerId)
    , mLife(0x3C)
    , mPos(*pos)
    , mRadius(radius)
    , mFinalRadius(finalRadius)
{
	_00    = (u32)gTAniSharkShockWaveName;
	mSize  = 0x40;
	mColli = (CharaColli*)colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, ownerId);
		colliSetMasks(mColli, 0x20, 0, 0xFF, 0x10);
		ColliShape* shape = (ColliShape*)colliGetShape((CharaColli*)mColli, 0);
		colliShapeSetCylinder(shape);
		colliShapeSetCylRadius(shape, radius);
		colliShapeSetCylHeight(shape, 10.0f);
		shape->mFlags |= 0x18;
		shape->mTag = 1;
		Vec3f at;
		at.set(pos->x, 10.0f + pos->y, pos->z);
		colliShapeSetPos(shape, &at);
	}
}

// 0x801CD610
TAniSharkShockWave::~TAniSharkShockWave()
{
	colliFree(mColli);
}

// 0x801CD678
void TAniSharkShockWave::draw()
{
	if (mLife != 0) {
		if (--mLife == 0) {
			mFlags |= 1;
		}
	}
	if (mColli != 0) {
		f32 rate = 1.0f / 60.0f;
		mRadius  = mRadius * (1.0f - rate) + mFinalRadius * rate;
		colliShapeSetCylRadius(colliGetShape(mColli, 0), mRadius);
	}
}
