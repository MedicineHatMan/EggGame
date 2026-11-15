#include "stage/TGame.h"
#include "stage/TGameGoal.h"

// 0x801144C0
extern "C" void* createGameGoal(TObject* parent)
{
	return new TGameGoal(parent);
}

const char* gTGameGoalName = "TGameGoal";

// 0x8011450C
TGameGoal::TGameGoal(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameGoalName;
	mSize = 0x38;
}

// 0x80114558
TGameGoal::~TGameGoal()
{
}

// 0x801145B4
void TGameGoal::startState()
{
	SET_GAME_STATE(this, createGameStateWaitClear);
}
