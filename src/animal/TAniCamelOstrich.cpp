#include "animal/AniMount.h"
#include "player/PlyState.h"
#include "player/Player.h"
#include "animal/TAniCamelOstrich.h"
#include "animal/TAniMount.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjLauncher.h"
#include "engine/ActorMotion.h"
#include "core/MathUtil.h"
#include "object/ObjShadow.h"
#include "sound/Sound3DPlay.h"
#include "engine/TParticle.h"
#include "engine/FxSystem.h"
#include "Ninja/ninja.h"
#include "stage/TStageScore.h"
struct Vec3f;
struct Fx;
struct DrawTask;

typedef char TAniCamelOstrich_size_check[(sizeof(TAniCamelOstrich) == 0x4C0) ? 1 : -1];

inline void TAniCamelOstrich::useMove(AniMove* move)
{
	if (mMove != move) {
		move->enter();
		mMove = move;
	}
}

// 0x801BEC0C
extern "C" void* createCamelOstrich(TObject* parent)
{
	return new TAniCamelOstrich(parent);
}

const MotionEntry sCamelOstrichMotions[] = {
	{ 1, 0, -1, 0, 0.0f, 29.0f, 0.45f, 0.0f, 0 },  { 0, 4, -1, 0, 0.0f, 20.0f, 0.62f, 0.0f, 0 },
	{ 0, 4, -1, 0, 0.0f, 20.0f, 0.72f, 0.0f, 0 },  { 0, 9, -1, 0, 0.0f, 30.0f, 0.18f, 0.0f, 0 },
	{ 0, 10, -1, 0, 0.0f, 32.0f, 0.45f, 0.0f, 1 }, { 0, 8, -1, 0, 0.0f, 32.0f, 0.65f, 0.0f, 1 },
	{ 1, 3, -1, 0, 6.0f, 22.0f, 0.65f, 0.0f, 0 },  { 1, 5, -1, 0, 0.0f, 19.0f, 0.65f, 0.0f, 0 },
	{ 3, 5, -1, 3, 19.0f, 22.0f, 0.65f, 0.1f, 0 }, { 1, 1, -1, 0, 0.0f, 4.0f, 0.3f, 0.0f, 0 },
	{ 1, 7, -1, 0, 0.0f, 14.0f, 1.5f, 0.0f, 0 },   { 1, 6, -1, 0, 0.0f, 14.0f, 0.5f, 0.0f, 0 },
	{ 1, 2, -1, 0, 0.0f, 29.0f, 0.42f, 0.0f, 0 },  { 1, 11, -1, 0, 0.0f, 14.0f, 0.5f, 0.0f, 0 },
	{ 1, 0, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
};

const AniNode sCamelOstrichModelNodes[3] = { { 0, 5 }, { 1, 2 }, { 2, 0x3D } };

const char* gTAniCamelOstrichName = "TAniCamelOstrich";

// 0x801BEC58
TAniCamelOstrich::TAniCamelOstrich(TObject* parent)
    : TAniMount(parent, 0x17)
    , mMove(0)
{
	mBirth = 0;
	_00    = (u32)gTAniCamelOstrichName;
	mSize  = 0x4C0;

	mBodyHit.mOwnerId  = mObjId;
	mBodyHit.mRadius   = 10.0f;
	mBodyHit.mOffset   = sCamelBodyOffset;
	mReachHit.mOwnerId = mObjId;
	mReachHit.mActive  = 0;
	mReachHit.mRadius  = 20.0f;
	mReachHit.mKind    = 1;
	mReachHit.mNode    = 2;
	mHits.mList.mBody  = &mBody;
	mHits.mList.mModel = &mModel;
	mHits.mList.mLife  = &mLife;
	mHits.mList.add(&mBodyHit);
	// The original stores mHp twice and leaves mMax alone
	mLife.mHp    = 100;
	mLife.mHp    = 100;
	mBlink.mLife = &mLife;
	mBlink.mBody = &mBody;
	mDrawTask    = addDrawTask(this, (void (*)(void*))drawTask, 0x21, 0);
	mModel.setNodes(sCamelOstrichModelNodes, 3);
	mModel.update(&mAsset);
	mMotions.set(&mMotion, sCamelOstrichMotions);
	mMotions.update(&mAsset);
	mShadow._30 = 20.0f;
	mShadow._34 = 20.0f;
	mMotion.play(3);

	mWalk.setBody(&mBody);
	mWalk.mRunSpeed  = 150.0f;
	mWalk.mRunFrom   = 70.0f;
	mWalk.mWalkSpeed = 70.0f;
	mWalk.mAccel     = 0.2f;
	mWalk.mDecel     = 1.0f;
	mJump.setBody(&mBody);
	mJump.mGravity   = -400.0f;
	mJump.mJumpSpeed = 170.0f;
	mJump.mAirSpeed  = 60.0f;
	mDash.setBody(&mBody);
	mDash.mSpeed = 120.0f;
	mDash.mBrake = -10.0f;
	mBlast.setBody(&mBody);
	mBlast.mPush = 400.0f;
	mBlast.mDrag = 0.2f;
}

// 0x801BF064
TAniCamelOstrich::~TAniCamelOstrich()
{
	setState(-1);
	removeDrawTask(mDrawTask);
}

// 0x801BF1F8
void TAniCamelOstrich::onLoaded()
{
	mPos       = mStartPos;
	mRotY      = mStartRot;
	mBody.mPos = mPos;
	updateMatrix();
	updateShadow();
	mFxBank = mAsset.mFile->mFxBank;
	setState(0);
}

// 0x801BF27C
void TAniCamelOstrich::vfn_94(f32* out)
{
	mModel.getMatrix(0, out);
}

// 0x801BF2A8
void TAniCamelOstrich::preUpdate()
{
	mMotions.update(&mAsset);
	mModel.update(&mAsset);
	mHits.mList.hitAll();
	mBlink.update();
	if (flagByteTest(&mFlags2, 0) == 0) {
		if (mBlink.getResult() == 1) {
			flagByteSet(&mFlags2, 0);
			sound(0x1F8);
			setState(0xC);
		} else if (mBlink.getResult() == 2) {
			flagByteSet(&mFlags2, 0);
			sound(0x1F8);
			die();
		} else if (flagTest(&mBlink.mFlags, 0) != 0) {
			flagClear(&mBlink.mFlags, 0);
			setState(0xB);
		}
	}
}

// 0x801BF3A4
void TAniCamelOstrich::postUpdate()
{
	if (mMove != 0) {
		mMove->step();
	}
	mBody.update();
	mPos = mBody.mPos;
	mMotion.update();
	updateMatrix();
	updateShadow();
}

// 0x801BF414
void TAniCamelOstrich::vfn_10()
{
	Vec3f out;
	if (cameraIsVisible(1000.0f, 10.0f, &mPos, &out) == 0) {
		mDrawTask->mFlags |= 1;
	} else {
		mDrawTask->mFlags &= ~1;
	}
}

// 0x801BF47C
void TAniCamelOstrich::drawTask(TAniCamelOstrich* self)
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

// 0x801BF564
void TAniCamelOstrich::updateMatrix()
{
	f32 quat[4];
	*(Quat*)quat = mBody.mQuat;
	mtxSetTransV(mMatrix, &mPos);
	mtxConcatQuat(mMatrix, mMatrix, quat);
	mModel.pose(&mAsset, mMatrix, &mMotion);
}

// 0x801BF5DC
void TAniCamelOstrich::updateShadow()
{
	mHits.mList.updateAll();
	u8 ground[0x10];
	if (mBody.touching(6, 0, (Vec3f*)ground, 0) == 0) {
		shadowDisable(&mShadow);
	} else {
		shadowSetGround(&mShadow, (const Vec3f*)ground, (const Vec3f*)0);
		shadowEnable(&mShadow);
	}
}

// 0x801BF650
int TAniCamelOstrich::canMount() const
{
	u8 can = 0;
	if (mState == 1 && mJump.state() == 3) {
		can = 1;
	}
	return can;
}

// 0x801BF69C
int TAniCamelOstrich::canDismount()
{
	return mState == 2;
}

// 0x801BF6B0
void TAniCamelOstrich::onMount()
{
	setState(2);
}

// 0x801BF6D4
void TAniCamelOstrich::onDismount()
{
	setState(1);
}

// 0x801BF6F8
void TAniCamelOstrich::playWalk(int motion)
{
	int current = *(volatile s16*)&mMotion.mCurrent;
	if ((mMotion.getUser(current) & 1) == 0) {
		mMotion.playBlend(motion, 0.1f);
	} else if (current != motion) {
		mMotion.play(motion);
	}
}

// 0x801BF764
void TAniCamelOstrich::feed(const PlyInput* input, AniMove* move)
{
	move->setStick(input->mStick, input->mAngle);
}

// 0x801BF790
void TAniCamelOstrich::sound(int id)
{
	aniMountSound(this, id, &mPos);
}

// 0x801BF7B4
void TAniCamelOstrich::bump(const Vec3f* pos)
{
	particleOnceById(this, pos, 0, 0);
}

// 0x801BF7DC
void TAniCamelOstrich::dust(const Vec3f* pos)
{
	f32 matrix[12];
	mtxSetTransV(matrix, pos);
	mtxRotateY(matrix, matrix, mRotY);
	mtxScale(2.0f, 2.0f, 2.0f, matrix, matrix);
	fxSpawnAt(1, 0, 0xF, matrix);
}

// 0x801BF848
void TAniCamelOstrich::land()
{
	bump(&mPos);
	rumble(2);
	sound(0x1FD);
	mMotion.play(8);
	setState(2);
}

// 0x801BF8A8
int TAniCamelOstrich::onHit(const TObjAction* other, int arg, int* result)
{
	if (other == this) {
		return 0;
	}
	HitInfo* hit = (HitInfo*)result;
	if (arg == 0) {
		hit->mDamage = 0x19;
		hit->mFlags.setBit(0xD);
	} else if (arg == 1) {
		hit->mDamage = 0x19;
		hit->mFlags.setBit(0xD);
		hit->_14 = mHitValue;
	}
	return 1;
}

// 0x801BF92C
void TAniCamelOstrich::runState(int state, int phase)
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
		runDash(phase);
		break;
	case 5:
		runJump(phase);
		break;
	case 6:
		runFall(phase);
		break;
	case 7:
		runCharge(phase);
		break;
	case 8:
		runSpit(phase);
		break;
	case 9:
		runBreath(phase);
		break;
	case 10:
		runBreathEnd(phase);
		break;
	case 11:
		runHurt(phase);
		break;
	case 12:
		runDead(phase);
		break;
	}
}

