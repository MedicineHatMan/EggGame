#include "animal/AniMount.h"
#include "engine/DrawTask.h"
#include "engine/MeshColli.h"
#include "player/PlyState.h"
#include "player/Player.h"
#include "animal/TAniCamelOstrich.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjLauncher.h"
#include "engine/ActorMotion.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "object/ObjShadow.h"
#include "animal/TAniMount.h"
#include "engine/TParticle.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "stage/TStageScore.h"
#include "animal/TAniHorseshoeCrabShark.h"
#include "enemy/Enemy.h"
struct Vec3f;
struct Fx;
struct DrawTask;

typedef char TAniHorseshoeCrabShark_size_check[(sizeof(TAniHorseshoeCrabShark) == 0x4D4) ? 1 : -1];

inline void TAniHorseshoeCrabShark::useMove(AniMove* move)
{
	if (mMove != move) {
		move->enter();
		mMove = move;
	}
}

// 0x801C3A90
extern "C" void* createHorseshoeCrabShark(TObject* parent)
{
	return new TAniHorseshoeCrabShark(parent);
}

const MotionEntry sHorseshoeCrabSharkMotions[] = {
	{ 0, 8, -1, 0, 0.0f, 30.0f, 0.35f, 0.0f, 0 },
	{ 0, 7, -1, 0, 0.0f, 30.0f, 0.3f, 0.0f, 0 },
	{ 0, 6, -1, 0, 0.0f, 30.0f, 0.45f, 0.0f, 0 },
	{ 1, 0, -1, 0, 0.0f, 19.0f, 0.37f, 0.0f, 0 },
	{ 1, 3, -1, 0, 5.0f, 7.0f, 0.3f, 0.0f, 0 },
	{ 1, 3, -1, 0, 8.0f, 17.0f, 0.3f, 0.0f, 0 },
	{ 2, 3, -1, 1, 18.0f, 20.0f, 0.3f, 0.0f, 0 },
	{ 0, 5, -1, 0, 0.0f, 20.0f, 0.36f, 0.0f, 0 },
	{ 1, 4, -1, 0, 0.0f, 44.0f, 0.45f, 0.0f, 0 },
	{ 1, 1, -1, 0, 0.0f, 4.0f, 0.3f, 0.0f, 0 },
	{ 1, 2, -1, 0, 0.0f, 25.0f, 0.45f, 0.0f, 0 },
	{ 1, 0, -1, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
};

const char* gTAniHorseshoeCrabSharkName = "TAniHorseshoeCrabShark";

// 0x801C3ADC
TAniHorseshoeCrabShark::TAniHorseshoeCrabShark(TObject* parent)
    : TAniMount(parent, 0x16), mBob(0), mMove(0)
{
	mBirth = 0;
	_00    = (u32)gTAniHorseshoeCrabSharkName;
	mSize  = 0x4D4;

	TObject* shotParent = new TObject(this);
	mShotParent = shotParent;

	mBodyHit.mOwnerId = mObjId;
	mBodyHit.mRadius  = 10.0f;
	mBodyHit.mOffset  = sHorseshoeBodyOffset;
	mDiveHit.mOwnerId = mObjId;
	mDiveHit.mActive  = 0;
	mDiveHit.mRadius  = 10.0f;
	mDiveHit.mKind      = 1;
	mDiveHit.mNode      = 2;
	mHits.mList.mBody  = &mBody;
	mHits.mList.mModel = &mModel;
	mHits.mList.mLife  = &mLife;
	mHits.mList.add(&mBodyHit);
	mLife.mHp = 100;
	mLife.mHp = 100; // set twice in the original
	mBody.setMask(4);
	mBlink.mLife = &mLife;
	mBlink.mBody = &mBody;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawTask, 0x21, 0);
	mModel.setNodes(sHorseshoeCrabSharkModelNodes, 3);
	mModel.update(&mAsset);
	mMotions.set(&mMotion, sHorseshoeCrabSharkMotions);
	mMotions.update(&mAsset);
	mShadow._30 = 20.0f;
	mShadow._34 = 20.0f;
	mMotion.play(1);

	mWalk.setBody(&mBody);
	mWalk.mRunSpeed = 100.0f;
	mWalk.mRunFrom = 70.0f;
	mWalk.mAccel = 0.1f;
	mWalk.mDecel = 0.1f;
	mWalk.mTilt = 1;
	mJump.setBody(&mBody);
	mJump.mGravity = -400.0f;
	mJump.mJumpSpeed = 200.0f;
	mJump.mAirSpeed = 60.0f;
	mDive.setBody(&mBody);
	mDive.mDrag = 0.985f;
	mDive.mSpeed = 120.0f;
	mDive.mStopSpeed = 40.0f;
	mSlam.setBody(&mBody);
	mSlam.mMaxFall = 300.0f;
	mSlam.mFallRate = 0.5f;
}

