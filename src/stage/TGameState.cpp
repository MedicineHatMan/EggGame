#include "stage/GameState.h"
#include "stage/TGame.h"
#include "ui/TFade.h"
#include "player/PlyPool.h"
#include "stage/TGameGoldenEgg.h"
#include "stage/TGameDino.h"
#include "stage/TGameSnowman.h"
#include "stage/TGameGoal.h"
#include "stage/TGameCollect.h"
#include "stage/TGameRace.h"
#include "stage/TGameBoss.h"
#include "stage/TGameMiniGame.h"
#include "stage/TGameRescue.h"
#include "stage/TGameFireworks.h"
#include "stage/TGameGear.h"
#include "stage/TGameBattery.h"
#include "stage/TGameWipeOut.h"
#include "stage/TGameBossAfter.h"
#include "stage/TGameLastBoss.h"
#include "stage/TGameFreePlay.h"
#include "stage/StageTime.h"
#include "object/TObjKey00.h"
#include "stage/TEventCtrl.h"
#include "stage/TGameState.h"

struct MissionContextEntry {
	int mType;                       // _00
	void* (*mCreate)(TObject* parent); // _04
};

// 0x80255410
static MissionContextEntry sMissionContexts[15] = {
	{ 5, createGameGoldenEgg }, { 2, createGameDino },       { 9, createGameSnowman },    { 4, createGameGoal },
	{ 1, createGameCollect },   { 7, createGameRace },       { 0, createGameBoss },       { 6, createGameMiniGame },
	{ 8, createGameRescue },    { 12, createGameFireworks }, { 10, createGameGear },      { 11, createGameBattery },
	{ 13, createGameWipeOut },  { 14, createGameBossAfter }, { 15, createGameLastBoss },
};

static TCustomGameContext* sGameContext; // 0x802C4DF0 messages for the game go here
static u16 sStageBgm;                    // 0x802C4DF4 0xFFFF for the stage's own

/** The clock the context shows sent each frame as message 0xB4 */
struct GameTimeMsg : public ObjMsg {
	u8 _02[2];
	int mMinute; // _04
	int mSecond; // _08
	int mMilli;  // _0C
};

/** The "minigame" event command's arguments sent on as a MiniGameRulesMsg */
struct MiniGameRules {
	u16 mScript; // _00
	u16 mSwitch; // _02
	u8 mMinutes; // _04
	u8 mSeconds; // _05
	u16 mNeeded; // _06
};

typedef char TGameState_size_check[(sizeof(TGameState) == 0x14) ? 1 : -1];
typedef char TGameContext_size_check[(sizeof(TGameContext) == 0x38) ? 1 : -1];

// 0x800421F8
TGameState::TGameState()
{
	mName = sGameStateName;
	mSize = 0x14;
}

// 0x80042250
int TGameState::onMessage(TGameContext* context, ObjMsg* msg)
{
	return context->handleMessage(msg);
}

// 0x80042278
// Five lives and a clock that stops at 59:59.99
TGameContext::TGameContext(TObject* parent)
    : TCustomGameContext(parent)
    , mTime(0)
{
	_00    = (u32)sGameContextName;
	mSize  = 0x38;
	mLives = 5;
	void* pool = ::operator new(0xC);
	if (pool != 0) {
		pool = initPlyPool(pool, 0x40);
	}
	mPool      = pool;
	mTimeLimit = 3599990;
}

// 0x8004236C
TGameContext::~TGameContext()
{
	freePlyPool(mPool, 1);
}

// 0x800423F8
void TGameContext::start()
{
	sendLives();
	startState();
}

// 0x80042438
// A life is lost and the run starts again
void TGameContext::respawn()
{
	mLives--;
	sendLives();
	mEndFlags.mBits &= ~4;
	startState();
}

// 0x80042490
int TGameContext::isLastLife()
{
	return mLives == 1 ? 1 : 0;
}

// 0x800424A4
void TGameContext::finish()
{
	mEndFlags.mBits |= 4;
}

// 0x800424B4
void TGameContext::setCleared()
{
	mEndFlags.mBits |= 2;
}

// 0x800424C4
void TGameContext::setWarped()
{
	mEndFlags.mBits |= 8;
}

// 0x800424D4
// 0 while running then 3 cleared 4 warped 2 on the last life and 1 otherwise
int TGameContext::getStatus()
{
	if (mEndFlags.mBits & 4) {
		if (mEndFlags.mBits & 2) {
			return 3;
		}
		if (mEndFlags.mBits & 8) {
			return 4;
		}
		return (mLives == 1 ? 1 : 0) + 1;
	}
	return 0;
}

