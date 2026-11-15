#ifndef _TMODEBATTLESETUP_H
#define _TMODEBATTLESETUP_H

#include "stage/Stage.h"
#include "core/TArray.h"
#include "egg/ObjEgg.h"
#include "engine/FxHolder.h"

class TSimpleText;
class TAeObjMsgWindow;
class TWhiteFadeInOut;
class TBattleModelPlayer;
class TBattleModelHiyoko;
class TStageSelectModel;

/** The pages of the battle setup */
enum BattleSetupState {
	BSETUP_NO_PADS    = 0, // fewer than two pads a message until it is closed
	BSETUP_ENTRY_WAIT = 1,
	BSETUP_CHARA      = 2, // each pad picks a fighter
	BSETUP_CHARA_DONE = 3,
	BSETUP_RULE_WAIT  = 4,
	BSETUP_RULE       = 5, // rule amount and stage
	BSETUP_RULE_BACK  = 6,
	BSETUP_START      = 7, // fading out to the battle
	BSETUP_QUIT       = 8, // the yes/no box
	BSETUP_QUIT_FADE  = 9,
};

/**
 * The battle setup: pick the fighters, then the rule, the amount and the stage. k_battle_select.prd. Vtable
 * 0x80263490. The 2D side is TAeObjBattleSelect (the fn_8019* calls). _08 ends 0 to play, 1 to go back.
 * The class name is ours.
 *
 * @size 0x170
 */
class TModeBattleSetup : public TMode {
public:
	TModeBattleSetup();
	static TModeBattleSetup* create(TModeAllocator* allocator); // 0x801935CC

	virtual ~TModeBattleSetup() { }
	virtual void loadFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	const char* getStageName(); // 0x80193D5C
	void openStages(u8 worlds); // 0x8019609C one bit a world

	void start();            // 0x80193EAC
	void updateNoPads();     // 0x80193F60
	void enterEntry();       // 0x80193FA0
	void updateEntryWait();  // 0x80194038
	void enterChara();       // 0x80194070
	void updateChara();      // 0x80194198
	void enterCharaDone();   // 0x8019459C
	void updateCharaDone();  // 0x801945F0
	void enterRuleWait();    // 0x8019467C
	void updateRuleWait();   // 0x80194708
	void enterRule();        // 0x80194740
	void updateRule();       // 0x80194814
	void enterRuleBack();    // 0x801949C4
	void updateRuleBack();   // 0x80194A1C
	void updateRuleKind();   // 0x80194A54
	void updateRuleAmount(); // 0x80194B78
	void updateRuleStage();  // 0x80194CD4
	void enterStart();       // 0x80194F98
	void updateStart();      // 0x80194FD4
	void enterQuit();        // 0x80195010
	void updateQuit();       // 0x80195068
	void updateQuitFade();   // 0x801950E8

	int padDecide(int pad);           // 0x80195124
	int padCancel(int pad);           // 0x80195150
	int padRepeat(int pad, u32 mask); // 0x8019517C a stick direction also counts as its d-pad one
	void closeBox();                  // 0x80195220
	void closeWindow();               // 0x80195264
	void openYesNo(void* text);       // 0x801952B4
	void openMessage(void* text);     // 0x8019536C
	int runBox();                     // 0x801953D8 non-zero once the box is answered
	int boxYes();                     // 0x8019554C
	void freeFade();                  // 0x80195560
	void startFade();                 // 0x8019558C
	int fadeDone();                   // 0x801955E8

	void buildText();             // 0x8019560C
	void hideAll();               // 0x80195908
	void showTitle(int page);     // 0x80195964
	void hideTitle();             // 0x801959B8
	void showTabs();              // 0x801959D4
	void hideTabs();              // 0x80195A08
	void showRuleHeads(int page); // 0x80195A3C
	void hideRuleHeads();         // 0x80195AC0
	void showRuleKind();          // 0x80195B0C
	void hideRuleKind();          // 0x80195BE8
	void showRuleAmount();        // 0x80195C04
	void hideRuleAmount();        // 0x80195DDC
	void showRuleHelp();          // 0x80195DF8
	void hideRuleHelp();          // 0x80195F94
	void showYesNo();             // 0x80195FB0
	void hideYesNo();             // 0x80196068
	void showStageIcons();        // 0x80196138

	int mState;                     // _0C a BattleSetupState
	TWhiteFadeInOut* mFade;         // _10
	int mFadeTimer;                 // _14
	int mTimer;                     // _18
	int mBoxPad;                    // _1C the pad that opened the box
	TSimpleText* mTitle;            // _20
	TSimpleText* mTab[2];           // _24
	TSimpleText* mRuleHead[3];      // _2C
	TSimpleText* mRuleKind;         // _38
	TSimpleText* mRuleAmount;       // _3C
	TSimpleText* mRuleHelp;         // _40
	TSimpleText* mYes;              // _44
	TSimpleText* mNo;               // _48
	TSimpleText* mBox;              // _4C
	s8 mBoxKind;                    // _50 1 yes/no 2 a message
	s8 mChoice;                     // _51 0 yes 1 no
	TAeObjMsgWindow* mWindow;       // _54
	TObject* _58;                   // _58
	HitVec mPlayerPos[4];           // _5C
	FxVec mPlayerRot[4];            // _8C
	TBattleModelPlayer* mPlayer[4]; // _BC
	Vec3f mStagePos;                // _CC
	SetRot mStageRot;               // _D8
	TStageSelectModel* mStage;      // _E4
	Vec3f mHiyokoPos;               // _E8
	SetRot mHiyokoRot;              // _F4
	TBattleModelHiyoko* mHiyoko;    // _100
	Vec3f mChickPos;                // _104
	SetRot mChickRot;               // _110
	TObject* mChick;                // _11C from createTBattleModelHiyokoProp
	int mTaken[4];                  // _120 per fighter
	s8 mCursor[4];                  // _130 per pad
	s8 mChara[4];                   // _134 per pad -1 none
	u8 mRow;                        // _138 0 the rule 1 the amount 2 the stage
	u8 mRule;                       // _139
	u8 mAmount;                     // _13A
	TArray<const char*> mStages;    // _13C the "battle" stages
	u8 mStageIdx;                   // _148
	int mStageOpen[8];              // _14C
	u8 mPad[4];                     // _16C
};

extern "C" {
extern u32 sModeBattleSetupClearColor;
}

#endif
