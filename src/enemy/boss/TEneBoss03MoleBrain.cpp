#include "enemy/boss/TEneBoss03Mole.h"
#include "camera/TCamera.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "core/MathUtil.h"
#include "enemy/EnemyFx.h"
#include "camera/TCameraCtrl.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "stl/math.h"
#include "animal/RacerMover.h"

static inline f32 random01()
{
	return 3.0517578e-5f * rand();
}

// 0x8007C134
MoleBrainBase::MoleBrainBase() { }

// 0x8007C164
MoleBrainBase::~MoleBrainBase() { }

// 0x8007C1C0
MoleBrain* createMoleBrain(TEneBoss03Mole* owner)
{
	return new MoleBrain(owner);
}

// 0x8007C208
MoleBrain::MoleBrain(TEneBoss03Mole* owner)
{
	mSight.reset();
	_40          = 0.0f;
	_44          = 0;
	_48          = 0;
	_50          = 0;
	mSavedHealth = 0;
	mTurn        = 0;
	mWave        = 0;
	mCamera      = 0;
	mStep        = 0;
	mTimer       = 0;
	mCount       = 0;
	mDigs        = 0;
	start(owner);
}

// 0x8007C2DC
MoleBrain::~MoleBrain()
{
	endCamera();
}

// 0x8007C348
// Slot _08
int MoleBrain::checkInterrupt()
{
	int changed = 0;
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		changed = 1;
	} else if (mOwner->wasHitKind1() != 0) {
		mOwner->flinch();
	}
	return changed;
}

// 0x8007C3F0
// Three to four seconds
int MoleBrain::randomWait()
{
	return (int)(60.0f * (3.0517578e-5f * rand()) + 180.0f);
}

// 0x8007C44C
// Looks for a player in a box ahead of it and a little to the side
int MoleBrain::look()
{
	Vec3f dir;
	mOwner->getDinoNode0(&dir);
	vecNormalizeXZ(&dir, &dir);
	Vec3f pos;
	pos = dir;
	gjScaleVector(140.0f, &pos, &pos);
	pos.x = 40.0f * dir.z + pos.x;
	pos.z = pos.z - 40.0f * dir.x;
	f32 angle = atan2(pos.x, pos.z);
	pos.y = 200.0f;
	return mSight.findInBox(&pos, (int)(10430.38043493439 * angle), 25.0f, 60.000004f);
}

// 0x8007C51C
// 0 while healthy up to 3 as it is hurt
int MoleBrain::getStage()
{
	return thresholdIndex(mOwner->mHealth, gMoleParams->mHealthSteps);
}

// 0x8007C548
int MoleBrain::getDigCount()
{
	int stage = getStage();
	return gMoleParams->mDigCount[stage];
}

// 0x8007C578
int MoleBrain::getAttack()
{
	int stage = getStage();
	return gMoleParams->mAttack[stage];
}

// 0x8007C5A8
// Circles so as to head towards a player within 200 of the middle if there is one
void MoleBrain::startCircle()
{
	Vec3f center;
	mTurn = 0x95;
	center.x = 0.0f;
	center.y = 200.0f;
	center.z = 0.0f;
	if (mSight.findInRange(40000.0f, &center) != 0) {
		Vec3f target;
		target = mSight.mTarget;
		Vec3f pos;
		pos = *mOwner->getPos();
		njSubVector(&target, &center);
		target.y = 0.0f;
		njSubVector(&pos, &center);
		pos.y = 0.0f;
		if (vecCrossXZ(&pos, &target) >= 0.0f) {
			mTurn = -0x95;
		} else {
			mTurn = 0x95;
		}
	}
	mWave = (int)(65536.0f * random01());
	int stage = getStage();
	mOwner->setRate(gMoleParams->mCircleSpeed[stage]);
}

// 0x8007C70C
// Steers round the middle 120 to 160 out
void MoleBrain::circle()
{
	f32 radius = 20.0f * njSin(mWave) + 140.0f;
	mWave += 0x180;
	Vec3f dir;
	dir = *mOwner->getPos();
	vecNormalizeXZ(&dir, &dir);
	vecRotateY(&dir, &dir, mTurn);
	gjScaleVector(radius, &dir, &dir);
	dir.y = 200.0f;
	mOwner->setTarget(&dir);
}

