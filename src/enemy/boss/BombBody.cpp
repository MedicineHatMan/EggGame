#include "enemy/boss/EneBoss.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyAnim.h"
#include "engine/MotionPlayer.h"
#include "enemy/ObjEnemy.h"
#include "object/ObjCommon.h"
#include "engine/FxHolder.h"
#include "egg/ObjEgg.h"
#include "stage/SetManager.h"
#include "item/ItemParts.h"
#include "animal/AniRider.h"
#include "engine/MeshColli.h"
#include "sound/SeHolder.h"
#include "object/ObjAction.h"
#include "enemy/EnemyFx.h"
#include "egg/EggCreature.h"
#include "ui/AeUtil.h"
#include "enemy/boss/TEneLastBossDarkCtrl.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/MotionPose.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "object/ObjShadow.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "engine/WorldMesh.h"
#include "engine/GroundQuery.h"
#include "enemy/boss/TEneLastExBossFeather.h"
#include "animal/AniBody.h"
struct DrawTask;
struct Vec3f;
struct CharaColli;

// What the drop query keeps the nearest floor below
struct BombDropHit {
	MeshColli* mMesh; // _00
	u16 mTri;         // _04
	Vec3f mPos;       // _08
	f32 mDist;        // _14
};

// 0x8018AAF0
BombBody::BombBody()
{
	riderInit((AniRider*)&mRider);
	mPos.x  = 0.0f;
	mPos.y  = 0.0f;
	mPos.z  = 0.0f;
	mVel.x  = 0.0f;
	mVel.y  = 0.0f;
	mVel.z  = 0.0f;
	mRadius = 0.0f;
	mState  = BOMB_NONE;
}

// 0x8018AB54
BombBody::~BombBody()
{
	run(mState, BOMB_EXIT);
	riderDtor((AniRider*)&mRider, -1);
}

// 0x8018ABB4
int BombBody::getGround(Vec3f* out) const
{
	if ((mFlags.mBits & 1) == 0) {
		return 0;
	}
	out->x = mGround.x;
	out->y = mGround.y;
	out->z = mGround.z;
	return 1;
}

// 0x8018ABE8
void BombBody::launch(const Vec3f* vel)
{
	mVel = *vel;
	setState(BOMB_FLY);
}

// 0x8018AC24
void BombBody::start(const Vec3f* vel)
{
	mVel = *vel;
	setState(BOMB_FALL);
}

// 0x8018AC60
void BombBody::stop()
{
	riderDetach((Rider*)&mRider);
	mVel.x = 0.0f;
	mVel.y = 0.0f;
	mVel.z = 0.0f;
	mFlags.mBits &= ~1;
	setState(BOMB_STOPPED);
}

// 0x8018ACB8
void BombBody::update()
{
	run(mState, BOMB_CHECK);
	run(mState, BOMB_UPDATE);
}

// 0x8018AD04
void BombBody::run(int state, int event)
{
	switch (state) {
	case BOMB_RIDE:
		stateRide(event);
		break;
	case BOMB_FALL:
		stateFall(event);
		break;
	case BOMB_FLY:
		stateFly(event);
		break;
	}
}

// 0x8018AD64
void BombBody::setState(int state)
{
	if (mState != -1) {
		run(mState, BOMB_EXIT);
	}
	if (state != -1) {
		run(state, BOMB_ENTER);
	}
	mState = state;
}

// 0x8018ADC4
// Riding a mesh carried along when it moves and falling when it goes
void BombBody::stateRide(int event)
{
	if (event == BOMB_CHECK) {
		if (mRider.mMesh == 0) {
			setState(BOMB_FALL);
		}
	} else if (event == BOMB_UPDATE) {
		MeshColli* mesh = mRider.mMesh;
		if (mesh->mFlags & 2) {
			mtxMultVec(mesh->mMatrix, &mLocal, &mPos);
			mPos.y += mRadius;
		}
	}
}

// 0x8018AE44
static int bombDropHit(MeshColli* mesh, u16 tri, f32 dist, const Vec3f* pos, BombDropHit* hit)
{
	if (hit->mDist > dist) {
		hit->mDist = dist;
		hit->mMesh = mesh;
		hit->mTri  = tri;
		hit->mPos  = *pos;
	}
	return 1;
}

// 0x8018AE7C
// Falling with gravity and drag
void BombBody::stateFall(int event)
{
	if (event == BOMB_CHECK) {
		if (mRider.mMesh != 0) {
			mVel.x = 0.0f;
			mVel.y = 0.0f;
			mVel.z = 0.0f;
			setState(BOMB_RIDE);
		}
	} else if (event == BOMB_UPDATE) {
		mVel.y += -6.666667f;
		vecScale(0.985f, &mVel, &mVel);
		vecScaleAdd(0.016666668f, &mPos, &mVel, &mPos);
		probe();
	}
}

// 0x8018AF18
// Thrown no gravity
void BombBody::stateFly(int event)
{
	if (event == BOMB_CHECK) {
		if (mRider.mMesh != 0) {
			mVel.x = 0.0f;
			mVel.y = 0.0f;
			mVel.z = 0.0f;
			setState(BOMB_RIDE);
		}
	} else if (event == BOMB_UPDATE) {
		vecScaleAdd(0.016666668f, &mPos, &mVel, &mPos);
		probe();
	}
}

// 0x8018AF94
// Lands on the floor below once it reaches it and notes the floor while it is near
void BombBody::probe()
{
	BombDropHit hit;
	f32 bottom;

	riderDetach((Rider*)&mRider);
	mFlags.mBits &= ~1;
	bottom    = mPos.y - (0.0001f + mRadius);
	hit.mMesh = 0;
	hit.mDist = gFarRayDist;
	if (groundQueryDown((const void*)&mPos, (void*)0x10D, (void*)bombDropHit, (void*)&hit) != 0) {
		if (hit.mMesh != 0 && bottom <= hit.mPos.y) {
			riderSetMesh((Rider*)&mRider, (RiderList*)hit.mMesh);
			mTri   = hit.mTri;
			mPos.y = hit.mPos.y + mRadius;
			if (hit.mMesh->mFlags & 2) {
				mtxMultVec(hit.mMesh->mInverse, &hit.mPos, &mLocal);
			}
		}
		if (hit.mMesh != 0 && bottom <= 50.0f + hit.mPos.y) {
			mFlags.mBits |= 1;
			mGround = hit.mPos;
		}
	}
}
