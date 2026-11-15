#ifndef _TGAME_H
#define _TGAME_H

#include "enemy/boss/EneBoss.h"
#include "stage/GameState.h"

extern "C" {
void aeSendMissionIcon(void*);
void aeSendMessage(void*);
int battleFindPlayer(TGameContextBattle*, u16);
}

// Shows a number on the screen
struct GameCountMsg {
	s16 mKind;  // _00
	int mCount; // _04
};

inline void gameShowCount(const u16& count)
{
	GameCountMsg msg;
	msg.mKind  = 0xBE;
	msg.mCount = count;
	aeSendMissionIcon(&msg);
}

inline void gameShowCount(const int& count)
{
	GameCountMsg msg;
	msg.mKind  = 0xBE;
	msg.mCount = count;
	aeSendMissionIcon(&msg);
}

#define SET_BATTLE_STATE(context, create) \
	do { \
		void* pool = (context)->mPool; \
		releasePlyState(pool, changeBattleState(&(context)->mStateSlot, (context), create(pool, (context)))); \
	} while (0)

/**
 * @size 0x38
 */
class TGameFreePlay : public TGameContext {
public:
	TGameFreePlay(TObject* parent);
	virtual ~TGameFreePlay();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();
};

typedef char TGameFreePlay_size_check[(sizeof(TGameFreePlay) == 0x38) ? 1 : -1];

/**
 * @size 0x3C
 */
class TGameDino : public TGameContext {
public:
	TGameDino(TObject* parent);
	virtual ~TGameDino();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	int hatched(); // 0x800CDC20 true when none are left

	int mLeft; // _38
};

typedef char TGameDino_size_check[(sizeof(TGameDino) == 0x3C) ? 1 : -1];

/**
 * @size 0x3C
 */
class TGameGoldenEgg : public TGameContext {
public:
	TGameGoldenEgg(TObject* parent);
	virtual ~TGameGoldenEgg();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	int mDone; // _38
};

typedef char TGameGoldenEgg_size_check[(sizeof(TGameGoldenEgg) == 0x3C) ? 1 : -1];

// Message 0xAC which starts a mini game the script and switch for the result the time and the score to reach
struct MiniGameRulesMsg : public ObjMsg {
	u8 _02[2];   // _02
	u16 mScript; // _04 run when the time is up
	u16 mSwitch; // _06 set to whether the score was reached
	u8 mMinutes; // _08
	u8 mSeconds; // _09
	u16 mNeeded; // _0A
};

/**
 * The mini games. Code 0x8010DEF0-0x8010E35C, vtable 0x8025D644. The class name is ours; its constructor stores no name.
 * Points come in through addPoints. A hit within 60 frames of the last one is a combo and adds a second to the clock.
 *
 * @size 0x5C
 */
class TGameMiniGame : public TGameContext {
public:
	TGameMiniGame(TObject* parent);
	virtual ~TGameMiniGame();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	void readRules(MiniGameRulesMsg* msg); // 0x8010E09C
	int tickClock();                       // 0x8010E0F8 true when the time is up
	void addPoints(int points);            // 0x8010E1B0
	int reportResult();                    // 0x8010E2F4 true once the result script runs

	u16 mScript;    // _38
	u16 mSwitch;    // _3A
	int mDone;      // _3C set wait for the clear instead
	u32 mClock;     // _40 milliseconds left
	int mBonus;     // _44 seconds the last hit added
	int mScore;     // _48
	int mComboTime; // _4C frames left for the next hit to be a combo
	int mCombo;     // _50
	int mBestCombo; // _54
	int mNeeded;    // _58
};

typedef char TGameMiniGame_size_check[(sizeof(TGameMiniGame) == 0x5C) ? 1 : -1];

/**
 * @size 0x3C
 */
class TGameRescue : public TGameContext {
public:
	TGameRescue(TObject* parent);
	virtual ~TGameRescue();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	s8 mPhase; // _38
};

typedef char TGameRescue_size_check[(sizeof(TGameRescue) == 0x3C) ? 1 : -1];

/**
 * @size 0x3C
 */
class TGameCollect : public TGameContext {
public:
	TGameCollect(TObject* parent);
	virtual ~TGameCollect();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	void collected(); // 0x80113C10
	int isDone();     // 0x80113C58

	u16 mTotal; // _38
	u16 mLeft;  // _3A
};

typedef char TGameCollect_size_check[(sizeof(TGameCollect) == 0x3C) ? 1 : -1];

/**
 * @size 0x3C
 */
class TGameBoss : public TGameContext {
public:
	TGameBoss(TObject* parent);
	virtual ~TGameBoss();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	int mBeaten; // _38
};

typedef char TGameBoss_size_check[(sizeof(TGameBoss) == 0x3C) ? 1 : -1];

/**
 * @size 0x3C
 */
class TGameSnowman : public TGameContext {
public:
	TGameSnowman(TObject* parent);
	virtual ~TGameSnowman();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	int mDone; // _38
};

typedef char TGameSnowman_size_check[(sizeof(TGameSnowman) == 0x3C) ? 1 : -1];

/**
 * @size 0x38
 */
class TGameGoal : public TGameContext {
public:
	TGameGoal(TObject* parent);
	virtual ~TGameGoal();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();
};

typedef char TGameGoal_size_check[(sizeof(TGameGoal) == 0x38) ? 1 : -1];

/**
 * @size 0x3C
 */
