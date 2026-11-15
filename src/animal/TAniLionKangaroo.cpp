#include "animal/AniMount.h"
#include "engine/DrawTask.h"
#include "engine/FxSystem.h"
#include "player/PlyState.h"
#include "player/Player.h"
#include "animal/TAniCamelOstrich.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjLauncher.h"
#include "engine/ActorMotion.h"
#include "core/MathUtil.h"
#include "object/ObjShadow.h"
#include "animal/TAniMount.h"
#include "engine/TParticle.h"
#include "sound/Sound3DPlay.h"
#include "stage/TStageScore.h"
#include "object/TObjEggRing.h"
#include "animal/TAniLionKangaroo.h"
struct NJS_QUATERNION;
struct DrawTask;
struct Fx;

typedef char TAniLionKangaroo_size_check[(sizeof(TAniLionKangaroo) == 0x474) ? 1 : -1];

inline void TAniLionKangaroo::useMove(AniMove* move)
{
	if (mMove != move) {
		move->enter();
		mMove = move;
	}
}

// 0x801C2114
extern "C" void* createLionKangaroo(TObject* parent)
{
	return new TAniLionKangaroo(parent);
}

const MotionEntry sLionKangarooMotions[] = {
	{ 0, 11, -1, 0, 0.0f, 30.0f, 0.7f, 0.0f, 1 },  { 0, 8, -1, 0, 0.0f, 16.0f, 0.39f, 0.0f, 1 },
	{ 1, 6, -1, 0, 5.0f, 11.0f, 0.35f, 0.0f, 0 },  { 1, 6, -1, 0, 11.0f, 16.0f, 0.35f, 0.0f, 0 },
	{ 3, 6, -1, 5, 16.0f, 19.0f, 0.35f, 0.1f, 0 }, { 0, 10, -1, 0, 0.0f, 30.0f, 0.2f, 0.0f, 0 },
	{ 1, 0, -1, 0, 0.0f, 19.0f, 0.28f, 0.0f, 0 },  { 1, 9, -1, 0, 0.0f, 16.0f, 0.33f, 0.0f, 0 },
	{ 0, 7, -1, 0, 0.0f, 5.0f, 0.5f, 0.0f, 0 },    { 1, 3, -1, 0, 0.0f, 6.0f, 0.3f, 0.0f, 0 },
	{ 0, 2, -1, 0, 0.0f, 16.0f, 0.3f, 0.0f, 0 },   { 1, 1, -1, 0, 0.0f, 6.0f, 0.3f, 0.0f, 0 },
	{ 1, 4, -1, 0, 0.0f, 4.0f, 0.27f, 0.0f, 0 },   { 1, 5, -1, 0, 0.0f, 29.0f, 0.25f, 0.0f, 0 },
	{ 1, 0, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
};

const AniNode sLionKangarooModelNodes[3] = { { 0, 0x43 }, { 1, 0x3D }, { 2, 0x1F } };

const char* gTAniLionKangarooName = "TAniLionKangaroo";

// 0x801C2160
TAniLionKangaroo::TAniLionKangaroo(TObject* parent)
    : TAniMount(parent, 0x15)
    , mCharge(0)
    , mMove(0)
{
	mChargeFx = 0;
	mBirth    = 0;
	_00       = (u32)gTAniLionKangarooName;
	mSize     = 0x474;

	mBodyHit.mOwnerId  = mObjId;
	mBodyHit.mRadius   = 10.0f;
	mBodyHit.mOffset   = sLionBodyOffset;
	mHits.mList.mBody  = &mBody;
	mHits.mList.mModel = &mModel;
	mHits.mList.mLife  = &mLife;
	mHits.mList.add(&mBodyHit);
	mLife.mHp    = 100; // the DOL stores to this one twice and never to mMax
	mLife.mHp    = 100;
	mBlink.mLife = &mLife;
	mBlink.mBody = &mBody;
	mDrawTask    = addDrawTask(this, (void (*)(void*))drawTask, 0x21, 0);
	mModel.setNodes(sLionKangarooModelNodes, 3);
	mMotions.set(&mMotion, sLionKangarooMotions);
	mMotions.update(&mAsset);
	mModel.update(&mAsset);

	TObject* shotParent = new TObject(this);
	mShotParent         = shotParent;

	mShadow._30 = 20.0f;
	mShadow._34 = 20.0f;
	mMotion.play(5);

	mWalk.setBody(&mBody);
	mWalk.mRunSpeed  = 150.0f;
	mWalk.mRunFrom   = 140.0f;
	mWalk.mWalkSpeed = 100.0f;
	mWalk.mAccel     = 0.05f;
	mWalk.mDecel     = 0.2f;
	mJump.setBody(&mBody);
	mJump.mGravity   = -400.0f;
	mJump.mJumpSpeed = 225.0f;
	mJump.mAirSpeed  = 60.0f;
}

// 0x801C23A8
TAniLionKangaroo::~TAniLionKangaroo()
{
	setState(-1);
	if (mChargeFx != 0) {
		fxStop(mChargeFx);
	}
	removeDrawTask(mDrawTask);
}

// 0x801C24F0
void TAniLionKangaroo::onLoaded()
{
	mPos       = mStartPos;
	mRotY      = mStartRot;
	mBody.mPos = mPos;
	updateMatrix();
	updateShadow();
	mFxBank = mAsset.mFile->mFxBank;
	setState(0);
}

// 0x801C2574
void TAniLionKangaroo::vfn_94(f32* out)
{
	mModel.getMatrix(0, out);
}

// 0x801C25A0
void TAniLionKangaroo::preUpdate()
{
	mMotions.update(&mAsset);
	mModel.update(&mAsset);
	mHits.mList.hitAll();
	mBlink.update();
	if (flagByteTest(&mFlags2, 0) == 0) {
		if (mBlink.getResult() == 1) {
			flagByteSet(&mFlags2, 0);
			sound(0x1E0);
			setState(0xA);
		} else if (mBlink.getResult() == 2) {
			flagByteSet(&mFlags2, 0);
			sound(0x1E0);
			die();
		} else if (flagTest(&mBlink.mFlags, 0) != 0) {
			flagClear(&mBlink.mFlags, 0);
			setState(9);
		}
	}
}

// 0x801C269C
void TAniLionKangaroo::postUpdate()
{
	if (mMove != 0) {
		mMove->step();
	}
	mBody.update();
	mPos = mBody.mPos;
	mMotion.update();
	updateMatrix();
	updateShadow();
	if (mChargeFx != 0) {
		mModel.getMatrix(2, mChargeFx->mMatrix);
		njUnitRotPortion(mChargeFx->mMatrix);
	}
}

// 0x801C2734
void TAniLionKangaroo::vfn_10()
{
	Vec3f at;
	at.x = mPos.x;
	at.y = 10.0f + mPos.y;
	at.z = mPos.z;
	Vec3f out;
	if (cameraIsVisible(1000.0f, 10.0f, &at, &out) == 0) {
		mDrawTask->mFlags |= 1;
	} else {
		mDrawTask->mFlags &= ~1;
	}
}

// 0x801C27B8
void TAniLionKangaroo::drawTask(TAniLionKangaroo* self)
{
	if (self->mBlink.isHidden() == 0) {
		f32 matrix[12];
		DrawState state;
		state.mFlags = 0;
		state._20    = 0;
		if (self->mBirth != 0) {
			self->mBirth->getDrawState(&state);
		} else {
			state = *(const DrawState*)getDefaultDrawState();
			if (self->mBlink.tint((DrawState*)&state.mTev2) != 0) {
				state.mFlags |= 0x20;
			}
		}
		mtxFromView(matrix, self->mMatrix);
		self->mModel.draw(&self->mAsset, matrix, &state);
	}
}

// 0x801C28A0
void TAniLionKangaroo::updateMatrix()
{
	f32 quat[4];
	*(Quat*)quat = mBody.mQuat;
	mtxSetTransV(mMatrix, &mPos);
	mtxConcatQuat(mMatrix, mMatrix, quat);
	mModel.pose(&mAsset, mMatrix, &mMotion);
}

// 0x801C2918
void TAniLionKangaroo::updateShadow()
{
	mHits.mList.updateAll();
	Vec3f ground;
	if (mBody.touching(6, 0, &ground, 0) == 0) {
		shadowDisable(&mShadow);
	} else {
		shadowSetGround(&mShadow, &ground, 0);
		shadowEnable(&mShadow);
	}
}

// 0x801C298C
int TAniLionKangaroo::canMount() const
{
	u8 can = 0;
	if (mState == 1 && mJump.state() == 3) {
		can = 1;
	}
	return can;
}

// 0x801C29D8
int TAniLionKangaroo::canDismount()
{
	return mState == 2;
}

// 0x801C29EC
void TAniLionKangaroo::onMount()
{
	setState(2);
}

// 0x801C2A10
void TAniLionKangaroo::onDismount()
{
	clearCharge();
	setState(1);
}

// 0x801C2A48
void TAniLionKangaroo::playWalk(int motion)
{
	int current = *(volatile s16*)&mMotion.mCurrent;
	if ((mMotion.getUser(current) & 1) == 0) {
		mMotion.playBlend(motion, 0.1f);
	} else if (current != motion) {
		mMotion.play(motion);
	}
}

// 0x801C2AB4
void TAniLionKangaroo::feed(const PlyInput* input, AniMove* move)
{
	move->setStick(input->mStick, input->mAngle);
}

// 0x801C2AE0
void TAniLionKangaroo::sound(int id)
{
	aniMountSound(this, id, &mPos);
}

// 0x801C2B04
void TAniLionKangaroo::bump(const Vec3f* pos)
{
	particleOnceById(this, pos, 0, 0);
}

// 0x801C2B2C
void TAniLionKangaroo::land()
{
	bump(&mPos);
	rumble(2);
	sound(0x1E5);
	mMotion.playBlend(5, 0.1f);
	setState(2);
}

// 0x801C2B90
int TAniLionKangaroo::onHit(const TObjAction* other, int arg, int* result)
{
	if (other == this) {
		return 0;
	}
	HitInfo* hit = (HitInfo*)result;
	if (arg == 1) {
		hit->mDamage = 0x19;
		hit->mFlags.setBit(0xD);
		return 1;
	}
	if (arg == 0) {
		hit->mDamage = 0x19;
		hit->mFlags.setBit(0xD);
		hit->_14 = mHitValue;
		return 1;
	}
	return 0;
}

// 0x801C2C20
void TAniLionKangaroo::updateCharge()
{
	if ((mInput.mHeld & 2) == 0) {
		clearCharge();
	} else {
		if (mCharge < 0x3C) {
			mCharge++;
		}
		if (mChargeFx == 0) {
			sound(0x1E6);
			mChargeFx = fxSpawn(1, mFxBank, 7);
		}
	}
}

// 0x801C2C9C
void TAniLionKangaroo::clearCharge()
{
	if (mChargeFx != 0) {
		fxStop(mChargeFx);
		mChargeFx = 0;
	}
	mCharge = 0;
}

// 0x801C2CE4
int TAniLionKangaroo::chargeFull()
{
	return mCharge == 0x3C;
}

// 0x801C2CF8
void TAniLionKangaroo::runState(int state, int phase)
{
	switch (state) {
	case 0:
		runBirth(phase);
		break;
	case 1:
		runIdle(phase);
		break;
	case 2:
		runStand(phase);
		break;
	case 3:
		runWalk(phase);
		break;
	case 4:
		runJump(phase);
		break;
	case 5:
		runFall(phase);
		break;
	case 6:
		runChargeUp(phase);
		break;
	case 7:
		runShot(phase);
		break;
	case 8:
		runBreath(phase);
		break;
	case 9:
		runHurt(phase);
		break;
	case 10:
		runDead(phase);
		break;
	}
}

// 0x801C2DB4
void TAniLionKangaroo::runBirth(int phase)
{
	if (phase == 0) {
		mMotion.play(0xE);
		TAniBirth* birth = new TAniBirth(this, this, (ItemHolder*)&mAsset, &mBody);
		mBirth           = birth;
		mBirth->mFxBank  = 0x22;
		mBirth->mTextId  = 0x1B;
		TAniBirth* at    = mBirth;
		at->mTextAtA.x   = 0.0f;
		at->mTextAtA.y   = 15.0f;
		at->mTextAtA.z   = 0.0f;
		at->mTextAtB.x   = 60.0f;
		at->mTextAtB.y   = 25.0f;
		at->mTextAtB.z   = 35.0f;
	} else if (phase == 1) {
		mBirth->mFlags |= 1;
		mBirth = 0;
	} else if (phase == 2) {
		if ((int)(u8)mBirth->mState == 6) {
			setState(1);
		}
	} else if (phase == 3) {
		if (flagByteTest(&mBirth->mFlags2, 0) != 0) {
			flagByteClear(&mBirth->mFlags2, 0);
			mMotion.play(6);
			sePlay3D(0x1DF, &mPos);
		}
		if (flagByteTest(&mBirth->mFlags2, 1) != 0) {
			flagByteClear(&mBirth->mFlags2, 1);
			mMotion.play(5);
		}
	}
}

// 0x801C2F30
void TAniLionKangaroo::runIdle(int phase)
{
	if (phase == 0) {
		mMotion.playBlend(5, 0.1f);
		useMove(&mJump);
	}
}

// 0x801C2F98
void TAniLionKangaroo::runStand(int phase)
{
	if (phase == 0) {
		useMove(&mWalk);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mWalk.state() == 4) {
			setState(5);
		} else if ((input->mPressed & 1) != 0) {
			feed(input, &mJump);
			setState(4);
		} else if ((input->mPressed & 2) != 0) {
			setState(7);
		} else if ((input->mNotHeld & 2) != 0 && chargeFull() != 0) {
			setState(8);
		} else {
			feed(input, &mWalk);
			if (input->mStick > 0.0f) {
				setState(3);
			}
		}
	} else if (phase == 3) {
		updateCharge();
	}
}

