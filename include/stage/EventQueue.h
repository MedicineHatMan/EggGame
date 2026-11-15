#ifndef _EVENTQUEUE_H
#define _EVENTQUEUE_H

#include "core/types.h"

/**
 * @size 0x10
 */
struct EventEntry {
	EventEntry(void* arg, int (*start)(void*, int), int id); // 0x8018C76C
	void clear();                                            // 0x8018C784

	EventEntry* mNext;         // _00
	void* mArg;                // _04
	int (*mStart)(void*, int); // _08 starts it given mArg and mId
	int mId;                   // _0C 0x20 when cleared
};

/** The events waiting for TEventCtrl to start them, first in first out. @size 0xC */
struct EventQueue {
	EventQueue();  // 0x8018C7A0
	~EventQueue(); // 0x8018C7B4

	EventEntry* tail();                            // 0x8018C81C
	int push(void* arg, int (*start)(void*, int)); // 0x8018C84C returns the entry's number
	int pop(EventEntry* out);                      // 0x8018C8D0

	int mCount;        // _00
	EventEntry* mHead; // _04
	int mNextId;       // _08
};

#endif
