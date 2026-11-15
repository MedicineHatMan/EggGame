#include "stage/EventScript.h"
#include "io/Resource.h"
#include "camera/EventCamRef.h"
#include "object/ObjAction.h"
#include "stage/TEventCtrl.h"
#include "core/dolphin.h"
#include "egg/EggCreature.h"
#include "object/TObjKey00.h"
#include "sound/SoundCtrl.h"
#include "stage/SetManager.h"
#include "animal/AniMount.h"
#include "object/TObjGoldenEgg.h"
#include "object/TObjMGLeader.h"
#include "object/mission/TMsnObjGoalPole.h"
#include "object/mission/TMsnObjRing.h"
#include "animal/TAnimalRacer.h"
#include "enemy/TEneGreenDemo.h"
#include "stage/TGameState.h"
#include "sound/Sound3DPlay.h"
#include "engine/Landscape.h"
#include "object/purple/TObjPurpleBlizzard.h"
#include "object/green/TObjGreenHoverLeaf.h"
#include "object/TObjHelios.h"
#include "object/green/TObjGreenSpore.h"
#include "object/mission/TObjMissionPrisoner.h"
#include "ui/TAeObjStageTitle.h"
#include "object/TObjPlayer.h"
#include "enemy/boss/TEneYellowBossDemo_Gate.h"
#include "ui/AeObjControl.h"
#include "animal/TAniRacePath.h"
#include "engine/TLightEffect.h"
#include "object/mission/TObjMissionBomb.h"
#include "object/purple/TObjPurplePropeller.h"
#include "object/mission/TMsnObjChicken.h"
#include "object/purple/TObjPurpleGear.h"
#include "object/mission/TMsnObjOrangeCannon.h"
struct ObjMsg;

extern "C" {

// Commands kept in other files
}

/** What loadArc gives for a script archive */
TArc* gEventArchive;   // 0x802C5628
EventScriptSet* gEventScripts; // 0x802C562C
EventScriptMgr* gEventMgr;     // 0x802C5630

static void eventCamStart();
static void eventCamStop();
static void eventCamRestart();
static int eventCallback(int arg);

// 0x8012C148
EventScriptMgr::EventScriptMgr()
{
	clear();
}

// 0x8012C178
EventScriptMgr::~EventScriptMgr()
{
}

// 0x8012C1B4
void EventScriptMgr::clear()
{
	mState   = STATE_IDLE;
	mTimer   = 0;
	mEventId = 0xFFFF;
	mBits    = 0;
	mQueued  = 0;
	mScript  = 0;
	mLine    = 0;
	mKind    = 0;
	mPhase   = 0;
}

// 0x8012C1E8
void EventScriptMgr::reset()
{
	end();
	clearQueue();
	mBits  = 0;
	mTimer = 0;
	mKind  = 0;
}

// 0x8012C22C
EventScript* EventScriptMgr::find(int kind, int id)
{
	EventScriptSet* set = gEventScripts;
	if (set != 0) {
		for (int i = 0; i < set->mCount; i++) {
			EventScript* script = set->mScripts[i];
			if (script->mKind == kind && script->mId == id) {
				return script;
			}
		}
	}
	return 0;
}

// 0x8012C27C
int EventScriptMgr::start(int kind, int id)
{
	if (eventIsBusy() != 0 || eventCanStart() == 0) {
		return 0;
	}
	if (mState != STATE_IDLE) {
		return 0;
	}
	mScript = find(kind, id);
	if (mScript == 0) {
		return 0;
	}
	mLine  = 0;
	mKind  = kind;
	mPhase = 0;
	mBits  = 0;
	return 1;
}

// 0x8012C320
EventLine* EventScriptMgr::nextLine()
{
	EventLine* line = 0;
	int more        = 1;
	while (more && mLine < mScript->mLineCount) {
		line     = mScript->mLines[mLine];
		more     = 0;
		int wait = line->mBits & 1;
		switch (mPhase) {
		case 0:
			if (mBits & BIT_FAILED) {
				line = 0;
				more = 0;
			} else if (wait) {
				mPhase = 1;
			}
			break;
		case 1:
			if (mBits & BIT_FAILED) {
				if (wait == 0) {
					line = 0;
					more = 1;
				}
			} else {
				mPhase = 2;
			}
			break;
		case 2:
			if ((mBits & BIT_FAILED) || wait) {
				mPhase = 4;
				line   = 0;
			}
			break;
		}
		if (more) {
			mLine++;
		}
	}
	return line;
}

