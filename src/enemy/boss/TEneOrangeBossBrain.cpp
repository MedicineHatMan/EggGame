#include "enemy/boss/TEneOrangeBoss.h"
#include "sound/SeHolder.h"
#include "player/Player.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "animal/AniMount.h"
#include "enemy/EnemyFx.h"
#include "stage/TStageOrangeBoss.h"
#include "enemy/boss/TEneOrangeBossFloor.h"
#include "core/Mem.h"
#include "object/TObjPlayer.h"
struct NJS_QUATERNION;
struct TChrPlayer;
struct TObjPlayer;

static void orangeRootUpdate(OrangeBrainRoot* root, TEneOrangeBoss* owner) { root->OrangeBrainRoot::update(owner); }

// 0x8011A3E0
inline void OrangeBrainRoot::update(TEneOrangeBoss* owner)
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

// 0x802C9CB0
// TEneOrangeBoss builds it with createOrangeBrain and runs it each frame

// How long STATE_1 waits before it gives up on the player standing at a mirror
int sOrangeWaitFrames = 600;

// The centre mirror's player id
#define centerPlayerId() (((int (*)())mirrorCenterId)())

// 0 to 1 from the game's random number
#define random01() (3.0517578e-5f * rand())

// 0x8011A478
OrangeBrainBase::OrangeBrainBase() { mOwner = 0; }

// 0x8011A4AC
OrangeBrainBase::~OrangeBrainBase() { }

// 0x8011A508
OrangeBrain* createOrangeBrain(TEneOrangeBoss* owner)
{
	return new OrangeBrain(owner);
}

// 0x8011A550
OrangeBrain::OrangeBrain(TEneOrangeBoss* owner)
{
	mSight.reset();
	mSight.mHeightLimit = 1000.0f;
	mSavedHealth = gOrangeBossParams->mHealth;
	mReturnState = STATE_4;
	mHurtWait    = 0;
	mHurtRecover = 0;
	mStep        = 0;
	mTimer       = 0;
	mPick        = 0;
	start(owner);
	orangeStageMirrorsOff();
}

// 0x8011A624
OrangeBrain::~OrangeBrain()
{
	orangeStageMirrorsOff();
}

// 0x8011A690
// Slot _08
int OrangeBrain::checkInterrupt()
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

// 0x8011A744
void OrangeBrain::look()
{
	mSight.findInRange(100000000.0f, mOwner->getPos());
}

// 0x8011A79C
void OrangeBrain::aim()
{
	mOwner->setLookTarget(&mSight.mTarget);
}

// 0x8011A7C8
void OrangeBrain::saveHealth()
{
	mSavedHealth = mOwner->mHealth;
	mOwner->setHold();
}

// 0x8011A7F8
int OrangeBrain::isHurtStep()
{
	if (healthDropReached(mSavedHealth, mOwner->mHealth, gOrangeBossParams->mHealthSteps,
	                      gOrangeBossParams->mHurtDrop) != 0) {
		mOwner->clearHold();
		return 1;
	}
	return 0;
}

// 0x8011A858
int OrangeBrain::getStage()
{
	return thresholdIndex(mOwner->mHealth, gOrangeBossParams->mHealthSteps);
}

// 0x8011A888
int OrangeBrain::playerSlot()
{
	return mirrorSlotOf(mOwner->getPos());
}

// 0x8011A8BC
int OrangeBrain::attackState()
{
	return getStage() == 0 ? STATE_1 : STATE_10;
}

