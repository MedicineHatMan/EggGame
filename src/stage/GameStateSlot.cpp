#include "stage/Stage.h"
#include "stage/GameState.h"

// 0x80042AFC
GameStateSlot::GameStateSlot()
{
	mState = 0;
}

// 0x80042B08
GameStateSlot::~GameStateSlot()
{
}

// 0x80042B44
void GameStateSlot::update(TGameContext* context)
{
	if (mState != 0) {
		mState->run(context);
	}
}

// 0x80042B7C
TGameState* GameStateSlot::change(TGameContext* context, TGameState* next)
{
	TGameState* old = mState;
	if (old != 0) {
		old->exit(context);
	}
	if (next != 0) {
		next->enter(context);
	}
	mState = next;
	return old;
}
