#include "engine/TMobileObject.h"
#include "engine/Fx.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "engine/GroundQuery.h"
#include "sound/TrackHit.h"

// 0x801D1628
MobileDust::MobileDust(TMobileObject* owner)
{
	mOwner = owner;
	mBody  = NULL;
	mFx    = NULL;
	mTimer = 0;
}

// 0x801D1640
MobileDust::~MobileDust()
{
	fxStop(mFx);
}

// 0x801D168C
void MobileDust::start()
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

// 0x801D1744
void MobileDust::stop()
{
	if (mFx != NULL) {
		fxStop(mFx);
		mFx = NULL;
	}
}

// 0x801D1784
static int dustHit(MeshColli*, u16, f32 dist, const Vec3f* pos, TrackHit* hit)
{
	if (hit->mDist > dist) {
		hit->mDist  = dist;
		hit->mPos   = *pos;
		hit->mFound = 1;
	}
	return 1;
}

// 0x801D17BC
void MobileDust::update()
{
	f32 mtx[12];
	TrackHit hit;
	u16 id;

	mTimer++;
	if (mFx != NULL && (mTimer & 0xF) == 0) {
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
		groundQueryDown((const void*)&from, (void*)8, (void*)dustHit, (void*)&hit);
		if (hit.mFound == 0) {
			fxStop(mFx);
			mFx = NULL;
		} else {
			mtxSetTransV(mtx, &hit.mPos);
			mtxRotateY(mFx->mMatrix, mtx, vecAngleXZ(&vel));
		}
	}
}

// 0x801D1920
u16 MobileDust::pickEffect(Vec3f* vel)
{
	if (vecLengthSq(vel) > 10000.0f) {
		return 0x88;
	}
	return 0x8A;
}
