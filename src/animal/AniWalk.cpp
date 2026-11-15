#include "animal/AniMount.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "Ninja/ninja.h"

// 0x801C5EDC
AniWalk::AniWalk()
{
	mState     = 0;
	mTilt      = 0;
	mSpeed     = 0.0f;
	mRunSpeed  = 0.0f;
	mRunFrom   = 0.0f;
	mWalkSpeed = 0.0f;
	mAccel     = 1.0f;
	mDecel     = 1.0f;
}

// 0x801C5F44
void AniWalk::enter()
{
	mSpeed = 0.0f;
	mRotY  = getRotY();
	mState = 0;
}

// 0x801C5F84
void AniWalk::update(f32 dt)
{
	AniBody* body = mBody;
	Vec3f up;
	if (body->touching(5, 0, NULL, &up) == 0) {
		mState = 5;
		return;
	}

	f32 stick;
	int angle;
	int pushed = getStick(&stick, &angle);
	switch (mState) {
	case 0:
		if (pushed) {
			if (stick >= 0.8f) {
				mState = 2;
			} else {
				mState = 1;
			}
		}
		break;
	case 1:
		if (!pushed) {
			mState = 0;
		} else if (stick >= 0.8f) {
			mState = 2;
		}
		break;
	case 2:
		if (!pushed) {
			mState = 0;
		} else if (stick >= 0.95f && mSpeed >= mRunFrom) {
			mState = 3;
		}
		break;
	case 3:
		if (!pushed) {
			mState = 4;
		} else if (stick < 0.8f && mSpeed < mRunFrom) {
			mState = 2;
		}
		// fall through
	case 4:
		if (mSpeed < 1.0f) {
			mState = 0;
		}
		break;
	}

	switch (mState) {
	case 0:
		mSpeed = 0.0f;
		body->followGround();
		body->followTurn();
		break;
	case 1:
		mSpeed = aniLerp(mAccel / dt, mSpeed, mWalkSpeed * stick / 0.8f);
		mRotY  = aniTurn(0.2f / dt, mRotY, angle);
		face(mRotY, &up);
		body->followGround();
		break;
	case 2:
	case 3:
		mSpeed = aniLerp(mAccel / dt, mSpeed, mRunSpeed * stick);
		mRotY  = aniTurn(0.1f / dt, mRotY, angle);
		face(mRotY, &up);
		body->followGround();
		break;
	case 4:
		mSpeed = aniLerp(mDecel / dt, mSpeed, 0.0f);
		body->followGround();
		break;
	}

	// Head along the ground the heading with the part along the ground's normal taken out
	Vec3f vel;
	vecFromAngleXZ(1.0f, &vel, mRotY);
	vecScaleAdd(-njInnerProduct(&vel, &up), &vel, &up, &vel);
	vecNormalize(&vel, &vel);
	vecScale(mSpeed, &vel, &vel);
	body->mVel = vel;
}

// 0x801C62B8
int AniWalk::state()
{
	switch (mState) {
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
		return 2;
	case 3:
	case 4:
		return 3;
	case 5:
		break;
	}
	return 4;
}

// 0x801C6314
void AniWalk::face(int rotY, const Vec3f* up)
{
	if (mTilt == 0) {
		setRotY(rotY);
		return;
	}

	// A frame from the heading and the ground's normal so the animal leans with the slope
	Vec3f side;
	Vec3f fwd;
	vecFromAngleXZ(1.0f, &fwd, mRotY);
	njOuterProduct(up, &fwd, &side);
	vecNormalize(&side, &side);
	njOuterProduct(&side, up, &fwd);
	vecNormalize(&fwd, &fwd);

	f32 mtx[12];
	mtxIdentity(mtx);
	mtx[0]  = side.x;
	mtx[4]  = side.y;
	mtx[8]  = side.z;
	mtx[1]  = up->x;
	mtx[5]  = up->y;
	mtx[9]  = up->z;
	mtx[2]  = fwd.x;
	mtx[6]  = fwd.y;
	mtx[10] = fwd.z;

	Quat quat;
	mtxToQuat((const f32*)mtx, (f32*)&quat);
	mBody->mQuat = quat;
}
