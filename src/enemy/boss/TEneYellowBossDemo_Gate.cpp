#include "stage/Stage.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "stage/SetManager.h"
#include "object/ObjCommon.h"
#include "object/ObjAction.h"
#include "egg/EggCreature.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "enemy/EnemyFx.h"
#include "enemy/boss/TEneYellowBossDemo_Gate.h"
#include "core/TMainTask.h"

extern "C" {
MotionEntry sYellowBossDemo_GateMotions[] = {
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 599.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 519.0f, 1.0f, 0.0f, 0 },
};
}

/** The frame each motion's sparkles start on */

// 0x802CAEF0
static const f32 sGatePartStart[2] = { 108.0f, 360.0f };
/** And the frame they stop on */
// 0x802CAEF8
static const f32 sGatePartEnd[2] = { 260.0f, 520.0f };

typedef char TEneYellowBossDemo_Gate_size_check[(sizeof(TEneYellowBossDemo_Gate) == 0xB4) ? 1 : -1];

// 0x80164B88
// Spawns the set object with this id if it is not out yet and sends it the message
static void sendSetMsg(int id, ObjMsg* msg)
{
	TObjAction* obj = findObjById(spawnSetById(id));

	if (obj != 0) {
		obj->sendToHandle(msg);
	}
}

/** What the event-script commands below send */
struct EventMsg {
	ObjMsg mMsg; // _00
	u8 _02[2];   // _02
	int mArg;    // _04
};

// 0x80164BD8
// The event command "talk" the set object then which line
extern "C" int eventTalk(int* args)
{
	EventMsg msg;

	msg.mMsg.mType   = 0x1003;
	*(s16*)&msg.mArg = args[1];
	sendSetMsg(args[0], &msg.mMsg);
	return 1;
}

// 0x80164C14
// The event command "prison_save"
extern "C" int eventPrisonSave(int* args)
{
	EventMsg msg;

	msg.mMsg.mType = 0x1000;
	sendSetMsg(args[0], &msg.mMsg);
	return 1;
}

// 0x80164C48
// The event command "gold_save"
extern "C" int eventGoldSave(int* args)
{
	EventMsg msg;

	msg.mMsg.mType = 0x1002;
	sendSetMsg(args[0], &msg.mMsg);
	return 1;
}

// 0x80164C7C
// The event command "set_talk_mode"
extern "C" int eventSetTalkMode(int* args)
{
	EventMsg msg;

	msg.mMsg.mType = 0x1004;
	msg.mArg       = args[1];
	sendSetMsg(args[0], &msg.mMsg);
	return 1;
}

// 0x80164CB8
// The yellow boss's demo puts up the gate playing motion 0
extern "C" void spawnDemoGate0()
{
	new TEneYellowBossDemo_Gate(gTL_TIMESTOP, 0);
}

// 0x80164CF4
// And the one playing motion 1
extern "C" void spawnDemoGate1()
{
	new TEneYellowBossDemo_Gate(gTL_TIMESTOP, 1);
}

const char* gTEneYellowBossDemo_GateName = "TEneYellowBossDemo_Gate";

// 0x80164D30
TEneYellowBossDemo_Gate::TEneYellowBossDemo_Gate(TObject* parent, int arg)
    : TObject(parent)
{
	_00    = (u32)gTEneYellowBossDemo_GateName;
	mSize  = 0xB4;
	mArg   = arg;
	mModel = modelTEneYellowBoss(5);
	mMotion.init(sYellowBossDemo_GateMotions, motionTEneYellowBoss(3));
	mMotion.play((s16)mArg);
	mXform.mPos.x = 0.0f;
	mXform.mPos.y = 0.0f;
	mXform.mPos.z = 0.0f;
	mDraw         = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	for (int i = 0; i < 2; i++) {
		mPart[i] = 0;
	}
}

// 0x80164E18
TEneYellowBossDemo_Gate::~TEneYellowBossDemo_Gate()
{
	freeParts();
	removeDrawTask(mDraw);
}

// 0x80164EAC
void TEneYellowBossDemo_Gate::freeParts()
{
	for (int i = 1; i >= 0; i--) {
		if (mPart[i] != 0) {
			mPart[i]->mFlags |= 1;
			mPart[i] = 0;
		}
	}
}

// 0x80164EE8
// Gone when the motion ends
void TEneYellowBossDemo_Gate::draw()
{
	mMotion.update();
	if (mMotion.isDone()) {
		mFlags |= 1;
	}
	if (mMotion.mFrame >= sGatePartEnd[mArg]) {
		freeParts();
	} else if (mMotion.mFrame >= sGatePartStart[mArg]) {
		Vec3f at;

		at.x = 0.0f;
		at.y = 50.02f;
		at.z = 0.0f;
		particleKeepAt((void**)&mPart[0], &at, 1, 0x43);
		particleKeepAt((void**)&mPart[1], &at, 1, 0x44);
	}
}

// 0x80164FB0
void TEneYellowBossDemo_Gate::vfn_10()
{
	mFlags &= ~0x10;
}

// 0x80164FC0
void TEneYellowBossDemo_Gate::render()
{
	f32 frame;

	njSetTexture(textureTEneYellowBoss());
	njPushMatrixEx();
	njTranslateEx(&mXform.mPos);
	int cur            = mMotion.mCurrent;
	frame              = mMotion.mFrame;
	NJS_MOTION* motion = mMotion.getMotion(cur);
	drawMotionDefault(mModel, motion, frame);
	njPopMatrixEx();
}
