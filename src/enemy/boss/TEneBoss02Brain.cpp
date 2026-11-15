#include "enemy/boss/TEneBoss02.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "enemy/boss/TEneBoss02_Pool.h"
#include "Ninja/ninja.h"
#include "animal/RacerMover.h"

static int sPlan[] = { 12, 1, 2, 3, 5, 9, 13 };

// 0x8005DF9C
void Boss02Brain::update(TEneBoss02* owner)
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

// 0x8005E034
Boss02BrainBase::Boss02BrainBase() { }

// 0x8005E064
Boss02BrainBase::~Boss02BrainBase() { }

// 0x8005E0C0
Boss02FightBrain* Boss02FightBrain::create(TEneBoss02* owner)
{
	return new Boss02FightBrain(owner);
}

// 0x8005E108
Boss02FightBrain::Boss02FightBrain(TEneBoss02* owner)
{
	int i;
	mSight.reset();
	mPlanStep = 0;
	mRound = 0;
	for (i = 0; i < 10; i++) {
		mSlots[i] = -1;
		mGates[i].x = 0.0f;
		mGates[i].y = 0.0f;
		mGates[i].z = 0.0f;
	}
	mDives = 0;
	mSlams = 0;
	mFlags = 0;
	mStep = 0;
	mTimer = 0;
	mGap = 0;
	mShots = 0;
	_10C = 0;
	mOwner = owner;
	mNext = -1;
	mState = STATE_START;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
	mHealthMark = mOwner->mHealth;
	mFx = 0;
}

// 0x8005E244
Boss02FightBrain::~Boss02FightBrain()
{
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
}

// 0x8005E2CC
// Death wins A heavy hit is noted for the state to act on and guards the boss
int Boss02FightBrain::checkInterrupt()
{
	int handled = 1;
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
	} else if (mOwner->wasHitKind3() != 0) {
		mFlags |= 1;
		mOwner->onHeavyHit();
		mOwner->setGuarded();
	} else if (mOwner->wasHitKind1() != 0) {
		mOwner->onLightHit();
	} else {
		handled = 0;
	}
	return handled;
}

// 0x8005E3A8
// The next state of the plan
void Boss02FightBrain::nextState()
{
	int state;
	int again;
	do {
		again = 0;
		state = sPlan[mPlanStep];
		mPlanStep++;
		if (state < 11) {
			changeState(state);
		} else {
			switch (state) {
			case 12:
				again = 1;
				break;
			case 13:
				again = 1;
				mRound++;
				mPlanStep = 0;
				while (sPlan[mPlanStep++] != 12) { }
				break;
			}
		}
	} while (again != 0);
}

// 0x8005E49C
int Boss02FightBrain::look()
{
	return mSight.findInRange(100000000.0f, mOwner->getPos());
}

// 0x8005E4F4
// The player's place pulled in to margin inside the ring of 245 or the middle
void Boss02FightBrain::aimAtPlayer(f32 margin)
{
	Vec3f aim;
	Vec3f dir;
	if (look() != 0) {
		f32 len;
		aim = mSight.mTarget;
		dir = aim;
		dir.y = 0.0f;
		len = njUnitVector(&dir);
		if (len < gUnitVectorMin) {
			dir = gVecZero;
		}
		margin = 245.0f - margin;
		if (len > margin) {
			aim.x = dir.x * margin;
			aim.z = dir.z * margin;
		}
	} else {
		aim.x = 0.0f;
		aim.z = 0.0f;
	}
	aim.y = 610.2f;
	mOwner->setAim(&aim);
}

// 0x8005E5E4
// 0 at full health then 1 and 2
int Boss02FightBrain::getTier()
{
	int health = mOwner->mHealth;
	int tier;
	const int* limits = gBoss02Params->mTierHealth;
	for (tier = 0; tier < 2; tier++) {
		if (health >= limits[tier]) {
			return tier;
		}
	}
	return tier;
}

// 0x8005E620
// A point on the ring of 200 a quarter turn a step turned by the round
void Boss02FightBrain::ringPoint(Vec3f* out, int step)
{
	f32 s;
	f32 c;
	njSinCos(((step + mRound) % 4) * 0x10000 / 4, &s, &c);
	out->x = 200.0f * s;
	out->y = 610.2f;
	out->z = 200.0f * c;
}

// 0x8005E6A0
void Boss02FightBrain::goHome()
{
	Vec3f at;
	ringPoint(&at, 2);
	mOwner->setTarget(&at);
}