// 0x801C3E38
TAniHorseshoeCrabShark::~TAniHorseshoeCrabShark()
{
	setState(-1);
	removeDrawTask(mDrawTask);
}

// 0x801C3FCC
void TAniHorseshoeCrabShark::onLoaded()
{
	mPos  = mStartPos;
	mRotY = mStartRot;
	mBody.mPos = mPos;
	updateMatrix();
	updateShadow();
	mFxBank = mAsset.mFile->mFxBank;
	setState(0);
}

// 0x801C4050
void TAniHorseshoeCrabShark::vfn_94(f32* out)
{
	mModel.getMatrix(0, out);
}

// 0x801C407C
void TAniHorseshoeCrabShark::preUpdate()
{
	mMotions.update(&mAsset);
	mModel.update(&mAsset);
	mHits.mList.hitAll();
	mBlink.update();
	if (flagByteTest(&mFlags2, 0) == 0) {
		if (mBlink.getResult() == 1) {
			flagByteSet(&mFlags2, 0);
			sound(0x1ED);
			setState(0xB);
		} else if (mBlink.getResult() == 2) {
			flagByteSet(&mFlags2, 0);
			sound(0x1ED);
			die();
		} else if (flagTest(&mBlink.mFlags, 0) != 0) {
			flagClear(&mBlink.mFlags, 0);
			setState(0xA);
		}
	}
}

// 0x801C4178
void TAniHorseshoeCrabShark::postUpdate()
{
	if (mMove != 0) {
		mMove->step();
	}
	mBody.update();
	mPos = mBody.mPos;
	mMotion.update();
	updateMatrix();
	updateShadow();
	mBob += 0x200;
	mClock++;
}

// 0x801C4200
void TAniHorseshoeCrabShark::vfn_10()
{
	Vec3f out;
	if (cameraIsVisible(1000.0f, 10.0f, &mPos, &out) == 0) {
		mDrawTask->mFlags |= 1;
	} else {
		mDrawTask->mFlags &= ~1;
	}
}

