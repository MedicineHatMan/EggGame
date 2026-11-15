#include "animal/AniMount.h"
#include "animal/TAniCamelOstrich.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "Ninja/ninja.h"

// 0x801CAF7C
AniBlast::AniBlast()
{
	mState = 0;
	mPush  = 0.0f;
	mSpeed = 0.0f;
	mDrag  = 1.0f;
}

// 0x801CAFD4
void AniBlast::enter()
{
	mSpeed = 0.0f;
	mRotY  = getRotY();
	mState = 0;
	flagClear(&mBody->mFlags, 1);
}

// 0x801CB024
void AniBlast::update(f32 dt)
{
	AniBody* body = mBody;
	Vec3f up;

	switch (mState) {
	case 0:
		if (body->touching(5, 0, NULL, &up) == 0) {
			mState = 3;
		} else if (body->touching(0, 0, NULL, NULL) != 0 || body->touching(1, 0, NULL, NULL) != 0
		           || body->touching(2, 0, NULL, NULL) != 0 || body->touching(3, 0, NULL, NULL) != 0) {
			mState = 1;
		}
		break;
	case 1:
		if (body->touching(5, 0, NULL, &up) == 0) {
			mState = 3;
		} else if (mSpeed < 1.0f) {
			mState = 2;
		}
		break;
	}

	switch (mState) {
	case 0:
		mSpeed = aniLerp(mDrag / dt, mSpeed, mPush);
		body->followGround();
		push(mRotY, &up);
		break;
	case 1:
		mSpeed = aniLerp(0.2f / dt, mSpeed, 0.0f);
		body->followGround();
		push(mRotY + 0x8000, &up);
		break;
	}
}

// 0x801CB20C
int AniBlast::state()
{
	switch (mState) {
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
		return 2;
	case 3:
	default:
		return 3;
	}
}

// 0x801CB254
void AniBlast::push(int angle, const Vec3f* up)
{
	AniBody* body = mBody;
	Vec3f dir;
	vecFromAngleXZ(1.0f, &dir, angle);
	vecScaleAdd(-njInnerProduct(&dir, up), &dir, up, &dir);
	vecNormalize(&dir, &dir);
	vecScale(mSpeed, &dir, &dir);
	body->mVel = dir;
}
