#include "stage/TGame.h"
#include "stage/TGameBoss.h"

const char* gTGameBossName = "TGameBoss";
const char* gTGameStateBossName = "TGameStateBoss";

// 0x80113E48
extern "C" void* createGameBoss(TObject* parent)
{
	return new TGameBoss(parent);
}

// 0x80113E94
TGameBoss::TGameBoss(TObject* parent)
    : TGameContext(parent)
{
	_00     = (u32)gTGameBossName;
	mSize   = 0x3C;
	mBeaten = 0;
}

// 0x80113EE8
TGameBoss::~TGameBoss()
{
}

// 0x80113F44
void TGameBoss::startState()
{
	if (mBeaten == 0) {
		SET_GAME_STATE(this, createGameStateBoss);
	} else {
		SET_GAME_STATE(this, createGameStateWaitClear);
	}
}

typedef char TGameStateBoss_size_check[(sizeof(TGameStateBoss) == 0x14) ? 1 : -1];

// 0x80113FD4
extern "C" void* createGameStateBoss(void* pool, TGameContext*)
{
	return new (pool) TGameStateBoss;
}

// 0x80114010
TGameStateBoss::TGameStateBoss()
{
	mName = gTGameStateBossName; // "TGameStateBoss"
	mSize = 0x14;
}

// 0x8011405C
int TGameStateBoss::onBeaten(TGameContext* context)
{
	((TGameBoss*)context)->mBeaten = 1;
	SET_GAME_STATE(context, createGameStateSucceed);
	return 1;
}

// 0x801140B8
TGameStateBoss::~TGameStateBoss()
{
}

// 0x80114130
int TGameStateBoss::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x90:
		return onBeaten(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
