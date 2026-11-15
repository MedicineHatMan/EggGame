#include "animal/AniMount.h"
#include "animal/TAniCamelOstrich.h"

// 0x801CB2E4
AniSlam::AniSlam()
{
	mState    = 0;
	mMaxFall  = 300.0f;
	mFall     = 0.0f;
	mFallRate = 1.0f;
}

// 0x801CB340
void AniSlam::enter()
{
	mState = 0;
	flagClear(&mBody->mFlags, 0);
}

// 0x801CB374
void AniSlam::update(f32 dt)
{
	AniBody* body = mBody;
	int ground = (bool)body->touching(5, 0, NULL, NULL);
	f32 x = body->mVel.x;
	f32 y = body->mVel.y;
	f32 z = body->mVel.z;

	switch (mState) {
	case 0:
		if (y <= 0.0f && ground != 0) {
			mState = 1;
			y      = 0.0f;
		}
		break;
	}

	if (mState == 0) {
		mFall = aniLerp(mFallRate / dt, mFall, mMaxFall);
		y     = -mFall;
	}
	body->mVel.x = x;
	body->mVel.y = y;
	body->mVel.z = z;
}

// 0x801CB470
int AniSlam::state()
{
	switch (mState) {
	case 0:
		return 0;
	case 1:
	default:
		return 1;
	}
}
