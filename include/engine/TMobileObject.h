#ifndef _TMOBILEOBJECT_H
#define _TMOBILEOBJECT_H

#include "core/types.h"
#include "stage/Stage.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "sound/MobileSound.h"
#include "animal/AniRider.h"

class TChrPlayer;

// What the player steers while rolling an egg
// See "The mobile object" in docs/notes.md

// The names are readings of what each state does and who enters it
enum MobileState {
	MOBILE_IDLE      = 0,
	MOBILE_SETTLE    = 1,    // no gravity scale and tells the owner when it lands
	MOBILE_ATTACHED  = 2,    // follows another object's part from message 0x16
	MOBILE_BRAKE     = 3,    // eases its facing round while it stops
	MOBILE_FLUNG     = 4,    // from message 0x23
	MOBILE_ROLL      = 6,
	MOBILE_LOB       = 7,
	MOBILE_AIR       = 8,
	MOBILE_HOP       = 9,
	MOBILE_DASH      = 10,
	MOBILE_SLAM      = 11,   // hits the ground with a shock wave then becomes MOBILE_SLAM_LAND
	MOBILE_SLAM_LAND = 12,
	MOBILE_UP        = 13,   // goes straight up and needs ability 0x20
	MOBILE_DOWN      = 14,
	MOBILE_LAUNCH    = 15,
	MOBILE_DRIVE     = 16,
	MOBILE_THROWN    = 17,
	MOBILE_LONG_JUMP = 18,
	MOBILE_RAIL      = 19,
	MOBILE_NONE      = 20,
	MOBILE_DETACHED  = 0x16, // what the owner sees before any state has run
};

// The four things a state function is asked to do
enum MobilePhase {
	MOBILE_ENTER  = 0,
	MOBILE_EXIT   = 1,
	MOBILE_CHECK  = 2,
	MOBILE_UPDATE = 3,
};

/** Tuning read from playerGetParams(0) Only the fields this class reads are named */
struct MobileParams {
	u8 _00[0xC];
	u16 mLiftFrom;     // _0C
	u16 mLiftTo;       // _0E
	u8 _10[8];
	u16 mRollTime;     // _18 how long a thrown ball rolls at full speed
	u16 mSlowTime;     // _1A and how long it then slows
	u16 mDashTime;     // _1C
	u16 mDriveTime;    // _1E
	u8 _20[0x14];
	int mTurn;         // _34
	u8 _38[0x34];
	f32 mLaunchSpeed;  // _6C
	f32 mLaunchHeight; // _70
	f32 mAirSteer;     // _74
	f32 mLift;         // _78
	u8 _7C[0x48];
	f32 mDashMinSpeed; // _C4
	u8 _C8[8];
	f32 mDriveCap;     // _D0
	f32 mDriveMin;     // _D4
	f32 mLandHeight;   // _D8
	f32 mLobAscent;    // _DC
	f32 mUpHeight;     // _E0
	f32 mHopHeight;    // _E4
	f32 mThrownAscent; // _E8
	f32 mJumpSpeed;    // _EC
	f32 mJumpHeight;   // _F0
	f32 mAirCap;       // _F4
};

/** Tuning read from playerGetParamsTail */
struct MobileBodyParams {
	f32 mFriction;  // _00
	f32 mAirCap;    // _04
	u8 _08[0x14];
	f32 mRollCap;   // _1C
	u8 _20[4];
	f32 mGravity;   // _24
};

// Holds the collision model being stood on
struct MobileGround { // size 0x14
	u8 _00[0x10];
	void* mModel; // _10
};

// Started by sePlay3DHeld and stopped by seStop3D
struct MobileLoopEffect { // size 0x2C
	MobileLoopEffect()
	{
		_00 = 0;
		_04 = 0;
		_0C = 0;
	}

	void* _00;
	void* _04;
	u8 _08[4];
	void* _0C;
	u8 _10[0x10];
	Vec3f mPos; // _20
};

struct Fx;
class TMobileObject;

/**
 * The dust under the body, effect 0x88 or 0x8A by speed. It looks for ground (mask 8) under the body every frame and
 * goes out when there is none. See MobileTrack.cpp. The constructor points mBody at the object's body.
 * @size 0x10
 */