// 0x8005E6E0
void Boss02FightBrain::setupShots(int* count, int* gap)
{
	int tier = getTier();
	*count = 0;
	*gap = gBoss02Params->mShotGap[tier];
	mOwner->setTurnRate(gBoss02Params->mTurnRate[tier]);
}

// 0x8005E744
// Takes up to count free gates ordered from the boss's left to its right
int Boss02FightBrain::pickGates(int count)
{
	f32 side[10];
	Vec3f pos;
	Vec3f d;
	Vec3f dir;
	int taken = 0;
	int j;
	int best;

	for (int i = 0; i < count; i++) {
		int slot = takeFreeGateSlot(&mGates[taken]);
		if (slot >= 0) {
			mSlots[i] = slot;
			taken++;
		}
	}
	pos = *mOwner->getPos();
	dir = pos;
	dir.y = 0.0f;
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 dz = dir.z;
	f32 nx = -dir.x;
	for (int i = 0; i < taken; i++) {
		d = mGates[i];
		njSubVector(&d, &pos);
		d.y = 0.0f;
		if (njUnitVector(&d) < gUnitVectorMin) {
			d = gVecZero;
		}
		side[i] = d.x * dz + d.z * nx;
	}
	for (int i = 0; i < taken - 1; i++) {
		f32 top = -2.0f;
		for (j = i; j < taken; j++) {
			if (side[j] > top) {
				top = side[j];
				best = j;
			}
		}
		if (i != best) {
			Vec3f swap;
			int slot;
			side[best] = side[i];
			side[i] = top;
			swap = mGates[i];
			mGates[i] = mGates[best];
			mGates[best] = swap;
			slot = mSlots[i];
			mSlots[i] = mSlots[best];
			mSlots[best] = slot;
		}
	}
	return taken;
}

// 0x8005E9F4
int Boss02FightBrain::gateCount()
{
	return getTier() + 4;
}

// 0x8005EA18
void Boss02FightBrain::goToGate()
{
	Vec3f at;
	findTakenGateSlot(&at);
	mOwner->setTarget(&at);
}

// 0x8005EA54
int Boss02FightBrain::diveCount()
{
	int tier = getTier();
	return gBoss02Params->mDives[tier];
}

// 0x8005EA84
void Boss02FightBrain::setMoveRate()
{
	int tier = getTier();
	mOwner->setMoveRate(gBoss02Params->mMoveRate[tier]);
}

// 0x8005EAC8
int Boss02FightBrain::slamCount()
{
	int tier = getTier();
	return gBoss02Params->mSlams[tier];
}

// 0x8005EAF8
int Boss02FightBrain::slamDamage()
{
	int tier = getTier();
	return gBoss02Params->mSlamDamage[tier];
}

// 0x8005EB28
int Boss02FightBrain::diveWait()
{
	int tier = getTier();
	return gBoss02Params->mDiveWait[tier];
}

// 0x8005EB58
void Boss02FightBrain::glowOn()
{
	mOwner->glowFlash();
}

// 0x8005EB7C
void Boss02FightBrain::glowOff()
{
	mOwner->glowFade();
}

// 0x8005EBA0
void Boss02FightBrain::endStun()
{
	mFlags &= ~1;
	mOwner->setNeutral();
}

// 0x8005EBD0
void Boss02FightBrain::beginStun()
{
	mFlags &= ~1;
	mOwner->setOpen();
}

// 0x8005EC00
// Slot _0C
void Boss02FightBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_START:
		phaseStart(phase);
		break;
	case STATE_SHOOT:
		phaseShoot(phase);
		break;
	case STATE_GATES:
		phaseGates(phase);
		break;
	case STATE_SPIT:
		phaseSpit(phase);
		break;
	case STATE_STUN:
		phaseStun(phase);
		break;
	case STATE_SINK:
		phaseSink(phase);
		break;
	case STATE_SLAM:
		phaseSlam(phase);
		break;
	case STATE_DIVE:
		phaseDive(phase);
		break;
	case STATE_HURT:
		phaseHurt(phase);
		break;
	case STATE_RETURN:
		phaseReturn(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x8005ECBC
void Boss02FightBrain::phaseStart(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setHealth(gBoss02Params->mHealth);
		resetBossGates();
		goHome();
		mNext = TEneBoss02::ACT_APPEAR;
		break;
	case 2:
		if (mOwner->isAppearDone() != 0) {
			nextState();
		}
		break;
	case 1:
		break;
	case 3:
		break;
	}
}

