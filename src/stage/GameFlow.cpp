#include "stage/GameFlow.h"
#include "stage/GameSelect.h"
#include "ui/MainMenu.h"
#include "mode/TModeStageMenu.h"
#include "mode/TModeStageSelect.h"
#include "mode/TModeTitle.h"
#include "mode/TModeTvConfig.h"
#include "mode/TModeLicense.h"
#include "mode/TModeResult.h"
#include "ui/StaffRoll.h"
#include "mode/TModeBattleSetup.h"
#include "mode/TModeBattle.h"
#include "mode/TModeGallery.h"
#include "Ninja/ninja.h"
#include "core/JoyCarry.h"
#include "mode/TModeMovie.h"
#include "object/TObjKey00.h"
#include "stage/TStageScore.h"
#include "io/SaveData.h"
#include "stage/TGameState.h"
#include "object/TObjEggYolk.h"
#include "core/Mem.h"
#include "enemy/boss/TEneYellowBossGreen.h"

#include "mode/TModeBattleMenu.h"
#include "mode/TModeMemCheck.h"

struct FlowStep {
	int mId;
	s16 mNext;
	s16 mPad;
	int mArg;
	f32 mTime;
	f32 mScale;
	int mUnk14;
	int mUnk18;
};
static const FlowStep sFlowSteps[9] = {
	{ 0, -1, 0, 0, 30.0f, 1.0f, 0, 0 },
	{ 1, -1, 0, 0, 10.0f, 1.0f, 0, 0 },
	{ 2, -1, 0, 0, 10.0f, 1.0f, 0, 0 },
	{ 3, -1, 0, 0, 30.0f, 1.0f, 0, 0 },
	{ 4, -1, 0, 0, 30.0f, 1.0f, 0, 0 },
	{ (int)0xFF000000, 0, 0, 0, 0.0f, 0.0f, 0, 0 },
	{ 0, -1, 0, 0, 30.0f, 1.0f, 0, 0 },
	{ 1, -1, 0, 0, 10.0f, 1.0f, 0, 0 },
	{ 0x02000000, 0, 0, 0, 0.0f, 0.0f, 0, 0 },
};

// The stage names Eight per world the last ones the bosses

// 0x80262608
// The boss stages in world order
const char* gBossStageNames[8] = {
	"blue_boss", "red_boss", "purple_boss", "orange_boss", "yellow_boss", "green_boss", "last_boss", "last_boss2",
};

// 0x80262628
// The bosses fought again
const char* gBossAgainNames[8] = {
	"blue_boss2", "red_boss2", "purple_boss2", "orange_boss2", "yellow_boss2", "green_boss2", NULL, NULL,
};

static const int sBossMovie[8]          = { 5, 6, 7, 8, 9, 4, 10, 0 };            // 0x8024AFF8
static const int sWorldOpenBit[7]       = { 8, 16, 24, 32, 40, 0, 49 };          // 0x8024B018
static const int sWorldKeyBase[7]       = { 0x41, 0x49, 0x51, 0x59, 0x61, 0x39, 0x69 }; // 0x8024B034
static const int sBossKey[8]            = { 0x72, 0x73, 0x74, 0x75, 0x76, 0x71, 0x77, 0x78 }; // 0x8024B050

struct WorldEmblem {
	u16 mBit;
	u8 mMask;
};
static const WorldEmblem sWorldEmblem[7] = {
	{ 0x00, 0x01 }, { 0x08, 0x02 }, { 0x10, 0x04 }, { 0x18, 0x08 }, { 0x20, 0x10 }, { 0x28, 0x20 }, { 0x31, 0x40 },
}; // 0x8024B070

static const int sWorldStageBase[7]     = { 8, 16, 24, 32, 40, 0, 48 };          // 0x8024B08C

// 0x80181970
// The boss of a world the same number
int bossOfWorld(u32 world)
{
	switch (world) {
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
		return 2;
	case 3:
		return 3;
	case 4:
		return 4;
	case 6:
		return 6;
	case 5:
	default:
		return 5;
	}
}

// 0x801819C8
TGameFlow::TGameFlow()
    : mAllocator(0x300), mScene(SCENE_NONE), mPrevScene(SCENE_NONE), mAttract(0)
{
	_58        = 0;
	mScene     = SCENE_NONE;
	mStageName = 0;
	mModeCtrl  = new TModeCtrl;
	mKeyBits   = (u8*)memAlloc(0x80);
	syMemSet(mKeyBits, 0, 0x80);
}

