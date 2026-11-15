#define PLYSTATE_CPP
#include "object/ObjAction.h"
#include "sdk/Dolphin/pad.h"
#include "player/PlyState.h"
#include "camera/TCamera.h"
#include "player/TPlyItem.h"
#include "egg/ObjEgg.h"
#include "player/PlyPool.h"
#include "player/Player.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "camera/TCameraCtrl.h"
#include "stage/TEventCtrl.h"
#include "camera/EventCamArc.h"
#include "object/TObjBowlEgg.h"
#include "core/Pad.h"
#include "core/PlacementNew.h"
#include "Ninja/ninja.h"
#include "sound/Sound3DPlay.h"
#include "object/TObjEggRing.h"
#include "stage/TGameState.h"
#include "engine/TParticle.h"
#include "engine/FxSystem.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "engine/GroundQuery.h"
#include "egg/EggBody.h"
#include "object/TObjPlayer.h"
#include "core/VecMath.h"
#include "sound/SoundCtrl.h"
#include "object/mission/TMsnObjEmblem.h"
#include "core/TMainTask.h"

class TEmblemGet;
struct Fx;

const char* gTPlyStateAppearName = "TPlyStateAppear";
const char* gTPlyStateAscendName = "TPlyStateAscend";
const char* gTPlyStateBowlStandName = "TPlyStateBowlStand";
const char* gTPlyStateBowlWalkName = "TPlyStateBowlWalk";
const char* gTPlyStateClimbName = "TPlyStateClimb";
const char* gTPlyStateClimbDownName = "TPlyStateClimbDown";
const char* gTPlyStateCrowName = "TPlyStateCrow";
const char* gTPlyStateDamageName = "TPlyStateDamage";
const char* gTPlyStateDieName = "TPlyStateDie";
const char* gTPlyStateDieBurnName = "TPlyStateDieBurn";
const char* gTPlyStateDieDrownName = "TPlyStateDieDrown";
const char* gTPlyStateDieSandyName = "TPlyStateDieSandy";
const char* gTPlyStateDisableName = "TPlyStateDisable";
const char* gTPlyStateEnterGateName = "TPlyStateEnterGate";
const char* gTPlyStateEventName = "TPlyStateEvent";
const char* gTPlyStateEventExName = "TPlyStateEventEx";
const char* gTPlyStateFallName = "TPlyStateFall";
const char* gTPlyStateGetAnimalName = "TPlyStateGetAnimal";
const char* gTPlyStateGetItemName = "TPlyStateGetItem";
const char* gTPlyStateGlideName = "TPlyStateGlide";
const char* gTPlyStateHangEdgeName = "TPlyStateHangEdge";
const char* gTPlyStateHangEggName = "TPlyStateHangEgg";
const char* gTPlyStateHangHookName = "TPlyStateHangHook";
const char* gTPlyStateHoldObjName = "TPlyStateHoldObj";
const char* gTPlyStateInTheMirrorName = "TPlyStateInTheMirror";
const char* gTPlyStateJumpName = "TPlyStateJump";
const char* gTPlyStateJump2ndName = "TPlyStateJump2nd";
const char* gTPlyStateLaunchName = "TPlyStateLaunch";
const char* gTPlyStateLiftName = "TPlyStateLift";
const char* gTPlyStateLiftStandName = "TPlyStateLiftStand";
const char* gTPlyStateLiftThrowName = "TPlyStateLiftThrow";
const char* gTPlyStateLiftWalkName = "TPlyStateLiftWalk";
const char* gTPlyStateMountName = "TPlyStateMount";
const char* gTPlyStateOrderCreatureName = "TPlyStateOrderCreature";
const char* gTPlyStatePlaceName = "TPlyStatePlace";
const char* gTPlyStatePushWallName = "TPlyStatePushWall";
const char* gTPlyStateRemoteName = "TPlyStateRemote";
const char* gTPlyStateRideLaunchName = "TPlyStateRideLaunch";
const char* gTPlyStateRideStandName = "TPlyStateRideStand";
const char* gTPlyStateRideWalkName = "TPlyStateRideWalk";
const char* gTPlyStateRollName = "TPlyStateRoll";
const char* gTPlyStateRollBounceName = "TPlyStateRollBounce";
const char* gTPlyStateRollBounceJumpName = "TPlyStateRollBounceJump";
const char* gTPlyStateRollBrakeName = "TPlyStateRollBrake";
const char* gTPlyStateRollDashName = "TPlyStateRollDash";
const char* gTPlyStateRollDriveName = "TPlyStateRollDrive";
const char* gTPlyStateRollJumpName = "TPlyStateRollJump";
const char* gTPlyStateRollJumpThrowName = "TPlyStateRollJumpThrow";
const char* gTPlyStateSlideName = "TPlyStateSlide";
const char* gTPlyStateSlideJumpName = "TPlyStateSlideJump";
const char* gTPlyStateSlipName = "TPlyStateSlip";
const char* gTPlyStateSorryName = "TPlyStateSorry";
const char* gTPlyStateSpeakName = "TPlyStateSpeak";
const char* gTPlyStateSpinName = "TPlyStateSpin";
const char* gTPlyStateSpinBounceName = "TPlyStateSpinBounce";
const char* gTPlyStateSpinDashName = "TPlyStateSpinDash";
const char* gTPlyStateSpinFallName = "TPlyStateSpinFall";
const char* gTPlyStateSpinLaunchName = "TPlyStateSpinLaunch";
const char* gTPlyStateSpinLongJumpName = "TPlyStateSpinLongJump";
const char* gTPlyStateSpinOnLaneName = "TPlyStateSpinOnLane";
const char* gTPlyStateSpinStandName = "TPlyStateSpinStand";
const char* gTPlyStateStandName = "TPlyStateStand";
const char* gTPlyStateTransformName = "TPlyStateTransform";
const char* gTPlyStateVictoryName = "TPlyStateVictory";
const char* gTPlyStateWalkName = "TPlyStateWalk";
u16 sRollDashVoices[2]   = { 0xAE, 0xAF };
u16 sBounceJumpVoices[3] = { 0xBB, 0xBC, 0xBD };

// Each move is a TPlyState made from the player's pool
extern "C" {
u16 sPlyAnimalCheers[2] = {
	180,
	181,
}; // the two cheers for an animal
u16 sRollThrowVoices[2] = {
	191,
	192,
}; // the two throw shouts
}

// Swaps the player into a new state and returns the old one's memory to the pool
static inline void plySetState(TChrPlayer* player, void* pool, void* state)
{
	releasePlyState(pool, changePlyState(&player->mState, player, state));
}

#define SET_STATE(player, create)                            \
	do {                                                     \
		void* pool = (player)->mStates;                      \
		plySetState((player), pool, create(pool, (player))); \
	} while (0)

// Crossing frame 10 sends message 0x71 the player's id and a direction of length 150 with y set to 150 too
// 0x800B1E0C
// Message 0x71 the throw the lifted object flies off along mDir
struct PlyThrowMsg {
	inline PlyThrowMsg(u16 type)
	{
		mFrom = 0xFFFF;
		mType = type;
	}

	u16 mType; // _00
	u8 _02[2];
	u16 mFrom; // _04
	u16 _06;
	Vec3f mDir; // _08
};

// The egg states The player rolls an egg by steering a TMobileObject plyGetMobile and the egg follows it
// 0x15 0x3C 0x4D 0x50 0x52 0x53 and 0x60 go to shared handlers that have not been read

static inline int plyRandom(f32 count)
{
	return (int)(count * (3.0517578e-05f * (f32)rand()));
}

static inline void plyLoseEgg(TChrPlayer* player)
{
	plyStopAim(player);
	player->playMotion(0);
	SET_STATE(player, createPlyStateStand);
}

