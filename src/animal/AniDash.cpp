#include "animal/AniMount.h"
#include "animal/TAniCamelOstrich.h"
#include "core/MathUtil.h"

// 0x801CB4A0
AniDash::AniDash()
{
	mState = 0;
	mBrake = 0.0f;
	mSpeed = 200.0f;
}

// 0x801CB4F4
void AniDash::enter()
{
	mState = 0;
	mRotY  = getRotY();
	flagClear(&mBody->mFlags, 0);
}

// 0x801CB53C
void AniDash::update(f32 dt)
{
	AniBody* body = mBody;

	switch (mState) {
	case 0:
		if (body->touching(5, 0, NULL, NULL) != 0) {
			mState = 2;
		} else if (body->touching(0, 0, NULL, NULL) != 0 || body->touching(1, 0, NULL, NULL) != 0 || body->touching(3, 0, NULL, NULL) != 0
		           || body->touching(2, 0, NULL, NULL) != 0 || body->touching(4, 0, NULL, NULL) != 0) {
			mState = 1;
		}
		break;
	}

	if (mState == 0) {
		int angle;
		Vec3f vel;
		vel = body->mVel;
		if (getStick(NULL, &angle) != 0) {
			mRotY = angleApproach(mRotY, angle, 0x100);
			setRotY(mRotY);
		}
		vecFromAngleXZ(mSpeed, &vel, mRotY);
		vel.y      = mBrake;
		body->mVel = vel;
	}
}

// 0x801CB6D8
int AniDash::state()
{
	switch (mState) {
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
	default:
		return 2;
	}
}
