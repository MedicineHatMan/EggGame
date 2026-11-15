#include "stage/TGame.h"
#include "ui/TFade.h"
#include "sound/SoundCtrl.h"
#include "object/TObjPlayer.h"
#include "stage/TGameBossAfter.h"
struct TChrPlayer;
struct TObjPlayer;

// 0x801AC01C
extern "C" void* createGameBossAfter(TObject* parent)
{
	return new TGameBossAfter(parent);
}

// 0x801AC068
TGameBossAfter::TGameBossAfter(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameBossAfterName;
	mSize = 0x38;
}

// 0x801AC0B4
void TGameBossAfter::startState()
{
	SET_GAME_STATE(this, createGameStateBossAfter);
}

typedef char TGameStateBossAfter_size_check[(sizeof(TGameStateBossAfter) == 0x14) ? 1 : -1];

// 0x801AC108
extern "C" void* createGameStateBossAfter(void* pool, TGameContext*)
{
	return new (pool) TGameStateBossAfter;
}

// 0x801AC144
TGameStateBossAfter::TGameStateBossAfter()
{
	mName = gTGameStateBossAfterName; // "TGameStateBossAfter"
	mSize = 0x14;
}

// 0x801AC190
int TGameStateBossAfter::onEnd(TGameContext* context)
{
	TChrPlayer* player = (TChrPlayer*)findPlayer(0);
	if (player != 0) {
		player->vfn_B4(0);
	}
	bgmFadeTo(0.0f, 120);
	context->setCleared();
	fadeToWhite();
	SET_GAME_STATE(context, createGameStateWarp);
	return 1;
}

// 0x801AC224
TGameStateBossAfter::~TGameStateBossAfter()
{
}

// 0x801AC29C
int TGameStateBossAfter::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x94:
		return onEnd(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
