#ifndef _PLYSTATE_H
#define _PLYSTATE_H
struct Fx;

#include "player/Player.h"
#include "player/PlyInput.h"

// The player's moves The TPlyState class names come from the DOL and method and member names are ours

// Anything made from the fixed pool in the player
extern "C" void* allocPlyState(u32 size, void* pool);

class TCamPath;
class TPlyItem;

class TActionState {
public:
	TActionState();

	u32 mNext;         // _00 in the free list
	u32 _04;           // _04
	const char* mName; // _08
	u16 mSize;         // _0C
	u8 _0E[2];

	// _10 = vtable
	virtual ~TActionState();
};

// The payloads of the messages the states take
struct PlyVecMsg : public ObjMsg {
	u8 _02[2];  // _02
	Vec3f mVec; // _04
};

struct PlyGateMsg : public ObjMsg {
	u8 _02[2];  // _02
	Vec3f mPos; // _04
	int mAngle; // _10
};

struct PlyHangMsg : public ObjMsg {
	u8 _02[2]; // _02
	u16 mId;   // _04
	u16 mPart; // _06
};

struct PlyPlaceMsg : public ObjMsg {
	u8 _02[2];   // _02
	u16 _04;     // _04
	u16 _06;     // _06
	u16 mMotion; // _08
};

struct PlyRailMsg : public ObjMsg {
	u8 _02[2];    // _02
	void* mPath;  // _04
	void* mPoint; // _08
	f32 mSpeed;   // _0C
};

// The message handlers the states share in PlyState.cpp
extern "C" {
int plyMsgClimb(TChrPlayer* player, ObjMsg* msg); // drops the egg and climbs
int plyMsgEnterGate(TChrPlayer* player, ObjMsg* msg); // enters a gate
int plyMsgKnockBack(TChrPlayer* player, ObjMsg* msg); // knocked back
int plyMsgGetAnimal(TChrPlayer* player, ObjMsg* msg);
int plyMsgPickup(TChrPlayer* player, ObjMsg* msg); // takes a pickup
int plyMsgHangHook(TChrPlayer* player, ObjMsg* msg); // grabs a hook
int plyMsgLaunch(TChrPlayer* player, ObjMsg* msg); // launched
int plyMsgToStart(TChrPlayer* player, ObjMsg* msg); // back to the start point
int plyMsgStatusBits(TChrPlayer* player, ObjMsg* msg);
int plyMsgHangEgg(TChrPlayer* player, ObjMsg* msg);
int plyMsgMountHangEgg(TChrPlayer* player, ObjMsg* msg);
int plyMsgMountHangEggStopAim(TChrPlayer* player, ObjMsg* msg);
int plyMsgSpinLaunch(TChrPlayer* player, ObjMsg* msg);
int plyMsgRideLaunch(TChrPlayer* player, ObjMsg* msg);
int plyMsgMountSpinLaunch(TChrPlayer* player, ObjMsg* msg);
int plyMsgRail(TChrPlayer* player, ObjMsg* msg); // puts the egg on a rail
int plyMsgKicked(TChrPlayer* player, ObjMsg* msg);
int plyMsgDrive(TChrPlayer* player, ObjMsg* msg);
int plyMsgIgnoreA(TChrPlayer* player, ObjMsg* msg);
int plyMsgIgnoreB(TChrPlayer* player, ObjMsg* msg);
int plyMsgIgnoreC(TChrPlayer* player, ObjMsg* msg);
int plyMsgBounceFlag(TChrPlayer* player, ObjMsg* msg);
int plyMsgStartEvent(TChrPlayer* player, ObjMsg* msg); // starts an event
int plyMsgFall(TChrPlayer* player, ObjMsg* msg); // loses the egg and falls
}

/**
 * An all-inline layer between TActionState and TPlyState. Its vtable is at 0x80255314 and its bodies
 * are emitted in Player.cpp. The name is ours.
 *
 * @size 0x14
 */
class TPlyStateBase : public TActionState {
public:
	TPlyStateBase() { }
	virtual ~TPlyStateBase() { }                         // _08
	virtual void enter(TChrPlayer* player) { }           // _0C
	virtual void exit(TChrPlayer* player) { }            // _10
	virtual void run(TChrPlayer* player) { }             // _14
	virtual void update(TChrPlayer* player) = 0;         // _18
};

/**
 * @size 0x14
 */
class TPlyState : public TPlyStateBase {
public:
	TPlyState();
	static void* operator new(unsigned long size, void* pool) { return allocPlyState(size, pool); }
	virtual ~TPlyState() { }                                  // _08
	virtual void run(TChrPlayer* player);                     // _14 calls update
	virtual void update(TChrPlayer* player);                  // _18
	virtual void check(TChrPlayer* player);                   // _1C moves to another state when the world calls for it
	virtual void vfn_20(TChrPlayer* player);                  // _20
	virtual int handleInput(TChrPlayer* player, PlyInput* input); // _24 1 = the input started something
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);  // _28
	virtual int vfn_2C(TChrPlayer* player);                   // _2C 1 in Roll and RollDash
	virtual int vfn_30(TChrPlayer* player);                   // _30
};

