#include "io/ResBank.h"
#include "enemy/ObjEnemy.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyAppear.h"
#include "enemy/EnemyCommon.h"
#include "enemy/EnemySight.h"
#include "enemy/EnemyLight.h"
#include "engine/MotionPose.h"
#include "enemy/boss/BossParts.h"
#include "object/ObjCommon.h"
#include "enemy/TEneAm01Parts.h"
#include "engine/FxHolder.h"
#include "egg/ObjEgg.h"
#include "sound/SeHolder.h"
#include "enemy/boss/TEneYellowBossCrystal.h"
#include "object/ObjAction.h"
#include "enemy/EnemyHit.h"
#include "object/last/TObjLastBossEgg_Frag.h"
#include "enemy/boss/TEneYellowBossGreen.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "core/MathUtil.h"
#include "object/ObjShadow.h"
#include "enemy/EnemyFx.h"
#include "camera/TCameraCtrl.h"
#include "enemy/boss/TEneYellowBossDemo_Gate.h"
#include "object/TObject.h"
#include "enemy/EnemyTexFx.h"
#include "object/TObjLauncher.h"
#include "object/ObjModel.h"
#include "engine/DrawState.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "ui/AeArc.h"
#include "camera/TCamPath.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "engine/TParticle.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "enemy/boss/TEneYellowBossBlue.h"
#include "enemy/boss/TEneYellowBossOrange.h"
#include "enemy/boss/TEneYellowBossPurple.h"
#include "enemy/boss/TEneYellowBossRed.h"
#include "enemy/TEneAm01.h"

// 0x8014C904
void YellowBossAiBase::tick(TEneYellowBoss* owner)
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

// 0x8014C99C
YellowBossAiState::YellowBossAiState()
{
	mOwner = 0;
}

// 0x8014C9D0
YellowBossAiState::~YellowBossAiState()
{
}

// 0x8014CA2C
extern "C" YellowBossAi* createYellowBossAi(TEneYellowBoss* owner)
{
	return new YellowBossAi(owner);
}

// 0x8014CA74
YellowBossAi::YellowBossAi(TEneYellowBoss* owner)
{
	int i;

	mOwner      = owner;
	mHealthMark = sYellowBossParams[0];
	mFlags      = 0;
	(&mSight)->reset();
	mSight.mHeightLimit = 1000.0f;
	mNextPart           = 0;
	for (i = 0; i < 8; i++) {
		mCopy[i] = 0;
	}
	mParent     = 0;
	mDest.x     = 0.0f;
	mDest.y     = 0.0f;
	mDest.z     = 0.0f;
	_7C         = 0;
	mWaitFrames = 0;
	mEndFrames  = 0;
	mStep       = 0;
	mCount      = 0;
	_94         = 0;
	if (mOwner != 0) {
		mNext  = -1;
		mState = 0;
		onPhase(mState, 0);
		if (mNext != -1) {
			mOwner->setAction(mNext);
		}
	}
}

// 0x8014CB8C
YellowBossAi::~YellowBossAi()
{
	removeCopies();
}

// 0x8014CBF8
int YellowBossAi::checkInterrupt()
{
	if (mFlags & AI_COPY) {
		if (mFlags & AI_VANISH) {
			mFlags &= ~AI_VANISH;
			changeState(6);
		} else if (mOwner->wasHitKind1()) {
			mOwner->onLightHit();
		}
	} else {
		if (mOwner->isDead() && mState != 11) {
			changeState(11);
			return 1;
		}
		if (!mOwner->wasHitHard() && mOwner->wasHitKind1()) {
			mOwner->onLightHit();
		}
	}
	return 0;
}

// 0x8014CD24
void YellowBossAi::track()
{
	(&mSight)->findInRange(100000000.0f, mOwner->getPos());
}

// 0x8014CD7C
void YellowBossAi::aimAtTarget()
{
	mOwner->setAim(&mSight.mTarget, mSight.mAngle);
}

// 0x8014CDAC
int YellowBossAi::getPhase()
{
	return thresholdIndex(mOwner->mHealth, sYellowBossParams + 1);
}

// 0x8014CDDC
void YellowBossAi::rememberHealth()
{
	mHealthMark = mOwner->mHealth;
}

// 0x8014CDEC
int YellowBossAi::lostHealth(const int* table)
{
	return healthDropReached(mHealthMark, mOwner->mHealth, sYellowBossParams + 1, table);
}