// Declared up front because the definitions below are in the DOL's order PlyInput the create functions then each
// state's methods with the player helpers after the states
extern "C" void* createPlyStateAppear(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateAscend(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateBowlStand(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateBowlWalk(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateClimb(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateClimbDown(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateCrow(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateDamage(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateDie(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateDieBurn(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateDieDrown(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateDieSandy(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateDisable(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateEnterGate(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateEvent(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateEventEx(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateFall(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateGetAnimal(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateGetItem(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateGlide(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateHangEdge(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateHangEgg(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateHangHook(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateHoldObj(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateInTheMirror(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateJump(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateJump2nd(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateLaunch(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateLift(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateLiftStand(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateLiftThrow(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateLiftWalk(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateMount(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateOrderCreature(void* pool, TChrPlayer* player);
extern "C" void* createPlyStatePlace(void* pool, TChrPlayer* player);
extern "C" void* createPlyStatePushWall(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRemote(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRideLaunch(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRideStand(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRideWalk(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRoll(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRollBounce(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRollBounceJump(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRollBrake(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRollDash(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRollDrive(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRollJump(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateRollJumpThrow(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSlide(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSlideJump(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSlip(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSorry(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSpeak(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSpin(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSpinBounce(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSpinDash(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSpinFall(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSpinLaunch(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSpinLongJump(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSpinOnLane(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateSpinStand(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateStand(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateTransform(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateVictory(void* pool, TChrPlayer* player);
extern "C" void* createPlyStateWalk(void* pool, TChrPlayer* player);
extern "C" void plyItemShowMatrix(TChrPlayer* player, f32* mtx);
extern "C" void plyMsgEndMove(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgClimb(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgEnterGate(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgKnockBack(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgGetAnimal(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgPickup(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgHangHook(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgLaunch(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgToStart(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgStatusBits(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgHangEgg(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgMountHangEgg(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgMountHangEggStopAim(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgSpinLaunch(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgRideLaunch(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgMountSpinLaunch(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgRail(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgKicked(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgDrive(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgIgnoreA(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgIgnoreB(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgIgnoreC(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgBounceFlag(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgStartEvent(TChrPlayer* player, ObjMsg* msg);
extern "C" int plyMsgFall(TChrPlayer* player, ObjMsg* msg);
extern "C" void plyReachToward(Vec3f* from, const Vec3f* to, Vec3f* out);
extern "C" void plyFaceEgg(TChrPlayer* player);
extern "C" void plyStartFall(TChrPlayer* player);
extern "C" void plyToHoldObj(TChrPlayer* player);
extern "C" void plyToJump(TChrPlayer* player);
extern "C" int plyTryHold(TChrPlayer* player);
extern "C" int plyCheckPickupBehind(TChrPlayer* player);
extern "C" int plyCheckCrow(TChrPlayer* player, PlyInput* input);
extern "C" void plyLetGoEgg(TChrPlayer* player);
extern "C" int plyCheckGround(TChrPlayer* player);
extern "C" int plyCheckPickup(TChrPlayer* player);
extern "C" int plyCheckLedge(TChrPlayer* player);
extern "C" int plyCheckLanding(TChrPlayer* player);
extern "C" int plyCheckFallNoEgg(TChrPlayer* player);
extern "C" int plyCheckStandNoEgg(TChrPlayer* player);
extern "C" int plyCheckMountFalling(TChrPlayer* player);
extern "C" int plyCheckOrderMount(TChrPlayer* player, PlyInput* input);
extern "C" int plyCheckPushWall(TChrPlayer* player);
extern "C" int plyCheckLandOnEgg(TChrPlayer* player);
extern "C" int plyCheckMobileLost(TChrPlayer* player);
extern "C" int plyTryRollJump(TChrPlayer* player, PlyInput* input);
extern "C" int plyCheckThrowItem(TChrPlayer* player, const PlyInput* input);
extern "C" int plyCheckDamage(TChrPlayer* player);
extern "C" int plyCheckPickUpFront(TChrPlayer* player, PlyInput* input);
extern "C" int plyCheckJump(TChrPlayer* player, PlyInput* input);
extern "C" int plyCheckEggJump(TChrPlayer* player, const PlyInput* input);
extern "C" int plyIsGroundSteep(TChrPlayer* player);
extern "C" int plyAtFootstep(TChrPlayer* player);
extern "C" int plyCheckMovePushed(TChrPlayer* player, PlyInput* input);
extern "C" void plyDropEgg(TChrPlayer* player);
extern "C" void plySteerMobile(TChrPlayer* player, PlyInput* input);
extern "C" void plySetMove(TChrPlayer* player, PlyInput* input);
extern "C" int plyGetMoveSpeed(TChrPlayer* player, f32* speed);

// Never called Its local builds EggHits' implicit constructor so the functions with EggHits locals stay in place
static void plyHitsSetup()
{
	EggHits hits;
}

extern "C" void* createPlyStateAppear(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateAppear;
}

extern "C" void* createPlyStateAscend(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateAscend;
}

extern "C" void* createPlyStateBowlStand(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateBowlStand;
}

extern "C" void* createPlyStateBowlWalk(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateBowlWalk;
}

extern "C" void* createPlyStateClimb(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateClimb;
}

extern "C" void* createPlyStateClimbDown(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateClimbDown;
}

extern "C" void* createPlyStateCrow(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateCrow;
}

extern "C" void* createPlyStateDamage(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateDamage;
}

extern "C" void* createPlyStateDie(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateDie;
}

extern "C" void* createPlyStateDieBurn(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateDieBurn;
}

extern "C" void* createPlyStateDieDrown(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateDieDrown;
}

extern "C" void* createPlyStateDieSandy(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateDieSandy;
}

extern "C" void* createPlyStateDisable(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateDisable;
}

extern "C" void* createPlyStateEnterGate(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateEnterGate;
}

extern "C" void* createPlyStateEvent(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateEvent;
}

extern "C" void* createPlyStateEventEx(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateEventEx;
}

extern "C" void* createPlyStateFall(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateFall;
}

extern "C" void* createPlyStateGetAnimal(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateGetAnimal;
}

extern "C" void* createPlyStateGetItem(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateGetItem;
}

extern "C" void* createPlyStateGlide(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateGlide;
}

extern "C" void* createPlyStateHangEdge(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateHangEdge;
}

extern "C" void* createPlyStateHangEgg(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateHangEgg;
}

extern "C" void* createPlyStateHangHook(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateHangHook;
}

extern "C" void* createPlyStateHoldObj(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateHoldObj;
}

extern "C" void* createPlyStateInTheMirror(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateInTheMirror;
}

extern "C" void* createPlyStateJump(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateJump;
}

extern "C" void* createPlyStateJump2nd(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateJump2nd;
}

extern "C" void* createPlyStateLaunch(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateLaunch;
}

extern "C" void* createPlyStateLift(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateLift;
}

extern "C" void* createPlyStateLiftStand(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateLiftStand;
}

extern "C" void* createPlyStateLiftThrow(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateLiftThrow;
}

extern "C" void* createPlyStateLiftWalk(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateLiftWalk;
}

extern "C" void* createPlyStateMount(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateMount;
}

extern "C" void* createPlyStateOrderCreature(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateOrderCreature;
}

extern "C" void* createPlyStatePlace(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStatePlace;
}

extern "C" void* createPlyStatePushWall(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStatePushWall;
}

extern "C" void* createPlyStateRemote(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRemote;
}

extern "C" void* createPlyStateRideLaunch(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRideLaunch;
}

extern "C" void* createPlyStateRideStand(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRideStand;
}

extern "C" void* createPlyStateRideWalk(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRideWalk;
}

extern "C" void* createPlyStateRoll(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRoll;
}

extern "C" void* createPlyStateRollBounce(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRollBounce;
}

extern "C" void* createPlyStateRollBounceJump(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRollBounceJump;
}

extern "C" void* createPlyStateRollBrake(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRollBrake;
}

extern "C" void* createPlyStateRollDash(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRollDash;
}

extern "C" void* createPlyStateRollDrive(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRollDrive;
}

extern "C" void* createPlyStateRollJump(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRollJump;
}

extern "C" void* createPlyStateRollJumpThrow(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateRollJumpThrow;
}

extern "C" void* createPlyStateSlide(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSlide;
}

extern "C" void* createPlyStateSlideJump(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSlideJump;
}

extern "C" void* createPlyStateSlip(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSlip;
}

extern "C" void* createPlyStateSorry(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSorry;
}

extern "C" void* createPlyStateSpeak(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSpeak;
}

extern "C" void* createPlyStateSpin(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSpin;
}

extern "C" void* createPlyStateSpinBounce(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSpinBounce;
}

extern "C" void* createPlyStateSpinDash(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSpinDash;
}

extern "C" void* createPlyStateSpinFall(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSpinFall;
}

extern "C" void* createPlyStateSpinLaunch(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSpinLaunch;
}

extern "C" void* createPlyStateSpinLongJump(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSpinLongJump;
}

extern "C" void* createPlyStateSpinOnLane(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSpinOnLane;
}

extern "C" void* createPlyStateSpinStand(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateSpinStand;
}

extern "C" void* createPlyStateStand(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateStand;
}

extern "C" void* createPlyStateTransform(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateTransform;
}

extern "C" void* createPlyStateVictory(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateVictory;
}

extern "C" void* createPlyStateWalk(void* pool, TChrPlayer* player)
{
	return new (pool) TPlyStateWalk;
}

TPlyStateAppear::TPlyStateAppear()
{
	mName = gTPlyStateAppearName;
	mSize = 0x18;
}

// Billy pops out of the egg with a burst and calls out a second later
// 0x800AEB4C
void TPlyStateAppear::enter(TChrPlayer* player)
{
	player->playMotion(0x3D);
	Vec3f* pos = player->getPos();
	f32 mtx[12];
	mtxSetTrans(mtx, pos->x, 40.0f + pos->y, pos->z);
	fxSpawnAt(1, 7, 0x4D, mtx);
	mTimer = 0;
}

// 0x800AEBD4
void TPlyStateAppear::check(TChrPlayer* player)
{
	if (player->mBody.isDone()) {
		player->playMotion(0);
		SET_STATE(player, createPlyStateStand);
	}
}

// 0x800AEC44
void TPlyStateAppear::update(TChrPlayer* player)
{
	if (mTimer++ == 60) {
		plyPlayVoice(player, 0xB9, 0);
	}
	player->updatePose();
}

TPlyStateAscend::TPlyStateAscend()
{
	mName = gTPlyStateAscendName;
	mSize = 0x14;
}

// The stick unless it is 0 is stored as the move
int TPlyStateAscend::handleInput(TChrPlayer* player, PlyInput* input)
{
	plySetMoveIfAny(player, input->mStick, input->mAngle);
	return 0;
}

// plyIsFalling is mVel.y < 0
void TPlyStateAscend::check(TChrPlayer* player)
{
	if (plyIsFalling(player) != 0) {
		plyStartFall(player);
	}
}

void TPlyStateAscend::update(TChrPlayer* player)
{
	plyAirNudge(player);
	plyApplyGravity(player);
	plyMoveAndCollide(player);
	player->updatePose();
}

// 0x800AED9C
// Pushed by the message's vector
int TPlyStateAscend::onPush(TChrPlayer* player, ObjMsg* msg)
{
	njAddVector(&player->_2F8, &((PlyVecMsg*)msg)->mVec);
	return 1;
}

// 0x800AEDC8
TPlyStateBowlStand::TPlyStateBowlStand()
{
	mName = gTPlyStateBowlStandName;
	mSize = 0x18;
}

// 0x800AEE1C
void TPlyStateBowlStand::enter(TChrPlayer* player)
{
	player->mVel = gVecZero;
}

// 0x800AEE3C
void TPlyStateBowlStand::exit(TChrPlayer* player)
{
	plyStopAim(player);
}

// Waits for the push in motion 0xB to reach frame 10 then sends the egg off
// 0x800AEE60
void TPlyStateBowlStand::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	TObjAction* egg = plyGetEgg(player);
	if (egg == NULL) {
		player->playMotion(0);
		SET_STATE(player, createPlyStateStand);
	} else if (player->mBody.mCurrent == 0xB && player->mBody.passedFrame(10.0f) != 0) {
		if (flagTest(&player->mBitsD8, 4) != 0 && plyPushEggChecked(player, egg) != 0) {
			SET_STATE(player, createPlyStateRemote);
		} else {
			plyPushEgg(player, egg);
			SET_STATE(player, createPlyStateStand);
		}
	}
}

// 0x800AEF94
void TPlyStateBowlStand::update(TChrPlayer* player)
{
	plyRollMove(player);
	player->updatePose();
}

// 0x800AEFCC
TPlyStateBowlWalk::TPlyStateBowlWalk()
{
	mName = gTPlyStateBowlWalkName;
	mSize = 0x18;
}

// 0x800AF020
void TPlyStateBowlWalk::enter(TChrPlayer* player)
{
	player->mVel = gVecZero;
}

// 0x800AF040
void TPlyStateBowlWalk::exit(TChrPlayer* player)
{
	plyStopAim(player);
}

// Waits for the push in motion 0xB to reach frame 10 then sends the egg off
// 0x800AF064
void TPlyStateBowlWalk::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	TObjAction* egg = plyGetEgg(player);
	if (egg == NULL) {
		player->playMotion(1);
		SET_STATE(player, createPlyStateWalk);
	} else if (player->mBody.mCurrent == 0xB && player->mBody.passedFrame(10.0f) != 0) {
		if (flagTest(&player->mBitsD8, 4) != 0 && plyPushEggChecked(player, egg) != 0) {
			SET_STATE(player, createPlyStateRemote);
		} else {
			plyPushEgg(player, egg);
			SET_STATE(player, createPlyStateWalk);
		}
	}
}

// 0x800AF198
void TPlyStateBowlWalk::update(TChrPlayer* player)
{
	plyRollMove(player);
	player->updatePose();
}

TPlyStateClimb::TPlyStateClimb()
{
	mName = gTPlyStateClimbName;
	mSize = 0x14;
}

// Voice 0xA3 The motion is already playing Walk starts row 0x20 on the way here
void TPlyStateClimb::enter(TChrPlayer* player)
{
	plyPlayVoice(player, 0xA3, 0);
}

// Probe 7 is the surface being climbed
void TPlyStateClimb::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyGetProbe(player, 7, 0, 0, 0) == 0) {
		plyStartFall(player);
	} else if (player->mBody.isDone() != 0) {
		player->playMotion(0);
		SET_STATE(player, createPlyStateStand);
	}
}

void TPlyStateClimb::update(TChrPlayer* player)
{
	plyHangOnLedge(player);
}

TPlyStateClimbDown::TPlyStateClimbDown()
{
	mName = gTPlyStateClimbDownName;
	mSize = 0x14;
}

// Same probe as Climb The finished motion is row 0x1D and the next state is the ledge
void TPlyStateClimbDown::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyGetProbe(player, 7, 0, 0, 0) == 0) {
		plyStartFall(player);
	} else if (player->mBody.isDone() != 0) {
		player->playMotion(0x1D);
		SET_STATE(player, createPlyStateHangEdge);
	}
}

void TPlyStateClimbDown::update(TChrPlayer* player)
{
	plyHangOnLedge(player);
}

TPlyStateCrow::TPlyStateCrow()
{
	mName = gTPlyStateCrowName;
	mSize = 0x20;
}

// Held by a crow Row 0x15 is the grab 0x16 the escape and 0x17 getting up
// 0x800AF480
void TPlyStateCrow::enter(TChrPlayer* player)
{
	mPhase  = 0;
	mTimer  = gPlyParams->mCrowHold;
	mEffect = fxSpawnAt(1, 0, 0x1C, plyGetJoint(player, 0x1B));
	plyRumble(player, 0x11);
	plyPlayVoice(player, 0xB0, 0);
}

void TPlyStateCrow::exit(TChrPlayer*)
{
	if (mEffect != 0) {
		fxStop(mEffect);
	}
}

// Once the grab motion has finished letting go of R starts the escape
// 0x800AF52C
int TPlyStateCrow::handleInput(TChrPlayer* player, PlyInput* input)
{
	if (mPhase == 0 && player->getBodyRow() == 0x15 && player->mBody.isDone() != 0 && (input->mNotHeld & PLY_BTN_R2) != 0) {
		startEscape(player);
		return 0;
	}
	return 0;
}

// 0x800AF5AC
void TPlyStateCrow::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	switch (mPhase) {
	case 1:
		if (player->mBody.isDone() != 0) {
			getUp(player);
			mPhase = 2;
		}
		break;
	case 2:
		if (--mTimer == 0) {
			player->playMotion(0x17);
			player->mFace.play(5);
			player->mLeftHand.play(5);
			player->mRightHand.play(5);
			SET_STATE(player, createPlyStateStand);
		}
		break;
	}
}

// 0x800AF698
void TPlyStateCrow::update(TChrPlayer* player)
{
	switch (mPhase) {
	case 0:
		if (tickTimer32(&mTimer) != 0) {
			startEscape(player);
		}
		break;
	}
	plyMoveAndCollide(player);
	player->updatePose();
}

void TPlyStateCrow::startEscape(TChrPlayer* player)
{
	player->playMotion(0x16);
	player->mFace.play(4);
	player->mLeftHand.play(4);
	player->mRightHand.play(4);
	mPhase = 1;
	fxStop(mEffect);
	mEffect = 0;
}

// plySpawnCrow spawns object 0xDC from joint 0x38 and stores it at _194
void TPlyStateCrow::getUp(TChrPlayer* player)
{
	plySpawnCrow(player);
	mTimer = gPlyParams->mCrowRecover;
	plyPlayVoice(player, 0x9F, 0);
	plyRumble(player, 0x12);
}

TPlyStateDamage::TPlyStateDamage()
{
	mName = gTPlyStateDamageName;
	mSize = 0x18;
}

// 0x800AF81C
void TPlyStateDamage::enter(TChrPlayer* player)
{
	mTimer = gPlyParams->mDamageTime;
	plyLeaveGround(player);
}

void TPlyStateDamage::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) == 0 && plyCheckLanding(player) == 0 && tickTimer32(&mTimer) != 0) {
		plyStartFall(player);
	}
}

void TPlyStateDamage::update(TChrPlayer* player)
{
	plyApplyGravity(player);
	plyMoveAndCollide(player);
	player->updatePose();
}

TPlyStateDie::TPlyStateDie()
{
	((FlagByte*)_14)->__ct();
	mName = gTPlyStateDieName;
	mSize = 0x18;
}

// 0x800AF944
void TPlyStateDie::enter(TChrPlayer* player)
{
	player->mVel.x = 0.0f;
	player->mVel.z = 0.0f;
}

void TPlyStateDie::update(TChrPlayer* player)
{
	plyApplyGravity(player);
	plyMoveAndCollide(player);
	player->updatePose();
	if (plyOnGround(player) != 0) {
		player->mVel = gVecZero;
	}
}

TPlyStateDieBurn::TPlyStateDieBurn()
{
	mName = gTPlyStateDieBurnName;
	mSize = 0x1c;
}

// 0x800AFA0C
void TPlyStateDieBurn::enter(TChrPlayer* player)
{
	player->playMotion(0x31);
	plyPlayVoice(player, 0xAB, 0);
	mStep = 0;
	f32 mtx[12];
	mtxSetTransV(mtx, &player->mPrevPos);
	mtxTranslate(mtx, mtx, 0.0f, 0.1f, 0.0f);
	fxSpawnAt(1, 0, 0x2D, mtx);
	mEffect = fxSpawnFollow(1, 0, 0x26, player->mMatrix);
}

// 0x800AFAAC
void TPlyStateDieBurn::exit(TChrPlayer* player)
{
	fxStop(mEffect);
}

// Once the burning motion ends the body rises
// 0x800AFAD0
void TPlyStateDieBurn::update(TChrPlayer* player)
{
	switch (mStep) {
	case 0:
		if (player->mBody.isDone()) {
			player->playMotion(0x32);
			mStep = 1;
		}
		break;
	case 1: {
		Vec3f* at = player->getPos();
		Vec3f pos;
		pos.x = at->x;
		pos.y = at->y;
		pos.z = at->z;
		pos.y += 1.6666667f;
		player->setPos(&pos);
		break;
	}
	}
	player->updateMatrix();
	player->updatePose();
}

TPlyStateDieDrown::TPlyStateDieDrown()
{
	mName = gTPlyStateDieDrownName;
	mSize = 0x14;
}

// A fast fall makes the bigger splash
// 0x800AFBF0
void TPlyStateDieDrown::enter(TChrPlayer* player)
{
	f32 fall = player->mVel.y;
	f32 mtx[12];
	mtxSetTransV(mtx, &player->mPrevPos);
	if (fall < -100.0f) {
		fxSpawnAt(1, 0, 0x20, mtx);
	} else {
		fxSpawnAt(1, 0, 0x22, mtx);
	}
	plyPlaySe(player, 0x93);
	plyPlayVoice(player, 0xAC, 0);
	player->blendMotion(0x34);
}

// 0x800AFC98
void TPlyStateDieDrown::update(TChrPlayer* player)
{
	plyUpdateMatrixPose(player);
}

TPlyStateDieSandy::TPlyStateDieSandy()
{
	mName = gTPlyStateDieSandyName;
	mSize = 0x18;
}

// 0x800AFD08
void TPlyStateDieSandy::enter(TChrPlayer* player)
{
	player->playMotion(0x36);
	plyShowShadowTask(player, 0);
	f32 mtx[12];
	mtxSetTransV(mtx, &player->mPrevPos);
	mtxTranslate(mtx, mtx, 0.0f, 0.1f, 0.0f);
	mEffect = fxSpawnAt(1, 0, 0x29, mtx);
}

// 0x800AFD88
void TPlyStateDieSandy::exit(TChrPlayer* player)
{
	fxStop(mEffect);
}

// Sinks a fifteenth of a unit a frame
// 0x800AFDAC
void TPlyStateDieSandy::update(TChrPlayer* player)
{
	Vec3f* at = player->getPos();
	Vec3f pos;
	pos.x = at->x;
	pos.y = at->y;
	pos.z = at->z;
	pos.y -= 0.06666667f;
	player->setPos(&pos);
	player->updateMatrix();
	player->updatePose();
}

TPlyStateDisable::TPlyStateDisable()
{
	mName = gTPlyStateDisableName;
	mSize = 0x14;
}

TPlyStateEnterGate::TPlyStateEnterGate()
{
	mName = gTPlyStateEnterGateName;
	mSize = 0x14;
}

// 0x800AFECC
void TPlyStateEnterGate::update(TChrPlayer* player)
{
	if (player->mBody.isDone()) {
		plyHide(player);
	} else {
		player->updatePose();
	}
}

TPlyStateEvent::TPlyStateEvent()
{
	mName = gTPlyStateEventName;
	mSize = 0x14;
}

// 0x800AFF64
void TPlyStateEvent::update(TChrPlayer* player)
{
	player->updatePose();
}

// 0x800AFF88
int TPlyStateEvent::onEventEnd(TChrPlayer* player, ObjMsg* msg)
{
	SET_STATE(player, createPlyStateStand);
	return 1;
}

// 0x800AFFDC
// Poses the player at the message's place
int TPlyStateEvent::onEventPose(TChrPlayer* player, ObjMsg* msg)
{
	PlyGateMsg* pose = (PlyGateMsg*)msg;
	plyPlayVoice(player, 0xDA, 0);
	player->playMotion(0x2F);
	player->mFace.play(6);
	player->mLeftHand.play(6);
	player->mRightHand.play(6);
	player->setPos(&pose->mPos);
	plySetAngle(player, pose->mAngle);
	player->updateMatrix();
	return 1;
}

// 0x800B0078
TPlyStateEventEx::TPlyStateEventEx()
    : mFrame(0)
{
	mName = gTPlyStateEventExName;
	mSize = 0x1c;
}

// 0x800B00CC
void TPlyStateEventEx::exit(TChrPlayer* player)
{
}

// 0x800B00D0
void TPlyStateEventEx::update(TChrPlayer* player)
{
	plyPoseAtFrame(player, 0.5f * (f32)mFrame);
	mFrame++;
}

// 0x800B0130
int TPlyStateEventEx::onEventEnd(TChrPlayer* player, ObjMsg* msg)
{
	player->playMotion(0);
	SET_STATE(player, createPlyStateStand);
	return 1;
}

TPlyStateFall::TPlyStateFall()
{
	mName = gTPlyStateFallName;
	mSize = 0x18;
}

// Row 0x33 is the fall
void TPlyStateFall::enter(TChrPlayer* player)
{
	if (player->getBodyRow() != 0x33) {
		mTimer = 40;
	}
	plySetShadow(player, 1);
}

int TPlyStateFall::handleInput(TChrPlayer*, PlyInput*)
{
	return 0;
}

// 0x800B0238
void TPlyStateFall::check(TChrPlayer* player)
{
	if (plyCheckDamage(player)) {
		return;
	}
	if (plyCheckLanding(player)) {
		return;
	}
}

// 0x800B027C
void TPlyStateFall::update(TChrPlayer* player)
{
	if (mTimer != 0 && --mTimer == 0) {
		plyPlayVoice(player, 0xA4, 0);
		player->playMotion(0x33);
	}
	plyAirControl(player);
	plyApplyGravity(player);
	plyMoveAndCollide(player);
	player->updatePose();
}

TPlyStateGetAnimal::TPlyStateGetAnimal()
{
	mName = gTPlyStateGetAnimalName;
	mSize = 0x4c;
}

// 0x800B0368
// Holds up the animal for its own camera arc and event
void TPlyStateGetAnimal::enter(TChrPlayer* player)
{
	mCam = eventCamArcPlay(1.0f, 0.0f, 7);
	mCam->attachToMtx();
	mCam->clearAutoDelete();
	mFrame = 0;
	eventStartById(player->mObjId, NULL);
	Vec3f* at = player->getPos();
	Vec3f pos;
	pos.x = at->x;
	pos.y = at->y;
	pos.z = at->z;
	mtxSetTransV(mMtx, &pos);
	mtxRotateY(mMtx, mMtx, player->mAngle);
	player->playMotion(0x2F);
	player->mFace.play(6);
	player->mLeftHand.play(6);
	player->mRightHand.play(6);
}

void TPlyStateGetAnimal::exit(TChrPlayer* player)
{
	mCam->mFlags |= 1;
}

// Three seconds in the event ends
void TPlyStateGetAnimal::check(TChrPlayer* player)
{
	if (mFrame == 180) {
		eventEnd();
		SET_STATE(player, createPlyStateStand);
	}
}

// A jingle on the second frame and one of two cheers at frame 60
void TPlyStateGetAnimal::update(TChrPlayer* player)
{
	if (mFrame++ == 1) {
		sePlayFor(0x2A2, 0);
	}
	if (player->mBody.passedFrame(60.0f) != 0) {
		int pick = 2.0f * ((f32)rand() * (1.0f / 32768.0f));
		if (pick >= 2) {
			pick = 1;
		}
		plyPlayVoice(player, sPlyAnimalCheers[pick], 0);
	}
	mtxCopy(mCam->getMtx(), mMtx);
	player->updatePose();
}

// 0x800B0574
// Where a held-up item goes 20 above the player facing back toward the camera
extern "C" void plyItemShowMatrix(TChrPlayer* player, f32* mtx)
{
	Vec3f* pos = player->getPos();
	mtxSetTrans(mtx, pos->x, 20.0f + pos->y, pos->z);
	mtxRotateY(mtx, mtx, player->getRotY() + 0x8000);
}

TPlyStateGetItem::TPlyStateGetItem()
{
	mName = gTPlyStateGetItemName;
	mSize = 0x80;
}

// 0x800B05F8
// Holds up the item a camera arc the event and the item's model over the player
void TPlyStateGetItem::enter(TChrPlayer* player)
{
	mCam = eventCamArcPlay(1.0f, 0.0f, 1);
	mCam->attachToMtx();
	mCam->clearAutoDelete();
	mFrame = 0;
	eventStartById(player->mObjId, NULL);
	Vec3f* at = player->getPos();
	Vec3f pos;
	pos.x = at->x;
	pos.y = at->y;
	pos.z = at->z;
	mtxSetTransV(mMtx, &pos);
	mtxRotateY(mMtx, mMtx, player->mAngle);
	player->playMotion(0x30);
	player->mFace.play(7);
	player->mLeftHand.play(7);
	player->mRightHand.play(7);
	plyItemShowMatrix(player, mItemMtx);
	mItem = new TPlyItem(player, ((ItemHolder*)player->_508)->firstId(), mItemMtx);
}

void TPlyStateGetItem::exit(TChrPlayer* player)
{
	plyUseHeldItem(player);
	mCam->mFlags |= 1;
	mItem->mFlags |= 1;
}

void TPlyStateGetItem::check(TChrPlayer* player)
{
	if (mFrame == 180) {
		eventEnd();
		SET_STATE(player, createPlyStateStand);
	}
}

void TPlyStateGetItem::update(TChrPlayer* player)
{
	if (player->mBody.passedFrame(60.0f) != 0) {
		plyPlayVoice(player, 0xB6, 0);
	}
	mtxCopy(mCam->getMtx(), mMtx);
	player->updatePose();
	plyItemShowMatrix(player, mItemMtx);
	mFrame++;
}

TPlyStateGlide::TPlyStateGlide()
{
	mName = gTPlyStateGlideName;
	mSize = 0x14;
}

// A held keeps the glide
int TPlyStateGlide::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mNotHeld & PLY_BTN_A) != 0) {
		plyStartFall(player);
		return 1;
	}
	plySetMoveFromInput(player, input);
	return 0;
}

void TPlyStateGlide::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (flagTest(&player->mBitsD8, 2) == 0) {
		plyStartFall(player);
		return;
	}
	if (plyCheckLanding(player) != 0) {
		return;
	}
	// Bits 0 1 3 2 and 4 of the byte at _31C
	if (flagByteTest((FlagByte*)player->_31C, 0) != 0 || flagByteTest((FlagByte*)player->_31C, 1) != 0
	    || flagByteTest((FlagByte*)player->_31C, 3) != 0 || flagByteTest((FlagByte*)player->_31C, 2) != 0
	    || flagByteTest((FlagByte*)player->_31C, 4) != 0) {
		plyStartFall(player);
	}
}

void TPlyStateGlide::update(TChrPlayer* player)
{
	plySlideMove(player);
	plyApplyGravity(player);
	plyMoveAndCollide(player);
	player->updatePose();
}

TPlyStateHangEdge::TPlyStateHangEdge()
{
	mName = gTPlyStateHangEdgeName;
	mSize = 0x18;
}

// Forty frames of hanging before the stick or A does anything
void TPlyStateHangEdge::enter(TChrPlayer* player)
{
	(void)player;
	mTimer = 40;
}

// 0x2AAA is 60 degrees and 0x6AAA is 150 measured from the facing by angleDist
// A stick past 0.5 inside 60 degrees climbs row 0x1E
// A climbs too on row 0x1F
// 0x800B0AB0
int TPlyStateHangEdge::handleInput(TChrPlayer* player, PlyInput* input)
{
	if (mTimer != 0) {
		return 0;
	}
	if (input->mStick > 0.5f) {
		int stick  = input->mAngle;
		int facing = player->mAngle;
		if (angleDist(stick, facing) < 0x2AAA) {
			player->playMotion(0x1E);
			SET_STATE(player, createPlyStateClimb);
			return 1;
		}
		if (angleDist(stick, facing) > 0x6AAA) {
			player->playMotion(4);
			SET_STATE(player, createPlyStateFall);
			return 1;
		}
	}
	if ((input->mPressed & PLY_BTN_A) != 0) {
		player->playMotion(0x1F);
		SET_STATE(player, createPlyStateClimb);
		return 1;
	}
	return 0;
}

void TPlyStateHangEdge::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) == 0 && plyGetProbe(player, 7, 0, 0, 0) == 0) {
		plyStartFall(player);
	}
}

void TPlyStateHangEdge::update(TChrPlayer* player)
{
	if (mTimer != 0) {
		mTimer--;
	}
	plyHangOnLedge(player);
}

TPlyStateHangEgg::TPlyStateHangEgg()
{
	mName = gTPlyStateHangEggName;
	mSize = 0x14;
}

void TPlyStateHangEgg::enter(TChrPlayer* player)
{
	plySetShadow(player, 0);
}

// A with an egg still in hand and only while that egg's state byte is 2
// sendHang then sends message 0x34
// 0x800B0D0C
int TPlyStateHangEgg::handleInput(TChrPlayer* player, PlyInput* input)
{
	TMobileObject* mobile;
	if ((input->mPressed & PLY_BTN_A) != 0 && (mobile = plyGetMobile(player)) != 0) {
		mobile->sendHang(0.0f, 0);
	}
	return 0;
}

// 0x800B0D54
void TPlyStateHangEgg::check(TChrPlayer* player)
{
	if (plyCheckDamage(player)) {
		return;
	}
	if (plyCheckMobileLost(player)) {
		return;
	}
}

void TPlyStateHangEgg::update(TChrPlayer* player)
{
	plyFollowMobile(player);
	player->updatePose();
}

// 0x800B0DD0
int TPlyStateHangEgg::onEggLost(TChrPlayer* player, ObjMsg* msg)
{
	plyReleaseEgg(player, plyGetEgg(player));
	plyStartFall(player);
	return 1;
}

// 0x800B0E18
int TPlyStateHangEgg::onRollJump(TChrPlayer* player, ObjMsg* msg)
{
	SET_STATE(player, createPlyStateRollJump);
	return 1;
}

TPlyStateHangHook::TPlyStateHangHook()
{
	mName = gTPlyStateHangHookName;
	mSize = 0x14;
}

// A always leaves the hook
int TPlyStateHangHook::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_A) != 0 && plyForceJump(player, input->mStick, input->mAngle) != 0) {
		plyToJump(player);
		return 1;
	}
	return 0;
}

// plyGetHangObject is the object hung from the reference at +0x130
void TPlyStateHangHook::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) == 0 && plyGetHangObject(player) == 0) {
		plyStartFall(player);
	}
}

void TPlyStateHangHook::update(TChrPlayer* player)
{
	plyHangOnObject(player);
	player->updatePose();
}

TPlyStateHoldObj::TPlyStateHoldObj()
{
	mName = gTPlyStateHoldObjName;
	mSize = 0x18;
}

// 0x800B0FFC
void TPlyStateHoldObj::enter(TChrPlayer* player)
{
	mTimer = 30;
	plySetShadow(player, 1);
	Vec3f* zero    = &gVecZero;
	player->mVel.x = zero->x;
	player->mVel.y = zero->y;
	player->mVel.z = zero->z;
}

void TPlyStateHoldObj::exit(TChrPlayer* player)
{
	player->setMotionSpeed(1.0f);
}

// 0x800B106C
void TPlyStateHoldObj::vfn_20(TChrPlayer* player)
{
	plyStopAim(player);
}

int TPlyStateHoldObj::handleInput(TChrPlayer* player, PlyInput* input)
{
	return 0;
}
// 0x800B10B4
// Holding the egg overhead B aims and throws it the stick puts it down and after a while the player puts it down anyway
void TPlyStateHoldObj::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	TObjAction* egg = plyGetEgg(player);
	if (egg == NULL) {
		SET_STATE(player, createPlyStateStand);
		return;
	}
	if (plyNoLedge(player) != 0) {
		player->mVel = gVecZero;
		plyReleaseEgg(player, egg);
		plyStartFall(player);
		return;
	}
	if (plyIsAiming(player) == 0 && mTimer != 0 && --mTimer == 0) {
		plyReleaseEgg(player, egg);
		player->blendMotion(0);
		SET_STATE(player, createPlyStateStand);
		return;
	}
	PlyInput* input = &player->mInput;
	if (plyTryRollJump(player, input) != 0) {
		return;
	}
	if (plyIsAiming(player) != 0) {
		if ((input->mNotHeld & PLY_BTN_B) != 0) {
			player->playMotion(0xB);
			SET_STATE(player, createPlyStateBowlStand);
			return;
		}
	} else if ((input->mPressed & PLY_BTN_B) != 0 && plyStartAim(player) != 0) {
		player->playMotion(0xA);
		return;
	}
	if (plyTryLetGoEgg(player, input->mStick, input->mAngle) != 0) {
		plyStopAim(player);
		SET_STATE(player, createPlyStateWalk);
		return;
	}
	if (plyCheckEggJump(player, input) != 0) {
		SET_STATE(player, createPlyStateRoll);
	}
}

void TPlyStateHoldObj::update(TChrPlayer* player)
{
	plyRollMove(player);
	player->updatePose();
}

TPlyStateInTheMirror::TPlyStateInTheMirror()
{
	mName = gTPlyStateInTheMirrorName;
	mSize = 0x14;
}

// 0x800B0FF0
void TPlyStateInTheMirror::enter(TChrPlayer* player)
{
}

// Only damage can end it
void TPlyStateInTheMirror::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
}

TPlyStateJump::TPlyStateJump()
{
	mName = gTPlyStateJumpName;
	mSize = 0x18;
}

void TPlyStateJump::enter(TChrPlayer* player)
{
	mTimer = 70;
}

// A second press of A in the air a double jump if the ability is on otherwise a glide
int TPlyStateJump::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_A) != 0) {
		if (flagTest(&player->mBitsD8, 0) != 0 && plyDoubleJump(player, input) != 0) {
			player->playMotion(5);
			SET_STATE(player, createPlyStateJump2nd);
			return 1;
		}
		if (flagTest(&player->mBitsD8, 2) != 0 && ((int (*)(TChrPlayer*, PlyInput*))plyStopInAir)(player, input) != 0) {
			player->playMotion(4);
			SET_STATE(player, createPlyStateGlide);
			return 1;
		}
	}
	if ((input->mHeld & PLY_BTN_A) != 0) {
		plyJumpHold(player);
	}
	plySetMoveFromInput(player, input);
	return 0;
}

// 0x800B152C
void TPlyStateJump::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckLandOnEgg(player) != 0) {
		return;
	}
	if (plyCheckMountFalling(player) != 0) {
		return;
	}
	if (plyCheckLanding(player) != 0) {
		return;
	}
	if (plyCheckLedge(player) != 0) {
		return;
	}
	if (player->getBodyRow() == 4 && mTimer == 0) {
		plyPlayVoice(player, 0xA4, 0);
		player->playMotion(0x33);
		SET_STATE(player, createPlyStateFall);
	}
}

// 0x800B160C
void TPlyStateJump::update(TChrPlayer* player)
{
	if (mTimer != 0) {
		mTimer--;
	}
	if (player->getBodyRow() != 4 && player->mVel.y < 0.0f) {
		player->blendMotion(4);
	}
	plyAirControl(player);
	plyApplyGravity(player);
	plyMoveAndCollide(player);
	player->updatePose();
}

// 0x800B1694
// 1 once the jump stops rising
int TPlyStateJump::vfn_30(TChrPlayer* player)
{
	return player->mVel.y >= 0.0f;
}

TPlyStateJump2nd::TPlyStateJump2nd()
{
	mName = gTPlyStateJump2ndName;
	mSize = 0x18;
}

void TPlyStateJump2nd::enter(TChrPlayer*)
{
	mTimer = 70;
}

// Holding A keeps the jump rising
int TPlyStateJump2nd::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mHeld & PLY_BTN_A) != 0) {
		plyJumpHold(player);
	}
	plySetMoveFromInput(player, input);
	return 0;
}

// 0x800B1758
void TPlyStateJump2nd::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckLanding(player) != 0) {
		return;
	}
	if (plyCheckLedge(player) != 0) {
		return;
	}
	if (player->getBodyRow() == 4) {
		SET_STATE(player, createPlyStateFall);
	}
}

void TPlyStateJump2nd::update(TChrPlayer* player)
{
	if (player->getBodyRow() != 4 && player->mVel.y < 0.0f) {
		player->blendMotion(4);
	}
	plyAirControl(player);
	plyApplyGravity(player);
	plyMoveAndCollide(player);
	player->updatePose();
}

// 0x800B1860
void TPlyStateJump2nd::vfn_20(TChrPlayer* player)
{
}

TPlyStateLaunch::TPlyStateLaunch()
{
	mName = gTPlyStateLaunchName;
	mSize = 0x14;
}

void TPlyStateLaunch::enter(TChrPlayer* player)
{
	player->playMotion(3);
}

// The damage call can change state
// 0x800B18D8
void TPlyStateLaunch::check(TChrPlayer* player)
{
	if (plyCheckDamage(player)) {
		return;
	}
}

void TPlyStateLaunch::update(TChrPlayer* player)
{
	plyApplyGravity(player);
	plyMoveAndCollide(player);
	player->updatePose();
}

TPlyStateLift::TPlyStateLift()
{
	mName = gTPlyStateLiftName;
	mSize = 0x14;
}

void TPlyStateLift::enter(TChrPlayer* player)
{
	player->playMotion(0x3B);
}

// 0x800B19B4
void TPlyStateLift::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckStandNoEgg(player) != 0) {
		return;
	}
	if (plyCheckGround(player) != 0) {
		return;
	}
	if (player->mBody.isDone()) {
		player->playMotion(0x38);
		SET_STATE(player, createPlyStateLiftStand);
	}
}

// Crossing frame 15 tells the carried object message 0x68 that the lift has taken it
// 0x800B1A54
void TPlyStateLift::update(TChrPlayer* player)
{
	if (player->mBody.passedFrame(15.0f) != 0) {
		PlyMsgT msg(0x68);
		msg.mFrom = player->mObjId;
		msg.mTo   = 0;
		sendMsgByIdU16(player->mEggRef.getId(), (ObjMsg*)&msg);
	}
	plyWalkMove(player);
	plyMoveAndStep(player);
	player->updatePose();
}

TPlyStateLiftStand::TPlyStateLiftStand()
{
	mName = gTPlyStateLiftStandName;
	mSize = 0x18;
}

// Rows 0x39 and 0x3A are the carry walk and run
// 0x800B1B28
void TPlyStateLiftStand::enter(TChrPlayer* player)
{
	switch (player->mBodyCopy.mCurrent) {
	case 0x39:
	case 0x3A:
		mBlended = 0;
		break;
	default:
		mBlended = 1;
		break;
	}
}

int TPlyStateLiftStand::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_B) != 0) {
		SET_STATE(player, createPlyStateLiftThrow);
		return 1;
	}
	if (plyCheckMovePushed(player, input) != 0) {
		SET_STATE(player, createPlyStateLiftWalk);
		return 1;
	}
	return 0;
}

// 0x800B1C08
void TPlyStateLiftStand::check(TChrPlayer* player)
{
	if (plyCheckDamage(player)) {
		return;
	}
	if (plyCheckStandNoEgg(player)) {
		return;
	}
	if (plyCheckGround(player)) {
		return;
	}
}

// 0x800B1C5C
void TPlyStateLiftStand::update(TChrPlayer* player)
{
	switch (mBlended) {
	case 0:
		if (plyAtFootstep(player) != 0) {
			player->blendMotion(0x38);
			mBlended = 1;
		}
		break;
	}
	plyWalkMove(player);
	plyMoveAndStep(player);
	player->updatePose();
}

TPlyStateLiftThrow::TPlyStateLiftThrow()
{
	mName = gTPlyStateLiftThrowName;
	mSize = 0x18;
}

void TPlyStateLiftThrow::enter(TChrPlayer* player)
{
	player->playMotion(0x3C);
	mThrown = 0;
}

// Until the throw has gone out losing the object stands at once
void TPlyStateLiftThrow::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (mThrown == 0 && plyCheckStandNoEgg(player) != 0) {
		return;
	}
	if (plyCheckGround(player) != 0) {
		return;
	}
	if (player->mBody.isDone()) {
		player->playMotion(0);
		SET_STATE(player, createPlyStateStand);
	}
}

void TPlyStateLiftThrow::update(TChrPlayer* player)
{
	if (player->mBody.passedFrame(10.0f) != 0) {
		PlyThrowMsg msg(0x71);
		Vec3f dir;

		vecFromAngleXZ(150.0f, &dir, player->mAngle);
		dir.y     = 150.0f;
		msg.mFrom = player->mObjId;
		msg.mDir  = dir;
		sendMsgByIdU16(player->mEggRef.getId(), (ObjMsg*)&msg);
		plyForgetEgg(player);
		mThrown = 1;
	}
	plyWalkMove(player);
	plyMoveAndStep(player);
	player->updatePose();
}

TPlyStateLiftWalk::TPlyStateLiftWalk()
{
	mName = gTPlyStateLiftWalkName;
	mSize = 0x14;
}

void TPlyStateLiftWalk::enter(TChrPlayer* player)
{
	player->playMotion(0x39);
}

void TPlyStateLiftWalk::exit(TChrPlayer* player)
{
	player->setMotionSpeed(1.0f);
}

int TPlyStateLiftWalk::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_B) != 0) {
		SET_STATE(player, createPlyStateLiftThrow);
		return 1;
	}
	plySetMove(player, input);
	return 0;
}

// plyGetTurnKind returning 1 is the same idle test Walk uses to stand
// 0x800B1FDC
void TPlyStateLiftWalk::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckStandNoEgg(player) != 0) {
		return;
	}
	if (plyCheckGround(player) != 0 || plyGetTurnKind(player) != 1) {
		return;
	}
	SET_STATE(player, createPlyStateLiftStand);
}

// Row 0x39 is the carry walk and 0x3A the carry run
// 0x800B2074
void TPlyStateLiftWalk::update(TChrPlayer* player)
{
	f32 speed;
	int row = plyGetMoveSpeed(player, &speed) == 0 ? 0x39 : 0x3A;

	player->setMotionSpeed(speed);
	player->playMotion(row);
	plyWalkMove(player);
	plyMoveAndStep(player);
	player->updatePose();
	plyFootstepFx(player);
}

TPlyStateMount::TPlyStateMount()
{
	mName = gTPlyStateMountName;
	mSize = 0x14;
}

// 0x800B2178
void TPlyStateMount::enter(TChrPlayer* player)
{
	player->playMotion(0x1B);
	plySetShadow(player, 0);
	camRestoreSaved(player->mObjId);
}

void TPlyStateMount::exit(TChrPlayer* player)
{
	camUndoSaved(player->mObjId);
	plySetShadow(player, 1);
}

// X gets off thrown up and back then a jump
int TPlyStateMount::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_X) != 0 && plyTryLeaveMount(player) != 0) {
		Vec3f vel;
		vel.x = 0.0f;
		vel.y = 150.0f;
		vel.z = -50.0f;
		vecRotateY(&vel, &vel, player->getRotY());
		plyThrownOff(player, &vel);
		player->playMotion(0x19);
		SET_STATE(player, createPlyStateJump);
		return 1;
	}
	return 0;
}

void TPlyStateMount::check(TChrPlayer* player)
{
	if (plyGetMount(player) == NULL) {
		plyStartFall(player);
	}
}

void TPlyStateMount::update(TChrPlayer* player)
{
	plyPlaceOnMount(player);
	player->updatePose();
}

TPlyStateOrderCreature::TPlyStateOrderCreature()
{
	mName = gTPlyStateOrderCreatureName;
	mSize = 0x18;
}

// 0x800B2364
void TPlyStateOrderCreature::enter(TChrPlayer* player)
{
	particleOnceById(gTL_PARTICLE, player->getPos(), 0, 0x20);
}

void TPlyStateOrderCreature::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (player->mBody.isDone() != 0) {
		player->playMotion(0);
		SET_STATE(player, createPlyStateStand);
	}
}

// At frame 26 of the motion the creature is told to get off
void TPlyStateOrderCreature::update(TChrPlayer* player)
{
	if ((*(volatile u32*)&mFlags.mBits & 1) == 0 && player->mBody.mFrame >= 26.0f) {
		mFlags.mBits |= 1;
		plySendMountOff(player);
	}
	plyMoveAndCollide(player);
	player->updatePose();
}

TPlyStatePlace::TPlyStatePlace()
{
	mName = gTPlyStatePlaceName;
	mSize = 0x14;
}

// 0x800B24D4
// Rides the node of the object the player is on
void TPlyStatePlace::update(TChrPlayer* player)
{
	f32 mtx[12];
	plyGetPlaceObject(player)->getNodeMatrix(player->_2A2, mtx);
	plyGetMatrix(player, mtx);
	player->updatePose();
}

// 0x800B2530
// Set down back on the ground and standing
int TPlyStatePlace::onPlaced(TChrPlayer* player, ObjMsg* msg)
{
	plyPosFromMatrix(player);
	player->_2A0 = 0xFFFF;
	player->playMotion(0);
	SET_STATE(player, createPlyStateStand);
	return 1;
}

// 0x800B25A8
int TPlyStatePlace::onPlayMotion(TChrPlayer* player, ObjMsg* msg)
{
	player->playMotion(((PlyHangMsg*)msg)->mId);
	return 0;
}

TPlyStatePushWall::TPlyStatePushWall()
{
	mName = gTPlyStatePushWallName;
	mSize = 0x18;
}

// Probe 0 is the wall
void TPlyStatePushWall::enter(TChrPlayer*)
{
	mTimer = 10;
}

int TPlyStatePushWall::handleInput(TChrPlayer* player, PlyInput* input)
{
	plySetMove(player, input);
	return 0;
}

// 0x800B2658
void TPlyStatePushWall::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	int angle;
	if (plyGetProbe(player, 0, 0, 0, 0) == 0) {
		player->blendMotion(0);
		SET_STATE(player, createPlyStateStand);
		return;
	}
	if (plyGetMove(player, 0, &angle) == 0) {
		player->blendMotion(0);
		SET_STATE(player, createPlyStateStand);
		return;
	}
	if (angleDist(player->mAngle, angle) > 0x2AAA) {
		player->blendMotion(0);
		SET_STATE(player, createPlyStateStand);
		return;
	}
	if (plyNoLedge(player) != 0) {
		plyStartFall(player);
		return;
	}
	if (plyOnSlipperyGround(player) != 0) {
		player->blendMotion(0x37);
		SET_STATE(player, createPlyStateSlide);
		return;
	}
	if (plyOnSteepGround(player) != 0) {
		player->mVel = gVecZero;
		player->blendMotion(0x25);
		SET_STATE(player, createPlyStateSlip);
		return;
	}
}

// 0x800B2860
void TPlyStatePushWall::update(TChrPlayer* player)
{
	if (mTimer != 0 && --mTimer == 0) {
		plyPlayVoice(player, 0xA8, 0);
	}
	plyMoveAndStep(player);
	player->updatePose();
}

TPlyStateRemote::TPlyStateRemote()
{
	mName = gTPlyStateRemoteName;
	mSize = 0x1c;
}

void TPlyStateRemote::enter(TChrPlayer* player)
{
	player->playMotion(0x18);
	mDone = 0;
}

void TPlyStateRemote::exit(TChrPlayer* player)
{
	plyReleaseKickedEgg(player);
}

// Ends on B or when the message says so
void TPlyStateRemote::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyNoLedge(player) != 0) {
		SET_STATE(player, createPlyStateFall);
	} else if ((player->mInput.mPressed & PLY_BTN_B) != 0) {
		SET_STATE(player, createPlyStateStand);
	} else if (mDone != 0) {
		SET_STATE(player, createPlyStateStand);
	}
}

void TPlyStateRemote::update(TChrPlayer* player)
{
	plyWalkMove(player);
	plyMoveAndStep(player);
	player->updatePose();
}

// 0x800B2A9C
int TPlyStateRemote::onRemoteEnd(TChrPlayer* player, ObjMsg* msg)
{
	mDone = 1;
	return 1;
}

TPlyStateRideLaunch::TPlyStateRideLaunch()
{
	mName = gTPlyStateRideLaunchName;
	mSize = 0x14;
}

void TPlyStateRideLaunch::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckFallNoEgg(player) != 0) {
		return;
	}
}

void TPlyStateRideLaunch::update(TChrPlayer* player)
{
}

// 0x800B2B40
int TPlyStateRideLaunch::onIgnore(TChrPlayer* player, ObjMsg* msg)
{
	return 0;
}

TPlyStateRideStand::TPlyStateRideStand()
{
	mName = gTPlyStateRideStandName;
	mSize = 0x18;
}

// 0x800B2B8C
// Coming in from the riding walk it blends to the stand on the next footstep
void TPlyStateRideStand::enter(TChrPlayer* player)
{
	plySetShadow(player, 0);
	if (player->getBodyRow() == 0x10) {
		mFromWalk = 1;
	} else {
		mFromWalk = 0;
	}
}

// A jumps off the egg
int TPlyStateRideStand::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_A) != 0 && plyJumpOffEgg(player) != 0) {
		player->playMotion(0x19);
		SET_STATE(player, createPlyStateJump);
		return 1;
	}
	if (plySetMoveIfPushed2(player, input->mStick, input->mAngle) != 0) {
		player->blendMotion(0x10);
		SET_STATE(player, createPlyStateRideWalk);
		return 1;
	}
	return 0;
}

void TPlyStateRideStand::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckFallNoEgg(player) != 0) {
		return;
	}
}

void TPlyStateRideStand::update(TChrPlayer* player)
{
	if (mFromWalk != 0) {
		if (player->mBodyCopy.passedFrame(12.0f) != 0 || player->mBodyCopy.passedFrame(27.0f) != 0) {
			player->blendMotion(0xF);
			mFromWalk = 1;
		}
	}
	plyStandOnMobile(player);
	player->updatePose();
}

TPlyStateRideWalk::TPlyStateRideWalk()
{
	mName = gTPlyStateRideWalkName;
	mSize = 0x14;
}

void TPlyStateRideWalk::enter(TChrPlayer* player)
{
	if (player->getBodyRow() != 0x10) {
		player->blendMotion(0x10);
	}
}

void TPlyStateRideWalk::exit(TChrPlayer* player)
{
	player->setMotionSpeed(1.0f);
}

int TPlyStateRideWalk::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_A) != 0 && plyJumpOffEgg(player) != 0) {
		player->playMotion(0x19);
		SET_STATE(player, createPlyStateJump);
		return 1;
	}
	plySetMove(player, input);
	return 0;
}

// Slows to the riding stand
void TPlyStateRideWalk::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckFallNoEgg(player) != 0) {
		return;
	}
	if (plyMobileUnderTen(player) != 0) {
		SET_STATE(player, createPlyStateRideStand);
	}
}

void TPlyStateRideWalk::update(TChrPlayer* player)
{
	plyRideMove(player);
	plyStandOnMobile(player);
	player->updatePose();
}

TPlyStateRoll::TPlyStateRoll()
{
	mName = gTPlyStateRollName;
	mSize = 0x18;
}

void TPlyStateRoll::enter(TChrPlayer* player)
{
	if (plyIsAiming(player) != 0) {
		player->mBody.play(10);
	}
	_14 = 0;
	plySetShadow(player, 1);
}

void TPlyStateRoll::exit(TChrPlayer* player)
{
	player->setMotionSpeed(1.0f);
}

void TPlyStateRoll::vfn_20(TChrPlayer* player)
{
	plyStopAim(player);
}

// 0x800B30A4
int TPlyStateRoll::handleInput(TChrPlayer* player, PlyInput* input)
{
	if (plyTryRollJump(player, input) != 0) {
		return 1;
	}
	if ((input->mPressed & PLY_BTN_R) != 0 && plyTryRollDash(player) != 0) {
		SET_STATE(player, createPlyStateRollDash);
		return 1;
	}
	if (plyIsAiming(player) != 0) {
		if ((input->mNotHeld & PLY_BTN_B) != 0) {
			player->mBody.play(0xB);
			SET_STATE(player, createPlyStateBowlWalk);
			return 1;
		}
	} else {
		if ((input->mPressed & PLY_BTN_B) != 0 && plyStartAim(player) != 0) {
			player->mBody.play(10);
			return 0;
		}
	}
	if (plyCheckBrake(player, input->mStick, input->mAngle) != 0) {
		player->blendMotion(0x29);
		SET_STATE(player, createPlyStateRollBrake);
		return 1;
	}
	plySetMove(player, input);
	return 0;
}

// 0x800B3230
void TPlyStateRoll::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckEggLost(player) != 0) {
		plyLoseEgg(player);
	} else if (plyMobileStopped(player) != 0) {
		if (plyIsAiming(player) != 0) {
			plyPlayCopyMotion(player, &player->mBodyCopy, 6);
		} else {
			player->blendMotion(6);
		}
		SET_STATE(player, createPlyStateHoldObj);
	}
}

// 0x800B3328
void TPlyStateRoll::update(TChrPlayer* player)
{
	Vec3f vel;
	vel       = player->mVel;
	f32 speed = njScalor(&vel);
	if (plyIsAiming(player) == 0) {
		if (speed < 1.0f) {
			player->playMotion(7);
		} else {
			player->playMotion(8);
		}
	} else {
		if (speed < 1.0f) {
			plyPlayCopyMotion(player, &player->mBodyCopy, 7);
		} else {
			plyPlayCopyMotion(player, &player->mBodyCopy, 8);
		}
	}
	plyRollMove(player);
	player->updatePose();
	plyFootstepFx(player);
}

TPlyStateRollBounce::TPlyStateRollBounce()
{
	mName = gTPlyStateRollBounceName;
	mSize = 0x1c;
}

void TPlyStateRollBounce::enter(TChrPlayer* player)
{
	mStep = 0;
	clearFlagByte(&mFlags);
	mTimer = 1;
	mDelay = 10;
	player->playMotion(0x24);
	player->mVel = gVecZero;
}

// 0x800B34D0
int TPlyStateRollBounce::handleInput(TChrPlayer* player, PlyInput* input)
{
	switch (mStep) {
	case 0:
	case 1:
		if ((input->mReleased & PLY_BTN_A) != 0) {
			mFlags.mBits |= 4;
		}
		break;
	}
	plySetMove(player, input);
	return 0;
}

// 0x800B352C
void TPlyStateRollBounce::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckMobileLost(player) != 0) {
		return;
	}
	if (mStep == 1) {
		if ((mFlags.mBits & 4) != 0) {
			player->playMotion(0x27);
			plyGetMobile(player)->startHop();
			mStep = 2;
		} else if (mTimer == 0) {
			plyPlayVoice(player, 0xA1, 0);
			plyGetMobile(player)->startSlam();
			SET_STATE(player, createPlyStateRollDrive);
			return;
		}
	}
	if ((mFlags.mBits & 2) != 0) {
		plyRumble(player, 0x16);
		if (plyTryBounceJump(player) != 0) {
			SET_STATE(player, createPlyStateRollBounceJump);
		} else {
			plyDropEgg(player);
			plyStartFall(player);
		}
	}
}

// 0x800B3674
void TPlyStateRollBounce::update(TChrPlayer* player)
{
	if (mTimer != 0) {
		mTimer--;
	}
	switch (mStep) {
	case 0:
		if (tickTimer16((u16*)&mDelay) != 0) {
			if ((mFlags.mBits & 4) != 0) {
				player->playMotion(0x27);
				plyGetMobile(player)->startHop();
				mStep = 2;
			} else {
				plyPlayVoice(player, 0xA2, 0);
				mTimer = 0x28;
				mStep  = 1;
			}
		}
		break;
	case 1:
		if (mTimer != 0) {
			mTimer--;
		}
		break;
	}
	plyFollowMobile(player);
	player->updatePose();
}

// 0x800B3758
int TPlyStateRollBounce::onLand(TChrPlayer* player)
{
	if (plyGetMobile(player)->onSpecialGround() != 0) {
		return 0;
	}
	mFlags.mBits |= 2;
	return 0;
}

TPlyStateRollBounceJump::TPlyStateRollBounceJump()
{
	mName = gTPlyStateRollBounceJumpName;
	mSize = 0x18;
}

// 0x800B3800
void TPlyStateRollBounceJump::enter(TChrPlayer* player)
{
	int voice = plyRandom(3.0f);
	if (voice >= 3) {
		voice = 2;
	}
	plyPlayVoice(player, sBounceJumpVoices[voice], 0);
}

// 0x800B3888
int TPlyStateRollBounceJump::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((mFlags.mBits & 1) != 0) {
		return 0;
	}
	if ((input->mPressed & PLY_BTN_A) != 0) {
		plyRumble(player, 0x17);
		if (plyTryBounce(player) != 0) {
			SET_STATE(player, createPlyStateRollBounce);
			return 1;
		}
	}
	if ((input->mPressed & PLY_BTN_R) != 0 && plyTrySpinDash(player, input->mStick, input->mAngle) != 0) {
		plyPlayVoice(player, 0xA6, 0);
		SET_STATE(player, createPlyStateSpinDash);
		return 1;
	}
	if ((input->mPressed & PLY_BTN_B) != 0 && plyTryThrow(player) != 0) {
		plyFaceEgg(player);
		int voice = plyRandom(2.0f);
		if (voice >= 2) {
			voice = 1;
		}
		plyPlayVoice(player, sRollThrowVoices[voice], 0);
		player->playMotion(0x28);
		SET_STATE(player, createPlyStateRollJumpThrow);
		return 1;
	}
	plySetMove(player, input);
	return 0;
}

// 0x800B3A60
void TPlyStateRollBounceJump::check(TChrPlayer* player)
{
	f32 amount;
	int angle;
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckMobileLost(player) != 0) {
		return;
	}
	if ((mFlags.mBits & 1) != 0) {
		plyDropEgg(player);
		plyStartFall(player);
	} else if (plyGetMove(player, &amount, &angle) != 0) {
		plyGetMobile(player)->steer(amount, angle);
	}
}

void TPlyStateRollBounceJump::update(TChrPlayer* player)
{
	plyFollowMobile(player);
	player->updatePose();
}

// 0x800B3B30
int TPlyStateRollBounceJump::onLand(TChrPlayer* player)
{
	if (plyGetMobile(player)->onSpecialGround() != 0) {
		return 0;
	}
	mFlags.mBits |= 1;
	return 0;
}

TPlyStateRollBrake::TPlyStateRollBrake()
{
	mName = gTPlyStateRollBrakeName;
	mSize = 0x20;
}

void TPlyStateRollBrake::enter(TChrPlayer* player)
{
	mFrame      = 0;
	mDustLeft   = 3;
	mVoiceDelay = 0x14;
}

void TPlyStateRollBrake::vfn_20(TChrPlayer* player)
{
	plyStopAim(player);
}

// 0x800B3C10
int TPlyStateRollBrake::handleInput(TChrPlayer* player, PlyInput* input)
{
	if (plyTryRollJump(player, input) != 0) {
		return 1;
	}
	if (plyIsAiming(player) != 0) {
		if ((input->mNotHeld & PLY_BTN_B) != 0) {
			player->playMotion(0xB);
			SET_STATE(player, createPlyStateBowlWalk);
			return 1;
		}
	} else {
		if ((input->mPressed & PLY_BTN_B) != 0 && plyStartAim(player) != 0) {
			player->playMotion(10);
			return 0;
		}
	}
	if (plyTryEndBrake(player, input->mStick, input->mAngle) != 0) {
		SET_STATE(player, createPlyStateRoll);
		return 1;
	}
	((int (*)(TChrPlayer*, f32, int))plyHasMobile)(player, input->mStick, input->mAngle);
	return 0;
}

void TPlyStateRollBrake::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckEggLost(player) != 0) {
		plyLoseEgg(player);
	} else {
		Vec3f vel;
		vel = player->mVel;
		if (vecLenXZ(&vel) < 5.0f) {
			player->blendMotion(6);
			SET_STATE(player, createPlyStateHoldObj);
		}
	}
}

// 0x800B3E34
void TPlyStateRollBrake::update(TChrPlayer* player)
{
	if (mVoiceDelay != 0 && --mVoiceDelay == 0) {
		plyPlayVoice(player, 0xA8, 0);
	}
	if ((mFrame++ & 7) == 0 && mDustLeft != 0) {
		Vec3f pos;
		mDustLeft--;
		mtxGetPos(plyGetJoint(player, 0x3A), &pos);
		particleOnceById(gTL_PARTICLE, &pos, 0, 0x36);
	}
	plyBrakeMove(player);
	plyMoveAndCollide(player);
	player->updatePose();
	plyFootstepFx(player);
}

TPlyStateRollDash::TPlyStateRollDash()
{
	mName = gTPlyStateRollDashName;
	mSize = 0x18;
}

// 0x800B3F4C
void TPlyStateRollDash::enter(TChrPlayer* player)
{
	clearFlagWord((u32*)&mFlags);
	int voice = plyRandom(2.0f);
	if (voice >= 2) {
		voice = 1;
	}
	plyPlayVoice(player, sRollDashVoices[voice], 0);
}

int TPlyStateRollDash::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_A) != 0 && plyGetMobile(player)->longJump() != 0) {
		plyPlayVoice(player, 0xA7, 0);
		plyMountMobile(player);
		player->playMotion(0x27);
		SET_STATE(player, createPlyStateSpinLongJump);
		return 1;
	}
	plySetMove(player, input);
	return 0;
}

void TPlyStateRollDash::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckEggLost(player) != 0) {
		plyLoseEgg(player);
	} else if ((mFlags.mBits & 1) != 0) {
		plyGetMobile(player)->startAir();
		SET_STATE(player, createPlyStateRoll);
	}
}

void TPlyStateRollDash::update(TChrPlayer* player)
{
	plyDashMove(player);
	player->updatePose();
	plyFootstepFx(player);
}

int TPlyStateRollDash::onFall(TChrPlayer* player)
{
	plyMountMobile(player);
	player->playMotion(0x27);
	plyGetMobile(player)->startDrive();
	SET_STATE(player, createPlyStateSpin);
	return 1;
}

int TPlyStateRollDash::onDashEnd(TChrPlayer* player)
{
	mFlags.mBits |= 1;
	return 0;
}

TPlyStateRollDrive::TPlyStateRollDrive()
{
	mName = gTPlyStateRollDriveName;
	mSize = 0x1c;
}

void TPlyStateRollDrive::enter(TChrPlayer* player)
{
	player->playMotion(0x24);
	player->mVel = gVecZero;
	mStep        = 0;
	mTimer       = 0x1E;
}

// 0x800B42E0
void TPlyStateRollDrive::check(TChrPlayer* player)
{
	if (plyCheckMobileLost(player) != 0) {
		return;
	}
	switch (mStep) {
	case 2:
		if (tickTimer32(&mTimer) != 0 && plyGetMobile(player)->onSpecialGround() == 0) {
			plyDropEgg(player);
			plyStartFall(player);
		}
		break;
	}
}

// 0x800B4360
void TPlyStateRollDrive::update(TChrPlayer* player)
{
	switch (mStep) {
	case 0:
		if (tickTimer32(&mTimer) != 0) {
			plyPlayVoice(player, 0xA1, 0);
			player->playMotion(0x27);
			plyGetMobile(player)->startSlam();
			mStep = 1;
		}
		break;
	}
	plyFollowMobile(player);
	player->updatePose();
}

// 0x800B43EC
int TPlyStateRollDrive::onLand(TChrPlayer* player)
{
	Vec3f pos;
	pos = *player->getPos();
	camShakeNearSoft(200.0f, &pos);
	plyRumble(player, 0x15);
	mStep  = 2;
	mTimer = 0x1E;
	return 1;
}

TPlyStateRollJump::TPlyStateRollJump()
{
	mName = gTPlyStateRollJumpName;
	mSize = 0x18;
}

void TPlyStateRollJump::enter(TChrPlayer* player)
{
	plySetShadow(player, 0);
}

// 0x800B44EC
int TPlyStateRollJump::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_B) != 0 && plyTryThrow(player) != 0) {
		int voice = plyRandom(2.0f);
		if (voice >= 2) {
			voice = 1;
		}
		plyPlayVoice(player, sRollThrowVoices[voice], 0);
		player->playMotion(0x28);
		SET_STATE(player, createPlyStateRollJumpThrow);
		return 1;
	}
	if ((input->mPressed & PLY_BTN_R) != 0 && plyTrySpinDash(player, input->mStick, input->mAngle) != 0) {
		plyPlayVoice(player, 0xA6, 0);
		SET_STATE(player, createPlyStateSpinDash);
		return 1;
	}
	if ((input->mPressed & PLY_BTN_A) != 0 && plyTryBounce(player) != 0) {
		SET_STATE(player, createPlyStateRollBounce);
		return 1;
	}
	if ((input->mHeld & PLY_BTN_A) != 0) {
		plyHoldJump(player);
	}
	player->setMove(input->mStick, input->mAngle);
	return 0;
}

// 0x800B46B4
void TPlyStateRollJump::check(TChrPlayer* player)
{
	f32 amount;
	int angle;
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckMobileLost(player) != 0) {
		return;
	}
	if ((mFlags.mBits & 1) != 0) {
		plyRumble(player, 3);
		plyFaceEgg(player);
		if (plyFindLanding(player) != 0) {
			plyGetMobile(player)->mBody.mVel = gVecZero;
			plySetAngle(player, player->mAngle);
			player->playMotion(6);
			SET_STATE(player, createPlyStateHoldObj);
		} else {
			plyReleaseEgg(player, plyGetEgg(player));
			player->playMotion(0xD);
			SET_STATE(player, createPlyStateFall);
		}
	} else {
		if (plyGetMove(player, &amount, &angle) != 0) {
			plyGetMobile(player)->steer(amount, angle);
		}
	}
}

void TPlyStateRollJump::update(TChrPlayer* player)
{
	plyJumpMove(player);
	player->updatePose();
}

// 0x800B4850
int TPlyStateRollJump::onLand(TChrPlayer* player)
{
	if (plyGetMobile(player)->onSpecialGround() != 0) {
		return 0;
	}
	mFlags.mBits |= 1;
	return 0;
}

TPlyStateRollJumpThrow::TPlyStateRollJumpThrow()
{
	mName = gTPlyStateRollJumpThrowName;
	mSize = 0x18;
}

void TPlyStateRollJumpThrow::enter(TChrPlayer* player)
{
	mStep        = 0;
	player->mVel = gVecZero;
}

int TPlyStateRollJumpThrow::handleInput(TChrPlayer* player, PlyInput* input)
{
	plySetMove(player, input);
	return 0;
}

// 0x800B4948
void TPlyStateRollJumpThrow::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	switch (mStep) {
	case 0:
		if (plyCheckMobileLost(player) != 0) {
			return;
		}
		break;
	case 1:
		if (player->mBody.isDone() != 0) {
			plyStartFall(player);
			break;
		}
	}
}

// Frame 15 of the motion is where the egg leaves
// 0x800B49CC
void TPlyStateRollJumpThrow::update(TChrPlayer* player)
{
	switch (mStep) {
	case 0:
		if (player->mBody.mFrame >= 15.0f) {
			plyFaceEgg(player);
			plyRumble(player, 0xC);
			plySlamEgg(player);
			mStep = 1;
		} else {
			plyJumpMove(player);
		}
		player->updatePose();
		break;
	case 1:
		player->updatePose();
		break;
	}
}

TPlyStateSlide::TPlyStateSlide()
{
	mName = gTPlyStateSlideName;
	mSize = 0x18;
}

// Row 0x37 A on the ground jumps and landing comes back here instead of to Stand
void TPlyStateSlide::enter(TChrPlayer* player)
{
	player->playMotion(0x37);
	mFrames = 0;
}

int TPlyStateSlide::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_A) != 0 && plyOnGround(player) != 0) {
		plyLeaveGround(player);
		player->mVel.y = 140.0f;
		SET_STATE(player, createPlyStateSlideJump);
		return 1;
	}
	player->setMove(input->mStick, input->mAngle);
	return 0;
}