// 800ADBC4
class TPlyStateAppear : public TPlyState {
public:
	TPlyStateAppear();
	virtual ~TPlyStateAppear() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);

	u16 mTimer; // _14 frames since entering
	u8 _16[0x2];
};
extern "C" void* createPlyStateAppear(void* pool, TChrPlayer* player);

// 800ADC00
class TPlyStateAscend : public TPlyState {
public:
	TPlyStateAscend();
	virtual ~TPlyStateAscend() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x02:
			onPush(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	int onPush(TChrPlayer* player, ObjMsg* msg);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
};
extern "C" void* createPlyStateAscend(void* pool, TChrPlayer* player);

// 800ADC3C
class TPlyStateBowlStand : public TPlyState {
public:
	TPlyStateBowlStand();
	virtual ~TPlyStateBowlStand() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x15:
			plyMsgKnockBack(player, msg);
			return;
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);

	FlagWord mFlags; // _14
};
extern "C" void* createPlyStateBowlStand(void* pool, TChrPlayer* player);

// 800ADC78
class TPlyStateBowlWalk : public TPlyState {
public:
	TPlyStateBowlWalk();
	virtual ~TPlyStateBowlWalk() { }
	virtual int vfn_2C(TChrPlayer* player) { return 1; }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x15:
			plyMsgKnockBack(player, msg);
			return;
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);

	FlagWord mFlags; // _14
};
extern "C" void* createPlyStateBowlWalk(void* pool, TChrPlayer* player);

// 800ADCB4
class TPlyStateClimb : public TPlyState {
public:
	TPlyStateClimb();
	virtual ~TPlyStateClimb() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
};
extern "C" void* createPlyStateClimb(void* pool, TChrPlayer* player);

// 800ADCF0
class TPlyStateClimbDown : public TPlyState {
public:
	TPlyStateClimbDown();
	virtual ~TPlyStateClimbDown() { }
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
};
extern "C" void* createPlyStateClimbDown(void* pool, TChrPlayer* player);

// 800ADD2C
class TPlyStateCrow : public TPlyState {
public:
	TPlyStateCrow();
	virtual ~TPlyStateCrow() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	void startEscape(TChrPlayer* player);
	void getUp(TChrPlayer* player);

	int mPhase;    // _14 0 held 1 escaping 2 getting up
	int mTimer;    // _18
	Fx* mEffect; // _1C stopped on escape or exit
};
extern "C" void* createPlyStateCrow(void* pool, TChrPlayer* player);

// 800ADD68
class TPlyStateDamage : public TPlyState {
public:
	TPlyStateDamage();
	virtual ~TPlyStateDamage() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);

	int mTimer;   // _14 read by tickTimer32
};
extern "C" void* createPlyStateDamage(void* pool, TChrPlayer* player);

// 800ADDA4
class TPlyStateDie : public TPlyState {
public:
	TPlyStateDie();
	virtual ~TPlyStateDie() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x2B:
			plyMsgToStart(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);

	u8 _14[4];    // _14 a FlagByte
};
extern "C" void* createPlyStateDie(void* pool, TChrPlayer* player);

// 800ADDE0
class TPlyStateDieBurn : public TPlyState {
public:
	TPlyStateDieBurn();
	virtual ~TPlyStateDieBurn() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x2B:
			plyMsgToStart(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);

	s8 mStep;      // _14 0 burning 1 rising as ash
	u8 _15[0x3];
	Fx* mEffect; // _18 the flames that follow the body
};
extern "C" void* createPlyStateDieBurn(void* pool, TChrPlayer* player);

// 800ADE1C
class TPlyStateDieDrown : public TPlyState {
public:
	TPlyStateDieDrown();
	virtual ~TPlyStateDieDrown() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x2B:
			plyMsgToStart(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
};
extern "C" void* createPlyStateDieDrown(void* pool, TChrPlayer* player);

// 800ADE58
class TPlyStateDieSandy : public TPlyState {
public:
	TPlyStateDieSandy();
	virtual ~TPlyStateDieSandy() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x2B:
			plyMsgToStart(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);

	Fx* mEffect; // _14 the sand
};
extern "C" void* createPlyStateDieSandy(void* pool, TChrPlayer* player);

// 800ADE94
class TPlyStateDisable : public TPlyState {
public:
	TPlyStateDisable();
	virtual ~TPlyStateDisable() { }
};
extern "C" void* createPlyStateDisable(void* pool, TChrPlayer* player);

// 800ADED0
class TPlyStateEnterGate : public TPlyState {
public:
	TPlyStateEnterGate();
	virtual ~TPlyStateEnterGate() { }
	virtual void update(TChrPlayer* player);
};
extern "C" void* createPlyStateEnterGate(void* pool, TChrPlayer* player);

// 800ADF0C
class TPlyStateEvent : public TPlyState {
public:
	TPlyStateEvent();
	virtual ~TPlyStateEvent() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x79:
			onEventEnd(player, msg);
			return;
		case 0x7E:
			onEventPose(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	int onEventEnd(TChrPlayer* player, ObjMsg* msg);
	int onEventPose(TChrPlayer* player, ObjMsg* msg);
	virtual void update(TChrPlayer* player);
};
extern "C" void* createPlyStateEvent(void* pool, TChrPlayer* player);

// 800ADF48
class TPlyStateEventEx : public TPlyState {
public:
	TPlyStateEventEx();
	virtual ~TPlyStateEventEx() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x79:
			onEventEnd(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	int onEventEnd(TChrPlayer* player, ObjMsg* msg);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);

