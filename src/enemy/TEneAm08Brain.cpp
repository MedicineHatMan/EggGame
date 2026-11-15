#include "enemy/TEneAm08.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyCommon.h"
#include "engine/MotionPose.h"
#include "engine/CharaColli.h"
#include "io/ResBank.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/ObjEnemy.h"
#include "enemy/EnemyFx.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "enemy/EnemyHit.h"
#include "engine/TParticle.h"
#include "enemy/TEneAm01.h"

void Am08Brain::update(TEneAm08* owner)
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

// 0x800753C8
// Installs the base vtable then this one
Am08BrainBase::Am08BrainBase()
{
	mOwner = 0;
}

// 0x800753FC
Am08BrainBase::~Am08BrainBase()
{
}

// 0x80075458
Am08LizardBrain* Am08LizardBrain::create(TEneAm08* owner)
{
	return new Am08LizardBrain(owner);
}

// 0x800754A0
Am08LizardBrain::Am08LizardBrain(TEneAm08* owner)
{
	mSight.reset();
	mMemory.reset();
	mTimer = 0;
	mWait  = 0;
	mOwner = owner;
	mNext  = -1;
	mState = STATE_APPEAR;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x80075558
Am08LizardBrain::~Am08LizardBrain()
{
}

// 0x800755C0
// Slot _08 A flip hit while guarding only makes it flinch
int Am08LizardBrain::checkInterrupt()
{
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		return 1;
	}
	if (mOwner->wasHitKind3() != 0) {
		if (mOwner->canFlip() != 0) {
			changeState(STATE_FLIP);
		} else {
			mOwner->noteFlip();
		}
		mMemory.setChase(mOwner->getHitPlayer());
		return 1;
	}
	return 0;
}

// 0x800756D4
// A spot 25 to 35 from home
void Am08LizardBrain::pickWanderSpot()
{
	Vec3f spot;
	spot.x = mOwner->mHome.x;
	spot.y = mOwner->mHome.y;
	spot.z = mOwner->mHome.z;
	randomPointInRing(25.0f, 35.0f, &spot, &spot);
	mOwner->setAnchor(&spot);
}

// 0x80075738
// Follows the player it holds a grudge against or searches in front of it
int Am08LizardBrain::look(f32 farSq, f32 nearSq, int farAngle, int nearAngle)
{
	int found = 0;
	mMemory.tick();
	if (mMemory.isReady() != 0) {
		if (mMemory.isChasing() != 0) {
			found = mSight.track(mMemory.getPlayer(), mOwner->getPos(), mOwner->getRotY());
		} else {
			found = mSight.search(farSq, nearSq, mOwner->getPos(), mOwner->getRotY(), farAngle, nearAngle);
		}
	}
	return found;
}

// 0x8007586C
// 170 ahead 70 all round
int Am08LizardBrain::lookNear()
{
	return look(28900.0f, 4900.0f, 0x8000, 0xD555);
}

// 0x800758A0
// 220 ahead 70 all round
int Am08LizardBrain::lookFar()
{
	return look(48400.0f, 4900.0f, 0x8000, 0xD555);
}

// 0x800758D4
void Am08LizardBrain::aim()
{
	mOwner->setGoal(&mSight.mTarget, mSight.mAngle);
}

// 0x80075904
// Slot _0C
void Am08LizardBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_WANDER:
		phaseWander(phase);
		break;
	case STATE_STALK:
		phaseStalk(phase);
		break;
	case STATE_ATTACK:
		phaseAttack(phase);
		break;
	case STATE_RECOVER:
		phaseRecover(phase);
		break;
	case STATE_FLIP:
		phaseFlip(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x80075990
void Am08LizardBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm08::ACT_APPEAR;
		break;
	case 1:
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

// 0x80075A30
// Walks to a spot idles there and picks a new one every 2 to 7 seconds
void Am08LizardBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		pickWanderSpot();
		mNext  = TEneAm08::ACT_WALK;
		mTimer = randomInt(300.0f, 120.0f);
		mWait  = 0;
		break;
	case 2:
		if (lookNear() != 0) {
			changeState(STATE_STALK);
		}
		break;
	case 3:
		break;
	case 1:
		mTimer--;
		if (mTimer < 0) {
			pickWanderSpot();
			mTimer = randomInt(300.0f, 120.0f);
			if (mWait != 0) {
				mWait = 0;
				mNext = TEneAm08::ACT_WALK;
			}
		} else if (mOwner->hasArrived() != 0) {
			mWait = 1;
			mNext = TEneAm08::ACT_IDLE;
		}
		break;
	}
}

// 0x80075B58
// Threatens then closes in until the jaws touch
void Am08LizardBrain::phaseStalk(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm08::ACT_THREAT;
		mOwner->stopEffect();
		mTimer = 0;
		break;
	case 2:
		switch (mTimer) {
		case 0:
			lookFar();
			break;
		case 1:
		default:
			if (lookFar() == 0) {
				mMemory.startCooldown();
				changeState(STATE_WANDER);
			} else if (mOwner->canBite() != 0) {
				changeState(STATE_ATTACK);
			}
			break;
		}
		break;
	case 3:
		break;
	case 1:
		aim();
		switch (mTimer) {
		case 0:
			if (mOwner->isThreatDone() != 0) {
				mTimer = 1;
				mNext  = TEneAm08::ACT_APPROACH;
			}
			break;
		case 1:
			break;
		}
		break;
	}
}

// 0x80075CD0
// The lunge then a second of sliding
void Am08LizardBrain::phaseAttack(int phase)
{
	switch (phase) {
	case 0:
		mNext  = TEneAm08::ACT_LUNGE;
		mTimer = 0;
		mWait  = 0;
		break;
	case 2:
		lookFar();
		if (mTimer == 2) {
			changeState(STATE_RECOVER);
			mMemory.clearChase();
		}
		break;
	case 1:
		aim();
		switch (mTimer) {
		case 0:
			if (mOwner->isLungeDone() != 0) {
				mTimer = 1;
				mWait  = 60;
				mNext  = TEneAm08::ACT_SLIDE;
			}
			break;
		case 1:
			if (mOwner->isSlideDone() != 0) {
				mWait--;
				if (mWait <= 0) {
					mTimer = 2;
				}
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x80075E08
void Am08LizardBrain::phaseRecover(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm08::ACT_RECOVER;
		break;
	case 1:
		break;
	case 2:
		if (mOwner->isRecoverDone() != 0) {
			changeState(STATE_WANDER);
		}
		break;
	case 3:
		break;
	}
}

// 0x80075EA8
// Flipped mid-lunge it recovers Otherwise it goes back to wandering
void Am08LizardBrain::phaseFlip(int phase)
{
	switch (phase) {
	case 0:
		if (mOwner->isLunging() != 0) {
			mTimer = 0;
			mNext  = TEneAm08::ACT_FLIP_LUNGING;
		} else {
			mTimer = 1;
			mNext  = TEneAm08::ACT_FLIP;
		}
		break;
	case 1:
		break;
	case 2:
		if (mTimer == 0) {
			if (mOwner->isFlipLungingDone() != 0) {
				changeState(STATE_RECOVER);
			}
		} else if (mOwner->isFlipDone() != 0) {
			changeState(STATE_WANDER);
		}
		break;
	case 3:
		break;
	}
}

// 0x80075FD4
void Am08LizardBrain::phaseDie(int phase)
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
