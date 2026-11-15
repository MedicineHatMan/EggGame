#include "io/ResBank.h"
#include "enemy/boss/EneBoss.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyAppear.h"
#include "enemy/EnemySight.h"
#include "enemy/EnemyLight.h"
#include "engine/MotionPose.h"
#include "enemy/boss/BossParts.h"
#include "egg/ObjEgg.h"
#include "object/ObjCommon.h"
#include "enemy/TEneAm01Parts.h"
#include "engine/FxHolder.h"
#include "sound/SeHolder.h"
#include "enemy/boss/TEneLastBossDarkCtrl.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "enemy/EnemyHit.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "camera/TCamPath.h"
#include "engine/FxSystem.h"
#include "enemy/ObjEnemy.h"
#include "object/ObjShadow.h"
#include "enemy/EnemyFx.h"
#include "camera/TCameraCtrl.h"
#include "enemy/EnemyTexFx.h"
#include "core/dolphin.h"
#include "enemy/boss/LastBossRes.h"
#include "enemy/boss/TEneLastBoss.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "enemy/boss/TEneLastBossFeather_Home.h"
#include "object/last/TObjLastBossInd.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "engine/TParticle.h"
#include "enemy/boss/TEneLastBossDemo_Darkness.h"
#include "core/TMainTask.h"
#include "enemy/TEneAm01.h"