	u16 mFrame; // _14 the pose runs at half speed
	u8 _16[0x6];
};
extern "C" void* createPlyStateEventEx(void* pool, TChrPlayer* player);

// 800ADF84
class TPlyStateFall : public TPlyState {
public:
	TPlyStateFall();
	virtual ~TPlyStateFall() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x02:
			plyMsgClimb(player, msg);
			return;
		case 0x19:
			plyMsgPickup(player, msg);
			return;
		case 0x86:
			plyMsgBounceFlag(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	u16 mTimer; // _14 counts down to the fall voice
	u8 _16[2];
};
extern "C" void* createPlyStateFall(void* pool, TChrPlayer* player);

// 800ADFC0
class TPlyStateGetAnimal : public TPlyState {
public:
	TPlyStateGetAnimal();
	virtual ~TPlyStateGetAnimal() { }
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);

	TCamPath* mCam; // _14
	f32 mMtx[12];   // _18 where the camera path rides
	u16 mFrame;     // _48
	u8 _4A[2];
};
extern "C" void* createPlyStateGetAnimal(void* pool, TChrPlayer* player);

// 800ADFFC
class TPlyStateGetItem : public TPlyState {
public:
	TPlyStateGetItem();
	virtual ~TPlyStateGetItem() { }
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);

	u16 mFrame;       // _14
	u8 _16[2];
	TCamPath* mCam;   // _18
	f32 mMtx[12];     // _1C where the camera path rides
	f32 mItemMtx[12]; // _4C where the item shows
	TPlyItem* mItem;  // _7C
};
extern "C" void* createPlyStateGetItem(void* pool, TChrPlayer* player);

// 800AE038
class TPlyStateGlide : public TPlyState {
public:
	TPlyStateGlide();
	virtual ~TPlyStateGlide() { }
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
};
extern "C" void* createPlyStateGlide(void* pool, TChrPlayer* player);

// 800AE074
class TPlyStateHangEdge : public TPlyState {
public:
	TPlyStateHangEdge();
	virtual ~TPlyStateHangEdge() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	u16 mTimer; // _14 40 on entry The stick and A do nothing until it reaches 0
};
extern "C" void* createPlyStateHangEdge(void* pool, TChrPlayer* player);

// 800AE0B0
class TPlyStateHangEgg : public TPlyState {
public:
	TPlyStateHangEgg();
	virtual ~TPlyStateHangEgg() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x15:
			plyMsgKnockBack(player, msg);
			return;
		case 0x25:
			onEggLost(player, msg);
			return;
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		case 0x4E:
			onRollJump(player, msg);
			return;
		case 0x50:
			plyMsgSpinLaunch(player, msg);
			return;
		case 0x60:
			plyMsgFall(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	int onEggLost(TChrPlayer* player, ObjMsg* msg);
	int onRollJump(TChrPlayer* player, ObjMsg* msg);
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
};
extern "C" void* createPlyStateHangEgg(void* pool, TChrPlayer* player);

// 800AE0EC
class TPlyStateHangHook : public TPlyState {
public:
	TPlyStateHangHook();
	virtual ~TPlyStateHangHook() { }
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
};
extern "C" void* createPlyStateHangHook(void* pool, TChrPlayer* player);

// 800AE128
class TPlyStateHoldObj : public TPlyState {
public:
	TPlyStateHoldObj();
	virtual ~TPlyStateHoldObj() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x15:
			plyMsgKnockBack(player, msg);
			return;
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		case 0x4D:
			plyMsgMountHangEggStopAim(player, msg);
			return;
		case 0x53:
			plyMsgDrive(player, msg);
			return;
		case 0x60:
			plyMsgFall(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void vfn_20(TChrPlayer* player);
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	int mTimer; // _14
};
extern "C" void* createPlyStateHoldObj(void* pool, TChrPlayer* player);

// 800AE164
class TPlyStateInTheMirror : public TPlyState {
public:
	TPlyStateInTheMirror();
	virtual ~TPlyStateInTheMirror() { }
	virtual void enter(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
};
extern "C" void* createPlyStateInTheMirror(void* pool, TChrPlayer* player);

// 800AE1A0
class TPlyStateJump : public TPlyState {
public:
	TPlyStateJump();
	virtual ~TPlyStateJump() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x02:
			plyMsgClimb(player, msg);
			return;
		case 0x19:
			plyMsgPickup(player, msg);
			return;
		case 0x1B:
			plyMsgHangHook(player, msg);
			return;
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		case 0x86:
			plyMsgBounceFlag(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual int vfn_30(TChrPlayer* player);
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	u16 mTimer;   // _14 counts down before a fall can start
	u8 _16[2];
};
extern "C" void* createPlyStateJump(void* pool, TChrPlayer* player);

// 800AE1DC
class TPlyStateJump2nd : public TPlyState {
public:
	TPlyStateJump2nd();
	virtual ~TPlyStateJump2nd() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x02:
			plyMsgClimb(player, msg);
			return;
		case 0x19:
			plyMsgPickup(player, msg);
			return;
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		case 0x1B:
			plyMsgHangHook(player, msg);
			return;
		case 0x86:
			plyMsgBounceFlag(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void vfn_20(TChrPlayer* player);
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	s16 mTimer; // _14 set to 70 on enter
	u8 _16[2];
};
extern "C" void* createPlyStateJump2nd(void* pool, TChrPlayer* player);

// 800AE218
class TPlyStateLaunch : public TPlyState {
public:
	TPlyStateLaunch();
	virtual ~TPlyStateLaunch() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
};
extern "C" void* createPlyStateLaunch(void* pool, TChrPlayer* player);

// 800AE254
class TPlyStateLift : public TPlyState {
public:
	TPlyStateLift();
	virtual ~TPlyStateLift() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
};
extern "C" void* createPlyStateLift(void* pool, TChrPlayer* player);

// 800AE290
class TPlyStateLiftStand : public TPlyState {
public:
	TPlyStateLiftStand();
	virtual ~TPlyStateLiftStand() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	u16 mBlended; // _14 0 while the pickup rows 0x39 and 0x3A still have to blend into the stand
	u8 _16[2];
};
extern "C" void* createPlyStateLiftStand(void* pool, TChrPlayer* player);

// 800AE2CC
class TPlyStateLiftThrow : public TPlyState {
public:
	TPlyStateLiftThrow();
	virtual ~TPlyStateLiftThrow() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);