struct MobileDust {
	MobileDust(TMobileObject* owner); // 0x801D1628
	~MobileDust();                    // 0x801D1640

	void start();  // 0x801D168C
	void stop();   // 0x801D1744
	void update(); // 0x801D17BC
	u16 pickEffect(Vec3f* vel); // 0x801D1920

	TMobileObject* mOwner; // _00
	EggBody* mBody;        // _04
	Fx* mFx;               // _08
	s16 mTimer;            // _0C
};

/**
 * The wake on water (mask 4), effect 0x81 or 0x84 by speed. Otherwise the same as MobileDust.
 * @size 0x10
 */
struct MobileWake {
	MobileWake(TMobileObject* owner); // 0x801D195C
	~MobileWake();                    // 0x801D1974

	void start();  // 0x801D19C0
	void stop();   // 0x801D1A78
	void update(); // 0x801D1AF0
	u16 pickEffect(Vec3f* vel); // 0x801D1C64

	TMobileObject* mOwner; // _00
	EggBody* mBody;        // _04
	Fx* mFx;               // _08
	s16 mTimer;            // _0C
};

/**
 * A short lived object that plays one particle and frees itself, spawned by MOBILE_DASH when mSpecial is set.
 * It stores no class name of its own, so this name is ours.
 *
 * @size 0x28
 */
class MobileSpark : public TObject {
public:
	MobileSpark(TObject* parent, u16 player, Vec3f* pos, f32 scale);
	virtual ~MobileSpark();
	virtual void draw(); // _0C

	void* mEmitter; // _20
	int mLife;      // _24
};

/**
 * @size 0x34C
 */
class TMobileObject : public TObject {
public:
	TMobileObject(TObject* parent);
	virtual ~TMobileObject();
	virtual void draw(); // _0C runs every frame despite the name inherited from TObject

	// Setup and what the player's states call
	void setHandle(EggHandleBase* handle);
	void reset(f32 radius, const Vec3f* pos, const EggRot* rot);
	TObjAction* getHeld();
	void setFacing(void* mtx);
	int takeHit(HitInfo* hit, int power);
	void setMove(f32 amount, int angle);
	int getMove(f32* amount, int* angle);
	void setSpecial(int special);
	void startRoll();
	void startAir();
	void startDrive();
	void applyDrag(f32 amount);
	int onSpecialGround();
	void stopSettle();
	void stopBrake();
	int sendHang(f32 unused0, int unused1); // neither is read PlyState passes 0.0f and 0
	void addLift();
	void steer(f32 amount, int angle);
	void updateDustMatrix();
	void saveLoopPos();
	f32 getMotion(Vec3f* pos, Vec3f* vel);
	void setRollSound(f32 level, Vec3f* pos);
	void startRail(void* path, void* point, f32 speed);
	void moveAlongGround(f32 amount, int angle);
	int startHop();
	int tryDash();
	int startSlam();
	int tryUp();
	int launch(f32 amount, int angle);
	int startThrown(Vec3f* dir);

	// Probes
	int probeFloor(int any) const;
	int tryLand();
	int longJump();
	int isSinking();
	int checkRail();
	int checkHazards();
	int bounceOffWall();

	// Movement one call per frame from a state
	void capSpeed(f32 cap);
	void capSpeedHigh();
	void slideOnGround();
	void spawnShockWave(f32 power);
	void checkBumps();
	void moveGround(f32 cap);
	void moveRail();
	void moveSettle();
	void moveAttached();
	void moveDefaultNoGravity();
	void moveAscent();
	void moveDefault();
	void moveDown();
	void moveAirSpin();
	void moveThrown();
	void moveRollCap(f32 cap);
	void stepBody();

	int onMessage(EggHandleBase* handle, ObjMsg* msg);

