#include "core/Vibration.h"
#include "Dolphin/pad.h"

VibSystem gVib; // 0x80282730

// 0x8011EEE4
// Chains the nodes into the free list and clears the pads and tables
void vibInit(VibNode* nodes, int count)
{
	VibSystem* vib = &gVib;
	u32 i;

	vib->mFree      = nodes;
	vib->mNodes     = nodes;
	vib->mNodeCount = count;
	for (i = 0; i < count - 1; i++) {
		nodes[i].mNext = &nodes[i + 1];
	}
	nodes[count - 1].mNext = 0;
	for (i = 0; i < 4; i++) {
		vib->mChannels[i].mHead  = 0;
		vib->mChannels[i].mCount = 0;
	}
	for (i = 0; i < 4; i++) {
		vib->mTables[i] = 0;
	}
	vib->mMax = 0x10;
}

// 0x8011EF88
void vibSetTable(int slot, VibTable* table)
{
	gVib.mTables[(u8)slot] = table;
}

// 0x8011EFA0
void vibSetMax(int max)
{
	gVib.mMax = max;
}

// 0x8011EFB0
// Unlinks node and gives it back to the free list
void vibRemove(VibChannel* channel, VibNode* node)
{
	VibSystem* vib = &gVib;
	if (node->mPrev == node) {
		channel->mHead = 0;
	} else if (channel->mHead == node) {
		channel->mHead = node->mNext;
		if (node->mNext != 0) {
			node->mNext->mPrev = node->mPrev;
		}
	} else {
		node->mPrev->mNext = node->mNext;
		if (node->mNext != 0) {
			node->mNext->mPrev = node->mPrev;
		} else {
			channel->mHead->mPrev = node->mPrev;
		}
	}
	channel->mCount--;
	node->mNext = vib->mFree;
	vib->mFree  = node;
}

// 0x8011F044
// Runs the script up to the next wait
int vibStep(VibNode* node, u8* motor)
{
	u32 op;
	const u8* pc;
	u16 wait;

	if (node->mFlags & 1) {
		return 0;
	}
	if (node->mWait != 0 && --node->mWait == 0) {
		pc   = node->mCode + node->mPc;
		wait = 0;
		do {
			op = *pc++;
			if (op == 0xFF) {
				if ((node->mFlags & 2) == 0) {
					*motor = 2;
				}
				return 1;
			}
			if (op < 0x80) {
				wait = op & 0x1F;
				if (op & 0x20) {
					wait = ((u16)(op & 0x1F) << 8) + *pc++;
				}
			} else {
				switch (op) {
				case 0x80:
					node->mMotor = 1;
					break;
				case 0x81:
					node->mMotor = 0;
					break;
				case 0x82:
					node->mMotor = 2;
					break;
				case 0xF0:
					node->mLoop   = *pc++;
					node->mLoopPc = pc - node->mCode;
					break;
				case 0xF1:
					if (--node->mLoop != 0) {
						pc = node->mCode + node->mLoopPc;
					}
					break;
				case 0xF2:
					node->mMarkPc = pc - node->mCode;
					break;
				case 0xF3:
					pc = node->mCode + node->mMarkPc;
					break;
				}
			}
		} while (wait == 0);
		node->mPc   = pc - node->mCode;
		node->mWait = wait;
	}
	if ((node->mFlags & 2) == 0) {
		*motor = node->mMotor;
	}
	return 0;
}

// 0x8011F1DC
// Once a frame steps every pattern and sends a pad's motor command when it changes
void vibUpdate()
{
	VibChannel* channel;
	int chan;
	VibNode* node;
	VibNode* next;

	channel = gVib.mChannels;
	for (chan = 0; chan < 4; chan++, channel++) {
		channel->mMotor = 2;
		for (node = channel->mHead; node != 0; node = next) {
			next = node->mNext;
			if (vibStep(node, &channel->mMotor)) {
				vibRemove(channel, node);
			}
		}
		if (channel->mMotor != channel->mPrevMotor) {
			switch (channel->mMotor) {
			case 1:
				PADControlMotor(chan, 1);
				break;
			case 0:
				PADControlMotor(chan, 0);
				break;
			case 2:
				PADControlMotor(chan, 2);
				break;
			}
		}
		channel->mPrevMotor = channel->mMotor;
	}
}

// 0x8011F2C8
void vibResume(int chan)
{
	VibNode* node;

	for (node = gVib.mChannels[(u8)chan].mHead; node != 0; node = node->mNext) {
		node->mFlags &= ~1;
	}
}