// 0x80181A78
void TGameFlow::start()
{
	setScene(SCENE_LICENSE);
}

// 0x80181A9C
void TGameFlow::startProgressive()
{
	setScene(SCENE_PROGRESSIVE);
}

inline void TGameFlow::freeMode()
{
	mAllocator.free(mModeCtrl->setMode(0));
}

// 0x80181AC0
// After a reset back to the title unless already before it
int TGameFlow::reset()
{
	mModeCtrl->release();
	if (mScene == SCENE_LICENSE || mScene == SCENE_TITLE || mScene == SCENE_PROGRESSIVE || mScene == SCENE_ATTRACT) {
		return 0;
	}
	freeMode();
	setScene(SCENE_TITLE);
	return 1;
}

// 0x80181B48
void TGameFlow::update()
{
	runScene(mScene, FLOW_CHECK);
	runScene(mScene, FLOW_UPDATE);
	mModeCtrl->update();
}

// 0x80181B9C
void TGameFlow::setMode(TMode* mode)
{
	mModeCtrl->setMode(mode);
}

// 0x80181BC0
void TGameFlow::runScene(int scene, int event)
{
	switch (scene) {
	case SCENE_LICENSE:
		sceneLicense(event);
		break;
	case SCENE_TITLE:
		sceneTitle(event);
		break;
	case SCENE_PROGRESSIVE:
		sceneProgressive(event);
		break;
	case SCENE_ATTRACT:
		sceneAttract(event);
		break;
	case SCENE_MAIN_MENU:
		sceneMainMenu(event);
		break;
	case SCENE_STORY_STAGE:
		sceneStoryStage(event);
		break;
	case SCENE_BOSS_STAGE:
		sceneBossStage(event);
		break;
	case SCENE_BOSS_AGAIN:
		sceneBossAgain(event);
		break;
	case SCENE_TEST_STAGE:
		sceneTestStage(event);
		break;
	case SCENE_BATTLE_STAGE:
		sceneBattleStage(event);
		break;
	case SCENE_SAVE:
		sceneSave(event);
		break;
	case SCENE_MOVIE:
		sceneMovie(event);
		break;
	case SCENE_12:
		scene12(event);
		break;
	case SCENE_STAGE_MENU:
		sceneStageMenu(event);
		break;
	case SCENE_14:
		scene14(event);
		break;
	case SCENE_BATTLE_SETUP:
		sceneBattleSetup(event);
		break;
	case SCENE_16:
		scene16(event);
		break;
	case SCENE_STAFF_ROLL:
		sceneStaffRoll(event);
		break;
	case SCENE_DEBUG_MENU:
		sceneDebugMenu(event);
		break;
	case SCENE_STAGE_SELECT:
		sceneStageSelect(event);
		break;
	case SCENE_BATTLE_MENU:
		sceneBattleMenu(event);
		break;
	case SCENE_JOY_CARRY:
		sceneJoyCarry(event);
		break;
	case SCENE_MEM_CHECK:
		sceneMemCheck(event);
		break;
	}
}

// 0x80181D0C
void TGameFlow::setScene(int scene)
{
	if (mScene != SCENE_NONE) {
		runScene(mScene, FLOW_EXIT);
	}
	if (scene != SCENE_NONE) {
		runScene(scene, FLOW_ENTER);
	}
	mPrevScene = mScene;
	mScene     = scene;
}

// 0x80181D74
int TGameFlow::isModeDone()
{
	return (u8)(mModeCtrl->getState() == 0);
}

// 0x80181DA0
void TGameFlow::loadKeys()
{
	loadKeyBits(mKeyBits, 0x400, 0x400);
}

// 0x80181DCC
void TGameFlow::setStage(int world, int stage)
{
	mWorld     = world;
	mStage     = stage;
	mStageName = gStageNames[world][stage];
}

// 0x80181DF4
void TGameFlow::setBoss(int boss)
{
	mBoss      = boss;
	mStageName = gBossStageNames[boss];
}

// 0x80181E10
void TGameFlow::startStage()
{
	TModeStage* stage = TModeStage::create(&mAllocator);
	stage->mName      = mStageName;
	stage->mPlayerNo  = mPlayerNo;
	stage->_0E        = mLives;
	stage->setRecord(mRecord);
	stage->setTime(mTime);
	setMode(stage);
	saveKeyBits(mKeyBits, 0x400, 0x400);
}

// 0x80181E8C
void TGameFlow::readLives()
{
	mLives = ((TModeStage*)mode())->_0E;
}

