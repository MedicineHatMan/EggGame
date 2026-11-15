#include "core/types.h"
#include "camera/TCameraCtrl.h"

extern "C" {

// 0x80098734
void camKeyClear(CamPathKey* key)
{
	key->mValue   = 0;
	key->mPressed = 0;
	key->mHeld    = 0;
}

// 0x80098748
void camKeyUpdate(CamPathKey* key, int down, u16 value)
{
	key->mPressed = 0;
	if (down != 0) {
		if ((key->mHeld & 1) == 0) {
			key->mPressed |= 1;
		}
		key->mHeld |= 1;
	} else {
		key->mHeld &= ~1;
	}
	if ((key->mHeld & 2) == 0) {
		if (value > 90) {
			key->mHeld |= 2;
			key->mPressed |= 2;
		} else {
			key->mHeld &= ~2;
		}
	} else if (value < 80) {
		key->mHeld &= ~2;
	} else {
		key->mHeld |= 2;
	}
	key->mValue = value;
}

// 0x800987F0
void camSpanClear(CamPathSpan* span)
{
	span->mPressed = 0;
	span->mHeld    = 0;
	span->mRepeat  = 0;
	span->mRaw     = 0;
}

// 0x80098808
void camSpanUpdate(CamPathSpan* span, int value)
{
	int repeat = span->mRepeat;
	u32 bits   = 0;
	if (value <= -56) {
		if (repeat >= -1) {
			bits |= 1;
		}
		bits |= 2;
	} else if (value <= -15) {
		if (repeat > 0) {
			repeat = 0;
		}
		bits |= 4;
		repeat--;
	} else if (value < 15) {
		repeat = 0;
	} else if (value < 56) {
		if (repeat < 0) {
			repeat = 0;
		}
		bits |= 8;
		repeat++;
	} else {
		if (repeat <= 1) {
			bits |= 0x20;
		}
		bits |= 0x10;
	}
	span->mPressed = bits & ~span->mHeld;
	span->mHeld    = bits;
	span->mRepeat  = repeat;
	span->mRaw     = value;
	if (value >= 56) {
		span->mValue = 1.0f;
	} else if (value >= 15) {
		span->mValue = (f32)(value - 15) / 41.0f;
	} else if (value > -15) {
		span->mValue = 0.0f;
	} else if (value > -56) {
		span->mValue = (f32)(value + 15) / 41.0f;
	} else {
		span->mValue = -1.0f;
	}
}

// 0x80098950
void camPadInit(CamPath* pad)
{
	for (int i = 0; i < 2; i++) {
		camKeyClear(&pad->mKey[i]);
	}
	pad->mKeyIndex = 0;
	for (int i = 0; i < 2; i++) {
		camSpanClear(&pad->mSpan[i]);
	}
}

// 0x800989C4
void camPadUpdate(CamPath* pad, int downA, u16 valueA, int downB, u16 valueB, s16 x, s16 y)
{
	camKeyUpdate(&pad->mKey[0], downA, valueA);
	camKeyUpdate(&pad->mKey[1], downB, valueB);
	camSpanUpdate(&pad->mSpan[0], x);
	camSpanUpdate(&pad->mSpan[1], y);
}

// 0x80098A28
void camPadClear(CamPath* pad)
{
	for (int i = 0; i < 2; i++) {
		pad->mKey[i].mPressed = 0;
		pad->mKey[i].mValue   = 0;
	}
	for (int i = 0; i < 2; i++) {
		pad->mSpan[i].mPressed = 0;
		pad->mSpan[i].mRaw     = 0;
	}
}

// 0x80098A78
int camPadDown(CamPath* pad)
{
	return pad->mKey[pad->mKeyIndex].mPressed & 1;
}

// 0x80098A90
int camPadHardHeld(CamPath* pad)
{
	return pad->mKey[pad->mKeyIndex].mHeld & 2;
}

// 0x80098AA8
int camPadHardPressed(CamPath* pad)
{
	return pad->mKey[pad->mKeyIndex].mPressed & 2;
}

// 0x80098AC0
int camPadAhead(CamPath* pad)
{
	return pad->mSpan[1].mHeld & 0x18;
}

// 0x80098ACC
int camPadBack(CamPath* pad)
{
	return pad->mSpan[1].mHeld & 6;
}

// 0x80098AD8
int camPadRight(CamPath* pad)
{
	return pad->mSpan[0].mHeld & 0x18;
}

// 0x80098AE4
int camPadLeft(CamPath* pad)
{
	return pad->mSpan[0].mHeld & 6;
}

// 0x80098AF0
// Whether the stick is pushed any way
int camPadStickMoved(CamPath* pad)
{
	u8 moved = 0;
	if (camPadAhead(pad) != 0 || camPadLeft(pad) != 0 || camPadBack(pad) != 0 || camPadRight(pad) != 0) {
		moved = 1;
	}
	return moved;
}

// 0x80098B60
f32 camPadX(CamPath* pad)
{
	return pad->mSpan[0].mValue;
}

// 0x80098B68
f32 camPadY(CamPath* pad)
{
	return pad->mSpan[1].mValue;
}
}
