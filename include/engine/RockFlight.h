#ifndef _ROCKFLIGHT_H
#define _ROCKFLIGHT_H

#include "engine/RigidBody.h"

class TObject;

/**
 * Carries a thrown object through the air: a spinning body that is pushed by what it touches, has its speed
 * capped, and bounces and rolls off the ground meshes. While it rolls fast on the ground it keeps a dust effect
 * under it. Its code is 0x80041568-0x80041E48. TEneAm02_Rock holds one, and TEneAm01 and TEneAm02 allocate one when
 * they are thrown. It has no vtable of its own: the one at _28 is the body's. The name is ours.
 *
 * @size 0x74
 */
struct RockFlight {
	RockFlight();                                   // 0x80041568
	RockFlight(f32 size, const SetTransform* from); // 0x800415A0 starts where from is facing the same way
	~RockFlight();                                  // 0x80041620

	void init();                    // 0x800416B4 sets gravity the radius mMaxSpeed mFriction and mDust
	void setPos(const Vec3f* pos);  // 0x800416EC
	void setVel(const Vec3f* vel);  // 0x80041708
	void setRot(const SetRot* rot); // 0x80041724 through the body's vfn_1C
	void setSize(f32 size);         // 0x80041750 through rigidBodySetSize

	f32 clampSpeed(f32 max);           // 0x80041770 returns the speed capped
	void addForce(const Vec3f* force); // 0x80041858
	void step();                       // 0x8004187C the body's vfn_14
	void push(void* colli);            // 0x800418A8 a tenth of each contact's push
	int collide(f32 friction);         // 0x80041B1C against the ground meshes
	void setGroundFriction();          // 0x80041D78
	void update(void* colli);          // 0x80041D84 once a frame

	RigidBody mBody; // _00
	f32 mMaxSpeed;   // _68
	f32 mFriction;   // _6C taken off the speed along the ground on each hit
	TObject* mDust;  // _70 the destructor sets bit 1 of its mFlags and lets go of it
};

#endif