// 0x80181EA0
void TGameFlow::startMovie(int movie, int skippable)
{
	TModeMovie* mode = TModeMovie::create(&mAllocator);
	mode->setMovie(movie, skippable);
	setMode(mode);
}

// 0x80181EF4
void TGameFlow::playMovie(int movie, int next)
{
	mMovie      = movie;
	mAfterMovie = next;
	setScene(SCENE_MOVIE);
}

// 0x80181F20
void TGameFlow::resetStory()
{
	mPlayerNo = 0;
	mWorld    = 5;
	mBoss     = 0;
	mStage    = 0;
	if (mLives < 5) {
		mLives = 5;
	}
}

// 0x80181F4C
void TGameFlow::resetBattle()
{
	for (int i = 0; i < 4; i++) {
		battle()->mChara[i] = 0;
		battle()->mPad[i]   = 0;
	}
	battle()->mRule      = 0;
	battle()->mTimeLimit = 1;
	battle()->mStock     = 1;
	battle()->mTarget    = 0;
}

// 0x80181F8C
// The save's key bits become key bits 0x400 on
void TGameFlow::readSave()
{
	for (int i = 0; i < 0x400; i++) {
		setKeySwitch(i + 0x400, saveGetKeyBit(i));
	}
	mLives = saveGetLives();
}

// 0x80181FE4
void TGameFlow::writeSave()
{
	for (int i = 0; i < 0x400; i++) {
		saveSetKeyBit(i, keyBitIsSet(i + 0x400));
	}
	saveSetLives(mLives);
}

// 0x8018203C
void TGameFlow::sceneLicense(int event)
{
	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(TModeLicense::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK && isModeDone()) {
		setScene(SCENE_TITLE);
	}
}

// 0x801820C8
void TGameFlow::sceneTitle(int event)
{
	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(TModeTitle::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		switch (mModeCtrl->getState()) {
		case 0:
			mFlags.mBits |= FLOW_FROM_TITLE;
			setScene(SCENE_MAIN_MENU);
			break;
		case 1:
			setScene(SCENE_ATTRACT);
			break;
		case 4:
			setScene(SCENE_DEBUG_MENU);
			break;
		}
	}
}

// 0x801821A0
void TGameFlow::sceneProgressive(int event)
{
	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(TModeTvConfig::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK && isModeDone()) {
		setScene(SCENE_LICENSE);
	}
}

// 0x8018222C
void TGameFlow::sceneAttract(int event)
{
	if (event == FLOW_ENTER) {
		int movie = MOVIE_DEMO_MOVIE;
		switch ((s8)mAttract) {
		case 0:
			movie = MOVIE_DEMO_MOVIE;
			break;
		case 1:
			movie = MOVIE_DEMO_PLAY;
			break;
		}
		startMovie(movie, 1);
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		switch (mModeCtrl->getState()) {
		case 0:
		case 1:
			mAttract = (mAttract + 1) & 1;
			setScene(SCENE_TITLE);
			break;
		}
	}
}

// 0x80182300
void TGameFlow::sceneMainMenu(int event)
{
	if (event == FLOW_ENTER) {
		TModeMainMenu* menu = TModeMainMenu::create(&mAllocator);
		if ((mFlags.mBits & FLOW_FROM_TITLE) != 0) {
			menu->mOpened = 0;
		} else {
			menu->mOpened = 1;
		}
		clearKeyGroupHigh();
		setMode(menu);
	} else if (event == FLOW_EXIT) {
		mFlags.mBits &= ~FLOW_FROM_TITLE;
		freeMode();
	} else if (event == FLOW_CHECK) {
		int state = mModeCtrl->getState();
		if (state == 0) {
			TModeMainMenu* menu = (TModeMainMenu*)mode();
			if (menu->mFlow != 0) {
				setScene(SCENE_DEBUG_MENU);
			} else {
				readSave();
				switch (menu->mSlot) {
				case 0:
					resetStory();
					if (keyBitIsSet(0x401) == 0) {
						setKeyBit(0x401);
						setStage(5, 0);
						playMovie(2, SCENE_STORY_STAGE);
					} else {
						setScene(SCENE_STAGE_MENU);
					}
					break;
				case 1:
					resetBattle();
					setScene(SCENE_BATTLE_SETUP);
					break;
				default:
					setScene(SCENE_MAIN_MENU);
					break;
				}
			}
		} else if (state == 1) {
			setScene(SCENE_TITLE);
		}
	}
}

