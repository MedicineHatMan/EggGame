#ifndef _GAMEFLOW_H
#define _GAMEFLOW_H

#include "stage/Stage.h"
#include "object/ObjCommon.h"

enum GameScene {
	SCENE_LICENSE      = 0,
	SCENE_TITLE        = 1,
	SCENE_PROGRESSIVE  = 2, // the progressive scan question TModeTvConfig::create
	SCENE_ATTRACT      = 3, // the title's demo movie alternating between two
	SCENE_MAIN_MENU    = 4,
	SCENE_STORY_STAGE  = 5,
	SCENE_BOSS_STAGE   = 6,
	SCENE_BOSS_AGAIN   = 7, // a boss stage replayed from the stage menu
	SCENE_TEST_STAGE   = 8, // a stage picked from the debug stage list
	SCENE_BATTLE_STAGE = 9,
	SCENE_SAVE         = 10, // TModeResult writes the key bits back to the save
	SCENE_MOVIE        = 11, // mMovie then mAfterMovie
	SCENE_12           = 12, // TModeComing::create nothing goes here
	SCENE_STAGE_MENU   = 13,
	SCENE_14           = 14, // the E3 stage select createModeStageSelectE3 then a story stage
	SCENE_BATTLE_SETUP = 15, // TModeBattleSetup
	SCENE_16           = 16, // TModeGallery then the stage menu
	SCENE_STAFF_ROLL   = 17,
	SCENE_DEBUG_MENU   = 18, // Story Battle JoyCarry
	SCENE_STAGE_SELECT = 19,
	SCENE_BATTLE_MENU  = 20,
	SCENE_JOY_CARRY    = 21, // TModeJoyCarry
	SCENE_MEM_CHECK    = 22,
	SCENE_NONE         = -1,
};

enum GameFlowEvent {
	FLOW_ENTER  = 0,
	FLOW_EXIT   = 1,
	FLOW_UPDATE = 2, // no scene does anything on it
	FLOW_CHECK  = 3,
};

enum GameFlowFlags {
	FLOW_FROM_TITLE = 0x1, // the main menu opens on its idle page
	FLOW_BOSS_DONE  = 0x2, // the boss being fought was beaten before
};

/** What a battle carries It sits over the story fields from _08 on */
struct GameBattle {
	s8 mChara[4];   // _08 -1 for an empty seat
	u8 mPad[4];     // _0C
	int mRule;      // _10
	int mTimeLimit; // _14 for rule 0
	int mStock;     // _18 for rule 1
	int mTarget;    // _1C for rule 2
};

/** The play time the flow carries between stages */
struct FlowTime : public StageTime {
	FlowTime()
	{
		mHour   = 0;
		mMinute = 0;
		mSecond = 0;
		mMilli  = 0;
	}
};

/**
 * @size 0x5C
 */
class TGameFlow {
public:
	TGameFlow();

	void start();
	void startProgressive();
	int reset();
	void update();
	void setMode(TMode* mode);
	void runScene(int scene, int event);
	void setScene(int scene);
	int isModeDone();
	void loadKeys();
	void setStage(int world, int stage);
	void setBoss(int boss);
	void startStage();
	void readLives();
	void startMovie(int movie, int skippable);
	void playMovie(int movie, int next);
	void resetStory();
	void resetBattle();
	void readSave();
	void writeSave();
	inline void freeMode();

	void sceneLicense(int event);
	void sceneTitle(int event);
	void sceneProgressive(int event);
	void sceneAttract(int event);
	void sceneMainMenu(int event);
	void sceneStoryStage(int event);
	void sceneBossStage(int event);
	void sceneBossAgain(int event);
	void sceneTestStage(int event);
	void sceneBattleStage(int event);
	void sceneSave(int event);
	void sceneMovie(int event);
	void scene12(int event);
	void sceneStageMenu(int event);
	void scene14(int event);
	void sceneBattleSetup(int event);
	void scene16(int event);
	void sceneStaffRoll(int event);
	void sceneDebugMenu(int event);
	void sceneStageSelect(int event);
	void sceneBattleMenu(int event);
	void sceneJoyCarry(int event);
	void sceneMemCheck(int event);

	GameBattle* battle() { return (GameBattle*)&mWorld; }
	TMode* mode() { return mModeCtrl->mMode; }

	s8 mScene;                 // _00 a GameScene
	s8 mPrevScene;             // _01
	u8 mAttract;               // _02 which demo movie plays next
	s8 mAfterMovie;            // _03
	s8 mMovie;                 // _04
	u8 mLives;                 // _05
	u8 _06[2];                 // _06
	int mWorld;                // _08
	int mBoss;                 // _0C
	int mStage;                // _10
	u8 mPlayerNo;              // _14
	FlowTime mTime;            // _16
	StageRecord mRecord;       // _1C
	FlagByte mFlags;           // _3C GameFlowFlags
	const char* mStageName;    // _40
	TModeCtrl* mModeCtrl;      // _44
	TModeAllocator mAllocator; // _48
	u8* mKeyBits;              // _54 0x80 bytes key bits 0x400 to 0x7FF while a stage runs
	int _58;                   // _58
};

typedef char TGameFlow_size_check[(sizeof(TGameFlow) == 0x5C) ? 1 : -1];

extern "C" {
extern const char* gStageNames[7][8];
extern const char* gBossStageNames[8];
extern const char* gBossAgainNames[8];
}

#endif
