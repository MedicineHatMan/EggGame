#include "egg/ObjEgg.h"
#include "engine/MeshColli.h"
#include "egg/EggCreature.h"
#include "egg/EggPlane.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "Ninja/ninja.h"
#include "player/Player.h"
#include "engine/Viewport.h"
#include "core/VecMath.h"
#include "engine/WorldMesh.h"
#include "engine/GroundQuery.h"
#include "animal/AniBody.h"
struct NJS_QUATERNION;

// The nearest face a line query crossed
struct EggLineHit {
	EggLineHit()
	{
		mMesh = NULL;
		mDist = gFarRayDist;
	}

	MeshColli* mMesh; // _00
	u16 mTri;         // _04
	Vec3f mPos;       // _08
	u32 mMask;        // _14
	f32 mDist;        // _18
};

// What sweep's line query gathers the nearest floor and the nearest face of any kind
struct EggLineHits {
	EggLineHit mFloor; // _00
	EggLineHit mAny;   // _1C
};

/** The graze callback as the sweep calls it the same registers as EggFaceFn with the distance named before arg */
typedef void (*EggGrazeFn)(MeshColli* mesh, u16 tri, Vec3f* pos, const Vec3f* normal, f32 dist, void* arg);

/**
 * A ball swept from mFrom to mTo, and the nearest face that stops it. The triangle tests fill it in.
 * @size 0x78
 */
struct EggSweep {
	Vec3f mFrom; // _00
	Vec3f mTo;   // _0C
	Vec3f mDir;  // _18 mTo - mFrom
	Vec3f mUnit; // _24
	Vec3f mPos;  // _30 where the ball stops
	f32 _3C;
	f32 mRadius;       // _40
	f32 mDist;         // _44
	Vec3f mNormal;     // _48 of the face it stops against
	MeshColli* mMesh;  // _54 that face's mesh or null
	EggBody* mBody;    // _58
	u16 mTri;          // _5C
	void* mArg;        // _60
	u32 mMask;         // _64 the faces it tests
	EggGrazeFn mGraze; // _68
	Vec3f mPush;       // _6C out of the edges it overlaps
};

// 0x8008FAB0
// Counts down true on the tick that reaches 0
extern "C" int tickTimer16(u16* timer)
{
	u16 time = *timer;
	if (time != 0) {
		*timer = --time;
		if (time == 0) {
			return 1;
		}
	}
	return 0;
}

// 0x8008FADC
EggBody::EggBody()
{
	mStopMask = 0x810C;
	mAccel.x  = 0.0f;
	mAccel.y  = 0.0f;
	mAccel.z  = 0.0f;
	mVel.x    = 0.0f;
	mVel.y    = 0.0f;
	mVel.z    = 0.0f;
	mPos.x    = 0.0f;
	mPos.y    = 0.0f;
	mPos.z    = 0.0f;
	njCreateQuaternion(0, 0.0f, 0.0f, 1.0f, (NJS_QUATERNION*)&mRot);
	mAngVel.x = 0.0f;
	mAngVel.y = 0.0f;
	mAngVel.z = 0.0f;
	mPush.x   = 0.0f;
	mPush.y   = 0.0f;
	mPush.z   = 0.0f;
	mFriction = 0.01f;
	mBounce   = 0.1f;
	mRadius   = 1.0f;
}

// 0x8008FBE8
void EggBody::makeMatrix(f32* mtx)
{
	mtxSetTransV(mtx, &mPos);
	mtxConcatQuat(mtx, mtx, (const f32*)&mRot);
}

// 0x8008FC30
f32 EggBody::getBounce(MeshColli* mesh, u16 tri, void* arg)
{
	return mBounce;
}

// 0x8008FC38
f32 EggBody::getFriction(MeshColli* mesh, u16 tri, void* arg)
{
	return mFriction;
}

// 0x8008FC40
// Only floors
int EggBody::canStop(MeshColli* mesh, u16 tri, u32 flags, void* arg)
{
	return flags & 1;
}

