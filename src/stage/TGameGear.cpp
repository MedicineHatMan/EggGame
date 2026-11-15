#include "stage/TGame.h"
#include "ui/AeObjControl.h"
#include "stage/TGameGear.h"

const char* gTGameGearName = "TGameGear";
const char* gTGameStateGearName = "TGameStateGear";

// 0x8013BA94
extern "C" void* createGameGear(TObject* parent)
{
	return new TGameGear(parent);
}

// 0x8013BAE0
TGameGear::TGameGear(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameGearName;
	mSize = 0x40;
	mLeft = 3;
	mDone = 0;
}

// 0x8013BB3C
TGameGear::~TGameGear()
{
}

// 0x8013BB98
void TGameGear::startState()
{
	if (mDone == 0) {
		aeCreateMissionIcon1();
		gameShowCount(mLeft);
		SET_GAME_STATE(this, createGameStateGear);
	} else {
		SET_GAME_STATE(this, createGameStateWaitClear);
	}
}

// 0x8013BC44
int TGameGear::turned()
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
#include "stage/TGameStateGear.h"

typedef char TGameStateGear_size_check[(sizeof(TGameStateGear) == 0x14) ? 1 : -1];

// 0x8013BCB0
extern "C" void* createGameStateGear(void* pool, TGameContext*)
{
	return new (pool) TGameStateGear;
}

// 0x8013BCEC
TGameStateGear::TGameStateGear()
{
	mName = gTGameStateGearName; // "TGameStateGear"
	mSize = 0x14;
}

// 0x8013BD38
int TGameStateGear::onCount(TGameContext* context)
{
	((TGameGear*)context)->turned();
	return 0;
}

// 0x8013BD60
int TGameStateGear::onDone(TGameContext* context)
{
	((TGameGear*)context)->mDone = 1;
	SET_GAME_STATE(context, createGameStateSucceed);
	return 0;
}

// 0x8013BDBC
TGameStateGear::~TGameStateGear()
{
}

// 0x8013BE34
int TGameStateGear::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x8E:
		return onDone(context);
	case 0xA0:
		return onCount(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
