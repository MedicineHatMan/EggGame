#include "stage/SetManager.h"
#include "stage/SetTransform.h"
#include "engine/RigidBody.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
struct NJS_QUATERNION;

// 0x8000E7EC
SetTransformBase::SetTransformBase()
{
	mRadius  = 1.0f;
	mPos.x   = 0.0f;
	mPos.y   = 0.0f;
	mPos.z   = 0.0f;
	mVel.x   = 0.0f;
	mVel.y   = 0.0f;
	mVel.z   = 0.0f;
	mForce.x = 0.0f;
	mForce.y = 0.0f;
	mForce.z = 0.0f;
	mGravity = gVecZero;
}

// 0x8000E848
void SetTransformBase::vfn_0C()
{
	mForce = mGravity;
}

// 0x8000E864
// Here mRadius is the mass
inline Vec3f scaledBy(f32 scale, const Vec3f* v)
{
	Vec3f r;
	gjScaleVector(scale, &r, v);
	return r;
}

void transAddForce(SetTransformBase* body, const Vec3f* force)
{
	Vec3f copy = scaledBy(1.0f / body->mRadius, force);
	njAddVector(&body->mForce, &copy);
}

// 0x8000E8C4
void transAddDrag(f32 drag, SetTransformBase* body)
{
	Vec3f copy = scaledBy(drag, &body->mVel);
	njSubVector(&body->mForce, &copy);
}

// 0x8000E91C
void SetTransformBase::vfn_14()
{
	njAddVector(&mVel, &mForce);
}

// 0x8000E948
void SetTransformBase::vfn_18()
{
	njAddVector(&mPos, &mVel);
}

// 0x8000E974
SetTransform::SetTransform()
{
	mRot.x = 0;
	mRot.y = 0;
	mRot.z = 0;
}

// 0x8000EA08
void SetTransform::vfn_1C(const SetRot* rot)
{
	mRot = *rot;
}

// 0x8000EA24
void SetTransform::vfn_20()
{
	njRotateEx((const s32*)&mRot, (s32)0);
}

// 0x8000EA4C
QuatTransform::QuatTransform()
{
	njCreateQuaternion(0, 0.0f, 0.0f, 1.0f, (NJS_QUATERNION*)mQuat);
}

// 0x8000EAA0
void QuatTransform::vfn_1C(const SetRot* rot)
{
	njXYZAngleToQuaternion((const s32*)rot, (NJS_QUATERNION*)mQuat);
}

// 0x8000EACC
void QuatTransform::vfn_20()
{
	njQuaternionEx((const NJS_QUATERNION*)mQuat);
}

// Unused and stripped by the linker
static void dropQuat(QuatTransform* transform)
{
	delete transform;
}

// 0x8000EB4C
// The base ignores where the force lands
void SetTransformBase::vfn_10(const Vec3f* point, const Vec3f* force)
{
	transAddForce(this, force);
}
