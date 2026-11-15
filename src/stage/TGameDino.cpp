#include "stage/TGame.h"
#include "ui/AeObjControl.h"
#include "stage/TGameDino.h"

// 0x800CDA78
extern "C" void* createGameDino(TObject* parent)
{
	return new TGameDino(parent);
}

// 0x800CDAC4
TGameDino::TGameDino(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameDinoName;
	mSize = 0x3C;
	mLeft = 3;
}

// 0x800CDB18
TGameDino::~TGameDino() { }

// 0x800CDB74
void TGameDino::startState()
{
	aeCreateMissionIcon0();
	gameShowCount(mLeft);
	if (mLeft == 0) {
		SET_GAME_STATE(this, createGameStateWaitClear);
	} else {
		SET_GAME_STATE(this, createGameStateDino);
	}
}

// 0x800CDC20
int TGameDino::hatched()
{
	if (mLeft > 0) {
		mLeft--;
		gameShowCount(mLeft);
	}
	if (mLeft == 0) {
		return 1;
	}
	return 0;
}

#include "stage/TGame.h"
#include "stage/TGameStateDino.h"

typedef char TGameStateDino_size_check[(sizeof(TGameStateDino) == 0x14) ? 1 : -1];

// 0x800CDC8C
extern "C" void* createGameStateDino(void* pool, TGameContext*)
{
	return new (pool) TGameStateDino;
}

// 0x800CDCC8
TGameStateDino::TGameStateDino()
{
	mName = gTGameStateDinoName; // "TGameStateDino"
	mSize = 0x14;
}

// 0x800CDD14
int TGameStateDino::onHatchedAll(TGameContext* context)
{
	if (((TGameDino*)context)->hatched() != 0) {
		SET_GAME_STATE(context, createGameStateSucceed);
	}
	return 0;
}

// 0x800CDD7C
TGameStateDino::~TGameStateDino()
{
}

// 0x800CDDF4
int TGameStateDino::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x92:
		return onHatchedAll(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
