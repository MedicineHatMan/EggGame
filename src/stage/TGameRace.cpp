#include "stage/TGame.h"
#include "stage/TGameRace.h"
#include "stage/GameState.h"
#include "stage/TGameStateRace.h"

// 0x801B3F58
extern "C" void* createGameRace(TObject* parent)
{
	return new TGameRace(parent);
}

const char* gTGameRaceName      = "TGameRace";
const char* gTGameStateRaceName = "TGameStateRace";

// 0x801B3FA4
TGameRace::TGameRace(TObject* parent)
    : TGameContext(parent)
{
	mDone = 0;
	_00   = (u32)gTGameRaceName;
	mSize = 0x3C;
}

// 0x801B3FF8
TGameRace::~TGameRace()
{
}

// 0x801B4054
void TGameRace::startState()
{
	if (mDone == 0) {
		SET_GAME_STATE(this, createGameStateRace);
	} else {
		SET_GAME_STATE(this, createGameStateWaitClear);
	}
}

typedef char TGameStateRace_size_check[(sizeof(TGameStateRace) == 0x14) ? 1 : -1];

// 0x801B40E4
extern "C" void* createGameStateRace(void* pool, TGameContext*)
{
	return new (pool) TGameStateRace;
}

// 0x801B4120
TGameStateRace::TGameStateRace()
{
	mName = gTGameStateRaceName; // "TGameStateRace"
	mSize = 0x14;
}

// 0x801B416C
int TGameStateRace::onLost(TGameContext* context)
{
	SET_GAME_STATE(context, createGameStateFailed);
	return 1;
}

// 0x801B41C0
int TGameStateRace::onWon(TGameContext* context)
{
	SET_GAME_STATE(context, createGameStateSucceed);
	return 1;
}

// 0x801B4214
TGameStateRace::~TGameStateRace()
{
}

// 0x801B428C
int TGameStateRace::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA7:
		return onLost(context);
	case 0xA9:
		return onWon(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
