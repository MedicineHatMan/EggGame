#include "stage/GameState.h"
#include "ui/AeObjControl.h"
#include "player/PlyPool.h"
#include "sound/SoundCtrl.h"
#include "stage/TGameStateBattle.h"

// 0x8019EF84
BattleStateSlot::BattleStateSlot()
{
	mState = 0;
}

// 0x8019EF90
BattleStateSlot::~BattleStateSlot()
{
}

// 0x8019EFCC
void BattleStateSlot::update(TGameContextBattle* context)
{
	if (mState != 0) {
		mState->run(context);
	}
}

// 0x8019F004
// Leaves the old state enters the new one and returns the old one for the caller to release
extern "C" void* changeBattleState(void* slot, TGameContextBattle* context, void* state)
{
	BattleStateSlot* s     = (BattleStateSlot*)slot;
	TBattleStateRoot* old  = s->mState;
	TBattleStateRoot* next = (TBattleStateRoot*)state;
	if (old != 0) {
		old->exit(context);
	}
	if (next != 0) {
		next->enter(context);
	}
	s->mState = (TGameStateBattle*)next;
	return old;
}
