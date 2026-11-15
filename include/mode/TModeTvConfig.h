#ifndef _TMODETVCONFIG_H
#define _TMODETVCONFIG_H

#include "stage/Stage.h"

class TSimpleText;
class TAeObjMsgWindow;

/**
 * The progressive scan question at boot: k_tv_config.prd, with its text from mes_tv_config.bin. Saying yes switches
 * the video mode and asks again after a hundred frames; saying no, or not answering in time, keeps interlaced.
 * The names are ours. Vtable 0x80261D08.
 *
 * @size 0x28
 */
class TModeTvConfig : public TMode {
public:
	TModeTvConfig();
	static TModeTvConfig* create(TModeAllocator* allocator);

	virtual ~TModeTvConfig();
	virtual void loadFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void loadText();
	void step();
	void setProgressive();
	void clearPrompt();
	void closeWindow();
	void closePrompt();
	void ask(void* text);
	void say(void* text);
	int answered();
	int closed();
	int pressedA();
	int pressedB();
	int pressedDir(u32 dir);
	void showTitles();
	void hideTitles();

	s8 mState;               // _0C 0 asking 1 waiting for the answer to close 2 trying progressive 3 done
	u16 mTimer;              // _0E
	TSimpleText* mTitle;     // _10
	TSimpleText* mSubtitle;  // _14
	TSimpleText* mText;      // _18
	int mPrompt;             // _1C 1 a yes/no question 2 a message
	int mChoice;             // _20 0 yes 1 no
	TAeObjMsgWindow* mWindow; // _24
};

#include "Dolphin/GX/GXTypes.h"
extern "C" {
extern GXRenderModeObj GXNtsc480Prog;
extern u32 sModeTvConfigClearColor;
}

#endif