	int mThrown; // _14 set once the throw message has gone out
};
extern "C" void* createPlyStateLiftThrow(void* pool, TChrPlayer* player);

// 800AE308
class TPlyStateLiftWalk : public TPlyState {
public:
	TPlyStateLiftWalk();
	virtual ~TPlyStateLiftWalk() { }
	virtual int vfn_2C(TChrPlayer* player) { return 1; }
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
};
extern "C" void* createPlyStateLiftWalk(void* pool, TChrPlayer* player);

// 800AE344
class TPlyStateMount : public TPlyState {
public:
	TPlyStateMount();
	virtual ~TPlyStateMount() { }
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
};
extern "C" void* createPlyStateMount(void* pool, TChrPlayer* player);

// 800AE380
class TPlyStateOrderCreature : public TPlyState {
public:
	TPlyStateOrderCreature();
	virtual ~TPlyStateOrderCreature() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);

	FlagWord mFlags; // _14 1 = the creature was told
};
extern "C" void* createPlyStateOrderCreature(void* pool, TChrPlayer* player);

// 800AE3BC
class TPlyStatePlace : public TPlyState {
public:
	TPlyStatePlace();
	virtual ~TPlyStatePlace() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x05:
			onPlaced(player, msg);
			return;
		case 0x83:
			onPlayMotion(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	int onPlaced(TChrPlayer* player, ObjMsg* msg);
	int onPlayMotion(TChrPlayer* player, ObjMsg* msg);
	virtual void update(TChrPlayer* player);
};
extern "C" void* createPlyStatePlace(void* pool, TChrPlayer* player);

// 800AE3F8
class TPlyStatePushWall : public TPlyState {
public:
	TPlyStatePushWall();
	virtual ~TPlyStatePushWall() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	u32 mTimer; // _14 voice 0xA8 plays when it runs out
};
extern "C" void* createPlyStatePushWall(void* pool, TChrPlayer* player);

// 800AE434
class TPlyStateRemote : public TPlyState {
public:
	TPlyStateRemote();
	virtual ~TPlyStateRemote() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x31:
			onRemoteEnd(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	int onRemoteEnd(TChrPlayer* player, ObjMsg* msg);
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);

	int mDone; // _14 set by message 0x31
	u8 _18[4];
};
extern "C" void* createPlyStateRemote(void* pool, TChrPlayer* player);

// 800AE470
class TPlyStateRideLaunch : public TPlyState {
public:
	TPlyStateRideLaunch();
	virtual ~TPlyStateRideLaunch() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x4D:
			plyMsgMountHangEgg(player, msg);
			return;
		case 0x4F:
			onIgnore(player, msg);
			return;
		case 0x50:
			plyMsgRideLaunch(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	int onIgnore(TChrPlayer* player, ObjMsg* msg);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
};
extern "C" void* createPlyStateRideLaunch(void* pool, TChrPlayer* player);

// 800AE4AC
class TPlyStateRideStand : public TPlyState {
public:
	TPlyStateRideStand();
	virtual ~TPlyStateRideStand() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x15:
			plyMsgKnockBack(player, msg);
			return;
		case 0x4D:
			plyMsgMountHangEgg(player, msg);
			return;
		case 0x50:
			plyMsgRideLaunch(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	int mFromWalk; // _14 1 = blend to the stand on the next footstep
};
extern "C" void* createPlyStateRideStand(void* pool, TChrPlayer* player);

// 800AE4E8
class TPlyStateRideWalk : public TPlyState {
public:
	TPlyStateRideWalk();
	virtual ~TPlyStateRideWalk() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x15:
			plyMsgKnockBack(player, msg);
			return;
		case 0x4D:
			plyMsgMountHangEgg(player, msg);
			return;
		case 0x50:
			plyMsgRideLaunch(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
};
extern "C" void* createPlyStateRideWalk(void* pool, TChrPlayer* player);

// 800AE524
class TPlyStateRoll : public TPlyState {
public:
	TPlyStateRoll();
	virtual ~TPlyStateRoll() { }
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual void vfn_20(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual int vfn_2C(TChrPlayer* player);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);