// 0x80159938
void LastBossAiBase::tick(TEneLastBoss* owner)
{
	mOwner = owner;
	mNext  = -1;
	if (!checkInterrupt()) {
		onPhase(mState, 2);
	}
	onPhase(mState, 1);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x801599D0
LastBossAiState::LastBossAiState()
{
	mOwner = 0;
}

// 0x80159A04
LastBossAiState::~LastBossAiState()
{
}

// 0x80159A60
extern "C" LastBossAi* createLastBossAi(TEneLastBoss* owner)
{
	return new LastBossAi(owner);
}

// 0x80159AA8
LastBossAi::LastBossAi(TEneLastBoss* owner)
{
	mOwner      = owner;
	mHealthMark = sLastBossParams[3];
	mToggle     = 0;
	mDarks      = 0;
	(&mSight)->reset();
	mSight.mHeightLimit = 1000.0f;
	mStep               = 0;
	mCount              = 0;
	mCount2             = 0;
	mAngle              = 0;
	mAngleStep          = 0;
	if (mOwner != 0) {
		mNext  = -1;
		mState = 0;
		onPhase(mState, 0);
		if (mNext != -1) {
			mOwner->setAction(mNext);
		}
	}
}

// 0x80159B88
LastBossAi::~LastBossAi()
{
}

// 0x80159BF0
int LastBossAi::checkInterrupt()
{
	if (mOwner->isDead() && mState != 7) {
		changeState(7);
		return 1;
	}
	if (!mOwner->wasHitHard() && mOwner->wasHitKind1()) {
		mOwner->onLightHit();
	}
	return 0;
}

// 0x80159CA4
void LastBossAi::track()
{
	(&mSight)->findInRange(100000000.0f, mOwner->getPos());
}

// 0x80159CFC
void LastBossAi::aimAtTarget()
{
	mOwner->setAim(&mSight.mTarget, mSight.mAngle);
}

// 0x80159D2C
void LastBossAi::rememberHealth()
{
	mHealthMark = mOwner->mHealth;
}

// 0x80159D3C
int LastBossAi::lostEnough()
{
	int health = mOwner->mHealth;
	int phase  = thresholdIndex(health, sLastBossParams);

	return mHealthMark - health > sLastBossParams[phase + 4];
}

// 0x80159DA4
int LastBossAi::getPhase()
{
	return thresholdIndex(mOwner->mHealth, sLastBossParams);
}

// 0x80159DD0
int LastBossAi::getDarkCount()
{
	return sLastBossParams[getPhase() + 7];
}

// 0x80159E00
int LastBossAi::getCircleFrames()
{
	return sLastBossParams[getPhase() + 10];
}

// 0x80159E30
int LastBossAi::getHomeCount(int shots)
{
	return shots % 3 + 2;
}

// 0x80159E48
int LastBossAi::getHomeShots()
{
	return sLastBossParams[getPhase() + 16];
}

// 0x80159E78
int LastBossAi::getHomeWait()
{
	return sLastBossParams[getPhase() + 19];
}

// 0x80159EA8
int LastBossAi::getStunFrames()
{
	return sLastBossParams[getPhase() + 13];
}

// 0x80159ED8
void LastBossAi::onPhase(int state, int phase)
{
	switch (state) {
	case 0:
		stateIntro(phase);
		break;
	case 1:
		stateWalk(phase);
		break;
	case 2:
		stateCircle(phase);
		break;
	case 3:
		stateSpiral(phase);
		break;
	case 4:
		stateStun(phase);
		break;
	case 5:
		stateHome(phase);
		break;
	case 6:
		stateStun(phase);
		break;
	case 7:
		stateDie(phase);
		break;
	}
}

// 0x80159F70
void LastBossAi::stateIntro(int phase)
{
	switch (phase) {
	case 0:
		mNext = 0;
		mOwner->setMaxHealth(sLastBossParams[3]);
		darkCtrlClearAll();
		lastBossEggClose();
		lastBossIndShow();
		break;
	case 2:
		if (mOwner->isDone0()) {
			changeState(1);
		}
		break;
	case 1:
	case 3: {
		Vec3f pos;

		pos.x = 0.0f;
		pos.y = 20.0f;
		pos.z = -160.0f;
		mOwner->setPos(&pos);
		break;
	}
	}
}

// A random number in [-1 1)
static inline f32 randSigned()
{
	return 2.0f * (3.0517578E-5f * (f32)rand() - 0.5f);
}

// 0x8015A06C
void LastBossAi::stateWalk(int phase)
{
	switch (phase) {
	case 0: {
		Vec3f pos;

		mNext = 2;
		randomOnRingTEneLastBoss(&pos, 10.0f * randSigned() + 160.0f);
		mOwner->setMoveTo(&pos);
		break;
	}
	case 2:
		if (mOwner->hasArrived()) {
			if (getPhase() <= 0) {
				mToggle = 0;
				changeState(2);
			} else if (mToggle == 0) {
				mToggle = 1;
				changeState(5);
			} else {
				mToggle = 0;
				changeState(2);
			}
		}
		break;
	case 1:
		track();
		aimAtTarget();
		break;
	case 3:
		break;
	}
}

// 0x8015A230
void LastBossAi::stateCircle(int phase)
{
	int* angle = &mAngle;

	switch (phase) {
	case 0:
		darkCtrlClearAll();
		darkCtrlClearNoContact();
		mDarks = getDarkCount();
		mOwner->setDarkCount(mDarks);
		mNext = 3;
		mStep = 0;
		break;
	case 2:
		if (mStep == 2) {
			changeState(3);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		switch (mStep) {
		case 0:
			if (mOwner->isDone3()) {
				Vec3f pos;
				f32 radius;

				mStep   = 1;
				mCount  = getCircleFrames();
				mCount2 = mCount;
				mNext   = 2;
				pos     = *mOwner->getPos();
				polarOfTEneLastBoss(&pos, &radius, angle);
				mAngleStep = 0x8000 / mCount;
				if (3.0517578E-5f * (f32)rand() >= 0.5f) {
					mAngleStep = -mAngleStep;
				}
			}
			break;
		case 1: {
			Vec3f pos;
			f32 rate;

			*angle += mAngleStep;
			fromPolarTEneLastBoss(160.0f, &pos, *angle);
			mOwner->setMoveTo(&pos);
			if (mCount >= mCount2 / 2) {
				rate = 0.5f;
			} else if (mCount >= mCount2 / 4) {
				rate = 1.0f;
			} else if (mCount >= mCount2 / 8) {
				rate = 2.0f;
			} else {
				rate = 4.0f;
			}
			darkCtrlSetPhaseRate(rate);
			mCount--;
			if (mCount <= 0) {
				mStep = 2;
			}
			break;
		}
		}
		break;
	case 3:
		break;
	}
}

// 0x8015A494
void LastBossAi::stateSpiral(int phase)
{
	switch (phase) {
	case 0:
		mNext = 4;
		mStep = 0;
		break;
	case 2:
		if (mStep == 4) {
			changeState(4);
		} else if (mStep == 5 && mOwner->isDone5()) {
			changeState(1);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		switch (mStep) {
		case 0:
			if (mOwner->isDone4()) {
				darkCtrlSetNoContact();
				darkCtrlFadeMaps();
				mStep  = 1;
				mCount = 0x3C;
			}
			break;
		case 1:
			mCount--;
			if (mCount <= 0) {
				mNext = 5;
				mStep = 2;
			}
			break;
		case 2:
			if (mOwner->hasLanded()) {
				if (darkCtrlCountMaps() <= 0) {
					mStep = 4;
				} else {
					mStep   = 3;
					mCount  = 0;
					mCount2 = 0;
				}
			}
			break;
		case 3:
			mCount--;
			if (mCount <= 0) {
				Vec3f pos;
				f32 rotY;

				if (darkCtrlGetMap(mCount2, &pos, &rotY)) {
					int left;

					createLastBossMagic(mOwner, &pos, rotY);
					mCount2++;
					left = darkCtrlCountMaps();
					if (mCount2 >= left) {
						if (left < mDarks) {
							mStep = 4;
						} else {
							mStep = 5;
						}
						darkCtrlFadeOut();
					} else {
						mCount = 0xA;
					}
				} else {
					mStep = 4;
				}
			}
			break;
		}
		break;
	case 3:
		darkCtrlFadeOut();
		break;
	}
}

// 0x8015A710
void LastBossAi::stateHome(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setHomeCount(getHomeCount(0));
		mNext = 6;
		mOwner->setGuard();
		mStep   = 0;
		mCount2 = 0;
		mCount  = 0;
		break;
	case 2:
		if (mOwner->wasHitHard()) {
			changeState(6);
		} else if (mOwner->isDone7()) {
			changeState(1);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		switch (mStep) {
		case 0:
			if (mOwner->isDone6()) {
				mCount2++;
				if (mCount2 >= getHomeShots()) {
					mNext = 7;
					mStep = 2;
				} else {
					mStep  = 1;
					mCount = getHomeWait();
				}
			}
			break;
		case 1:
			mCount--;
			if (mCount <= 0) {
				mOwner->setHomeCount(getHomeCount(mCount2));
				mNext = 6;
				mStep = 0;
			}
			break;
		case 2:
			break;
		}
		break;
	case 3:
		mOwner->clearGuard();
		break;
	}
}

// 0x8015A914
int LastBossAi::checkStunHits(int* timer)
{
	int hit = 0;

	if (mOwner->wasHitKind3()) {
		hit = 1;
		mOwner->onHardHit();
	}
	if (lostEnough()) {
		mOwner->clearOpen();
		*timer = 0;
	}
	return hit;
}

// 0x8015A984
void LastBossAi::stateStun(int phase)
{
	int* timer = &mCount;

	switch (phase) {
	case 0:
		if (mState == 4) {
			mNext = 8;
		} else {
			mNext = 9;
		}
		rememberHealth();
		mOwner->setOpen();
		mOwner->setUntouchable();
		mStep  = 0;
		*timer = getStunFrames();
		break;
	case 2:
		if (mOwner->isDone11()) {
			changeState(1);
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			checkStunHits(timer);
			if (mOwner->isDone8() || mOwner->isDone9()) {
				mNext = 0xA;
				mStep = 1;
			}
			break;
		case 1:
			(*timer)--;
			if (checkStunHits(timer)) {
				if (*timer > 0) {
					mStep = 2;
					mNext = 0xC;
				}
			} else if (*timer <= 0) {
				mOwner->clearOpen();
				mNext = 0xB;
				mStep = 3;
			}
			break;
		case 2:
			(*timer)--;
			if (checkStunHits(timer)) {
				if (*timer > 0) {
					mStep = 2;
					mNext = 0xC;
				}
			} else if (mOwner->isDone12()) {
				mNext = 0xA;
				mStep = 1;
			}
			break;
		case 3:
			break;
		}
		break;
	case 3:
		mOwner->clearOpen();
		mOwner->setTouchable();
		break;
	}
}

// 0x8015ABB0
void LastBossAi::stateDie(int phase)
{
	switch (phase) {
	case 0:
		mNext = 0xD;
		darkCtrlFadeOut();
		break;
	case 1:
	case 2:
	case 3:
		break;
	}
}
