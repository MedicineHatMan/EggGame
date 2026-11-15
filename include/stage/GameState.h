#ifndef _GAMESTATE_H
#define _GAMESTATE_H

#include "player/PlyState.h"

class TGameContextBattle;

#include "stage/GameStateRoot.h"

/**
 * @size 0x14
 */
class TGameState : public TGameStateRoot {
public:
	TGameState();
	virtual ~TGameState() { }                                      // 0x80042A94
	virtual void run(TGameContext* context) { update(context); }   // _14 0x80042A58
	virtual void vfn_18(TGameContext* context) { }                 // _18 0x80042A50
	virtual void update(TGameContext* context) { }                 // _1C 0x80042A54
	virtual int onMessage(TGameContext* context, ObjMsg* msg);     // _20 1 = handled
};

/**
 * @size 0x14
 */
class TGameStateBattle : public TBattleStateRoot {
public:
	TGameStateBattle();
	virtual ~TGameStateBattle();                                       // 0x8019EC44
	virtual void vfn_18(TGameContextBattle* context) { }               // _18 0x8019EF7C
	virtual int onMessage(TGameContextBattle* context, ObjMsg* msg);   // _1C
};

#pragma push
#pragma sym off
inline TGameStateBattle::~TGameStateBattle() { }
#pragma pop

/**
 * A player's place in a battle.
 *
 * @size 0x18
 */
struct BattlePlayer {
	u8 _00;      // _00 passed along in the 0xB1 message
	u8 _01;      // _01
	u16 mId;     // _02
	u16 mTimer;  // _04 frames until the 0x2B message tells the player they can be hit again
	u8 _06[2];   // _06
	s8 mPlace;   // _08 0 until it is decided
	u8 _09[3];   // _09
	int mPoints; // _0C
	int mLost;   // _10
	int mHatched; // _14 eggs hatched
};

/** The battle context's current state, like GameStateSlot. The name is ours. @size 0x4 */
struct BattleStateSlot {
	BattleStateSlot();  // 0x8019EF84
	~BattleStateSlot(); // 0x8019EF90
	void update(TGameContextBattle* context); // 0x8019EFCC

	TGameStateBattle* mState; // _00
};

extern "C" {
int battleDefaultMessage(TGameContextBattle* context, ObjMsg* msg);                  // 0x8019E838
void battleAddPlayer(TGameContextBattle* context, u16 id, u8 chara);                 // 0x8019E9C8
int battleFindPlayer(TGameContextBattle* context, u16 id);                           // 0x8019E9E8 -1 if none
int battlePlayerOut(TGameContextBattle* context, u16 player, u16 other, int guard); // 0x8019EA30
}

/**
 * The battle modes' rules: up to four players, each with points, losses and hatches, and a state.
 *
 * @size 0x98
 */
class TGameContextBattle : public TCustomGameContext {
public:
	TGameContextBattle(TObject* parent); // 0x8019E6D8
	virtual ~TGameContextBattle();    // 0x8019E7AC
	virtual void draw();                       // _0C 0x8019E914
	virtual int onMessage(ObjMsg* msg);        // _20 0x8019E870
	virtual int getStatus();                   // _24 0x8019EAE0
	virtual void startState();        // _28 0x8002FAA4 inline at the end of TModeBattle.cpp
	virtual void start() { }          // _2C 0x8019EF80

	void endOnResult0(); // 0x8019EB58 with kind 5
	void endOnResult1(); // 0x8019EAFC with kind 6

	int mPlayerCount; // _20
	int mTime;       // _24
	int mEndKind;    // _28
	FlagByte mDone;  // _2C bit 0 = finished
	u8 _2D[3];
	BattlePlayer mPlayers[4]; // _30
	void* mPool;     // _90
	BattleStateSlot mStateSlot; // _94
};

// What the handlers read from their messages
struct BattleMsg : public ObjMsg {
	u8 _02[2];
	u8 mPlayer; // _04
	u8 mOther;  // _05 the other player or not one at all
};

struct HatchMsg : public ObjMsg {
	u8 _02[2];
	u16 mPlayer; // _04
	u16 mObject; // _06
	int mKind4;    // _08
	int mKind5;   // _0C
};

struct RescueMsg : public ObjMsg {
	u8 _02[2];
	u32 mLimit;  // _04
	u32 mTime;   // _08
};

struct AddTimeMsg : public ObjMsg {
	u8 _02[2];
	s16 mMinute; // _04
	s16 mSecond; // _06
};

