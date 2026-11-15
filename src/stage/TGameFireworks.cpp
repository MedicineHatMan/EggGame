#include "stage/TGame.h"
#include "stage/TGameFireworks.h"

// 0x8013B6F0
extern "C" void* createGameFireworks(TObject* parent)
{
	return new TGameFireworks(parent);
}

// 0x8013B73C
TGameFireworks::TGameFireworks(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameFireworksName;
	mSize = 0x3C;
	mDone = 0;
}

// 0x8013B790
TGameFireworks::~TGameFireworks() { }

// 0x8013B7EC
void TGameFireworks::startState()
{
	if (mDone == 0) {
		SET_GAME_STATE(this, createGameStateFireworks);
	} else {
		SET_GAME_STATE(this, createGameStateWaitClear);
	}
}

typedef char TGameStateFireworks_size_check[(sizeof(TGameStateFireworks) == 0x14) ? 1 : -1];

// 0x8013B87C
extern "C" void* createGameStateFireworks(void* pool, TGameContext*)
{
	return new (pool) TGameStateFireworks;
}

// 0x8013B8B8
TGameStateFireworks::TGameStateFireworks()
{
	mName = gTGameStateFireworksName; // "TGameStateFireworks"
	mSize = 0x14;
}

// 0x8013B904
int TGameStateFireworks::onLit(TGameContext* context)
{
	((TGameFireworks*)context)->mDone = 1;
	SET_GAME_STATE(context, createGameStateSucceed);
	return 1;
}

// 0x8013B960
int TGameStateFireworks::onFailed(TGameContext* context)
{
	SET_GAME_STATE(context, createGameStateFailed);
	return 1;
}

// 0x8013B9B4
TGameStateFireworks::~TGameStateFireworks()
{
}

// 0x8013BA2C
int TGameStateFireworks::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x9E:
		return onLit(context);
	case 0x9F:
		return onFailed(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
