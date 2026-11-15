#include "egg/TMorphEggAnim2.h"
#include "engine/Color.h"
#include "Ninja/ninja.h"

typedef char TMorphEggAnim2_size_check[(sizeof(TMorphEggAnim2) == 0x7C) ? 1 : -1];

const char* gTMorphEggAnim2Name = "TMorphEggAnim2";

// 0x800881C8
TMorphEggAnim2::TMorphEggAnim2(TObject* parent, int kind)
    : TObject(parent)
{
	_00          = (u32)gTMorphEggAnim2Name;
	mSize        = 0x7C;
	mDraw.mFlags = 0x1000;
	mDraw._20    = mBasis;
	mChild       = 0;
	mHatched     = 0;
	mOpened      = 0;
	mFinished    = 0;
	mAngle       = 0;
	mSpin        = 0x400;
	mState       = kind;
	switch (mState) {
	case 0:
		mScale         = 0.9f;
		mDraw.mAmbient = 0;
		break;
	case 2:
		mScale         = 0.1f;
		mDraw.mAmbient = 0xff;
		break;
	}
}

// 0x8008829C
TMorphEggAnim2::~TMorphEggAnim2()
{
	if (mChild != 0) {
		mChild->mFlags |= 1;
	}
}

// 0x80088314
void TMorphEggAnim2::draw()
{
	f32 s;
	f32 c;

	switch (mState) {
	case 0:
		if ((mDraw.mAmbient & 0xff) < 0xcc) {
			addColorAlpha(&mDraw.mAmbient, 8);
		} else {
			setColorAlpha(&mDraw.mAmbient, 0xcc);
			mState  = 1;
			mOpened = 1;
		}
		break;
	case 1:
		if (mScale > 0.1f) {
			mScale -= 0.03f;
		} else {
			mScale   = 0.1f;
			mState   = 4;
			mHatched = 1;
		}
		break;
	case 2:
		if (mScale < 0.9f) {
			mScale += 0.02f;
		} else {
			mScale    = 0.9f;
			mState    = 3;
			mFinished = 1;
		}
		break;
	case 3:
		if ((mDraw.mAmbient & 0xff) != 0) {
			addColorAlpha(&mDraw.mAmbient, -4);
		} else {
			setColorAlpha(&mDraw.mAmbient, 0);
			mState   = 4;
			mHatched = 1;
		}
		break;
	case 4:
		break;
	}
	s = mScale * njSin(mAngle);
	c = mScale * njCos(mAngle);
	mAngle += mSpin;
	mAngle &= 0xffff;
	mBasis[0] = s;
	mBasis[1] = c;
	mBasis[2] = 0.0f;
	mBasis[3] = c;
	mBasis[4] = s;
	mBasis[5] = 0.0f;
	mDraw._20 = mBasis;
}
