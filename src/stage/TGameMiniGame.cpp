#include "stage/TGame.h"
#include "stage/EventScript.h"
#include "object/TObjKey00.h"
#include "stage/TGameStateMiniGame.h"
#include "sound/SoundCtrl.h"
#include "stage/StageTime.h"
#include "stage/TEventCtrl.h"
#include "ui/AeObjControl.h"
#include "ui/TFade.h"
#include "object/TObjPlayer.h"

// 0x8010DEF0
extern "C" void* createGameMiniGame(TObject* parent)
{
	return new TGameMiniGame(parent);
}

// 0x8010DF3C
TGameMiniGame::TGameMiniGame(TObject* parent)
    : TGameContext(parent)
{
	mClock = 0;
	mBonus = 0;
	mScore = 0;
	mBestCombo = 0;
	mScript = 0;
	mSwitch = 0;
	mDone = 0;
}

// 0x8010DF98
TGameMiniGame::~TGameMiniGame() { }

// 0x8010DFF4
void TGameMiniGame::startState()
{
	if (mDone == 0) {
		mBonus = 0;
		mScore = 0;
		mBestCombo = 0;
		mCombo = 0;
		mComboTime = 0;
		SET_GAME_STATE(this, createGameStateMiniGameWait);
	} else {
		SET_GAME_STATE(this, createGameStateWaitClear);
	}
}

// 0x8010E09C
void TGameMiniGame::readRules(MiniGameRulesMsg* msg)
{
	mScript = msg->mScript;
	mSwitch = msg->mSwitch;
	mNeeded = msg->mNeeded;
	mClock = msg->mSeconds * 1000 + msg->mMinutes * 60000;
	sePlay(0x38);
}

// 0x8010E0F8
int TGameMiniGame::tickClock()
{
	if (eventIsBusy() != 0) {
		return 0;
	}
	if (mClock == 0) {
		return 1;
	}
	if (mComboTime != 0) {
		mComboTime--;
	}
	stageTimerTick(&mClock);

	StageTime now;
	now.mHour = 0;
	now.mMinute = 0;
	now.mSecond = 0;
	now.mMilli = 0;
	stageTimeSet(&now, mClock);

	HudTimeMsg time;
	time.mType = 0xBF;
	time.mMinute = now.mMinute;
	time.mSecond = now.mSecond;
	time.mMilli = now.mMilli;
	aeSendMissionIcon(&time);
	return 0;
}

// 0x8010E1B0
void TGameMiniGame::addPoints(int points)
{
	if (mComboTime != 0) {
		mComboTime = 60;
		mCombo++;
		if (mCombo > mBestCombo) {
			mBestCombo = mCombo;
		}
		mBonus = 1;

		StageTime now;
		now.mHour = 0;
		now.mMinute = 0;
		now.mSecond = 0;
		now.mMilli = 0;
		StageTime add;
		add.mHour = 0;
		add.mMinute = 0;
		add.mSecond = 0;
		add.mMilli = 0;
		stageTimeSet(&now, mClock);
		add.mSecond = mBonus;
		stageTimeAdd(&now, &add);
		mClock = now.toMillis();
		sePlay(0x2B);

		HudMsg bonus;
		bonus.mType = 0xC0;
		bonus.mValue = mBonus;
		aeSendMissionIcon(&bonus);
	} else {
		mCombo = 0;
		mBonus = 0;
		mComboTime = 60;
	}
	mScore += points;
	gameShowCount(mScore);
}

// 0x8010E2F4
int TGameMiniGame::reportResult()
{
	if (eventScriptRequest(mScript) == 0) {
		return 0;
	}
	setKeySwitch(mSwitch, mScore >= mNeeded);
	return 1;
}

struct TObjPlayer;

typedef char TGameStateMiniGameWait_size_check[(sizeof(TGameStateMiniGameWait) == 0x18) ? 1 : -1];
typedef char TGameStateMiniGame_size_check[(sizeof(TGameStateMiniGame) == 0x1C) ? 1 : -1];

