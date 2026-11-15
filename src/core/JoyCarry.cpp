#include "core/JoyCarry.h"
#include "Dolphin/OS/OSSerial.h"
#include "core/SdkCalls.h"

#define JOY_TYPE_GC  0x40
#define JOY_TYPE_GBA 0x40000

extern "C" {
s32 GBAJoyBootAsync(s32 chan, s32 paletteColor, s32 paletteSpeed, void* program, u32 length, u8* status,
                    void (*callback)(s32 chan, s32 result));
}

static JoyCarry* sJoyCarry;

static void bootDone(s32 chan, s32 result);

// 0x801AEC8C
JoyCarry* JoyCarry::get()
{
	if (sJoyCarry == 0) {
		sJoyCarry = new JoyCarry;
	}
	return sJoyCarry;
}

// 0x801AECD4
JoyCarry::JoyCarry()
{
	mState   = JOYCARRY_NONE;
	mType    = JOY_TYPE_GC;
	mProgram = 0;
	mSize    = 0;
	setState(JOYCARRY_WAIT);
}

// 0x801AED24
// 1 while sending 0 once sent 2 when nothing was sent
int JoyCarry::getResult()
{
	switch (mState) {
	case JOYCARRY_BOOT:
		return 1;
	case JOYCARRY_WAIT:
	case JOYCARRY_FAIL:
		return 2;
	default:
		return 0;
	}
}

// 0x801AED68
void JoyCarry::update()
{
	run(mState, JOYCARRY_TICK);
}

// 0x801AED94
int JoyCarry::boot(void* program, u32 size)
{
	if (mState != JOYCARRY_IDLE && mState != JOYCARRY_FAIL) {
		return 0;
	}
	mProgram = program;
	mSize    = size;
	mDone    = 0;
	mRetries = 300;
	if (GBAJoyBootAsync(3, 0, 1, mProgram, mSize, &mStatus, bootDone) != 0) {
		return 0;
	}
	mPercent = 0;
	setState(JOYCARRY_BOOT);
	return 1;
}

// 0x801AEE40
int JoyCarry::getPercent(u8* percent)
{
	if (mState != JOYCARRY_BOOT) {
		return 0;
	}
	*percent = mPercent;
	return 1;
}

// 0x801AEE64
void JoyCarry::run(int state, int event)
{
	switch (state) {
	case JOYCARRY_WAIT:
		stateWait(event);
		break;
	case JOYCARRY_IDLE:
		break;
	case JOYCARRY_BOOT:
		stateBoot(event);
		break;
	}
}

// 0x801AEEB8
void JoyCarry::setState(int state)
{
	if (mState != JOYCARRY_NONE) {
		run(mState, JOYCARRY_LEAVE);
	}
	if (state != JOYCARRY_NONE) {
		run(state, JOYCARRY_ENTER);
	}
	mState = state;
}

// 0x801AEF18
static void bootDone(s32 chan, s32 result)
{
	sJoyCarry->mDone   = 1;
	sJoyCarry->mResult = result;
}

// 0x801AEF30
void JoyCarry::stateWait(int event)
{
	if (event != JOYCARRY_TICK) {
		return;
	}
	int type = SIProbe(3);
	if (type == SI_ERROR_BUSY || type == SI_ERROR_NO_RESPONSE || type == JOY_TYPE_GC) {
		return;
	}
	if (type == JOY_TYPE_GBA) {
		mType = type;
		setState(JOYCARRY_IDLE);
	}
}

// 0x801AEFA0
void JoyCarry::stateBoot(int event)
{
	if (event != JOYCARRY_TICK) {
		return;
	}
	if (mRetries != 0) {
		mRetries--;
	}
	GBAGetProcessStatus(3, &mPercent);
	if (mDone) {
		if (mResult == 1 || mResult == 3) {
			if (mRetries != 0) {
				mDone = 0;
				GBAJoyBootAsync(3, 0, 1, mProgram, mSize, &mStatus, bootDone);
			} else {
				setState(JOYCARRY_FAIL);
			}
		} else {
			setState(JOYCARRY_IDLE);
		}
	}
}