// 0x8008FC48
// Moves turning the spin to match the roll
void EggBody::stepRoll(f32 dt, void* arg, u32 mask, EggFaceFn graze, EggFaceFn face)
{
	move(dt, arg, mask, graze, face, 1);
	spin(dt);
}

// 0x8008FC90
// Moves keeping the spin
void EggBody::stepSlide(f32 dt, void* arg, u32 mask, EggFaceFn graze, EggFaceFn face)
{
	move(dt, arg, mask, graze, face, 0);
	spin(dt);
}

static void sweepTri(MeshColli* mesh, u16 tri, EggSweep* sweep);

// 0x8008FCD8
void EggBody::move(f32 dt, void* arg, u32 mask, EggFaceFn graze, EggFaceFn face, int roll)
{
	EggSweep ctx;
	f32 box[6];
	Vec3f from;
	Vec3f to;
	MeshList list;
	Vec3f accel;
	Vec3f hitPos;
	Vec3f impulse;
	Vec3f tangent;
	Vec3f touch;
	f32 dist;
	u16 tri;

	followMesh(&mPos);
	riderDetach((Rider*)&mItems[0]);
	riderDetach((Rider*)&mItems[2]);
	from = mPos;
	vecScaleAdd(dt, &to, &mVel, &from);
	vecScaleAdd(dt * dt, &to, &mAccel, &to);
	vecAdd(&to, &mPush, &to);
	f32 radius = mRadius;
	boxAroundLine(box, &from, &to);
	box[0] -= radius;
	box[1] -= radius;
	box[2] -= radius;
	box[3] += radius;
	box[4] += radius;
	box[5] += radius;
	accel    = mAccel;
	mAccel.x = 0.0f;
	mAccel.y = 0.0f;
	mAccel.z = 0.0f;
	mPush.x  = 0.0f;
	mPush.y  = 0.0f;
	mPush.z  = 0.0f;
	queryBegin(&list);
	if (queryGatherBox(&list, box) == 0) {
		mPos = to;
		vecScaleAdd(dt, &mVel, &accel, &mVel);
		return;
	}

	dist           = 0.0f;
	MeshColli* hit = sweep(&list, &from, &to, &hitPos, &dist, &tri);
	if (hit != NULL) {
		to = hitPos;
	}
	ctx.mMesh   = NULL;
	ctx.mPush.x = 0.0f;
	ctx.mPush.y = 0.0f;
	ctx.mPush.z = 0.0f;
	ctx.mFrom   = from;
	ctx.mTo     = to;
	vecSub(&ctx.mDir, &ctx.mTo, &ctx.mFrom);
	vecNormalize(&ctx.mUnit, &ctx.mDir);
	ctx.mMesh   = hit;
	ctx.mTri    = tri;
	ctx.mRadius = radius;
	ctx.mDist   = dist;
	ctx.mPos    = hitPos;
	ctx.mArg    = arg;
	ctx.mGraze  = (EggGrazeFn)graze;
	ctx.mBody   = this;
	ctx.mMask   = mask;
	if (hit != NULL) {
		meshTriNormal(hit, tri, &ctx.mNormal);
	}
	meshListWalk(&list, (void*)sweepTri, &ctx);

	if (ctx.mMesh == NULL) {
		mPos = to;
		vecScaleAdd(dt, &mVel, &accel, &mVel);
		mPush = ctx.mPush;
		return;
	}

	impulse.x    = 0.0f;
	impulse.y    = 0.0f;
	impulse.z    = 0.0f;
	f32 bounce   = getBounce(ctx.mMesh, ctx.mTri, arg);
	f32 friction = getFriction(ctx.mMesh, ctx.mTri, arg);
	mPos         = ctx.mPos;
	if (roll) {
		tangent.x = 0.0f;
		tangent.y = -1.0f;
		tangent.z = 0.0f;
		njOuterProduct(&mVel, &tangent, &tangent);
		vecScale(1.0f / mRadius, &mAngVel, &tangent);
	}
	f32 dot = njInnerProduct(&mVel, &ctx.mNormal);
	if (dot < 0.0f) {
		vecScale(dot * -(1.0f + bounce) / dt, &impulse, &ctx.mNormal);
	}
	if (friction > 0.0f) {
		vecScaleAdd(-njInnerProduct(&ctx.mDir, &ctx.mNormal), &tangent, &ctx.mNormal, &ctx.mDir);
		vecNormalize(&tangent, &tangent);
		vecScaleAdd(-(friction * vecLength(&impulse)), &impulse, &tangent, &impulse);
	}
	vecAdd(&accel, &accel, &impulse);
	if (face != NULL) {
		vecScaleAdd(-radius, &touch, &ctx.mNormal, &ctx.mPos);
		face(ctx.mMesh, ctx.mTri, &touch, &ctx.mNormal, arg, ctx.mDist);
	}
	if (ctx.mNormal.y > 0.7071f) {
		riderSetMesh((Rider*)&mItems[0], (RiderList*)ctx.mMesh);
		mItems[0].mTri = ctx.mTri;
		vecScaleAdd(-radius, &mItems[0].mPos, &ctx.mNormal, &ctx.mPos);
		if (ctx.mMesh->mFlags & 2) {
			mtxMultVec(ctx.mMesh->mInverse, &mItems[0].mPos, &mMeshPos);
			mtxCopy(mMeshMtx, ctx.mMesh->mMatrix);
		}
	}
	vecScaleAdd(dt, &mVel, &accel, &mVel);
	mPush = ctx.mPush;
}

