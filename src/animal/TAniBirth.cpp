#include "animal/AniMount.h"
#include "io/Resource.h"
#include "camera/TCamera.h"
#include "egg/TMorphEggAnim2.h"
#include "stage/TEventCtrl.h"
#include "engine/FxSystem.h"
#include "object/TObjLauncher.h"
#include "core/MathUtil.h"
#include "player/Player.h"
#include "animal/TAniCamelOstrich.h"
#include "sound/SoundCtrl.h"
#include "egg/TEggContentsIntro.h"
#include "core/VecMath.h"
#include "animal/TAniBirth.h"
struct Fx;
struct TEggContentsIntro;

static void applyBody(AniBody* body, f32* matrix);

const char* gTAniBirthName = "TAniBirth";

// 0x801D0630
TAniBirth::TAniBirth(TObject* parent, TObjAction* owner, ItemHolder* holder, AniBody* body)
    : TObject(parent)
    , mState(7)
{
	mFxBank     = 0;
	mTextId     = 0;
	mWait       = 0x14;
	mEgg        = 0;
	mHolder     = holder;
	mOwner      = owner;
	mFx         = 0;
	mBody       = body;
	mHoldsOwner = 0;
	mText       = 0;
	_00         = (u32)gTAniBirthName;
	mSize       = 0x78;
	mOffset.x   = 0.0f;
	mOffset.y   = 0.0f;
	mOffset.z   = 0.0f;
	mTextAtA.x  = 0.0f;
	mTextAtA.y  = 6.0f;
	mTextAtA.z  = 0.0f;
	mTextAtB.x  = 0.0f;
	mTextAtB.y  = 10.0f;
	mTextAtB.z  = 28.0f;
	setState(0);
}

// 0x801D0708
TAniBirth::~TAniBirth()
{
	runState(mState, 1);
	if (mHoldsOwner != 0) {
		eventEnd();
	}
	fxStop((Fx*)mFx);
}

// 0x801D0790
void TAniBirth::getDrawState(DrawState* out) const
{
	if (mState == 0 || (u8)(mState - 1) <= 1 || mState == 3) {
		*out = mEgg->mDraw;
	} else {
		*out = *(const DrawState*)getDefaultDrawState();
	}
}

// 0x801D087C
void TAniBirth::draw()
{
	runState(mState, 2);
	runState(mState, 3);
}

// 0x801D08C8
void TAniBirth::setState(int state)
{
	if (mState != 7) {
		runState(mState, 1);
	}
	if (state != 7) {
		runState(state, 0);
	}
	mState = state;
}

// 0x801D0928
void TAniBirth::runState(int state, int phase)
{
	switch (state) {
	case 0:
		runMakeEgg(phase);
		break;
	case 1:
		runEggEffect(phase);
		break;
	case 2:
		runWaitOwner(phase);
		break;
	case 3:
		runEggModel(phase);
		break;
	case 4:
		runAnimal(phase);
		break;
	case 5:
		runText(phase);
		break;
	case 6:
		runEnd(phase);
		break;
	}
}

// 0x801D09B4
static void applyBody(AniBody* body, f32* matrix)
{
	Vec3f pos;
	pos = body->mPos;
	Quat quat;
	quat = body->mQuat;
	mtxSetTransV(matrix, &pos);
	mtxConcatQuat(matrix, matrix, &quat.w);
}

// 0x801D0A30
void TAniBirth::runMakeEgg(int phase)
{
	if (phase == 0) {
		mEgg = (AniBirthEgg*)new TMorphEggAnim2(this, 2);
	} else if (phase == 2) {
		setState(1);
	}
}

// 0x801D0A9C
void TAniBirth::runEggEffect(int phase)
{
	if (phase == 0) {
		sePlayFor(0x2A0, 0x258);
		Vec3f pos;
		pos = mBody->mPos;
		f32 matrix[12];
		mtxSetTransV(matrix, &pos);
		mFx = fxSpawnAt(1, 7, mFxBank, matrix);
	} else if (phase == 2) {
		if (mEgg->mFinished != 0) {
			setState(2);
		}
	}
}

