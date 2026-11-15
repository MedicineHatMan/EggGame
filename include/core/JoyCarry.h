#ifndef _JOYCARRY_H
#define _JOYCARRY_H

#include "core/types.h"
#include "io/ResFile.h"
#include "stage/Stage.h"

enum JoyCarryState {
	JOYCARRY_WAIT = 0, // looking for a GBA
	JOYCARRY_IDLE = 1, // one is plugged in
	JOYCARRY_BOOT = 2, // sending
	JOYCARRY_FAIL = 3, // gave up after mRetries frames
	JOYCARRY_NONE = 4,
};

enum JoyCarryEvent {
	JOYCARRY_ENTER = 0,
	JOYCARRY_LEAVE = 1,
	JOYCARRY_TICK  = 2,
};

/**
 * @size 0x18
 */
class JoyCarry {
public:
	JoyCarry();

	static JoyCarry* get();

	int getResult();
	void update();
	int boot(void* program, u32 size);
	int getPercent(u8* percent);
	void run(int state, int event);
	void setState(int state);
	void stateWait(int event);
	void stateBoot(int event);

	s8 mState;      // _00 JoyCarryState
	s8 mDone;       // _01 set by the JoyBoot callback
	u8 mStatus;     // _02 JoyBoot's status byte
	u8 mPercent;    // _03
	u32 mType;      // _04 what SI says is on the port
	int mResult;    // _08 from the JoyBoot callback
	int mRetries;   // _0C
	void* mProgram; // _10
	u32 mSize;      // _14
};

/**
 * The debug mode that sends one of five GBA programs, SCENE_JOY_CARRY. See TModeJoyCarry.cpp.
 * @size 0x24
 */
class TModeJoyCarry : public TMode {
public:
	TModeJoyCarry();
	static TModeJoyCarry* create(TModeAllocator* allocator);

	virtual ~TModeJoyCarry() { }
	virtual int init();
	virtual int shutdown();
	virtual void update();

	BinFile* mFiles[5]; // _0C
	s8 mState;          // _20 0 = picking 1 = sending 2 = failed 3 = sent
	s8 mCursor;         // _21
	s16 mTimer;         // _22
};

#endif
