#ifndef _TAEOBJSTAGETITLE_H
#define _TAEOBJSTAGETITLE_H

#include "ui/TAeLayer.h"
struct DrawTask;

class TSimpleText;

class TCamPath;

/**
 * Stage-name banner. The layer id comes from a table indexed by the current stage.
 * A draw task on list 0x3C calls render(), which draws the layer until mDone is set.
 * draw() waits a frame, starts an event that hides the stage's objects, plays the "hint" event camera once,
 * then steps the layer. When the camera or the layer is done, or A or Start is pressed, finish() ends it.
 *
 * @size 0x54
 */
class TAeObjStageTitle : public TAeObject {
public:
	TAeObjStageTitle(TObject* parent, void* data, void* assets);
	virtual ~TAeObjStageTitle();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	void finish(); // 0x800E9408

	TSimpleText* mMissionText; // _34 the mission's name under the banner
	TAeLayer* mLayer; // _38
	DrawTask* mTask;     // _3C
	int mWait;       // _40 starts at 1
	s32 mEvent;      // _44 what eventStartById returned 0 before
	TCamPath* mHintCam; // _48 "cam_hint" from the stage's event cameras
	s32 mHintTried;  // _4C the hint camera only gets one try
	s32 mDone;       // _50 set by finish() nothing more is drawn
};

extern "C" {
void initStageTitleSet();
int stageTitleLoad();
void stageTitleRelease();
void stageTitleReset();
void termStageTitleSet();
}

extern "C" {
int stageTitleLoadCmd(s32*);
TAeObjStageTitle* createStageTitle();
}

#endif