// 0x80182490
void TGameFlow::sceneStoryStage(int event)
{
	if (event == FLOW_ENTER) {
		stageRecordClear(&mRecord);
		mTime.mHour   = 0;
		mTime.mMinute = 0;
		mTime.mSecond = 0;
		mTime.mMilli  = 0;
		startStage();
		TModeStage* stage  = (TModeStage*)mode();
		stage->_98         = mStage + sWorldKeyBase[mWorld];
		stage->mStageIndex = mStage + sWorldStageBase[mWorld];
		if (mWorld == 6) {
			if (mStage == 0) {
				stage->_9C = 1;
			}
		} else if (mStage == 1) {
			stage->_9C = 1;
		}
		if (keyBitIsSet(0x43C) == 0) {
			((TModeStage*)mode())->_24 = 0;
		}
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		switch (mModeCtrl->getState()) {
		case 2: {
			readLives();
			TModeStage* stage = (TModeStage*)mode();
			mRecord           = stage->mSavedRecord;
			stage             = (TModeStage*)mode();
			mTime.mHour       = stage->mSavedTime.mHour;
			mTime.mMinute     = stage->mSavedTime.mMinute;
			mTime.mSecond     = stage->mSavedTime.mSecond;
			mTime.mMilli      = stage->mSavedTime.mMilli;
			if (mWorld == 6) {
				if (mStage == 0) {
					setBoss(bossOfWorld(mWorld));
					setScene(SCENE_BOSS_STAGE);
				} else {
					mLives++;
					setScene(SCENE_SAVE);
				}
			} else if (mStage == 1) {
				setBoss(bossOfWorld(mWorld));
				setScene(SCENE_BOSS_STAGE);
			} else {
				mLives++;
				setScene(SCENE_SAVE);
			}
			break;
		}
		case 3:
			setScene(SCENE_TITLE);
			break;
		case 1:
		case 6:
			readLives();
			loadKeys();
			setScene(SCENE_STAGE_MENU);
			break;
		case 5:
			setScene(SCENE_TITLE);
			break;
		}
	}
}

// 0x80182758
int bossCleared(int boss)
{
	u16 bits[8] = { 0x45, 0x4D, 0x55, 0x5D, 0x65, 0x3D, 0, 0 };
	return keyBitIsSet(bits[boss] + 0x400);
}

// 0x801827B0
void TGameFlow::sceneBossStage(int event)
{
	if (event == FLOW_ENTER) {
		startStage();
		TModeStage* stage  = (TModeStage*)mode();
		stage->_98         = sBossKey[mBoss];
		stage->mStageIndex = mStage + sWorldStageBase[mWorld];
		if (mBoss == 6) {
			stage->_9C = 1;
		}
		if (bossCleared(mBoss) != 0) {
			mFlags.mBits |= FLOW_BOSS_DONE;
		} else {
			mFlags.mBits &= ~FLOW_BOSS_DONE;
		}
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		switch (mModeCtrl->getState()) {
		case 2:
			readLives();
			storySeqEval(sBossKey[mBoss]);
			if (mBoss == 6) {
				setBoss(7);
				setScene(SCENE_BOSS_STAGE);
			} else if (mBoss == 7) {
				playMovie(3, SCENE_STAFF_ROLL);
			} else if (mFlags.mBits & FLOW_BOSS_DONE) {
				mLives++;
				setScene(SCENE_SAVE);
			} else {
				mStageName = gBossAgainNames[mBoss];
				playMovie(sBossMovie[mBoss], SCENE_BOSS_AGAIN);
			}
			break;
		case 3:
			setScene(SCENE_TITLE);
			break;
		case 1:
			readLives();
			loadKeys();
			setScene(SCENE_STAGE_MENU);
			break;
		case 5:
			setScene(SCENE_TITLE);
			break;
		}
	}
}

// 0x801829BC
void TGameFlow::sceneBossAgain(int event)
{
	if (event == FLOW_ENTER) {
		startStage();
		TModeStage* stage = (TModeStage*)mode();
		stage->_98        = sBossKey[mBoss];
		stage->_9C        = 1;
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		switch (mModeCtrl->getState()) {
		case 2:
			readLives();
			mLives++;
			setScene(SCENE_SAVE);
			break;
		case 3:
			setScene(SCENE_TITLE);
			break;
		case 1:
			readLives();
			loadKeys();
			setScene(SCENE_STAGE_MENU);
			break;
		case 5:
			setScene(SCENE_TITLE);
			break;
		}
	}
}