// 0x8007C7D0
int MoleBrain::isNearGoal()
{
	Vec3f pos;
	pos = *mOwner->getPos();
	Vec3f goal;
	mOwner->getDinoMouth(&goal);
	return distXZ(&pos, &goal) < 140.0f;
}

// 0x8007C84C
void MoleBrain::startCamera()
{
	Vec3f pos;
	pos = *mOwner->getPos();
	mCamera = createTCamConversation(0, 2, &pos, 0x400, 0x1000);
	if (mCamera != 0) {
		mCamera->setTarget(camGetTarget(0));
		((TCamera*)mCamera)->setBlendStep(0.001f);
	}
}

// 0x8007C8F4
void MoleBrain::updateCamera()
{
	if (mCamera != 0) {
		Vec3f pos;
		pos = *mOwner->getPos();
		mCamera->setFocus(&pos);
	} else {
		startCamera();
	}
}

// 0x8007C968
void MoleBrain::endCamera()
{
	if (mCamera != 0) {
		if (mCamera != 0) {
			mCamera->mFlags |= 1;
			mCamera = 0;
		}
		camSetFar(0.001f, 0);
	}
}

// 0x8007C9B4
void MoleBrain::setSpeed()
{
	int stage = getStage();
	mOwner->setDinoRate(gMoleParams->mSpeed[stage]);
}

// 0x8007C9F8
void MoleBrain::saveHealth()
{
	mSavedHealth = mOwner->mHealth;
}

// 0x8007CA08
// Whether the health has dropped past a step since saveHealth
int MoleBrain::isHurtStep()
{
	return healthDropReached(mSavedHealth, mOwner->mHealth, gMoleParams->mHealthSteps, gMoleParams->mHurtDrop);
}

// 0x8007CA3C
int MoleBrain::checkHurt()
{
	if (isHurtStep() != 0) {
		mOwner->clearGuard();
		return 1;
	}
	return 0;
}

