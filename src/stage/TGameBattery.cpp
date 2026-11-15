#include "stage/TGame.h"
#include "ui/AeObjControl.h"
#include "stage/TGameBattery.h"

// 0x8013B320
extern "C" void* createGameBattery(TObject* parent)
{
	return new TGameBattery(parent);
}

// 0x8013B36C
TGameBattery::TGameBattery(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameBatteryName;
	mSize = 0x3C;
	mLeft = 4;
}

// 0x8013B3C0
TGameBattery::~TGameBattery() { }

// 0x8013B41C
void TGameBattery::startState()
{
	if (mLeft != 0) {
		aeCreateMissionIcon2();
		gameShowCount(mLeft);
		SET_GAME_STATE(this, createGameStateBattery);
	} else {
		SET_GAME_STATE(this, createGameStateWaitClear);
	}
}

// 0x8013B4C8
int TGameBattery::lit()
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
#include "stage/TGameStateBattery.h"

typedef char TGameStateBattery_size_check[(sizeof(TGameStateBattery) == 0x14) ? 1 : -1];

// 0x8013B534
extern "C" void* createGameStateBattery(void* pool, TGameContext*)
{
	return new (pool) TGameStateBattery;
}

// 0x8013B570
TGameStateBattery::TGameStateBattery()
{
	mName = gTGameStateBatteryName; // "TGameStateBattery"
	mSize = 0x14;
}

// 0x8013B5BC
int TGameStateBattery::onCharged(TGameContext* context)
{
	if (((TGameBattery*)context)->lit() != 0) {
		SET_GAME_STATE(context, createGameStateSucceed);
	}
	return 0;
}

// 0x8013B624
TGameStateBattery::~TGameStateBattery()
{
}

// 0x8013B69C
int TGameStateBattery::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x8C:
		return onCharged(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}