	int _14; // _14 cleared on enter
};
extern "C" void* createPlyStateRoll(void* pool, TChrPlayer* player);

// 800AE560
class TPlyStateRollBounce : public TPlyState {
public:
	TPlyStateRollBounce();
	virtual ~TPlyStateRollBounce() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onLand(TChrPlayer* player);

	FlagByte mFlags; // _14 2 = landed 4 = the jump button came up
	u8 mStep;    // _15
	s16 mDelay;  // _16
	u16 mTimer;  // _18
	u8 _1A[2];
};
extern "C" void* createPlyStateRollBounce(void* pool, TChrPlayer* player);

// 800AE59C
class TPlyStateRollBounceJump : public TPlyState {
public:
	TPlyStateRollBounceJump();
	virtual ~TPlyStateRollBounceJump() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onLand(TChrPlayer* player);

	FlagByte mFlags; // _14 1 = landed
	u8 _15[3];
};
extern "C" void* createPlyStateRollBounceJump(void* pool, TChrPlayer* player);

// 800AE5D8
class TPlyStateRollBrake : public TPlyState {
public:
	TPlyStateRollBrake();
	virtual ~TPlyStateRollBrake() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual void vfn_20(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);

	u32 mVoiceDelay; // _14
	u32 mFrame;      // _18
	int mDustLeft;   // _1C
};
extern "C" void* createPlyStateRollBrake(void* pool, TChrPlayer* player);

// 800AE614
class TPlyStateRollDash : public TPlyState {
public:
	TPlyStateRollDash();
	virtual ~TPlyStateRollDash() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual int vfn_2C(TChrPlayer* player);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onFall(TChrPlayer* player);
	int onDashEnd(TChrPlayer* player);

	FlagWord mFlags; // _14
};
extern "C" void* createPlyStateRollDash(void* pool, TChrPlayer* player);

// 800AE650
class TPlyStateRollDrive : public TPlyState {
public:
	TPlyStateRollDrive();
	virtual ~TPlyStateRollDrive() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onLand(TChrPlayer* player);

	int mStep;  // _14
	int mTimer; // _18
};
extern "C" void* createPlyStateRollDrive(void* pool, TChrPlayer* player);

// 800AE68C
class TPlyStateRollJump : public TPlyState {
public:
	TPlyStateRollJump();
	virtual ~TPlyStateRollJump() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onLand(TChrPlayer* player);

	FlagByte mFlags; // _14 1 = landed
	u8 _15[3];
};
extern "C" void* createPlyStateRollJump(void* pool, TChrPlayer* player);

// 800AE6C8
class TPlyStateRollJumpThrow : public TPlyState {
public:
	TPlyStateRollJumpThrow();
	virtual ~TPlyStateRollJumpThrow() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);

	int mStep; // _14
};
extern "C" void* createPlyStateRollJumpThrow(void* pool, TChrPlayer* player);

// 800AE704
class TPlyStateSlide : public TPlyState {
public:
	TPlyStateSlide();
	virtual ~TPlyStateSlide() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		case 0x23:
			plyMsgLaunch(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	int mFrames; // _14 a dust puff every fourth frame
};
extern "C" void* createPlyStateSlide(void* pool, TChrPlayer* player);

// 800AE740
class TPlyStateSlideJump : public TPlyState {
public:
	TPlyStateSlideJump();
	virtual ~TPlyStateSlideJump() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		case 0x23:
			plyMsgLaunch(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
};
extern "C" void* createPlyStateSlideJump(void* pool, TChrPlayer* player);

// 800AE77C
class TPlyStateSlip : public TPlyState {
public:
	TPlyStateSlip();
	virtual ~TPlyStateSlip() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		case 0x23:
			plyMsgLaunch(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);

	u16 mTimer;  // _14 reset while the slope is steep then counts down
	u16 mFrames; // _16 a dust puff every fourth frame
};
extern "C" void* createPlyStateSlip(void* pool, TChrPlayer* player);

// 800AE7B8
class TPlyStateSorry : public TPlyState {
public:
	TPlyStateSorry();
	virtual ~TPlyStateSorry() { }
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);

