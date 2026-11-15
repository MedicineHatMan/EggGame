#include "player/TPlyDemoLastCtrl.h"

const char* gTPlyDemoLastCtrlName = "TPlyDemoLastCtrl";

#include "player/Player.h"
#include "player/TPlyDemoLastLose.h"
#include "player/TPlyDemoLastSuper.h"
#include "camera/TCameraCtrl.h"
#include "enemy/boss/TEneLastExBoss.h"
#include "object/TObjPlayer.h"
struct TObjPlayer;

typedef char TPlyDemoLastCtrl_size_check[(sizeof(TPlyDemoLastCtrl) == 0x30) ? 1 : -1];

// 0x801CFBBC
TPlyDemoLastCtrl::TPlyDemoLastCtrl(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTPlyDemoLastCtrlName;
	mSize = 0x30;
	mStep = &TPlyDemoLastCtrl::stepLose;
}

// 0x801CFC28
TPlyDemoLastCtrl::~TPlyDemoLastCtrl()
{
}

// 0x801CFC84
void TPlyDemoLastCtrl::draw()
{
	(this->*mStep)();
}

// 0x801CFCAC
void TPlyDemoLastCtrl::stepLose()
{
	TChrPlayer* player;
	ObjMsg msg;
	int ready;

	player = (TChrPlayer*)findPlayer(0);
	ready  = player->vfn_84();
	if (ready == 0) {
		return;
	}
	msg.mType = 0x82;
	sendMsgById(0, &msg);
	new TPlyDemoLastLose(this);
	hideLastExBoss();
	resetLastExBoss();
	mStep = &TPlyDemoLastCtrl::stepAfterLose;
}

// 0x801CFD4C
void TPlyDemoLastCtrl::stepAfterLose()
{
	TChrPlayer* player;

	// The word is inside TObject's unnamed gap
	if (*(u32*)((u8*)this + 0x14) != 0) {
		return;
	}
	player = (TChrPlayer*)findPlayer(0);
	player->vfn_74(100);
	mTimer = 900;
	showLastExBoss();
	camSetFar(1.0f, 0);
	camResetCamera(0);
	mStep = &TPlyDemoLastCtrl::stepSuper;
}

// 0x801CFDDC
void TPlyDemoLastCtrl::stepSuper()
{
	ObjMsg msg;

	if (mTimer != 0) {
		mTimer--;
		return;
	}
	if (lastExBossIsIdle() == 0) {
		return;
	}
	msg.mType = 0x82;
	sendMsgById(0, &msg);
	new TPlyDemoLastSuper(this);
	hideLastExBoss();
	resetLastExBoss();
	mStep = &TPlyDemoLastCtrl::stepAfterSuper;
}

// 0x801CFE80
void TPlyDemoLastCtrl::stepAfterSuper()
{
	TChrPlayer* player;

	if (*(u32*)((u8*)this + 0x14) != 0) {
		return;
	}
	player = (TChrPlayer*)findPlayer(0);
	player->vfn_74(100);
	camSetFar(1.0f, 0);
	camResetCamera(0);
	showLastExBoss();
	mStep = &TPlyDemoLastCtrl::stepDone;
}

// 0x801CFF08
void TPlyDemoLastCtrl::stepDone()
{
}
