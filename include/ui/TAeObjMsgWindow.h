#ifndef _TAEOBJMSGWINDOW_H
#define _TAEOBJMSGWINDOW_H
struct TArc;
#include "Ninja/ninja.h"
#include "core/types.h"
struct MsgColor;

#include "ui/TAeLayer.h"

struct DrawTask;

/**
 * A message window: a talk box, a notice box, or a box of choices. The kind picks the layers build() makes. draw()
 * pulses the cursor row's tint and steps every layer; mState follows the window playing in and out. Each mode with a
 * choice calls setPrompt, then setCursor as the cursor moves, then close() and waits for MSGWIN_CLOSED.
 *
 * @size 0x74
 */
class TAeObjMsgWindow : public TAeObject {
public:
	/** mKind */
	enum {
		MSG_TALK_SE      = 0, // the talk box with the opening sound
		MSG_TALK         = 1,
		MSG_NOTICE_ICON  = 2, // a notice with an icon bound in
		MSG_NOTICE       = 3, // notices each with its own frame cell
		MSG_NOTICE_5     = 4,
		MSG_NOTICE_7     = 5,
		MSG_NOTICE_9     = 6,
		MSG_YES_NO       = 7,
		MSG_CHOICE3      = 8,
		MSG_YES_NO_SMALL = 9, // its second panel pulses too
	};

	/** mState */
	enum {
		MSGWIN_OPENING = 0,
		MSGWIN_OPEN    = 1,
		MSGWIN_CLOSING = 2,
		MSGWIN_CLOSED  = 3,
	};

	/** mArrowState */
	enum {
		ARROW_PLAYING = 0,
		ARROW_SHOWN   = 1,
		ARROW_HIDDEN  = 2,
	};

	TAeObjMsgWindow(TObject* parent, void* data, void* assets, u32 kind);
	virtual ~TAeObjMsgWindow();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	void build();                 // 0x800E9FC4
	void makeTalk();              // 0x800EA12C
	void makeNotice();            // 0x800EA1B0
	void makeChoices();           // 0x800EA240
	void makeYesNo();             // 0x800EA444
	void makeChoice3();           // 0x800EA510
	void makeYesNoSmall();        // 0x800EA600
	void setPrompt(int prompt);   // 0x800EA834
	void setCursor(int cursor);   // 0x800EA8A4
	void setMark(int row);        // 0x800EAA64
	BOOL close();                 // 0x800EAB38
	void playArrow();             // 0x800EAD08
	void showArrow();             // 0x800EADA4
	void hideArrow();             // 0x800EAEA4
	void pulse(int step);         // 0x800EB0E8

	DrawTask* mTask;       // _34
	int mKind;             // _38
	TAeLayer* mWindow;     // _3C the box with the others bound into its slots
	TAeLayer* mArrow;      // _40 the next-page arrow
	TAeLayer* mChoices;    // _44 the choice panel
	TAeLayer* mPanelA;     // _48
	TAeLayer* mPanelB;     // _4C
	TAeLayer* mCursorRow;  // _50 the row under the cursor
	TAeLayer* mRow;        // _54 the other rows
	TAeLayer* mMark;       // _58
	TAeLayer* mIcon;       // _5C
	int mState;            // _60
	f32 mPulse;            // _64 0 to 1
	u32 mPulseAngle;       // _68
	BOOL mDieOnClose;      // _6C it kills itself once closed
	s8 mArrowState;        // _70
	u8 _71[3];             // _71
};

extern "C" TAeObjMsgWindow* createMsgWindow(TObject* parent, u32 kind); // 0x800E9E48

extern "C" {
void freeMsgWindow();
void loadMsgWindow();
}

/** A layer tint copied whole before it is set */
struct MsgColor {
	f32 v[4];
};

#include "Ninja/njtypes.h"
extern "C" {
extern const char* gTAeObjMsgWindowName;
extern const f32 gMsgPulseWhite[4];
extern const f32 gMsgPulseMagenta[4];
extern const f32 gMsgPulseOrange[4];
extern TArc* sMsgWindowArc;
extern void* sMsgWindowAe;
extern NJS_TEXLIST* sMsgWindowTexList;
extern const u32 gMsgWindowBanks[3];
extern const u32 gMsgPromptSlots[4];
extern const u32 gMsgChoice3Slots[3];
extern const MsgColor gMsgCursorColor;
extern const MsgColor gMsgRowColor;
extern const MsgColor gMsgMarkColor;
extern const u32 gMsgYesNoSlots[2];
extern f32 sMsgWindowHalf;
}

#endif