class TGameBattery : public TGameContext {
public:
	TGameBattery(TObject* parent);
	virtual ~TGameBattery();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	int lit(); // 0x8013B4C8 true when none are left

	int mLeft; // _38
};

typedef char TGameBattery_size_check[(sizeof(TGameBattery) == 0x3C) ? 1 : -1];

/**
 * @size 0x3C
 */
class TGameFireworks : public TGameContext {
public:
	TGameFireworks(TObject* parent);
	virtual ~TGameFireworks();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	int mDone; // _38
};

typedef char TGameFireworks_size_check[(sizeof(TGameFireworks) == 0x3C) ? 1 : -1];

/**
 * @size 0x40
 */
class TGameGear : public TGameContext {
public:
	TGameGear(TObject* parent);
	virtual ~TGameGear();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	int turned(); // 0x8013BC44 true when none are left

	int mDone; // _38
	int mLeft; // _3C
};

typedef char TGameGear_size_check[(sizeof(TGameGear) == 0x40) ? 1 : -1];

/**
 * @size 0x40
 */
class TGameWipeOut : public TGameContext {
public:
	TGameWipeOut(TObject* parent);
	virtual ~TGameWipeOut();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	void beaten(); // 0x8015F3D0
	int isDone();  // 0x8015F418

	int mTotal; // _38
	int mLeft;  // _3C
};

typedef char TGameWipeOut_size_check[(sizeof(TGameWipeOut) == 0x40) ? 1 : -1];

/**
 * @size 0x38
 */
class TGameLastBoss : public TGameContext {
public:
	TGameLastBoss(TObject* parent);
	virtual ~TGameLastBoss();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();
};

typedef char TGameLastBoss_size_check[(sizeof(TGameLastBoss) == 0x38) ? 1 : -1];

/**
 * @size 0x3C
 */
class TGameRace : public TGameContext {
public:
	TGameRace(TObject* parent);
	virtual ~TGameRace();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	int mDone; // _38
};

typedef char TGameRace_size_check[(sizeof(TGameRace) == 0x3C) ? 1 : -1];

/**
 * @size 0x38
 */
class TGameBossAfter : public TGameContext {
public:
	TGameBossAfter(TObject* parent);
	virtual ~TGameBossAfter() { } // 0x801AC2F0

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();
};

typedef char TGameBossAfter_size_check[(sizeof(TGameBossAfter) == 0x38) ? 1 : -1];

/**
 * @size 0x8
 */
struct SurvivalScore {
	int mLives; // _00
	int mOrder; // _04 when they went out
};

/**
 * The last one standing, with a set number of lives each.
 *
 * @size 0xBC
 */
class TGameBattleSurvival : public TGameContextBattle {
public:
	TGameBattleSurvival(TObject* parent);
	virtual void startState();
	virtual ~TGameBattleSurvival();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void start();

	void setLives(int setting); // 0x8019F110
	int checkEnd();             // 0x8019F284 0 one left 1 none left 2 still going
	void rankOut(int place);    // 0x8019F3C4

	u8 mStartLives;              // _98
	SurvivalScore mScores[4];    // _9C
};

typedef char TGameBattleSurvival_size_check[(sizeof(TGameBattleSurvival) == 0xBC) ? 1 : -1];

/**
 * Most points when the clock runs out.
 *
 * @size 0xA4
 */
class TGameBattleTime : public TGameContextBattle {
public:
	TGameBattleTime(TObject* parent);
	virtual ~TGameBattleTime() { } // 0x801AB958

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();
	virtual void start();

	int update(); // 0x801AB4EC 0 one winner 1 a tie 2 still going

	int mTimeLeft;   // _98 milliseconds
	int mSetting;    // _9C 0 is 1 minute 1 is 3 2 is 5
	u8 mLastSecond;  // _A0
};

typedef char TGameBattleTime_size_check[(sizeof(TGameBattleTime) == 0xA4) ? 1 : -1];

/**
 * @size 0x8
 */
struct SuddenScore {
	int mOut;   // _00
	int mOrder; // _04
};

/**
 * The tie break after a battle. One hit puts a player out.
 *
 * @size 0xB8
 */
class TGameSuddenDeath : public TGameContextBattle {
public:
	TGameSuddenDeath(TObject* parent);
	virtual ~TGameSuddenDeath() { } // 0x801ABFC0

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();

	int checkEnd();          // 0x801ABAC0 0 one left 1 none left 2 still going
	void rankOut(int place); // 0x801ABC00

	SuddenScore mScores[4]; // _98
};

typedef char TGameSuddenDeath_size_check[(sizeof(TGameSuddenDeath) == 0xB8) ? 1 : -1];

/**
 * First to a set score.
 *
 * @size 0xB0
 */
class TGameBattleHatch : public TGameContextBattle {
public:
	TGameBattleHatch(TObject* parent);
	// Inline so startState is the key function and the vtable follows TGameStateHatch's
	virtual ~TGameBattleHatch() { }

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void startState();
	virtual void start();

	void setGoal(int setting);            // 0x801CE34C
	int checkEnd();                       // 0x801CE528 0 one winner 1 a tie 2 still going
	void addScore(u16 player, int points); // 0x801CE680

	int mGoal;      // _98
	int mChanged;   // _9C the scores moved since the last check
	int mScores[4]; // _A0
};

typedef char TGameBattleHatch_size_check[(sizeof(TGameBattleHatch) == 0xB0) ? 1 : -1];

#endif