// 0x8011A8EC
// Slot _0C
void OrangeBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_1:
		phaseState1(phase);
		break;
	case STATE_2:
		phaseState2(phase);
		break;
	case STATE_3:
		phaseState3(phase);
		break;
	case STATE_4:
		phaseState4(phase);
		break;
	case STATE_5:
		phaseState5(phase);
		break;
	case STATE_6:
		phaseState6(phase);
		break;
	case STATE_7:
		phaseState7(phase);
		break;
	case STATE_8:
		phaseState8(phase);
		break;
	case STATE_PUNCH:
		phasePunch(phase);
		break;
	case STATE_10:
		phaseState10(phase);
		break;
	case STATE_11:
		phaseState11(phase);
		break;
	case STATE_12:
		phaseState12(phase);
		break;
	case STATE_13:
		phaseState13(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x8011A9D8
// Comes in at the back of the arena
void OrangeBrain::phaseAppear(int phase)
{
	Vec3f pos;

	switch (phase) {
	case 0:
		mNext = 0;
		mirrorsReset();
		mOwner->setHealth(gOrangeBossParams->mHealth);
		break;
	case 2:
		if (mOwner->isAction0Done() != 0) {
			changeState(STATE_1);
		}
		break;
	case 1:
	case 3:
		pos.x = 0.0f;
		pos.y = 50.0f;
		pos.z = 270.0f;
		mOwner->setPos(&pos);
		break;
	}
}

// 0x8011AACC
// Waits for the player to stand in front of a mirror
void OrangeBrain::phaseState1(int phase)
{
	int slot;

	switch (phase) {
	case 0:
		mNext = 1;
		mirrorRingSetFlag2();
		mirrorRingRaise();
		mirrorCenterLower(1);
		orangeFloorPhase0();
		mStep  = 1;
		mTimer = sOrangeWaitFrames;
		break;
	case 2:
		if (mirrorRingAllDown() != 0) {
			Vec3f pos = *mOwner->getPos();
			if (mirrorSlotFacing(-20.0f, &pos, &slot) != 0) {
				int clear = mirrorIsClear(slot);
				if (mStep == 1) {
					if (clear != 0) {
						mStep = 0;
					}
				} else if (clear == 0 || mTimer <= 0) {
					changeState(STATE_2);
				}
			} else {
				mStep = 1;
			}
		}
		break;
	case 3:
		break;
	case 1:
		mTimer--;
		break;
	}
}

// 0x8011AC2C
void OrangeBrain::phaseState2(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setSlot(playerSlot());
		mNext = 2;
		mirrorRingRaise();
		mirrorCenterLower(1);
		orangeFloorPhase0();
		break;
	case 2:
		if (mOwner->isAction2Done() != 0) {
			changeState(STATE_3);
		}
		break;
	case 3:
		break;
	}
}

// 0x8011ACEC
void OrangeBrain::phaseState3(int phase)
{
	switch (phase) {
	case 0: {
		int slot = mirrorSlotOpposite(mOwner->getPos());
		mOwner->setSlot(slot);
		mNext = 3;
		mirrorRingRaise();
		mirrorCenterLower(1);
		orangeFloorPhase0();
		mStep = 0;
		saveHealth();
		break;
	}
	case 2:
		if (mStep == 1 && mOwner->isAction4Done() != 0) {
			if (mirrorIsClear(playerSlot()) != 0) {
				changeState(STATE_4);
			} else {
				changeState(STATE_5);
			}
		}
		break;
	case 1:
		if (mOwner->wasHitHard() != 0) {
			isHurtStep();
			mOwner->stageBeaten();
		}
		switch (mStep) {
		case 0:
			look();
			aim();
			if ((int)mOwner->isReady() != 0) {
				mNext = 4;
				mirrorRingSetFlag1();
				mStep = 1;
			}
			break;
		}
		break;
	case 3:
		mirrorRingClearFlag1();
		mOwner->clearHold();
		break;
	}
}

// 0x8011AEBC
void OrangeBrain::phaseState4(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setSlot(playerSlot());
		mNext = 5;
		mirrorRingSetFlag1();
		mirrorRingRaise();
		mirrorCenterLower(1);
		orangeFloorPhase0();
		break;
	case 2:
		if (mOwner->isAction5Done() != 0) {
			changeState(STATE_6);
		}
		break;
	case 3:
		mirrorRingClearFlag1();
		break;
	}
}

// 0x8011AF90
void OrangeBrain::phaseState5(int phase)
{
	switch (phase) {
	case 0:
		mOwner->setSlot(playerSlot());
		mNext = 6;
		mirrorRingSetFlag1();
		mirrorRingRaise();
		mirrorCenterLower(1);
		orangeFloorPhase0();
		break;
	case 2:
		if (mOwner->isAction6Done() != 0) {
			mReturnState = STATE_6;
			mHurtWait    = gOrangeBossParams->mDazeWait;
			mHurtRecover = 0;
			changeState(STATE_12);
		}
		break;
	case 3:
		mirrorRingSetFlag2();
		break;
	}
}

// 0x8011B080
void OrangeBrain::phaseState6(int phase)
{
	Vec3f pos;

	switch (phase) {
	case 0:
		mirrorRingSetFlag1();
		mirrorRingLower();
		mirrorCenterLower(1);
		orangeFloorPhase0();
		mirrorCenterSetFlag2();
		mStep = 0;
		break;
	case 2:
		if (mOwner->isAction10Done() != 0) {
			changeState(STATE_7);
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mirrorRingAllUp() != 0) {
				rand();
				pos.x = 0.0f;
				pos.y = 50.0f;
				pos.z = 0.0f;
				mOwner->setMoveTarget(&pos, 0);
				mNext = 9;
				mirrorCenterTurn(0);
				mirrorCenterRaise(1);
				orangeFloorPhase1();
				mStep = 1;
			}
			break;
		case 1:
			if (mirrorCenterDown() != 0) {
				mNext = 10;
				mStep = 2;
			}
			break;
		case 2:
			break;
		}
		break;
	case 3:
		mirrorRingClearFlag1();
		break;
	}
}

