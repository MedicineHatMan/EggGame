#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "player/Player.h"
#include "Ninja/ninja.h"

// 0x801CBA98
AniDive::AniDive()
{
	mState     = -1;
	mDrag      = 1.0f;
	mSpeed     = 10.0f;
	mStopSpeed = 0.0f;
}

// 0x801CBAF4
void AniDive::enter()
{
	mRotY = getRotY();
	setState(3);
}

// 0x801CBB30
int AniDive::resume()
{
	if (mState == 0) {
		setState(3);
		return 1;
	}
	return 0;
}

// 0x801CBB6C
void AniDive::update(f32 dt)
{
	runState(mState, 2);
	runState(mState, 3);
}

// 0x801CBBB8
int AniDive::state() const
{
	Vec3f vel;
	switch (mState) {
	case 0:
		vel = mBody->mVel;
		return !(vecLength(&vel) > mSpeed) ? 1 : 0;
	case 3:
		return 0;
	case 1:
		return 3;
	case 2:
		return 2;
	case 4:
	default:
		return 4;
	}
}

// 0x801CBC70
void AniDive::heading(f32 speed, Vec3f* out, int angle)
{
	Vec3f dir;
	Vec3f up;
	if (mBody->touching(5, 0, NULL, &up) == 0) {
		vecFromAngleXZ(speed, out, angle);
	} else {
		vecFromAngleXZ(1.0f, &dir, angle);
		vecScaleAdd(-njInnerProduct(&dir, &up), &dir, &up, &dir);
		vecNormalize(&dir, &dir);
		vecScale(speed, out, &dir);
	}
}

// 0x801CBD30
void AniDive::drift(f32 drag, const Vec3f* accel)
{
	AniBody* body = mBody;
	Vec3f vel;
	vel = body->mVel;
	vecScaleAdd(1.0f / 60.0f, &vel, accel, &vel);
	vecScale(drag, &vel, &vel);
	body->mVel = vel;
	body->followGround();
}

// 0x801CBDC4
int AniDive::slowed()
{
	Vec3f vel;
	vel = mBody->mVel;
	if (vecLength(&vel) > mStopSpeed) {
		return 0;
	}
	setState(2);
	return 1;
}

// 0x801CBE34
int AniDive::fellOff()
{
	if (mBody->touching(5, 0, NULL, NULL) != 0) {
		return 0;
	}
	setState(4);
	return 1;
}

// 0x801CBE94
int AniDive::hitWall()
{
	bool hit      = false;
	AniBody* body = mBody;
	if (body->touching(0, 0, NULL, NULL) != 0 || body->touching(1, 0, NULL, NULL) != 0 || body->touching(2, 0, NULL, NULL) != 0
	    || body->touching(3, 0, NULL, NULL) != 0) {
		hit = true;
	}
	return hit;
}

// 0x801CBF48
void AniDive::setState(int state)
{
	if (mState != -1) {
		runState(mState, 1);
	}
	mState = state;
	if (state != -1) {
		runState(state, 0);
	}
}

// 0x801CBFA8
void AniDive::runState(int state, int phase)
{
	switch (state) {
	case 3:
		runStart(phase);
		break;
	case 0:
		runSwim(phase);
		break;
	case 1:
		runBounce(phase);
		break;
	case 2:
		runStop(phase);
		break;
	}
}

// 0x801CC014
void AniDive::runStart(int phase)
{
	if (phase == 0) {
		mTimer = 20;
		mSwim  = 0.0f;
	} else if (phase == 2) {
		if (fellOff() == 0 && mTimer == 0) {
			setState(0);
		}
	} else if (phase == 3) {
		if (mTimer != 0) {
			mTimer--;
		}
		mSwim = aniLerp(29.999998f, mSwim, 500.0f);
		Vec3f vel;
		heading(mSwim, &vel, mRotY);
		drift(mDrag, &vel);
	}
}

// 0x801CC0DC
void AniDive::runSwim(int phase)
{
	if (phase == 2) {
		if (fellOff() == 0 && slowed() == 0 && hitWall()) {
			setState(1);
		}
	} else if (phase == 3) {
		f32 mtx[12];
		Vec3f up;
		Vec3f vel;
		Vec3f fwd;
		Vec3f side;
		Quat rot;
		int angle;

		if (mBody->touching(5, 0, NULL, &up) == 0) {
			up.x = 0.0f;
			up.y = 1.0f;
			up.z = 0.0f;
		}
		if (getStick(NULL, &angle) != 0) {
			mRotY = angleApproach(mRotY, angle, 0x80);
			vecFromAngleXZ(1.0f, &fwd, mRotY);
			njOuterProduct(&up, &fwd, &side);
			vecNormalize(&side, &side);
			njOuterProduct(&side, &up, &fwd);
			vecNormalize(&fwd, &fwd);
			mtxIdentity(mtx);
			mtx[0]  = side.x;
			mtx[4]  = side.y;
			mtx[8]  = side.z;
			mtx[1]  = up.x;
			mtx[5]  = up.y;
			mtx[9]  = up.z;
			mtx[2]  = fwd.x;
			mtx[6]  = fwd.y;
			mtx[10] = fwd.z;
			mtxToQuat((const f32*)mtx, (f32*)&rot);
			mBody->mQuat = rot;

			vel       = mBody->mVel;
			f32 speed = vecLength(&vel);
			mtxMultVecSR(mtx, (Vec3f*)&gAxisZ, &vel);
			vecScale(speed, &vel, &vel);
			mBody->mVel = vel;
		}
		drift(mDrag, &gVecZero);
	}
}

// 0x801CC304
void AniDive::runBounce(int phase)
{
	if (phase == 0) {
		Vec3f vel;
		vel = mBody->mVel;
		vecScale(-0.8f, &vel, &vel);
		mBody->mVel = vel;
	} else if (phase == 2) {
		if (fellOff() != 0) {
			return;
		}
		if (slowed() != 0) {
			return;
		}
	} else if (phase == 3) {
		drift(0.8f, &gVecZero);
	}
}

// 0x801CC3C0
void AniDive::runStop(int phase)
{
	if (phase == 2) {
		if (fellOff() != 0) {
			return;
		}
	}
}
