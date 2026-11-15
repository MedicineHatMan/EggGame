#include "enemy/TEneAm07.h"
#include "enemy/TEneAm07BrainInline.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyCommon.h"
#include "enemy/EnemyAppear.h"
#include "engine/MotionPose.h"
#include "io/ResBank.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/EnemyHit.h"
#include "enemy/ObjEnemy.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "enemy/EnemyFx.h"
#include "Ninja/ninja.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "engine/TParticle.h"
#include "animal/RacerMover.h"
#include "enemy/TEneAm01.h"
struct Vec3f;
struct ColliShape;
struct ObjShadow;

// 0x8006F93C
// Slot _14 A state change from checkInterrupt skips phase 2
void Am07Brain::update(TEneAm07* owner)
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

// 0x8006F9D4
// Installs the base vtable then this one
Am07BrainBase::Am07BrainBase()
{
	mOwner = 0;
}

// 0x8006FA08
Am07BrainBase::~Am07BrainBase() { }

// 0x8006FA64
Am07BeeBrain* Am07BeeBrain::create(TEneAm07* owner)
{
	return new Am07BeeBrain(owner);
}

// 0x8006FAAC
Am07BeeBrain::Am07BeeBrain(TEneAm07* owner)
{
	mSight.reset();
	mSight.mHeightLimit = 500.0f;
	mMemory.reset();
	mTimer = 0;
	mWait = 0;
	mOwner = owner;
	mNext = -1;
	mState = STATE_APPEAR;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x8006FB6C
Am07BeeBrain::~Am07BeeBrain() { }

// 0x8006FBD4
// Slot _08 A flip hit while stinging only makes it flinch
int Am07BeeBrain::checkInterrupt()
{
	int changed = 0;
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		changed = 1;
	} else if (mOwner->wasHitKind3() != 0) {
		if (mOwner->canFlip() != 0) {
			changeState(STATE_FLIP);
			changed = 1;
		} else {
			mOwner->noteFlip();
		}
		mMemory.setChase(mOwner->getHitPlayer());
	}
	return changed;
}

// 0x8006FCE8
// A spot 60 from home in a random direction 20 up dropped to the ground if there is any below
void Am07BeeBrain::pickWanderSpot()
{
	Vec3f spot;
	mOwner->getHome(&spot);
	f32 s;
	f32 c;
	njSinCos((int)(65536.0f * (3.0517578e-5f * rand())), &s, &c);
	spot.x = 60.0f * s + spot.x;
	spot.z = 60.0f * c + spot.z;
	spot.y += 20.0f;
	Vec3f ground;
	if (probeGround(&spot, &ground, 0) != 0) {
		mOwner->setAnchor(&ground);
	} else {
		mOwner->setAnchor(&spot);
	}
}

// 0x8006FDCC
// 4 to 8 seconds
int Am07BeeBrain::wanderTime()
{
	return (int)(240.0f * (3.0517578e-5f * rand()) + 240.0f);
}

// 0x8006FE24
void Am07BeeBrain::aim()
{
	mOwner->setGoal(&mSight.mTarget);
}

// 0x8006FE50
// Follows the player it holds a grudge against or looks for anyone within 270
int Am07BeeBrain::look()
{
	int found = 0;
	mMemory.tick();
	if (mMemory.isReady() != 0) {
		if (mMemory.isChasing() != 0) {
			found = mSight.track(mMemory.getPlayer(), mOwner->getPos(), mOwner->getRotY());
		} else {
			found = mSight.findInRange(72900.0f, mOwner->getPos());
		}
	}
	return found;
}

// 0x8006FF3C
void Am07BeeBrain::attack()
{
	if (pickAttack() == 0) {
		changeState(STATE_HOVER);
	}
}

// 0x8006FFB0
// Stings within 80 dives within 150 or while it holds a grudge
int Am07BeeBrain::pickAttack()
{
	if (mSight.mDistSq < 6400.0f) {
		changeState(STATE_STING);
		return 1;
	}
	if (mSight.mDistSq < 22500.0f || mMemory.isChasing() != 0) {
		changeState(STATE_DIVE);
		return 1;
	}
	return 0;
}

// 0x80070094
// Slot _0C
void Am07BeeBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_WANDER:
		phaseWander(phase);
		break;
	case STATE_HOVER:
		phaseHover(phase);
		break;
	case STATE_DIVE:
		phaseDive(phase);
		break;
	case STATE_STING:
		phaseSting(phase);
		break;
	case STATE_STUCK:
		phaseStuck(phase);
		break;
	case STATE_FLIP:
		phaseFlip(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x8007012C
void Am07BeeBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm07::ACT_APPEAR;
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

// 0x800701CC
void Am07BeeBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		pickWanderSpot();
		mTimer = wanderTime();
		mNext = TEneAm07::ACT_WANDER;
		break;
	case 2:
		if (look() != 0) {
			mOwner->stopEffect();
			attack();
		}
		break;
	case 3:
		break;
	case 1:
		mTimer--;
		if (mTimer < 0) {
			pickWanderSpot();
			mTimer = wanderTime();
		}
		break;
	}
}

// 0x80070278
void Am07BeeBrain::phaseHover(int phase)
{
	switch (phase) {
	case 0:
		aim();
		mNext = TEneAm07::ACT_HOVER;
		break;
	case 2:
		if (look() != 0) {
			pickAttack();
		} else {
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

// 0x80070330
void Am07BeeBrain::phaseDive(int phase)
{
	switch (phase) {
	case 0:
		aim();
		mNext = TEneAm07::ACT_DIVE;
		break;
	case 2:
		if (look() == 0) {
			changeState(STATE_WANDER);
		} else if (mOwner->isDiveDone() != 0) {
			mMemory.startCooldown();
			mMemory.clearChase();
			attack();
		}
		break;
	case 1:
		aim();
		break;
	case 3:
		break;
	}
}

// 0x80070408
void Am07BeeBrain::phaseSting(int phase)
{
	switch (phase) {
	case 0:
		aim();
		mNext = TEneAm07::ACT_STING;
		break;
	case 2:
		look();
		if (mOwner->isStingDone() != 0) {
			mMemory.startCooldown();
			mMemory.clearChase();
			changeState(STATE_STUCK);
		}
		break;
	case 1:
		aim();
		break;
	case 3:
		break;
	}
}

// 0x800704CC
// Waits out the parameter then pulls free
void Am07BeeBrain::phaseStuck(int phase)
{
	switch (phase) {
	case 0:
		mTimer = 0;
		mWait = sAm07Param[0];
		mNext = TEneAm07::ACT_STUCK;
		break;
	case 2:
		if (mOwner->isPullDone() != 0) {
			if (look() != 0) {
				attack();
			} else {
				changeState(STATE_WANDER);
			}
		}
		break;
	case 3:
		break;
	case 1:
		switch (mTimer) {
		case 0:
			mWait--;
			if (mWait <= 0) {
				mTimer = 1;
				mNext = TEneAm07::ACT_PULL;
			}
			break;
		case 1:
			break;
		}
		break;
	}
}

// 0x800705E8
void Am07BeeBrain::phaseFlip(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm07::ACT_FLIP;
		break;
	case 1:
		break;
	case 2:
		if (mOwner->isFlipDone() != 0) {
			if (look() != 0) {
				attack();
			} else {
				changeState(STATE_WANDER);
			}
		}
		break;
	case 3:
		break;
	}
}

// 0x800706A4
void Am07BeeBrain::phaseDie(int phase)
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

