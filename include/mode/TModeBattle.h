#ifndef _TMODEBATTLE_H
#define _TMODEBATTLE_H

#include "stage/Stage.h"

class TStageBattle;
class TGameContextBattle;
class TBattleResult;

/** The states of TModeBattle mState picks the handler */
enum BattleModeState {
	BATTLE_START  = 0, // waits for the first update then plays
	BATTLE_QUIT   = 1, // ends the mode with _08 = 0
	BATTLE_PLAY   = 2,
	BATTLE_PAUSE  = 3,
	BATTLE_RESULT = 4,
	BATTLE_NONE   = 5,
};

/** What a state handler is called for */
enum BattleModeEvent {
	BATTLE_ENTER  = 0,
	BATTLE_EXIT   = 1,
	BATTLE_DRAW   = 2,
	BATTLE_UPDATE = 3,
};

/**
 * One player slot of the battle. The constructor is empty; TModeBattle clears the fields itself.
 * @size 0x30
 */
struct BattleEntry {
	BattleEntry(); // 0x8002EB14

	int mActive; // _00 the slot has a fighter
	int mHuman;  // _04 zero for a computer player
	int mWins;   // _08 rounds won kept by TGameFlow across battles
	int mChara;  // _0C the fighter
	u8 mPlace;   // _10 from the round's BattlePlayer
	int mPoints; // _14
	int mLost;   // _18
	int _1C;     // _1C
	Vec3f mPos;  // _20 the spawn point
	int mAngle;  // _2C the spawn angle in 0x10000ths of a turn
};

/**
 * The battle itself: loads the battle stage, spawns the four slots and runs the rule's game context. Vtable
 * 0x80254E40. TGameFlow::sceneBattleStage builds it. The class name is ours.
 *
 * @size 0xF8
 */
class TModeBattle : public TMode {
public:
	TModeBattle();
	static TModeBattle* create(TModeAllocator* allocator); // 0x8002EA0C

	virtual ~TModeBattle();
	virtual void loadFiles();
	virtual int init();
	virtual int shutdown();
	virtual void update();

	void setChara(int slot, int chara); // 0x8002EB40

	void callState(int state, int event); // 0x8002F040
	void setState(int state);             // 0x8002F0C0
	void startRound(int sudden);          // 0x8002F120 sudden death when nonzero
	void endRound();                      // 0x8002F360
	void readResults();                   // 0x8002F3D0
	void resetEntries();                  // 0x8002F45C
	void drawPlay();                      // 0x8002F494
	void drawPause();                     // 0x8002F518
	void countWins();                     // 0x8002F56C
	int checkPause();                     // 0x8002F5AC

	void stateStart(int event);  // 0x8002F650
	void stateQuit(int event);   // 0x8002F69C
	void statePlay(int event);   // 0x8002F6B0
	void statePause(int event);  // 0x8002F7CC
	void stateResult(int event); // 0x8002F8D0

	s8 mState;                 // _0C a BattleModeState
	int mRule;                 // _10 0 survival 1 time 2 hatch
	int mLives;                // _14 the survival setting
	int mTime;                 // _18 the time setting
	int mGoal;                 // _1C the hatch setting
	TStageBattle* mStage;      // _20
	const char* mName;         // _24 the stage
	int mSkipDraw;             // _28 set when the last frame ran long
	BattleEntry mEntries[4];   // _2C
	TGameContextBattle* mGame; // _EC
	TPauseMenu* mPause;        // _F0
	TBattleResult* mResult;    // _F4
};

#include "Dolphin/GX/GXTypes.h"
extern "C" {
extern GXColor sModeBattleClearColor;
}

#endif