// 0x8014CE24
int YellowBossAi::lostHealthA()
{
	return lostHealth(sYellowBossParams + 7);
}

// 0x8014CE4C
int YellowBossAi::lostHealthB()
{
	return lostHealth(sYellowBossParams + 16);
}

// 0x8014CE74
void YellowBossAi::startTimer()
{
	mTimer = sYellowBossParams[getPhase() + 13];
}

// 0x8014CEB4
int YellowBossAi::randomWait()
{
	int wait = (int)(180.0f * (3.0517578E-5f * (f32)rand()));

	if (wait >= 180) {
		wait = 179;
	}
	return wait + 120;
}

// 0x8014CF1C
int YellowBossAi::getCopyCount()
{
	int count = sYellowBossParams[getPhase() + 10];

	if (count < 0) {
		return 0;
	}
	return count > 7 ? 7 : count;
}

// 0x8014CF6C
void YellowBossAi::setDest(const Vec3f* dest, int wait, int end)
{
	mDest       = *dest;
	mWaitFrames = wait;
	mEndFrames  = end;
}

// 0x8014CF90
void YellowBossAi::removeCopies()
{
	int i;

	if (mParent != 0) {
		mParent->forgetCopy(this);
	}
	for (i = 0; i < 8; i++) {
		if (mCopy[i] != 0) {
			mCopy[i]->forgetParent(this);
			mCopy[i] = 0;
		}
	}
}

// 0x8014D004
YellowBossAi* YellowBossAi::spawnCopy(const Vec3f* pos, int rotY, const Vec3f* dest, int wait, int end)
{
	YellowBossAi* ai     = new YellowBossAi(0);
	TEneYellowBoss* boss = new TEneYellowBoss(mOwner, ai);

	boss->setPos((Vec3f*)pos);
	boss->mBody.mRot.y = rotY;
	ai->mOwner         = boss;
	ai->setDest(dest, wait, end);
	ai->becomeCopy(this);
	return ai;
}

// A random index below n
static inline int randomIndex(int n)
{
	int r = (int)((f32)n * (3.0517578E-5f * (f32)rand()));
	if (r >= n) {
		r = n - 1;
	}
	return r;
}

// 0x8014D0C4
void YellowBossAi::splitIntoCopies(int count)
{
	int pick = randomIndex(count);
	Vec3f from;
	Vec3f to;
	int angle;
	int spread;
	int rotY;
	int wait;
	int end;
	int i;
	f32 height;

	from   = *mOwner->getPos();
	rotY   = mOwner->getRotY();
	spread = (count - 1) << 12;
	to     = from;
	angle  = (int)(10430.38043493439 * (f32)atan2(from.x, from.z));
	angle += 0x8000;
	angle += spread / 2;
	height = 50.02f;
	end    = (count - 1) * 5 + 0x3A;
	wait   = 0x1E;
	for (i = 0; i < count; i++) {
		vecFromAngleXZ(180.0f, &to, angle);
		to.y = height;
		if (i == pick) {
			setDest(&to, wait, end);
		} else if (mCopy[i] == 0) {
			mCopy[i] = spawnCopy(&from, rotY, &to, wait, end);
		}
		angle -= spread;
		spread = spread > 0 ? 0x1000 - spread : -0x1000 - spread;
		wait += 5;
		end += 0x28;
	}
}

// 0x8014D2A0
void YellowBossAi::vanishCopies()
{
	int i;

	for (i = 0; i < 8; i++) {
		if (mCopy[i] != 0) {
			mCopy[i]->vanish();
		}
	}
}

// 0x8014D2F4
int YellowBossAi::copiesWaiting()
{
	int all = 1;
	int i;

	for (i = 0; i < 8; i++) {
		if (mCopy[i] != 0 && !(mCopy[i]->mFlags & AI_WAITING)) {
			all = 0;
			break;
		}
	}
	return all;
}

// 0x8014D338
void YellowBossAi::releaseCopies()
{
	int i;

	for (i = 0; i < 8; i++) {
		if (mCopy[i] != 0) {
			mCopy[i]->mFlags |= AI_RELEASE;
		}
	}
}

// 0x8014D36C
void YellowBossAi::forgetCopy(YellowBossAi* copy)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (mCopy[i] == copy) {
			mCopy[i] = 0;
		}
	}
}

