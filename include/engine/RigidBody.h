#ifndef _RIGIDBODY_H
#define _RIGIDBODY_H

#include "stage/SetManager.h"

struct QuatTransform : public SetTransformBase {
	QuatTransform();
	inline virtual ~QuatTransform() { } // _08
	virtual void vfn_1C(const SetRot*); // _1C
	virtual void vfn_20();              // _20

	f32 mQuat[4]; // _38
};

/** @size 0x68 */
struct RigidBody : public QuatTransform {
	RigidBody();
	inline virtual ~RigidBody() { }                              // _08
	virtual void vfn_0C();                                       // _0C
	virtual void vfn_10(const Vec3f* point, const Vec3f* force); // _10
	virtual void vfn_14();                                       // _14
	virtual void vfn_18();                                       // _18

	Vec3f mSpin;   // _48
	f32 mSize;     // _54
	f32 mInertia;  // _58
	Vec3f mTorque; // _5C
};

extern "C" void rigidBodySetSize(f32, RigidBody*);

#endif
