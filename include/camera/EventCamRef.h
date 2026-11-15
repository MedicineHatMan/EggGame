#ifndef _EVENTCAMREF_H
#define _EVENTCAMREF_H

#include "core/types.h"
#include "object/ObjCommon.h"

struct EventCamMsg : public ObjMsg {
	int mId;  // _04
	int mArg; // _08
};

/**
 * A handle to an event camera, taken from the message that starts one. Objects that can bring in an event camera
 * hold one as a member. Releasing it hands the camera back to the event script. EventCam.cpp, 0x8012CF74.
 *
 * @size 0x8
 */
struct EventCamRef {
	EventCamRef();  // 0x8012CF74
	~EventCamRef(); // 0x8012CF84

	int isActive();        // 0x8012CFCC
	void set(ObjMsg* msg); // 0x8012CFE0 takes the camera an EventCamMsg names
	void release();        // 0x8012CFF4

	int mActive; // _00
	int mId;     // _04
};

void eventCamInit();       // 0x8012D03C
void eventCamExit();       // 0x8012D050
void eventCamReset();      // 0x8012D054
void eventCamFree(int id); // 0x8012D074 lets the script go on once the camera it started is given back

#endif
