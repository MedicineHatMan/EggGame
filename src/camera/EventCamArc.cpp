#include "core/types.h"
#include "camera/EventCamArc.h"
#include "io/ResManager.h"
#include "stl/string.h"
#include "camera/TCameraCtrl.h"

// 0x800EEAB8
extern "C" void eventCamArcInit()
{
	loadArc("ar_event_cam_common.arc", &sEventCamCommonArc);
	sEventCamCommon   = (EventCamList*)sEventCamCommonArc->mData;
	sEventCamStageArc = 0;
	sEventCamStage    = 0;
}

// 0x800EEAFC
extern "C" void eventCamArcExit()
{
	sEventCamStage  = 0;
	sEventCamCommon = 0;
	releaseResource(sEventCamStageArc);
	sEventCamStageArc = 0;
	releaseResource(sEventCamCommonArc);
	sEventCamCommonArc = 0;
}

// 0x800EEB44
extern "C" void eventCamArcLoad(const char* name)
{
	loadArc(name, &sEventCamStageArc);
	sEventCamStage = (EventCamList*)sEventCamStageArc->mData;
}

// 0x800EEB74
extern "C" TCamPath* eventCamArcPlay(f32 to, f32 from, int kind)
{
	return eventCamArcPlayCurrent(to, from, kind);
}

// 0x800EEB94
extern "C" TCamPath* eventCamArcPlayCurrent(f32 to, f32 from, int kind)
{
	return eventCamArcPlayFor(to, from, camGetDrawIndex(), kind);
}

// 0x800EEBE8
extern "C" TCamPath* eventCamArcPlayFor(f32 to, f32 from, int player, int kind)
{
	EventCamList* list;
	if (kind < 0x100) {
		list = sEventCamCommon;
	} else {
		list = sEventCamStage;
		kind -= 0x100;
	}
	if (list == 0) {
		return 0;
	}
	if (kind < 0 || kind >= list->mCount) {
		return 0;
	}
	return createCamPath(player, 3, list->mCams[kind], list->mMotions[kind], 0, from, to);
}

// 0x800EEC74
extern "C" TCamPath* eventCamArcPlayNamed(f32 to, f32 from, const char* name)
{
	char path[256];
	if (sEventCamStageArc == 0) {
		return 0;
	}
	strcpy(path, "cam_");
	strcat(path, name);
	void* cam = findArcExport(sEventCamStageArc, path);
	strcpy(path, "mtn_");
	strcat(path, name);
	void* motion = findArcExport(sEventCamStageArc, path);
	if (cam == 0 || motion == 0) {
		return 0;
	}
	return createCamPath(camGetDrawIndex(), 3, cam, motion, 0, from, to);
}
