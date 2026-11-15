#include "stage/GameState.h"
#include "ui/AeObjControl.h"
#include "ui/TFade.h"
#include "sound/SoundCtrl.h"
#include "object/TObjKey00.h"
#include "stage/TEventCtrl.h"
#include "stage/TGameStateClear.h"

typedef char TGameStateClear_size_check[(sizeof(TGameStateClear) == 0x18) ? 1 : -1];
typedef char TGameStateWarp_size_check[(sizeof(TGameStateWarp) == 0x18) ? 1 : -1];
typedef char TGameStateDead_size_check[(sizeof(TGameStateDead) == 0x18) ? 1 : -1];
typedef char TGameStateFailed_size_check[(sizeof(TGameStateFailed) == 0x1C) ? 1 : -1];
typedef char TGameStateGameOver_size_check[(sizeof(TGameStateGameOver) == 0x18) ? 1 : -1];
typedef char TGameStateMiss_size_check[(sizeof(TGameStateMiss) == 0x18) ? 1 : -1];
typedef char TGameStateSucceed_size_check[(sizeof(TGameStateSucceed) == 0x18) ? 1 : -1];
typedef char TGameStateWaitClear_size_check[(sizeof(TGameStateWaitClear) == 0x14) ? 1 : -1];

// 0x80110D10
extern "C" void* createGameStateClear(void* pool, TGameContext*)
{
	return new (pool) TGameStateClear;
}

// 0x80110D4C
extern "C" void* createGameStateWarp(void* pool, TGameContext*)
{
	return new (pool) TGameStateWarp;
}

// 0x80110D88
extern "C" void* createGameStateDead(void* pool, TGameContext*)
{
	return new (pool) TGameStateDead;
}

// 0x80110DC4
extern "C" void* createGameStateFailed(void* pool, TGameContext*)
{
	return new (pool) TGameStateFailed;
}

// 0x80110E00
extern "C" void* createGameStateGameOver(void* pool, TGameContext*)
{
	return new (pool) TGameStateGameOver;
}

// 0x80110E3C
extern "C" void* createGameStateMiss(void* pool, TGameContext*)
{
	return new (pool) TGameStateMiss;
}

// 0x80110E78
extern "C" void* createGameStateSucceed(void* pool, TGameContext*)
{
	return new (pool) TGameStateSucceed;
}

// 0x80110EB4
extern "C" void* createGameStateWaitClear(void* pool, TGameContext*)
{
	return new (pool) TGameStateWaitClear;
}

// 0x80110EF0
TGameStateClear::TGameStateClear()
{
	mName = gTGameStateClearName; // "TGameStateClear"
	mSize = 0x18;
}

// 0x80110F3C
void TGameStateClear::enter(TGameContext*)
{
	mTimer = 420;
}

// 0x80110F48
void TGameStateClear::update(TGameContext* context)
{
	if (mTimer != 0 && --mTimer == 0) {
		context->finish();
	}
}

// 0x80110F88
TGameStateWarp::TGameStateWarp()
{
	mName = gTGameStateWarpName; // "TGameStateWarp"
	mSize = 0x18;
}

// 0x80110FD4
void TGameStateWarp::enter(TGameContext*)
{
	mTimer = 60;
}

// 0x80110FE0
void TGameStateWarp::update(TGameContext* context)
{
	if (mTimer != 0 && --mTimer == 0) {
		context->finish();
	}
}

// 0x80111020
int TGameStateWarp::onMsgA4()
{
	return 1;
}

// 0x80111028
TGameStateDead::TGameStateDead()
{
	mName = gTGameStateDeadName; // "TGameStateDead"
	mSize = 0x18;
}

// 0x80111074
void TGameStateDead::enter(TGameContext*)
{
	mTimer = 90;
}

// 0x80111080
void TGameStateDead::update(TGameContext* context)
{
	if (mTimer != 0) {
		mTimer--;
	}
	if (mTimer == 0) {
		if (context->isLastLife() != 0) {
			SET_GAME_STATE(context, createGameStateGameOver);
		} else {
			SET_GAME_STATE(context, createGameStateMiss);
		}
	}
}

// 0x80111134
int TGameStateDead::onMsgA4()
{
	return 1;
}

