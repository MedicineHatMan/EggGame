#include "stage/GameState.h"
#include "ui/AeObjControl.h"
#include "player/PlyPool.h"
#include "sound/SoundCtrl.h"
#include "stage/TGameStateBattle.h"

const char* gTGameStateBattleName = "TGameStateBattle";

typedef char TGameStateBattle_size_check[(sizeof(TGameStateBattle) == 0x14) ? 1 : -1];
typedef char TGameStateBattleStart_size_check[(sizeof(TGameStateBattleStart) == 0x1C) ? 1 : -1];
typedef char TGameStateBattleEnd_size_check[(sizeof(TGameStateBattleEnd) == 0x18) ? 1 : -1];

// 0x8019E658
TGameStateBattle::TGameStateBattle()
{
	mName = gTGameStateBattleName; // "TGameStateBattle"
	mSize = 0x14;
}

// 0x8019E6B0
int TGameStateBattle::onMessage(TGameContextBattle* context, ObjMsg* msg)
{
	return battleDefaultMessage(context, msg);
}

// 0x8019E6D8
TGameContextBattle::TGameContextBattle(TObject* parent)
    : TCustomGameContext(parent)
{
	_00        = (u32)sGameContextBattleName;
	mSize      = 0x98;
	void* pool = ::operator new(0xC);
	if (pool != 0) {
		pool = initPlyPool(pool, 0x40);
	}
	mPool        = pool;
	mPlayerCount = 0;
	mTime        = 0;
	for (int i = 0; i < 4; i++) {
		mPlayers[i].mId      = 0xFFFF;
		mPlayers[i].mTimer   = 0;
		mPlayers[i].mPlace   = 0;
		mPlayers[i].mPoints  = 0;
		mPlayers[i].mLost    = 0;
		mPlayers[i].mHatched = 0;
	}
}

// 0x8019E7AC
TGameContextBattle::~TGameContextBattle()
{
	freePlyPool(mPool, 1);
}

static int battleOnHatch(TGameContextBattle* context, HatchMsg* msg);

// 0x8019E838
// What every battle state does with a message it does not take itself
extern "C" int battleDefaultMessage(TGameContextBattle* context, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0xA5:
		return battleOnHatch(context, (HatchMsg*)msg);
	}
	return 0;
}

// 0x8019E870
int TGameContextBattle::onMessage(ObjMsg* msg)
{
	if (mStateSlot.mState != 0) {
		return mStateSlot.mState->onMessage(this, msg);
	}
	return 0;
}

// 0x8019E8BC
// Message 0xA5 an egg hatched
static int battleOnHatch(TGameContextBattle* context, HatchMsg* msg)
{
	int index = battleFindPlayer(context, msg->mPlayer);
	if (index == -1) {
		return 0;
	}
	context->mPlayers[index].mHatched++;
	return 0;
}

// 0x8019E914
void TGameContextBattle::draw()
{
	for (int i = 0; i < mPlayerCount; i++) {
		if (mPlayers[i].mTimer != 0) {
			if (--mPlayers[i].mTimer == 0) {
				ObjMsg msg;
				msg.mType = 0x2B;
				sendMsgById(mPlayers[i].mId, &msg);
			}
		}
	}
	if (mStateSlot.mState != 0) {
		mStateSlot.mState->vfn_18(this);
	}
	mStateSlot.update(this);
	mTime++;
}

// 0x8019E9C8
// From the battle mode for each port that plays
extern "C" void battleAddPlayer(TGameContextBattle* context, u16 id, u8 chara)
{
	int index                    = context->mPlayerCount++;
	context->mPlayers[index].mId = id;
	context->mPlayers[index]._00 = chara;
}

// 0x8019E9E8
extern "C" int battleFindPlayer(TGameContextBattle* context, u16 id)
{
	for (int i = 0; i < context->mPlayerCount; i++) {
		if (id == context->mPlayers[i].mId) {
			return i;
		}
	}
	return -1;
}

// 0x8019EA30
// The player is out a loss for them a point for whoever did it and with guard set a 180 frame wait before they can be
// hit again
extern "C" int battlePlayerOut(TGameContextBattle* context, u16 player, u16 other, int guard)
{
	int index = battleFindPlayer(context, player);
	if (index == -1) {
		return -1;
	}
	context->mPlayers[index].mLost++;
	if (other != player) {
		int by = battleFindPlayer(context, other);
		if (by != -1) {
			context->mPlayers[by].mPoints++;
		}
	}
	if (guard != 0) {
		context->mPlayers[index].mTimer = 180;
	}
	return index;
}

// 0x8019EAE0
// Once done how it ended
int TGameContextBattle::getStatus()
{
	if (mDone.mBits & 1) {
		return mEndKind;
	}
	return 0;
}

// 0x8019EAFC
void TGameContextBattle::endOnResult1()
{
	mEndKind   = 6;
	void* pool = mPool;
	releasePlyState(pool, changeBattleState(&mStateSlot, this, createGameStateBattleEnd(pool, this)));
}

// 0x8019EB58
void TGameContextBattle::endOnResult0()
{
	mEndKind   = 5;
	void* pool = mPool;
	releasePlyState(pool, changeBattleState(&mStateSlot, this, createGameStateBattleEnd(pool, this)));
}

// 0x8019EBB4
extern "C" void* createGameStateBattleStart(void* pool, TGameContextBattle*)
{
	return new (pool) TGameStateBattleStart;
}

// 0x8019EBF0
TGameStateBattleStart::TGameStateBattleStart()
{
	mName = gTGameStateBattleStartName; // "TGameStateBattleStart"
	mSize = 0x1C;
	mStep = 0;
}

// 0x8019ECAC
void TGameStateBattleStart::run(TGameContextBattle* context)
{
	switch (mStep) {
	case 0:
		aeBattleReady();
		sePlay(0x32);
		mStep  = 1;
		mTimer = 60;
	case 1:
		if (--mTimer == 0) {
			sePlay(0x33);
			aeBattleGo();
			mStep  = 2;
			mTimer = 30;
		}
		break;
	case 2:
		if (--mTimer == 0) {
			mStep = 3;
			context->start();
		}
		break;
	}
}

// 0x8019ED78
extern "C" void* createGameStateBattleEnd(void* pool, TGameContextBattle*)
{
	return new (pool) TGameStateBattleEnd;
}

// 0x8019EDB4
TGameStateBattleEnd::TGameStateBattleEnd()
{
	mName  = gTGameStateBattleEndName; // "TGameStateBattleEnd"
	mSize  = 0x18;
	mTimer = 120;
}

// 0x8019EE08
void TGameStateBattleEnd::run(TGameContextBattle* context)
{
	if (--mTimer == 0) {
		context->mDone.mBits |= 1;
	}
}

// 0x8019EE28
TGameStateBattleEnd::~TGameStateBattleEnd()
{
}

// 0x8019EEA0
TGameStateBattleStart::~TGameStateBattleStart()
{
}

// 0x8019EF80
// is TGameContextBattle::start empty and inline