// 0x800B4B94
void TPlyStateSlide::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyNoLedge(player) != 0) {
		plyStartFall(player);
		return;
	}
	if (plyOnSlipperyGround(player) != 0) {
		return;
	}
	PlyParams* params = gPlyParams;
	Vec3f vel;
	vel = player->mVel;
	// vecLenXZ is the horizontal speed it uses x and z and ignores y
	if (vecLenXZ(&vel) <= params->mRunSpeed) {
		player->blendMotion(0);
		SET_STATE(player, createPlyStateStand);
	}
}

// 0x800B4C64
void TPlyStateSlide::update(TChrPlayer* player)
{
	plySlideOnEgg(player);
	plyApplyGravity(player);
	plyMoveAndStep(player);
	player->updatePose();
	if ((mFrames & 3) == 0) {
		particleOnceById(gTL_PARTICLE, player->getPos(), 0, 1);
	}
	mFrames++;
}

TPlyStateSlideJump::TPlyStateSlideJump()
{
	mName = gTPlyStateSlideJumpName;
	mSize = 0x14;
}

// 0x800B4D3C
void TPlyStateSlideJump::check(TChrPlayer* player)
{
	if (plyOnGround(player) != 0) {
		particleOnceById(gTL_PARTICLE, player->getPos(), 0, 0);
		SET_STATE(player, createPlyStateSlide);
	}
}