// By value as the DOL copies the rotation into each helper's own slot
static inline EggRot eggRotMul(EggRot q, const EggRot* rot)
{
	njMultiQuaternion((const NJS_QUATERNION*)&q, (const NJS_QUATERNION*)rot, (NJS_QUATERNION*)&q);
	return q;
}

static inline EggRot eggRotScale(EggRot q, f32 scale)
{
	quatScale((Quat*)&q, scale);
	return q;
}

// 0x80090234
// Turns mRot by the spin
void EggBody::spin(f32 dt)
{
	EggRot turn;
	Vec3f axis;

	vecScale(dt, &axis, &mAngVel);
	turn.mW         = 0.0f;
	turn.mAxis.v[0] = axis.x;
	turn.mAxis.v[1] = axis.y;
	turn.mAxis.v[2] = axis.z;
	EggRot add      = eggRotScale(eggRotMul(turn, &mRot), 0.5f);
	quatAdd((Quat*)&mRot, (const Quat*)&add);
	quatNormalize((Quat*)&mRot);
}

// 0x80090318
// Carries pos along with the mesh it rests on when that mesh moves
void EggBody::followMesh(Vec3f* pos)
{
	Vec3f was;
	Vec3f now;

	MeshColli* mesh = touching(0, NULL, NULL, NULL);
	if (mesh != NULL && (mesh->mFlags & 2)) {
		mtxMultVec(mMeshMtx, &mMeshPos, &was);
		mtxMultVec(mesh->mMatrix, &mMeshPos, &now);
		vecSub(&now, &now, &was);
		njAddVector(pos, &now);
	}
}

// 0x800903A8
static int lineHit(MeshColli* mesh, u16 tri, f32 dist, const Vec3f* pos, EggLineHits* hits)
{
	MeshTri* face = &mesh->mTris[tri];
	if ((face->mMask & 1) && hits->mFloor.mDist > dist) {
		hits->mFloor.mDist = dist;
		hits->mFloor.mMesh = mesh;
		hits->mFloor.mTri  = tri;
		hits->mFloor.mPos  = *pos;
		hits->mFloor.mMask = face->mMask;
	}
	if (hits->mAny.mDist > dist) {
		hits->mAny.mDist = dist;
		hits->mAny.mMesh = mesh;
		hits->mAny.mTri  = tri;
		hits->mAny.mPos  = *pos;
		hits->mAny.mMask = face->mMask;
	}
	return 1;
}

