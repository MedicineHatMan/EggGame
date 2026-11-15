#include "enemy/TEneAm01.h"
#include "enemy/EnemyBrainDtors.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"

#define random01() (3.0517578e-5f * rand())

// 0x80049E04
EnemySeekBrain::EnemySeekBrain()
{
	mSight.reset();
	mMemory.reset();
	mStep  = 0;
	mTimer = 0;
}

// 0x80049E94
// Slot _08 Death then a hit that flips it take over from whatever the brain is doing
int EnemySeekBrain::checkInterrupt()
{
	if (mOwner->isDead() != 0) {
		if (mState != STATE_DIE) {
			changeState(STATE_DIE);
			return 1;
		}
	} else if (mOwner->wasHitKind3() != 0) {
		changeState(STATE_FLIP);
		mMemory.setChase(mOwner->getHitPlayer());
		return 1;
	}
	return 0;
}

// 0x80049F8C
// Slot _20
void EnemySeekBrain::onDeathStart()
{
}

// 0x80049F90
void EnemySeekBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm01::ACT_APPEAR;
		break;
	case 1:
		break;
	case 2:
		if (mOwner->isAppearDone() != 0) {
			onAppearDone();
		}
		break;
	case 3:
		break;
	}
}

// 0x8004A004
void EnemySeekBrain::phaseFlip(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm01::ACT_FLIP;
		break;
	case 1:
		break;
	case 2:
		if (mOwner->isFlipDone() != 0) {
			onFlipDone();
		}
		break;
	case 3:
		break;
	}
}

// 0x8004A078
void EnemySeekBrain::phaseDie(int phase)
{
	switch (phase) {
	case 0:
		mNext = mOwner->pickDeath();
		onDeathStart();
		break;
	case 1:
		break;
	case 2:
		break;
	}
}

// 0x8004A0D0
Am01WanderBrain::Am01WanderBrain()
{
}

// 0x8004A10C
Am01WanderBrain::~Am01WanderBrain()
{
}

