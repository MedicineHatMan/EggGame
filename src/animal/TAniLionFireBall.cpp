#include "core/VecMath.h"
#include "animal/AniMount.h"
#include "animal/AniAttacks.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "engine/GroundQuery.h"
#include "Ninja/ninja.h"
#include "animal/TAniLionFireBall.h"
struct NJS_QUATERNION;
struct Fx;
struct CharaColli;

const char* gTAniLionFireBallName = "TAniLionFireBall";

// 0x801CCE94
TAniLionFireBall::TAniLionFireBall(TObject* parent, u16 ownerId, int fxBank, const Vec3f* pos, const Vec3f* vel)
    : TObject(parent)
    , mOwnerId(ownerId)
    , mFxBank(fxBank)
    , mPos(*pos)
    , mVel(*vel)
{
	_00      = (u32)gTAniLionFireBallName;
	mSize    = 0x88;
	mColli   = (CharaColli*)colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)ownerId);
		colliSetMasks(mColli, 0x20, 0, 0xFF, 0x10);
		ColliShape* shape = (ColliShape*)colliGetShape((CharaColli*)mColli, 0);
		colliShapeSetSphere(shape);
		shape->mSizeX = 17.5f;
		shape->mFlags |= 0x18;
		shape->mTag = 1;
		colliShapeSetPos(shape, pos);
	}
	mLife = (int)(50.0f / vecLength(vel) / (1.0f / 60.0f));
	njCreateQuaternion(vecAngleXZ((Vec3f*)vel), 0.0f, 1.0f, 0.0f, (NJS_QUATERNION*)mQuat);
	mtxSetTransV(mMatrix, &mPos);
	mtxConcatQuat(mMatrix, mMatrix, mQuat);
	mFx = fxSpawnFollow(1, fxBank, 3, mMatrix);
}

// 0x801CD018
TAniLionFireBall::~TAniLionFireBall()
{
	fxStop((Fx*)mFx);
	colliFree(mColli);
}

// 0x801CD088
void TAniLionFireBall::draw()
{
	if (mLife != 0) {
		if (--mLife == 0) {
			mFlags |= 1;
		}
	}
	Vec3f from;
	from.x = mPos.x;
	from.y = mPos.y;
	from.z = mPos.z;
	vecScaleAdd(1.0f / 60.0f, &mPos, &mVel, &mPos);
	EggHits hits;
	if (meshColliProbeLine((EggHit*)hits.mHits, (const Vec3f*)&from, (const Vec3f*)&mPos, (unsigned long)1) != 0) {
		fxSpawnAt(1, mFxBank, 0xC, mMatrix);
		mFlags |= 1;
	} else {
		colliShapeSetPos(colliGetShape(mColli, 0), &mPos);
		mtxSetTransV(mMatrix, &mPos);
		mtxConcatQuat(mMatrix, mMatrix, mQuat);
	}
}