	TCamPath* mCam; // _14
	f32 mMtx[12];   // _18 where the camera path rides
};
extern "C" void* createPlyStateSorry(void* pool, TChrPlayer* player);

// 800AE7F4
class TPlyStateSpeak : public TPlyState {
public:
	TPlyStateSpeak();
	virtual ~TPlyStateSpeak() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x30:
			onSpeakEnd(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	int onSpeakEnd(TChrPlayer* player, ObjMsg* msg);
	virtual void update(TChrPlayer* player);
};
extern "C" void* createPlyStateSpeak(void* pool, TChrPlayer* player);

// 800AE830
class TPlyStateSpin : public TPlyState {
public:
	TPlyStateSpin();
	virtual ~TPlyStateSpin() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);

	u16 mStillFrames; // _14
	u8 _16[2];
};
extern "C" void* createPlyStateSpin(void* pool, TChrPlayer* player);

// 800AE86C
class TPlyStateSpinBounce : public TPlyState {
public:
	TPlyStateSpinBounce();
	virtual ~TPlyStateSpinBounce() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onLand(TChrPlayer* player);

	FlagByte mFlags; // _14 2 = landed 4 = the jump button came up
	u8 mStep;    // _15
	s16 mDelay;  // _16
	u16 mTimer;  // _18
	u8 _1A[2];
};
extern "C" void* createPlyStateSpinBounce(void* pool, TChrPlayer* player);

// 800AE8A8
class TPlyStateSpinDash : public TPlyState {
public:
	TPlyStateSpinDash();
	virtual ~TPlyStateSpinDash() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onDashEnd(TChrPlayer* player);

	FlagWord mFlags; // _14
};
extern "C" void* createPlyStateSpinDash(void* pool, TChrPlayer* player);

// 800AE8E4
class TPlyStateSpinFall : public TPlyState {
public:
	TPlyStateSpinFall();
	virtual ~TPlyStateSpinFall() { }
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onLand(TChrPlayer* player);

};
extern "C" void* createPlyStateSpinFall(void* pool, TChrPlayer* player);

// 800AE920
class TPlyStateSpinLaunch : public TPlyState {
public:
	TPlyStateSpinLaunch();
	virtual ~TPlyStateSpinLaunch() { }
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onLand(TChrPlayer* player);

};
extern "C" void* createPlyStateSpinLaunch(void* pool, TChrPlayer* player);

// 800AE95C
class TPlyStateSpinLongJump : public TPlyState {
public:
	TPlyStateSpinLongJump();
	virtual ~TPlyStateSpinLongJump() { }
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onFall(TChrPlayer* player);
	int onLand(TChrPlayer* player);

	FlagWord mFlags; // _14
};
extern "C" void* createPlyStateSpinLongJump(void* pool, TChrPlayer* player);

// 800AE998
class TPlyStateSpinOnLane : public TPlyState {
public:
	TPlyStateSpinOnLane();
	virtual ~TPlyStateSpinOnLane() { }
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onLaneEnd(TChrPlayer* player);

};
extern "C" void* createPlyStateSpinOnLane(void* pool, TChrPlayer* player);

// 800AE9D4
class TPlyStateSpinStand : public TPlyState {
public:
	TPlyStateSpinStand();
	virtual ~TPlyStateSpinStand() { }
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg);
	int onLand(TChrPlayer* player);

	FlagWord mFlags; // _14
};
extern "C" void* createPlyStateSpinStand(void* pool, TChrPlayer* player);

// 800AEA10
class TPlyStateStand : public TPlyState {
public:
	TPlyStateStand();
	virtual ~TPlyStateStand() { }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x02:
			plyMsgClimb(player, msg);
			return;
		case 0x14:
			plyMsgEnterGate(player, msg);
			return;
		case 0x17:
			plyMsgGetAnimal(player, msg);
			return;
		case 0x19:
			plyMsgPickup(player, msg);
			return;
		case 0x23:
			plyMsgLaunch(player, msg);
			return;
		case 0x7B:
			plyMsgIgnoreA(player, msg);
			return;
		case 0x80:
			plyMsgIgnoreB(player, msg);
			return;
		case 0x81:
			plyMsgIgnoreC(player, msg);
			return;
		case 0x86:
			plyMsgBounceFlag(player, msg);
			return;
		case 0x87:
			plyMsgStartEvent(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	u16 mPhase;   // _14 0 = settling 1 = waiting to settle 2 = waiting to fidget 3 = fidgeting 4 = finishing 5 = hurt pose
	u16 mTimer;   // _16
	u16 mLoops;   // _18
	u8 _1A[2];
};
extern "C" void* createPlyStateStand(void* pool, TChrPlayer* player);

// 800AEA4C
class TPlyStateTransform : public TPlyState {
public:
	TPlyStateTransform();
	inline virtual ~TPlyStateTransform();
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x79:
			onEventEnd(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	int onEventEnd(TChrPlayer* player, ObjMsg* msg);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);

