#ifndef _EVENTCAMARC_H
#define _EVENTCAMARC_H
struct TArc;
struct EventCamList;

#include "camera/TCamera.h"

// The event cameras EventCamArc.cpp A kind below 0x100 is in ar_event_cam_common.arc's list the rest in the stage's
// Each play makes a TCamPath with createCamPath passing to and from the other way round

extern "C" {
void eventCamArcInit();                 // 0x800EEAB8
void eventCamArcExit();                 // 0x800EEAFC
void eventCamArcLoad(const char* name); // 0x800EEB44 the stage's arc

TCamPath* eventCamArcPlay(f32 to, f32 from, int kind);                // 0x800EEB74 for the current player
TCamPath* eventCamArcPlayCurrent(f32 to, f32 from, int kind);         // 0x800EEB94
TCamPath* eventCamArcPlayFor(f32 to, f32 from, int player, int kind); // 0x800EEBE8
TCamPath* eventCamArcPlayNamed(f32 to, f32 from, const char* name);   // 0x800EEC74 "cam_<name>" and "mtn_<name>"
}

/** The table at the start of an event camera arc's data */
struct EventCamList {
	int mCount;      // _00
	void** mCams;    // _04
	void** mMotions; // _08
};

extern "C" {
extern TArc* sEventCamCommonArc;
extern TArc* sEventCamStageArc;
extern EventCamList* sEventCamCommon;
extern EventCamList* sEventCamStage;
}

#endif