void TPlyStateSlideJump::update(TChrPlayer* player)
{
	plyAirControl(player);
	plyApplyGravity(player);
	plyMoveAndCollide(player);
	player->updatePose();
}

TPlyStateSlip::TPlyStateSlip()
{
	mName = gTPlyStateSlipName;
	mSize = 0x18;
}

// Row 0x25 Probe 5 is the ground under the player the same slot plyLeaveGround clears
void TPlyStateSlip::enter(TChrPlayer* player)
{
	player->playMotion(0x25);
	mTimer  = 20;
	mFrames = 0;
}

// 0x800B4EA0
void TPlyStateSlip::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyNoLedge(player) != 0) {
		plyStartFall(player);
		return;
	}
	Vec3f normal;
	if (plyGetProbe(player, 5, 0, 0, &normal) == 0) {
		return;
	}
	if (normal.y >= gPlyParams->mSlipNormal) {
		if (mTimer != 0 && --mTimer == 0) {
			player->blendMotion(0);
			SET_STATE(player, createPlyStateStand);
			return;
		}
		if (normal.y > 0.98f) {
			player->blendMotion(0);
			SET_STATE(player, createPlyStateStand);
		}
	} else {
		mTimer = 20;
	}
}

// 0x800B4FDC
void TPlyStateSlip::update(TChrPlayer* player)
{
	plySlideDownSlope(player);
	plyApplyGravity(player);
	plyMoveAndStep(player);
	player->updatePose();
	if ((mFrames & 3) == 0) {
		Vec3f pos = *player->getPos();
		pos.y += 5.0f;
		particleOnceById(gTL_PARTICLE, &pos, 0, 1);
	}
	mFrames++;
}