// 0x8011113C
TGameStateFailed::TGameStateFailed()
{
	mName = gTGameStateFailedName; // "TGameStateFailed"
	mSize = 0x1C;
}

// 0x80111188
void TGameStateFailed::enter(TGameContext*)
{
	mSent  = 0;
	mTimer = 180;
}

// 0x8011119C
void TGameStateFailed::update(TGameContext* context)
{
	if (mSent == 0) {
		if (eventStartById(0, 0) != 0) {
			ObjMsg msg;
			msg.mType = 0x7A;
			sendMsgById(0, &msg);
			mSent = 1;
		}
		return;
	}
	if (mTimer != 0) {
		mTimer--;
	}
	if (mTimer == 0) {
		if (context->isLastLife() != 0) {
			SET_GAME_STATE(context, createGameStateGameOver);
		} else {
			SET_GAME_STATE(context, createGameStateMiss);
		}
	}
}

// 0x80111294
TGameStateGameOver::TGameStateGameOver()
{
	mName = gTGameStateGameOverName; // "TGameStateGameOver"
	mSize = 0x18;
}

// 0x801112E0
void TGameStateGameOver::enter(TGameContext*)
{
	mFrame = 0;
	aeCreateGameOver();
	bgmFadeTo(0.0f, 60);
}

// 0x80111314
void TGameStateGameOver::update(TGameContext* context)
{
	if (mFrame == 20) {
		sePlayFor(0x2B5, 0);
	} else if (mFrame == 300) {
		context->finish();
	}
	if (mFrame <= 300) {
		mFrame++;
	}
}

// 0x8011137C
int TGameStateGameOver::onMsgA4()
{
	return 1;
}

// 0x80111384
TGameStateMiss::TGameStateMiss()
{
	mName = gTGameStateMissName; // "TGameStateMiss"
	mSize = 0x18;
}

// 0x801113D0
void TGameStateMiss::enter(TGameContext*)
{
	fadeCircleClose();
	mTimer = 60;
}

// 0x80111404
void TGameStateMiss::update(TGameContext* context)
{
	if (mTimer != 0 && --mTimer == 0) {
		context->finish();
	}
}

// 0x80111444
int TGameStateMiss::onMsgA4()
{
	return 1;
}

// 0x8011144C
TGameStateSucceed::TGameStateSucceed()
{
	mName = gTGameStateSucceedName; // "TGameStateSucceed"
	mSize = 0x18;
}

// 0x80111498
void TGameStateSucceed::enter(TGameContext*)
{
	mTimer = 60;
	setKeyBit(0xFA);
}

// 0x801114C4
void TGameStateSucceed::update(TGameContext* context)
{
	if (mTimer != 0) {
		mTimer--;
	}
	if (mTimer == 0) {
		if (eventIsBusy() == 0) {
			SET_GAME_STATE(context, createGameStateWaitClear);
		}
	}
}

// 0x80111544
TGameStateWaitClear::TGameStateWaitClear()
{
	mName = gTGameStateWaitClearName; // "TGameStateWaitClear"
	mSize = 0x14;
}

// 0x80111590
void TGameStateWaitClear::enter(TGameContext*)
{
}

// 0x80111594
int TGameStateWaitClear::onWarp(TGameContext* context)
{
	context->setCleared();
	fadeModel11();
	SET_GAME_STATE(context, createGameStateWarp);
	return 1;
}

// 0x801115F8
int TGameStateWaitClear::onClear(TGameContext* context)
{
	context->setCleared();
	SET_GAME_STATE(context, createGameStateClear);
	return 1;
}

// 0x80111658
// Every mission state sends message 0xA8 here
extern "C" int gameStateToDead(TGameContext* context, ObjMsg*)
{
	SET_GAME_STATE(context, createGameStateDead);
	return 1;
}

// Not in the DOL an unused C helper the linker stripped
extern "C" void gameStateClearStripped(TGameStateMiss* miss, TGameStateWaitClear* wait, TGameStateSucceed* succeed, TGameContext* context,
                                       ObjMsg* msg)
{
	miss->onMessage(context, msg);
	wait->onMessage(context, msg);
	miss->~TGameStateMiss();
	succeed->~TGameStateSucceed();
}
