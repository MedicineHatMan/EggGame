#include "stage/TGame.h"
#include "ui/AeObjControl.h"
#include "stage/TGameGoldenEgg.h"
#include "stage/TGameStateGoldenEgg.h"
#include "stage/TEventCtrl.h"
#include "egg/TGoldenEggBreak.h"

// 0x800CDE48
extern "C" void* createGameGoldenEgg(TObject* parent)
{
	return new TGameGoldenEgg(parent);
}

// 0x800CDE94
TGameGoldenEgg::TGameGoldenEgg(TObject* parent)
    : TGameContext(parent)
{
	_00   = (u32)gTGameGoldenEggName;
	mSize = 0x3C;
	mDone = 0;
}

// 0x800CDEE8
TGameGoldenEgg::~TGameGoldenEgg() { }

// 0x800CDF44
void TGameGoldenEgg::startState()
{
	if (mDone == 0) {
		aeCreateMissionIcon7();
		SET_GAME_STATE(this, createGameStateGoldenEgg);
	} else {
		SET_GAME_STATE(this, createGameStateWaitClear);
	}
}

typedef char TGameStateGoldenEgg_size_check[(sizeof(TGameStateGoldenEgg) == 0x14) ? 1 : -1];
typedef char TGameStateGoldenEggLost_size_check[(sizeof(TGameStateGoldenEggLost) == 0x1C) ? 1 : -1];

// The two virtuals are inline so that the class has no key function its vtable and these two come last
// 0x800CE2CC
inline TGameStateGoldenEgg::~TGameStateGoldenEgg()
{
}

// 0x800CE344
inline int TGameStateGoldenEgg::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA8:
		return gameStateToDead(context, msg);
	case 0x8D:
		return onGot(context);
	case 0x93:
		return onLost(context);
	}
	return TGameState::onMessage(context, msg);
}

// 0x800CDFD8
extern "C" void* createGameStateGoldenEgg(void* pool, TGameContext*)
{
	return new (pool) TGameStateGoldenEgg;
}

// 0x800CE014
TGameStateGoldenEgg::TGameStateGoldenEgg()
{
	mName = gTGameStateGoldenEggName; // "TGameStateGoldenEgg"
	mSize = 0x14;
}

// 0x800CE060
int TGameStateGoldenEgg::onGot(TGameContext* context)
{
	((TGameGoldenEgg*)context)->mDone = 1;
	SET_GAME_STATE(context, createGameStateSucceed);
	return 0;
}

// 0x800CE0BC
int TGameStateGoldenEgg::onLost(TGameContext* context)
{
	SET_GAME_STATE(context, createGameStateGoldenEggLost);
	return 0;
}

// 0x800CE110
extern "C" void* createGameStateGoldenEggLost(void* pool, TGameContext*)
{
	return new (pool) TGameStateGoldenEggLost;
}

// 0x800CE14C
TGameStateGoldenEggLost::TGameStateGoldenEggLost()
{
	mName = gTGameStateGoldenEggLostName; // "TGameStateGoldenEggLost"
	mSize = 0x1C;
	mTarget = 0;
	eventQueueAdd((int)this, (int)goldenEggLostFound);
}

// 0x800CE1B0
extern "C" int goldenEggLostFound(TGameStateGoldenEggLost* state)
{
	state->mTarget = (u8*)goldenEggBreakDemoNew();
	return 1;
}

// 0x800CE1E4
void TGameStateGoldenEggLost::update(TGameContext* context)
{
	if (mTarget != 0 && *(u16*)(mTarget + 0x20) == 0) {
		eventEnd();
		SET_GAME_STATE(context, createGameStateFailed);
	}
}

// 0x800CE254
TGameStateGoldenEggLost::~TGameStateGoldenEggLost()
{
}
