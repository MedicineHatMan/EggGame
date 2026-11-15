#ifndef _VIBRATION_H
#define _VIBRATION_H

#include "core/types.h"

struct VibScript {
	const u8* mCode; // _00
};

/** A pattern table entry. @size 0x8 */
struct VibEntry {
	u32 _00;            // _00
	VibScript* mScript; // _04
};

/** A pattern table from a .vbl archive */
struct VibTable {
	u32 _00;            // _00
	int mBase;          // _04 the id of the first entry
	int mCount;         // _08
	VibEntry* mEntries; // _0C
};

/** A running pattern. @size 0x20 */
struct VibNode {
	VibNode* mNext;    // _00
	VibNode* mPrev;    // _04 the head's points at the tail
	void* mOwner;      // _08
	void* mArg;        // _0C
	u8 mMotor;         // _10 0 off 1 on 2 stop hard
	u8 mFlags;         // _11 1 paused 2 silent
	u8 mPriority;      // _12
	u8 mLoop;          // _13
	u16 mWait;         // _14
	u16 mPc;           // _16
	u16 mLoopPc;       // _18
	u16 mMarkPc;       // _1A
	const u8* mCode;   // _1C
};

/** One pad's patterns. @size 0xC */
struct VibChannel {
	VibNode* mHead; // _00
	int mCount;     // _04
	u8 mMotor;      // _08 this frame's
	u8 mPrevMotor;  // _09
};

/** @size 0x50 */
struct VibSystem {
	VibNode* mNodes;         // _00
	int mNodeCount;          // _04
	VibNode* mFree;          // _08
	VibChannel mChannels[4]; // _0C
	VibTable* mTables[4];    // _3C
	int mMax;                // _4C patterns per pad
};

extern "C" {
extern VibSystem gVib;

void vibInit(VibNode* nodes, int count);
void vibSetTable(int slot, VibTable* table);
void vibSetMax(int max);
void vibRemove(VibChannel* channel, VibNode* node);
int vibStep(VibNode* node, u8* motor);
void vibUpdate();
void vibResume(int chan);
void vibResumeAll();
void vibResumeOwner(int chan, void* owner);
void vibStop(int chan);
void vibStopAll();
void vibStopOwner(int chan, void* owner);
void* vibPlay(u8 chan, void* owner, u8 priority, u32 table, u32 id);
VibNode* vibTakeLowest(VibChannel* channel, u8 priority);
void* vibPlayEx(u8 chan, void* owner, void* arg, u8 priority, u32 table, u32 id);
void vibPause(int chan);
void vibPauseAll();
void vibPauseOwner(int chan, void* owner);
}

#endif