// 0x8005ED40
// Turns to the player and shoots a few times then waits
void Boss02FightBrain::phaseShoot(int phase)
{
	int* gap = &mGap;
	int* shots = &mShots;
	switch (phase) {
	case 0:
		goHome();
		mNext = TEneBoss02::ACT_RISE;
		setupShots(gap, shots);
		mStep = 0;
		mTimer = 300;
		glowOn();
		mOwner->setNeutral();
		break;
	case 2:
		if (mStep == 5) {
			nextState();
		}
		break;
	case 1: {
		int next = 5;
		switch (mStep) {
		case 0:
			if (mOwner->isRiseDone() != 0) {
				next = 1;
				endStun();
			}
			break;
		case 1: {
			int close;
			aimAtPlayer(5.0f);
			mTimer--;
			close = 0;
			if (mOwner->isFacing() != 0) {
				if (mOwner->isClose() == 0) {
					mNext = TEneBoss02::ACT_SHOOT;
					mStep = 2;
				} else {
					close = 1;
				}
			}
			if (mTimer <= 0 || close != 0) {
				Vec3f at;
				f32 s;
				f32 c;
				njSinCos(mOwner->getMouth(&at), &s, &c);
				at.x = 245.0f * s + at.x;
				at.y = 610.2f;
				at.z = 245.0f * c + at.z;
				mOwner->setAim(&at);
				mNext = TEneBoss02::ACT_SHOOT;
				mStep = 2;
			}
			break;
		}
		case 2:
			if (mOwner->isShootDone() != 0) {
				(*shots)--;
				mNext = TEneBoss02::ACT_WAIT;
				mStep = 3;
				mTimer = *gap;
			}
			break;
		case 3:
			mTimer--;
			if (mTimer <= 0) {
				if (*shots <= 0) {
					mStep = 5;
				} else {
					next = 1;
				}
			}
			break;
		case 4:
			if (mOwner->isSpitDone() != 0) {
				mStep = 5;
			}
			break;
		case 5:
			break;
		}
		if (mFlags & 1) {
			mFlags &= ~1;
			mNext = TEneBoss02::ACT_SPIT;
			mStep = 4;
		} else if (next == 1) {
			aimAtPlayer(5.0f);
			mNext = TEneBoss02::ACT_TURN;
			mStep = 1;
			mTimer = 300;
		}
		break;
	}
	case 3:
		break;
	}
}

// 0x8005EFE4
void Boss02FightBrain::aimGate(int n)
{
	mOwner->setAimSlot(&mGates[n - 1], mSlots[n - 1]);
	mNext = TEneBoss02::ACT_TURN;
}

