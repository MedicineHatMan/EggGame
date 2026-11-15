#ifndef _TMODESTAGEMENU_H
#define _TMODESTAGEMENU_H
struct StageMenuLook;

#include "stage/Stage.h"

class TSimpleText;
class TStageSelectModel;
class TAeObjMsgWindow;
struct NJS_OBJECT;

/**
 * The stage select: a 3D map of the seven worlds (k_stage_select.prd, mes_stg_select.bin). The stick or the
 * d-pad moves the cursor between worlds along sStageMenuLinks, A opens a world's stage list, and B returns to
 * the main menu. World 7 opens the "challenge" list of GBA link games and world 8 the options. mModels[0..9] are
 * the world models, mModels[10] the map and mModels[11] the cursor. The name is ours; the DOL stores none.
 * 0x80175114-0x801792F8, vtable 0x8026224C.
 *
 * @size 0x2A0
 */
class TModeStageMenu : public TMode {
public:
	TModeStageMenu();
	static TModeStageMenu* create(TModeAllocator* allocator);

	virtual ~TModeStageMenu();
	virtual void loadFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void unlockGba();
	int pressedA();
	int pressedB();
	int repeated(u32 mask);

	// State entries Each sets mState
	void enterMap();        // 0
	void enterWorlds();     // 1
	void enterStages();     // 3
	void enterStageWait();  // 4
	void enterGba();        // 5
	void enterGbaWait();    // 7
	void enterGbaSend();    // 8
	void enterGbaBack();    // 9
	void enterOptions();    // 10
	void enterOptionWait(); // 11
	void enterStart();      // 12
	void enterLeave();      // 13
	void enterQuit();       // 15

	// State updates
	void updateMap();        // 0
	void updateWorlds();     // 1
	void updateStagesIn();   // 2
	void updateStagesWait(); // 3
	void updateStages();     // 4
	void updateGbaIn();      // 5
	void updateGbaIn2();     // 6
	void updateGba();        // 7
	void updateGbaSend();    // 8
	void updateGbaBack();    // 9
	void updateOptionsIn();  // 10
	void updateOptions();    // 11
	void updateStart();      // 12
	void updateLeave();      // 13
	void updateQuit();       // 15

	void moveToWorld();
	void moveCursor();
	void showStageInfo();
	void findWorldPos(NJS_OBJECT* node);
	void placeModels(NJS_OBJECT* node);
	void findBurstMtx(f32* mtx, NJS_OBJECT* node, s8 world);

	// Text
	void hideAllText();
	void createTitle(int msg);
	void showTitle(int msg);
	void hideTitle();
	void createInfo();
	void showInfo();
	void hideInfo();
	void createYesNo();
	void showYesNo();
	void hideYesNo();
	void createTabs();
	void showTabs();
	void hideTabs();
	void createHelp(int msg);
	void showHelp(int kind);
	void hideHelp();
	void createCancel();
	void showChoices3();
	void hideChoices3();
	void showGbaInfo();
	void hideGbaInfo();

	// The message box
	void closeBox();
	void closeYesNoWindow();
	void closeChoiceWindow();
	void openYesNo(void* message);
	void openChoice3(void* message);
	void openMessage(void* message);
	void openTimed(void* message);
	int runBox();
	int choseYes();

	s16 mTimer;                     // _0C
	f32 mMoveT;                     // _10 0 to 1 while the cursor moves
	void* mLight;                   // _14
	s8 mState;                      // _18
	s8 mSub;                        // _19
	s8 mWorld;                      // _1A
	s8 mStage;                      // _1B
	u8 _1C;                         // _1C
	s8 mGbaBack;                    // _1D 1 = "back" is chosen on the GBA page
	s8 mGba;                        // _1E the chosen GBA game -1 for none
	TSimpleText* mTitle;            // _20
	TSimpleText* mInfoHead;         // _24
	TSimpleText* mInfo;             // _28
	TSimpleText* mBox;              // _2C
	TSimpleText* mTab[2];           // _30
	TSimpleText* mYes;              // _38
	TSimpleText* mNo;               // _3C
	TSimpleText* mCancel;           // _40
	TSimpleText* mHelp;             // _44
	s8 mBoxKind;                    // _48
	s8 mChoice;                     // _49
	s16 mBoxTimer;                  // _4A
	TAeObjMsgWindow* mWindow;       // _4C
	Vec3f mMoveFrom;                // _50
	TStageSelectModel* mModels[12]; // _5C
	TStageSelectModel* mMarker;     // _8C
	Vec3f mMarkerPos;               // _90
	u8 mFlags;                      // _9C bit 1 keeps the title up while moving
	int mNext;                      // _A0 the result _08 gets on leaving
	int mWorldOpen[7];              // _A4
	int _C0;                        // _C0
	int mHasGba;                    // _C4
	int mStageOpen[7][8];           // _C8
	int mStageClear[7][8];          // _1A8
	int mGbaOpen[5];                // _288
	u8 _29C;                        // _29C
	u8 _29D;                        // _29D
	s8 mGbaStep;                    // _29E
};

// The light of each map look mStageClear decides which 0x80262120 ambient colour and direction
struct StageMenuLook {
	Vec3f mAmbient; // _00
	Vec3f mColor;   // _0C
	Vec3f mDir;     // _18
};

extern "C" {
extern GXColor sModeStageMenuClearColor;
extern const int sStageMenuWorldNodes[9];
extern const int sStageMenuModelNodes[10];
extern const int sStageMenuBurstNodes[9];
extern const int sStageMenuLinks[9][8];
extern const s16 sStageMenuStageKeysA[7][8];
extern const s16 sStageMenuStageKeysB[7][8];
extern const s16 sStageMenuStageIds[7][8];
extern const s16 sStageMenuGbaFlags[5];
extern const int sStageMenuWorldBase[7];
extern StageMenuLook sStageMenuLooks[5];
extern f32 sStageMenuMarkerRates[9];
extern const char* sStageMenuGbaFilesJP[5];
extern const char* sStageMenuGbaFiles[5];
}

#endif
