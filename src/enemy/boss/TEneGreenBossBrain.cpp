#include "enemy/boss/TEneGreenBoss.h"
#include "sound/SeHolder.h"
#include "core/MathUtil.h"
#include "enemy/EnemyFx.h"
#include "camera/TCameraCtrl.h"
#include "object/green/TObjGreenGrassField.h"
#include "core/TMainTask.h"

// Not in the DOL Calling update before its body makes the compiler generate it here at the start of the file rather
// than deferring it to the end
static void greenRootUpdate(GreenBrainRoot* root, TEneGreenBoss* owner) { root->GreenBrainRoot::update(owner); }

// 0x801632C0
inline void GreenBrainRoot::update(TEneGreenBoss* owner)
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

// 0x80163358
GreenBrainBase::GreenBrainBase()
{
	mOwner = 0;
}

// 0x8016338C
GreenBrainBase::~GreenBrainBase() { }

// 0x801633E8
GreenBrain* createGreenBrain(TEneGreenBoss* owner)
{
	return new GreenBrain(owner);
}

// 0x80163430
GreenBrain::GreenBrain(TEneGreenBoss* owner)
{
	mOwner = owner;
	mSavedHealth = gGreenBossParams->mHealth;
	mSight.reset();
	mSight.mHeightLimit = 1000.0f;
	mStep   = 0;
	mTimer  = 0;
	mTimer2 = 0;
	_5C     = 0;
	_60     = 0;
	if (mOwner != 0) {
		mNext  = -1;
		mState = 0;
		onPhase(mState, 0);
		if (mNext != -1) {
			mOwner->setAction(mNext);
		}
	}
	mPath     = createTObjGreenGrassField(gTL_TIMESTOP);
	mNode     = 0;
	mPrevNode = 0;
}

// 0x8016351C
GreenBrain::~GreenBrain() { }

// 0x80163584
// Slot _08
int GreenBrain::checkInterrupt()
{
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		return 1;
	}
	if (mOwner->wasHitHard() == 0 && mOwner->wasHitKind1() != 0) {
		mOwner->hurtReact();
	}
	return 0;
}

// 0x80163638
void GreenBrain::look()
{
	mSight.findInRange(100000000.0f, mOwner->getPos());
}

// 0x80163690
// The next graph point toward the player
int GreenBrain::findNode()
{
	if (mNode == 0) {
		return mPath->findFarOuter(&mSight.mTarget, mPrevNode, gGreenBossParams->mPathArg);
	}
	return mPath->findFarNeighbor(&mSight.mTarget, mNode, mPrevNode, gGreenBossParams->mPathArg);
}

// 0x801636F0
void GreenBrain::aim()
{
	mOwner->setLookTarget(&mSight.mTarget, mSight.mAngle);
}

// 0x80163720
void GreenBrain::lookAt(Vec3f* pos)
{
	int angle = angleToXZ(mOwner->getPos(), pos);
	mOwner->setLookTarget(pos, angle);
}

// 0x8016377C
void GreenBrain::saveHealth()
{
	mSavedHealth = mOwner->mHealth;
}

// 0x8016378C
int GreenBrain::getStage()
{
	return thresholdIndex(mOwner->mHealth, gGreenBossParams->mHealthSteps);
}

// 0x801637BC
// Whether it has lost this stage's drop since saveHealth
int GreenBrain::isHurtStep()
{
	int health = mOwner->mHealth;
	int stage  = thresholdIndex(health, gGreenBossParams->mHealthSteps);
	return mSavedHealth - health > gGreenBossParams->mHurtDrop[stage];
}

// 0x80163828
int GreenBrain::isOver(int count, int* table)
{
	bool over = count >= table[getStage()];
	return over;
}

// 0x80163874
int GreenBrain::isWalkOver(int count)
{
	return isOver(count, gGreenBossParams->mWalkTime);
}

// 0x8016389C
int GreenBrain::isChaseOver(int count)
{
	return isOver(count, gGreenBossParams->mChaseTime);
}

// 0x801638C4
int GreenBrain::isAttackOver(int count)
{
	return isOver(count, gGreenBossParams->mAttackTime);
}

// 0x801638EC
int GreenBrain::isRestOver(int count)
{
	return isOver(count, gGreenBossParams->mRestTime);
}

// 0x80163914
int GreenBrain::isInRange(Vec3f* pos)
{
	f32 dist = distXZ(mOwner->getPos(), pos);
	return dist <= gGreenBossParams->mRange[getStage()];
}

// 0x80163998
int GreenBrain::isFacing(int angle)
{
	int limit = gGreenBossParams->mFacing[getStage()];
	bool facing = angle <= limit || angle >= -limit;
	return facing;
}

// 0x801639F4
void GreenBrain::setSpeed(f32 rate)
{
	mOwner->mSpeed = rate * gGreenBossParams->mSpeed[getStage()];
}

// 0x80163A50
void GreenBrain::setNode(int node)
{
	mPrevNode = mNode;
	mNode     = node;
}