// 0x80042520
int TGameContext::handleMessage(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x8F:
		setWarped();
		fadeToWhite();
		SET_GAME_STATE(this, createGameStateWarp);
		return 1;
	case 0x9A:
		mLives++;
		sendLives();
		return 1;
	case 0x9B:
		StageTime now;
		StageTime extra;
		now.mHour     = 0;
		now.mMinute   = 0;
		now.mSecond   = 0;
		now.mMilli    = 0;
		extra.mHour   = 0;
		extra.mMinute = 0;
		extra.mSecond = 0;
		extra.mMilli  = 0;
		stageTimeSet(&now, mTime);
		extra.mMinute = ((AddTimeMsg*)msg)->mMinute;
		extra.mSecond = ((AddTimeMsg*)msg)->mSecond;
		stageTimeSub(&now, &extra);
		mTime = now.toMillis();
		return 1;
	}
	return 0;
}

// 0x8004265C
void TGameContext::sendLives()
{
	LivesMsg msg;
	msg.mType  = 0xB1;
	msg._04    = 0;
	msg.mLives = mLives;
	msg.mSlot  = mSlot;
	aeSendMessage(&msg);
}

// 0x800426A0
int TGameContext::onMessage(ObjMsg* msg)
{
	if (mStateSlot.mState != 0) {
		return mStateSlot.mState->onMessage(this, msg);
	}
	return 0;
}

// 0x800426EC
// Runs the state until the mission ends shows the clock and runs it while no event plays
void TGameContext::draw()
{
	if (!(mEndFlags.mBits & 4)) {
		if (mStateSlot.mState != 0) {
			mStateSlot.mState->vfn_18(this);
		}
		mStateSlot.update(this);
	}
	StageTime now;
	GameTimeMsg msg;
	now.mHour   = 0;
	now.mMinute = 0;
	now.mSecond = 0;
	now.mMilli  = 0;
	msg.mType   = 0xB4;
	stageTimeSet(&now, mTime);
	msg.mMinute = now.mMinute;
	msg.mSecond = now.mSecond;
	msg.mMilli  = now.mMilli;
	aeSendMessage(&msg);
	if (!eventIsBusy() && mTime < mTimeLimit) {
		mTime += 16;
		if (mTime > mTimeLimit) {
			mTime = mTimeLimit;
		}
	}
}

// 0x800427C8
// Gives msg to the current context
extern "C" int sendGameMsg(ObjMsg* msg)
{
	if (sGameContext != 0) {
		return sGameContext->onMessage(msg);
	}
	return 0;
}

// 0x80042814
extern "C" void gameContextInit()
{
	sGameContext = 0;
	sStageBgm    = 0xFFFF;
}

// 0x8004282C
extern "C" void gameContextExit()
{
	sGameContext = 0;
}

// 0x80042838
// The rules for a mission type or free play for one not in the table
extern "C" TGameContext* createGameContext(int type)
{
	TGameContext* context = 0;
	for (int i = 0; i < 15; i++) {
		if (type == sMissionContexts[i].mType) {
			context = (TGameContext*)sMissionContexts[i].mCreate(0);
			break;
		}
	}
	if (context == 0) {
		context = (TGameContext*)createGameFreePlay(0);
	}
	return context;
}

// 0x800428BC
extern "C" void setGameContext(TCustomGameContext* context)
{
	sGameContext = context;
}

// 0x800428C4
extern "C" int getStageBgm()
{
	return sStageBgm;
}

// 0x800428CC
// Event command "set_bgm"
extern "C" int cmdSetBgm(u16* args)
{
	sStageBgm = args[0];
	return 1;
}

// 0x800428DC
// Event command "minigame" the rules of the mini game as message 0xAC
extern "C" int cmdMiniGame(MiniGameRules* args)
{
	MiniGameRulesMsg msg;
	msg.mType    = 0xAC;
	msg.mScript  = args->mScript;
	msg.mSwitch  = args->mSwitch;
	msg.mMinutes = args->mMinutes;
	msg.mSeconds = args->mSeconds;
	msg.mNeeded  = args->mNeeded;
	sendGameMsg(&msg);
	return 1;
}

// 0x80042934
// Event command "ms_success"
extern "C" int cmdMissionSuccess(void*)
{
	ObjMsg msg;
	msg.mType = 0xAB;
	sendGameMsg(&msg);
	return 1;
}

// 0x80042964
// Event command "ms_fail"
extern "C" int cmdMissionFail(void*)
{
	ObjMsg msg;
	msg.mType = 0x9C;
	sendGameMsg(&msg);
	return 1;
}

// Not in the DOL an unused C helper the linker stripped
extern "C" void gameStateStep(TGameState* state, TGameContext* context)
{
	state->run(context);
	state->update(context);
}

// 0x80042994
// How many of the key bits 0x43C-0x477 are set
extern "C" int countKeyBits()
{
	int count = 0;
	for (int i = 0x3C; i <= 0x77; i++) {
		if (keyBitIsSet(i + 0x400)) {
			count++;
		}
	}
	return count;
}
