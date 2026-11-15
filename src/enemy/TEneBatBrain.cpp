#include "enemy/TEneBat.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "enemy/EnemyFx.h"

extern "C" {
}

// -1 to 1 from the game's random number
static inline f32 randomSigned()
{
	return 2.0f * (3.0517578e-5f * rand() - 0.5f);
}

// 0 to 1 from the game's random number
static inline f32 random01()
{
	return 3.0517578e-5f * rand();
}

// 0x800D8EA4
void BatBrain::update(TEneBat* owner)
{
	mOwner = owner;
	mNext  = -1;
	if (checkInterrupt() == 0) {
		onPhase(mState, 2);
	}
	onPhase(mState, 1);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x800D8F3C
BatBrainBase::BatBrainBase()
{
	mOwner = 0;
}

// 0x800D8F70
BatBrainBase::~BatBrainBase()
{
}

// 0x800D8FCC
BatHuntBrain* BatHuntBrain::create(TEneBat* owner)
{
	return new BatHuntBrain(owner);
}

// 0x800D9014
BatHuntBrain::BatHuntBrain(TEneBat* owner)
{
	mSight.reset();
	mMemory.reset();
	mTimer   = 0;
	mResting = 0;
	mOwner   = owner;
	mNext    = -1;
	mState   = STATE_APPEAR;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x800D90CC
BatHuntBrain::~BatHuntBrain()
{
}

// 0x800D9134
// A hard hit flips it low in a dive and stuns it otherwise
int BatHuntBrain::checkInterrupt()
{
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		return 1;
	}
	if (mOwner->wasHitHard() != 0) {
		if (mOwner->canFlip() != 0) {
			changeState(STATE_FLIP);
		} else {
			changeState(STATE_STUN);
		}
		mMemory.setChase(mOwner->getHitPlayer());
	}
	return 0;
}

// 0x800D9278
// A random point 25 to 80 from home 20 either way of 40 over the ground under it
void BatHuntBrain::pickTarget()
{
	Vec3f home;
	Vec3f target;
	Vec3f ground;
	home = mOwner->mHome;
	randomPointInRing(25.0f, 80.0f, &home, &target);
	if (probeGround(&target, &ground, 0) != 0) {
		target.y = 20.0f * randomSigned() + (40.0f + ground.y);
	} else {
		target.y += 25.0f;
	}
	mOwner->setTarget(&target);
}

// 0x800D9354
// Follows the player it remembers or looks 240 ahead in a cone
int BatHuntBrain::look()
{
	int seen = 0;
	mMemory.tick();
	if (mMemory.isReady() != 0) {
		if (mMemory.isChasing() != 0) {
			seen = mSight.track(mMemory.getPlayer(), mOwner->getPos(), mOwner->getRotY());
		} else {
			seen = mSight.findInCone(57600.0f, mOwner->getPos(), mOwner->getRotY(), 0xF8E3);
		}
	}
	return seen;
}

// 0x800D9464
void BatHuntBrain::aim()
{
	mOwner->setAim(&mSight.mTarget, mSight.mAngle);
}

// 0x800D9494
// Slot _0C
void BatHuntBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_WANDER:
		phaseWander(phase);
		break;
	case STATE_RISE:
		phaseRise(phase);
		break;
	case STATE_DIVE:
		phaseDive(phase);
		break;
	case STATE_STUN:
		phaseStun(phase);
		break;
	case STATE_FLIP:
		phaseFlip(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x800D9520
void BatHuntBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneBat::ACT_APPEAR;
		break;
	case 2:
		if (mOwner->isAppearDone() != 0) {
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		break;
	case 3:
		break;
	}
}

// 0x800D95C0
// Flies between random points resting on arrival
void BatHuntBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		pickTarget();
		mTimer   = randomInt(300.0f, 120.0f);
		mNext    = TEneBat::ACT_FLY;
		mResting = 0;
		break;
	case 2:
		if (look() != 0) {
			if (mSight.mDistSq <= 52900.0f || mMemory.isChasing() != 0) {
				mOwner->stopEffect();
				changeState(STATE_RISE);
			}
		}
		break;
	case 1:
		mTimer--;
		if (mTimer < 0) {
			pickTarget();
			mTimer = randomInt(300.0f, 120.0f);
			if (mResting != 0) {
				mResting = 0;
				mNext    = TEneBat::ACT_FLY;
			}
		} else if (mOwner->hasArrived() != 0) {
			mResting = 1;
			mNext    = TEneBat::ACT_HOVER;
		}
		break;
	case 3:
		break;
	}
}

// 0x800D9714
// Rises over the player then waits there
void BatHuntBrain::phaseRise(int phase)
{
	switch (phase) {
	case 0:
		aim();
		mNext  = TEneBat::ACT_RISE;
		mTimer = 0;
		break;
	case 2:
		if (look() != 0) {
			if (mTimer == 1) {
				if (mSight.mDistSq <= 36100.0f || mMemory.isChasing() != 0) {
					changeState(STATE_DIVE);
				}
			}
		} else if (mTimer == 1) {
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		aim();
		switch (mTimer) {
		case 0:
			if (mOwner->isRiseDone() != 0) {
				mTimer = 1;
				mNext  = TEneBat::ACT_AIM;
			}
			break;
		case 1:
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x800D9884
void BatHuntBrain::phaseDive(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneBat::ACT_DIVE;
		break;
	case 2:
		look();
		if (mOwner->isDiveDone() != 0) {
			mMemory.startCooldown();
			mMemory.clearChase();
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		aim();
		break;
	case 3:
		break;
	}
}

// 0x800D9944
void BatHuntBrain::phaseFlip(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneBat::ACT_FLIP;
		break;
	case 2:
		if (mOwner->isFlipDone() != 0) {
			look();
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		break;
	case 3:
		break;
	}
}

// 0x800D99EC
void BatHuntBrain::phaseStun(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneBat::ACT_STUN;
		break;
	case 2:
		if (mOwner->isStunDone() != 0) {
			look();
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		break;
	case 3:
		break;
	}
}

// 0x800D9A94
void BatHuntBrain::phaseDie(int phase)
{
	switch (phase) {
	case 0:
		mNext = mOwner->deathAction();
		break;
	case 1:
		break;
	case 2:
		break;
	}
}