// 0x801BFA00
void TAniCamelOstrich::runBirth(int phase)
{
	if (phase == 0) {
		mMotion.play(0xE);
		TAniBirth* birth = new TAniBirth(this, this, (ItemHolder*)&mAsset, &mBody);
		mBirth           = birth;
		mBirth->mFxBank  = 0x23;
		mBirth->mOffset.set(0.0f, 10.0f, 0.0f);
		mBirth->mTextId = 0x1D;
		TAniBirth* at   = mBirth;
		at->mTextAtA.x  = 0.0f;
		at->mTextAtA.y  = 20.0f;
		at->mTextAtA.z  = 0.0f;
		at->mTextAtB.x  = 60.0f;
		at->mTextAtB.y  = 25.0f;
		at->mTextAtB.z  = 20.0f;
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
			mMotion.play(0);
			sePlay3D(0x1F7, &mPos);
		}
		if (flagByteTest(&mBirth->mFlags2, 1) != 0) {
			flagByteClear(&mBirth->mFlags2, 1);
			mMotion.play(3);
		}
	}
}

// 0x801BFB8C
void TAniCamelOstrich::runIdle(int phase)
{
	if (phase == 0) {
		mMotion.playBlend(3, 0.1f);
		useMove(&mJump);
	}
}

// 0x801BFBF4
void TAniCamelOstrich::runStand(int phase)
{
	if (phase == 0) {
		useMove(&mWalk);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mWalk.state() == 4) {
			setState(6);
		} else if ((input->mPressed & 1) != 0) {
			feed(input, &mJump);
			setState(5);
		} else if ((input->mPressed & 2) != 0) {
			setState(7);
		} else {
			feed(input, &mWalk);
			if (input->mStick > 0.0f) {
				setState(3);
			}
		}
	}
}