// 0x80182AD8
void TGameFlow::sceneTestStage(int event)
{
	if (event == FLOW_ENTER) {
		stageRecordClear(&mRecord);
		mTime.mHour   = 0;
		mTime.mMinute = 0;
		mTime.mSecond = 0;
		mTime.mMilli  = 0;
		startStage();
		((TModeStage*)mode())->_24 = 0;
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		if (mModeCtrl->getState() != 8) {
			setScene(SCENE_TITLE);
		}
	}
}

// 0x80182B88
void TGameFlow::sceneBattleStage(int event)
{
	TModeBattle* mode;
	int i;

	if (event == FLOW_ENTER) {
		mode = TModeBattle::create(&mAllocator);
		mode->mRule       = battle()->mRule;
		switch (battle()->mRule) {
		case 0:
			mode->mLives = battle()->mTimeLimit;
			break;
		case 1:
			mode->mTime = battle()->mStock;
			break;
		case 2:
			mode->mGoal = battle()->mTarget;
			break;
		}
		mode->mName = mStageName;
		for (i = 0; i < 4; i++) {
			if (battle()->mChara[i] >= 0) {
				mode->setChara(i, battle()->mChara[i]);
			}
			mode->mEntries[i].mWins = battle()->mPad[i];
		}
		setMode(mode);
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		int state = mModeCtrl->getState();
		if (state == 5) {
			setScene(SCENE_TITLE);
		} else if (state != 8) {
			mode = (TModeBattle*)this->mode();
			for (i = 0; i < 4; i++) {
				// the DOL indexes from this battle()->mPad[i] adds the index to this first
				((u8*)this)[i + 12] = mode->mEntries[i].mWins;
			}
			setScene(SCENE_BATTLE_SETUP);
		}
	}
}