// 0x8012C41C
int EventScriptMgr::run()
{
	if (mScript == 0) {
		return 1;
	}
	EventCmd* cmds = getEventCmds();
	int more       = 1;
	EventLine* line;
	while (more) {
		line = nextLine();
		if (line == 0) {
			break;
		}
		switch (cmds[line->mCmd].mWait) {
		case 0:
			exec();
			more = 1;
			break;
		case 1:
			mTimer = 1800;
			if (mBits & BIT_EVENT) {
				exec();
				if (mBits & BIT_DONE) {
					more = 1;
				} else {
					more = 0;
				}
			} else if (eventStartById(mEventId, eventCallback) != 0) {
				mBits |= BIT_EVENT;
				more   = 0;
				mState = STATE_EVENT;
			} else {
				more   = 0;
				mState = STATE_WAITING;
			}
			break;
		}
	}
	return line == 0;
}

// 0x8012C538
int EventScriptMgr::exec()
{
	EventCmd* cmds  = getEventCmds();
	mBits &= ~BIT_DONE;
	EventLine* line = mScript->mLines[mLine];
	if (cmds[line->mCmd].mFunc(line->mArgs) != 0) {
		mBits &= ~BIT_FAILED;
	} else {
		mBits |= BIT_FAILED;
	}
	mLine++;
	return !(mBits & BIT_FAILED);
}

// 0x8012C5E4
void EventScriptMgr::end()
{
	mScript = 0;
	mLine   = 0;
	mPhase  = 0;
	if (mBits & BIT_EVENT) {
		mBits &= ~BIT_EVENT;
		eventEnd();
	}
	mEventId = 0xFFFF;
	mState   = STATE_IDLE;
}

// 0x8012C648
int EventScriptMgr::request(int id, u16 eventId)
{
	if (id <= 0) {
		return 0;
	}
	mEventId = eventId;
	if (start(0, id) != 0) {
		if (run() != 0) {
			end();
		}
		return 1;
	}
	return 0;
}

// 0x8012C6BC
void EventScriptMgr::clearQueue()
{
	EventScriptSet* set = gEventScripts;
	if (set == 0) {
		return;
	}
	int count = set->mCount;
	for (int i = 0; i < count; i++) {
		gEventScripts->mScripts[i]->mBits = 0;
	}
	mQueued = 0;
}

// 0x8012C704
void EventScriptMgr::trigger(int id)
{
	EventScript* script = find(1, id);
	if (script != 0 && (script->mBits & 1) == 0) {
		script->mBits |= 1;
		mQueued++;
	}
}

// 0x8012C760
int EventScriptMgr::runQueued()
{
	if (gEventScripts == 0) {
		return 1;
	}
	if (mQueued <= 0) {
		return 1;
	}
	int count  = gEventScripts->mCount;
	int result = 1;
	EventScript* script;
	for (int i = 0; i < count; i++) {
		script = gEventScripts->mScripts[i];
		if ((script->mBits & 1) && start(1, script->mId) != 0) {
			script->mBits &= ~1;
			mQueued--;
			if (run() == 0) {
				result = 0;
				break;
			}
		}
	}
	return result;
}

// 0x8012C838
static int eventCallback(int arg)
{
	if (gEventMgr != 0) {
		return gEventMgr->resume(arg);
	}
	return 0;
}

// 0x8012C874
int EventScriptMgr::resume(int)
{
	exec();
	if (mBits & BIT_DONE) {
		if (run() != 0 && runQueued() != 0) {
			end();
		}
	}
	return !(mBits & BIT_DONE);
}

// 0x8012C8E4
void EventScriptMgr::signal()
{
	mBits |= BIT_SIGNAL;
}

// 0x8012C8F4
void EventScriptMgr::done()
{
	mBits |= BIT_DONE;
}

// 0x8012C904
void EventScriptMgr::update()
{
	if (gEventScripts == 0) {
		return;
	}
	switch (mState) {
	case STATE_IDLE:
		runQueued();
		break;
	case STATE_WAITING:
		if (eventStartById(mEventId, eventCallback) != 0) {
			mBits |= BIT_EVENT;
			mState = STATE_EVENT;
		}
		break;
	case STATE_EVENT:
		mTimer--;
		if (mBits & BIT_SIGNAL) {
			mBits &= ~BIT_SIGNAL;
			if (run() != 0 && runQueued() != 0) {
				end();
			}
		}
		break;
	}
}

// 0x8012C9DC
void eventScriptInit()
{
	gEventArchive = 0;
	gEventScripts = 0;
	gEventMgr     = 0;
	gEventMgr     = new EventScriptMgr;
	eventCamStart();
}

// 0x8012CA28
void eventScriptExit()
{
	eventCamStop();
	if (gEventMgr != 0) {
		delete gEventMgr;
		gEventMgr = 0;
	}
	if (gEventArchive != 0) {
		releaseResource(gEventArchive);
	}
	gEventArchive = 0;
	gEventScripts = 0;
}