// 0x801BFCEC
void TAniCamelOstrich::runWalk(int phase)
{
	if (phase == 0) {
		useMove(&mWalk);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mWalk.state() == 4) {
			setState(6);
		} else if ((input->mPressed & 1) != 0) {
			feed(input, &mJump);
			setState(5);
		} else if ((input->mPressed & 2) != 0) {
			setState(7);
		} else {
			feed(input, &mWalk);
			if (mWalk.state() == 0) {
				mMotion.playBlend(3, 0.1f);
				setState(2);
			}
		}
	} else if (phase == 3) {
		switch (mWalk.state()) {
		case 1:
			playWalk(4);
			if ((*(volatile u32*)&mMotion.mFlags & 0x100) != 0) {
				mMotion.mFlags &= ~0x100;
				sound(0x1F9);
				rumble(2);
			}
			break;
		case 2:
		case 3:
			playWalk(5);
			if ((*(volatile u32*)&mMotion.mFlags & 0x100) != 0) {
				mMotion.mFlags &= ~0x100;
				sound(0x1FA);
				rumble(2);
			}
			break;
		}
		mRotY = mWalk.mRotY;
	}
}

// 0x801BFEA4
void TAniCamelOstrich::runDash(int phase)
{
	if (phase == 0) {
		useMove(&mDash);
		mMotion.play(1);
		sePlay3DHeld(400.0f, &mSound[0], 0x1FE, 0xC8, (const Vec3f*)&mPos);
	} else if (phase == 1) {
		seStop3D(&mSound[0]);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mDash.state() == 2) {
			land();
		} else if (mDash.state() == 1) {
			setState(6);
		} else if ((input->mNotHeld & 1) != 0) {
			setState(6);
		} else {
			feed(input, &mDash);
		}
	} else if (phase == 3) {
		mSound[0].mPos = mPos;
	}
}