// 0x801C4268
void TAniHorseshoeCrabShark::drawTask(TAniHorseshoeCrabShark* self)
{
	if (self->mBlink.isHidden() == 0) {
		f32 matrix[12];
		DrawState state;
		state.mFlags = 0;
		state._20 = 0;
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

// 0x801C4350
void TAniHorseshoeCrabShark::updateMatrix()
{
	f32 quat[4];
	*(Quat*)quat = mBody.mQuat;
	mtxSetTransV(mMatrix, &mPos);
	mtxTranslate((float*)mWakeMatrix, (const float*)mMatrix, 0.0f, 2.0f, 0.0f);
	mtxRotateY(mWakeMatrix, mWakeMatrix, mRotY);
	if (inWater() != 0) {
		mtxTranslate((float*)mMatrix, (const float*)mMatrix, 0.0f, -2.0f * njSin(mBob) + -5.0f, 0.0f);
	}
	mtxConcatQuat(mMatrix, mMatrix, quat);
	mModel.pose(&mAsset, mMatrix, &mMotion);
}

// 0x801C4428
void TAniHorseshoeCrabShark::updateShadow()
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

// 0x801C449C
int TAniHorseshoeCrabShark::canMount() const
{
	u8 can = 0;
	if (mState == 1 && mJump.state() == 3) {
		can = 1;
	}
	return can;
}

// 0x801C44E8
int TAniHorseshoeCrabShark::canDismount()
{
	return mState == 2;
}

// 0x801C44FC
void TAniHorseshoeCrabShark::onMount()
{
	setState(2);
}

// 0x801C4520
void TAniHorseshoeCrabShark::onDismount()
{
	setState(1);
}

// 0x801C4544
void TAniHorseshoeCrabShark::feed(const PlyInput* input, AniMove* move)
{
	move->setStick(input->mStick, input->mAngle);
}

// 0x801C4570
void TAniHorseshoeCrabShark::sound(int id)
{
	aniMountSound(this, id, &mPos);
}

// 0x801C4594
void TAniHorseshoeCrabShark::bump(const Vec3f* pos)
{
	particleOnceById(this, pos, 0, 0);
}

// 0x801C45BC
void TAniHorseshoeCrabShark::land()
{
	bump(&mPos);
	rumble(2);
	sound(0x1F2);
	mMotion.play(6);
	setState(2);
}

// 0x801C461C
int TAniHorseshoeCrabShark::inWater()
{
	u16 tri;
	MeshColli* mesh = mBody.touching(ANIRIDER_FLOOR, &tri, NULL, NULL);
	if (mesh != NULL) {
		return ((u32*)&mesh->mTris[tri])[2] & 4;
	}
	return 0;
}

// 0x801C4678
static void placeFx(AniModel* model, void* fx)
{
	f32 matrix[12];
	model->getMatrix(2, matrix);
	mtxRotateY((f32*)((u8*)fx + 0x2C), matrix, 0x4000);
}

// 0x801C46BC
void TAniHorseshoeCrabShark::startDiveFx()
{
	if (mFx == 0) {
		mFx = fxSpawn(1, mFxBank, 1);
		if (mFx != 0) {
			placeFx(&mModel, mFx);
		}
	}
	if (mFx2 == 0) {
		mFx2 = fxSpawnFollow(1, mFxBank, 4, mWakeMatrix);
	}
}

// 0x801C473C
int TAniHorseshoeCrabShark::onHit(const TObjAction* other, int arg, int* result)
{
	if (other == this) {
		return 0;
	}
	HitInfo* hit = (HitInfo*)result;
	if (arg == 0) {
		hit->mDamage = 0x19;
		hit->mFlags.setBit(0xD);
		return 1;
	}
	if (arg == 1) {
		hit->mDamage = 0x19;
		hit->mFlags.setBit(0xD);
		hit->_14 = mHitValue;
		return 1;
	}
	return 0;
}

// 0x801C47CC
void TAniHorseshoeCrabShark::runState(int state, int phase)
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
		runDive(phase);
		break;
	case 5:
		runSurface(phase);
		break;
	case 6:
		runJump(phase);
		break;
	case 7:
		runFall(phase);
		break;
	case 8:
		runSlam(phase);
		break;
	case 9:
		runStun(phase);
		break;
	case 10:
		runHurt(phase);
		break;
	case 11:
		runDead(phase);
		break;
	}
}

// 0x801C4894
void TAniHorseshoeCrabShark::runBirth(int phase)
{
	if (phase == 0) {
		mMotion.play(0xB);
		TAniBirth* birth = new TAniBirth(this, this, (ItemHolder*)&mAsset, &mBody);
		mBirth = birth;
		mBirth->mFxBank    = 0x23;
		mBirth->mTextId    = 0x1C;
		TAniBirth* at = mBirth;
		at->mTextAtA.x = 0.0f;
		at->mTextAtA.y = 6.0f;
		at->mTextAtA.z = 0.0f;
		at->mTextAtB.x = 60.0f;
		at->mTextAtB.y = 10.0f;
		at->mTextAtB.z = 10.0f;
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
			mMotion.play(3);
			sePlay3D(0x1EC, &mPos);
		}
		if (flagByteTest(&mBirth->mFlags2, 1) != 0) {
			flagByteClear(&mBirth->mFlags2, 1);
			mMotion.play(1);
		}
	}
}

