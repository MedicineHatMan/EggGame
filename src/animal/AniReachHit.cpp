#include "animal/AniMount.h"
#include "object/ObjAction.h"
#include "enemy/boss/EneBoss.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "engine/ActorMotion.h"

// 0x801CCC88
AniReachHit::AniReachHit()
{
	mColli  = NULL;
	mActive = 0;
}

// 0x801CCCD0
AniReachHit::~AniReachHit()
{
	if (mColli != NULL) {
		colliFree(mColli);
	}
}

// 0x801CCD44
void AniReachHit::attach()
{
	if (mColli == NULL) {
		mColli = (CharaColli*)colliCreate(1, 1);
	}
	colliSetOwner(mColli, (u16)mOwnerId);
	colliSetMasks(mColli, 0x20, 0, 0xFF, 0x10);
	colliGetShape(mColli, 0)->mTag = mActive;
	colliShapeSetSphere(colliGetShape(mColli, 0));
	colliGetShape(mColli, 0)->mSizeX = mRadius;
	colliGetShape(mColli, 0)->mFlags |= 0x18;
}

// 0x801CCE0C
void AniReachHit::detach()
{
	if (mColli != NULL) {
		colliFree(mColli);
		mColli = NULL;
	}
}

// 0x801CCE4C
void AniReachHit::hit(AniLife* life)
{
}

// 0x801CCE50
void AniReachHit::update(AniBody* body)
{
	Vec3f pos;
	getPos(&pos);
	colliShapeSetPos(colliGetShape(mColli, 0), &pos);
}
