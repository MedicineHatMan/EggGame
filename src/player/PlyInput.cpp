#include "player/PlyState.h"
#include "sdk/Dolphin/pad.h"
#include "core/Pad.h"
#include "core/MathUtil.h"
#include "player/Player.h"

struct PlyPad {
	u32 mHold;      // _00
	u8 _04[4];
	u32 mTrigger;   // _08
	u32 mRelease;   // _0C
	u8 _10[0x14];
	int mStickAngle; // _24
	f32 mStickMag;  // _28
};

struct PlyButtonMap {
	u32 mPad;
	u32 mButton;
};

extern "C" const PlyButtonMap sPlyButtonMap[7] = {
	{ PAD_BUTTON_A, PLY_BTN_A },
	{ PAD_BUTTON_B, PLY_BTN_B },
	{ PAD_TRIGGER_R, PLY_BTN_R },
	{ PAD_TRIGGER_R, PLY_BTN_R2 },
	{ PAD_BUTTON_X, PLY_BTN_X },
	{ PAD_BUTTON_B, PLY_BTN_B2 },
	{ PAD_BUTTON_Y, PLY_BTN_Y },
};

PlyInput::PlyInput()
{
	clear();
}

void PlyInput::clear()
{
	mStick    = 0.0f;
	mAngle    = 0;
	mPressed  = 0;
	mReleased = 0;
	mHeld     = 0;
	mNotHeld  = -1;
}

// Turns the raw pad into what states use a stick direction relative to the camera and the game's own button bits
// 0x800ADA30
void PlyInput::read(int pad, f32* camera)
{
	Vec3f forward;

	PlyPad* state = (PlyPad*)padGet((u8)pad);
	f32 stick = state->mStickMag;
	stick     = stick * stick * stick;
	if (stick < 0.1f) {
		stick = 0.0f;
	}
	mStick = stick;

	mtxMultVecSR(camera, &gAxisZ, &forward);
	int angle = state->mStickAngle + vecAngleXZ(&forward);
	mAngle = angle - 0x4000;

	mPressed  = 0;
	mReleased = 0;
	mHeld     = 0;
	for (int i = 0; i < 7; i++) {
		if (state->mTrigger & sPlyButtonMap[i].mPad) {
			mPressed |= sPlyButtonMap[i].mButton;
		}
		if (state->mRelease & sPlyButtonMap[i].mPad) {
			mReleased |= sPlyButtonMap[i].mButton;
		}
		if (state->mHold & sPlyButtonMap[i].mPad) {
			mHeld |= sPlyButtonMap[i].mButton;
		}
	}
	mNotHeld = ~mHeld;
}

// 0x800ADB4C
// Reads only whether a confirm button was pressed as 0x80
void PlyInput::readConfirm(int pad)
{
	mStick    = 0.0f;
	mReleased = 0;
	mHeld     = 0;
	if ((((PlyPad*)padGet((u8)pad))->mTrigger & 0xF00300) != 0) {
		mPressed = 0x80;
	} else {
		mPressed = 0;
	}
	mNotHeld = ~mHeld;
}
