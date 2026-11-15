#ifndef _SPRINGBODY_H
#define _SPRINGBODY_H

#include "core/types.h"
#include "stage/Stage.h"

/**
 * @size 0x68
 */
struct SpringPoint {
	Vec3f mPos;            // _00
	Vec3f mVel;            // _0C
	Vec3f mForce;          // _18 from the springs this step
	SpringPoint* mLink[8]; // _24
	f32 mRest[8];          // _44 each link's rest length
	u16 mLinkCount;        // _64
	u8 _66[2];
};

/**
 * A point held in place.
 *
 * @size 0x10
 */
struct SpringPin {
	Vec3f mPos;          // _00
	SpringPoint* mPoint; // _0C
};

/**
 * Vtable 0x80258BD0.
 *
 * @size 0x50
 */
class SpringBody {
public:
	SpringBody(u16 count, u16 pins, int flags, f32 spring, f32 loss, f32 param); // 0x800A8FA4

	void init(u16 count, u16 pins, int flags, f32 spring, f32 loss, f32 param); // 0x800A8FE0
	static SpringPoint* allocPoints(int count);                                 // 0x800A9118
	static SpringPin* allocPins(int count);                                     // 0x800A9140
	static void freeArray(void* array);                                         // 0x800A9168
	void addSprings();                                                          // 0x800A91EC
	void move();                                                                // 0x800A9320
	void holdPins();                                                            // 0x800A93F4
	int addPin(SpringPoint* point);                                             // 0x800A9558 0 when full
	void setSphere(const Vec3f* pos, Vec3f* vel, f32 radius);                   // 0x800A95C0

	SpringPoint* mPoints; // _00
	u16 mCount;           // _04
	u16 mCapacity;        // _06
	SpringPin* mPins;     // _08
	u16 mPinCount;        // _0C
	u16 mPinCapacity;     // _0E
	f32 mSpring;          // _10
	f32 mLoss;            // _14 what of the speed is kept each step
	f32 mParam;           // _18 set by the constructor
	Vec3f mWind;          // _1C added to every point
	int mReact;           // _28 set when the sphere shoved a point
	Vec3f mSpherePos;     // _2C
	f32 mSphereRadius;    // _38
	Vec3f mSphereVel;     // _3C given to the points it shoves
	int mHit;             // _48 bit 0 test the sphere

	// _4C = vtable
	virtual void getForce(u16 index, Vec3f* out); // _08 0x800A93E0
	virtual void collide();                       // _0C 0x800A943C
	virtual ~SpringBody();                        // _10 0x800A90B8
	virtual void step();                          // _14 0x800A9188
	virtual void reset();                         // _18 0x800A9514
};

/**
 * A chain of points between two ends, both pinned. The hanging bridge makes one. Vtable 0x80258BB4.
 *
 * @size 0x54
 */
class SpringRope : public SpringBody {
public:
	SpringRope(Vec3f* from, const Vec3f* to, u16 segments, f32 spring, f32 loss, f32 param); // 0x800A95F8

	virtual void getForce(u16 index, Vec3f* out); // _08 0x800A9888
	virtual ~SpringRope();                        // _10 0x800A97F0
	virtual void step();                          // _14 0x800A9848
	virtual void reset();                         // _18 0x800A9868

	Vec3f* mForces; // _50 one per point set by the owner
};

typedef char SpringBody_size_check[(sizeof(SpringBody) == 0x50) ? 1 : -1];
typedef char SpringRope_size_check[(sizeof(SpringRope) == 0x54) ? 1 : -1];

#endif
