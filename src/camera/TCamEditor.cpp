#include "camera/TCamera.h"
#include "core/Pad.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "camera/TCameraCtrl.h"
#include "engine/TFogCtrl.h"
#include "object/ObjAction.h"
#include "camera/TCamEditor.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "core/TMainTask.h"
#include "animal/RacerMover.h"
#include "core/SdkCalls.h"

typedef char TCamEditor_size_check[(sizeof(TCamEditor) == 0x64) ? 1 : -1];

// 0x800156D4
// Called from TCamera.cpp
extern "C" TCamera* createTCamEditor(int player, int look, void* user)
{
	return new TCamEditor(gTL_01, player, look, user);
}

const char* gTCamEditorName = "TCamEditor";

// 0x80015734
TCamEditor::TCamEditor(TObject* parent, int player, int look, void* user)
    : TCamera(parent, player, look)
{
	_00     = (u32)gTCamEditorName;
	mSize   = 0x64;
	mMoveAt = (BOOL)user;
	init();
	apply();
}

// 0x80015798
TCamEditor::~TCamEditor()
{
	((CamView*)mView)->mRoll = 0;
}

// 0x80015800
// How fast a push of value goes at a speed as it is at 0 else a fixed step its way
f32 editSpeed(int speed, f32 value)
{
	f32 sign;

	if (speed == 0) {
		return value;
	}
	if (value > 0.0f) {
		sign = 1.0f;
	} else if (value < 0.0f) {
		sign = -1.0f;
	} else {
		sign = 0.0f;
	}
	switch (speed) {
	case 1:
		return 2.0f * sign;
	case 2:
		return 20.0f * sign;
	case 3:
		return 120.0f * sign;
	}
	return value;
}

// 0x80015874
// With the button the stick raises and lowers the point
void editMoveAt(f32 scale, Vec3f* at, int pad, int yaw, int speed)
{
	if (padGet(pad)->mHold & 0x800) {
		f32 v = editSpeed(speed, padGet(pad)->mStickY);
		at->y += v * scale;
	} else if (!(padGet(pad)->mHold & 0x200)) {
		PadState* state = padGet(pad);
		f32 s;
		f32 c;
		f32 len = 64.0f;
		len *= state->mLength;
		f32 step = editSpeed(speed, len);
		step *= scale;
		njSinCos(state->mAngle - 0x4000 + yaw, &s, &c);
		at->z += c * step;
		at->x += s * step;
	}
}

// 0x80015980
void TCamEditor::init()
{
	mPitch                   = -0xA00;
	mYaw                     = 0x8000;
	mDist                    = 100.0f;
	((CamView*)mView)->mRoll = 0;
	mSpeed                   = 0;
}

// 0x800159B0
void TCamEditor::apply()
{
	((CamView*)mView)->mPos.x = 0.0f;
	((CamView*)mView)->mPos.y = 0.0f;
	((CamView*)mView)->mPos.z = mDist;
	njPushMatrixEx();
	njUnitMatrix(0);
	njTranslateEx(&((CamView*)mView)->mTarget);
	njRotateY(0, mYaw);
	njRotateX(0, mPitch);
	njCalcPoint(0, &((CamView*)mView)->mPos, &((CamView*)mView)->mPos);
	njPopMatrixEx();
}

// 0x80015A3C
void TCamEditor::vfn_20()
{
	int pad = camGetPad(mPlayer);
	int x   = padGet(pad)->mStickX;
	int y   = padGet(pad)->mStickY;

	if (padGet(pad)->mPressed & 0x40) {
		nextSpeed();
	}
	if (x != 0 || y != 0) {
		if (padGet(pad)->mHold & 0x200) {
			mPitch -= y * 4;
			mYaw += x * 4;
			if (mPitch >= 0x3C71) {
				mPitch = 0x3C71;
			} else if (mPitch <= -0x3C71) {
				mPitch = -0x3C71;
			}
		} else if (mMoveAt) {
			editMoveAt(0.1f, &((CamView*)mView)->mTarget, pad, mYaw - 0x8000, mSpeed);
		}
	}
	if (padGet(pad)->mHold & 0x800) {
		if (padGet(pad)->mHold & 0x8) {
			mDist -= 2.0f;
			if (mDist <= 5.0f) {
				mDist = 5.0f;
			}
		} else if (padGet(pad)->mHold & 0x4) {
			mDist += 2.0f;
			if (mDist >= 2000.0f) {
				mDist = 2000.0f;
			}
		}
		if (padGet(pad)->mHold & 0x2) {
			((CamView*)mView)->mRoll += 0x80;
		} else if (padGet(pad)->mHold & 0x1) {
			((CamView*)mView)->mRoll -= 0x80;
		}
	}
	if ((padGet(pad)->mHold & 0x40) && (padGet(pad)->mHold & 0x20) && (padGet(pad)->mPressed & 0x200)) {
		init();
	}
	apply();
}

// 0x80015C68
void TCamEditor::vfn_24()
{
}

// 0x80015C6C
void TCamEditor::nextSpeed()
{
	mSpeed++;
	if (mSpeed >= 4) {
		mSpeed = 0;
	}
}
