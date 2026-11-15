#include "stage/TGame.h"
#include "stage/GameState.h"
#include "stage/TGameFreePlay.h"

// 0x800CD7F0
extern "C" void* createGameFreePlay(TObject* parent)
{
	return new TGameFreePlay(parent);
}

// 0x800CD83C
TGameFreePlay::TGameFreePlay(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameFreePlayName;
	mSize = 0x38;
}

// 0x800CD888
TGameFreePlay::~TGameFreePlay()
{
}

// 0x800CD8E4
void TGameFreePlay::startState()
{
	SET_GAME_STATE(this, createGameStateFreePlay);
}

typedef char TGameStateFreePlay_size_check[(sizeof(TGameStateFreePlay) == 0x14) ? 1 : -1];

// 0x800CD938
extern "C" void* createGameStateFreePlay(void* pool, TGameContext*)
{
	return new (pool) TGameStateFreePlay;
}

// 0x800CD974
TGameStateFreePlay::TGameStateFreePlay()
{
	mName = gTGameStateFreePlayName; // "TGameStateFreePlay"
	mSize = 0x14;
}

// 0x800CD9C0
TGameStateFreePlay::~TGameStateFreePlay()
{
}

// 0x800CDA38
int TGameStateFreePlay::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
