#include "stage/TGame.h"
#include "ui/AeObjControl.h"
#include "stage/TGameRescue.h"
#include "stage/StageTime.h"
#include "stage/TGameStateRescue.h"

const char* gTGameRescueName = "TGameRescue";
const char* gTGameStateRescueName = "TGameStateRescue";
const char* gTGameStateRescueEraseName = "TGameStateRescueErase";

// 0x8010E960
extern "C" void* createGameRescue(TObject* parent)
{
	return new TGameRescue(parent);
}

// 0x8010E9AC
TGameRescue::TGameRescue(TObject* parent)
    : TGameContext(parent)
{
	_00    = (u32)gTGameRescueName;
	mSize  = 0x3C;
	mPhase = 0;
}

// 0x8010EA00
TGameRescue::~TGameRescue()
{
}

// 0x8010EA5C
void TGameRescue::startState()
{
	switch (mPhase) {
	case 0:
		aeCreateMissionIcon8();
		SET_GAME_STATE(this, createGameStateRescue);
		break;
	case 1:
		SET_GAME_STATE(this, createGameStateRescueErase);
		break;
	case 2:
		SET_GAME_STATE(this, createGameStateWaitClear);
		break;
	}
}

typedef char TGameStateRescue_size_check[(sizeof(TGameStateRescue) == 0x14) ? 1 : -1];
typedef char TGameStateRescueErase_size_check[(sizeof(TGameStateRescueErase) == 0x14) ? 1 : -1];

// 0x8010EB40
extern "C" void* createGameStateRescue(void* pool, TGameContext*)
{
	return new (pool) TGameStateRescue;
}

// 0x8010EB7C
TGameStateRescue::TGameStateRescue()
{
	mName = gTGameStateRescueName; // "TGameStateRescue"
	mSize = 0x14;
}

// 0x8010EBC8
int TGameStateRescue::onTime(TGameContext* context, RescueMsg* msg)
{
	HudRatioMsg ratio;
	ratio.mType  = 0xC1;
	ratio.mRatio = (f32)msg->mTime / (f32)msg->mLimit;
	aeSendMissionIcon(&ratio);

	StageTime time;
	HudTimeMsg hud;
	hud.mType    = 0xBF;
	time.mHour   = 0;
	time.mMinute = 0;
	time.mSecond = 0;
	time.mMilli  = 0;
	stageTimeSet(&time, msg->mTime);

	hud.mMinute = time.mMinute;
	hud.mSecond = time.mSecond;
	hud.mMilli  = time.mMilli;
	aeSendMissionIcon(&hud);
	return 0;
}

// 0x8010EC84
int TGameStateRescue::onFailed(TGameContext* context)
{
	SET_GAME_STATE(context, createGameStateFailed);
	return 1;
}

// 0x8010ECD8
int TGameStateRescue::onSaved(TGameContext* context)
{
	((TGameRescue*)context)->mPhase = 1;
	SET_GAME_STATE(context, createGameStateRescueErase);
	return 1;
}

// Inline so that the class has no key function its vtable follows the erase state's and these come last
// 0x8010EEF0
inline int TGameStateRescue::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x89:
		return onTime(context, (RescueMsg*)msg);
	case 0x99:
		return onSaved(context);
	case 0xA6:
		return onFailed(context);
	case 0xA8:
		return gameStateToDead(context, msg);
	}
	return TGameState::onMessage(context, msg);
}

// 0x8010EF6C
inline TGameStateRescue::~TGameStateRescue()
{
}

// 0x8010ED34
extern "C" void* createGameStateRescueErase(void* pool, TGameContext*)
{
	return new (pool) TGameStateRescueErase;
}

// 0x8010ED70
TGameStateRescueErase::TGameStateRescueErase()
{
	mName = gTGameStateRescueEraseName; // "TGameStateRescueErase"
	mSize = 0x14;
}

// 0x8010EDBC
int TGameStateRescueErase::onErased(TGameContext* context)
{
	((TGameRescue*)context)->mPhase = 2;
	SET_GAME_STATE(context, createGameStateSucceed);
	return 1;
}

// 0x8010EE18
TGameStateRescueErase::~TGameStateRescueErase()
{
}

// 0x8010EE90
int TGameStateRescueErase::onMessage(TGameContext* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA8:
		return gameStateToDead(context, msg);
	case 0xAA:
		return onErased(context);
	}
	return TGameState::onMessage(context, msg);
}
