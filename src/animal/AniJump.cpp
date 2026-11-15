#include "animal/AniMount.h"
#include "animal/TAniCamelOstrich.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"

// 0x801CB714
AniJump::AniJump()
{
	mState      = 1;
	mGravity    = 0.0f;
	mAirControl = 90.0f;
}

// 0x801CB768
void AniJump::enter()
{
	mState = 0;
	mRotY  = getRotY();
	flagClear(&mBody->mFlags, 0);
}

// 0x801CB7B0
void AniJump::launch()
{
	f32 stick;
	int angle;
	Vec3f vel;
	if (getStick(&stick, &angle) == 0) {
		vel.y = mJumpSpeed;
		vel.x = 0.0f;
		vel.z = 0.0f;
	} else {
		vecFromAngleXZ(mAirSpeed, &vel, angle);
		vel.y = mJumpSpeed;
		mRotY = angle;
		setRotY(angle);
	}
	mBody->mVel = vel;
	mState      = 2;
}

// 0x801CB854
void AniJump::update(f32 dt)
{
	Vec3f vel;
	Vec3f accel;
	Vec3f up;
	Vec3f add;
	f32 stick;
	int angle;

	bool onFlat   = false;
	AniBody* body = mBody;
	if (body->touching(5, 0, NULL, &up) != 0 && up.y > 0.5f) {
		onFlat = true;
	}
	int flat = onFlat;
	vel      = body->mVel;

	switch (mState) {
	case 0:
		if (vel.y <= 0.0f && flat) {
			mState = 4;
		}
		break;
	case 2:
		if (vel.y <= 0.0f) {
			if (flat) {
				mState = 4;
			} else {
				mState = 3;
			}
		}
		break;
	case 3:
		if (flat) {
			mState = 4;
		}
		break;
	}

	int pushed = getStick(&stick, &angle);
	accel.y    = mGravity;
	accel.x    = 0.0f;
	accel.z    = 0.0f;
	if (mState == 2 && pushed != 0) {
		vecFromAngleXZ(mAirControl * stick, &add, angle);
		vecAdd(&accel, &add, &accel);
	}
	accel.x -= 60.0f * (0.02f * vel.x);
	accel.y -= 60.0f * (0.0001f * vel.y);
	accel.z -= 60.0f * (0.02f * vel.z);
	vecScaleAdd(1.0f / 60.0f, &vel, &accel, &vel);
	if (vel.y < -300.0f) {
		vel.y = -300.0f;
	}
	body->mVel = vel;
}

// 0x801CBA4C
int AniJump::state() const
{
	switch (mState) {
	case 0:
	case 1:
		return 2;
	case 2:
		return 0;
	case 3:
		return 1;
	case 4:
	default:
		return 3;
	}
}
