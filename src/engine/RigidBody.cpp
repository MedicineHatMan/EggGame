#include "engine/RigidBody.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "engine/Viewport.h"
struct NJS_QUATERNION;

// 0x80013848
RigidBody::RigidBody()
{
	rigidBodySetSize(1.0f, this);
	mSpin.x   = 0.0f;
	mSpin.y   = 0.0f;
	mSpin.z   = 0.0f;
	mTorque.x = 0.0f;
	mTorque.y = 0.0f;
	mTorque.z = 0.0f;
}

// 0x800138AC
void RigidBody::vfn_0C()
{
	SetTransformBase::vfn_0C();
	mTorque.x = 0.0f;
	mTorque.y = 0.0f;
	mTorque.z = 0.0f;
}

// 0x800138E8
void rigidBodySetSize(f32 size, RigidBody* body)
{
	body->mSize    = size;
	body->mInertia = size * (0.4f * body->mRadius * size);
}

// 0x80013908
static inline Vec3f rigidScale(f32 s, const Vec3f* v)
{
	Vec3f r;
	gjScaleVector(s, &r, v);
	return r;
}

static inline Vec3f rigidSub(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjSubVector(&r, a, b);
	return r;
}

void RigidBody::vfn_10(const Vec3f* point, const Vec3f* force)
{
	Vec3f torque;
	Vec3f add = rigidScale(1.0f / mRadius, force);
	njAddVector(&mForce, &add);

	Vec3f lever = rigidSub(point, &mPos);
	njOuterProduct(&lever, force, &torque);
	njAddVector(&mTorque, &torque);
}

// 0x800139B8
void RigidBody::vfn_14()
{
	SetTransformBase::vfn_14();
	Vec3f add = rigidScale(1.0f / mInertia, &mTorque);
	njAddVector(&mSpin, &add);
}

// 0x80013A20
// q += 0.5 * spin * q then normalise
struct SpinQuat {
	f32 w;
	f32 x;
	f32 y;
	f32 z;
};

static inline SpinQuat spinScale(SpinQuat q, f32 scale)
{
	quatScale((Quat*)&q, scale);
	return q;
}

// Both helpers take the quaternion by value the DOL copies it into each one's own slot
static inline SpinQuat spinMul(SpinQuat q, const void* rot)
{
	njMultiQuaternion((const NJS_QUATERNION*)&q, (const NJS_QUATERNION*)rot, (NJS_QUATERNION*)&q);
	return q;
}

void RigidBody::vfn_18()
{
	SetTransformBase::vfn_18();
	SpinQuat turn;
	turn.w       = 0.0f;
	turn.x       = mSpin.x;
	turn.y       = mSpin.y;
	turn.z       = mSpin.z;
	SpinQuat add = spinScale(spinMul(turn, mQuat), 0.5f);
	quatAdd((Quat*)mQuat, (const Quat*)&add);
	quatNormalize((Quat*)mQuat);
}