TPlyStateSorry::TPlyStateSorry()
{
	mName = gTPlyStateSorryName;
	mSize = 0x48;
}

// 0x800B50AC
// Bows to the camera on its own arc
void TPlyStateSorry::enter(TChrPlayer* player)
{
	mCam = eventCamArcPlay(1.0f, 0.0f, 3);
	mCam->attachToMtx();
	mCam->clearAutoDelete();
	Vec3f* at = player->getPos();
	Vec3f pos;
	pos.x = at->x;
	pos.y = at->y;
	pos.z = at->z;
	mtxSetTransV(mMtx, &pos);
	mtxRotateY(mMtx, mMtx, player->mAngle);
	player->playMotion(0x2C);
	player->mFace.play(1);
	player->mLeftHand.play(1);
	player->mRightHand.play(1);
}

void TPlyStateSorry::exit(TChrPlayer* player)
{
	mCam->mFlags |= 1;
}

void TPlyStateSorry::update(TChrPlayer* player)
{
	if (player->mBody.passedFrame(175.0f) != 0) {
		plyPlayVoice(player, 0xAD, 0);
	}
	mtxCopy(mCam->getMtx(), mMtx);
	player->updatePose();
}

TPlyStateSpeak::TPlyStateSpeak()
{
	mName = gTPlyStateSpeakName;
	mSize = 0x14;
}