// 0x8011B1E8
// Behind the middle mirror waiting for the player to come round
void OrangeBrain::phaseState7(int phase)
{
	switch (phase) {
	case 0:
		mNext = 11;
		mirrorRingSetFlag1();
		mirrorRingLower();
		mirrorCenterRaise(1);
		mirrorCenterClearFlag20();
		orangeFloorPhase1();
		mStep  = gOrangeBossParams->mCenterWait;
		mTimer = 0;
		break;
	case 2:
		if (mTimer == 1) {
			changeState(STATE_PUNCH);
		} else if (mTimer == 2) {
			changeState(STATE_8);
		}
		break;
	case 1:
		look();
		if (mirrorCenterDown() != 0) {
			mirrorCenterHollow();
		}
		mStep--;
		if ((int)mirrorCenterFlag20() != 0) {
			if (findPlayer(centerPlayerId()) != 0) {
				mTimer = 1;
			}
		} else if (mStep <= 0) {
			mTimer = 2;
		}
		mOwner->setFacing(&mSight.mTarget, mSight.mAngle, (u16)mSight.mPlayerId);
		break;
	case 3:
		mirrorCenterSolid();
		break;
	}
}

// 0x8011B37C
void OrangeBrain::phaseState8(int phase)
{
	switch (phase) {
	case 0:
		mNext = 12;
		mirrorRingSetFlag1();
		mirrorRingLower();
		mirrorCenterRaise(1);
		break;
	case 2:
		if (mOwner->isAction12Done() != 0) {
			mReturnState = attackState();
			mHurtWait    = gOrangeBossParams->mPunchWait;
			mHurtRecover = 1;
			changeState(STATE_12);
		}
		break;
	case 3:
		break;
	}
}

// 0x8011B44C
// Pulls the player through the middle mirror and punches
void OrangeBrain::phasePunch(int phase)
{
	s32 angle;
	switch (phase) {
	case 0: {
		TChrPlayer* player;

		mNext = 13;
		mirrorRingSetFlag1();
		mirrorRingLower();
		mirrorCenterRaise(1);
		mirrorCenterMesh(0);
		orangeFloorPhase1();
		mStep = 0;
		mOwner->clearPunchOut();
		player = (TChrPlayer*)(findPlayer(centerPlayerId()));
		if (player != 0) {
			Vec3f at;
			Vec3f dir;
			EggRot rot;
			angle = mOwner->getRotY();
			njSinCos(angle, &dir.x, &dir.z);
			dir.y = 0.0f;
			at.x  = 0.0f;
			at.y  = 70.0f;
			at.z  = 0.0f;
			gjMulAddVector(-10.0f, &at, &at, &dir);
			njCreateQuaternion(angle, 0.0f, 1.0f, 0.0f, (NJS_QUATERNION*)&rot);
			player->vfn_C0(&at, &rot);
			mirrorCenterAimAt(&at);
			orangeStageMirrorsOn();
			seStartAtFar(0x146, &at);
		}
		break;
	}
	case 2:
		if (mOwner->isAction5Done() != 0) {
			changeState(attackState());
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mOwner->isAction13Done() != 0) {
				mNext = 14;
				mStep = 1;
			}
			break;
		case 1:
			if (mOwner->isAction14Done() != 0) {
				mStep = 2;
				mOwner->setSlot(playerSlot());
				mNext = 15;
				mirrorCenterLower(1);
				mirrorCenterMesh(1);
			}
			if (mOwner->isPunchOut() != 0) {
				orangeStageMirrorsOff();
			}
			break;
		case 2:
			if (mirrorCenterUp() != 0) {
				mStep = 3;
				mirrorRingRaise();
				orangeFloorPhase0();
			}
			break;
		case 3:
			if (mOwner->isAction15Done() != 0 && mirrorRingAllDown() != 0) {
				mStep = 4;
				mNext = 5;
			}
			break;
		case 4:
			break;
		}
		break;
	case 3:
		mirrorRingClearFlag1();
		mirrorCenterMesh(1);
		orangeStageMirrorsOff();
		mOwner->throwPunch();
		break;
	}
}

