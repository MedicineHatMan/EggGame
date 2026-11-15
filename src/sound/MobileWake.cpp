#include "engine/TMobileObject.h"
#include "engine/Fx.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "engine/GroundQuery.h"
#include "sound/TrackHit.h"

// 0x801D195C
MobileWake::MobileWake(TMobileObject* owner)
{
	mOwner = owner;
	mBody  = NULL;
	mFx    = NULL;
	mTimer = 0;
}

// 0x801D1974
MobileWake::~MobileWake()
{
	fxStop(mFx);
}

// 0x801D19C0
void MobileWake::start()
{
	f32 mtx[12];
	Vec3f pos;
	Vec3f vel;

	if (mFx == NULL) {
		pos = mBody->mPos;
		vel = mBody->mVel;
		mtxSetTransV(mtx, &pos);
		mtxRotateY(mtx, mtx, vecAngleXZ(&vel));
		mFx    = fxSpawnAt(1, 0, pickEffect(&vel), mtx);
		mTimer = 0;
	}
}

// 0x801D1A78
void MobileWake::stop()
{
	if (mFx != NULL) {
		fxStop(mFx);
		mFx = NULL;
	}
}

// 0x801D1AB8
static int wakeHit(MeshColli*, u16, f32 dist, const Vec3f* pos, TrackHit* hit)
{
	if (hit->mDist > dist) {
		hit->mDist  = dist;
		hit->mPos   = *pos;
		hit->mFound = 1;
	}
	return 1;
}

// 0x801D1AF0
void MobileWake::update()
{
	f32 mtx[12];
	TrackHit hit;
	u16 id;

	mTimer++;
	if (mFx != NULL && (mTimer & 7) == 0) {
		Vec3f vel;
		vel = mBody->mVel;
		id  = pickEffect(&vel);
		if (id != mFx->mIndex) {
			fxStop(mFx);
			mFx = fxSpawn(1, 0, id);
		}
	}
	if (mFx != NULL) {
		Vec3f from;
		Vec3f vel;
		from        = mBody->mPos;
		vel         = mBody->mVel;
		hit.mFound  = 0;
		from.y      = from.y + mBody->mRadius;
		hit.mDist   = 2.0f * mBody->mRadius;
		groundQueryDown((const void*)&from, (void*)4, (void*)wakeHit, (void*)&hit);
		if (hit.mFound == 0) {
			fxStop(mFx);
			mFx = NULL;
		} else {
			mtxSetTrans(mtx, hit.mPos.x, 0.5f + hit.mPos.y, hit.mPos.z);
			mtxRotateY(mFx->mMatrix, mtx, vecAngleXZ(&vel));
		}
	}
}

// 0x801D1C64
u16 MobileWake::pickEffect(Vec3f* vel)
{
	if (vecLengthSq(vel) > 10000.0f) {
		return 0x84;
	}
	return 0x81;
}
