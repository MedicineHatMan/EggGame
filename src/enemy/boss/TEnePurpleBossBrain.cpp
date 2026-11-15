#include "enemy/boss/TEnePurpleBoss.h"
#include "enemy/boss/TEnePurpleBoss_IceBall.h"
#include "core/Mem.h"

static void purpleRootUpdate(PurpleBrainRoot* root, TEnePurpleBoss* owner) { root->PurpleBrainRoot::update(owner); }

// 0x800CB09C
inline void PurpleBrainRoot::update(TEnePurpleBoss* owner)
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

// 0x800CB134
PurpleBrainBase::PurpleBrainBase() { mOwner = 0; }

// 0x800CB168
PurpleBrainBase::~PurpleBrainBase() { }

// 0x800CB1C4
PurpleBrain* createPurpleBrain(TEnePurpleBoss* owner)
{
	return new PurpleBrain(owner);
}

// 0x800CB20C
PurpleBrain::PurpleBrain(TEnePurpleBoss* owner)
{
	mSight.reset();
	mWait  = 0;
	mStep  = 0;
	mTimer = 0;
	start(owner);
}

// 0x800CB2B8
PurpleBrain::~PurpleBrain() { }

// 0x800CB320
// Slot _08
int PurpleBrain::checkInterrupt()
{
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		return 1;
	}
	if (mOwner->wasHitHard() != 0) {
		mOwner->stageBeaten();
	} else if (mOwner->wasHitKind1() != 0) {
		mOwner->hurtReact();
	}
	return 0;
}

// 0x800CB3E0
// Looks for the player once mWait has run out
int PurpleBrain::look()
{
	if (mWait <= 0) {
		return mSight.findInRange(100000000.0f, mOwner->getPos());
	}
	mWait--;
	return 0;
}

// 0x800CB454
void PurpleBrain::aim()
{
	mOwner->setTargetAngle(mSight.mAngle);
}

// 0x800CB480
// How many of the health thresholds ending at one not above 0 the owner is at or below
int PurpleBrain::getStage(const int* steps)
{
	int health = mOwner->mHealth;
	int i;
	for (i = 0; steps[i] > 0; i++) {
		if (health > ((volatile const int*)steps)[i]) {
			break;
		}
	}
	return i;
}

// 0x800CB4B8
void PurpleBrain::setSpeed()
{
	int stage = getStage(gPurpleBossParams->mHealthSteps);

	mOwner->setSkateSpeed(gPurpleBossParams->mSkateSpeed[stage]);
}

// 0x800CB504
void PurpleBrain::throwBalls()
{
	int stage = getStage(gPurpleBossParams->mHealthSteps2);

	setIceBallLimit(gPurpleBossParams->mBallLimit[stage]);
	mOwner->throwBalls(gPurpleBossParams->mBallCount[stage]);
}