struct LivesMsg : public ObjMsg {
	u8 _02[2];
	u16 _04;    // _04
	u16 mLives; // _06
	u16 mSlot;  // _08
};

struct HudMsg : public ObjMsg {
	u8 _02[2];
	int mValue; // _04
};

struct HudRatioMsg : public ObjMsg {
	u8 _02[2];
	f32 mRatio; // _04
};

struct HudTimeMsg : public ObjMsg {
	u8 _02[2];
	int mMinute; // _04
	int mSecond; // _08
	int mMilli;  // _0C
};

// 0x800CD974
class TGameStateFreePlay : public TGameState {
public:
	TGameStateFreePlay();
	virtual ~TGameStateFreePlay();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
};

// 0x800CDCC8
class TGameStateDino : public TGameState {
public:
	TGameStateDino();
	virtual ~TGameStateDino();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onHatchedAll(TGameContext* context);
};

// 0x800CE014
class TGameStateGoldenEgg : public TGameState {
public:
	TGameStateGoldenEgg();
	virtual ~TGameStateGoldenEgg();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onGot(TGameContext* context);
	int onLost(TGameContext* context);
};

// 0x800CE14C
class TGameStateGoldenEggLost : public TGameState {
public:
	TGameStateGoldenEggLost();
	virtual ~TGameStateGoldenEggLost();
	virtual void update(TGameContext* context);

	u8 _14[4];
	u8* mTarget; // _18 from the search that the constructor queues
};

// 0x8010E398
class TGameStateMiniGameWait : public TGameState {
public:
	TGameStateMiniGameWait();
	virtual void update(TGameContext* context);
	virtual ~TGameStateMiniGameWait();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onReady(TGameContext* context, ObjMsg* msg);

	int mReady; // _14 set by message 0xAC
};

// 0x8010E510
class TGameStateMiniGame : public TGameState {
public:
	TGameStateMiniGame();
	virtual void update(TGameContext* context);
	virtual ~TGameStateMiniGame();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onScore(TGameContext* context, ObjMsg* msg);
	int onFailed(TGameContext* context);
	int onWon(TGameContext* context);

	int mStep;  // _14
	int mTimer; // _18
};

// 0x8010EB7C
class TGameStateRescue : public TGameState {
public:
	TGameStateRescue();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	virtual ~TGameStateRescue();
	int onTime(TGameContext* context, RescueMsg* msg);
	int onFailed(TGameContext* context);
	int onSaved(TGameContext* context);
};

// 0x8010ED70
class TGameStateRescueErase : public TGameState {
public:
	TGameStateRescueErase();
	virtual ~TGameStateRescueErase();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onErased(TGameContext* context);
};

// 0x80110EF0
class TGameStateClear : public TGameState {
public:
	TGameStateClear();
	virtual ~TGameStateClear() { } // 0x80111B40
	virtual void enter(TGameContext* context);
	virtual void update(TGameContext* context);

	u8 _14[2];
	u16 mTimer; // _16
};

extern "C" int gameStateToDead(TGameContext* context, ObjMsg* msg);

// 0x80110F88
class TGameStateWarp : public TGameState {
public:
	TGameStateWarp();
	virtual ~TGameStateWarp() { } // 0x80111A90
	virtual void enter(TGameContext* context);
	virtual void update(TGameContext* context);
	// 0x80111B08
	virtual int onMessage(TGameContext* context, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0xA4:
			return onMsgA4();
		}
		return TGameState::onMessage(context, msg);
	}
	int onMsgA4();

	u16 mTimer; // _14
	u8 _16[2];
};

// 0x80111028
class TGameStateDead : public TGameState {
public:
	TGameStateDead();
	virtual ~TGameStateDead() { } // 0x801119E0
	virtual void enter(TGameContext* context);
	virtual void update(TGameContext* context);
	// 0x80111A58
	virtual int onMessage(TGameContext* context, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0xA4:
			return onMsgA4();
		}
		return TGameState::onMessage(context, msg);
	}
	int onMsgA4();

	u16 mTimer; // _14
	u8 _16[2];
};

// 0x8011113C
class TGameStateFailed : public TGameState {
public:
	TGameStateFailed();
	virtual ~TGameStateFailed() { } // 0x80111968
	virtual void enter(TGameContext* context);
	virtual void update(TGameContext* context);

	u16 mTimer; // _14
	u8 _16[2];
	int mSent;  // _18 message 0x7A has gone out
};

