#include "enemy/TEnePenguin.h"
#include "enemy/EnemyFx.h"

// 0x800AC2E0
void PenguinBrain::update(TEnePenguin* owner)
{
	mOwner = owner;
	mNext = -1;
	if (checkInterrupt() == 0) {
		onPhase(mState, 2);
	}
	onPhase(mState, 1);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x800AC378
PenguinBrainBase::PenguinBrainBase()
{
	mOwner = 0;
}

// 0x800AC3AC
PenguinBrainBase::~PenguinBrainBase() { }

// 0x800AC408
PenguinHuntBrain* PenguinHuntBrain::create(TEnePenguin* owner)
{
	return new PenguinHuntBrain(owner);
}

// 0x800AC450
PenguinHuntBrain::PenguinHuntBrain(TEnePenguin* owner)
{
	mSight.reset();
	mMemory.reset();
	mTimer = 0;
	mResting = 0;
	mOwner = owner;
	mNext = -1;
	mState = STATE_APPEAR;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x800AC508
PenguinHuntBrain::~PenguinHuntBrain() { }

// 0x800AC570
// A hard hit stuns it unless it is winding up a slide
int PenguinHuntBrain::checkInterrupt()
{
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		return 1;
	}
	if (mOwner->wasHitHard() != 0) {
		if (mOwner->canStun() != 0) {
			changeState(STATE_STUN);
		} else {
			mOwner->noteStun();
		}
		mMemory.setChase(mOwner->getHitPlayer());
	}
	return 0;
}

// 0x800AC67C
// A random point 25 to 35 from home
void PenguinHuntBrain::pickTarget()
{
	Vec3f home;
	Vec3f target;
	home = mOwner->mHome;
	randomPointInRing(25.0f, 35.0f, &home, &target);
	mOwner->setTarget(&target);
}

// 0x800AC6E0
// Follows the player it remembers or looks 170 ahead in a cone
int PenguinHuntBrain::look()
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

// 0x800AC7F0
void PenguinHuntBrain::aim()
{
	mOwner->setAim(&mSight.mTarget);
}

// 0x800AC81C
// Slot _0C
void PenguinHuntBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_WANDER:
		phaseWander(phase);
		break;
	case STATE_SLIDE:
		phaseSlide(phase);
		break;
	case STATE_REST:
		phaseRest(phase);
		break;
	case STATE_STUN:
		phaseStun(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x800AC8B0
void PenguinHuntBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEnePenguin::ACT_APPEAR;
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

// 0x800AC950
// Waddles between random points resting on arrival until it sees someone
void PenguinHuntBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		pickTarget();
		mTimer = randomInt(300.0f, 120.0f);
		mNext = TEnePenguin::ACT_WALK;
		mResting = 0;
		break;
	case 2:
		if (look() != 0) {
			changeState(STATE_SLIDE);
		}
		break;
	case 1:
		mTimer--;
		if (mTimer < 0) {
			pickTarget();
			mTimer = randomInt(300.0f, 120.0f);
			if (mResting != 0) {
				mResting = 0;
				mNext = TEnePenguin::ACT_WALK;
			}
		} else if (mOwner->hasArrived() != 0) {
			mResting = 1;
			mNext = TEnePenguin::ACT_IDLE;
		}
		break;
	case 3:
		break;
	}
}

// 0x800ACA78
// Slides at the player for a while then spins to a stop
void PenguinHuntBrain::phaseSlide(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEnePenguin::ACT_SLIDE;
		mOwner->stopEffect();
		mTimer = gPenguinParams->mSlideTime;
		mResting = 0;
		break;
	case 2:
		if (mOwner->isSlideDone() != 0) {
			changeState(STATE_REST);
			mMemory.clearChase();
		}
		break;
	case 1:
		switch (mResting) {
		case 0:
			look();
			aim();
			mTimer--;
			if (mTimer < 0) {
				mResting = 1;
				mNext = TEnePenguin::ACT_SPIN;
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

// 0x800ACB94
// Gets up and catches its breath for three seconds
void PenguinHuntBrain::phaseRest(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEnePenguin::ACT_GETUP;
		mTimer = 180;
		mResting = 0;
		break;
	case 2:
		if (mOwner->isRestDone() != 0) {
			mMemory.startCooldown();
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		switch (mResting) {
		case 0:
			mTimer--;
			if (mTimer < 0) {
				mResting = 1;
				mNext = TEnePenguin::ACT_REST;
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

// 0x800ACC98
void PenguinHuntBrain::phaseStun(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEnePenguin::ACT_STUN;
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

// 0x800ACD38
void PenguinHuntBrain::phaseDie(int phase)
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