// 0x8009043C
// The floor the centre's path from from to to crosses first if it stops the ball pos is set just short of it and dist
// to how far along it is
MeshColli* EggBody::sweep(MeshList* list, const Vec3f* from, const Vec3f* to, Vec3f* pos, f32* dist, u16* tri)
{
	Vec3f end;
	Vec3f dir;

	vecSub(&dir, to, from);
	f32 len = vecNormalize(&dir, &dir);
	vecScaleAdd(mRadius, &end, &dir, to);
	EggLineHits hits;
	hits.mFloor.mDist = hits.mAny.mDist = len + mRadius;
	if ((u32)meshListQueryLine(list, from, (const Vec3f*)&end, 0x818D, (int (*)(MeshColli*, int, float, const Vec3f*, void*))lineHit,
	                           (void*)&hits)
	    == 0) {
		return NULL;
	}
	if (hits.mAny.mMesh != NULL) {
		if (hits.mAny.mDist <= len) {
			u32 kind = hits.mAny.mMask;
			if (kind & mStopMask) {
				if (kind & 0x100) {
					flagByteSet(&mFlags, 3);
				}
				if (kind & 8) {
					flagByteSet(&mFlags, 4);
				}
				if (kind & 0x8000) {
					flagByteSet(&mFlags, 6);
				}
				if (kind & 4) {
					flagByteSet(&mFlags, 5);
				}
				flagByteSet(&mFlags, 2);
			}
		}
		riderSetMesh((Rider*)&mItems[2], (RiderList*)hits.mAny.mMesh);
		mItems[2].mTri = hits.mAny.mTri;
		mItems[2].mPos = hits.mAny.mPos;
	}
	if (hits.mFloor.mMesh == NULL) {
		return NULL;
	}
	*dist = hits.mFloor.mDist - mRadius;
	if (*dist < 0.0f) {
		return NULL;
	}
	vecScaleAdd(*dist - 0.0001f, pos, &dir, from);
	*tri = hits.mFloor.mTri;
	if (canStop(hits.mFloor.mMesh, hits.mFloor.mTri, hits.mFloor.mMesh->mTris[*tri].mMask, NULL)) {
		return hits.mFloor.mMesh;
	}
	return NULL;
}

// 0x80090670
// What a rider stands on and the triangle point and normal it touched
MeshColli* EggBody::touching(int slot, u16* tri, Vec3f* pos, Vec3f* normal) const
{
	MeshColli* mesh;
	const EggBodyItem* item;
	item = &mItems[slot];
	mesh = item->mMesh;
	if (mesh != NULL) {
		if (tri != NULL) {
			*tri = item->mTri;
		}
		if (pos != NULL) {
			if (mesh->mFlags & 2) {
				mtxMultVec(mesh->mMatrix, &item->mPos, pos);
			} else {
				*pos = item->mPos;
			}
		}
		if (normal != NULL) {
			meshTriNormal(mesh, item->mTri, normal);
		}
	}
	return mesh;
}

// 0x8009071C
// Keeps a hit if it is the nearest yet and the face stops the ball
static int sweepHit(EggSweep* sweep, MeshColli* mesh, u16 tri, u32 flags, f32 dist)
{
	if (sweep->mMesh == NULL || dist <= sweep->mDist) {
		if (sweep->mBody->canStop(mesh, tri, flags, sweep->mArg)) {
			sweep->mMesh = mesh;
			sweep->mTri  = tri;
			sweep->mDist = dist;
			return 1;
		}
	}
	return 0;
}