// 0x80163A60
void GreenBrain::splash(f32 radius, Vec3f* pos)
{
	mPath->pushGroups(radius, pos);
	seStartAt(0x182, pos);
}

// 0x80163A9C
void GreenBrain::pathOn()
{
	mPath->setNear();
}

// 0x80163AC0
void GreenBrain::pathOff()
{
	mPath->clearNear();
}

// 0x80163AE4
// Slot _0C
void GreenBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_WAKE:
		phaseWake(phase);
		break;
	case STATE_WALK:
		phaseWalk(phase);
		break;
	case STATE_DAZE:
		phaseDaze(phase);
		break;
	case STATE_HURT:
		phaseHurt(phase);
		break;
	case STATE_GROW:
		phaseGrow(phase);
		break;
	case STATE_CHASE:
		phaseChase(phase);
		break;
	case STATE_SHRINK:
		phaseShrink(phase);
		break;
	case STATE_ATTACK:
		phaseAttack(phase);
		break;
	case STATE_SPIN:
		phaseSpin(phase);
		break;
	case STATE_10:
		phaseState10(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x80163BAC
void GreenBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0: {
		Vec3f at;

		mOwner->clearEngaged();
		mNext = 0;
		mOwner->setHealth(gGreenBossParams->mHealth);
		at.x = 0.0f;
		at.y = 25.31f;
		at.z = 0.0f;
		mOwner->setPos(&at);
		break;
	}
	case 2:
		if (mOwner->isAction0Done()) {
			changeState(STATE_WAKE);
		}
		break;
	case 1:
	case 3: {
		Vec3f at;

		at.x = 0.0f;
		at.y = 25.31f;
		at.z = 0.0f;
		mOwner->setPos(&at);
		camSetChasePitch(0, 0x800);
		break;
	}
	}
}

// 0x80163CD8
void GreenBrain::phaseWake(int phase)
{
	switch (phase) {
	case 0:
		mOwner->clearEngaged();
		mNext = 1;
		mStep = 0;
		break;
	case 2:
		if (mStep == 2) {
			changeState(STATE_WALK);
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mOwner->isAction1Done()) {
				mNext = 2;
				mStep = 1;
			}
			break;
		case 1:
			if (mOwner->isAction2Done()) {
				mStep = 2;
			}
			break;
		case 2:
			break;
		}
		break;
	case 3:
		look();
		mPrevNode = 0;
		mNode     = mPath->findFarOuter(&mSight.mTarget, mPrevNode, gGreenBossParams->mPathArg);
		mOwner->setPos(mPath->getGroupPos(mNode));
		break;
	}
}

// 0x80163E3C
// It walks the graph toward the player turning first when it has to
void GreenBrain::phaseWalk(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setEngaged();
		saveHealth();
		mTimer = 0;
		mStep  = 0;
		camLockF(camGetDrawIndex());
		break;
	case 2:
		if (mOwner->wasHitHard()) {
			if ((int)mOwner->isRecovered() != 0) {
				mOwner->clearFell();
			}
			changeState(STATE_HURT);
		} else if (mStep == 4) {
			changeState(STATE_DAZE);
		}
		break;
	case 1:
		mTimer++;
		switch (mStep) {
		case 0: {
			int node;

			look();
			node = findNode();
			if (mNode == node) {
				if (mPath->hasGrass(node, gGreenBossParams->mPathArg)) {
					if (isWalkOver(mTimer)) {
						mStep = 4;
					} else {
						mNext = 7;
						mStep = 3;
					}
				} else {
					mStep = 4;
				}
			} else {
				setNode(node);
				lookAt(mPath->getGroupPos(mNode));
				mNext = 4;
				mStep = 1;
			}
			break;
		}
		case 1:
			if (isWalkOver(mTimer)) {
				mStep = 4;
			} else if (mOwner->isAction4Done()) {
				mNext = 6;
				setSpeed(gGreenBossParams->mSlowRate);
				mStep = 2;
			}
			break;
		case 2:
			if (mPath->hasGrassNear(mOwner->getPos(), gGreenBossParams->mPathArg)) {
				setSpeed(gGreenBossParams->mFastRate);
			} else {
				setSpeed(gGreenBossParams->mSlowRate);
			}
			if (isWalkOver(mTimer)) {
				mStep = 4;
			} else if (mOwner->isAction6Done()) {
				mNext = 7;
				mStep = 3;
			}
			break;
		case 3:
			if (mOwner->isAction7Done()) {
				mStep = 0;
			}
			break;
		}
		break;
	case 3:
		camLockE(camGetDrawIndex());
		break;
	}
}

// 0x8016415C
void GreenBrain::phaseDaze(int phase)
{
	switch (phase) {
	case 0:
		mOwner->clearEngaged();
		mNext = 8;
		break;
	case 2:
		if (mOwner->isAction8Done()) {
			changeState(STATE_ATTACK);
		}
		break;
	case 3:
		break;
	}
}