// 0x8012CA80
void eventScriptReset()
{
	eventCamRestart();
	if (gEventMgr != 0) {
		gEventMgr->reset();
	}
}

// 0x8012CAB0
void eventScriptLoad(const char* name)
{
	loadArc(name, &gEventArchive);
	gEventScripts  = (EventScriptSet*)gEventArchive->mData;
	EventCmd* cmds = getEventCmds();
	int count      = gEventScripts->mCount;
	int lines;
	EventScript* script;
	EventLine* line;
	int i;
	int j;
	for (i = 0; i < count; i++) {
		script = gEventScripts->mScripts[i];
		lines  = script->mLineCount;
		for (j = 0; j < lines; j++) {
			line = script->mLines[j];
			line->mCmd      = -1;
			for (int k = 0; cmds[k].mName != 0; k++) {
				if (strncmp(line->mName, cmds[k].mName, 0x100) == 0) {
					line->mCmd = k;
					break;
				}
			}
		}
	}
	if (gEventMgr != 0) {
		gEventMgr->clearQueue();
	}
}

// 0x8012CB9C
int eventScriptRequest(int id)
{
	return eventScriptRequest(id, 0xFFFF);
}

// 0x8012CBC4
int eventScriptRequest(int id, u16 eventId)
{
	if (gEventMgr != 0) {
		return gEventMgr->request(id, eventId);
	}
	return 0;
}

// 0x8012CC08
void eventScriptSignal()
{
	if (gEventMgr != 0) {
		gEventMgr->signal();
	}
}

// 0x8012CC34
void eventScriptDone()
{
	if (gEventMgr != 0) {
		gEventMgr->done();
	}
}

// 0x8012CC60
void eventScriptTrigger(int id)
{
	if (gEventMgr != 0) {
		gEventMgr->trigger(id);
	}
}

// 0x8012CC94
void eventScriptUpdate()
{
	if (gEventMgr != 0) {
		gEventMgr->update();
	}
}

// 0x8012CCC0
static void eventCamStart()
{
	eventCamInit();
}

// 0x8012CCE0
static void eventCamStop()
{
	eventCamExit();
}

// 0x8012CD00
static void eventCamRestart()
{
	eventCamReset();
}

/** The argument blocks of the commands here */
struct EventSeArgs { // "se"
	int mId; // _00
};
struct EventSePosArgs { // "se_pos"
	int mId;    // _00
	Vec3f mPos; // _04
};
struct EventSwitchArgs { // "sw_chk_on" "sw_chk_off" "sw_on" "sw_off"
	int mBit; // _00
};
struct EventBgmArgs { // "bgm_play"
	u16 mId; // _00
};
struct EventBgmVolArgs { // "bgm_vol"
	f32 mVolume; // _00
	u16 mFrames; // _04
};
struct EventSendArgs { // "send" send a message to a set object
	int mSetId; // _00
	int mType;  // _04
	int mA;     // _08
	int mB;     // _0C
};

/** The message "send" builds */
struct EventSendMsg {
	s16 mType; // _00
	int mA;    // _04
	int mB;    // _08
};

// 0x8012CD20
EventCmd* getEventCmds()
{
	return gEventCmds;
}

// 0x8012CD2C
static int cmdSe(EventSeArgs* args)
{
	sePlay(args->mId);
	return 1;
}

// 0x8012CD54
static int cmdSePos(EventSePosArgs* args)
{
	Vec3f pos;
	pos.set(args->mPos.x, args->mPos.y, args->mPos.z);
	sePlay3D(args->mId, &pos);
	return 1;
}

// 0x8012CD98
static int cmdSwitchCheckOn(EventSwitchArgs* args)
{
	return keyBitIsSet((u16)args->mBit);
}

// 0x8012CDC0
static int cmdSwitchCheckOff(EventSwitchArgs* args)
{
	return keyBitIsSet((u16)args->mBit) == 0;
}

// 0x8012CDF0
static int cmdSwitchOn(EventSwitchArgs* args)
{
	setKeyBit((u16)args->mBit);
	return 1;
}

// 0x8012CE1C
static int cmdSwitchOff(EventSwitchArgs* args)
{
	clearKeyBit((u16)args->mBit);
	return 1;
}

// 0x8012CE48
static int cmdAlwaysTrue(void*)
{
	return 1;
}

static int cmdBgmPlay(EventBgmArgs* args);
static int cmdBgmStop(void*);
static int cmdBgmVol(EventBgmVolArgs* args);
static int cmdSend(EventSendArgs* args);
static int cmdMissionBoot(void*);

typedef int (*EventCmdFunc)(void*);