// 0x8005F038
// Opens a gate at each slot it took the last first
void Boss02FightBrain::phaseGates(int phase)
{
	switch (phase) {
	case 0:
		mTimer = gateCount();
		mTimer = pickGates(mTimer);
		mStep = 0;
		mGap = 0;
		if (mTimer > 0) {
			aimGate(mTimer);
		}
		break;
	case 2:
		if (mTimer <= 0) {
			nextState();
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			mGap++;
			if (mOwner->isFacing() != 0 || mGap >= 600) {
				mNext = TEneBoss02::ACT_SHOOT_GATE;
				mStep = 1;
			}
			break;
		case 1:
			if (mOwner->isShootGateDone() != 0) {
				mTimer--;
				if (mTimer > 0) {
					mStep = 0;
					mGap = 0;
					aimGate(mTimer);
				}
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8005F168
// Turns to the middle and roars open to hits
void Boss02FightBrain::phaseSpit(int phase)
{
	switch (phase) {
	case 0: {
		Vec3f at;
		mStep = 0;
		mTimer = 0;
		at.x = 0.0f;
		at.y = 610.2f;
		at.z = 0.0f;
		mOwner->setAim(&at);
		mNext = TEneBoss02::ACT_TURN;
		break;
	}
	case 2:
		if ((mFlags & 1) && mStep == 2) {
			changeState(STATE_STUN);
		} else if (mStep == 3) {
			nextState();
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			mTimer++;
			if (mOwner->isFacing() != 0 || mTimer >= 600) {
				mNext = TEneBoss02::ACT_ROAR;
				mStep = 1;
			}
			break;
		case 1:
			if (mOwner->isRoarDone() != 0) {
				mNext = TEneBoss02::ACT_STUN;
				mStep = 2;
				mTimer = 120;
				glowOff();
				beginStun();
			}
			break;
		case 2:
			mTimer--;
			if (mTimer <= 0) {
				mStep = 3;
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8005F318
void Boss02FightBrain::phaseStun(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneBoss02::ACT_SPIT;
		break;
	case 2:
		if (mOwner->isSpitDone() != 0) {
			nextState();
		}
		break;
	case 1:
		break;
	case 3:
		break;
	}
}

// 0x8005F380
// Sinks into a gate
void Boss02FightBrain::phaseSink(int phase)
{
	switch (phase) {
	case 0:
		goToGate();
		mNext = TEneBoss02::ACT_DIVE;
		mTimer = 180;
		mStep = 0;
		mDives = 0;
		mHealthMark = mOwner->mHealth;
		setMoveRate();
		break;
	case 2:
		if (mTimer <= 0) {
			changeState(STATE_SLAM);
		}
		break;
	case 1:
		if (mStep == 0) {
			if (mOwner->isDiveDone() != 0) {
				mStep = 1;
			}
		} else {
			mTimer--;
		}
		break;
	case 3:
		break;
	}
}

// 0x8005F480
// Leaps out at the player and slams until enough slams or enough damage
void Boss02FightBrain::phaseSlam(int phase)
{
	switch (phase) {
	case 0:
		goToGate();
		aimAtPlayer(60.0f);
		mNext = TEneBoss02::ACT_LEAP;
		mStep = 0;
		mSlams = 0;
		mHealthMark = mOwner->mHealth;
		setMoveRate();
		beginStun();
		break;
	case 2:
		if (mSlams >= slamCount()) {
			changeState(STATE_DIVE);
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			aimAtPlayer(60.0f);
			if (mOwner->isLeapDone() != 0) {
				mStep = 1;
				mNext = TEneBoss02::ACT_SWIM;
				beginStun();
			}
			break;
		case 1:
			mSlams++;
			if (mOwner->isSurfaceDone() != 0) {
				mNext = TEneBoss02::ACT_SWIM;
			}
			if (mFlags & 1) {
				int health;
				mNext = TEneBoss02::ACT_SURFACE;
				setMoveRate();
				health = mOwner->mHealth;
				if (mHealthMark - health >= slamDamage()) {
					mSlams = slamCount();
				} else {
					beginStun();
				}
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8005F628
// Dives through a gate a few times
void Boss02FightBrain::phaseDive(int phase)
{
	switch (phase) {
	case 0:
		goToGate();
		mNext = TEneBoss02::ACT_DIVE_DEEP;
		mTimer = 1;
		mStep = 0;
		break;
	case 2:
		if (mTimer <= 0) {
			mDives++;
			if (mDives >= diveCount()) {
				nextState();
			} else {
				changeState(STATE_SLAM);
			}
		}
		break;
	case 1:
		if (mStep == 0) {
			if (mOwner->isDiveDeepDone() != 0) {
				mStep = 1;
				mTimer = diveWait();
			}
		} else {
			mTimer--;
		}
		break;
	case 3:
		break;
	}
}

// 0x8005F748
void Boss02FightBrain::phaseHurt(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneBoss02::ACT_SURFACE;
		setMoveRate();
		break;
	case 2:
		if (mOwner->isSurfaceDone() != 0) {
			changeState(STATE_DIVE);
		}
		break;
	case 1:
		mSlams++;
		break;
	case 3:
		break;
	}
}

// 0x8005F800
// Rises through a gate onto the ring
void Boss02FightBrain::phaseReturn(int phase)
{
	switch (phase) {
	case 0: {
		Vec3f at;
		goToGate();
		ringPoint(&at, 1);
		mOwner->setAim(&at);
		mNext = TEneBoss02::ACT_GATE_RISE;
		mStep = 0;
		mTimer = 120;
		break;
	}
	case 2:
		if (mTimer == 0) {
			nextState();
			glowOn();
		}
		break;
	case 1:
		if (mStep == 0) {
			if (mOwner->isGateRiseDone() != 0) {
				mStep = 1;
				glowOff();
				beginStun();
			}
		} else {
			mTimer--;
			if (mFlags & 1) {
				mTimer = 0;
			}
		}
		break;
	case 3:
		break;
	}
}

// 0x8005F8F8
void Boss02FightBrain::phaseDie(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneBoss02::ACT_DIE;
		break;
	case 1:
		break;
	case 2:
		break;
	}
}
