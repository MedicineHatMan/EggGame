#include "stage/TGame.h"
#include "stage/TGameBattleHatch.h"
#include "egg/TEggCreature.h"
#include "ui/AeObjControl.h"
#include "sound/SoundCtrl.h"
#include "stage/TGameStateHatch.h"

/**
 * @size 0x8
 */
struct HatchLeaderMsg {
	s16 mKind; // _00
	u16 _02;
	u16 mPlayer; // _04
	u16 _06;
};

// And who scored with up to 0x18 bytes in all
struct HatchScoreMsg {
	s16 mKind; // _00
	u16 _02;
	u16 mPlayer; // _04
	u16 _06;
	int mPoints; // _08
	u8 _0C[4];
};

struct HatchRank {
	int mPlayer; // _00
	int mScore;  // _04
};

const char* gTGameBattleHatchName = "TGameBattleHatch";

// 0x801CE2B0
TGameBattleHatch::TGameBattleHatch(TObject* parent)
    : TGameContextBattle(parent)
{
	mGoal    = 9;
	mChanged = 0;
	_00      = (u32)gTGameBattleHatchName;
	mSize    = 0xB0;
}

// 0x801CE30C
extern "C" void* createGameBattleHatch()
{
	return new TGameBattleHatch(0);
}

// 0x801CE34C
void TGameBattleHatch::setGoal(int setting)
{
	switch (setting) {
	case 0:
		mGoal = 9;
		break;
	case 1:
		mGoal = 0xF;
		break;
	case 2:
		mGoal = 0x1E;
		break;
	}
}

// 0x801CE394
void TGameBattleHatch::startState()
{
	for (int i = 0; i < mPlayerCount; i++) {
		mScores[i] = 0;
	}
	SET_BATTLE_STATE(this, createGameStateBattleStart);
}

// 0x801CE414
void TGameBattleHatch::start()
{
	SET_BATTLE_STATE(this, createGameStateHatch);
}

// 0x801CE468
extern "C" int hatchRankBehind(HatchRank* a, HatchRank* b)
{
	return a->mScore < b->mScore;
}

// 0x801CE488
extern "C" void hatchRankSort(HatchRank* ranks, int count)
{
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count - 1; j++) {
			if (hatchRankBehind(&ranks[j], &ranks[j + 1])) {
				int player           = ranks[j].mPlayer;
				int score            = ranks[j].mScore;
				ranks[j]             = ranks[j + 1];
				ranks[j + 1].mPlayer = player;
				ranks[j + 1].mScore  = score;
			}
		}
	}
}

// 0x801CE528
static inline int hatchCount(TGameBattleHatch* game)
{
	return game->mPlayerCount;
}

int TGameBattleHatch::checkEnd()
{
	int leaders;
	int i;
	int place;
	int count = hatchCount(this);
	HatchRank ranks[4];
	for (i = 0; i < count; i++) {
		ranks[i].mPlayer = i;
		ranks[i].mScore  = mScores[i];
	}
	hatchRankSort(ranks, count);

	place                                           = 1;
	leaders                                         = 1;
	*((u8*)this + (ranks[0].mPlayer * 0x18 + 0x38)) = 1;
	for (i = 1; i < count; i++) {
		if (hatchRankBehind(&ranks[i], &ranks[i - 1])) {
			place = i + 1;
		}
		mPlayers[ranks[i].mPlayer].mPlace = place;
		if (place == 1) {
			leaders++;
		}
	}

	if (leaders == 1 && mChanged != 0) {
		HatchLeaderMsg leader;
		leader.mKind   = 0xC5;
		leader.mPlayer = mPlayers[ranks[0].mPlayer].mId;
		aeSendMessage(&leader);
	}
	mChanged = 0;
	if (ranks[0].mScore >= mGoal) {
		return leaders == 1 ? 0 : 1;
	}
	return 2;
}

// 0x801CE680
void TGameBattleHatch::addScore(u16 player, int points)
{
	int index = battleFindPlayer(this, player);
	if (index != -1) {
		index = index * 4 + 0xA0;
		*(int*)((u8*)this + index) += points;
		if (*(int*)((u8*)this + index) < 0) {
			*(int*)((u8*)this + index) = 0;
		}
		HatchScoreMsg msg;
		msg.mKind   = 0xC6;
		msg.mPlayer = player;
		msg.mPoints = points;
		aeSendMessage(&msg);
		mChanged = 1;
	}
}

typedef char TGameStateHatch_size_check[(sizeof(TGameStateHatch) == 0x14) ? 1 : -1];

const char* gTGameStateHatchName = "TGameStateHatch"; // 0x802C48AC

// 0x801CE704
extern "C" void* createGameStateHatch(void* pool, TGameContextBattle*)
{
	return new (pool) TGameStateHatch;
}

// 0x801CE740
TGameStateHatch::TGameStateHatch()
{
	mName = gTGameStateHatchName; // "TGameStateHatch"
	mSize = 0x14;
}

// 0x801CE78C
int TGameStateHatch::onPlayerOut(TGameContextBattle* context, BattleMsg* msg)
{
	if (battleFindPlayer(context, msg->mOther) < 0 || msg->mOther == msg->mPlayer) {
		((TGameBattleHatch*)context)->addScore(msg->mPlayer, -2);
	} else {
		int ofs = battleFindPlayer(context, msg->mPlayer) * 4 + 0xA0;
		((TGameBattleHatch*)context)->addScore(msg->mOther, *(int*)((u8*)context + ofs));
		((TGameBattleHatch*)context)->addScore(msg->mPlayer, -*(int*)((u8*)context + ofs));
	}
	battlePlayerOut(context, msg->mPlayer, msg->mOther, 1);
	return 1;
}

// 0x801CE83C
int TGameStateHatch::onHit(TGameContextBattle* context, HatchMsg* msg)
{
	TEggCreature* object = TEggCreature::find(msg->mObject);
	if (object == 0) {
		return 0;
	}

	int points;
	if (msg->mKind4 != 0) {
		points = 4;
	} else if (msg->mKind5 != 0) {
		points = 5;
	} else {
		points = 3;
	}

	if (msg->mPlayer == object->mEggId) {
		((TGameBattleHatch*)context)->addScore(msg->mPlayer, points);
	} else {
		((TGameBattleHatch*)context)->addScore(msg->mPlayer, points - 1);
	}
	return 1;
}

// 0x801CE8D8
void TGameStateHatch::run(TGameContextBattle* context)
{
	int result = ((TGameBattleHatch*)context)->checkEnd();
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

// 0x801CE93C
TGameStateHatch::~TGameStateHatch()
{
}

// 0x801CE9B4
int TGameStateHatch::onMessage(TGameContextBattle* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA8:
		return onPlayerOut(context, (BattleMsg*)msg);
	case 0xA1:
		return onHit(context, (HatchMsg*)msg);
	}
	return TGameStateBattle::onMessage(context, msg);
}

// ~TGameBattleHatch 0x801CEA00 is inline in the class so it comes out at the end of the file with the vtables