// 0x80111294
class TGameStateGameOver : public TGameState {
public:
	TGameStateGameOver();
	virtual ~TGameStateGameOver() { } // 0x801118B8
	virtual void enter(TGameContext* context);
	virtual void update(TGameContext* context);
	// 0x80111930
	virtual int onMessage(TGameContext* context, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0xA4:
			return onMsgA4();
		}
		return TGameState::onMessage(context, msg);
	}
	int onMsgA4();

	u16 mFrame; // _14
	u8 _16[2];
};

// 0x80111384
class TGameStateMiss : public TGameState {
public:
	TGameStateMiss();
	virtual ~TGameStateMiss() { } // 0x801117A0
	virtual void enter(TGameContext* context);
	virtual void update(TGameContext* context);
	// 0x80111880
	virtual int onMessage(TGameContext* context, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0xA4:
			return onMsgA4();
		}
		return TGameState::onMessage(context, msg);
	}
	int onMsgA4();

	u16 mTimer; // _14
	u8 _16[2];
};

// 0x8011144C
class TGameStateSucceed : public TGameState {
public:
	TGameStateSucceed();
	virtual ~TGameStateSucceed() { } // 0x80111728
	virtual void enter(TGameContext* context);
	virtual void update(TGameContext* context);

	u16 mTimer; // _14
	u8 _16[2];
};

// 0x80111544
class TGameStateWaitClear : public TGameState {
public:
	TGameStateWaitClear();
	virtual ~TGameStateWaitClear() { } // 0x801116B0
	virtual void enter(TGameContext* context);
	// 0x80111818
	virtual int onMessage(TGameContext* context, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x97:
			return onWarp(context);
		case 0xA2:
			return onClear(context);
		case 0xA8:
			return gameStateToDead(context, msg);
		}
		return TGameState::onMessage(context, msg);
	}
	int onWarp(TGameContext* context);
	int onClear(TGameContext* context);
};

// 0x80113CA4
class TGameStateCollect : public TGameState {
public:
	TGameStateCollect();
	virtual ~TGameStateCollect();
	virtual void update(TGameContext* context);
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onCollected(TGameContext* context);
};

// 0x80114010
class TGameStateBoss : public TGameState {
public:
	TGameStateBoss();
	virtual ~TGameStateBoss();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onBeaten(TGameContext* context);
};

// 0x8011434C
class TGameStateSnowman : public TGameState {
public:
	TGameStateSnowman();
	virtual ~TGameStateSnowman();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onDone(TGameContext* context);
};

// 0x8013B570
class TGameStateBattery : public TGameState {
public:
	TGameStateBattery();
	virtual ~TGameStateBattery();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onCharged(TGameContext* context);
};

// 0x8013B8B8
class TGameStateFireworks : public TGameState {
public:
	TGameStateFireworks();
	virtual ~TGameStateFireworks();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onLit(TGameContext* context);
	int onFailed(TGameContext* context);
};

// 0x8013BCEC
class TGameStateGear : public TGameState {
public:
	TGameStateGear();
	virtual ~TGameStateGear();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onCount(TGameContext* context);
	int onDone(TGameContext* context);
};

// 0x8015F464
class TGameStateWipeOut : public TGameState {
public:
	TGameStateWipeOut();
	virtual ~TGameStateWipeOut();
	virtual void update(TGameContext* context);
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onKilled(TGameContext* context);
};

// 0x8019EBF0
class TGameStateBattleStart : public TGameStateBattle {
public:
	TGameStateBattleStart();
	virtual void run(TGameContextBattle* context);
	virtual ~TGameStateBattleStart();

	s8 mStep;    // _14
	u8 _15[3];
	int mTimer;  // _18
};

// 0x8019EDB4
class TGameStateBattleEnd : public TGameStateBattle {
public:
	TGameStateBattleEnd();
	virtual void run(TGameContextBattle* context);
	virtual ~TGameStateBattleEnd();

	int mTimer; // _14
};

// 0x8019F52C
class TGameStateSurvival : public TGameStateBattle {
public:
	TGameStateSurvival();
	virtual ~TGameStateSurvival();
	virtual void run(TGameContextBattle* context);
	virtual int onMessage(TGameContextBattle* context, ObjMsg* msg);
	int onPlayerOut(TGameContextBattle* context, BattleMsg* msg);
};