void TPlyStateSpeak::update(TChrPlayer* player)
{
	player->updatePose();
}

// 0x800B5290
int TPlyStateSpeak::onSpeakEnd(TChrPlayer* player, ObjMsg* msg)
{
	((ObjRef*)player->_2BC)->setId(0xFFFF);
	player->playMotion(0);
	SET_STATE(player, createPlyStateStand);
	return 1;
}

TPlyStateSpin::TPlyStateSpin()
{
	mName = gTPlyStateSpinName;
	mSize = 0x18;
}

void TPlyStateSpin::enter(TChrPlayer* player)
{
	plySetShadow(player, 0);
}

int TPlyStateSpin::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_A) != 0) {
		if (plyTrySpinStand(player, 0.0f) != 0) {
			plyPlayVoice(player, 0xA3, 0);
			SET_STATE(player, createPlyStateSpinStand);
			return 1;
		}
		if (plyTryHop(player) != 0) {
			return 0;
		}
	}
	if (input->mStick > 0.0f && plyTrySpinStand(player, input->mStick) != 0) {
		plyPlayVoice(player, 0xA3, 0);
		SET_STATE(player, createPlyStateSpinStand);
		return 1;
	}
	plySteerMobile(player, input);
	return 0;
}

// Thirty slow frames and the spin settles
// 0x800B5494
void TPlyStateSpin::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckMobileLost(player) != 0) {
		return;
	}
	if (plyIsMobileSlow(player) != 0) {
		if (++mStillFrames > 0x1E && plyTrySpinStand(player, 0.0f) != 0) {
			plyPlayVoice(player, 0xA3, 0);
			SET_STATE(player, createPlyStateSpinStand);
		}
	} else {
		mStillFrames = 0;
	}
}

void TPlyStateSpin::update(TChrPlayer* player)
{
	plyFollowMobile(player);
	player->updatePose();
}

TPlyStateSpinBounce::TPlyStateSpinBounce()
{
	mName = gTPlyStateSpinBounceName;
	mSize = 0x1c;
}

void TPlyStateSpinBounce::enter(TChrPlayer* player)
{
	mStep = 0;
	clearFlagByte(&mFlags);
	mTimer = 1;
	mDelay = 10;
	player->playMotion(0x24);
	player->mVel = gVecZero;
}

// 0x800B5664
int TPlyStateSpinBounce::handleInput(TChrPlayer* player, PlyInput* input)
{
	switch (mStep) {
	case 0:
	case 1:
		if ((input->mReleased & PLY_BTN_A) != 0) {
			mFlags.mBits |= 4;
		}
		break;
	}
	plySetMove(player, input);
	return 0;
}

// 0x800B56C0
void TPlyStateSpinBounce::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckMobileLost(player) != 0) {
		return;
	}
	if (mStep == 1) {
		if ((mFlags.mBits & 4) != 0) {
			player->playMotion(0x27);
			plyGetMobile(player)->startHop();
			mStep = 2;
		} else if (mTimer == 0) {
			plyPlayVoice(player, 0xA1, 0);
			plyGetMobile(player)->startSlam();
			SET_STATE(player, createPlyStateRollDrive);
			return;
		}
	}
	if ((mFlags.mBits & 2) != 0) {
		plyRumble(player, 0x16);
		if (plyTryBounceJump(player) != 0) {
			SET_STATE(player, createPlyStateRollBounceJump);
		} else {
			plyDropEgg(player);
			plyStartFall(player);
		}
	}
}

// 0x800B5808
void TPlyStateSpinBounce::update(TChrPlayer* player)
{
	if (mTimer != 0) {
		mTimer--;
	}
	switch (mStep) {
	case 0:
		if (tickTimer16((u16*)&mDelay) != 0) {
			if ((mFlags.mBits & 4) != 0) {
				player->playMotion(0x27);
				plyGetMobile(player)->startHop();
				mStep = 2;
			} else {
				plyPlayVoice(player, 0xA2, 0);
				mTimer = 0x28;
				mStep  = 1;
			}
		}
		break;
	case 1:
		if (mTimer != 0) {
			mTimer--;
		}
		break;
	}
	plyFollowMobile(player);
	player->updatePose();
}

// 0x800B58EC
int TPlyStateSpinBounce::onLand(TChrPlayer* player)
{
	if (plyGetMobile(player)->onSpecialGround() != 0) {
		return 0;
	}
	mFlags.mBits |= 2;
	return 0;
}

TPlyStateSpinDash::TPlyStateSpinDash()
{
	mName = gTPlyStateSpinDashName;
	mSize = 0x18;
}

void TPlyStateSpinDash::enter(TChrPlayer* player)
{
	plySetShadow(player, 0);
	clearFlagWord((u32*)&mFlags);
}

// 0x800B59D0
int TPlyStateSpinDash::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_A) != 0 && plyTryHop(player) != 0) {
		return 0;
	}
	plySteerMobile(player, input);
	return 0;
}

// 0x800B5A30
void TPlyStateSpinDash::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckMobileLost(player) != 0) {
		return;
	}
	if ((mFlags.mBits & 1) != 0) {
		SET_STATE(player, createPlyStateSpin);
	}
}

void TPlyStateSpinDash::update(TChrPlayer* player)
{
	plyFollowMobile(player);
	player->updatePose();
}

int TPlyStateSpinDash::onDashEnd(TChrPlayer* player)
{
	mFlags.mBits |= 1;
	return 0;
}

TPlyStateSpinFall::TPlyStateSpinFall()
{
	mName = gTPlyStateSpinFallName;
	mSize = 0x14;
}

// 0x800B5B4C
void TPlyStateSpinFall::check(TChrPlayer* player)
{
	if (plyCheckDamage(player)) {
		return;
	}
	if (plyCheckMobileLost(player)) {
		return;
	}
}

void TPlyStateSpinFall::update(TChrPlayer* player)
{
	plyFollowMobile(player);
	player->updatePose();
}

int TPlyStateSpinFall::onLand(TChrPlayer* player)
{
	plyGetMobile(player)->startDrive();
	SET_STATE(player, createPlyStateSpin);
	return 0;
}

TPlyStateSpinLaunch::TPlyStateSpinLaunch()
{
	mName = gTPlyStateSpinLaunchName;
	mSize = 0x14;
}

// 0x800B5C78
void TPlyStateSpinLaunch::check(TChrPlayer* player)
{
	if (plyCheckDamage(player)) {
		return;
	}
	if (plyCheckMobileLost(player)) {
		return;
	}
}

void TPlyStateSpinLaunch::update(TChrPlayer* player)
{
	plyFollowMobile(player);
	player->updatePose();
}

int TPlyStateSpinLaunch::onLand(TChrPlayer* player)
{
	plyGetMobile(player)->startDrive();
	SET_STATE(player, createPlyStateSpin);
	return 0;
}

TPlyStateSpinLongJump::TPlyStateSpinLongJump()
{
	mName = gTPlyStateSpinLongJumpName;
	mSize = 0x18;
}

void TPlyStateSpinLongJump::enter(TChrPlayer* player)
{
	plySetShadow(player, 0);
	clearFlagWord((u32*)&mFlags);
}

int TPlyStateSpinLongJump::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_A) != 0 && plyTrySpinBounce(player) != 0) {
		SET_STATE(player, createPlyStateSpinBounce);
		return 1;
	}
	plySetMove(player, input);
	return 0;
}

// 0x800B5E74
void TPlyStateSpinLongJump::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckMobileLost(player) != 0) {
		return;
	}
	if ((mFlags.mBits & 1) != 0) {
		plyGetMobile(player)->startDrive();
		SET_STATE(player, createPlyStateSpin);
	}
}

void TPlyStateSpinLongJump::update(TChrPlayer* player)
{
	plyKeepSteering(player);
	plyFollowMobile(player);
	player->updatePose();
}

int TPlyStateSpinLongJump::onFall(TChrPlayer* player)
{
	plyMountMobile(player);
	player->playMotion(0x27);
	plyGetMobile(player)->stopSettle();
	SET_STATE(player, createPlyStateSpinFall);
	return 0;
}

int TPlyStateSpinLongJump::onLand(TChrPlayer* player)
{
	mFlags.mBits |= 1;
	return 0;
}

TPlyStateSpinOnLane::TPlyStateSpinOnLane()
{
	mName = gTPlyStateSpinOnLaneName;
	mSize = 0x14;
}

// 0x800B601C
void TPlyStateSpinOnLane::check(TChrPlayer* player)
{
	if (plyCheckDamage(player)) {
		return;
	}
	if (plyCheckMobileLost(player)) {
		return;
	}
}

void TPlyStateSpinOnLane::update(TChrPlayer* player)
{
	plyFollowMobile(player);
	player->updatePose();
}

int TPlyStateSpinOnLane::onLaneEnd(TChrPlayer* player)
{
	SET_STATE(player, createPlyStateSpin);
	return 0;
}

TPlyStateSpinStand::TPlyStateSpinStand()
{
	mName = gTPlyStateSpinStandName;
	mSize = 0x18;
}

int TPlyStateSpinStand::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_R) != 0 && plyTrySpinDash(player, input->mStick, input->mAngle) != 0) {
		plyPlayVoice(player, 0xA6, 0);
		SET_STATE(player, createPlyStateSpinDash);
		return 1;
	}
	return 0;
}

// 0x800B61D4
void TPlyStateSpinStand::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckMobileLost(player) != 0) {
		return;
	}
	if ((mFlags.mBits & 1) != 0) {
		plyFaceEgg(player);
		if (plyFindLanding(player) != 0) {
			plyGetMobile(player)->mBody.mVel = gVecZero;
			plySetAngle(player, player->mAngle);
			player->playMotion(6);
			SET_STATE(player, createPlyStateHoldObj);
		} else {
			plyReleaseEgg(player, plyGetEgg(player));
			player->playMotion(0xD);
			SET_STATE(player, createPlyStateFall);
		}
	}
}

void TPlyStateSpinStand::update(TChrPlayer* player)
{
	plyFollowMobile(player);
	player->updatePose();
}

// 0x800B6334
int TPlyStateSpinStand::onLand(TChrPlayer* player)
{
	if (plyGetMobile(player)->onSpecialGround() != 0) {
		return 0;
	}
	mFlags.mBits |= 1;
	return 0;
}

TPlyStateStand::TPlyStateStand()
{
	mName = gTPlyStateStandName;
	mSize = 0x1c;
}

// 0x800B63D4
void TPlyStateStand::enter(TChrPlayer* player)
{
	plySetShadow(player, 1);
	if (flagTest(&player->mBitsDC, 5) != 0 || flagTest(&player->mBitsDC, 0xE) != 0) {
		mPhase = 5;
		player->playMotion(0x35);
		return;
	}
	s16 row = player->mBodyCopy.mCurrent;
	switch (row) {
	case 0:
		mPhase = 2;
		mTimer = 300;
		break;
	case 1:
	case 2:
		mPhase = 0;
		break;
	default:
		mPhase = 1;
		mTimer = 30;
		break;
	}
}

