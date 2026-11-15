#include "egg/ObjEgg.h"
#include "engine/MeshColli.h"
#include "object/ObjAction.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "engine/Viewport.h"
#include "object/TObjPlayer.h"
#include "core/VecMath.h"
#include "engine/Colli.h"
#include "egg/EggRoll.h"
#include "engine/GroundQuery.h"
#include "egg/EggLevel.h"
#include "enemy/Enemy.h"
struct ColliContact;
struct CharaColli;
struct ColliShape;
struct NJS_QUATERNION;

// How a rolling object moves gravity pushing out of other shapes and following a path

// A triangle of a collision model
struct EggFace { // size 0x28
	u8 _00[8];
	u32 mFlags; // _08
	u8 _0C[0x1C];
};

struct EggColModel {
	u8 _00[0x14];
	EggFace* mFaces; // _14
};

struct EggPathHit {
	void* mPath;  // _00
	f32 mPoint;   // _04
	int mSegment; // _08
	u8 _0C[0x18];
};

EggRoll::EggRoll()
{
	f32 zero = 0.0f;
	mMask    = 0;
	_14.x    = zero;
	_14.y    = zero;
	_14.z    = zero;
	mHitCb   = 0;
}

void EggRoll::setVel(const Vec3f* vel)
{
	mBody->mVel = *vel;
}

// 0x800C1AF8
void EggRoll::startSlam(const Vec3f* vel)
{
	mGround.mBits &= ~3;
	mBody->mAccel  = gVecZero;
	mBody->mVel    = *vel;
	mBody->mAngVel = gVecZero;
}

void EggRoll::attachPath(void* path, void* point)
{
	Vec3f vel;
	Vec3f dir;
	mPath->setPath((RailPath*)path);
	mPath->setPoint((RailPoint*)point);
	vel = mBody->mVel;
	mPath->getDir(&dir);
	mPathSpeed = njInnerProduct(&vel, &dir);
}

void EggRoll::attachPathAt(void* path, void* point, f32 speed)
{
	mPath->setPath((RailPath*)path);
	mPath->setPoint((RailPoint*)point);
	mPathSpeed = speed;
}

void EggRoll::kick(u16 unused)
{
	Vec3f pos;
	Vec3f holderPos;
	Vec3f dir;

	TObjAction* holder = findObjById(mOwner->mHolderId);
	if (holder != 0) {
		pos       = mBody->mPos;
		holderPos = *holder->getPos();
		vecSub(&dir, &holderPos, &pos);
		vecFromAngleXZ(10.0f, &dir, vecAngleXZ(&dir));
		mBody->mVel = dir;
	}
}

// 0x800C1CEC
void EggRoll::rebound()
{
	Vec3f pos;
	Vec3f holderPos;
	Vec3f dir;

	TObjAction* holder = findObjById(mOwner->mHolderId);
	if (holder != 0) {
		pos       = mBody->mPos;
		holderPos = *holder->getPos();
		vecSub(&dir, &holderPos, &pos);
		vecFromAngleXZ(*(f32*)((u8*)playerGetParams(0) + 0xB4), &dir, vecAngleXZ(&dir));
		mBody->mVel = dir;
	}
}

void EggRoll::kickHeld()
{
	Vec3f pos;
	Vec3f holderPos;
	Vec3f dir;

	TObjAction* holder = findObjById(mOwner->mHolderId);
	if (holder != 0) {
		pos       = mBody->mPos;
		holderPos = *holder->getPos();
		vecSub(&dir, &holderPos, &pos);
		vecFromAngleXZ(10.0f, &dir, vecAngleXZ(&dir));
		mBody->mVel = dir;
	}
}

// 0x800C1E80
void EggRoll::reboundHard()
{
	Vec3f pos;
	Vec3f holderPos;
	Vec3f dir;

	TObjAction* holder = findObjById(mOwner->mHolderId);
	if (holder != 0) {
		pos       = mBody->mPos;
		holderPos = *holder->getPos();
		vecSub(&dir, &holderPos, &pos);
		vecFromAngleXZ(*(f32*)((u8*)playerGetParams(0) + 0xB4), &dir, vecAngleXZ(&dir));
		mBody->mVel = dir;
	}
}