// 0x8011F300
void vibResumeAll()
{
	int chan;

	for (chan = 0; chan < 4; chan++) {
		vibResume((u8)chan);
	}
}

// 0x8011F33C
void vibResumeOwner(int chan, void* owner)
{
	VibNode* node;

	for (node = gVib.mChannels[(u8)chan].mHead; node != 0; node = node->mNext) {
		if (node->mOwner == owner) {
			node->mFlags &= ~1;
		}
	}
}

// 0x8011F380
void vibStop(int chan)
{
	VibNode* node = gVib.mChannels[(u8)chan].mHead;
	VibNode* next;

	while (node != 0) {
		next = node->mNext;
		vibRemove(&gVib.mChannels[(u8)chan], node);
		node = next;
	}
}

// 0x8011F3DC
void vibStopAll()
{
	int chan;

	for (chan = 0; chan < 4; chan++) {
		vibStop((u8)chan);
	}
}

// 0x8011F418
void vibStopOwner(int chan, void* owner)
{
	VibNode* node = gVib.mChannels[(u8)chan].mHead;
	VibNode* next;

	while (node != 0) {
		next = node->mNext;
		if (node->mOwner == owner) {
			vibRemove(&gVib.mChannels[(u8)chan], node);
		}
		node = next;
	}
}

// 0x8011F484
void* vibPlay(u8 chan, void* owner, u8 priority, u32 table, u32 id)
{
	return vibPlayEx(chan, owner, 0, priority, table, id);
}

// 0x8011F4BC
// Takes the head the lowest priority when it is below priority
VibNode* vibTakeLowest(VibChannel* channel, u8 priority)
{
	VibNode* node = channel->mHead;

	if (node->mPriority < (u8)priority) {
		channel->mHead     = node->mNext;
		node->mPrev->mNext = 0;
		if (node->mNext != 0) {
			node->mNext->mPrev = node->mPrev;
		}
		return node;
	}
	return 0;
}

// 0x8011F508
// Starts pattern id of a table on a pad after the patterns of no higher priority
void* vibPlayEx(u8 chan, void* owner, void* arg, u8 priority, u32 table, u32 id)
{
	VibChannel* channel;
	VibNode* node;
	VibScript* script;
	VibNode* head;
	VibNode* it;
	VibTable* tab;
	u16 index;
	VibSystem* vib = &gVib;

	tab = gVib.mTables[(u16)table];
	if (tab == 0) {
		return 0;
	}
	index = id - tab->mBase;
	if (index >= tab->mCount) {
		return 0;
	}
	channel = &vib->mChannels[chan];
	if (channel->mCount == gVib.mMax) {
		node = vibTakeLowest(channel, priority);
		if (node == 0) {
			return 0;
		}
	} else {
		node = gVib.mFree;
		if (node != 0) {
			gVib.mFree = node->mNext;
			channel->mCount++;
		} else {
			return 0;
		}
	}
	script          = tab->mEntries[index].mScript;
	node->mOwner    = owner;
	node->mArg      = arg;
	node->mMotor    = 2;
	node->mFlags    = 0;
	node->mPriority = priority;
	node->mLoop     = 0;
	node->mWait     = script->mCode != 0 ? 1 : 0;
	node->mPc       = 0;
	node->mCode     = script->mCode;
	head            = channel->mHead;
	it              = head;
	if (head != 0) {
		while (it != 0 && it->mPriority <= priority) {
			it = it->mNext;
		}
		if (it != 0) {
			node->mPrev = it->mPrev;
			node->mNext = it;
			if (channel->mHead == it) {
				channel->mHead = node;
			} else {
				it->mPrev->mNext = node;
			}
			it->mPrev = node;
		} else {
			node->mPrev        = head->mPrev;
			node->mNext        = 0;
			head->mPrev->mNext = node;
			head->mPrev        = node;
		}
	} else {
		node->mPrev    = node;
		channel->mHead = node;
		node->mNext    = 0;
	}
	return node->mOwner;
}

// 0x8011F6C8
void vibPause(int chan)
{
	VibNode* node;

	for (node = gVib.mChannels[(u8)chan].mHead; node != 0; node = node->mNext) {
		node->mFlags |= 1;
	}
}

// 0x8011F700
void vibPauseAll()
{
	int chan;

	for (chan = 0; chan < 4; chan++) {
		vibPause((u8)chan);
	}
}

// 0x8011F73C
void vibPauseOwner(int chan, void* owner)
{
	VibNode* node;

	for (node = gVib.mChannels[(u8)chan].mHead; node != 0; node = node->mNext) {
		if (node->mOwner == owner) {
			node->mFlags |= 1;
		}
	}
}