// 0x801BFFD0
void TAniCamelOstrich::runJump(int phase)
{
	if (phase == 0) {
		useMove(&mJump);
		mJump.launch();
		mMotion.play(6);
		rumble(9);
		sound(0x1FC);
		bump(&mPos);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mJump.state() == 3) {
			land();
		} else if ((input->mHeld & 1) != 0 && mJump.state() == 1) {
			setState(4);
		} else {
			feed(input, &mJump);
		}
	} else if (phase == 3) {
		if (mMotion.mCurrent == 6 && mJump.state() == 1) {
			mMotion.play(7);
		}
		mRotY = mWalk.mRotY;
	}
}

// 0x801C0104
void TAniCamelOstrich::runFall(int phase)
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
	}
}

// 0x801C0198
void TAniCamelOstrich::runCharge(int phase)
{
	if (phase == 0) {
		mTimer = 0;
		mMotion.play(0xB);
		mBreathFx = fxSpawn(1, mFxBank, 5);
		if (mBreathFx != 0) {
			mModel.getMatrix(1, mBreathFx->mMatrix);
			njUnitRotPortion(mBreathFx->mMatrix);
		}
	} else if (phase == 1) {
		fxStop(mBreathFx);
	} else if (phase == 2) {
		if ((mInput.mNotHeld & 2) != 0) {
			if (mTimer == 0x1E) {
				setState(9);
			} else {
				setState(8);
			}
		}
	} else if (phase == 3) {
		if (mTimer == 0x1D) {
			sound(0x202);
		}
		if (mTimer < 0x1E) {
			mTimer++;
		}
		if (mBreathFx != 0) {
			mModel.getMatrix(1, mBreathFx->mMatrix);
			njUnitRotPortion(mBreathFx->mMatrix);
		}
	}
}

