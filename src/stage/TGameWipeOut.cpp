#include "stage/TGame.h"
#include "ui/AeObjControl.h"
#include "stage/TGameWipeOut.h"

const char* gTGameWipeOutName = "TGameWipeOut";
const char* gTGameStateWipeOutName = "TGameStateWipeOut";

// 0x8015F220
extern "C" void* createGameWipeOut(TObject* parent)
{
	return new TGameWipeOut(parent);
}

// 0x8015F26C
TGameWipeOut::TGameWipeOut(TObject* parent)
    : TGameContext(parent)
{
	_00    = (u32)gTGameWipeOutName;
	mSize  = 0x40;
	mLeft  = 0x64;
	mTotal = 0x64;
}

// 0x8015F2C4
TGameWipeOut::~TGameWipeOut()
{
}

// 0x8015F320
void TGameWipeOut::startState()
{
	aeCreateMissionIcon3();
	gameShowCount(mLeft);
	if (isDone()) {
		SET_GAME_STATE(this, createGameStateWaitClear);
	} else {
		SET_GAME_STATE(this, createGameStateWipeOut);
	}
}

// 0x8015F3D0
void TGameWipeOut::beaten()
{
	if (mLeft > 0) {
		mLeft--;
		gameShowCount(mLeft);
	}
}

// 0x8015F418
int TGameWipeOut::isDone()
{
	return mLeft == 0;
}

#include "stage/TGame.h"
#include "stage/TGameStateWipeOut.h"

typedef char TGameStateWipeOut_size_check[(sizeof(TGameStateWipeOut) == 0x14) ? 1 : -1];

// 0x8015F428
extern "C" void* createGameStateWipeOut(void* pool, TGameContext*)
{
	return new (pool) TGameStateWipeOut;
}

// 0x8015F464
TGameStateWipeOut::TGameStateWipeOut()
{
	mName = gTGameStateWipeOutName; // "TGameStateWipeOut"
	mSize = 0x14;
}

// 0x8015F4B0
void TGameStateWipeOut::update(TGameContext* context)
{
	if (((TGameWipeOut*)context)->isDone() != 0) {
		SET_GAME_STATE(context, createGameStateSucceed);
	}
}

// 0x8015F514
int TGameStateWipeOut::onKilled(TGameContext* context)
{
	((TGameWipeOut*)context)->beaten();
	return 0;
}

// 0x8015F53C
TGameStateWipeOut::~TGameStateWipeOut()
{
}

// 0x8015F5B4
int TGameStateWipeOut::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x96:
		return onKilled(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