// 0x8004A18C
// Slot _24
void Am01WanderBrain::attach(TEneAm01* owner)
{
	mOwner = owner;
	mNext  = -1;
	mState = STATE_APPEAR;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x8004A1F4
// A random point 25 to 50 from home
void Am01WanderBrain::pickTarget()
{
	f32 dist = 25.0f * random01() + 25.0f;
	Vec3f target;
	njSinCos((int)(65536.0f * random01()), &target.x, &target.z);
	target.y = 0.0f;
	gjScaleVector(dist, &target, &target);
	njAddVector(&target, &mOwner->mHome);
	mOwner->setTarget(&target);
}

// 0x8004A2D8
// Keeps track of a player it was hit by or else looks for one ahead
int Am01WanderBrain::look()
{
	int seen = 0;
	mMemory.tick();
	if (mMemory.isReady() != 0) {
		if (mMemory.isChasing() != 0) {
			Vec3f* pos;
			seen = mSight.track(mMemory.getPlayer(), mOwner->getPos(), mOwner->getRotY());
		} else {
			seen = mSight.search(10000.0f, 900.0f, mOwner->getPos(), mOwner->getRotY(), 0x38E3, 0xD555);
		}
	}
	return seen;
}

// 0x8004A3F0
// Slot _0C
void Am01WanderBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_WANDER:
		phaseWander(phase);
		break;
	case STATE_CHASE:
		phaseChase(phase);
		break;
	case STATE_FLIP:
		phaseFlip(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x8004A478
// Slot _18
void Am01WanderBrain::onAppearDone()
{
	changeState(STATE_WANDER);
}

// 0x8004A4DC
// Slot _1C
void Am01WanderBrain::onFlipDone()
{
	changeState(STATE_WANDER);
}

// 0x8004A540
// mStep 0 idling for mTimer frames 1 walking for up to 600 2 saw a player
void Am01WanderBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		mStep  = 1;
		mTimer = 0;
		pickTarget();
		mOwner->mTopSpeed = 0.3f;
		mNext             = TEneAm01::ACT_WALK;
		break;
	case 2:
		if (mStep == 2) {
			changeState(STATE_CHASE);
		}
		break;
	case 1:
		if (look() != 0) {
			mStep = 2;
			break;
		}
		switch (mStep) {
		case 1:
			mTimer++;
			if (mOwner->hasArrived() != 0 || mTimer >= 600) {
				if (random01() < 0.2f) {
					mTimer = (int)(420.0f * random01() + 180.0f);
					mStep  = 0;
					mNext  = TEneAm01::ACT_IDLE;
				} else {
					mTimer = 0;
					pickTarget();
				}
			}
			break;
		case 0:
			mTimer--;
			if (mTimer <= 0) {
				mTimer = 0;
				mStep  = 1;
				pickTarget();
				mOwner->mTopSpeed = 0.3f;
				mNext             = TEneAm01::ACT_WALK;
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8004A740
// mStep 0 chasing 1 lost the player or touched it
void Am01WanderBrain::phaseChase(int phase)
{
	switch (phase) {
	case 0:
		mStep             = 0;
		mOwner->mTopSpeed = 0.5f;
		mNext             = TEneAm01::ACT_CHASE;
		mOwner->mBody.mVel.y += 1.0f;
		mOwner->startEffect();
		break;
	case 2:
		if (mStep == 1) {
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		if (look() == 0) {
			mStep = 1;
			break;
		}
		if (mOwner->wasTouched() != 0) {
			mStep = 1;
			mMemory.startCooldown();
		}
		mOwner->setTarget(&mSight.mTarget);
		break;
	case 3:
		break;
	}
}

// 0x8004A854
Am01LeapBrain::Am01LeapBrain()
{
}

// 0x8004A890
Am01LeapBrain::~Am01LeapBrain()
{
}

// 0x8004A910
// Slot _24
void Am01LeapBrain::attach(TEneAm01* owner)
{
	mOwner = owner;
	mNext  = -1;
	mState = STATE_APPEAR;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x8004A978
// A random point 25 to 50 from home
void Am01LeapBrain::pickTarget()
{
	f32 dist = 25.0f * random01() + 25.0f;
	Vec3f target;
	njSinCos((int)(65536.0f * random01()), &target.x, &target.z);
	target.y = 0.0f;
	gjScaleVector(dist, &target, &target);
	njAddVector(&target, &mOwner->mHome);
	mOwner->setTarget(&target);
}

// 0x8004AA5C
// The same as Am01WanderBrain's
int Am01LeapBrain::look()
{
	int seen = 0;
	mMemory.tick();
	if (mMemory.isReady() != 0) {
		if (mMemory.isChasing() != 0) {
			Vec3f* pos;
			seen = mSight.track(mMemory.getPlayer(), mOwner->getPos(), mOwner->getRotY());
		} else {
			seen = mSight.search(10000.0f, 900.0f, mOwner->getPos(), mOwner->getRotY(), 0x38E3, 0xD555);
		}
	}
	return seen;
}

// 0x8004AB74
// Slot _0C
void Am01LeapBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_FLIP:
		phaseFlip(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	case STATE_WANDER:
		phaseLeap(phase);
		break;
	}
}

// 0x8004ABE4
// Slot _18
void Am01LeapBrain::onAppearDone()
{
	changeState(STATE_WANDER);
}

// 0x8004AC48
// Slot _1C
void Am01LeapBrain::onFlipDone()
{
	changeState(STATE_WANDER);
}

// 0x8004ACAC
// Slot _28 Hops around at random
void Am01LeapBrain::startWander()
{
	pickTarget();
	mOwner->mTopSpeed = 0.7f;
	mOwner->clearLeapAimed();
}

// 0x8004ACEC
// Leaps land on the target from here on
void Am01LeapBrain::startAiming()
{
	mOwner->setLeapAimed();
	mOwner->mTopSpeed = 0.7f;
}

// 0x8004AD28
// All it does is leap with the target changing
void Am01LeapBrain::phaseLeap(int phase)
{
	switch (phase) {
	case 0:
		mStep  = 0;
		mTimer = 0;
		startWander();
		mNext = TEneAm01::ACT_LEAP;
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (look() != 0) {
				mStep = 1;
				startAiming();
			}
			mTimer++;
			if (mOwner->hasArrived() != 0 || mTimer >= 600) {
				mTimer = 0;
				pickTarget();
			}
			break;
		case 1: {
			int lost = look() == 0;
			if (mOwner->wasTouched() != 0) {
				mMemory.startCooldown();
				lost = 1;
			}
			if (lost != 0) {
				mStep  = 0;
				mTimer = 0;
				startWander();
			}
			mOwner->setTarget(&mSight.mTarget);
			break;
		}
		}
		break;
	case 2:
		break;
	}
}