// 0x801C4A0C
void TAniHorseshoeCrabShark::runIdle(int phase)
{
	if (phase == 0) {
		mMotion.playBlend(1, 0.1f);
		useMove(&mJump);
	}
}

// 0x801C4A74
void TAniHorseshoeCrabShark::runStand(int phase)
{
	if (phase == 0) {
		useMove(&mWalk);
		mFx = 0;
	} else if (phase == 1) {
		fxStop(mFx);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mWalk.state() == 4) {
			setState(7);
		} else if ((input->mPressed & 1) != 0) {
			feed(input, &mJump);
			setState(6);
		} else if ((input->mPressed & 2) != 0) {
			feed(input, &mDive);
			setState(4);
		} else {
			feed(input, &mWalk);
			if (input->mStick > 0.0f) {
				setState(3);
			}
		}
	} else if (phase == 3) {
		if (inWater() != 0) {
			if (mFx == 0) {
				mFx = fxSpawnFollow(1, 0, 0x81, mWakeMatrix);
			}
		} else if (mFx != 0) {
			fxStop(mFx);
			mFx = 0;
		}
	}
}

// 0x801C4BF0
void TAniHorseshoeCrabShark::runWalk(int phase)
{
	if (phase == 0) {
		useMove(&mWalk);
		mMotion.play(0);
		mFx = 0;
	} else if (phase == 1) {
		fxStop(mFx);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mWalk.state() == 4) {
			setState(7);
		} else if ((input->mPressed & 1) != 0) {
			feed(input, &mJump);
			setState(6);
		} else if ((input->mPressed & 2) != 0) {
			feed(input, &mDive);
			setState(4);
		} else {
			feed(input, &mWalk);
			if (mWalk.state() == 0) {
				mMotion.playBlend(1, 0.1f);
				setState(2);
			}
		}
	} else if (phase == 3) {
		if ((*(volatile u32*)&mMotion.mFlags & 0x100) != 0) {
			mMotion.mFlags &= ~0x100;
			sound(0x1EE);
			rumble(2);
		}
		if (inWater() != 0) {
			if (mFx == 0) {
				mFx = fxSpawnFollow(1, 0, 0x84, mWakeMatrix);
			}
		} else if (mFx != 0) {
			fxStop(mFx);
			mFx = 0;
		}
		mRotY = mWalk.mRotY;
	}
}

// 0x801C4DC0
void TAniHorseshoeCrabShark::runDive(int phase)
{
	if (phase == 0) {
		useMove(&mDive);
		mMotion.play(7);
		mHits.mList.add(&mDiveHit);
		sound(0x1F6);
		mDives     = 1;
		mDiveFxOff = 0;
		mFx2 = 0;
		mFx  = 0;
		startDiveFx();
	} else if (phase == 1) {
		mHits.mList.remove(&mDiveHit);
		fxStop(mFx);
		fxStop(mFx2);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mDive.state() == 4) {
			setState(7);
		} else if (mDive.state() == 2) {
			mMotion.playBlend(1, 0.1f);
			setState(2);
		} else if (mDive.state() == 3) {
			setState(5);
		} else {
			if (mDives < 3 && (input->mPressed & 2) != 0 && mDive.resume() != 0) {
				sound(0x1F6);
				if ((s8)mDiveFxOff == 1) {
					mDiveFxOff = 0;
					startDiveFx();
				}
				mDives++;
			}
			if ((input->mPressed & 1) != 0) {
				setState(6);
			} else {
				feed(input, &mDive);
			}
		}
	} else if (phase == 3) {
		if (mDiveFxOff == 0 && mDive.state() == 1) {
			fxStop(mFx);
			fxStop(mFx2);
			mFx2 = 0;
			mFx  = 0;
			mDiveFxOff = 1;
		}
		if (mFx != 0) {
			placeFx(&mModel, mFx);
		}
		mRotY = mDive.mRotY;
	}
}

