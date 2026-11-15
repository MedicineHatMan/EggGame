#ifndef _TMODETITLE_H
#define _TMODETITLE_H

#include "stage/Stage.h"

class TCamPath;

class TObjTitleHelios;
class TAeObjTitle;

/**
 * The title screen mode, k_title.prd on the "title" stage. It swaps between the title itself and the attract
 * movie. The name is ours. Vtable 0x80252598.
 *
 * @size 0x2C
 */
class TModeTitle : public TMode {
public:
	TModeTitle();
	static TModeTitle* create(TModeAllocator* allocator);

	virtual ~TModeTitle();
	virtual void loadFiles();
	virtual void freeFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void nextScene();      // 0x80014B38
	void createModel();    // 0x80014B68
	void skipIntro();      // 0x80014BD4
	void requestSwitch();  // 0x80014EC4
	void switchPart();     // 0x80014ED4
	int startTitle();      // 0x80014F34
	int endTitle();        // 0x80015058
	void updateTitle();    // 0x8001512C
	int startMovie();      // 0x800152A0
	int endMovie();        // 0x80015328
	void updateMovie();    // 0x80015358

	int mTimer;                // _0C counts down from 1800 on the title up after Start
	TStage* mStage;            // _10
	TCamPath* mModel;          // _14 the opening camera
	TObjTitleHelios* mHelios;  // _18
	TAeObjTitle* mLogo;        // _1C
	s8 mState;                 // _20 0 waiting 1 running 2 Start pressed
	int mInMovie;              // _24
	u32 mFlags;                // _28 1 = switch next frame 2 = Start skipped the movie
};

extern s8 gTitleScene; // 0x802C4C58 which of the five title scenes plays next

extern "C" {
extern u32 sModeTitleClearColor;
}

#endif
