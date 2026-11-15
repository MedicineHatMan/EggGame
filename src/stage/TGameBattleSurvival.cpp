#include "stage/TGame.h"
#include "stage/TGameBattleSurvival.h"

/**
 * @size 0x8
 */
struct SurvivalRank {
	int mPlayer; // _00
	int mOrder;  // _04
};

const char* gTGameBattleSurvivalName = "TGameBattleSurvival";
const char* gTGameStateSurvivalName = "TGameStateSurvival";

// 0x8019F07C
TGameBattleSurvival::TGameBattleSurvival(TObject* parent)
    : TGameContextBattle(parent)
{
	_00         = (u32)gTGameBattleSurvivalName;
	mSize       = 0xBC;
	mStartLives = 1;
}

// 0x8019F0D0
extern "C" void* createGameBattleSurvival()
{
	return new TGameBattleSurvival(0);
}

// 0x8019F110
void TGameBattleSurvival::setLives(int setting)
{
	switch (setting) {
	case 0:
		mStartLives = 1;
		break;
	case 1:
		mStartLives = 3;
		break;
	case 2:
		mStartLives = 5;
		break;
	}
}

// 0x8019F158
void TGameBattleSurvival::startState()
{
	for (int i = 0; i < mPlayerCount; i++) {
		mScores[i].mLives = mStartLives;
	}
	SET_BATTLE_STATE(this, createGameStateBattleStart);
}

// 0x8019F1D8
void TGameBattleSurvival::start()
{
	void* pool;
	for (int i = 0; i < mPlayerCount; i++) {
		struct {
			s16 mKind;
			u16 _02;
			u16 mPlayer;
			s16 mLives;
			u16 mValue;
		} msg;
		msg.mKind   = 0xB1;
		msg.mPlayer = mPlayers[i].mId;
		msg.mLives  = mScores[i].mLives;
		msg.mValue  = mPlayers[i]._00;
		aeSendMessage(&msg);
	}
	pool = mPool;
	releasePlyState(pool, changeBattleState(&mStateSlot, this, createGameStateSurvival(pool, this)));
}

// 0x8019F284
int TGameBattleSurvival::checkEnd()
{
	int i;
	int alive = 0;
	int last  = -1;
	for (i = 0; i < mPlayerCount; i++) {
		if (mScores[i].mLives != 0) {
			alive++;
			last = i;
		}
	}
	if (alive == 0) {
		for (int i = 0; i < mPlayerCount; i++) {
			if (mPlayers[i].mPlace == 0) {
				mPlayers[i].mPlace = 1;
			}
		}
		return 1;
	}
	if (alive == 1) {
		((s8*)&mPlayers[last])[8] = 1;
		rankOut(2);
		return 0;
	}
	rankOut(alive + 1);
	return 2;
}

// 0x8019F360
extern "C" void survivalRankSort(SurvivalRank* ranks, int count)
{
	int player;
	SurvivalRank* cur;
	int order;
	int j;
	int i;
	for (i = 0; i < count; i++) {
		for (j = 0; j < count - 1; j++) {
			cur = &ranks[j];
			order = cur->mOrder;
			if (order > cur[1].mOrder) {
				player = cur->mPlayer;
				cur->mPlayer = cur[1].mPlayer;
				cur->mOrder = cur[1].mOrder;
				cur[1].mPlayer = player;
				cur[1].mOrder = order;
			}
		}
	}
}

// 0x8019F3C4
// Gives places to those newly out
void TGameBattleSurvival::rankOut(int place)
{
	SurvivalRank ranks[4];
	int count = 0;
	for (int i = 0; i < mPlayerCount; i++) {
		if (mScores[i].mLives == 0 && mPlayers[i].mPlace == 0) {
			ranks[count].mPlayer = i;
			ranks[count].mOrder  = mScores[i].mOrder;
			count++;
		}
	}
	if (count == 0) {
		return;
	}
	if (count == 1) {
		((s8*)&mPlayers[ranks[0].mPlayer])[8] = place;
		return;
	}
	survivalRankSort(ranks, count);
	((s8*)&mPlayers[ranks[0].mPlayer])[8] = place;
	int i;
	int given = place;
	for (i = 1; i < count; i++) {
		if (ranks[i].mOrder != ranks[i - 1].mOrder) {
			given = i + 1;
		}
		mPlayers[ranks[i].mPlayer].mPlace = given;
	}
}

#include "stage/TGame.h"
#include "ui/AeObjControl.h"
#include "sound/SoundCtrl.h"
#include "stage/TGameStateSurvival.h"

typedef char TGameStateSurvival_size_check[(sizeof(TGameStateSurvival) == 0x14) ? 1 : -1];

// 0x8019F4F0
extern "C" void* createGameStateSurvival(void* pool, TGameContextBattle*)
{
	return new (pool) TGameStateSurvival;
}

// 0x8019F52C
TGameStateSurvival::TGameStateSurvival()
{
	mName = gTGameStateSurvivalName; // "TGameStateSurvival"
	mSize = 0x14;
}

// 0x8019F578
int TGameStateSurvival::onPlayerOut(TGameContextBattle* context, BattleMsg* msg)
{
	int index = battleFindPlayer(context, msg->mPlayer);
	if (index == -1) {
		return 0;
	}

	int* entry = (int*)((u8*)context + index * 8 + 0x9C);
	if (entry[0] == 0) {
		return 0;
	}

	entry[0]--;
	if (entry[0] == 0) {
		entry[1] = context->mTime;
	}

	LivesMsg lives;
	lives.mType   = 0xB1;
	lives._04     = msg->mPlayer;
	lives.mLives  = entry[0];
	lives.mSlot   = context->mPlayers[index]._00;
	aeSendMessage(&lives);

	battlePlayerOut(context, msg->mPlayer, msg->mOther, entry[0] != 0);
	return 1;
}

// 0x8019F650
void TGameStateSurvival::run(TGameContextBattle* context)
{
	int result = ((TGameBattleSurvival*)context)->checkEnd();
	if (result != 2) {
		sePlay(0x34);
		aeBattleNothing();
		if (result == 0) {
			context->endOnResult0();
		} else {
			context->endOnResult1();
		}
	}
}

// 0x8019F6B4
TGameStateSurvival::~TGameStateSurvival()
{
}

// 0x8019F72C
int TGameStateSurvival::onMessage(TGameContextBattle* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA8:
		return onPlayerOut(context, (BattleMsg*)msg);
	}
	return TGameStateBattle::onMessage(context, msg);
}

// 0x8019F764
// The class is in TGame.h and its other code is in TGameBattleSurvival.cpp
TGameBattleSurvival::~TGameBattleSurvival() { }
