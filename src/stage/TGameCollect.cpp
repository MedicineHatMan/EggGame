#include "stage/TGame.h"
#include "ui/AeObjControl.h"
#include "stage/TGameCollect.h"

// 0x80113AA0
extern "C" void* createGameCollect(TObject* parent)
{
	return new TGameCollect(parent);
}

// 0x80113AEC
TGameCollect::TGameCollect(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameCollectName;
	mSize = 0x3C;
	mLeft  = 8;
	mTotal = 8;
}

// 0x80113B44
TGameCollect::~TGameCollect() { }

// 0x80113BA0
void TGameCollect::startState()
{
	aeCreateMissionIcon4();
	gameShowCount(mLeft);
	SET_GAME_STATE(this, createGameStateCollect);
}

// 0x80113C10
void TGameCollect::collected()
{
	if (mLeft != 0) {
		mLeft--;
		gameShowCount(mLeft);
	}
}

// 0x80113C58
int TGameCollect::isDone() { return mLeft == 0; }

#include "stage/TGame.h"
#include "stage/TGameStateCollect.h"

typedef char TGameStateCollect_size_check[(sizeof(TGameStateCollect) == 0x14) ? 1 : -1];

// 0x80113C68
extern "C" void* createGameStateCollect(void* pool, TGameContext*)
{
	return new (pool) TGameStateCollect;
}

// 0x80113CA4
TGameStateCollect::TGameStateCollect()
{
	mName = gTGameStateCollectName; // "TGameStateCollect"
	mSize = 0x14;
}

// 0x80113CF0
void TGameStateCollect::update(TGameContext* context)
{
	if (((TGameCollect*)context)->isDone() != 0) {
		SET_GAME_STATE(context, createGameStateSucceed);
	}
}

// 0x80113D54
int TGameStateCollect::onCollected(TGameContext* context)
{
	((TGameCollect*)context)->collected();
	return 1;
}

// 0x80113D7C
TGameStateCollect::~TGameStateCollect()
{
}

// 0x80113DF4
int TGameStateCollect::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA3:
		return onCollected(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
