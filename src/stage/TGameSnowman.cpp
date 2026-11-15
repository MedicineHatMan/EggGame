#include "stage/TGame.h"
#include "stage/TGameSnowman.h"

// 0x80114184
extern "C" void* createGameSnowman(TObject* parent)
{
	return new TGameSnowman(parent);
}

// 0x801141D0
TGameSnowman::TGameSnowman(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameSnowmanName;
	mSize = 0x3C;
	mDone = 0;
}

// 0x80114224
TGameSnowman::~TGameSnowman()
{
}

// 0x80114280
void TGameSnowman::startState()
{
	if (mDone == 0) {
		SET_GAME_STATE(this, createGameStateSnowman);
	} else {
		SET_GAME_STATE(this, createGameStateWaitClear);
	}
}

typedef char TGameStateSnowman_size_check[(sizeof(TGameStateSnowman) == 0x14) ? 1 : -1];

// 0x80114310
extern "C" void* createGameStateSnowman(void* pool, TGameContext*)
{
	return new (pool) TGameStateSnowman;
}

// 0x8011434C
TGameStateSnowman::TGameStateSnowman()
{
	mName = gTGameStateSnowmanName; // "TGameStateSnowman"
	mSize = 0x14;
}

// 0x80114398
int TGameStateSnowman::onDone(TGameContext* context)
{
	((TGameSnowman*)context)->mDone = 1;
	SET_GAME_STATE(context, createGameStateSucceed);
	return 1;
}

// 0x801143F4
TGameStateSnowman::~TGameStateSnowman()
{
}

// 0x8011446C
int TGameStateSnowman::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x9D:
		return onDone(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