void EggRoll::stopRolling()
{
	Vec3f spin;
	spin.x = mBody->mVel.x;
	spin.y = mBody->mVel.y;
	spin.z = mBody->mVel.z;
	vecNormalize(&spin, &spin);
	njOuterProduct(&spin, &gVecUp, &spin);
	f32 radius = mBody->mRadius;
	vecScale(*(f32*)((u8*)playerGetParams(0) + 0xB4) / radius, &spin, &spin);
	mBody->mAngVel = spin;
	mBody->mVel    = gVecZero;
}

void EggRoll::leavePath()
{
	Vec3f vel;
	mPath->getDir(&vel);
	vecScale(mPathSpeed, &vel, &vel);
	mBody->mVel = vel;
}

u32 EggRoll::getMask()
{
	return mMask;
}

int EggRoll::hitWhileRolling(int* result, int power)
{
	Vec3f vel;
	vel.x = mBody->mVel.x;
	vel.y = mBody->mVel.y;
	vel.z = mBody->mVel.z;
	if (njScalor(&vel) < 10.0f) {
		return 0;
	}
	HitInfo* hit = (HitInfo*)result;
	hit->mDamage = power;
	hit->mFlags.setBit(2);
	hit->mFlags.setBit(0xC);
	vecScale(0.016666668f, &hit->mPos, &vel);
	return power;
}

int EggRoll::hitWhileHeld(int* result, int power)
{
	HitInfo* hit = (HitInfo*)result;
	hit->mDamage = power;
	hit->mFlags.setBit(1);
	return power;
}

// 0x800C215C
void EggRoll::pushOff(void* coll, void* first, void* end)
{
	Vec3f pos;
	Vec3f dir;
	Vec3f push;
	Vec3f vel;

	pos        = mBody->mPos;
	vel        = mBody->mVel;
	f32 radius = mBody->mRadius;
	push.x     = 0.0f;
	push.y     = push.x;
	push.z     = push.x;
	for (EggContact* it = (EggContact*)first; it != end; it = it->mNext) {
		if ((*(u32*)((u8*)colliGetShape((CharaColli*)coll, it->mShape) + 0x24) & 0x400) != 0
		    && (*(u32*)((u8*)colliContactShape((ColliContact*)it) + 0x24) & 0x400) != 0) {
			vecSub(&dir, &pos, &it->mPos);
			f32 dist          = vecNormalize(&dir, &dir);
			ColliShape* other = (ColliShape*)(colliContactShape((ColliContact*)it));
			f32 force         = radius + other->mSizeX - dist;
			force *= 360.0f;
			vecScale(force, &dir, &dir);
			njAddVector(&mBody->mAccel, &dir);
		} else if ((*(u32*)((u8*)colliContactShape((ColliContact*)it) + 0x24) & 0x10) == 0
		           && (*(u32*)((u8*)colliGetShape((CharaColli*)coll, it->mShape) + 0x24) & 0x10) == 0) {
			f32 depth = vecNormalize(&dir, &it->mNormal);
			if (njInnerProduct(&vel, &dir) < 0.0f) {
				vecScaleAdd(depth / 0.016666668f / 0.016666668f, &push, &dir, &push);
			}
		}
	}
	njAddVector(&mBody->mAccel, &push);
}

void EggRoll::moveGround()
{
	Vec3f gravity;
	mBody->mFriction = *(f32*)playerGetParamsTail();
	gravity.y        = *(f32*)((u8*)playerGetParamsTail() + 0x24);
	gravity.x        = 0.0f;
	gravity.z        = gravity.x;
	njAddVector(&mBody->mAccel, &gravity);
	step();
}

void EggRoll::moveRolling()
{
	Vec3f gravity;
	mBody->mFriction = *(f32*)((u8*)playerGetParamsTail() + 0xC);
	gravity.y        = *(f32*)((u8*)playerGetParamsTail() + 0x24);
	gravity.x        = 0.0f;
	gravity.z        = gravity.x;
	njAddVector(&mBody->mAccel, &gravity);
	step();
}