// 0x8011B6F4
// One mirror of the ring is picked
void OrangeBrain::phaseState10(int phase)
{
	switch (phase) {
	case 0:
		mStep = 0;
		mirrorRingSetFlag1();
		mirrorRingSetFlag2();
		mirrorRingLower();
		mirrorCenterLower(1);
		break;
	case 2:
		if (mStep == 4) {
			changeState(STATE_6);
		} else if (mStep == 3) {
			changeState(STATE_11);
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mirrorRingAllUp() != 0) {
				mStep = 1;
				int pick = (int)(8.0f * random01());
				if (pick >= 8) {
					pick = 7;
				}
				mPick = pick;
				mOwner->setSlot(mPick);
				mNext = 16;
				orangeFloorPhase2();
				mirrorRingRaise();
				mirrorRingSetTarget(2);
			}
			break;
		case 1:
			if (mirrorRingAllDown() != 0) {
				mStep  = 2;
				mTimer = 0;
				mirrorRingClearFlag1();
			}
			break;
		case 2: {
			int target;
			int i;
			f32 t;

			mTimer++;
			t = (f32)mTimer / (f32)gOrangeBossParams->mFlashFrames;
			if (t < gOrangeBossParams->mFlashRamp) {
				t /= gOrangeBossParams->mFlashRamp;
				target = (int)(40.0f * t);
				if (target < 2) {
					target = 2;
				}
				mirrorRingSetTarget(target);
			} else {
				target = 40;
			}
			mirrorRingSetTarget(target);
			if (mTimer >= gOrangeBossParams->mFlashFrames) {
				mStep = 4;
			}
			for (i = 0; i < MIRROR_RING_COUNT; i++) {
				if (mirrorIsClear(i) == 0) {
					if (i == mPick) {
						mStep = 3;
						return;
					}
					mStep = 4;
					return;
				}
			}
			break;
		case 4:
			break;
		}
		}
		break;
	case 3:
		mirrorRingClearFlag1();
		mirrorRingSetTarget(40);
		break;
	}
}

// 0x8011B9D4
void OrangeBrain::phaseState11(int phase)
{
	switch (phase) {
	case 0:
		mNext = 17;
		mirrorRingSetFlag1();
		mirrorRingSetFlag2();
		mirrorRingRaise();
		mirrorCenterLower(1);
		orangeFloorPhase0();
		break;
	case 2:
		if (mOwner->isAction17Done() != 0) {
			mReturnState = STATE_1;
			mHurtWait    = gOrangeBossParams->mFlashWait;
			mHurtRecover = 0;
			changeState(STATE_12);
		}
		break;
	case 3:
		break;
	}
}

// 0x8011BAA8
// Open to attack for mHurtWait frames
void OrangeBrain::phaseState12(int phase)
{
	switch (phase) {
	case 0:
		mirrorRingSetFlag2();
		mirrorRingSetFlag1();
		mirrorRingRaise();
		mirrorCenterLower(1);
		orangeFloorPhase0();
		saveHealth();
		mNext  = 7;
		mStep  = 0;
		mTimer = mHurtWait;
		break;
	case 2:
		if (mOwner->isAction8Done() != 0) {
			changeState(STATE_13);
		}
		break;
	case 1:
		if (mOwner->wasHitHard() != 0) {
			if (isHurtStep() != 0) {
				mTimer = 0;
				mOwner->stageBeaten();
				mOwner->clearHold();
			} else if (mTimer > 0 && mStep != 2) {
				mStep = 1;
				mNext = 18;
			} else {
				mOwner->stageBeaten();
			}
		}
		mTimer--;
		switch (mStep) {
		case 0:
			if (mTimer <= 0 && mirrorCenterUp() != 0) {
				mStep = 2;
				mOwner->setSlot(playerSlot());
				mNext = 8;
			}
			break;
		case 1:
			if (mOwner->isAction18Done() != 0) {
				if (mTimer <= 0 && mirrorCenterUp() != 0) {
					mStep = 2;
					mOwner->setSlot(playerSlot());
					mNext = 8;
				} else {
					mStep = 0;
					mNext = 7;
				}
			}
			break;
		case 2:
			break;
		}
		break;
	case 3:
		mOwner->clearHold();
		break;
	}
}

// 0x8011BCD0
void OrangeBrain::phaseState13(int phase)
{
	switch (phase) {
	case 0:
		mirrorRingSetFlag2();
		mirrorRingSetFlag1();
		mirrorRingRaise();
		mirrorCenterLower(1);
		orangeFloorPhase0();
		if (mHurtRecover != 0) {
			mStep = 0;
			mNext = 15;
		} else {
			mStep = 1;
			mNext = 5;
		}
		break;
	case 2:
		if (mOwner->isAction5Done() != 0) {
			changeState(mReturnState);
		}
		break;
	case 1:
		switch (mStep) {
		case 0:
			if (mOwner->isAction15Done() != 0) {
				mStep = 1;
				mNext = 5;
			}
			break;
		}
		break;
	case 3:
		mirrorRingClearFlag1();
		break;
	}
}

// 0x8011BDF8
void OrangeBrain::phaseDie(int phase)
{
	switch (phase) {
	case 0:
			mirrorRingLower();
			mirrorCenterLower(1);
			orangeFloorPhase0();
			mNext = mOwner->dieAction();
		break;
	case 1:
		break;
	case 2:
		break;
	}
}
