#include "animal/AniMount.h"
#include "engine/MeshColli.h"
#include "egg/ObjEgg.h"
#include "Ninja/ninja.h"
#include "engine/GroundQuery.h"
#include "core/MathUtil.h"
#include "animal/TAniCamelOstrich.h"
#include "object/TObjEggRing.h"
#include "player/Player.h"
#include "core/VecMath.h"
#include "engine/WorldMesh.h"
#include "animal/AniBody.h"
struct NJS_QUATERNION;

// The end of an EggHit which ObjEgg.h leaves as bytes
struct EggHitResult {
	f32 mDist;        // _3C
	u32 mMask;        // _40
	MeshColli* mMesh; // _44
	u16 mTri;         // _48
};

// What gathers the line query's hits in move
struct AniLineHit {
	MeshColli* mMesh; // _00 the nearest surface it can stand on
	u16 mTri;         // _04
	Vec3f mPos;       // _08
	u32 mMask;        // _14
	f32 mDist;        // _18
	Vec3f mAnyPos;    // _1C the nearest surface of any kind
	u32 mAnyMask;     // _28
	f32 mAnyDist;     // _2C
};

// What gathers the downward query's hits in probeGround
struct AniDropHit {
	MeshColli* mMesh; // _00 the nearest floor it can stand on
	u16 mTri;         // _04
	Vec3f mAnyPos;    // _08 the nearest surface of any kind
	f32 mAnyDist;     // _14
	u32 mAnyMask;     // _18 every kind hit
	Vec3f mPos;       // _1C
	f32 mDist;        // _28
	u32 mMask;        // _2C
	u32 mWant;        // _30 the floors it can stand on
};

static int lineHit(MeshColli* mesh, u16 tri, f32 dist, const Vec3f* pos, AniLineHit* hit);
static int dropHit(MeshColli* mesh, u16 tri, f32 dist, const Vec3f* pos, AniDropHit* hit);

// 0x801C1110
AniBody::AniBody()
    : mState(2)
    , mMask(1)
    , mStopMask(0x810C)
{
	mStep       = 8.0f;
	mPushOut.x  = 0.0f;
	mPushOut.y  = 0.0f;
	mPushOut.z  = 0.0f;
	mWallPush.x = 0.0f;
	mWallPush.y = 0.0f;
	mWallPush.z = 0.0f;
	mVel.x      = 0.0f;
	mVel.y      = 0.0f;
	mVel.z      = 0.0f;
	njCreateQuaternion(0, 0.0f, 0.0f, 1.0f, (NJS_QUATERNION*)&mQuat);
	setState(0);
}

// 0x801C1258
AniBody::~AniBody()
{
}

// 0x801C12C0
void AniBody::setMask(u32 mask)
{
	mMask     = mask;
	mStopMask = ~mask & 0x810C;
}

// 0x801C12D4
void AniBody::update()
{
	runState(mState, 2);
	runState(mState, 3);
	mPushOut.x  = 0.0f;
	mPushOut.y  = 0.0f;
	mPushOut.z  = 0.0f;
	mWallPush.x = 0.0f;
	mWallPush.y = 0.0f;
	mWallPush.z = 0.0f;
}

// 0x801C133C
MeshColli* AniBody::touching(int slot, u16* tri, Vec3f* pos, Vec3f* normal) const
{
	MeshColli* mesh;
	const AniRider* rider;
	rider = &mRiders[slot];
	mesh  = rider->mMesh;
	if (mesh == NULL) {
		return NULL;
	}
	if (tri != NULL) {
		*tri = rider->mTri;
	}
	if (pos != NULL) {
		if (mesh->mFlags & 2) {
			mtxMultVec(mesh->mMatrix, &rider->mPos, pos);
		} else {
			*pos = rider->mPos;
		}
	}
	if (normal != NULL) {
		meshTriNormal(mesh, rider->mTri, normal);
	}
	return mesh;
}

// 0x801C13F0
void AniBody::detach(int slot)
{
	riderDetach((Rider*)&mRiders[slot]);
}

// 0x801C1420
static int lineHit(MeshColli* mesh, u16 tri, f32 dist, const Vec3f* pos, AniLineHit* hit)
{
	MeshTri* t = &mesh->mTris[tri];
	if ((t->mMask & 1) && hit->mDist > dist) {
		hit->mMesh = mesh;
		hit->mTri  = tri;
		hit->mPos  = *pos;
		hit->mMask = t->mMask;
		hit->mDist = dist;
	}
	if (hit->mAnyDist > dist) {
		hit->mAnyDist = dist;
		hit->mAnyPos  = *pos;
		hit->mAnyMask = t->mMask;
	}
	return 1;
}