// 0x8025E720
EventCmd gEventCmds[] = {
	{ "se", 0, (EventCmdFunc)cmdSe },
	{ "se_pos", 0, (EventCmdFunc)cmdSePos },
	{ "sw_chk_on", 0, (EventCmdFunc)cmdSwitchCheckOn },
	{ "sw_chk_off", 0, (EventCmdFunc)cmdSwitchCheckOff },
	{ "sw_on", 0, (EventCmdFunc)cmdSwitchOn },
	{ "sw_off", 0, (EventCmdFunc)cmdSwitchOff },
	{ "always_true", 0, cmdAlwaysTrue },
	{ "bgm_play", 0, (EventCmdFunc)cmdBgmPlay },
	{ "bgm_stop", 0, cmdBgmStop },
	{ "bgm_vol", 0, (EventCmdFunc)cmdBgmVol },
	{ "set", 1, (EventCmdFunc)eventCmdSet },
	{ "wait", 1, (EventCmdFunc)eventCmdWait },
	{ "msg", 1, (EventCmdFunc)eventCmdMsg },
	{ "send", 0, (EventCmdFunc)cmdSend },
	{ "set_bgm", 0, (int (*)(void*))cmdSetBgm },
	{ "sunrate", 0, (int (*)(void*))setLandscapeMode },
	{ "blizzard", 0, (int (*)(void*))scriptBlizzard },
	{ "hoverleaf", 0, (int (*)(void*))createTObjGreenHoverLeaf },
	{ "sol", 0, (int (*)(void*))eventSol },
	{ "spore", 0, (int (*)(void*))createTObjGreenSpore },
	{ "race", 0, (int (*)(void*))aniRaceSetupMake },
	{ "load_egg_suit", 0, (int (*)(void*))lightEffectLoad },
	{ "load_prisoner", 0, (int (*)(void*))loadTObjMissionPrisoner },
	{ "load_bomb", 0, (int (*)(void*))loadTObjMissionBomb },
	{ "load_gold", 0, eventCmdLoadGold },
	{ "load_mgleader", 0, scriptLoadMGLeader },
	{ "load_ring", 0, scriptLoadRing },
	{ "load_goal", 0, scriptLoadGoal },
	{ "load_chicken", 0, (int (*)(void*))loadChickenArc },
	{ "load_darkgate", 0, (int (*)(void*))prepareDarkGateAssets },
	{ "load_o_cannon", 0, (int (*)(void*))prepareOrangeCannonAssets },
	{ "load_stg_title", 0, (int (*)(void*))stageTitleLoadCmd },
	{ "load_p", 0, (int (*)(void*))evLoadPlayer },
	{ "load_racer", 0, racerLoadEvent },
	{ "load_green_ene_demo", 0, greenDemoBankLoad },
	{ "talk", 1, (int (*)(void*))eventTalk },
	{ "set_talk_mode", 0, (int (*)(void*))eventSetTalkMode },
	{ "prison_save", 0, (int (*)(void*))eventPrisonSave },
	{ "gold_save", 0, (int (*)(void*))eventGoldSave },
	{ "minigame", 0, (int (*)(void*))cmdMiniGame },
	{ "ms_success", 0, cmdMissionSuccess },
	{ "ms_fail", 0, cmdMissionFail },
	{ "ms_boot", 0, cmdMissionBoot },
	{ "start_race", 0, (int (*)(void*))aniRaceStart },
	{ "set_rescue", 0, (int (*)(void*))evSetRescue },
	{ "set_race", 0, (int (*)(void*))aniRaceSetup },
	{ "takeoff_suit", 0, (int (*)(void*))evTakeoffSuit },
	{ "ptcl_test", 0, eventCmdPtclTest },
	{ "wait_fake", 1, eventCmdWaitFake },
	{ 0, 0, 0 },
};

// 0x8012CE50
static int cmdBgmPlay(EventBgmArgs* args)
{
	bgmPlay(args->mId);
	bgmFadeTo(1.0f, 0);
	return 1;
}

// 0x8012CE84
static int cmdBgmStop(void*)
{
	bgmStop();
	return 1;
}

// 0x8012CEA8
static int cmdBgmVol(EventBgmVolArgs* args)
{
	bgmFadeTo(args->mVolume, args->mFrames);
	return 1;
}

// 0x8012CED4
static int cmdSend(EventSendArgs* args)
{
	TObjAction* obj = findObjById((u16)spawnSetById(args->mSetId));
	if (obj != 0) {
		EventSendMsg msg;
		msg.mType = args->mType;
		msg.mA    = args->mA;
		msg.mB    = args->mB;
		obj->sendToHandle((ObjMsg*)&msg);
	}
	return 1;
}

// 0x8012CF44
static int cmdMissionBoot(void*)
{
	s16 msg = 0x8F;
	sendGameMsg((ObjMsg*)&msg);
	return 1;
}