// 0x800907BC
// The ball against a triangle's face already touching it or meeting it along the sweep
static int sweepFace(EggSweep* sweep, const Vec3f* a, const Vec3f* b, const Vec3f* c, const Vec3f* normal, MeshColli* mesh, u16 tri,
                     u32 flags)
{
	Vec3f lifted;
	Vec3f meet;

	vecScaleAdd(sweep->mRadius, &lifted, normal, a);
	f32 height = planeHeight(&sweep->mFrom, &lifted, normal);
	if (height < 0.0f) {
		if (pointInTriangle(&sweep->mFrom, a, b, c, normal)) {
			if (sweepHit(sweep, mesh, tri, flags, 0.0f)) {
				sweep->mNormal = *normal;
				vecScaleAdd(0.01f - height, &sweep->mPos, normal, &sweep->mFrom);
			}
			if (sweep->mGraze != NULL) {
				sweep->mGraze(mesh, tri, &sweep->mFrom, normal, 0.0f, sweep->mArg);
			}
			return 1;
		}
		return 0;
	}
	if (planeMeet(height, &sweep->mFrom, &sweep->mDir, normal, &meet) == 0) {
		return 0;
	}
	if (pointInTriangle(&meet, a, b, c, normal) == 0) {
		return 0;
	}
	f32 dist = vecDistanceSq(&sweep->mFrom, &meet);
	if (sweepHit(sweep, mesh, tri, flags, dist)) {
		f32 depth = __fabs(planeHeight(&sweep->mTo, &meet, normal));
		depth += 0.01f;
		vecScaleAdd(depth, &sweep->mPos, normal, &sweep->mTo);
		sweep->mNormal = *normal;
	}
	if (sweep->mGraze != NULL) {
		vecScaleAdd(-sweep->mRadius, &lifted, normal, &meet);
		sweep->mGraze(mesh, tri, &lifted, normal, dist, sweep->mArg);
	}
	return 1;
}

// 0x800909FC
// The ball against an edge pushes it out when it overlaps
static int sweepEdge(EggSweep* sweep, const Vec3f* a, const Vec3f* b, MeshColli* mesh, u16 tri, u32 flags)
{
	Vec3f edge;
	Vec3f rel;
	Vec3f away;

	if (sweep->mBody->canStop(mesh, tri, flags, sweep->mArg) == 0) {
		return 0;
	}
	vecSub(&edge, b, a);
	vecSub(&rel, &sweep->mFrom, a);
	f32 len = njInnerProduct(&edge, &edge);
	f32 t   = njInnerProduct(&rel, &edge) / len;
	if (t < 0.0f || t > 1.0f) {
		return 0;
	}
	vecScaleAdd(t, &rel, &edge, a);
	vecSub(&away, &sweep->mFrom, &rel);
	f32 dist = vecNormalize(&away, &away);
	if (dist > sweep->mRadius || dist < 0.0f) {
		return 0;
	}
	dist = sweep->mRadius - dist;
	vecScaleAdd(0.5f * dist, &sweep->mPush, &away, &sweep->mPush);
	return 1;
}

// 0x80090B3C
// Each triangle near the sweep
#define meshTriCornersU16(mesh, tri, a, b, c) (((void (*)(MeshColli*, u16, Vec3f*, Vec3f*, Vec3f*))meshTriCorners)(mesh, tri, a, b, c))

static void sweepTri(MeshColli* mesh, u16 tri, EggSweep* param)
{
	MeshTri* face   = &mesh->mTris[tri];
	EggSweep* sweep = (EggSweep*)(const EggSweep*)param;
	if (face->mMask & param->mMask) {
		Vec3f a;
		Vec3f b;
		Vec3f c;
		Vec3f normal;
		meshTriCornersU16(mesh, tri, &a, &b, &c);
		meshTriNormalU16(mesh, tri, &normal);
		if (planeHeightInline(&sweep->mFrom, &a, &normal) < -0.0001f) {
			return;
		}
		if (sweepFace(sweep, &a, &b, &c, &normal, mesh, tri, face->mMask)) {
			return;
		}
		if (sweepEdge(sweep, &a, &b, mesh, tri, face->mMask)) {
			return;
		}
		if (sweepEdge(sweep, &b, &c, mesh, tri, face->mMask)) {
			return;
		}
		if (sweepEdge(sweep, &c, &a, mesh, tri, face->mMask)) {
			return;
		}
	}
}

// 0x80090C5C
void EggBody::setPos(const Vec3f* pos)
{
	mPos = *pos;
}