	void setState(int state);
	void runState(int state, int phase);
	void stateIdle(int phase);
	void stateSettle(int phase);
	void stateAttached(int phase);
	void stateBrake(int phase);
	void stateFlung(int phase);
	void stateRoll(int phase);
	void stateLob(int phase);
	void stateAir(int phase);
	void stateHop(int phase);
	void stateDash(int phase);
	void stateSlam(int phase);
	void stateSlamLand(int phase);
	void stateUp(int phase);
	void stateDown(int phase);
	void stateLaunch(int phase);
	void stateDrive(int phase);
	void stateThrown(int phase);
	void stateLongJump(int phase);
	void stateRail(int phase);
	void stateNone(int phase);

	// Messages to the owner
	int sendHit(int kind);
	int sendCrush(int kind);
	void sendLanded();
	void sendSpecial();

	void setLaunchVel(f32 amount, int angle);
	void steerInAir();
	void steerOnGround();

	// Message handlers from onMessage
	int msgPush(ObjMsg* msg);
	int msgIgnore(ObjMsg* msg);
	int msgAttach(ObjMsg* msg);
	int msgLaunch(ObjMsg* msg);
	int msgFling(ObjMsg* msg);
	int msgRumble2A(ObjMsg* msg);
	int msgRumble74(ObjMsg* msg);
	int msgCombo(ObjMsg* msg);
	u32 msgAbilities(ObjMsg* msg);
	u32 msgStateFlags(ObjMsg* msg);
	int msgGetVel(ObjMsg* msg);

	void rumble(s16 strength);
	int findContact(u32* flags, Vec3f* pos, Vec3f* normal, f32* dist) const;

	inline void operator delete(void* mem);

	s8 mState;                  // _20
	u8 _21[3];
	u16 mFrames;                // _24 counted by the states that use it
	u8 _26[2];
	u16 mRollFrames;            // _28
	u16 mTimer;                 // _2A
	u16 mTargetPart;            // _2C the part of mTarget to follow
	u16 mFace;                  // _2E the face of the ground model
	u16 mCombo;                 // _30
	u16 mComboTimer;            // _32
	ObjRef mTarget;             // _34
	FlagWord mFlags;            // _40 1 = on the ground 2 4 and 8 ask for a hazard check 0x20 = landed once
	EggSurface mBody;           // _44
	u8 _15C[0xC];
	f32 mAttachRot[4];          // _168
	EggHandleBase* mHandle;     // _178 plyGetMobile gives 0 while this is 0
	TChrPlayer* mOwner;         // _17C
	f32 mMatrix[12];            // _180
	f32 mTrailMatrix[12];       // _1B0
	f32 mDustMatrix[12];        // _1E0
	MobileParams* mParams;      // _210
	MobileBodyParams* mBodyParams; // _214
	Vec3f mGravity;             // _218
	Rider mGround;              // _224 on the ground model it stands on
	f32 mMoveAmount;            // _238
	int mMoveAngle;             // _23C
	f32 mFacingBlend;           // _240
	f32 mFacing[4];             // _244
	EggCollide mRail;           // _254
	EggCollide mRailNext;       // _270
	f32 mRailSpeed;             // _28C
	u8 _290[8];
	Vec3f mAim;                 // _298
	void* mTrail;               // _2A4
	void* mTrail2;              // _2A8
	u8 _2AC[0xC];
	Fx* mEffect;              // _2B8
	Fx* mEffect2;             // _2BC
	MobileLoopEffect mLoop;     // _2C0
	MobileSound mSound;         // _2EC
	int mPower;                 // _324
	int mSpecial;               // _328
	MobileDust mDust;           // _32C
	MobileWake mWake;           // _33C
};

// The handle eggs give their holder with the virtual that takes what the holder doesn't
struct MobileHandle : public EggHandleBase {
	virtual ~MobileHandle();
	virtual int onMessage(ObjMsg* msg); // _0C
};

struct RailPath;
struct RailPoint;
struct Vec3f;

extern "C" {
void mobileGroundContact(void*, u32, void*, Vec3f*, void*);
f32 mobileLerp(f32, f32, f32);
void mobileRailContact(RailPath*, const RailPoint*, Vec3f*, void*, int*);
void mobileRollRailContact(RailPath*, const RailPoint*, Vec3f*, void*, int*);
void mobileTurnVector(Vec3f*, Vec3f*, Vec3f*, int, int);
int mobileWallContact(f32, void*, u16, const Vec3f*, void*);
}

#endif