// 0x8014D39C
void YellowBossAi::becomeCopy(YellowBossAi* parent)
{
	mFlags |= AI_COPY;
	mParent = parent;
	mOwner->setCopy();
	mNext  = -1;
	mState = 5;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x8014D41C
void YellowBossAi::vanish()
{
	mFlags |= AI_VANISH;
}

// 0x8014D42C
void YellowBossAi::forgetParent(YellowBossAi* parent)
{
	if (mParent == parent) {
		mParent = 0;
	}
}

// 0x8014D444
void YellowBossAi::onPhase(int state, int phase)
{
	switch (state) {
	case 0: stateIntro(phase); break;
	case 1: stateSummon(phase); break;
	case 2: stateOpen(phase); break;
	case 3: stateRecover(phase); break;
	case 4: stateSplit(phase); break;
	case 5: stateCopyAppear(phase); break;
	case 6: stateCopyGone(phase); break;
	case 7: stateBarrage(phase); break;
	case 8: stateDash(phase); break;
	case 9: stateCopyWait(phase); break;
	case 10: stateTaunt(phase); break;
	case 11: stateDying(phase); break;
	}
}

// 0x8014D50C
void YellowBossAi::stateIntro(int phase)
{
	switch (phase) {
	case 0:
		mNext = 0;
		mOwner->setMaxHealth(sYellowBossParams[0]);
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
		pos.y = 50.02f;
		pos.z = 180.0f;
		mOwner->setMoveTo(&pos, 0x8000);
		break;
	}
	}
}