// 0x80164204
void GreenBrain::phaseHurt(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setEngaged();
		mNext  = 9;
		mTimer = 0;
		break;
	case 2:
		if (mOwner->isAction9Done()) {
			changeState(STATE_GROW);
		}
		break;
	case 3:
		break;
	case 1:
		if (isHurtStep()) {
			mOwner->clearEngaged();
		}
		break;
	}
}

// 0x801642D0
void GreenBrain::phaseGrow(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setEngaged();
		mNext = 10;
		break;
	case 2:
		if (isHurtStep() || isChaseOver(mTimer)) {
			changeState(STATE_SHRINK);
		} else if (mOwner->wasHitHard()) {
			changeState(STATE_CHASE);
		}
		break;
	case 1:
		mTimer++;
		break;
	case 3:
		break;
	}
}

// 0x801643F0
void GreenBrain::phaseChase(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setEngaged();
		mNext = 20;
		break;
	case 2:
		if (isHurtStep() || isChaseOver(mTimer)) {
			changeState(STATE_SHRINK);
		} else if (mOwner->isAction20Done()) {
			changeState(STATE_GROW);
		}
		break;
	case 1:
		mTimer++;
		break;
	case 3:
		break;
	}
}

// 0x80164510
void GreenBrain::phaseShrink(int phase)
{
	switch (phase) {
	case 0:
		mOwner->clearEngaged();
		mNext = 11;
		break;
	case 2:
		if (mOwner->isAction11Done()) {
			changeState(STATE_ATTACK);
		}
		break;
	case 3:
		break;
	}
}

// 0x801645B8
// It runs at the player and swings or slams when close
void GreenBrain::phaseAttack(int phase)
{
	switch (phase) {
	case 0:
		mOwner->clearEngaged();
		mStep  = 0;
		mTimer = 0;
		break;
	case 2:
		if (mStep == 6) {
			changeState(STATE_SPIN);
		}
		break;
	case 1:
		mTimer++;
		switch (mStep) {
		case 0:
			look();
			aim();
			mNext = 4;
			mStep = 1;
			break;
		case 1:
			if (isAttackOver(mTimer)) {
				mStep = 6;
			} else {
				if (mOwner->isAction4Done()) {
					setSpeed(1.0f);
					mNext = 6;
					mStep = 2;
				}
			}
			break;
		case 2:
			look();
			aim();
			if (isAttackOver(mTimer)) {
				mStep = 6;
			} else {
				if (isInRange(&mSight.mTarget)) {
					if (isFacing(mSight.mAngle)) {
						if (getStage() < 2) {
							mNext = 12;
							mStep = 3;
						} else {
							mNext = 13;
							mStep = 4;
						}
					} else {
						mNext = 4;
						mStep = 1;
					}
				} else if (mOwner->isAction6Done()) {
					if (getStage() < 2) {
						mNext = 12;
						mStep = 3;
					} else {
						mNext = 13;
						mStep = 4;
					}
				}
			}
			break;
		case 3:
			if (mOwner->isAction12Done()) {
				mNext   = 18;
				mStep   = 5;
				mTimer2 = 0;
			}
			break;
		case 4:
			if (mOwner->isAction13Done()) {
				mNext   = 17;
				mStep   = 5;
				mTimer2 = 0;
			}
			break;
		case 5:
			mTimer2++;
			if (isAttackOver(mTimer)) {
				mStep = 6;
			} else if (isRestOver(mTimer2)) {
				mStep = 0;
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x80164898
// It goes back to the start of the graph and spins there
void GreenBrain::phaseSpin(int phase)
{
	switch (phase) {
	case 0:
		mOwner->clearEngaged();
		setNode(0);
		lookAt(mPath->getGroupPos(mNode));
		mNext = 3;
		mStep = 0;
		break;
	case 2:
		if (mStep == 5) {
			changeState(STATE_WAKE);
		}
		break;
	case 3:
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mOwner->isAction3Done()) {
				mNext = 5;
				setSpeed(1.0f);
				mStep = 1;
			}
			break;
		case 1:
			if (mOwner->isAction5Done()) {
				mNext = 14;
				mStep = 2;
			}
			break;
		case 2:
			if (mOwner->isAction14Done()) {
				mNext = 15;
				mStep = 3;
			}
			break;
		case 3:
			if (mOwner->isAction15Done()) {
				mPath->resetGroups();
				seStartAt(0x183, mOwner->getPos());
				mNext = 16;
				mStep = 4;
			}
			break;
		case 4:
			if (mOwner->isAction16Done()) {
				mStep = 5;
			}
			break;
		case 5:
			break;
		}
		break;
	}
}

// 0x80164A80
void GreenBrain::phaseState10(int phase)
{
	switch (phase) {
	case 0:
		mNext = 17;
		break;
	case 1:
		look();
		aim();
		break;
	case 2:
		break;
	}
}

// 0x80164ADC
void GreenBrain::phaseDie(int phase)
{
	switch (phase) {
	case 0:
		camResetChasePitch(0);
		mNext = 21;
		break;
	case 1:
		look();
		aim();
		break;
	case 2:
		break;
	}
}