// 0x800C23E0
void EggRoll::movePath()
{
	Vec3f pos;
	Vec3f normal;
	Vec3f dir;
	Vec3f axis;
	EggRot rot;
	Quat q;
	Quat half;
	Quat tmp;
	Quat spin;

	mPath->advance(0.016666668f * mPathSpeed);
	mPath->getPos(&pos);
	mPath->getNormal(&normal);
	mPath->getDir(&dir);
	vecScaleAdd(mBody->mRadius, &pos, &normal, &pos);
	mBody->mPos = pos;
	njOuterProduct(&dir, &normal, &axis);
	vecScale((0.016666668f * -mPathSpeed) / mBody->mRadius, &axis, &axis);
	rot = mBody->mRot;
	q.w   = 0.0f;
	q.v.x = axis.x;
	q.v.y = axis.y;
	q.v.z = axis.z;
	spin  = q;
	njMultiQuaternion((const NJS_QUATERNION*)&spin, (const NJS_QUATERNION*)&rot, (NJS_QUATERNION*)&spin);
	tmp = spin;
	quatScale(&tmp, 0.5f);
	half = tmp;
	quatAdd((Quat*)&rot, &half);
	quatNormalize((Quat*)&rot);
	mBody->mRot = rot;
}

void EggRoll::moveBounce()
{
	Vec3f gravity;
	mBody->mFriction = *(f32*)((u8*)playerGetParamsTail() + 0xC);
	gravity.y        = *(f32*)((u8*)playerGetParamsTail() + 0x24);
	gravity.x        = 0.0f;
	gravity.z        = gravity.x;
	njAddVector(&mBody->mAccel, &gravity);
	step();
}

void EggRoll::moveSlow()
{
	Vec3f gravity;
	mBody->mFriction = *(f32*)((u8*)playerGetParamsTail() + 0xC);
	gravity.y        = *(f32*)((u8*)playerGetParamsTail() + 0x24);
	gravity.x        = 0.0f;
	gravity.z        = gravity.x;
	njAddVector(&mBody->mAccel, &gravity);
	stepSlow();
}

void EggRoll::moveSlam()
{
	Vec3f gravity;
	mBody->mFriction = *(f32*)playerGetParamsTail();
	gravity.y        = *(f32*)((u8*)playerGetParamsTail() + 0x24);
	gravity.x        = 0.0f;
	gravity.z        = gravity.x;
	njAddVector(&mBody->mAccel, &gravity);
	step();
	if (mGround.mBits & 1) {
		mBody->mVel = gVecZero;
	}
}

extern "C" int eggCollectMask(EggColModel* model, u16 face, EggHitInfo* info)
{
	info->mMask |= *(u32*)((u8*)model->mFaces + face * 0x28 + 8);
	return 1;
}

void EggRoll::checkBumps()
{
	EggHitInfo info;
	struct {
		Vec3f mPos;
		f32 mRadius;
	} sphere;

	info.mMask     = 0;
	sphere.mPos    = mBody->mPos;
	sphere.mRadius = mBody->mRadius;
	groundQuerySphere((const void*)&sphere, (void*)eggCollectMask, (void*)&info, (void*)(-1));
	mMask = info.mMask;
}

int EggRoll::isRollDone()
{
	return 0;
}

// 0x800C278C
int EggRoll::isPathDone()
{
	RailPath* path = mPath->mPath;
	if (path != 0) {
		if (mPathSpeed < 0.0f) {
			if (mPath->getDistance() == 0.0f) {
				return 1;
			}
		} else {
			f32 length = path->getLength();
			f32 dist   = mPath->getDistance();
			if (dist == length) {
				return 1;
			}
		}
	}
	return 0;
}

int EggRoll::isSlamDone()
{
	return 0;
}

int EggRoll::hasLanded()
{
	return mGround.mBits & 1;
}

// 0x800C2AC8
extern "C" void eggPathFound(RailPath* path, const RailPoint* point, Vec3f* unused, void* arg, int* more)
{
	EggPathHit* out = (EggPathHit*)arg;
	out->mPath      = path;
	int segment     = point->mSeg;
	f32 at          = point->mDist;
	out->mPoint     = at;
	out->mSegment   = segment;
	*more           = 0;
}

