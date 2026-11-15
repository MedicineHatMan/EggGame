#include "stage/TGame.h"
#include "ui/AeObjControl.h"
#include "sound/SoundCtrl.h"
#include "stage/TGameSuddenDeath.h"

/**
 * @size 0x8
 */
struct SuddenRank {
	int mPlayer; // _00
	int mOrder;  // _04
};

const char* gTGameSuddenDeathName      = "TGameSuddenDeath";
const char* gTGameStateSuddenDeathName = "TGameStateSuddenDeath";

// 0x801AB9B4
TGameSuddenDeath::TGameSuddenDeath(TObject* parent)
    : TGameContextBattle(parent)
{
	_00   = (u32)gTGameSuddenDeathName;
	mSize = 0xB8;
}

// 0x801ABA00
extern "C" void* createGameSuddenDeath()
{
	return new TGameSuddenDeath(0);
}

// 0x801ABA40
void TGameSuddenDeath::startState()
{
	for (int i = 0; i < mPlayerCount; i++) {
		mScores[i].mOut = 0;
	}
	SET_BATTLE_STATE(this, createGameStateSuddenDeath);
}

// 0x801ABAC0
int TGameSuddenDeath::checkEnd()
{
	int i;
	int alive = 0;
	int last  = -1;
	for (i = 0; i < mPlayerCount; i++) {
		if (mScores[i].mOut == 0) {
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
		*((s8*)this + (last * 0x18 + 0x38)) = 1;
		rankOut(2);
		return 0;
	}
	rankOut(alive + 1);
	return 2;
}

// 0x801ABB9C
extern "C" void suddenRankSort(SuddenRank* ranks, int count)
{
	int player;
	SuddenRank* cur;
	int order;
	int j;
	int i;
	for (i = 0; i < count; i++) {
		for (j = 0; j < count - 1; j++) {
			cur   = &ranks[j];
			order = cur->mOrder;
			if (order > cur[1].mOrder) {
				player         = cur->mPlayer;
				cur->mPlayer   = cur[1].mPlayer;
				cur->mOrder    = cur[1].mOrder;
				cur[1].mPlayer = player;
				cur[1].mOrder  = order;
			}
		}
	}
}

// 0x801ABC00
// Later ones get a place by their spot in the list as TGameBattleSurvival does
void TGameSuddenDeath::rankOut(int place)
{
	SuddenRank ranks[4];
	int count = 0;
	for (int i = 0; i < mPlayerCount; i++) {
		if (mScores[i].mOut != 0 && mPlayers[i].mPlace == 0) {
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
	suddenRankSort(ranks, count);
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

typedef char TGameStateSuddenDeath_size_check[(sizeof(TGameStateSuddenDeath) == 0x18) ? 1 : -1];

// 0x801ABD2C
extern "C" void* createGameStateSuddenDeath(void* pool, TGameContextBattle*)
{
	return new (pool) TGameStateSuddenDeath;
}

// 0x801ABD68
TGameStateSuddenDeath::TGameStateSuddenDeath()
{
	mName = gTGameStateSuddenDeathName; // "TGameStateSuddenDeath"
	mSize = 0x18;
	mStep = 0;
}

// 0x801ABDBC
int TGameStateSuddenDeath::onPlayerOut(TGameContextBattle* context, BattleMsg* msg)
{
	int index = battleFindPlayer(context, msg->mPlayer);
	if (index == -1) {
		return 0;
	}

	int* entry = (int*)((u8*)context + index * 8 + 0x98);
	entry[1]   = context->mTime;
	entry[0]   = 1;
	battlePlayerOut(context, msg->mPlayer, msg->mOther, 0);
	return 1;
}

// 0x801ABE38
void TGameStateSuddenDeath::run(TGameContextBattle* context)
{
	switch (mStep) {
	case 0:
		mTimer = 60;
		mStep  = 1;
		sePlay(0x36);
		aeBattleSuddenDeath();
	case 1:
		if (--mTimer == 0) {
			mStep = 2;
		}
		break;
	case 2:
		int result = ((TGameSuddenDeath*)context)->checkEnd();
		if (result != 2) {
			sePlay(0x34);
			aeBattleNothing();
			if (result == 0) {
				context->endOnResult0();
			} else if (result == 1) {
				context->endOnResult1();
			}
		}
		break;
	}
}

// 0x801ABF10
TGameStateSuddenDeath::~TGameStateSuddenDeath()
{
}

// 0x801ABF88
int TGameStateSuddenDeath::onMessage(TGameContextBattle* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA8:
		return onPlayerOut(context, (BattleMsg*)msg);
	}
	return TGameStateBattle::onMessage(context, msg);
}