// 0x800CB560
// Slot _0C
void PurpleBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_CHASE:
		phaseChase(phase);
		break;
	case STATE_BOUNCED:
		phaseBounced(phase);
		break;
	case STATE_DASH:
		phaseDash(phase);
		break;
	case STATE_DROP:
		phaseDash(phase);
		break;
	case STATE_FREEZE:
		phaseFreeze(phase);
		break;
	case STATE_SPIN:
		phaseSpin(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x800CB5F8
// Rises in the middle of the rink
void PurpleBrain::phaseAppear(int phase)
{
	Vec3f pos;

	switch (phase) {
	case 0:
		mNext = 0;
		mOwner->setHealth(gPurpleBossParams->mHealth);
		break;
	case 2:
		if (mOwner->isAction0Done() != 0) {
			changeState(STATE_CHASE);
		}
		break;
	case 3:
		pos.x = 0.0f;
		pos.y = 50.0f;
		pos.z = 0.0f;
		mOwner->setPos(&pos);
		break;
	}
}

// 0x800CB6E0
// Turns to the player and skates at them
void PurpleBrain::phaseChase(int phase)
{
	switch (phase) {
	case 0:
		aim();
		mNext = 1;
		mStep = 0;
		break;
	case 2:
		look();
		if (mStep == 1) {
			if (mOwner->wasBounced() != 0) {
				changeState(STATE_BOUNCED);
			} else if (mOwner->hitWall() != 0) {
				changeState(STATE_DASH);
			}
		}
		break;
	case 3:
		break;
	case 1:
		aim();
		switch (mStep) {
		case 0:
			if (mOwner->isAction1Done() != 0) {
				mStep = 1;
				mNext = 2;
			}
			break;
		case 1:
			break;
		}
		break;
	}
}

// 0x800CB838
void PurpleBrain::phaseBounced(int phase)
{
	switch (phase) {
	case 0:
		mNext = 3;
		break;
	case 2:
		if (mOwner->isAction3Done() != 0) {
			changeState(STATE_CHASE);
		}
		break;
	case 3:
		break;
	}
}

// 0x800CB8D8
// Dashes back or drops the ice block is dazed for 300 frames and gets up with a slam
void PurpleBrain::phaseDash(int phase)
{
	switch (phase) {
	case 0:
		if (mState == STATE_DASH) {
			mNext = 4;
		} else {
			mNext = 10;
		}
		mStep = 0;
		break;
	case 2:
		if (mStep == 3 && mOwner->isAction6Done() != 0) {
			if (mState == STATE_DASH) {
				changeState(STATE_FREEZE);
			} else {
				changeState(STATE_CHASE);
			}
		}
		break;
	case 3:
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mOwner->isAction4Done() != 0 || mOwner->isAction10Done() != 0) {
				mTimer = 300;
				mStep  = 1;
				mNext  = 5;
			}
			break;
		case 1:
			mTimer--;
			if (mTimer <= 0) {
				mStep = 3;
				mNext = 6;
			} else if (mOwner->wasHitHard() != 0) {
				mStep = 2;
				mNext = 11;
			}
			break;
		case 2:
			mTimer--;
			if (mOwner->isAction11Done() != 0) {
				if (mTimer <= 0) {
					mStep = 3;
					mNext = 6;
				} else {
					mStep = 1;
					mNext = 5;
				}
			}
			break;
		}
		break;
	}
}

// 0x800CBAF4
// Freezes into the ice block and spins up for 120 frames
void PurpleBrain::phaseFreeze(int phase)
{
	switch (phase) {
	case 0:
		mNext  = 7;
		mStep  = 0;
		mTimer = 0;
		break;
	case 2:
		look();
		if (mStep == 1 && mTimer <= 0) {
			changeState(STATE_SPIN);
		}
		break;
	case 3:
		break;
	case 1:
		aim();
		switch (mStep) {
		case 0:
			if (mOwner->isAction7Done() != 0) {
				mStep  = 1;
				mTimer = 120;
				mNext  = 8;
			}
			break;
		case 1:
			mTimer--;
			break;
		}
		break;
	}
}

// 0x800CBC0C
// Spins round the rink Each wall it hits throws ice balls
void PurpleBrain::phaseSpin(int phase)
{
	switch (phase) {
	case 0:
		mNext = 9;
		mStep = 0;
		break;
	case 2:
		if (mOwner->wasBumped() != 0) {
			throwBalls();
			mStep++;
			int stage = getStage(gPurpleBossParams->mHealthSteps);
			if (mStep >= gPurpleBossParams->mSpinCount[stage]) {
				changeState(STATE_DROP);
			}
		}
		break;
	case 3:
		break;
	case 1:
		setSpeed();
		break;
	}
}

// 0x800CBD00
void PurpleBrain::phaseDie(int phase)
{
	switch (phase) {
	case 0:
			mNext = mOwner->dieAction();
		break;
	case 1:
		break;
	case 2:
		break;
	}
}