// 0x801C14AC
void AniBody::move()
{
	AniLineHit hit;
	Vec3f from;
	Vec3f to;
	Vec3f vel;
	Vec3f push;
	Vec3f dir;

	from.x = mPos.x;
	from.y = 8.0f + mPos.y;
	from.z = mPos.z;
	vel    = mVel;
	vecScaleAdd(1.0f / 60.0f, &to, &vel, &from);
	vecAdd(&push, &mWallPush, &mPushOut);
	vecAdd(&to, &push, &to);
	hit.mMesh    = NULL;
	hit.mAnyDist = gFarRayDist;
	hit.mDist    = gFarRayDist;
	hit.mAnyMask = 0;
	if (groundQueryLine((const void*)&from, (const void*)&to, (void*)(-1), (void*)lineHit, (void*)&hit) != 0) {
		if (hit.mMesh != NULL) {
			vecSub(&dir, &to, &from);
			vecNormalize(&dir, &dir);
			vecScaleAdd(hit.mDist - 0.01f, &to, &dir, &from);
		}
		u32 bits = hit.mAnyMask & mStopMask;
		if (bits != 0) {
			if (bits & 4) {
				flagWordSetBit(&mFlags, 5);
			}
			if (bits & 0x100) {
				flagWordSetBit(&mFlags, 3);
			}
			if (bits & 8) {
				flagWordSetBit(&mFlags, 4);
			}
			if (bits & 0x8000) {
				flagWordSetBit(&mFlags, 6);
			}
			flagWordSetBit(&mFlags, 2);
		}
	}
	mPos.x = to.x;
	mPos.y = to.y - 8.0f;
	mPos.z = to.z;
}

// 0x801C164C
int AniBody::wall(MeshList* list, const Vec3f* from, const Vec3f* dir, f32 radius, Vec3f* push, AniRider* rider)
{
	EggHits hits;
	Vec3f to;

	riderDetach((Rider*)rider);
	vecScaleAdd(0.01f + radius, &to, dir, from);
	if (meshListProbeLine((EggHit*)&hits, list, from, (const Vec3f*)&to, 1) == 0) {
		return 0;
	}
	EggHitResult* nearest = (EggHitResult*)hits.mHits[0]._3C;
	if (nearest->mDist < radius) {
		vecScaleAdd(-(radius - nearest->mDist), push, dir, push);
	}
	riderSetMesh((Rider*)rider, (RiderList*)nearest->mMesh);
	rider->mTri = nearest->mTri;
	rider->mPos = hits.mHits[0].mPos;
	return 1;
}

// 0x801C1754
static int dropHit(MeshColli* mesh, u16 tri, f32 dist, const Vec3f* pos, AniDropHit* hit)
{
	MeshTri* t = &mesh->mTris[tri];
	if ((t->mMask & hit->mWant) && hit->mDist > dist) {
		hit->mDist = dist;
		hit->mMesh = mesh;
		hit->mTri  = tri;
		hit->mPos  = *pos;
		hit->mMask = t->mMask;
	}
	if (hit->mAnyDist > dist) {
		hit->mAnyDist = dist;
		hit->mAnyPos  = *pos;
		hit->mAnyMask |= t->mMask;
	}
	return 1;
}

// 0x801C17EC
void AniBody::probeWalls()
{
	f32 mtx[12];
	MeshList list;
	MeshSphere sphere;
	Vec3f center;
	Vec3f dir;
	Vec3f push;

	push.x   = 0.0f;
	push.y   = 0.0f;
	push.z   = 0.0f;
	center.x = mPos.x;
	center.y = 8.0f + mPos.y;
	center.z = mPos.z;
	queryBegin(&list);
	sphere.mPos    = center;
	sphere.mRadius = 10.01f;
	queryGatherBall(&list, &sphere);
	mtxFromQuat((f32*)mtx, (const f32*)&mQuat);

	dir.x = 0.0f;
	dir.y = 0.0f;
	dir.z = 1.0f;
	mtxMultVecSR(mtx, &dir, &dir);
	wall(&list, &center, &dir, 10.0f, &push, &mRiders[ANIRIDER_FRONT]);

	dir.x = 0.0f;
	dir.y = 0.0f;
	dir.z = -1.0f;
	mtxMultVecSR(mtx, &dir, &dir);
	wall(&list, &center, &dir, 10.0f, &push, &mRiders[ANIRIDER_BACK]);

	dir.x = 1.0f;
	dir.y = 0.0f;
	dir.z = 0.0f;
	mtxMultVecSR(mtx, &dir, &dir);
	wall(&list, &center, &dir, 10.0f, &push, &mRiders[ANIRIDER_LEFT]);

	dir.x = -1.0f;
	dir.y = 0.0f;
	dir.z = 0.0f;
	mtxMultVecSR(mtx, &dir, &dir);
	wall(&list, &center, &dir, 10.0f, &push, &mRiders[ANIRIDER_RIGHT]);

	mWallPush = push;
}

