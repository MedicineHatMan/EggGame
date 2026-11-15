#ifndef _TAEOBJPAUSE_H
#define _TAEOBJPAUSE_H
struct TArc;

#include "ui/TAeLayer.h"

class TSimpleText;
class TAeObjMsgWindow;
struct DrawTask;

/**
 * The pause menu: a backdrop with the title and four rows attached, the play-rest and coin counters over it, and a
 * yes/no window to confirm the last row (quit). Stage and TModeBattle read mResult once mDone is set; they still
 * see it through the stand-in TPauseMenu (Stage.h).
 *
 * @size 0x9C
 */
class TAeObjPause : public TAeObject {
public:
	/** mState */
	enum {
		PAUSE_OPEN    = 0, // the backdrop playing in
		PAUSE_SELECT  = 1,
		PAUSE_CONFIRM = 2, // the yes/no window for quitting
		PAUSE_WAIT    = 3, // for the window to close
		PAUSE_CLOSE   = 4, // the backdrop playing out
	};

	enum {
		PAUSE_ROWS = 4,
		PAUSE_QUIT = 3, // the row that asks first
	};

	TAeObjPause(TObject* parent, void* data, void* assets);
	virtual ~TAeObjPause();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	void stateOpen();    // 0x80124C90
	void stateSelect();  // 0x80124D24
	void stateConfirm(); // 0x801251C0
	void stateWait();    // 0x8012532C
	void stateClose();   // 0x80125398
	void close();        // 0x801253F0 starts the menu playing out
	void closeConfirm(); // 0x801254F8
	void setPad(int pad); // 0x801255C0 whose pad steers it shown on the backdrop

	TAeLayer* mBase;                // _34 the backdrop with the others attached
	TAeLayer* mTitle;               // _38
	TAeLayer* mRow[PAUSE_ROWS];     // _3C
	DrawTask* mTask;                // _4C
	int mState;                     // _50
	int mCursor;                    // _54
	s32 mResult;                    // _58 the row chosen -1 until then
	BOOL mDisabled[PAUSE_ROWS];     // _5C rows the cursor skips
	u32 mPad;                       // _6C
	u32 mCoins;                     // _70
	BOOL mDone;                     // _74 it has played out
	f32 mColor[4];                  // _78 what the rows are tinted
	TSimpleText* mHint;             // _88 the question over the yes/no window
	TSimpleText* mText[2];          // _8C shown with the window
	TAeObjMsgWindow* mWindow;       // _94
	int mChoice;                    // _98 0 yes 1 no
};

class TAeObjPlayRest;
class TObject;

extern "C" {
TAeObjPlayRest* createPlayRest(TObject*);
void freePause();
void loadPause();
}

extern "C" {
TAeObjPause* createPause(TObject*);
}

#include "Ninja/njtypes.h"
extern "C" {
extern const char* gTAeObjPauseName;
extern u32 sPausePulse;
extern s8 sPauseReady;
extern TArc* sPauseArc;
extern void* sPauseAe;
extern NJS_TEXLIST* sPauseTexList;
extern const int gPausePadCells[4];
}

extern "C" {
}

#endif