// 0x801C4FF8
void TAniHorseshoeCrabShark::runSurface(int phase)
{
	if (phase == 0) {
		useMove(&mDive);
		rumble(0xE);
		f32 matrix[12];
		mModel.getMatrix(2, matrix);
		fxSpawnAt(1, mFxBank, 0xD, matrix);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mDive.state() == 4) {
			setState(7);
		} else if (mDive.state() == 2) {
			mMotion.playBlend(1, 0.1f);
			setState(2);
		} else {
			feed(input, &mDive);
		}
	} else if (phase == 3) {
		mRotY = mDive.mRotY;
	}
}

// 0x801C5104
void TAniHorseshoeCrabShark::runJump(int phase)
{
	if (phase == 0) {
		useMove(&mJump);
		mJump.launch();
		f32 quat[4];
		njCreateQuaternionV((const Vec3f*)&gVecUp, mRotY, (NJS_QUATERNION*)quat);
		mBody.mQuat = *(Quat*)quat;
		mMotion.play(4);
		rumble(9);
		sound(0x1F1);
		bump(&mPos);
	} else if (phase == 2) {
		PlyInput* input = &mInput;
		if (mJump.state() == 3) {
			land();
		} else if ((input->mPressed & 1) != 0) {
			setState(8);
		} else {
			feed(input, &mJump);
		}
	} else if (phase == 3) {
		if (mMotion.mCurrent == 4 && mJump.state() == 1) {
			mMotion.play(5);
		}
		mRotY = mWalk.mRotY;
	}
}

// 0x801C525C
void TAniHorseshoeCrabShark::runFall(int phase)
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

// 0x801C52F0
void TAniHorseshoeCrabShark::runSlam(int phase)
{
	if (phase == 0) {
		mSlam.mFall = 0.0f;
		useMove(&mSlam);
		mHitValue = scoreNewChain(3);
		mMotion.play(8);
	} else if (phase == 2) {
		if (mSlam.state() == 1) {
			f32 matrix[12];
			mtxSetTransV(matrix, &mPos);
			fxSpawnAt(1, mFxBank, 8, matrix);
			new TAniSharkShockWave(mShotParent, (u16)mObjId, &mPos, 10.0f, 50.0f);
			rumble(0x15);
			camShakeNearHard(200.0f, &mPos);
			sound(0x1F3);
			mMotion.play(6);
			setState(9);
		}
	}
}

// 0x801C540C
void TAniHorseshoeCrabShark::runStun(int phase)
{
	if (phase == 0) {
		mWait = 0x3C;
		mBody.mVel = gVecZero;
		useMove(&mWalk);
	} else if (phase == 2) {
		if (mWalk.state() == 4) {
			setState(7);
		} else if (mWait != 0) {
			if (--mWait == 0) {
				setState(2);
			}
		}
	}
}

// 0x801C54E0
void TAniHorseshoeCrabShark::runHurt(int phase)
{
	if (phase == 0) {
		rumble(0xE);
		sound(0x1F4);
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

// 0x801C55A8
void TAniHorseshoeCrabShark::runDead(int phase)
{
	if (phase == 0) {
		mMotion.play(0xA);
		useMove(&mJump);
		sound(0x1F5);
	} else if (phase == 2) {
		if (mMotion.isDone() != 0) {
			sound(0x1ED);
			die();
		}
	}
}

// 0x801C5648
Vec3f* TAniHorseshoeCrabShark::getPos()
{
	return &mPos;
}

// 0x801C5650
int TAniHorseshoeCrabShark::getRotY()
{
	return mRotY;
}

// 0x801C5658
// its static initializer
AniBodyOffset sHorseshoeBodyOffset(0.0f, 10.0f, 0.0f);