	u16 mFrame;    // _14
	u8 _16[2];
	Fx* mEffect; // _18
};
extern "C" void* createPlyStateTransform(void* pool, TChrPlayer* player);

// 800AEA88
class TPlyStateVictory : public TPlyState {
public:
	TPlyStateVictory();
	inline virtual ~TPlyStateVictory();
	virtual void enter(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
};
extern "C" void* createPlyStateVictory(void* pool, TChrPlayer* player);

// 800AEAC4
class TPlyStateWalk : public TPlyState {
public:
	TPlyStateWalk();
	virtual ~TPlyStateWalk() { }
	virtual int vfn_2C(TChrPlayer* player) { return 1; }
	virtual void onMessage(TChrPlayer* player, ObjMsg* msg)
	{
		switch (msg->mType) {
		case 0x02:
			plyMsgClimb(player, msg);
			return;
		case 0x14:
			plyMsgEnterGate(player, msg);
			return;
		case 0x19:
			plyMsgPickup(player, msg);
			return;
		case 0x3C:
			plyMsgStatusBits(player, msg);
			return;
		case 0x17:
			plyMsgGetAnimal(player, msg);
			return;
		case 0x23:
			plyMsgLaunch(player, msg);
			return;
		case 0x7B:
			plyMsgIgnoreA(player, msg);
			return;
		case 0x80:
			plyMsgIgnoreB(player, msg);
			return;
		case 0x81:
			plyMsgIgnoreC(player, msg);
			return;
		case 0x86:
			plyMsgBounceFlag(player, msg);
			return;
		case 0x87:
			plyMsgStartEvent(player, msg);
			return;
		}
		TPlyState::onMessage(player, msg);
	}
	virtual void enter(TChrPlayer* player);
	virtual void exit(TChrPlayer* player);
	virtual void update(TChrPlayer* player);
	virtual void check(TChrPlayer* player);
	virtual int handleInput(TChrPlayer* player, PlyInput* input);

