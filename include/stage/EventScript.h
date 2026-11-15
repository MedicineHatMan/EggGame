#ifndef _EVENTSCRIPT_H
#define _EVENTSCRIPT_H

#include "core/types.h"

struct EventCmd {
	const char* mName;        // _00
	int mWait;                // _04 1 = runs in an event and waits
	int (*mFunc)(void* args); // _08
};

/** One line of a script */
struct EventLine {
	const char* mName; // _00
	s16 mCmd;          // _04 index in the command table -1 if unknown
	u16 mBits;         // _06 bit 0 = wait here
	void* mArgs;       // _08 the command's own argument block
};

/** A script */
struct EventScript {
	int mKind;          // _00 0 = on request 1 = on switch
	int mId;            // _04
	u32 mBits;          // _08 bit 0 = queued
	int mLineCount;     // _0C
	EventLine** mLines; // _10
};

/** The scripts of an archive */
struct EventScriptSet {
	int mCount;             // _00
	EventScript** mScripts; // _04
};

/**
 * Runs one script at a time. The one instance is made by eventScriptInit.
 *
 * @size 0x24
 */
class EventScriptMgr {
public:
	enum {
		STATE_IDLE    = 0,
		STATE_WAITING = 1, // for an event to start
		STATE_EVENT   = 2, // inside an event
	};
	enum {
		BIT_SIGNAL = 0x1, // eventScriptSignal
		BIT_FAILED = 0x2, // the last command returned 0
		BIT_EVENT  = 0x4, // an event is running
		BIT_DONE   = 0x8, // eventScriptDone while a waiting command runs
	};

	EventScriptMgr();  // 0x8012C148
	~EventScriptMgr(); // 0x8012C178

	void clear();                        // 0x8012C1B4
	void reset();                        // 0x8012C1E8
	EventScript* find(int kind, int id); // 0x8012C22C
	int start(int kind, int id);         // 0x8012C27C
	EventLine* nextLine();               // 0x8012C320
	int run();                           // 0x8012C41C returns 1 once the script has ended
	int exec();                          // 0x8012C538
	void end();                          // 0x8012C5E4
	int request(int id, u16 eventId);    // 0x8012C648
	void clearQueue();                   // 0x8012C6BC
	void trigger(int id);                // 0x8012C704
	int runQueued();                     // 0x8012C760
	int resume(int arg);                 // 0x8012C874
	void signal();                       // 0x8012C8E4
	void done();                         // 0x8012C8F4
	void update();                       // 0x8012C904

	int mState;           // _00
	int mTimer;           // _04
	u16 mEventId;         // _08
	u32 mBits;            // _0C
	int mQueued;          // _10 kind 1 scripts waiting
	EventScript* mScript; // _14
	int mLine;            // _18
	int mKind;            // _1C of the running script
	int mPhase;           // _20 how far nextLine is through the waiting lines
};

void eventScriptInit();                      // 0x8012C9DC
void eventScriptExit();                      // 0x8012CA28
void eventScriptReset();                     // 0x8012CA80
void eventScriptLoad(const char* name);      // 0x8012CAB0
int eventScriptRequest(int id);              // 0x8012CB9C
int eventScriptRequest(int id, u16 eventId); // 0x8012CBC4
void eventScriptSignal();                    // 0x8012CC08
void eventScriptDone();                      // 0x8012CC34
void eventScriptTrigger(int id);             // 0x8012CC60
void eventScriptUpdate();                    // 0x8012CC94
EventCmd* getEventCmds();                    // 0x8012CD20

// The argument blocks of the commands kept in EventCam.cpp
struct EventCamSetArgs { // "set" hand the set object mSetId an event camera
	int mSetId;          // _00
	int mArg;            // _04 passed on in the message
};
struct EventWaitArgs { // "wait"
	int mFrames;       // _00
};
struct EventMsgArgs { // "msg" show a message and wait for it to close
	u16 mId;          // _00
};

int eventCmdSet(EventCamSetArgs* args); // 0x8012D0B4 "set"
int eventCmdWait(EventWaitArgs* args);  // 0x8012D16C "wait"
int eventCmdWaitFake(void*);            // 0x8012D1C8 "wait_fake" takes no arguments
int eventCmdPtclTest(void*);            // 0x8012D1EC "ptcl_test" takes no arguments
int eventCmdMsg(EventMsgArgs* args);    // 0x8012D254 "msg"

extern "C" {
extern EventCmd gEventCmds[];
}

#endif