// 0x80182D04
void TGameFlow::sceneSave(int event)
{
	if (event == FLOW_ENTER) {
		writeSave();
		mModeCtrl->setMode(TModeResult::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK && isModeDone()) {
		if (mPrevScene == SCENE_STAFF_ROLL) {
			setScene(SCENE_LICENSE);
		} else {
			setScene(SCENE_STAGE_MENU);
		}
	}
}

// 0x80182DB0
void TGameFlow::sceneMovie(int event)
{
	if (event == FLOW_ENTER) {
		startMovie(mMovie, 0);
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		switch (mModeCtrl->getState()) {
		case 0:
		case 1:
			setScene(mAfterMovie);
			break;
		}
	}
}

// 0x80182E4C
void TGameFlow::scene12(int event)
{
	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(TModeComing::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK && isModeDone()) {
		setScene(SCENE_12);
	}
}

// Defined here after bossCleared's local table which comes first in .rodata
static const u16 sStageClearBase[7] = { 0x44, 0x4C, 0x54, 0x5C, 0x64, 0x3C, 0x6C }; // 0x8024B0B8

// 0x80182ED8
void TGameFlow::sceneStageMenu(int event)
{
	TModeStageMenu* menu;
	int w;
	int s;
	int open;

	if (event == FLOW_ENTER) {
		menu = TModeStageMenu::create(&mAllocator);
		for (w = 0; w < 7; w++) {
			open = 0;
			for (s = 0; s < 8; s++) {
				int stageOpen = storySeqEval(s + sWorldOpenBit[w]);
				if (stageOpen != 0) {
					open++;
				}
				menu->mStageOpen[w][s]  = stageOpen;
				menu->mStageClear[w][s] = keyBitIsSet((u16)(s + sStageClearBase[w]) + 0x400);
			}
			menu->mWorldOpen[w] = open;
		}
		menu->_29C   = mLives;
		menu->_29D   = countKeyBits();
		menu->mWorld = mWorld;
		setMode(menu);
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		switch (mModeCtrl->getState()) {
		case 0: {
			TModeStageMenu* menu = (TModeStageMenu*)mode();
			mWorld               = menu->mWorld;
			mStage               = menu->mStage;
			if (mStage == 5) {
				mPlayerNo = 1;
			} else if (mStage == 6) {
				mPlayerNo = 2;
			} else if (mStage == 7) {
				mPlayerNo = 3;
			} else {
				mPlayerNo = 0;
			}
			mStageName = gStageNames[mWorld][mStage];
			setScene(SCENE_STORY_STAGE);
			break;
		}
		case 1:
			setScene(SCENE_MAIN_MENU);
			break;
		case 7:
			setScene(SCENE_16);
			break;
		}
	}
}

// 0x801830E8
void TGameFlow::scene14(int event)
{
	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(createModeStageSelectE3(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK && isModeDone()) {
		setScene(SCENE_STORY_STAGE);
	}
}

// 0x80183174
void TGameFlow::sceneBattleSetup(int event)
{
	TModeBattleSetup* setup;
	u8 worlds;
	u32 i;

	if (event == FLOW_ENTER) {
		worlds = 0;
		for (i = 0; i < 7; i++) {
			if (storySeqEval(sWorldEmblem[i].mBit) != 0) {
				worlds |= sWorldEmblem[i].mMask;
			}
		}
		setup = TModeBattleSetup::create(&mAllocator);
		for (i = 0; i < 4; i++) {
			setup->mPad[(u8)i] = ((u8*)this)[i + 12];
		}
		setup->openStages(worlds);
		setMode(setup);
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		setup = (TModeBattleSetup*)mode();
		if (setup->_08 == 0) {
			battle()->mRule = setup->mRule;
			for (u32 k = 0; k < 4; k++) {
				((s8*)this)[k + 8] = setup->mChara[(u8)k];
			}
			switch (setup->mRule) {
			case 0:
				battle()->mTimeLimit = setup->mAmount;
				break;
			case 1:
				battle()->mStock = setup->mAmount;
				break;
			case 2:
				battle()->mTarget = setup->mAmount;
				break;
			}
			mStageName = setup->getStageName();
			setScene(SCENE_BATTLE_STAGE);
		} else if (setup->_08 == 1) {
			setScene(SCENE_MAIN_MENU);
		}
	}
}

// 0x8018331C
void TGameFlow::scene16(int event)
{
	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(TModeGallery::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK && isModeDone()) {
		setScene(SCENE_STAGE_MENU);
	}
}

// 0x801833A8
void TGameFlow::sceneStaffRoll(int event)
{
	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(TModeStaffRoll::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK && isModeDone()) {
		setScene(SCENE_SAVE);
	}
}

// 0x80183434
void TGameFlow::sceneDebugMenu(int event)
{
	if (event == FLOW_ENTER) {
		TModeGameSelect* menu = TModeGameSelect::create(&mAllocator);
		menu->addItem(SCENE_STAGE_SELECT, "Story");
		menu->addItem(SCENE_BATTLE_MENU, "Battle");
		menu->addItem(SCENE_JOY_CARRY, "JoyCarry");
		setMode(menu);
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		int state = mModeCtrl->getState();
		if (state == 0) {
			TModeGameSelect* menu = (TModeGameSelect*)mode();
			setScene(menu->mItems[menu->mCursor].mScene);
		} else if (state == 1) {
			setScene(SCENE_TITLE);
		}
	}
}

// 0x80183524
void TGameFlow::sceneStageSelect(int event)
{
	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(TModeStageSelect::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		TModeStageSelect* select = (TModeStageSelect*)mode();
		if (select->_08 == 0) {
			mStageName = select->getStageName();
			mPlayerNo  = select->mPlayer;
			mLives     = 3;
			setScene(SCENE_TEST_STAGE);
		} else if (select->_08 == 1) {
			setScene(SCENE_DEBUG_MENU);
		}
	}
}

// 0x801835E8
void TGameFlow::sceneBattleMenu(int event)
{
	GameBattle* b = battle();

	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(TModeBattleMenu::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		TModeBattleMenu* menu = (TModeBattleMenu*)mode();
		if (menu->_08 == 0) {
			b->mRule = menu->mRule;
			for (int i = 0; i < 4; i++) {
				((s8*)this)[i + 8] = i; // battle()->mChara[i] indexed from this as the DOL has it
			}
			mStageName = menu->mStages.mData[menu->mStage];
			setScene(SCENE_BATTLE_STAGE);
		} else if (menu->_08 == 1) {
			setScene(SCENE_DEBUG_MENU);
		}
	}
}

// 0x801836C8
void TGameFlow::sceneJoyCarry(int event)
{
	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(TModeJoyCarry::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		if (mModeCtrl->getState() != 8) {
			setScene(SCENE_DEBUG_MENU);
		}
	}
}

// 0x80183758
void TGameFlow::sceneMemCheck(int event)
{
	if (event == FLOW_ENTER) {
		mModeCtrl->setMode(TModeMemCheck::create(&mAllocator));
	} else if (event == FLOW_EXIT) {
		freeMode();
	} else if (event == FLOW_CHECK) {
		if (mModeCtrl->getState() != 8) {
			setScene(SCENE_LICENSE);
		}
	}
}
