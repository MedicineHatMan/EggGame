#include "player/TPlyEyelid.h"

typedef char TPlyEyelid_size_check[(sizeof(TPlyEyelid) == 0x2C) ? 1 : -1];

const char* gTPlyEyelidName = "TPlyEyelid";

// 0x80040900
TPlyEyelid::TPlyEyelid(TObject* parent)
    : TObject(parent)
{
	mState = 0;
	mTimer = 0;
	mOpen  = 1.0f;
	_00    = (u32)gTPlyEyelidName;
	mSize  = 0x2C;
}

// 0x80040960
void TPlyEyelid::close()
{
	mState = 3;
	mOpen  = 0.0f;
}

// 0x80040974
void TPlyEyelid::reset()
{
	mState = 0;
	mOpen  = 1.0f;
	mTimer = 0;
}

// 0x8004098C
void TPlyEyelid::draw()
{
	switch (mState) {
	case 0:
		if (mTimer < 0xB4) {
			mTimer = mTimer + 1;
		} else {
			mState = mState + 1;
			mTimer = 1;
		}
		break;
	case 1:
		if (mTimer < 2) {
			mOpen  = (f32)mTimer / 2.0f;
			mTimer = mTimer + 1;
		} else {
			mOpen  = 0.0f;
			mTimer = 0;
			mState = 2;
		}
		break;
	case 2:
		if (mTimer < 5) {
			mOpen  = (f32)mTimer / 5.0f;
			mTimer = mTimer + 1;
		} else {
			mOpen  = 1.0f;
			mTimer = 0;
			mState = 0;
		}
		break;
	}
}