// 0x800B6498
int TPlyStateStand::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_B) != 0 && plyCheckPickupBehind(player) != 0) {
		return 1;
	}
	if ((input->mPressed & PLY_BTN_B2) != 0 && plyTryTalk(player) != 0) {
		player->playMotion(0);
		SET_STATE(player, createPlyStateSpeak);
		return 1;
	}
	if (plyCheckThrowItem(player, input) != 0) {
		return 1;
	}
	if ((input->mPressed & PLY_BTN_B) != 0 && (u16)plyGiveEgg(player) != 0xFFFF) {
		SET_STATE(player, createPlyStateLift);
		return 1;
	}
	if (plyCheckOrderMount(player, input) != 0) {
		return 1;
	}
	if (plyTryRollJump(player, input) != 0) {
		return 1;
	}
	if (plyCheckCrow(player, input) != 0) {
		return 1;
	}
	if (plyCheckJump(player, input) != 0) {
		plyToJump(player);
		return 1;
	}
	if (plyCheckMovePushed(player, input) != 0) {
		SET_STATE(player, createPlyStateWalk);
		return 1;
	}
	return 0;
}

// 0x800B6678
void TPlyStateStand::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckPickup(player) != 0) {
		return;
	}
	if (plyNoLedge(player) != 0) {
		plyStartFall(player);
		return;
	}
	if (plyOnSlipperyGround(player) != 0) {
		player->blendMotion(0x37);
		SET_STATE(player, createPlyStateSlide);
		return;
	}
	if (plyOnSteepGround(player) != 0) {
		player->mVel = gVecZero;
		player->blendMotion(0x25);
		SET_STATE(player, createPlyStateSlip);
		return;
	}
	if (plyTryHold(player) != 0) {
		return;
	}
}

// Standing still long enough plays a fidget row 0x11 for five loops and finishes with row 0x12
// 0x800B67A0
void TPlyStateStand::update(TChrPlayer* player)
{
	switch (mPhase) {
	case 1:
		if (--mTimer == 0) {
			if (player->getBodyRow() != 0) {
				player->blendMotion(0);
			}
			mPhase = 2;
			mTimer = 300;
		}
		break;
	case 0:
		if (plyAtFootstep(player) != 0) {
			player->blendMotion(0);
			mPhase = 2;
			mTimer = 300;
		}
		break;
	case 2:
		if (--mTimer == 0) {
			player->blendMotion(0x11);
			mLoops = 0;
			player->mBody.mFlags &= ~0x100;
			mPhase = 3;
		}
		break;
	case 3:
		if (((volatile u32&)player->mBody.mFlags & 0x100) != 0) {
			player->mBody.mFlags &= ~0x100;
			mLoops++;
			if (mLoops == 5) {
				plyPlayVoice(player, 0xCA, 0xB4);
				player->blendMotion(0x12);
				mPhase = 4;
			}
		}
		break;
	case 4:
		if (player->mBody.isDone()) {
			player->blendMotion(0);
			mTimer = 300;
			mPhase = 2;
		}
		break;
	case 5:
		if (flagTest(&player->mBitsDC, 5) == 0 && flagTest(&player->mBitsDC, 0xE) == 0) {
			player->playMotion(0);
			mTimer = 300;
			mPhase = 2;
		}
		break;
	}
	plyUpdateSpeakCursor(player);
	plyWalkMove(player);
	plyMoveAndStep(player);
	player->updatePose();
}

// 0x800B69A4
TPlyStateTransform::TPlyStateTransform()
    : mFrame(0)
    , mEffect(0)
{
	mName = gTPlyStateTransformName;
	mSize = 0x1c;
}

void TPlyStateTransform::exit(TChrPlayer* player)
{
	fxStop(mEffect);
}

// 0x800B6A20
// The effect starts at frame 260
void TPlyStateTransform::update(TChrPlayer* player)
{
	if (mFrame == 260) {
		f32 mtx[12];
		mtxCopy(mtx, player->mMatrix);
		mEffect = fxSpawnAt(1, 0, 0x39, mtx);
	} else if (mFrame == 280) {
		player->vfn_D4();
	} else if (mFrame == 300) {
		fxStop(mEffect);
		mEffect = NULL;
	}
	plyPoseAtFrame(player, 0.5f * (f32)mFrame);
	if (mFrame < 1000) {
		mFrame++;
	}
}

// 0x800B6AF8
int TPlyStateTransform::onEventEnd(TChrPlayer* player, ObjMsg* msg)
{
	player->playMotion(0);
	SET_STATE(player, createPlyStateStand);
	return 1;
}

TPlyStateVictory::TPlyStateVictory()
{
	mName = gTPlyStateVictoryName;
	mSize = 0x14;
}

// 0x800B6BA8
// Tells the game runs the player's event fades the music out and shows the emblem
void TPlyStateVictory::enter(TChrPlayer* player)
{
	player->playMotion(0x2D);
	player->mFace.play(0);
	player->mLeftHand.play(0);
	player->mRightHand.play(0);
	ObjMsg msg;
	msg.mType = 0xA2;
	sendGameMsg(&msg);
	eventStartById(player->mObjId, NULL);
	bgmFadeTo(0.0f, 60);
	spawnEmblemGet(player, player->getPos(), player->mAngle);
}

void TPlyStateVictory::update(TChrPlayer* player)
{
	if (player->mBody.passedFrame(370.0f) != 0) {
		plyPlayVoice(player, 0xBE, 0);
	}
	player->updatePose();
}

TPlyStateWalk::TPlyStateWalk()
{
	mName = gTPlyStateWalkName;
	mSize = 0x18;
}

// 0x800B6CEC
void TPlyStateWalk::enter(TChrPlayer* player)
{
	mFrames = 0;
	plySetShadow(player, 1);
	switch (player->mBody.mCurrent) {
	case 0xB:
		mPhase = 1;
		break;
	default:
		player->playMotion(1);
		mPhase = 2;
		break;
	}
}

void TPlyStateWalk::exit(TChrPlayer* player)
{
	player->setMotionSpeed(1.0f);
}

// 0x800B6D84
int TPlyStateWalk::handleInput(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_B) != 0 && plyCheckPickupBehind(player) != 0) {
		return 1;
	}
	if (plyCheckThrowItem(player, input) != 0) {
		return 1;
	}
	if (plyCheckOrderMount(player, input) != 0) {
		return 1;
	}
	if (plyTryRollJump(player, input) != 0) {
		return 1;
	}
	if (plyCheckCrow(player, input) != 0) {
		return 1;
	}
	if (plyTryGrabEdge(player, input->mStick, input->mAngle) != 0) {
		player->playMotion(0x20);
		SET_STATE(player, createPlyStateClimbDown);
		return 1;
	}
	if (plyCheckJump(player, input) != 0) {
		plyToJump(player);
		return 1;
	}
	if (plyCheckPickUpFront(player, input) != 0) {
		plyToHoldObj(player);
		return 1;
	}
	plySetMove(player, input);
	return 0;
}

// 0x800B6EF4
void TPlyStateWalk::check(TChrPlayer* player)
{
	if (plyCheckDamage(player) != 0) {
		return;
	}
	if (plyCheckPickup(player) != 0) {
		return;
	}
	if (plyNoLedge(player) != 0) {
		plyStartFall(player);
		return;
	}
	if (plyOnSlipperyGround(player) != 0) {
		player->blendMotion(0x37);
		SET_STATE(player, createPlyStateSlide);
		return;
	}
	if (plyOnSteepGround(player) != 0) {
		player->mVel = gVecZero;
		player->blendMotion(0x25);
		SET_STATE(player, createPlyStateSlip);
		return;
	}
	if (plyTryHold(player) != 0) {
		return;
	}
	if (plyCheckPushWall(player) != 0 || plyGetTurnKind(player) != 1) {
		return;
	}
	SET_STATE(player, createPlyStateStand);
}

// Row 1 is the walk and row 2 the run
// 0x800B7070
void TPlyStateWalk::update(TChrPlayer* player)
{
	PlyParams* params = gPlyParams;
	f32 speed         = player->_340.z;
	f32 motionSpeed;
	int row;

	if (speed < params->mRunSpeed) {
		f32 slope;
		plyGetMove(player, &slope, 0);
		if (slope >= params->mSteepSlope) {
			motionSpeed = params->mRunMotionSpeed;
			row         = 2;
		} else {
			motionSpeed = params->mWalkMotionSpeed;
			row         = 1;
		}
	} else {
		motionSpeed = params->mRunMotionSpeed;
		row         = 2;
	}

	switch (mPhase) {
	case 2:
		player->setMotionSpeed(motionSpeed);
		player->playMotion(row);
		break;
	case 1:
		player->mBodyCopy.mSpeed = motionSpeed;
		plyPlayCopyMotion(player, &player->mBodyCopy, row);
		if (player->mBody.isDone()) {
			player->mBody.set(player->mBodyCopy.mCurrent);
			player->mBody.mFrame = player->mBodyCopy.mFrame;
			player->mBody.mSpeed = motionSpeed;
			mPhase               = 2;
		}
		break;
	}

	// A puff of dust every eighth frame at a run
	if (speed > params->mRunSpeed && (mFrames++ & 7) == 0) {
		Vec3f pos = *player->getPos();
		pos.y += 2.0f;
		particleOnceById(gTL_PARTICLE, &pos, 0, 1);
	}

	plyWalkMove(player);
	plyMoveAndStep(player);
	player->updatePose();
	plyFootstepFx(player);
}

// The message handlers the states' onMessage functions share
// 0x800B7220
extern "C" void plyMsgEndMove(TChrPlayer* player, ObjMsg* msg)
{
	plyEndMove(player);
}

// 0x800B7240
// Drops the egg and climbs
extern "C" int plyMsgClimb(TChrPlayer* player, ObjMsg* msg)
{
	TObjAction* egg = plyGetEgg(player);
	if (egg != NULL) {
		plyReleaseEgg(player, egg);
	}
	plyMsgEndMove(player, msg);
	player->playMotion(3);
	SET_STATE(player, createPlyStateAscend);
	return 1;
}

// 0x800B72C8
// Moves the player to a gate and walks it in
extern "C" int plyMsgEnterGate(TChrPlayer* player, ObjMsg* msg)
{
	PlyGateMsg* gate = (PlyGateMsg*)msg;
	player->setPos(&gate->mPos);
	plySetAngle(player, gate->mAngle);
	player->updateMatrix();
	player->playMotion(0x41);
	SET_STATE(player, createPlyStateEnterGate);
	return 1;
}

// 0x800B7358
// Knocked back thrown up and away from the way the player faces
extern "C" int plyMsgKnockBack(TChrPlayer* player, ObjMsg* msg)
{
	Vec3f dir;
	int angle = player->mAngle;
	vecFromAngleXZ(-30.0f, &dir, angle);
	dir.y          = 100.0f;
	player->mVel.x = dir.x;
	player->mVel.y = dir.y;
	player->mVel.z = dir.z;
	plySetAngle(player, angle);
	player->playMotion(0xC);
	plyReleaseEgg(player, plyGetEgg(player));
	plyPlayVoice(player, 0xA9, 0);
	plyRumble(player, 0xE);
	SET_STATE(player, createPlyStateDamage);
	return 1;
}

// 0x800B742C
extern "C" int plyMsgGetAnimal(TChrPlayer* player, ObjMsg* msg)
{
	SET_STATE(player, createPlyStateGetAnimal);
	return 1;
}

// 0x800B7484
extern "C" int plyMsgPickup(TChrPlayer* player, ObjMsg* msg)
{
	return plyTakePickup(player, (u8*)msg);
}

// 0x800B74A4
// Grabs a hook
extern "C" int plyMsgHangHook(TChrPlayer* player, ObjMsg* msg)
{
	PlyHangMsg* hang = (PlyHangMsg*)msg;
	if (plySetHangObject(player, hang->mId, hang->mPart) != 0) {
		player->playMotion(0x1C);
		SET_STATE(player, createPlyStateHangHook);
		return 1;
	}
	return 0;
}

// 0x800B7528
// Launched with the message's velocity
extern "C" int plyMsgLaunch(TChrPlayer* player, ObjMsg* msg)
{
	PlyVecMsg* launch = (PlyVecMsg*)msg;
	plyEndMove(player);
	player->mVel.x = launch->mVec.x;
	player->mVel.y = launch->mVec.y;
	player->mVel.z = launch->mVec.z;
	SET_STATE(player, createPlyStateLaunch);
	return 1;
}

// 0x800B75A0
// Back to the start point
extern "C" int plyMsgToStart(TChrPlayer* player, ObjMsg* msg)
{
	player->reset();
	plyToStartPoint(player);
	camResetCamera((s8)player->mPlayer);
	player->playMotion(0);
	SET_STATE(player, createPlyStateStand);
	return 1;
}

// 0x800B761C
extern "C" int plyMsgStatusBits(TChrPlayer* player, ObjMsg* msg)
{
	return plyGetStatusBits(player) | 0x8000;
}

// 0x800B7640
extern "C" int plyMsgHangEgg(TChrPlayer* player, ObjMsg* msg)
{
	SET_STATE(player, createPlyStateHangEgg);
	return 1;
}

// 0x800B7698
extern "C" int plyMsgMountHangEgg(TChrPlayer* player, ObjMsg* msg)
{
	plyMountMobile(player);
	player->playMotion(0x27);
	SET_STATE(player, createPlyStateHangEgg);
	return 1;
}

// 0x800B7700
extern "C" int plyMsgMountHangEggStopAim(TChrPlayer* player, ObjMsg* msg)
{
	plyMountMobile(player);
	plyStopAim(player);
	player->playMotion(0x27);
	SET_STATE(player, createPlyStateHangEgg);
	return 1;
}

// 0x800B7770
extern "C" int plyMsgSpinLaunch(TChrPlayer* player, ObjMsg* msg)
{
	SET_STATE(player, createPlyStateSpinLaunch);
	return 1;
}

// 0x800B77C8
extern "C" int plyMsgRideLaunch(TChrPlayer* player, ObjMsg* msg)
{
	SET_STATE(player, createPlyStateRideLaunch);
	return 1;
}

// 0x800B7820
extern "C" int plyMsgMountSpinLaunch(TChrPlayer* player, ObjMsg* msg)
{
	plyMountMobile(player);
	player->playMotion(0x27);
	SET_STATE(player, createPlyStateSpinLaunch);
	return 1;
}

// 0x800B7888
// Puts the egg on a rail
extern "C" int plyMsgRail(TChrPlayer* player, ObjMsg* msg)
{
	PlyRailMsg* rail      = (PlyRailMsg*)msg;
	TMobileObject* mobile = plyGetMobile(player);
	mobile->startRail(rail->mPath, rail->mPoint, rail->mSpeed);
	SET_STATE(player, createPlyStateSpinOnLane);
	return 1;
}

// 0x800B78F8
// Kicked passes the kick on to the held egg at a speed of at least 60 either way and lets go
struct PlyKickMsg : public PlyMsg {
	inline PlyKickMsg(u16 type)
	    : PlyMsg(type)
	{
	}
	void* mPath;  // _08
	void* mPoint; // _0C
	f32 mSpeed;   // _10
};

extern "C" int plyMsgKicked(TChrPlayer* player, ObjMsg* msg)
{
	PlyRailMsg* in  = (PlyRailMsg*)msg;
	TObjAction* egg = plyGetEgg(player);
	if (egg == NULL) {
		return 0;
	}
	PlyKickMsg kick(0x6B);
	kick.mFrom  = player->mObjId;
	kick.mPath  = in->mPath;
	kick.mPoint = in->mPoint;
	f32 least   = 60.0f;
	f32 speed   = in->mSpeed;
	if ((f32)__fabs(speed) < least) {
		if (speed < 0.0f) {
			least = -60.0f;
		}
		kick.mSpeed = least;
	} else {
		kick.mSpeed = speed;
	}
	if (egg->sendToHandle((ObjMsg*)&kick) != 0) {
		player->mVel = gVecZero;
		plyReleaseEgg(player, egg);
		player->playMotion(0);
		SET_STATE(player, createPlyStateStand);
		return 1;
	}
	return 0;
}