// 0x8010E35C
extern "C" void* createGameStateMiniGameWait(void* pool, TGameContext*)
{
	return new (pool) TGameStateMiniGameWait;
}

// 0x8010E398
TGameStateMiniGameWait::TGameStateMiniGameWait()
{
	mName = gTGameStateMiniGameWaitName; // "TGameStateMiniGameWait"
	mSize = 0x18;
	mReady = 0;
}

// 0x8010E3EC
void TGameStateMiniGameWait::update(TGameContext* context)
{
	if (mReady != 0 && eventIsBusy() == 0) {
		aeCreateMissionIcon6();

		HudTimeMsg time;
		time.mType = 0xBF;
		time.mMinute = 0;
		time.mSecond = 0;
		time.mMilli = 0;
		aeSendMissionIcon(&time);

		HudMsg hud;
		hud.mType = 0xBE;
		hud.mValue = 0;
		aeSendMissionIcon(&hud);

		SET_GAME_STATE(context, createGameStateMiniGame);
	}
}

// 0x8010E494
int TGameStateMiniGameWait::onReady(TGameContext* context, ObjMsg* msg)
{
	((TGameMiniGame*)context)->readRules((MiniGameRulesMsg*)msg);
	mReady = 1;
	return 0;
}

// 0x8010E4D4
extern "C" void* createGameStateMiniGame(void* pool, TGameContext*)
{
	return new (pool) TGameStateMiniGame;
}

// 0x8010E510
TGameStateMiniGame::TGameStateMiniGame()
{
	mName = gTGameStateMiniGameName; // "TGameStateMiniGame"
	mSize = 0x1C;
	mStep = 0;
}

// 0x8010E564
void TGameStateMiniGame::update(TGameContext* context)
{
	switch (mStep) {
	case 0:
		if (((TGameMiniGame*)context)->tickClock() != 0) {
			mStep = 1;
		}
		break;
	case 1:
		if (((TGameMiniGame*)context)->reportResult() != 0) {
			mStep = 2;
		}
		break;
	case 2:
		if (eventStartById(0, 0) != 0) {
			sePlay(0x38);
			aeCreateTimeUp();
			fadeCircleClose();
			mStep  = 3;
			mTimer = 60;
		}
		break;
	case 3:
		if (mTimer == 0) {
			ObjMsg msg;
			findPlayer(0)->vfn_B4(0);
			msg.mType = 0x82;
			sendMsgById(0, &msg);
			mTimer = 60;
			mStep  = 5;
			fadeCircleOpen();
			eventEnd();
		} else {
			mTimer--;
		}
		break;
	case 5:
		if (mTimer == 0) {
			findPlayer(0)->vfn_B4(1);
			mStep = 6;
		} else {
			mTimer--;
		}
		break;
	case 6:
		break;
	}
}

// 0x8010E6C0
int TGameStateMiniGame::onScore(TGameContext* context, ObjMsg* msg)
{
	((TGameMiniGame*)context)->addPoints(((s8*)msg)[4]);
	return 0;
}

// 0x8010E6F0
int TGameStateMiniGame::onFailed(TGameContext* context)
{
	SET_GAME_STATE(context, createGameStateFailed);
	return 0;
}

// 0x8010E744
int TGameStateMiniGame::onWon(TGameContext* context)
{
	*(int*)((u8*)context + 0x3C) = 1;
	SET_GAME_STATE(context, createGameStateSucceed);
	return 0;
}

// 0x8010E7A0
TGameStateMiniGame::~TGameStateMiniGame()
{
}

// 0x8010E818
int TGameStateMiniGame::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x91:
		return onScore(context, msg);
	case 0xA8:
		return gameStateToDead(context, msg);
	case 0xAB:
		return onWon(context);
	case 0x9C:
		return onFailed(context);
	}
	return TGameState::onMessage(context, msg);
}

// 0x8010E894
int TGameStateMiniGameWait::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA8:
		return gameStateToDead(context, msg);
	case 0xAC:
		return onReady(context, msg);
	}
	return TGameState::onMessage(context, msg);
}

// 0x8010E8E8
TGameStateMiniGameWait::~TGameStateMiniGameWait()
{
}
