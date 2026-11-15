#include "stage/TGame.h"
#include "stage/GameState.h"
#include "ui/TFade.h"
#include "stage/TGameLastBoss.h"

const char* gTGameLastBossName = "TGameLastBoss";
const char* gTGameStateLastBossName = "TGameStateLastBoss";

// 0x801AFE08
extern "C" void* createGameLastBoss(TObject* parent)
{
	return new TGameLastBoss(parent);
}

// 0x801AFE54
TGameLastBoss::TGameLastBoss(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameLastBossName;
	mSize = 0x38;
}

// 0x801AFEA0
TGameLastBoss::~TGameLastBoss()
{
}

// 0x801AFEFC
void TGameLastBoss::startState()
{
	SET_GAME_STATE(this, createGameStateLastBoss);
}

typedef char TGameStateLastBoss_size_check[(sizeof(TGameStateLastBoss) == 0x14) ? 1 : -1];

// 0x801AFF50
extern "C" void* createGameStateLastBoss(void* pool, TGameContext*)
{
	return new (pool) TGameStateLastBoss;
}

// 0x801AFF8C
TGameStateLastBoss::TGameStateLastBoss()
{
	mName = gTGameStateLastBossName; // "TGameStateLastBoss"
	mSize = 0x14;
}

// 0x801AFFD8
int TGameStateLastBoss::onEnd(TGameContext* context)
{
	context->setCleared();
	fadeToWhite();
	SET_GAME_STATE(context, createGameStateWarp);
	return 1;
}

// 0x801B003C
TGameStateLastBoss::~TGameStateLastBoss()
{
}

// 0x801B00B4
int TGameStateLastBoss::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x90:
		return onEnd(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
