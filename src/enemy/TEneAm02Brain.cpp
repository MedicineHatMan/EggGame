#include "enemy/TEneAm02.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"

inline Am02SeekBrain::~Am02SeekBrain()
{
}

// 0 to 1 from the game's random number
#define random01() (3.0517578e-5f * rand())

// 0x8004AE64
Am02SeekBrain::Am02SeekBrain()
{
	mSight.reset();
	mMemory.reset();
	mStep  = 0;
	mTimer = 0;
}

// 0x8004AEF4
// A random point from half of radius to radius away from home
void Am02SeekBrain::pickTarget(f32 radius)
{
	f32 half = 0.5f * radius;
	f32 dist = (radius - half) * random01() + half;
	Vec3f target;
	njSinCos((int)(65536.0f * random01()), &target.x, &target.z);
	target.y = 0.0f;
	gjScaleVector(dist, &target, &target);
	njAddVector(&target, &mOwner->mHome);
	mOwner->setTarget(&target);
}

// 0x8004AFF4
// Slot _08 Unlike EnemySeekBrain a kind 3 hit always counts but only flips it outside a swing
int Am02SeekBrain::checkInterrupt()
{
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		return 1;
	}
	if (mOwner->wasHitKind3() != 0) {
		if (mOwner->canFlip() != 0) {
			changeState(STATE_FLIP);
		} else {
			mOwner->hitSound();
		}
		mMemory.setChase(mOwner->getHitPlayer());
		return 1;
	}
	if (mOwner->wasHitKind1() != 0) {
		changeState(STATE_STUN);
		return 1;
	}
	return 0;
}

// 0x8004B160
void Am02SeekBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm02::ACT_APPEAR;
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

// 0x8004B1D4
void Am02SeekBrain::phaseStun(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm02::ACT_STUN;
		break;
	case 1:
		break;
	case 2:
		if (mOwner->isStunDone() != 0) {
			onStunDone();
		}
		break;
	case 3:
		break;
	}
}

// 0x8004B248
void Am02SeekBrain::phaseFlip(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm02::ACT_FLIP;
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

// 0x8004B2BC
void Am02SeekBrain::phaseDie(int phase)
{
	switch (phase) {
	case 0:
		mNext = mOwner->pickDeath();
		break;
	case 1:
		break;
	case 2:
		break;
	}
}

// 0x8004B300
Am02AttackBrain::Am02AttackBrain(TEneAm02* owner)
{
	start(owner);
}

// 0x8004B384
Am02AttackBrain::~Am02AttackBrain()
{
}

// 0x8004B404
void Am02AttackBrain::pickHomeTarget()
{
	pickTarget(50.0f);
}

// 0x8004B428
// Sees 120 degrees either side out to 200 or 150 degrees either side within 50
int Am02AttackBrain::look()
{
	int seen = 0;
	mMemory.tick();
	if (mMemory.isReady() != 0) {
		if (mMemory.isChasing() != 0) {
			Vec3f* pos;
			seen = mSight.track(mMemory.getPlayer(), mOwner->getPos(), mOwner->getRotY());
		} else {
			seen = mSight.search(40000.0f, 2500.0f, mOwner->getPos(), mOwner->getRotY(), 0xAAAA, 0xD555);
		}
	}
	return seen;
}

// 0x8004B540
// Slot _0C
void Am02AttackBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
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
	case STATE_WANDER:
		phaseWander(phase);
		break;
	case STATE_CHASE:
		phaseChase(phase);
		break;
	case STATE_ATTACK:
		phaseAttack(phase);
		break;
	}
}

// 0x8004B5CC
// Slot _18
void Am02AttackBrain::onAppearDone()
{
	changeState(STATE_WANDER);
}

// 0x8004B630
// Slot _1C
void Am02AttackBrain::onStunDone()
{
	changeState(STATE_WANDER);
}

// 0x8004B694
// Slot _20
void Am02AttackBrain::onFlipDone()
{
	changeState(STATE_WANDER);
}

