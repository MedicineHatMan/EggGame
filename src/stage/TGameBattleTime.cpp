#include "stage/TGame.h"
#include "ui/AeObjControl.h"
#include "stage/StageTime.h"
#include "sound/SoundCtrl.h"
#include "stage/TEventCtrl.h"
#include "stage/TGameBattleTime.h"
struct StageTime;

/**
 * @size 0xC
 */
struct TimeRank {
	int mPlayer; // _00
	int mPoints; // _04
	int mLost;   // _08
};

/**
 * @size 0x8
 */
struct GameClock {
	u8 mHours;   // _00
	u8 mMinutes; // _01
	u8 mSeconds; // _02
	u16 mMillis; // _04
};

const char* gTGameBattleTimeName = "TGameBattleTime";

// 0x801AB26C
TGameBattleTime::TGameBattleTime(TObject* parent)
    : TGameContextBattle(parent)
{
	mTimeLeft = 0;
	mSetting  = 0;
	_00       = (u32)gTGameBattleTimeName;
	mSize     = 0xA4;
}

// 0x801AB2C4
extern "C" void* createGameBattleTime()
{
	return new TGameBattleTime(0);
}

// 0x801AB304
void TGameBattleTime::startState()
{
	u8 minutes = 1;
	switch (mSetting) {
	case 0:
		minutes = 1;
		break;
	case 1:
		minutes = 3;
		break;
	case 2:
		minutes = 5;
		break;
	}
	mTimeLeft   = (u8)minutes * 60000;
	mLastSecond = 0;
	SET_BATTLE_STATE(this, createGameStateBattleStart);
}

// 0x801AB3B4
void TGameBattleTime::start()
{
	SET_BATTLE_STATE(this, createGameStateTime);
}

// 0x801AB408
extern "C" int timeRankBehind(TimeRank* a, TimeRank* b)
{
	u8 behind  = 0;
	int points = a->mPoints;
	if (points < b->mPoints || (points == b->mPoints && a->mLost > b->mLost)) {
		behind = 1;
	}
	return behind;
}

// 0x801AB43C
extern "C" void timeRankSort(TimeRank* ranks, int count)
{
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count - 1; j++) {
			if (timeRankBehind(&ranks[j], &ranks[j + 1])) {
				int player           = ranks[j].mPlayer;
				int points           = ranks[j].mPoints;
				int lost             = ranks[j].mLost;
				ranks[j]             = ranks[j + 1];
				ranks[j + 1].mPlayer = player;
				ranks[j + 1].mPoints = points;
				ranks[j + 1].mLost   = lost;
			}
		}
	}
}

static inline int timeCount(TGameBattleTime* game)
{
	return game->mPlayerCount;
}

// 0x801AB4EC
int TGameBattleTime::update()
{
	s8* slot;
	TimeRank* rank;
	int i;
	int winners;
	int place;
	int leaders;
	TimeRank ranks[4];
	struct {
		s16 mKind;
		int mMinutes;
		int mSeconds;
	} left;
	GameClock clock;
	clock.mHours   = 0;
	clock.mMinutes = 0;
	clock.mSeconds = 0;
	clock.mMillis  = 0;
	GameClock shown;
	shown.mHours   = 0;
	shown.mMinutes = 0;
	shown.mSeconds = 0;
	shown.mMillis  = 0;
	stageTimeSet((StageTime*)&clock, mTimeLeft);
	shown.mSeconds = 1;
	stageTimeAdd((StageTime*)&shown, (const StageTime*)&clock);

	if (mTimeLeft == 0) {
		winners = 0;
		for (int i = 0; i < mPlayerCount; i++) {
			if (mPlayers[i].mPlace == 1) {
				winners++;
			}
		}
		sePlay(0x38);
		sePlay(0x35);
		aeBattleTimeUp();
		return winners == 1 ? 0 : 1;
	}

	if (eventIsBusy() == 0) {
		stageTimerTick(&mTimeLeft);
	}

	int count = timeCount(this);
	for (i = 0; i < count; i++) {
		slot          = &mPlayers[i].mPlace;
		rank          = &ranks[i];
		rank->mPlayer = i;
		rank->mPoints = *(int*)((u8*)slot + 4);
		rank->mLost   = *(int*)((u8*)slot + 8);
	}
	timeRankSort(ranks, count);

	place                                           = 1;
	leaders                                         = 1;
	*((u8*)this + (ranks[0].mPlayer * 0x18 + 0x38)) = 1;
	for (i = 1; i < count; i++) {
		if (timeRankBehind(&ranks[i], &ranks[i - 1])) {
			place = i + 1;
		}
		mPlayers[ranks[i].mPlayer].mPlace = place;
		if (place == 1) {
			leaders++;
		}
	}

	if (mLastSecond != clock.mSeconds) {
		if (leaders == 1 && ((clock.mMinutes != 0 && clock.mSeconds == 0) || clock.mSeconds == 0x1E)) {
			struct {
				s16 mKind;
				u16 _02;
				u16 mPlayer;
				u16 _06;
			} leader;
			leader.mKind   = 0xC5;
			leader.mPlayer = mPlayers[ranks[0].mPlayer].mId;
			aeSendMessage(&leader);
		}
		if (shown.mMinutes != 0 || shown.mSeconds >= 4) {
			left.mKind    = 0xC4;
			left.mMinutes = shown.mMinutes;
			left.mSeconds = shown.mSeconds;
			aeSendMessage(&left);
		} else if (shown.mSeconds == 1) {
			sePlay(0x39);
			aeBattleCount1();
		} else if (shown.mSeconds == 2) {
			sePlay(0x39);
			aeBattleCount2();
		} else if (shown.mSeconds == 3) {
			sePlay(0x39);
			aeBattleCount3();
		}
	}
	mLastSecond = clock.mSeconds;
	return 2;
}

typedef char TGameStateTime_size_check[(sizeof(TGameStateTime) == 0x14) ? 1 : -1];

// 0x801AB798
extern "C" void* createGameStateTime(void* pool, TGameContextBattle*)
{
	return new (pool) TGameStateTime;
}

// 0x801AB7D4
TGameStateTime::TGameStateTime()
{
	mName = gTGameStateTimeName; // "TGameStateTime"
	mSize = 0x14;
}

// 0x801AB820
int TGameStateTime::onPlayerOut(TGameContextBattle* context, BattleMsg* msg)
{
	battlePlayerOut(context, msg->mPlayer, msg->mOther, 1);
	return 1;
}

// 0x801AB854
void TGameStateTime::run(TGameContextBattle* context)
{
	int result = ((TGameBattleTime*)context)->update();
	if (result != 2) {
		if (result == 0) {
			context->endOnResult0();
		} else {
			context->endOnResult1();
		}
	}
}

// 0x801AB8A8
TGameStateTime::~TGameStateTime()
{
}

// 0x801AB920
int TGameStateTime::onMessage(TGameContextBattle* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA8:
		return onPlayerOut(context, (BattleMsg*)msg);
	}
	return TGameStateBattle::onMessage(context, msg);
}