// 0x8007CA84
// Slot _0C
void MoleBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_DIG:
		phaseDig(phase);
		break;
	case STATE_HUNT:
		phaseHunt(phase);
		break;
	case STATE_SHOW:
		phaseShow(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x8007CB04
void MoleBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setMaxHealth(gMoleParams->mAppear);
		mNext = 0;
		mStep = 0;
		break;
	case 2:
		if (mOwner->dinoFaceSideDone() != 0) {
			changeState(STATE_DIG);
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mOwner->isAppearDone() != 0) {
				mStep = 1;
				mNext = 5;
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

// 0x8007CBFC
void MoleBrain::phaseDig(int phase)
{
	switch (phase) {
	case 0:
		mStep = 0;
		mNext = 1;
		setSpeed();
		mTimer = randomWait();
		mCount = getDigCount();
		mDigs  = 0;
		break;
	case 2:
		if (mStep == 6) {
			changeState(STATE_HUNT);
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mOwner->dinoAtRing() != 0) {
				mTimer--;
				if (mTimer < 0) {
					mNext = 4;
					mStep = 1;
				}
			}
			break;
		case 1:
			if (mOwner->dinoFaceInDone() != 0) {
				if (mDigs == 0) {
					mDigs++;
					mStep = 2;
					mNext = 9;
				} else {
					mStep = 3;
					mNext = 6;
				}
			}
			break;
		case 2:
			if (mOwner->dinoSwipeDone() != 0) {
				mStep = 3;
				mNext = 6;
			}
			break;
		case 3:
			if (mOwner->dinoRoarPeaked() != 0) {
				mOwner->burstRocks(getAttack());
				mStep = 4;
			}
			break;
		case 4:
			if (mOwner->dinoRoarDone() != 0) {
				mCount--;
				if (mCount <= 0) {
					mNext = 5;
					mStep = 5;
				} else {
					mStep = 3;
					mNext = 6;
				}
			}
			break;
		case 5:
			if (mOwner->dinoFaceSideDone() != 0) {
				mStep = 6;
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8007CE40
void MoleBrain::phaseHunt(int phase)
{
	switch (phase) {
	case 0:
		mStep  = 0;
		mTimer = 600;
		mCount = 1800;
		mNext  = 2;
		endCamera();
		setSpeed();
		mOwner->clearGuard();
		break;
	case 2:
		if (mStep == 9 && mOwner->dinoBreathEndDone() != 0) {
			switch (getStage()) {
			case 0:
			default:
				changeState(STATE_DIG);
				break;
			case 1:
			case 2:
				changeState(STATE_SHOW);
				break;
			}
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mOwner->dinoAtRing() != 0) {
				mTimer--;
				if (look() != 0 || mTimer <= 0) {
					mStep = 1;
					mNext = 7;
					startCamera();
				}
			}
			break;
		case 1:
			updateCamera();
			if (mOwner->dinoBreathing() != 0) {
				Vec3f dir;
				mOwner->getDinoNode0(&dir);
				dir.y = 0.0f;
				if (njUnitVector(&dir) < gUnitVectorMin) {
					dir = gVecZero;
				}
				dir.x *= -140.0f;
				dir.z *= -140.0f;
				dir.y += 200.0f;
				mOwner->setTarget(&dir);
				mNext = 12;
				mStep = 2;
			}
			break;
		case 2:
			updateCamera();
			if (mOwner->hasLanded() != 0) {
				mTimer = 240;
				mStep  = 3;
				endCamera();
				saveHealth();
				mOwner->setGuard();
			}
			break;
		case 3:
			if (mOwner->wasHitKind3() != 0) {
				mOwner->knockOut();
				checkHurt();
			}
			mTimer--;
			if (mTimer <= 0) {
				mNext = 13;
				mStep = 4;
			}
			break;
		case 4:
			if (mOwner->wasHitKind3() != 0) {
				mOwner->knockOut();
				checkHurt();
			}
			if (mOwner->isDiveDone() != 0) {
				if (isHurtStep() != 0) {
					mStep = 8;
					mNext = 15;
				} else {
					startCircle();
					circle();
					mNext  = 14;
					mStep  = 5;
					mTimer = 0;
				}
			}
			break;
		case 5:
			circle();
			mTimer--;
			mCount--;
			if (mOwner->wasHitKind3() != 0) {
				mOwner->knockOut();
				if (checkHurt() != 0) {
					mStep = 8;
					mNext = 15;
				} else if (mTimer <= 0) {
					mStep = 7;
					mNext = 17;
				}
			} else if (mOwner->isAtRock() != 0) {
				mStep = 6;
				mNext = 16;
			} else if (isNearGoal() != 0 || mCount <= 0) {
				mStep = 8;
				mNext = 15;
				mOwner->clearGuard();
			}
			break;
		case 6:
			mCount--;
			circle();
			if (mOwner->wasHitKind3() != 0) {
				mOwner->knockOut();
				if (checkHurt() != 0) {
					mStep = 8;
					mNext = 15;
				} else {
					mStep = 7;
					mNext = 17;
				}
			} else if (mOwner->isSurfaceDone() != 0) {
				mStep  = 5;
				mTimer = 0;
				mNext  = 14;
			}
			break;
		case 7:
			mCount--;
			if (mOwner->wasHitKind3() != 0) {
				mOwner->knockOut();
				checkHurt();
			}
			if (mOwner->isDownDone() != 0) {
				if (isHurtStep() != 0 || mCount <= 0) {
					mStep = 8;
					mNext = 15;
				} else {
					mStep  = 5;
					mTimer = 60;
					mNext  = 14;
				}
			}
			break;
		case 8:
			if (mOwner->isPounceDone() != 0) {
				mStep = 9;
				mNext = 8;
			}
			break;
		case 9:
			break;
		}
		break;
	case 3:
		endCamera();
		mOwner->clearGuard();
		break;
	}
}

// 0x8007D3CC
void MoleBrain::phaseShow(int phase)
{
	switch (phase) {
	case 0:
		mNext = 3;
		setSpeed();
		mStep  = 0;
		mTimer = randomWait();
		break;
	case 2:
		if (mStep == 4 && mOwner->dinoFaceSideDone() != 0) {
			changeState(STATE_DIG);
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mOwner->dinoAtRing() != 0) {
				mTimer--;
				if (mTimer < 0) {
					mNext = 4;
					mStep = 1;
				}
			}
			break;
		case 1:
			if (mOwner->dinoFaceInDone() != 0) {
				mStep = 2;
				mNext = 9;
			}
			break;
		case 2:
			if (mOwner->dinoSwipeDone() != 0) {
				mStep = 3;
				mNext = 10;
			}
			break;
		case 3:
			if (mOwner->dinoBiteDone() != 0) {
				mStep = 4;
				mNext = 5;
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8007D568
void MoleBrain::phaseDie(int phase)
{
	switch (phase) {
	case 0:
		mNext = 18;
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	}
}
