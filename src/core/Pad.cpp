#include "stage/Stage.h"
#include "io/Resource.h"
#include "core/Pad.h"
#include "egg/EggCreature.h"
#include "core/Vibration.h"
#include "Dolphin/pad.h"
#include "stl/math.h"
#include "core/Mem.h"

const PadState sPadNoState = { 0x0, 0xFFFFFFFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0.0f, 0, 0.0f, -1, -1 };
struct VibTable;

typedef char PadState_size_check[(sizeof(PadState) == 0x38) ? 1 : -1];

extern "C" {
u32 sPadChanBits[4] = {
	0x80000000,
	0x40000000,
	0x20000000,
	0x10000000,
}; // One bit per controller

struct VibNode;
}

// 0x8001248C
void padClampStick(s16* x, s16* y)
{
	if (*x > 100) {
		*x = 100;
	} else if (*x < -100) {
		*x = -100;
	}
	if (*y > 100) {
		*y = 100;
	} else if (*y < -100) {
		*y = -100;
	}
	if (*x > -16 && *x < 16) {
		*x = 0;
	}
	if (*y > -16 && *y < 16) {
		*y = 0;
	}
}

// 0x80012518
f32 padStickLength(f32 x, f32 y)
{
	f32 sq = x * x + y * y;
	if (sq > 0.0f) {
		f64 guess         = __frsqrte(sq);
		guess             = 0.5 * guess * (3.0 - guess * guess * sq);
		guess             = 0.5 * guess * (3.0 - guess * guess * sq);
		guess             = 0.5 * guess * (3.0 - guess * guess * sq);
		volatile f32 root = (f32)(sq * guess);
		sq                = root;
	}
	f32 len = sq;
	if (len != 0.0f) {
		len = len / 100.0f;
	}
	if (len > 1.0f) {
		len = 1.0f;
	}
	return len;
}

// 0x800125AC
int padStickAngle(f32 x, f32 y)
{
	f32 angle = atan2(y, x);
	return (int)(10430.38043493439 * angle) & 0xFFFF;
}

// 0x800125F4
void padInit()
{
	PadSystem* pad      = &sPadSys;
	pad->mRepeatDelay   = 30;
	pad->mRepeatRate    = 10;
	pad->mNone          = sPadNoState;
	pad->mResetInterval = 1;
	pad->mResetTimer    = 1;
	for (int i = 0; i < 4; i++) {
		pad->mPads[i].mRepeatTimer = 0;
		pad->mPads[i].mState       = sPadNoState;
	}
	pad->mVibWork = memAlloc(0x800);
	vibInit((VibNode*)pad->mVibWork, 0x40);
	vibSetMax(0x10);
	loadArc("VIBRegular.vbl", &sPadArc);
	if (sPadArc != NULL) {
		vibSetTable(0, (VibTable*)sPadArc->mData);
	}
	padSetVibEnabled(1);
}

// 0x80012788
s16 padScaleStick(int value)
{
	return (value << 7) / 56;
}

// 0x8001279C
// Turns the left stick into direction bits
void padStickToDirs(PadState* state)
{
	if (state->mStickY > 48) {
		state->mHold |= 0x100000;
	} else if (state->mStickY < -48) {
		state->mHold |= 0x200000;
	}
	if (state->mStickX > 48) {
		state->mHold |= 0x800000;
	} else if (state->mStickX < -48) {
		state->mHold |= 0x400000;
	}
}

// 0x80012800
void padUpdate()
{
	PadSystem* sys = &sPadSys;
	PADStatus st[4];
	PADRead(st);
	PADClampCircle(st);
	PADStatus* in;
	int i;
	for (i = 0; i < 4; i++) {
		PadEntry* ent = &sys->mPads[i];
		PadState* pad = &sys->mPads[i].mState;
		in            = &st[i];
		pad->mPrev    = pad->mHold;
		if (in->err == 0) {
			pad->mHold     = in->button;
			pad->mHoldInv  = ~in->button;
			pad->mStickX   = padScaleStick(in->stickX);
			pad->mStickY   = padScaleStick(in->stickY);
			pad->mSubX     = in->substickX;
			pad->mSubY     = in->substickY;
			pad->mTriggerL = in->triggerLeft;
			pad->mTriggerR = in->triggerRight;
			padClampStick(&pad->mStickX, &pad->mStickY);
			padClampStick(&pad->mSubX, &pad->mSubY);
			padStickToDirs(pad);
			if (pad->mLastErr != 0) {
				pad->mPressed  = 0;
				pad->mReleased = 0;
			} else {
				pad->mPressed  = pad->mHold & (pad->mHold ^ pad->mPrev);
				pad->mReleased = pad->mPrev & (pad->mHold ^ pad->mPrev);
			}
			pad->mStatus  = 0;
			pad->mLastErr = in->err;
		} else {
			pad->mHold     = 0;
			pad->mHoldInv  = 0xFFFFFFFF;
			pad->mPressed  = 0;
			pad->mReleased = 0;
			pad->mSubY     = 0;
			pad->mSubX     = 0;
			pad->mStickY   = 0;
			pad->mStickX   = 0;
			pad->mTriggerR = 0;
			pad->mTriggerL = 0;
			if (in->err == -1) {
				pad->mStatus = -1;
			} else {
				pad->mStatus = -2;
			}
			pad->mLastErr = in->err;
		}
		if ((pad->mPrev ^ pad->mHold) != 0) {
			pad->mRepeat      = pad->mPressed;
			ent->mRepeatTimer = sys->mRepeatDelay;
		} else if (--ent->mRepeatTimer != 0) {
			pad->mRepeat = 0;
		} else {
			pad->mRepeat      = pad->mHold;
			ent->mRepeatTimer = sys->mRepeatRate;
		}
		pad->mLength    = padStickLength(pad->mStickX, pad->mStickY);
		pad->mAngle     = padStickAngle(pad->mStickX, pad->mStickY);
		pad->mSubLength = padStickLength(pad->mSubX, pad->mSubY);
		pad->mSubAngle  = padStickAngle(pad->mSubX, pad->mSubY);
	}
	if (sys->mResetTimer != 0 && --sys->mResetTimer == 0) {
		sys->mResetTimer = sys->mResetInterval;
		u32 mask         = 0;
		for (int i = 0; i < 4; i++) {
			if (st[i].err == -1) {
				mask |= sPadChanBits[i];
			}
		}
		if (mask != 0) {
			PADReset(mask);
		}
	}
}

// 0x80012B6C
PadState* padGet(u8 no)
{
	if (no < 4) {
		return &sPadSys.mPads[no].mState;
	}
	return &sPadSys.mNone;
}

// 0x80012B98
// True once the reset combination has been held for 30 frames
int padResetHeld()
{
	if ((padGet(0)->mHold & 0x1600) != 0x1600) {
		sPadResetHeld = 0;
		return 0;
	}
	if (sPadResetHeld != 30) {
		sPadResetHeld++;
		return 0;
	}
	return 1;
}

// 0x80012BFC
void padRecalibrateAll()
{
	PADRecalibrate(0xF0000000);
}

// 0x80012C20
int padVibStart(u8 a, int b, u8 c, u32 d, u32 e)
{
	if (sPadEnabled != 0) {
		return (int)vibPlay(a, (void*)b, c, d & 0xFFFF, e & 0xFFFF);
	}
	return 0;
}

// 0x80012C5C
void padSetVibEnabled(int on)
{
	sPadEnabled = on;
}

// 0x80012C64
int padGetVibEnabled()
{
	return sPadEnabled;
}