// 0x801C19AC
void AniBody::probeGround(f32 snap)
{
	AniDropHit hit;
	f32 mtx[12];
	Vec3f from;
	Vec3f normal;

	from.x = mPos.x;
	from.y = 0.0001f + (mPos.y + mStep);
	from.z = mPos.z;
	flagClear(&mFlags, 0);
	detach(ANIRIDER_FLOOR);
	detach(ANIRIDER_BELOW);
	hit.mMesh    = NULL;
	hit.mAnyDist = gFarRayDist;
	hit.mAnyMask = 0;
	hit.mDist    = gFarRayDist;
	hit.mWant    = 1;
	hit.mWant |= mMask;
	if (groundQueryDown((const void*)&from, (void*)0x18D, (void*)dropHit, (void*)&hit) != 0) {
		if (hit.mMesh != NULL) {
			meshTriNormal(hit.mMesh, hit.mTri, &normal);
			f32 drop;
			if (normal.y > 0.7071f) {
				drop = snap;
			} else {
				drop = 0.0f;
			}
			if (mPos.y - drop <= 0.0001f + hit.mPos.y) {
				AniRider* floor = &mRiders[ANIRIDER_FLOOR];
				riderSetMesh((Rider*)floor, (RiderList*)hit.mMesh);
				floor->mTri = hit.mTri;
				mPos.y      = hit.mPos.y;
				if (hit.mMesh->mFlags & 2) {
					mtxMultVec(hit.mMesh->mInverse, &hit.mPos, &mFloorPos);
					mtxCopy(mFloorMtx, hit.mMesh->mMatrix);
					mtxFromQuat((f32*)mtx, (const f32*)&mQuat);
					mtxMultVecSR(mtx, (Vec3f*)&gAxisZ, &mFloorDir);
					mtxMultVecSR(hit.mMesh->mInverse, &mFloorDir, &mFloorDir);
				}
				flagWordSetBit(&mFlags, 0);
			}
		}
		if (hit.mMesh != NULL) {
			riderSetMesh((Rider*)&mRiders[ANIRIDER_BELOW], (RiderList*)hit.mMesh);
			mRiders[ANIRIDER_BELOW].mTri = hit.mTri;
			mRiders[ANIRIDER_BELOW].mPos = hit.mAnyPos;
		}
	}
}

// 0x801C1B98
void AniBody::checkFall()
{
	if (mPos.y < -400.0f) {
		flagWordSetBit(&mFlags, 6);
		flagWordSetBit(&mFlags, 2);
	}
}

// 0x801C1BE8
void AniBody::followGround()
{
	MeshColli* mesh = touching(ANIRIDER_FLOOR, NULL, NULL, NULL);
	if (mesh != NULL && (mesh->mFlags & 2)) {
		Vec3f was;
		Vec3f now;
		mtxMultVec(mFloorMtx, &mFloorPos, &was);
		mtxMultVec(mesh->mMatrix, &mFloorPos, &now);
		vecSub(&now, &now, &was);
		vecAdd(&mPushOut, &mPushOut, &now);
	}
}

// 0x801C1C78
void AniBody::followTurn()
{
	MeshColli* mesh = touching(ANIRIDER_FLOOR, NULL, NULL, NULL);
	if (mesh != NULL && (mesh->mFlags & 2)) {
		Vec3f dir;
		mtxMultVecSR(mesh->mMatrix, &mFloorDir, &dir);
		setRotY(vecAngleXZ(&dir));
	}
}

// 0x801C1CEC
void AniBody::setRotY(int angle)
{
	njCreateQuaternion(angle, 0.0f, 1.0f, 0.0f, (NJS_QUATERNION*)&mQuat);
}

// 0x801C1D24
void AniBody::setState(int state)
{
	if (mState != 2) {
		runState(mState, 1);
	}
	if (state != 2) {
		runState(state, 0);
	}
	mState = state;
}

// 0x801C1D84
void AniBody::runState(int state, int phase)
{
	switch (state) {
	case 0:
		runGround(phase);
		break;
	case 1:
		runAir(phase);
		break;
	}
}

// 0x801C1DCC
void AniBody::runGround(int phase)
{
	if (phase == 2) {
		if (flagTest(&mFlags, 0) == 0) {
			setState(1);
		}
	} else if (phase == 3) {
		probeWalls();
		move();
		probeGround(mStep);
		checkFall();
	}
}

// 0x801C1E48
void AniBody::runAir(int phase)
{
	if (phase == 2) {
		if (flagTest(&mFlags, 0) != 0) {
			setState(0);
		}
	} else if (phase == 3) {
		probeWalls();
		move();
		probeGround(0.0f);
		checkFall();
	}
}
