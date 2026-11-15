#ifndef _PAD_H
#define _PAD_H
struct TArc;
#include "Ninja/ninja.h"
struct PadEntry;
struct PadSystem;

#include "core/types.h"

// Pad input See Pad.cpp

/**
 * One controller as the game sees it, updated once a frame by padUpdate. The button words use the PAD bits: 0x100
 * is A, 0x200 is B, 0x1000 is Start.
 *
 * @size 0x38
 */
#ifndef PAD_BUTTON_START
#define PAD_BUTTON_START 0x1000 // as in the SDK's pad.h
#endif

struct PadState {
	u32 mHold;     // _00
	u32 mHoldInv;  // _04
	u32 mPressed;  // _08
	u32 mReleased; // _0C
	u32 mPrev;     // _10
	u32 mRepeat;   // _14
	u16 mTriggerR; // _18
	u16 mTriggerL; // _1A
	s16 mStickX;   // _1C
	s16 mStickY;   // _1E
	s16 mSubX;     // _20
	s16 mSubY;     // _22
	int mAngle;    // _24
	f32 mLength;   // _28
	int mSubAngle; // _2C
	f32 mSubLength; // _30
	s16 mStatus;   // _34
	s16 mLastErr;  // _36
};

extern "C" PadState* padGet(u8 no); // 0x80012B6C a blank state for a bad pad number

extern "C" {
s16 padScaleStick(int);
void padRecalibrateAll();
void padSetVibEnabled(int);
int padGetVibEnabled();
void padClampStick(s16*, s16*);
void padInit();
int padResetHeld();
int padStickAngle(f32, f32);
f32 padStickLength(f32, f32);
void padStickToDirs(PadState*);
void padUpdate();
}

extern "C" {
int padVibStart(u8 pad, int owner, u8 priority, u32 table, u32 id);
}

struct PadEntry {
	u32 _00;
	u32 mRepeatTimer; // _04
	PadState mState;  // _08
};

struct PadSystem {
	int mRepeatDelay;    // _00
	int mRepeatRate;     // _04
	PadEntry mPads[4];   // _08
	PadState mNone;      // _108 What padGet returns for a bad pad number
	u8 _140[0x30];
	int mResetTimer;     // _170
	int mResetInterval;  // _174
	void* mVibWork;      // _178
};

extern "C" {
extern PadSystem sPadSys;
extern const PadState sPadNoState;
extern TArc* sPadArc;
extern int sPadEnabled;
extern s16 sPadResetHeld;
}

#endif