// 0x800B7A28
extern "C" int plyMsgDrive(TChrPlayer* player, ObjMsg* msg)
{
	plyMountMobile(player);
	player->playMotion(0x27);
	plyGetMobile(player)->startDrive();
	SET_STATE(player, createPlyStateSpin);
	return 1;
}

// 0x800B7A9C
extern "C" int plyMsgIgnoreA(TChrPlayer* player, ObjMsg* msg)
{
	return 1;
}

// 0x800B7AA4
extern "C" int plyMsgIgnoreB(TChrPlayer* player, ObjMsg* msg)
{
	return 1;
}

// 0x800B7AAC
extern "C" int plyMsgIgnoreC(TChrPlayer* player, ObjMsg* msg)
{
	return 1;
}

// 0x800B7AB4
extern "C" int plyMsgBounceFlag(TChrPlayer* player, ObjMsg* msg)
{
	return plySetFlagBounce(player);
}

// 0x800B7AD4
// Starts an event
extern "C" int plyMsgStartEvent(TChrPlayer* player, ObjMsg* msg)
{
	player->playMotion(0x2E);
	player->mFace.play(2);
	player->mLeftHand.play(2);
	player->mRightHand.play(2);
	SET_STATE(player, createPlyStateEvent);
	return 1;
}

// 0x800B7B58
// Loses the egg and falls
extern "C" int plyMsgFall(TChrPlayer* player, ObjMsg* msg)
{
	plyForgetEgg(player);
	player->playMotion(4);
	SET_STATE(player, createPlyStateFall);
	return 1;
}

// Motion rows named in comments below are rows of the table at gPlayerBodyMotions
// 0x800B7BC0
// Where to stand to reach from one point toward another just past the target or short of whatever is in the way
extern "C" void plyReachToward(Vec3f* from, const Vec3f* to, Vec3f* out)
{
	EggHits hits;
	Vec3f dir;
	vecSub(&dir, to, from);
	vecScaleAdd(1.0f + vecNormalize(&dir, &dir), &dir, &dir, from);
	if (meshColliProbeLine(hits.mHits, from, &dir, 1) != NULL) {
		Vec3f back;
		vecSub(&back, &hits.mHits[0].mPos, from);
		vecScaleAdd(vecNormalize(&back, &back) - 1.0f, out, &back, from);
	} else {
		out->x = to->x;
		out->y = to->y;
		out->z = to->z;
	}
}

// 0x800B7CB0
// Moves the player toward the egg it holds
extern "C" void plyFaceEgg(TChrPlayer* player)
{
	TObjAction* egg = plyGetEgg(player);
	if (egg != NULL) {
		Vec3f* at = egg->getPos();
		Vec3f eggPos;
		eggPos.x = at->x;
		eggPos.y = at->y;
		eggPos.z = at->z;
		at       = player->getPos();
		Vec3f pos;
		pos.x = at->x;
		pos.y = at->y;
		pos.z = at->z;
		Vec3f out;
		plyReachToward(&eggPos, &pos, &out);
		player->setPos(&out);
	}
}

extern "C" void plyStartFall(TChrPlayer* player)
{
	player->playMotion(4);
	SET_STATE(player, createPlyStateFall);
}

// 0x800B7DBC
extern "C" void plyToHoldObj(TChrPlayer* player)
{
	player->playMotion(6);
	SET_STATE(player, createPlyStateHoldObj);
}

// 0x800B7E18
extern "C" void plyToJump(TChrPlayer* player)
{
	player->playMotion(3);
	SET_STATE(player, createPlyStateJump);
}

// 0x800B7E74
extern "C" int plyTryHold(TChrPlayer* player)
{
	if (plyHasNormalModels(player) == 0) {
		return 0;
	}
	if (((int (*)(TChrPlayer*))plyTryPickUpBehind)(player) == 0) {
		return 0;
	}
	player->mVel = gVecZero;
	player->playMotion(0x1A);
	SET_STATE(player, createPlyStateHoldObj);
	return 1;
}

// 0x800B7F24
// B picks up an egg from behind
extern "C" int plyCheckPickupBehind(TChrPlayer* player)
{
	if (plyHasNormalModels(player) == 0) {
		return 0;
	}
	if (plyFindEggBehind(player) == 0xFFFF) {
		return 0;
	}
	player->playMotion(0x1A);
	SET_STATE(player, createPlyStateStand);
	return 1;
}

// 0x800B7FB8
// R starts the Crow the player stops and calls
extern "C" int plyCheckCrow(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_R2) == 0) {
		return 0;
	}
	if (plyHasNormalModels(player) == 0) {
		return 0;
	}
	if (plyStopIfGrounded(player) == 0) {
		return 0;
	}
	player->playMotion(0x15);
	player->mFace.play(3);
	player->mLeftHand.play(3);
	player->mRightHand.play(3);
	SET_STATE(player, createPlyStateCrow);
	return 1;
}

// 0x800B8080
// Tells the egg the player let go of it
extern "C" void plyLetGoEgg(TChrPlayer* player)
{
	PlyMsg msg(0x69);
	msg.mFrom = player->mObjId;
	sendMsgByIdU16(player->mEggRef.getId(), (ObjMsg*)&msg);
	plyForgetEgg(player);
}

// 0x800B80DC
// The ground checks no ledge to stand on slippery ground or ground too steep
extern "C" int plyCheckGround(TChrPlayer* player)
{
	if (plyNoLedge(player) != 0) {
		plyLetGoEgg(player);
		player->playMotion(4);
		SET_STATE(player, createPlyStateFall);
		return 1;
	}
	if (plyOnSlipperyGround(player) != 0) {
		plyLetGoEgg(player);
		player->blendMotion(0x37);
		SET_STATE(player, createPlyStateSlide);
		return 1;
	}
	if (plyOnSteepGround(player) != 0) {
		plyLetGoEgg(player);
		player->mVel = gVecZero;
		player->blendMotion(0x25);
		SET_STATE(player, createPlyStateSlip);
		return 1;
	}
	return 0;
}

// 0x800B822C
extern "C" int plyCheckPickup(TChrPlayer* player)
{
	if (plyBounceFlag(player) != 0) {
		SET_STATE(player, createPlyStateVictory);
		return 1;
	}
	int item = *(u32*)(player->_508 + 0x10) != 0 ? *(int*)(*(u8**)(player->_508 + 0x14) + 8) : 0xFFFF;
	if (item != 0xFFFF) {
		if (flagTest(&gPlayerFlags, 2) == 0) {
			plyUseHeldItem(player);
		} else {
			SET_STATE(player, createPlyStateGetItem);
		}
		return 1;
	}
	return 0;
}

// 0x800B8318
// Catches a ledge
extern "C" int plyCheckLedge(TChrPlayer* player)
{
	if (plyTryCatchLedge(player) == 0) {
		return 0;
	}
	player->playMotion(0x1C);
	player->mLeftHand.play(9);
	player->mRightHand.play(0xC);
	SET_STATE(player, createPlyStateHangEdge);
	return 1;
}

// On touching the ground dust the landing motion then whichever state fits what is going on
// 0x800B83A8
extern "C" int plyCheckLanding(TChrPlayer* player)
{
	if (player->mVel.y > 0.0f || plyOnGround(player) == 0) {
		return 0;
	}
	Vec3f pos;
	pos = *player->getPos();
	particleOnceById(gTL_PARTICLE, &pos, 0, 0);
	player->playMotion(0x13);
	plyRumble(player, 2);
	if (plyBounceFlag(player) != 0) {
		SET_STATE(player, createPlyStateVictory);
	} else {
		int item = *(u32*)(player->_508 + 0x10) != 0 ? *(int*)(*(u8**)(player->_508 + 0x14) + 8) : 0xFFFF;
		if (item != 0xFFFF) {
			if (flagTest(&gPlayerFlags, 2) == 0) {
				plyUseHeldItem(player);
			} else {
				SET_STATE(player, createPlyStateGetItem);
			}
		} else if (plyIsGroundSteep(player) != 0) {
			player->mVel = gVecZero;
			player->blendMotion(0x25);
			SET_STATE(player, createPlyStateSlip);
		} else {
			SET_STATE(player, createPlyStateStand);
		}
	}
	return 1;
}

// 0x800B85A4
// Falls once the egg is gone
extern "C" int plyCheckFallNoEgg(TChrPlayer* player)
{
	if (plyGetEgg(player) != NULL) {
		return 0;
	}
	player->playMotion(4);
	SET_STATE(player, createPlyStateFall);
	return 1;
}

// 0x800B861C
extern "C" int plyCheckStandNoEgg(TChrPlayer* player)
{
	if (player->mEggRef.getId() != 0xFFFF) {
		return 0;
	}
	player->playMotion(0);
	SET_STATE(player, createPlyStateStand);
	return 1;
}

// 0x800B869C
// Onto a mount while falling
extern "C" int plyCheckMountFalling(TChrPlayer* player)
{
	if (player->mVel.y >= 0.0f) {
		return 0;
	}
	if (plyTryChangeMount(player) == 0) {
		return 0;
	}
	SET_STATE(player, createPlyStateMount);
	return 1;
}

// 0x800B8724
// X orders the creature being ridden
extern "C" int plyCheckOrderMount(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_X) == 0) {
		return 0;
	}
	if (plyStopIfMounted(player) == 0) {
		return 0;
	}
	plyPlayVoice(player, 0xA5, 0);
	player->playMotion(0x18);
	SET_STATE(player, createPlyStateOrderCreature);
	return 1;
}

// 0x800B87C0
// Pushes against a wall the stick points into within 30 degrees
extern "C" int plyCheckPushWall(TChrPlayer* player)
{
	Vec3f normal;
	int angle;
	if (plyGetMove(player, 0, &angle) == 0) {
		return 0;
	}
	if (plyGetProbe(player, 0, 0, 0, &normal) == 0) {
		return 0;
	}
	int wall = vecAngleXZ(&normal);
	if (angleDist(angle + 0x8000, wall) > 0x1555) {
		return 0;
	}
	plySetAngle(player, wall + 0x8000);
	player->mVel = gVecZero;
	player->blendMotion(0x2B);
	SET_STATE(player, createPlyStatePushWall);
	return 1;
}

// 0x800B88C8
// Lands on an egg and rides it
extern "C" int plyCheckLandOnEgg(TChrPlayer* player)
{
	if (plyHasNormalModels(player) == 0) {
		return 0;
	}
	if (plyTryLandOnEgg(player) == 0xFFFF) {
		return 0;
	}
	Vec3f* at = player->getPos();
	Vec3f pos;
	pos.x = at->x;
	pos.y = at->y;
	pos.z = at->z;
	particleOnceById(gTL_PARTICLE, &pos, 0, 0);
	player->playMotion(0x3F);
	SET_STATE(player, createPlyStateRideStand);
	return 1;
}

// 0x800B899C
// Falls when the egg being ridden is gone
extern "C" int plyCheckMobileLost(TChrPlayer* player)
{
	if (plyGetMobile(player) == NULL) {
		plyForgetEgg(player);
		SET_STATE(player, createPlyStateFall);
		return 1;
	}
	return 0;
}

// 0x800B8A10
// A launches the egg and jumps after it
extern "C" int plyTryRollJump(TChrPlayer* player, PlyInput* input)
{
	if (plyHasNormalModels(player) == 0) {
		return 0;
	}
	if ((input->mPressed & PLY_BTN_A) == 0) {
		return 0;
	}
	if (plyTryLaunchEgg(player, input->mStick, input->mAngle) == 0) {
		return 0;
	}
	Vec3f* at = player->getPos();
	Vec3f pos;
	pos.x = at->x;
	pos.y = at->y;
	pos.z = at->z;
	sePlay3D(0x78, &pos);
	plyPlayVoice(player, 0xA6, 0);
	plyRumble(player, 9);
	plyMountMobile(player);
	player->playMotion(0x26);
	SET_STATE(player, createPlyStateRollJump);
	return 1;
}

// 0x800B8B1C
// Y throws the worn item
extern "C" int plyCheckThrowItem(TChrPlayer* player, const PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_Y) == 0) {
		return 0;
	}
	if (plyThrowWornItem(player) == 0) {
		return 0;
	}
	player->playMotion(0x3E);
	return 1;
}

// Being hit beats anything else
// 0x800B8B7C
extern "C" int plyCheckDamage(TChrPlayer* player)
{
	if (plySearchDamage(player) == 0) {
		return 0;
	}
	plyEndMove(player);
	player->blendMotion(0xC);
	SET_STATE(player, createPlyStateDamage);
	return 1;
}

// 0x800B8BFC
extern "C" int plyCheckPickUpFront(TChrPlayer* player, PlyInput* input)
{
	if (plyHasNormalModels(player) != 0) {
		return plyTryPickUpInFront(player, input->mStick, input->mAngle);
	}
	return 0;
}

// 0x800B8C4C
// A jumps
extern "C" int plyCheckJump(TChrPlayer* player, PlyInput* input)
{
	if ((input->mPressed & PLY_BTN_A) != 0) {
		return plyTryJump(player, input->mStick, input->mAngle);
	}
	return 0;
}

// 0x800B8C88
extern "C" int plyCheckEggJump(TChrPlayer* player, const PlyInput* input)
{
	return plyTryEggJump(player, input->mStick, input->mAngle);
}

// 0x800B8CB0
// 1 on ground too steep to stand on
extern "C" int plyIsGroundSteep(TChrPlayer* player)
{
	Vec3f normal;
	if (plyGetProbe(player, 5, 0, 0, &normal) != 0) {
		return normal.y < gPlyParams->mSlipNormal;
	}
	return 0;
}

// Frames 12 and 27 of the walk are where the feet land
// 0x800B8D08
extern "C" int plyAtFootstep(TChrPlayer* player)
{
	if (player->mBodyCopy.passedFrame(12.0f) != 0 || player->mBodyCopy.passedFrame(27.0f) != 0) {
		return 1;
	}
	return 0;
}

// 0x800B8D64
extern "C" int plyCheckMovePushed(TChrPlayer* player, PlyInput* input)
{
	return plySetMoveIfPushed(player, input->mStick, input->mAngle);
}

// 0x800B8D8C
// Lets go of the egg turned toward it
extern "C" void plyDropEgg(TChrPlayer* player)
{
	TObjAction* egg = plyGetEgg(player);
	plyFaceEgg(player);
	if (egg != NULL) {
		plyReleaseEgg(player, egg);
	}
}

// 0x800B8DDC
extern "C" void plySteerMobile(TChrPlayer* player, PlyInput* input)
{
	TMobileObject* mobile = plyGetMobile(player);
	if (mobile != NULL) {
		mobile->steer(input->mStick, input->mAngle);
	}
}

// 0x800B8E1C
extern "C" void plySetMove(TChrPlayer* player, PlyInput* input)
{
	player->setMove(input->mStick, input->mAngle);
}

// 0x800B8E44
// The motion speed for the run or the walk
extern "C" int plyGetMoveSpeed(TChrPlayer* player, f32* speed)
{
	PlyParams* params = gPlyParams;
	if (player->_340.z < params->mRunSpeed) {
		f32 slope;
		plyGetMove(player, &slope, 0);
		if (slope >= params->mSteepSlope) {
			*speed = params->mRunMotionSpeed;
			return 1;
		}
		*speed = params->mWalkMotionSpeed;
		return 0;
	}
	*speed = params->mRunMotionSpeed;
	return 1;
}

#include "player/PlyStateTail.h"
