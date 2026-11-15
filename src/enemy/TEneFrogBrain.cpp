#include "enemy/TEneFrog.h"
#include "enemy/EnemyFx.h"

extern "C" {
}

// 0x800F0E64
void FrogBrain::update(TEneFrog* owner)
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

// 0x800F0EFC
FrogBrainBase::FrogBrainBase()
{
	mOwner = 0;
}

// 0x800F0F30
FrogBrainBase::~FrogBrainBase()
{
}

// 0x800F0F8C
FrogHuntBrain* FrogHuntBrain::create(TEneFrog* owner)
{
	return new FrogHuntBrain(owner);
}

// 0x800F0FD4
FrogHuntBrain::FrogHuntBrain(TEneFrog* owner)
{
	mSight.reset();
	mMemory.reset();
	mTimer   = 0;
	mResting = 0;
	mHops    = 0;
	mOwner   = owner;
	mNext    = -1;
	mState   = STATE_APPEAR;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x800F1090
FrogHuntBrain::~FrogHuntBrain()
{
}

// 0x800F10F8
// A hard hit stuns it and it goes after who hit it
int FrogHuntBrain::checkInterrupt()
{
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		return 1;
	}
	if (mOwner->wasHitHard() != 0) {
		changeState(STATE_STUN);
		mMemory.setChase(mOwner->getHitPlayer());
	}
	return 0;
}

// 0x800F11E8
// A random point 25 to 35 from home
void FrogHuntBrain::pickTarget()
{
	Vec3f home;
	Vec3f target;
	home = mOwner->mHome;
	randomPointInRing(25.0f, 35.0f, &home, &target);
	mOwner->setTarget(&target);
}

// 0x800F124C
// Follows the player it remembers or looks 170 ahead in a cone
int FrogHuntBrain::look()
{
	int seen = 0;
	mMemory.tick();
	if (mMemory.isReady() != 0) {
		if (mMemory.isChasing() != 0) {
			seen = mSight.track(mMemory.getPlayer(), mOwner->getPos(), mOwner->getRotY());
		} else {
			seen = mSight.findInCone(28900.0f, mOwner->getPos(), mOwner->getRotY(), 0xF8E3);
		}
	}
	return seen;
}

// 0x800F135C
void FrogHuntBrain::aim()
{
	mOwner->setAim(&mSight.mTarget, mSight.mAngle);
}

// 0x800F138C
// Slot _0C
void FrogHuntBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_WANDER:
		phaseWander(phase);
		break;
	case STATE_ATTACK:
		phaseAttack(phase);
		break;
	case STATE_STUN:
		phaseStun(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x800F140C
void FrogHuntBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneFrog::ACT_APPEAR;
		break;
	case 2:
		if (mOwner->isAppearDone() != 0) {
			changeState(STATE_WANDER);
		}
		break;
	case 3:
		break;
	}
}

// 0x800F14AC
// Hops between random points resting after every three
void FrogHuntBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		pickTarget();
		mNext    = TEneFrog::ACT_HOP;
		mTimer   = randomInt(300.0f, 120.0f);
		mResting = 0;
		mHops    = 0;
		break;
	case 2:
		if (look() != 0 && mOwner->isGrounded() != 0) {
			mOwner->stopEffect();
			changeState(STATE_ATTACK);
		}
		break;
	case 1: {
		int repick = 0;
		mTimer--;
		switch (mResting) {
		case 0:
			if (mTimer > 0) {
				if (mOwner->hasArrived() != 0) {
					mHops++;
					if (mHops >= 3) {
						mHops    = 0;
						mResting = 1;
						mNext    = TEneFrog::ACT_IDLE;
					} else {
						repick = 1;
					}
				}
			} else {
				repick = 1;
			}
			break;
		case 1:
			if (mTimer <= 0) {
				mResting = 0;
				repick   = 1;
				mNext    = TEneFrog::ACT_HOP;
			}
			break;
		}
		if (repick) {
			pickTarget();
			mTimer = randomInt(300.0f, 120.0f);
		}
		break;
	}
	case 3:
		break;
	}
}

// 0x800F1650
// Licks keeping the tongue out for the parameter's frames then draws it back
void FrogHuntBrain::phaseAttack(int phase)
{
	switch (phase) {
	case 0:
		mNext    = TEneFrog::ACT_LICK;
		mTimer   = *gFrogParams;
		mResting = 0;
		break;
	case 2:
		if (mOwner->isRetractDone() != 0) {
			mMemory.startCooldown();
			mMemory.clearChase();
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		look();
		aim();
		switch (mResting) {
		case 1:
			break;
		case 0:
			mTimer--;
			if (mTimer <= 0) {
				mResting = 1;
				mNext    = TEneFrog::ACT_RETRACT;
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x800F176C
void FrogHuntBrain::phaseStun(int phase)
{
	switch (phase) {
	case 0:
		mNext = mOwner->stunAction();
		break;
	case 2:
		if (mOwner->isStunDone() != 0) {
			changeState(STATE_WANDER);
		}
		break;
	case 3:
		break;
	}
}

// 0x800F1810
void FrogHuntBrain::phaseDie(int phase)
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