// 0x801AB7D4
class TGameStateTime : public TGameStateBattle {
public:
	TGameStateTime();
	virtual ~TGameStateTime();
	virtual void run(TGameContextBattle* context);
	virtual int onMessage(TGameContextBattle* context, ObjMsg* msg);
	int onPlayerOut(TGameContextBattle* context, BattleMsg* msg);
};

// 0x801ABD68
class TGameStateSuddenDeath : public TGameStateBattle {
public:
	TGameStateSuddenDeath();
	virtual ~TGameStateSuddenDeath();
	virtual void run(TGameContextBattle* context);
	virtual int onMessage(TGameContextBattle* context, ObjMsg* msg);
	int onPlayerOut(TGameContextBattle* context, BattleMsg* msg);

	s8 mStep;    // _14
	u8 _15;
	u16 mTimer;  // _16
};

// 0x801AC144
class TGameStateBossAfter : public TGameState {
public:
	TGameStateBossAfter();
	virtual ~TGameStateBossAfter();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onEnd(TGameContext* context);
};

// 0x801AFF8C
class TGameStateLastBoss : public TGameState {
public:
	TGameStateLastBoss();
	virtual ~TGameStateLastBoss();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onEnd(TGameContext* context);
};

// 0x801B4120
class TGameStateRace : public TGameState {
public:
	TGameStateRace();
	virtual ~TGameStateRace();
	virtual int onMessage(TGameContext* context, ObjMsg* msg);
	int onLost(TGameContext* context);
	int onWon(TGameContext* context);
};

// 0x801CE740
class TGameStateHatch : public TGameStateBattle {
public:
	TGameStateHatch();
	virtual ~TGameStateHatch();
	virtual void run(TGameContextBattle* context);
	virtual int onMessage(TGameContextBattle* context, ObjMsg* msg);
	int onPlayerOut(TGameContextBattle* context, BattleMsg* msg);
	int onHit(TGameContextBattle* context, HatchMsg* msg);
};

extern "C" {
void* changeBattleState(void* slot, TGameContextBattle* context, void* state);
int gameStateToDead(TGameContext* context, ObjMsg* msg);
void* createGameStateFreePlay(void* pool, TGameContext* context);
void* createGameStateDino(void* pool, TGameContext* context);
void* createGameStateGoldenEgg(void* pool, TGameContext* context);
void* createGameStateGoldenEggLost(void* pool, TGameContext* context);
void* createGameStateMiniGameWait(void* pool, TGameContext* context);
void* createGameStateMiniGame(void* pool, TGameContext* context);
void* createGameStateRescue(void* pool, TGameContext* context);
void* createGameStateRescueErase(void* pool, TGameContext* context);
void* createGameStateClear(void* pool, TGameContext* context);
void* createGameStateWarp(void* pool, TGameContext* context);
void* createGameStateDead(void* pool, TGameContext* context);
void* createGameStateFailed(void* pool, TGameContext* context);
void* createGameStateGameOver(void* pool, TGameContext* context);
void* createGameStateMiss(void* pool, TGameContext* context);
void* createGameStateSucceed(void* pool, TGameContext* context);
void* createGameStateWaitClear(void* pool, TGameContext* context);
void* createGameStateCollect(void* pool, TGameContext* context);
void* createGameStateBoss(void* pool, TGameContext* context);
void* createGameStateSnowman(void* pool, TGameContext* context);
void* createGameStateBattery(void* pool, TGameContext* context);
void* createGameStateFireworks(void* pool, TGameContext* context);
void* createGameStateGear(void* pool, TGameContext* context);
void* createGameStateWipeOut(void* pool, TGameContext* context);
void* createGameStateBattleStart(void* pool, TGameContextBattle* context);
void* createGameStateBattleEnd(void* pool, TGameContextBattle* context);
void* createGameStateSurvival(void* pool, TGameContextBattle* context);
void* createGameStateTime(void* pool, TGameContextBattle* context);
void* createGameStateSuddenDeath(void* pool, TGameContextBattle* context);
void* createGameStateBossAfter(void* pool, TGameContext* context);
void* createGameStateLastBoss(void* pool, TGameContext* context);
void* createGameStateRace(void* pool, TGameContext* context);
void* createGameStateHatch(void* pool, TGameContextBattle* context);
}

// Swaps the context into a new state and returns the old one's memory to the pool
#define SET_GAME_STATE(context, create) \
	do { \
		void* pool = (context)->mPool; \
		releasePlyState(pool, (context)->mStateSlot.change((context), (TGameState*)create(pool, (context)))); \
	} while (0)

#endif