	u16 mPhase;   // _14 1 = blending in from row 0xB 2 = walking
	u16 mFrames;  // _16
};
extern "C" void* createPlyStateWalk(void* pool, TChrPlayer* player);

class TChrPlayer;
struct PlyInput;

#include "player/FlagByteTest.h"

extern "C" {
int plyCheckPickupBehind(TChrPlayer*);
int plyCheckCrow(TChrPlayer*, PlyInput*);
int plyCheckGround(TChrPlayer*);
int plyCheckLedge(TChrPlayer*);
int plyCheckFallNoEgg(TChrPlayer*);
int plyCheckStandNoEgg(TChrPlayer*);
int plyCheckMountFalling(TChrPlayer*);
int plyCheckOrderMount(TChrPlayer*, PlyInput*);
int plyCheckPushWall(TChrPlayer*);
int plyCheckLandOnEgg(TChrPlayer*);
int plyCheckThrowItem(TChrPlayer*, const PlyInput*);
int plyCheckPickUpFront(TChrPlayer*, PlyInput*);
int plyCheckJump(TChrPlayer*, PlyInput*);
int plyCheckEggJump(TChrPlayer*, const PlyInput*);
int plyIsGroundSteep(TChrPlayer*);
int plyCheckMovePushed(TChrPlayer*, PlyInput*);
int plyGetMoveSpeed(TChrPlayer*, f32*);
int plyCheckDamage(TChrPlayer*);
int plyCheckMobileLost(TChrPlayer*);
void plyDropEgg(TChrPlayer*);
void plyFaceEgg(TChrPlayer*);
void plySetMove(TChrPlayer*, PlyInput*);
void plyStartFall(TChrPlayer*);
void plySteerMobile(TChrPlayer*, PlyInput*);
int plyTryRollJump(TChrPlayer*, PlyInput*);
}

extern "C" {
extern const char* gTPlyStateAppearName;
extern const char* gTPlyStateAscendName;
extern const char* gTPlyStateBowlStandName;
extern const char* gTPlyStateBowlWalkName;
extern const char* gTPlyStateClimbName;
extern const char* gTPlyStateClimbDownName;
extern const char* gTPlyStateCrowName;
extern const char* gTPlyStateDamageName;
extern const char* gTPlyStateDieName;
extern const char* gTPlyStateDieBurnName;
extern const char* gTPlyStateDieDrownName;
extern const char* gTPlyStateDieSandyName;
extern const char* gTPlyStateDisableName;
extern const char* gTPlyStateEnterGateName;
extern const char* gTPlyStateEventName;
extern const char* gTPlyStateEventExName;
extern const char* gTPlyStateFallName;
extern const char* gTPlyStateGetAnimalName;
extern const char* gTPlyStateGetItemName;
extern const char* gTPlyStateGlideName;
extern const char* gTPlyStateHangEdgeName;
extern const char* gTPlyStateHangEggName;
extern const char* gTPlyStateHangHookName;
extern const char* gTPlyStateHoldObjName;
extern const char* gTPlyStateInTheMirrorName;
extern const char* gTPlyStateJumpName;
extern const char* gTPlyStateJump2ndName;
extern const char* gTPlyStateLaunchName;
extern const char* gTPlyStateLiftName;
extern const char* gTPlyStateLiftStandName;
extern const char* gTPlyStateLiftThrowName;
extern const char* gTPlyStateLiftWalkName;
extern const char* gTPlyStateMountName;
extern const char* gTPlyStateOrderCreatureName;
extern const char* gTPlyStatePlaceName;
extern const char* gTPlyStatePushWallName;
extern const char* gTPlyStateRemoteName;
extern const char* gTPlyStateRideLaunchName;
extern const char* gTPlyStateRideStandName;
extern const char* gTPlyStateRideWalkName;
extern const char* gTPlyStateRollName;
extern const char* gTPlyStateRollBounceName;
extern const char* gTPlyStateRollBounceJumpName;
extern const char* gTPlyStateRollBrakeName;
extern const char* gTPlyStateRollDashName;
extern const char* gTPlyStateRollDriveName;
extern const char* gTPlyStateRollJumpName;
extern const char* gTPlyStateRollJumpThrowName;
extern const char* gTPlyStateSlideName;
extern const char* gTPlyStateSlideJumpName;
extern const char* gTPlyStateSlipName;
extern const char* gTPlyStateSorryName;
extern const char* gTPlyStateSpeakName;
extern const char* gTPlyStateSpinName;
extern const char* gTPlyStateSpinBounceName;
extern const char* gTPlyStateSpinDashName;
extern const char* gTPlyStateSpinFallName;
extern const char* gTPlyStateSpinLaunchName;
extern const char* gTPlyStateSpinLongJumpName;
extern const char* gTPlyStateSpinOnLaneName;
extern const char* gTPlyStateSpinStandName;
extern const char* gTPlyStateStandName;
extern const char* gTPlyStateTransformName;
extern const char* gTPlyStateVictoryName;
extern const char* gTPlyStateWalkName;
extern const u16 sBounceJumpVoices[3];
extern u16 sRollThrowVoices[2];
extern const u16 sRollDashVoices[2];
}

// The Spin and Roll states' message handlers
#ifdef PLYSTATE_CPP

// 0x800B9308
inline void TPlyStateSpinStand::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x4F:
		onLand(player);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x53:
		plyMsgDrive(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800B9460
inline void TPlyStateSpinOnLane::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x51:
		onLaneEnd(player);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800B9548
inline void TPlyStateSpinLongJump::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4C:
		onFall(player);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x4F:
		onLand(player);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800B9698
inline void TPlyStateSpinLaunch::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x4F:
		onLand(player);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800B97D4
inline void TPlyStateSpinFall::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x4F:
		onLand(player);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800B9910
inline void TPlyStateSpinDash::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4A:
		onDashEnd(player);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800B9A4C
inline void TPlyStateSpinBounce::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x4F:
		onLand(player);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x53:
		plyMsgDrive(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800B9BA4
inline void TPlyStateSpin::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800BA070
inline void TPlyStateRollJumpThrow::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800BA17C
inline void TPlyStateRollJump::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x4F:
		onLand(player);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x53:
		plyMsgDrive(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

inline void TPlyStateRollDrive::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4D:
		plyMsgMountHangEggStopAim(player, msg);
		return;
	case 0x4F:
		onLand(player);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x53:
		plyMsgDrive(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

inline int TPlyStateRollDash::vfn_2C(TChrPlayer* player)
{
	return 1;
}

// 0x800BA434
inline void TPlyStateRollDash::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x3C:
		plyMsgStatusBits(player, msg);
		return;
	case 0x4C:
		onFall(player);
		return;
	case 0x4A:
		onDashEnd(player);
		return;
	case 0x4D:
		plyMsgMountHangEggStopAim(player, msg);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgKicked(player, msg);
		return;
	case 0x53:
		plyMsgDrive(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800BA5BC
inline void TPlyStateRollBrake::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x3C:
		plyMsgStatusBits(player, msg);
		return;
	case 0x4D:
		plyMsgMountHangEggStopAim(player, msg);
		return;
	case 0x50:
		plyMsgMountSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgKicked(player, msg);
		return;
	case 0x53:
		plyMsgDrive(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800BA71C
inline void TPlyStateRollBounceJump::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x4F:
		onLand(player);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x53:
		plyMsgDrive(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

// 0x800BA874
inline void TPlyStateRollBounce::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x4D:
		plyMsgHangEgg(player, msg);
		return;
	case 0x4F:
		onLand(player);
		return;
	case 0x50:
		plyMsgSpinLaunch(player, msg);
		return;
	case 0x52:
		plyMsgRail(player, msg);
		return;
	case 0x53:
		plyMsgDrive(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

inline int TPlyStateRoll::vfn_2C(TChrPlayer* player)
{
	return 1;
}

// 0x800BA9D4
inline void TPlyStateRoll::onMessage(TChrPlayer* player, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x15:
		plyMsgKnockBack(player, msg);
		return;
	case 0x3C:
		plyMsgStatusBits(player, msg);
		return;
	case 0x4D:
		plyMsgMountHangEggStopAim(player, msg);
		return;
	case 0x50:
		plyMsgMountSpinLaunch(player, msg);
		return;
	case 0x60:
		plyMsgFall(player, msg);
		return;
	case 0x52:
		plyMsgKicked(player, msg);
		return;
	case 0x53:
		plyMsgDrive(player, msg);
		return;
	}
	TPlyState::onMessage(player, msg);
}

#endif

#endif