// 0x801C30CC
void TAniLionKangaroo::runWalk(int phase)
{
	if (phase == 0) {
		useMove(&mWalk);
	} else if (phase == 1) {
		mMotion.mSpeed = 1.0f;
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mWalk.state() == 4) {
			setState(5);
		} else if ((input->mPressed & 1) != 0) {
			feed(input, &mJump);
			setState(4);
		} else if ((input->mPressed & 2) != 0) {
			setState(7);
		} else {
			feed(input, &mWalk);
			if (mWalk.state() == 0) {
				mMotion.playBlend(5, 0.1f);
				setState(2);
			}
		}
	} else if (phase == 3) {
		updateCharge();
		switch (mWalk.state()) {
		case 1:
		case 2:
			mMotion.mSpeed = mWalk.mSpeed / 100.0f;
			playWalk(0);
			if ((*(volatile u32*)&mMotion.mFlags & 0x100) != 0) {
				mMotion.mFlags &= ~0x100;
				sound(0x1E1);
				rumble(2);
			}
			break;
		case 3:
			mMotion.mSpeed = 1.0f;
			playWalk(1);
			if ((*(volatile u32*)&mMotion.mFlags & 0x100) != 0) {
				mMotion.mFlags &= ~0x100;
				sound(0x1E2);
				rumble(2);
			}
			break;
		}
		mRotY = mWalk.mRotY;
	}
}