// 0x8004B6F8
// mStep 1 walking for up to 600 frames per target 2 saw a player
void Am02AttackBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		mStep  = 1;
		mTimer = 0;
		pickHomeTarget();
		mNext = TEneAm02::ACT_WALK;
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
				mTimer = 0;
				pickHomeTarget();
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8004B80C
// mStep 0 chasing 1 lost the player 2 close enough to attack
void Am02AttackBrain::phaseChase(int phase)
{
	switch (phase) {
	case 0:
		mStep = 0;
		mNext = TEneAm02::ACT_CHASE;
		mOwner->startEffect();
		break;
	case 2:
		if (mStep == 1) {
			changeState(STATE_WANDER);
		} else if (mStep == 2) {
			changeState(STATE_ATTACK);
		}
		break;
	case 1:
		if (look() == 0) {
			mStep = 1;
			break;
		}
		if (mSight.mDistSq <= 2500.0f && mSight.mAngleDiff <= 0x2AAA) {
			mStep = 2;
		}
		mOwner->setTarget(&mSight.mTarget);
		break;
	case 3:
		break;
	}
}

// 0x8004B950
// Afterwards it stops looking for a while and forgets who hit it
void Am02AttackBrain::phaseAttack(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm02::ACT_ATTACK;
		break;
	case 1:
		break;
	case 2:
		if (mOwner->isAttackDone() != 0) {
			changeState(STATE_WANDER);
			mMemory.startCooldown();
			mMemory.clearChase();
		}
		break;
	case 3:
		break;
	}
}

// 0x8004BA00
Am02ThrowBrain::Am02ThrowBrain(TEneAm02* owner)
{
	mWanderStep  = 0;
	mWanderTimer = 0;
	start(owner);
}

// 0x8004BA8C
Am02ThrowBrain::~Am02ThrowBrain()
{
}

// 0x8004BB0C
void Am02ThrowBrain::pickHomeTarget()
{
	pickTarget(30.0f);
}

// 0x8004BB30
// Sees about 85 degrees either side out to 220 or 150 degrees either side within 50
int Am02ThrowBrain::look()
{
	int seen = 0;
	mMemory.tick();
	if (mMemory.isReady() != 0) {
		if (mMemory.isChasing() != 0) {
			Vec3f* pos;
			seen = mSight.track(mMemory.getPlayer(), mOwner->getPos(), mOwner->getRotY());
		} else {
			seen = mSight.search(48400.0f, 2500.0f, mOwner->getPos(), mOwner->getRotY(), 0x78E3, 0xD555);
		}
	}
	return seen;
}

// 0x8004BC48
// Slot _0C
void Am02ThrowBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
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
	case STATE_WANDER:
		phaseWander(phase);
		break;
	case STATE_CHASE:
		phaseThrow(phase);
		break;
	}
}

// 0x8004BCDC
// Slot _18
void Am02ThrowBrain::onAppearDone()
{
	changeState(STATE_WANDER);
}

// 0x8004BD40
// Slot _1C
void Am02ThrowBrain::onStunDone()
{
	changeState(STATE_WANDER);
}

// 0x8004BDA4
// Slot _20
void Am02ThrowBrain::onFlipDone()
{
	changeState(STATE_WANDER);
}

// 0x8004BE08
// mWanderStep 1 walking for up to 600 frames per target 2 saw a player
void Am02ThrowBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		mWanderStep  = 1;
		mWanderTimer = 0;
		pickHomeTarget();
		mNext = TEneAm02::ACT_WALK;
		break;
	case 2:
		if (mWanderStep == 2) {
			changeState(STATE_CHASE);
		}
		break;
	case 1:
		if (look() != 0) {
			mWanderStep = 2;
			break;
		}
		switch (mWanderStep) {
		case 1:
			mWanderTimer++;
			if (mOwner->hasArrived() != 0 || mWanderTimer >= 600) {
				mWanderTimer = 0;
				pickHomeTarget();
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8004BF1C
// Afterwards it stops looking for a while and forgets who hit it
void Am02ThrowBrain::phaseThrow(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm02::ACT_THROW;
		break;
	case 2:
		if (mOwner->isAttackDone() != 0) {
			mMemory.startCooldown();
			mMemory.clearChase();
			changeState(STATE_WANDER);
		}
		break;
	case 1:
		look();
		mOwner->setTarget(&mSight.mTarget);
		break;
	case 3:
		break;
	}
}
