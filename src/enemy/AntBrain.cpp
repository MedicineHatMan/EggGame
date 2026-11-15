#include "enemy/AntBrain.h"
#include "enemy/EnemyBrainDtors.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"

#define random01() (3.0517578e-5f * rand())

// 0x80095164
Am01AntBrain::Am01AntBrain(AntLink* head, int slot)
{
	mLink.insert(head, slot, this);
}

// 0x800951C4
Am01AntBrain::~Am01AntBrain() { }

// 0x80095254
// Slot _24 Starts coming out of the queen's mouth and drops nothing when it dies
void Am01AntBrain::attach(TEneAm01* owner)
{
	mOwner = owner;
	mNext = -1;
	mState = STATE_APPEAR_AIMED;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
	mOwner->mFlags2 |= TEneAm01::AM_NO_DROP;
}

// 0x800952CC
void Am01AntBrain::setChase(u16 player)
{
	mMemory.setChase(player);
}

// 0x800952F0
// Its slot by the queen or without her a random point 25 out from home
void Am01AntBrain::pickTarget()
{
	Vec3f target;
	TEneAntsQueen* queen = mLink.getOwner();
	if (queen != 0) {
		queen->getSlotPos(mLink.mSlot, &target);
	} else {
		f32 dist = 25.0f * random01() + 25.0f;
		njSinCos((int)(65536.0f * random01()), &target.x, &target.z);
		target.y = 0.0f;
		gjScaleVector(dist, &target, &target);
		njAddVector(&target, &mOwner->mHome);
	}
	mOwner->setTarget(&target);
}

// 0x800953F4
// Keeps track of a player it was hit by or else looks for one ahead
int Am01AntBrain::look()
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

// 0x8009550C
// Slot _0C
void Am01AntBrain::onPhase(int state, int phase)
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
	case STATE_APPEAR_AIMED:
		phaseAppearAimed(phase);
		break;
	}
}

// 0x8009558C
// Slot _18
void Am01AntBrain::onAppearDone()
{
	changeState(STATE_WANDER);
}

// 0x800955F0
// Slot _1C
void Am01AntBrain::onFlipDone()
{
	changeState(STATE_WANDER);
}

// 0x80095654
// Thrown from the mouth Where it lands is its home
void Am01AntBrain::phaseAppearAimed(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneAm01::ACT_APPEAR_AIMED;
		break;
	case 2:
		if (mOwner->isAppearAimedDone() != 0) {
			mOwner->mHome = mOwner->mBody.mPos;
			onAppearDone();
		}
		break;
	case 3:
		break;
	}
}

// 0x800956E4
void Am01AntBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		mStep = 1;
		mTimer = 0;
		pickTarget();
		mOwner->mTopSpeed = 0.3f;
		mNext = TEneAm01::ACT_WALK;
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
					mStep = 0;
					mNext = TEneAm01::ACT_IDLE;
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
				mStep = 1;
				pickTarget();
				mOwner->mTopSpeed = 0.3f;
				mNext = TEneAm01::ACT_WALK;
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x800958E4
void Am01AntBrain::phaseChase(int phase)
{
	switch (phase) {
	case 0:
		mStep = 0;
		mOwner->mTopSpeed = 0.5f;
		mNext = TEneAm01::ACT_CHASE;
		mOwner->mBody.mVel.y += 1.0f;
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