// 0x801C32B4
void TAniLionKangaroo::runJump(int phase)
{
	if (phase == 0) {
		useMove(&mJump);
		mJump.launch();
		mMotion.play(2);
		rumble(9);
		sound(0x1E4);
		bump(&mPos);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mJump.state() == 3) {
			land();
		} else {
			feed(input, &mJump);
		}
	} else if (phase == 3) {
		updateCharge();
		if (mMotion.mCurrent == 2 && mJump.state() == 1) {
			mMotion.play(3);
		}
		mRotY = mWalk.mRotY;
	}
}

// 0x801C33C0
void TAniLionKangaroo::runFall(int phase)
{
	if (phase == 0) {
		useMove(&mJump);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mJump.state() == 3) {
			land();
		} else {
			feed(input, &mJump);
		}
	} else if (phase == 3) {
		updateCharge();
	}
}

// 0x801C3464
void TAniLionKangaroo::runChargeUp(int phase)
{
	if (phase == 0) {
		mMotion.play(8);
		mCharge = 0x3C;
		sound(0x1E6);
		mFx = fxSpawn(1, mFxBank, 7);
		if (mFx != 0) {
			mModel.getMatrix(2, mFx->mMatrix);
			njUnitRotPortion(mFx->mMatrix);
		}
	} else if (phase == 1) {
		fxStop(mFx);
	} else if (phase == 2) {
		if (mCharge == 0) {
			setState(8);
		} else if ((mInput.mNotHeld & 2) != 0) {
			setState(7);
		}
	} else if (phase == 3) {
		if (mCharge != 0) {
			mCharge--;
		}
		if (mFx != 0) {
			mModel.getMatrix(2, mFx->mMatrix);
			njUnitRotPortion(mFx->mMatrix);
		}
	}
}