// 0x801C02C0
void TAniCamelOstrich::runSpit(int phase)
{
	if (phase == 0) {
		sound(0x201);
		mStep = 0;
		mMotion.play(0xA);
	} else if (phase == 1) {
		mHits.mList.remove(&mReachHit);
	} else if (phase == 2) {
		if ((s8)mStep == 2) {
			mMotion.playBlend(3, 0.1f);
			setState(2);
		}
	} else if (phase == 3) {
		switch (mStep) {
		case 0:
			if (mMotion.isDone() != 0) {
				mReachHit.mActive = 0;
				mHits.mList.addAt(&mReachHit);
				mMotion.play(0xD);
				mStep = 1;
				f32 matrix[12];
				mModel.getMatrix(2, matrix);
				mtxRotateY(matrix, matrix, 0x4000);
				fxSpawnAt(1, mFxBank, 3, matrix);
			}
			break;
		case 1:
			if (mMotion.isDone() != 0) {
				mStep = 2;
			}
			break;
		}
	}
}

// 0x801C040C
void TAniCamelOstrich::runBreath(int phase)
{
	if (phase == 0) {
		useMove(&mBlast);
		mMotion.play(0xA);
		sound(0x203);
		mBreathTime = 0x1E;
		mBreathFx   = fxSpawn(1, mFxBank, 4);
		if (mBreathFx != 0) {
			mModel.getMatrix(1, mBreathFx->mMatrix);
			mtxRotateY(mBreathFx->mMatrix, mBreathFx->mMatrix, 0x4000);
		}
		mReachHit.mActive = 1;
		mHits.mList.addAt(&mReachHit);
		mHitValue = scoreNewChain(3);
	} else if (phase == 1) {
		mHits.mList.remove(&mReachHit);
		fxStop(mBreathFx);
	} else if (phase == 2) {
		if (mBreathTime == 0) {
			mMotion.play(3);
			setState(2);
		} else if (mBlast.state() == 3) {
			setState(6);
		} else if (mBlast.state() == 1) {
			setState(0xA);
		}
	} else if (phase == 3) {
		if (mBreathTime != 0) {
			mBreathTime--;
		}
		if ((mBreathTime & 7) == 0) {
			dust(&mPos);
		}
		if (mBreathFx != 0) {
			mModel.getMatrix(1, mBreathFx->mMatrix);
			mtxRotateY(mBreathFx->mMatrix, mBreathFx->mMatrix, 0x4000);
		}
	}
}

// 0x801C05E0
void TAniCamelOstrich::runBreathEnd(int phase)
{
	if (phase == 0) {
		useMove(&mBlast);
		mMotion.play(0xB);
		sound(0x1FF);
		rumble(0xE);
	} else if (phase == 2) {
		if (mBlast.state() == 3) {
			setState(6);
		} else if (mBlast.state() == 2) {
			mMotion.playBlend(3, 0.1f);
			setState(2);
		}
	}
}

// 0x801C06B4
void TAniCamelOstrich::runHurt(int phase)
{
	if (phase == 0) {
		rumble(0xE);
		sound(0x1FF);
		mMotion.play(9);
		useMove(&mJump);
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

// 0x801C077C
void TAniCamelOstrich::runDead(int phase)
{
	if (phase == 0) {
		mMotion.play(0xC);
		useMove(&mJump);
		sound(0x200);
	} else if (phase == 2) {
		if (mMotion.isDone() != 0) {
			sound(0x1F8);
			die();
		}
	}
}

#include "animal/AniFlags.h"

// 0x801C09E0
AniBodyOffset sCamelBodyOffset(0.0f, 10.0f, 0.0f);