// 0x801D0B3C
void TAniBirth::runWaitOwner(int phase)
{
	if (phase == 2) {
		if (eventStartById(mOwner->mObjId, 0) != 0) {
			mHoldsOwner = 1;
			setState(3);
		}
	}
}

// 0x801D0B98
void TAniBirth::runEggModel(int phase)
{
	if (phase == 0) {
		mModel
		    = createCamPath((s8)mOwner->mPlayer, 2, ((void**)gAnimalCommonModel[1])[0], ((void**)gAnimalCommonModel[2])[0], 0, 0.0f, 0.18f);
		mModel->attachToMtx();
		applyBody(mBody, mModel->getMtx());
		mtxTranslateV(mModel->getMtx(), mModel->getMtx(), &mOffset);
	} else if (phase == 1) {
		mModel->mFlags |= 1;
		if (mEgg != 0) {
			mEgg->mFlags |= 1;
			mEgg = 0;
		}
	} else if (phase == 2) {
		if (mEgg->mHatched != 0 && mModel->isDone() != 0) {
			fxStop(mFx);
			mFx = 0;
			flagByteSet(&mFlags2, 0);
			setState(4);
		}
	}
}

// 0x801D0CCC
void TAniBirth::runAnimal(int phase)
{
	if (phase == 0) {
		void** assets = (void**)findArcExport(mHolder->mEntry->mData.mArc, "AniModel");
		mModel        = createCamPath((s8)mOwner->mPlayer, 2, ((void**)assets[2])[0], ((void**)assets[3])[0], 0, 0.0f, 0.31f);
		mModel->attachToMtx();
		applyBody(mBody, mModel->getMtx());
		mtxTranslateV(mModel->getMtx(), mModel->getMtx(), &mOffset);
		mBody->mVel = sBirthJump;
		flagClear(&mBody->mFlags, 0);
		mSubState = 0;
	} else if (phase == 1) {
		mModel->mFlags |= 1;
	} else if (phase == 2) {
		switch (mSubState) {
		case 0:
			if (mModel->isDone() != 0) {
				mSubState = 1;
			}
			break;
		case 1:
			if (mBody->mVel.y < 0.0f && mBody->touching(5, 0, NULL, NULL) != 0) {
				mSubState = 2;
			}
			break;
		case 2:
			if (mWait != 0) {
				mWait--;
			}
			if (mWait == 0) {
				if (mTextId == 0 || *(int*)((u8*)mOwner + 0x44) == 0) {
					setState(6);
				} else {
					setState(5);
				}
			}
			break;
		}
	} else if (phase == 3) {
		mBody->mVel.addY(-4.9f);
		applyBody(mBody, mModel->getMtx());
	}
}

// 0x801D0EF4
void TAniBirth::runText(int phase)
{
	if (phase == 0) {
		mText = (AniBirthPart*)createTEggContentsIntro(this, mTextId);
		Vec3f pos;
		pos = mBody->mPos;
		Vec3f atA;
		vecAdd(&atA, &pos, &mTextAtA);
		Vec3f atB;
		vecAdd(&atB, &pos, &mTextAtB);
		mTextBack = (AniBirthPart*)createTCamFix((s8)mOwner->mPlayer, 2, &atA, &atB);
		flagByteSet(&mFlags2, 1);
	} else if (phase == 1) {
		mTextBack->mFlags |= 1;
		mText->mFlags |= 1;
	} else if (phase == 2) {
		if (mText->mDone != 0) {
			setState(6);
		}
	}
}

// 0x801D0FE8
void TAniBirth::runEnd(int phase)
{
	if (phase == 0) {
		eventEnd();
		mHoldsOwner = 0;
	}
}

// The __sinit at 0x801D1024 fills this
BirthJump sBirthJump(0.0f, 130.0f, 0.0f);
