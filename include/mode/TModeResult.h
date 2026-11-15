#ifndef _TMODERESULT_H
#define _TMODERESULT_H

#include "stage/Stage.h"

class TSimpleText;
class TAeObjMsgWindow;

/**
 * The game mode that saves after a stage: the memory card checks and the save prompt, run from k_result.prd. It
 * keeps its own copy of the main menu's card pages (MainMenu.cpp): the same message ids from getStageSelectWarning,
 * the same message box and the same phases (0 enter, 1 update, 2 draw, 3 exit). All names here are ours.
 *
 * _08 is what the mode switcher at 0x80182D04 waits on. A page sets it to 0 once the save is done or skipped.
 *
 * @size 0x48
 */
class TModeResult : public TMode {
public:
	TModeResult();
	static TModeResult* create(TModeAllocator* allocator);

	virtual ~TModeResult() { }
	virtual void loadFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void buildText();
	void advance();
	int pressedA();
	int pressedB();
	int anyButton();
	int repeated(u32 mask);
	void closeBox();
	void closeChoice();
	void openChoice(void* message);
	void openMessage(void* message);
	void openNotice(void* message);
	void openTimed(void* message);
	int runBox();
	int choseYes();
	void setState(int state);
	int checkCard();
	void runState(int state, int phase);
	void showOptions();
	void hideOptions();

	int mState;               // _0C
	int mBackState;           // _10 the page a message returns to
	int mNextState;           // _14 setState writes this advance applies it after the current page's update
	int mBoxKind;             // _18 how runBox treats the box 1 yes/no 2 message 3 skippable 4 timed
	int mChoice;              // _1C 0 yes 1 no
	int mBoxTimer;            // _20
	int _24;                  // _24
	int mCard;                // _28 the card state from cardGetState
	int mPrevCard;            // _2C
	int mFrame;               // _30
	u32 mPressedA;            // _34 pad bit 0x100 A latched for this frame
	TSimpleText* mOption[2];  // _38 the two answers beside the yes/no window messages 0x27 and 0x28
	TSimpleText* mBox;        // _40
	TAeObjMsgWindow* mWindow; // _44 the yes/no window
};

extern "C" {
extern u32 sModeResultClearColor;
}

#endif