int EggRoll::findPath(void** path, void* point)
{
	EggPathHit hit;
	Vec3f pos;

	pos = mBody->mPos;
	if (railSearch(mBody->mRadius, &pos, eggPathFound, &hit) != 0) {
		if (path != 0) {
			*path = hit.mPath;
		}
		if (point != 0) {
			((f32*)point)[0] = hit.mPoint;
			((int*)point)[1] = hit.mSegment;
		}
		return 1;
	}
	return 0;
}

// 0x800C28D4
int EggRoll::isPastHolder()
{
	Vec3f pos;
	Vec3f holderPos;
	Vec3f vel;

	TObjAction* holder = findObjById(mOwner->mHolderId);
	if (holder != 0) {
		pos       = mBody->mPos;
		holderPos = *holder->getPos();
		vel       = mBody->mVel;
		if (angleDist(angleToXZ(&holderPos, &pos), vecAngleXZ(&vel)) > 0x4000) {
			return 0;
		}
	}
	return 1;
}

int EggRoll::hitWall()
{
	return mGround.mBits & 2;
}

void EggRoll::step()
{
	EggHitInfo info;

	info.mMask = 0;
	info.mCb   = mHitCb;
	info.mArg  = mHitArg;
	mBody->stepRoll(0.016666668f, &info, -1, NULL, (EggFaceFn)eggNoteFace);
	mGround  = info.mBits;
	f32 zero = 0.0f;
	mMask    = info.mMask;
	_14.x    = zero;
	_14.y    = zero;
	_14.z    = zero;
}

void EggRoll::stepSlow()
{
	EggHitInfo info;

	info.mMask = 0;
	info.mCb   = mHitCb;
	info.mArg  = mHitArg;
	mBody->stepSlide(0.016666668f, &info, -1, NULL, (EggFaceFn)eggNoteFace);
	mGround  = info.mBits;
	f32 zero = 0.0f;
	mMask    = info.mMask;
	_14.x    = zero;
	_14.y    = zero;
	_14.z    = zero;
}

// 0x800C2AE8
extern "C" void eggNoteFace(EggColModel* model, u32 face, void* a3, const f32* normal, EggHitInfo* info)
{
	f32 y      = normal[1];
	int kind   = 0;
	EggFace* f = &model->mFaces[face & 0xFFFF];
	info->mMask |= f->mFlags;
	if (y > 0.8660254f) {
		kind |= 1;
		info->mBits.mBits |= 1;
	} else if (y < 0.7f) {
		kind |= 2;
		info->mBits.mBits |= 2;
	}
	if (info->mCb != 0) {
		((void (*)(EggColModel*, u32, u32, void*, const f32*, int, void*))info->mCb)(model, face, f->mFlags, a3, normal, kind, info->mArg);
	}
}

extern "C" int eggStopOnGround(EggColModel* model, u16 face, u32 flags)
{
	return flags & 0;
}

int EggRoll::isGrounded()
{
	return mGround.mBits & 1;
}

// Walls give back some of the speed into them
f32 EggSurface::getBounce(MeshColli* mesh, u16 tri, void* arg)
{
	Vec3f normal;
	meshTriNormalU16(mesh, tri, &normal);
	if (normal.y < 0.5) {
		return 0.4f;
	}
	return 0.0f;
}

// Faces with 0x400 have none
// 0x800C2C14
f32 EggSurface::getFriction(MeshColli* mesh, u16 tri, void* arg)
{
	struct N {
		u32 a, b;
		struct {
			u32 m;
		} n;
		u8 p[0x1C];
	};
	if (((N*)mesh->mTris)[tri].n.m & 0x400) {
		return 0.0f;
	}
	return EggBody::getFriction(mesh, tri, arg);
}

// Floors unless they have 0x2000
// 0x800C2C58
int EggSurface::canStop(MeshColli* mesh, u16 tri, u32 flags, void* arg)
{
	return (flags & 0x2000) ? 0 : (flags & 1);
}