// 0x8014D5F8
void YellowBossAi::stateSummon(int phase)
{
	switch (phase) {
	case 0:
		mNext  = 1;
		mStep  = 0;
		mCount = 0x78;
		break;
	case 2:
		if (mOwner->isDone3()) {
			changeState(2);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		switch (mStep) {
		case 0:
			if (mOwner->isDone1()) {
				mStep = 1;
				mOwner->summonPart(mNextPart);
				mNext = 2;
				switch (mNextPart) {
				case 0:
				default:
					mNextPart = 1;
					break;
				case 1:
					mNextPart = 2;
					break;
				case 2:
					mNextPart = 3;
					break;
				case 3:
					mNextPart = 4;
					break;
				case 4:
					mNextPart = 0;
					break;
				}
			}
			break;
		case 1:
			if (mOwner->isPartsDone()) {
				mStep = 2;
				mNext = 3;
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8014D78C
void YellowBossAi::stateOpen(int phase)
{
	switch (phase) {
	case 0:
		mStep  = 0;
		mCount = sYellowBossParams[getPhase() + 4];
		mNext  = 4;
		rememberHealth();
		mOwner->setOpen();
		break;
	case 2:
		if (mStep == 3) {
			changeState(3);
		}
		break;
	case 1:
		if (mOwner->wasHitHard()) {
			if (lostHealthA()) {
				mCount = 0;
				mOwner->onHardHit();
				mOwner->clearOpen();
			} else if (mStep != 0 && mCount > 0) {
				mStep = 2;
				mNext = 0x12;
			} else {
				mOwner->onHardHit();
			}
		}
		switch (mStep) {
		case 0:
			if (mOwner->isDone4()) {
				mStep = 1;
				mNext = 5;
			} else if (mCount <= 0) {
				mStep = 3;
			}
			break;
		case 1:
			mCount--;
			if (mCount <= 0) {
				mStep = 3;
			}
			break;
		case 2:
			mCount--;
			if (mOwner->isDone18()) {
				mStep = 1;
			}
			break;
		}
		break;
	case 3:
		mOwner->clearOpen();
		break;
	}
}

// 0x8014D98C
void YellowBossAi::stateRecover(int phase)
{
	switch (phase) {
	case 0:
		mNext = 6;
		break;
	case 2:
		if (mOwner->isDone6()) {
			if (getPhase() == 0) {
				changeState(1);
			} else {
				changeState(4);
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

// 0x8014DA94
void YellowBossAi::stateSplit(int phase)
{
	switch (phase) {
	case 0:
		mNext = 7;
		break;
	case 2:
		if (mOwner->isDone7()) {
			splitIntoCopies(getCopyCount());
			changeState(5);
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

// 0x8014DB5C
void YellowBossAi::stateCopyAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext  = 8;
		mStep  = 0;
		mCount = 0;
		break;
	case 2:
		if (mCount >= mEndFrames) {
			changeState(7);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		mCount++;
		switch (mStep) {
		case 0:
			if (mCount > mWaitFrames) {
				mStep = 1;
				mOwner->setTravelTime(8.0f);
				mOwner->setMoveTo(&mDest);
				mNext = 9;
			}
			break;
		case 1:
			if (mOwner->isDone9()) {
				mNext = 8;
				mStep = 2;
			}
			break;
		case 2:
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8014DCA0
void YellowBossAi::stateBarrage(int phase)
{
	switch (phase) {
	case 0:
		mNext = 0xB;
		rememberHealth();
		startTimer();
		break;
	case 2:
		if (mOwner->isDone11()) {
			changeState(8);
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

// 0x8014DD60
void YellowBossAi::stateDash(int phase)
{
	switch (phase) {
	case 0:
		mNext  = 0xC;
		mStep  = 0;
		mCount = 0;
		mOwner->setSolid();
		break;
	case 2:
		if (mOwner->isDone14()) {
			if (mFlags & AI_COPY) {
				changeState(9);
			} else if (mTimer < 0) {
				vanishCopies();
				changeState(1);
			} else {
				changeState(9);
			}
		}
		break;
	case 1:
		track();
		aimAtTarget();
		mTimer--;
		switch (mStep) {
		case 0:
			if (mOwner->isDone12()) {
				if (mSight.mDistSq < 2500.0f) {
					mStep = 2;
					mNext = 0xE;
				} else {
					mStep  = 1;
					mCount = 0x14;
					mNext  = 0xD;
				}
			}
			break;
		case 1:
			mCount--;
			if (mCount <= 0) {
				mStep = 0;
				mNext = 0xC;
			}
			break;
		case 2:
			break;
		}
		break;
	case 3:
		mOwner->clearSolid();
		break;
	}
}

// 0x8014DF88
void YellowBossAi::stateCopyWait(int phase)
{
	switch (phase) {
	case 0:
		mNext = 0xF;
		if (mFlags & AI_COPY) {
			mStep  = 1;
			mCount = 0x1C20;
		} else {
			mStep = 0;
		}
		mOwner->setOpen();
		mFlags |= AI_WAITING;
		mFlags &= ~AI_RELEASE;
		break;
	case 2:
		if (mOwner->wasHitHard()) {
			if (mFlags & AI_COPY) {
				changeState(6);
			} else {
				changeState(10);
				vanishCopies();
			}
		} else if (mOwner->isDone16()) {
			changeState(8);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		mTimer--;
		switch (mStep) {
		case 0:
			if (copiesWaiting()) {
				releaseCopies();
				mStep  = 2;
				mCount = randomWait();
			}
			break;
		case 1:
			mCount--;
			if (mCount <= 0 || (mFlags & AI_RELEASE)) {
				mStep  = 2;
				mCount = randomWait();
			}
			break;
		case 2:
			mCount--;
			if (mCount <= 0) {
				mStep = 3;
				mNext = 0x10;
			}
			break;
		case 3:
			break;
		}
		break;
	case 3:
		mOwner->clearOpen();
		mFlags &= ~AI_WAITING;
		break;
	}
}

// 0x8014E214
void YellowBossAi::stateTaunt(int phase)
{
	switch (phase) {
	case 0:
		mNext  = 0x13;
		mStep  = 0;
		mCount = mTimer;
		mOwner->setOpen();
		break;
	case 2:
		if (mStep == 2) {
			changeState(1);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		mCount--;
		if (mOwner->wasHitHard()) {
			if (lostHealthB()) {
				mCount = 0;
				mOwner->onHardHit();
				mOwner->clearOpen();
			} else if (mCount > 0) {
				mStep = 0;
				mNext = 0x13;
			} else {
				mOwner->onHardHit();
			}
		}
		switch (mStep) {
		case 0:
			if (mOwner->isDone19()) {
				mStep = 1;
				mNext = 0x11;
			}
			break;
		case 1:
			if (mCount <= 0) {
				mStep = 2;
			}
			break;
		}
		break;
	case 3:
		mOwner->clearOpen();
		break;
	}
}

// 0x8014E3B0
void YellowBossAi::stateDying(int phase)
{
	switch (phase) {
	case 0:
		mNext = 0x14;
		vanishCopies();
		break;
	case 1:
		track();
		aimAtTarget();
		break;
	case 2:
		break;
	}
}

// 0x8014E410
void YellowBossAi::stateCopyGone(int phase)
{
	switch (phase) {
	case 0:
		mNext = 0xA;
		removeCopies();
		break;
	case 1:
		track();
		aimAtTarget();
		break;
	case 2:
		break;
	}
}
