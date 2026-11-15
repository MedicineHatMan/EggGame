#ifndef _MAINMENU_H
#define _MAINMENU_H
struct BinFile;

#include "stage/Stage.h"

class TSimpleText;
class TMainMenuModel;
class TAeObjMsgWindow;
class TWhiteFadeInOut;
struct FileInfo;

// The front-end Mode id 4 in the switcher at 0x80181BC0
//
// @size 0xC8
class TModeMainMenu : public TMode {
public:
	TModeMainMenu();
	static TModeMainMenu* create(TModeAllocator* allocator);

	virtual ~TModeMainMenu();
	virtual void loadFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void buildText();
	void advance();
	void runState(int state, int phase);
	void setState(int state);
	int pressedA();
	int pressedB();
	int anyButton();
	int repeated(u32 mask); // the mask's d-pad bit or stick bit either one on the pad's repeat

	// Page bodies 0x801708D4-0x80172350 Each update returns 1 once its page is done
	void resetRoot();
	int updateRoot();
	void chooseFiles();
	void readFileInfos();
	void readFileInfo(FileInfo* info, s8 slot);
	void resetFile();
	int updateFile();
	void resetSound();
	int updateSound();

	// The message box mBoxKind 1 yes/no 2 message 3 notice 4 timed 5 file choice 6 card choice 7 sound mode 8 vibration
	void closeBox();
	void closeYesNoWindow();
	void closeChoiceWindow();
	void openYesNo(void* message);
	void openMessage(void* message, s8 kind);
	void openNotice(void* message);
	void openTimed(void* message);
	void openFileChoice(s8 kind);
	void openCardChoice(s8 kind);
	void openSoundChoice();
	void openVibration();
	int runBox();
	int choseYes();

	// The white fade The five fadeOut copies are identical free mFade start a new one count 60 frames
	void freeFade();
	void fadeOutBack();
	void fadeOutRoot();
	void fadeOutFileBack();
	void fadeOutFile0();
	void fadeOutFile1();
	int fadeDone();
	int pickState(); // maps mCursor to a state

	// Text pages 0x80173B88-0x801748D8 Each show clears the text's hidden bit 0x10 restarts it on its string and places it
	void hideAll();
	void showCards(int slot);
	void hideCards();
	void showCaptions(int slot);
	void hideCaptions();
	void showTitle(int msg);
	void hideTitle();
	void showFileHelp(int slot);
	void hideHelp();
	void showTabs();
	void hideTabs();
	void showLines();
	void hideLines();
	void showSoundHelp(int mode);
	void showLinePair();
	void hideLinePair();
	void showCaptionRow();
	void hideCaptionRow();
	void showCaptionPair();
	void hideCaptionPair();
	void showIcons();
	void hideIcons();

	// Set by the switcher after create before the mode starts
	u8 mOpened; // _0C
	u8 _0D[3];
	int mFlow;      // _10 0 the switcher reads mSlot
	int mPage;      // _14
	int mState;     // _18
	int mBackState; // _1C the page a confirm returns to
	int mNextState; // _20 setState writes this advance applies it after the current page's update
	int mSelect;    // _24 the root page's choice
	int mCopyTo;    // _28 the copy page's target file
	int mFile;      // _2C the root page's file
	int _30;        // _30
	int _34;        // _34
	int mFileStep;  // _38
	int mSlot;      // _3C the file page's choice
	int mSoundStep; // _40
	int mSoundMode; // _44 0 the sound mode 1 vibration
	int _48;        // _48
	int _4C;        // _4C
	int mBoxKind;   // _50 0 when no box is up
	int mChoice;    // _54 the box's cursor 0 is yes
	int mBoxTimer;  // _58 frames left on a timed box
	int _5C;        // _5C
	int mCursor;     // _60 from cardGetState 0 idle 0x12 confirm 0xB back
	int mPrevCursor; // _64
	int mFrame;      // _68
	u32 mPressedA;   // _6C pad bit 0x100 A latched for this frame
	TWhiteFadeInOut* mFade; // _70
	int mFadeTimer;         // _74
	TSimpleText* mText[6];    // _78 title help two tabs yes no
	TSimpleText* mLine[3];    // _90
	TSimpleText* mCaption[3]; // _9C
	TSimpleText* mBox;               // _A8 the message box text
	TAeObjMsgWindow* mChoiceWindow;  // _AC the three-way window
	TAeObjMsgWindow* mYesNoWindow;   // _B0
	TMainMenuModel* mIcon[4]; // _B4 the fourth is the one advance ticks
	void* mLight;    // _C4
};

extern "C" {
extern BinFile* sMainMenuFile;
extern int sMainMenuCount;
extern int* sMainMenuEntries;
extern u32 sMainMenuClearColor;
}

#endif