// 0x801C3584
void TAniLionKangaroo::runShot(int phase)
{
	if (phase == 0) {
		mMotion.play(7);
		rumble(0xB);
		sound(0x1E9);
		clearCharge();
		f32 matrix[12];
		mModel.getMatrix(1, matrix);
		Vec3f at;
		mtxGetPos(matrix, &at);
		Vec3f vel;
		vecFromAngleXZ(200.0f, &vel, mRotY);
		new TAniLionFireBall(mShotParent, (u16)mObjId, mFxBank, &at, &vel);
	} else if (phase == 2) {
		if (mMotion.isDone() != 0) {
			mMotion.playBlend(5, 0.1f);
			setState(2);
		}
	}
}

// 0x801C3670
void TAniLionKangaroo::runBreath(int phase)
{
	if (phase == 0) {
		mMotion.play(9);
		mSweep = 0;
		mStep  = 0;
		sePlay3DHeld(400.0f, &mSounds[0], 0x1EB, 0xC8, (const Vec3f*)&mPos);
		sePlay3DHeld(400.0f, &mSounds[1], 0x1EA, 0xC8, (const Vec3f*)&mPos);
		mFx = fxSpawn(1, mFxBank, 0xA);
		if (mFx != 0) {
			mModel.getMatrix(1, mFx->mMatrix);
		}
		clearCharge();
		mHitValue                  = scoreNewChain(3);
		TAniLionFireBreath* breath = new TAniLionFireBreath(this, mObjId);
		mBreath                    = breath;
	} else if (phase == 1) {
		fxStop(mFx);
		seStop3D(&mSounds[0]);
		seStop3D(&mSounds[1]);
		if (mBreath != 0) {
			mBreath->mFlags |= 1;
		}
	} else if (phase == 2) {
		if ((s8)mStep == 3) {
			mMotion.playBlend(5, 0.1f);
			setState(2);
		}
	} else if (phase == 3) {
		switch (mStep) {
		case 0:
			if (mMotion.isDone() != 0) {
				mMotion.play(0xA);
				mStep = 1;
			}
			break;
		case 1:
			mSweep += 0x71C;
			if (mSweep > 0x20000) {
				mMotion.play(0xB);
				mStep = 2;
			}
			break;
		case 2:
			if (mMotion.isDone() != 0) {
				mStep = 3;
			}
			break;
		}
		f32 quat[4];
		njCreateQuaternion(mSweep + mRotY, 0.0f, 1.0f, 0.0f, (NJS_QUATERNION*)quat);
		mBody.mQuat = *(Quat*)quat;
		f32 matrix[12];
		mModel.getMatrix(1, matrix);
		if (mFx != 0) {
			mtxCopy(mFx->mMatrix, matrix);
		}
		if (mBreath != 0) {
			mBreath->setMatrix(matrix);
		}
	}
}

// 0x801C38EC
void TAniLionKangaroo::runHurt(int phase)
{
	if (phase == 0) {
		rumble(0xE);
		sound(0x1E7);
		mMotion.play(0xC);
		useMove(&mJump);
		clearCharge();
	} else if (phase == 2) {
		if (mJump.state() == 3 && mMotion.isDone() != 0) {
			if (mHolderId == 0xFFFF) {
				setState(1);
			} else {
				land();
			}
		}
	}
}

// 0x801C39BC
void TAniLionKangaroo::runDead(int phase)
{
	if (phase == 0) {
		mMotion.play(0xD);
		useMove(&mJump);
		clearCharge();
		sound(0x1E8);
	} else if (phase == 2) {
		if (mMotion.isDone() != 0) {
			sound(0x1E0);
			die();
		}
	}
}

// 0x801C3A64
Vec3f* TAniLionKangaroo::getPos()
{
	return &mPos;
}

// 0x801C3A6C
int TAniLionKangaroo::getRotY()
{
	return mRotY;
}

// 0x801C3A74
AniBodyOffset sLionBodyOffset(0.0f, 10.0f, 0.0f);
