#ifndef _PLAYER_H
#define _PLAYER_H
struct PlyArray;
struct PlyDamageSearch;
struct PlyEdgeSearch;
struct PlyEggSearch;
struct PlyGrabSearch;
struct PlyGroundHit;
struct PlyJoint;
struct PlyMoveHit;
struct PlyPose;
struct PlyRayHit;
struct PlyRollSearch;
struct PlySlamSearch;
struct PlyTalkSearch;
struct DrawTask;
struct CharaColli;

#include "core/types.h"
struct NJS_MOTION;
struct NJS_TEXLIST;
#include "stage/Stage.h"
#include "object/ObjAction.h"
#include "engine/MotionPlayer.h"
#include "player/PlyInput.h"
#include "object/ObjCommon.h"
#include "engine/TMobileObject.h"

/**
 * A player's health. plyHealthSet keeps it between mMin and mMax and marks it changed. The name is ours.
 *
 * @size 0x10
 */
struct PlyHealth {
	u32 mChanged; // _00 1 once it has changed
	int mNow;     // _04
	int mMax;     // _08
	int mMin;     // _0C
};

extern "C" void plyHealthInit(PlyHealth* health);         // 0x8003F6C8
extern "C" void plyHealthSet(PlyHealth* health, int value); // 0x8003F6D4

class TChrPlayer;
struct ItemSpec;
class TPlyEyelid;
class TPlyComb;
class TSpeakCursor;

// A timed ability kind 1 or status kind 2
struct PlyEffect {
	PlyEffect* mNext; // _00
	PlyEffect* mPrev; // _04
	u16 mTime;        // _08 in frames
	u8 mKind;         // _0A
	u8 mId;           // _0B
};

/**
 * @size 0x88
 */
struct PlyEffectList {
	PlyEffectList();  // 0x8003EE88 a free list of the ten nodes
	~PlyEffectList(); // 0x8003EEDC

	PlyEffect* mHead;     // _00
	PlyEffect* mTail;     // _04
	int mCount;           // _08
	PlyEffect mNodes[10]; // _0C
	PlyEffect* mFree;     // _84
};

// The playable characters TObjPlayer is the name the DOL gives the base class and everything else here is ours

/**
 * @size 0x3C
 */
class TObjPlayer : public TObjAction {
public:
	TObjPlayer(TObject* parent, u16 id, u8 charId);
	virtual ~TObjPlayer();
	virtual int getKind();

	virtual int vfn_6C();
	virtual void vfn_70(int arg);
	virtual void vfn_74(int arg);
	virtual int vfn_78();
	virtual void vfn_7C();
	virtual void vfn_80();
	virtual int vfn_84();
	virtual int vfn_88();
	virtual int vfn_8C();
	virtual void vfn_90(ItemSpec* spec);
	virtual void vfn_94(ItemSpec* spec);
	virtual void vfn_98();
	virtual void vfn_9C(int arg0, int arg1);
	// TChrPlayer gives these The first two have no default so their slots are 0
	virtual u16 vfn_A0() = 0;                             // _A0
	virtual void vfn_A4(void* model, void* texList) = 0;  // _A4
	virtual void vfn_A8(int joint, f32* mtx);          // _A8 0x80013790
	virtual void vfn_AC(int joint, f32* mtx) const;          // _AC 0x80013794
	virtual void vfn_B0(const int* motions);                 // _B0 0x80013798
	virtual void vfn_B4(int on);                       // _B4 0x8001379C
	virtual void resetCamera();                        // _B8 0x800137A0
	virtual void vfn_BC(const int* motions, f32* mtx);       // _BC 0x800137A4
	virtual void vfn_C0(const Vec3f* pos, const EggRot* rot);         // _C0 0x800137A8
	virtual void vfn_C4(const int* motions, f32* mtx);       // _C4 0x800137AC
	virtual void vfn_C8(const int* motions, f32* mtx);       // _C8 0x800137B0
	virtual void vfn_CC();                             // _CC 0x800137B4
	virtual void vfn_D0();                             // _D0 0x800137B8
	virtual int vfn_D4();                    // _D4 0x800137BC
	virtual int vfn_D8();                    // _D8 0x800137C4

	u8 mCharId;    // _30
	u8 _31[3];
	int _34;       // _34
	LightSet* mStatus; // _38
};

/** The character in play with its motions and its state machine */
extern "C" void objHeapFree(void*, void*);

/** How event_lose and event_super in superbilly.arc start the camera's object and motion then the motions the player
 * plays */
struct PlyDemoExport {
	void* mCamObject; // _00
	void* mCamMotion; // _04
	u32 mMotions[5];  // _08
};

// A message to another object
struct PlyMsg {
	inline PlyMsg(u16 type) { mFrom = 0xFFFF; mType = type; }

	u16 mType; // _00
	u8 _02[2];
	u16 mFrom; // _04
	u16 mTo;   // _06
};

// The same message built type first
struct PlyMsgT {
	inline PlyMsgT(u16 type) { mType = type; mFrom = 0xFFFF; }

	u16 mType; // _00
	u8 _02[2];
	u16 mFrom; // _04
	u16 mTo;   // _06
};

class MeshColli;
struct ColliContact;

// One of the player's probes: what an AniRider holds and the normal it touched as well
struct PlyRider {
	PlyRider* mNext;  // _00
	PlyRider* mPrev;  // _04
	PlyRider* mSelf;  // _08
	void* mList;      // _0C
	MeshColli* mMesh; // _10 the mesh it stands on or null
	u16 mTri;         // _14 the triangle it touched
	u8 _16[2];
	Vec3f mPos;       // _18 where it touched in the mesh's space
	Vec3f mNormal;    // _24
};

class TChrPlayer : public TObjPlayer {
public:
	TChrPlayer(TObject* parent, int playerNo, u8 charId, int active);
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	virtual ~TChrPlayer();
	virtual void draw();   // _0C runs every frame despite the name inherited from TObject
	virtual void vfn_10(); // _10
	virtual Vec3f* getPos();                        // _20
	virtual void getNodeMatrix(int node, f32* mtx); // _24
	virtual int getRotY();                          // _28
	virtual f32 vfn_40();                           // _40
	virtual int canAct();                           // _44
	virtual int sendToHandle(ObjMsg* msg);          // _4C
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58
	virtual u16 vfn_A0();                           // _A0 the egg's id
	virtual void vfn_A4(void* model, void* texList); // _A4 swaps the outer model
	virtual void vfn_A8(int joint, f32* mtx);       // _A8 copies a joint's matrix
	virtual void vfn_AC(int joint, f32* mtx) const;       // _AC the same for the outer model
	virtual void vfn_B0(const int* motions);              // _B0
	virtual void vfn_B4(int on);                    // _B4 whether the pad is read
	virtual void resetCamera();                     // _B8 shows the player and starts Appear
	virtual void vfn_BC(const int* motions, f32* mtx);    // _BC
	virtual void vfn_C0(const Vec3f* pos, const EggRot* rot);      // _C0
	virtual void vfn_C4(const int* motions, f32* mtx);    // _C4
	virtual void vfn_C8(const int* motions, f32* mtx);    // _C8
	virtual void vfn_CC();                          // _CC
	virtual void vfn_D0();                          // _D0
	virtual int vfn_D4();                           // _D4
	virtual int vfn_D8();                           // _D8
	virtual void setPos(const Vec3f* pos);                // _DC

	// What runs when a status bit changes from the table at gPlayerStatusEntries
	void statusFlag2(int on);
	void statusInvincible(int on);
	void statusLowHealth(int on);
	void statusSleep(int on);
	void statusStop(int on);
	void statusBlind(int on);
	void statusDrain(int on);
	void statusStrength(int on);
	void statusSuper(int on);
	void statusDoze(int on);

	void reset();
	void setHealth(int amount);
	int getBodyRow();
	int getHealth();
	void addHealth(int amount);
	void playMotion(int row);
	void blendMotion(int row);
	void setMotionSpeed(f32 speed);
	void updateMotions();
	void poseModels(int force);
	void updateJoints();
	void updateMatrix();
	void updatePose();
	void setMove(f32 amount, int kind);

	inline void** vtable() { return *(void***)((u8*)this + 0x18); }
	inline PlyEffectList* effects() { return &mEffects; }

	u8 _3C[0xC];            // _3C plyClearWord
	PlyEffectList mEffects; // _48
	u16 _D0;                // _D0
	u8 _D2[2];
	u8 _D4;                 // _D4
	u8 _D5;                 // _D5
	u16 mTimerD6;           // _D6 counts down
	u16 mAge;               // _D8 counts up
	u8 _DA[2];
	PlyHealth mHealth;      // _DC
	void** mBodyBuffer;     // _EC
	DrawTask* mDrawTask;        // _F0
	DrawTask* mShadowTask;      // _F4 0 when the character casts no separate shadow
	u16 mTimerF8;           // _F8 counts down
	u8 _FA[2];
	CharaColli* mEmitter0;        // _FC
	CharaColli* mEmitter1;        // _100
	f32 mMoveAmount;        // _104 the stick's strength
	int mMoveAngle;         // _108 the stick's angle
	ObjRef mEggRef;         // _10C the egg held now
	ObjRef mLastEggRef;     // _118 the last egg held
	ObjRef mThrownEggRef;   // _124 the egg just let go of
	ObjRef mHangRef;        // _130 what the player hangs from
	ObjRef mMountRef;       // _13C what the player rides
	u8 _148[0x38];          // _148 an ObjShadow
	int mAngle;             // _180
	int _184;               // _184
	void* mLeftHandBuffer;  // _188
	void* mRightHandBuffer; // _18C
	void* mFaceBuffer;      // _190
	int _194;               // _194
	u16 _198;               // _198
	u8 _19A[0xE];
	u8 _1A8[0xC];           // _1A8 plyTintStop
	f32 mMatrix[12];        // _1B4
	u16 mTargetId;          // _1E4
	u8 _1E6[6];
	u8 _1EC[0x50];          // _1EC a SpawnGuard in the first 0x20
	Vec3f _23C;             // _23C
	Vec3f _248;             // _248
	void* mStates;          // _254 a stack of states initPlyPool
	void* mState;           // _258
	TMobileObject* mMobile; // _25C
	u8 _260[0x10];
	f32 _270[12];           // _270 the player's matrix against the body they ride
	u16 _2A0;               // _2A0 what the Place state is on
	u16 _2A2;               // _2A2 the part of it
	PlyInput mInput;        // _2A4
	u8 _2BC[0xC];           // _2BC
	TSpeakCursor* _2C8;     // _2C8
	int _2CC;               // _2CC
	Vec3f mPrevPos;         // _2D0
	u8 _2DC[4];
	Vec3f mPos;             // _2E0
	Vec3f mVel;             // _2EC
	Vec3f _2F8;             // _2F8
	EggRot mRot;            // _304
	TPlyComb* mModel;       // _314
	u8 _318;                // _318
	u8 _319;
	u16 mFade;              // _31A
	u8 _31C[0x4];           // _31C
	u32 _320;               // _320
	u32 _324;               // _324
	u32 _328;               // _328
	u32 _32C;               // _32C
	u32 _330;               // _330
	u32 _334;               // _334
	int _338;               // _338
	volatile u32 mPlayFlags;         // _33C
	Vec3f _340;             // _340
	TPlyEyelid* _34C;       // _34C
	MotionPlayer mBody;     // _350 the motions export
	MotionPlayer mBodyCopy; // _38C the same table and motions on a second buffer
	MotionPlayer mFace;     // _3C8 fa_mtn with fa_sm
	MotionPlayer mRightHand; // _404 rh_mtn
	MotionPlayer mLeftHand; // _440 lh_mtn
	PlyPose* mPose;         // _47C
	u8 _480[8];
	u8* mJoints;            // _488
	int mJointCount;        // _48C
	u8 _490[4];
	Vec3f _494;             // _494 where the player stands on a moving mesh in its space
	Vec3f _4A0;             // _4A0 the facing likewise
	u8 _4AC[0xC];
	Vec3f _4B8;             // _4B8 the way a ledge faces likewise
	Vec3f mStartPos;        // _4C4
	TObject* _4D0;          // _4D0 the status effects are its children
	int _4D4;               // _4D4 health saved by status 4
	u32 mBitsD8;            // _4D8
	u32 mBitsDC;            // _4DC tested with flagTest
	u32 mBitsE0;            // _4E0
	u32 mBitsE4;            // _4E4
	void** mModels;         // _4E8 the models export
	void** mTexLists;       // _4EC the texlists export
	u8 _4F0[0x18];          // _4F0 an ItemHolder
	u8 _508[0x18];          // _508 an ItemHolder
	void* _520;             // _520
	PlyRider mRiders[8];    // _524 riderArrayCtor
};

class ObjRef;
class TEggCreature;
class TMobileObject;
class TObjAction;
class TObject;
struct MotionPlayer;
struct ObjMsg;
struct PlyInput;
struct Vec3f;

// One joint's pose as plyReadNodes fills it and plyBuildNodes turns into a matrix
struct PlyPose {
	Vec3f mPos;   // _00
	s32 mAng[3];  // _0C
	Vec3f mScale; // _18
	f32 mQuat[4]; // _24 a rotation copied as one block
	int mRotMode; // _34 what njGetMotionNodeData returned 0 and 1 rotate in that order, else mQuat
	f32 mMtx[12]; // _38
};

extern "C" {
void clearFlagByte(void*);
void drawPlayerMotion(void*, MotionPlayer*);
PlyEffect* effectListAdd(PlyEffectList*);
PlyEffect* effectListAlloc(PlyEffectList*);
void effectListClear(PlyEffectList*);
void effectListFree(PlyEffectList*, PlyEffect*);
void effectListRemove(PlyEffectList*, PlyEffect*);
int flagTest(void*, int);
void flagWordOr(void*, void*);
void flagWordXor(void*, void*);
void initPlyStateSlot(void**);
void plyAddGroundMotion(TChrPlayer*, Vec3f*);
void plyAddStatus(TChrPlayer*, int, u16);
void plyAirControl(TChrPlayer*);
void plyAirNudge(TChrPlayer*);
void plyApplyEggForm(TChrPlayer*, TObjAction*);
void plyApplyGravity(TChrPlayer*);
void plyApplyScale(TChrPlayer*, f32*);
void plyApplySpec(TChrPlayer*, const ItemSpec*);
f32 plyApproach(f32, f32, f32);
f32 plyApproachAngle(f32, int, int);
int plyAttachEgg(TChrPlayer*, TObjAction*);
int plyAttachEgg2(TChrPlayer*, TObjAction*);
void plyBrakeMove(TChrPlayer*);
int plyBuildNodes(NJS_OBJECT*, int, void*);
void plyBuildPose(void**, void*);
int plyCheckBrake(TChrPlayer*, f32, int);
void plyCheckDeathRay(TChrPlayer*, Vec3f*, Vec3f*);
int plyCheckEggLost(TChrPlayer*);
void plyClearHeldEffect(TChrPlayer*);
void plyClearProbe(TChrPlayer*, int);
void plyCollectPushes(TChrPlayer*);
void plyDashMove(TChrPlayer*);
int plyDefaultMessage(TChrPlayer*, ObjMsg*);
void plyDie(TChrPlayer*);
int plyDoubleJump(TChrPlayer*, PlyInput*);
void plyDrawBegin(TChrPlayer*);
void plyDrawEnd(TChrPlayer*);
void plyDrawGlassCoord(u8);
void plyDrawGlassTev(u8);
void plyDrawParts(TChrPlayer*);
void plyDrawTask(void*);
void plyDrawTaskFlat(void*);
void plyDrawTaskGlass(void*);
void plyDrawTaskShadow(void*);
void plyDropItems(TChrPlayer*);
void plyEmpty7();
void plyEndMove(TChrPlayer*);
void plyEndStatusEffect(TChrPlayer*, int);
PlyEffect* plyFindEffect(TChrPlayer*, u8, u8);
u16 plyFindEggBehind(TChrPlayer*);
TObjAction* plyFindEggInReach(TChrPlayer*, f32, int);
void plyFindGroundFalling(TChrPlayer*);
void plyFindGroundWalking(TChrPlayer*);
int plyFindLanding(TChrPlayer*);
void plyFollowMobile(TChrPlayer*);
void plyFootstepFx(TChrPlayer*);
int plyForceJump(TChrPlayer*, f32, int);
void plyForgetEgg(TChrPlayer*);
TObjAction* plyGetEgg(TChrPlayer*);
u16 plyGetHurtTimer(TChrPlayer*);
int plyGetItemIds(TChrPlayer*, int*, int*);
f32* plyGetJoint(TChrPlayer*, int);
void plyGetMatrix(TChrPlayer*, void*);
TMobileObject* plyGetMobile(TChrPlayer*);
void* plyGetMount(TChrPlayer*);
int plyGetMove(TChrPlayer*, f32*, int*);
TObjAction* plyGetPlaceObject(const TChrPlayer*);
int plyGetStatusBits(TChrPlayer*);
int plyGetTurnKind(TChrPlayer*);
int* plyGlobalsClear();
void plyGlobalsEmpty();
void plyHandSearchHit(ColliContact*, PlyMsg*, int*);
void plyHangOnLedge(TChrPlayer*);
void plyHangOnObject(TChrPlayer*);
void plyHide(TChrPlayer*);
void plyHoldEgg(TChrPlayer*, TObjAction*);
void plyHoldJump(TChrPlayer*);
int plyIsDrawn(TChrPlayer*);
int plyIsFalling(TChrPlayer*);
int plyIsMobileSlow(TChrPlayer*);
void plyJumpHold(TChrPlayer*);
void plyJumpMove(TChrPlayer*);
int plyJumpOffEgg(TChrPlayer*);
void plyKeepSteering(TChrPlayer*);
void plyLayOnGround(Vec3f*, Vec3f*, Vec3f*);
void plyLeaveGround(TChrPlayer*);
void plyLetGoAll(TChrPlayer*);
void plyLoadShared();
void plyLoseAllAndDie(TChrPlayer*);
void plyLoseItems(TChrPlayer*);
int plyMobileStopped(TChrPlayer*);
int plyMobileUnderTen(TChrPlayer*);
void plyMountMobile(TChrPlayer*);
void plyMountSearchHit(ColliContact*, u8*, int*);
void plyMoveAndCollide(TChrPlayer*);
void plyMoveAndStep(TChrPlayer*);
void plyMoveByVel(TChrPlayer*);
int plyMsgAltModels(TChrPlayer*);
int plyMsgDie(TChrPlayer*);
int plyMsgDisable(TChrPlayer*);
int plyMsgEnable(TChrPlayer*);
int plyMsgEvent(TChrPlayer*);
int plyMsgEventAt(TChrPlayer*, u8*);
int plyMsgGetEggId(TChrPlayer*);
int plyMsgGetId(TChrPlayer*);
int plyMsgGetLookPos(TChrPlayer*, u8*);
int plyMsgGetStatus(TChrPlayer*);
int plyMsgPlace(TChrPlayer*, u8*);
int plyMsgPush(TChrPlayer*, u8*);
int plyMsgRestart(TChrPlayer*);
int plyMsgSetMount(TChrPlayer*, u8*);
int plyMsgSink(TChrPlayer*);
int plyMsgSorry(TChrPlayer*);
int plyMsgStageClear(TChrPlayer*, u8*);
int plyMsgVictory(TChrPlayer*);
int plyNoLedge(TChrPlayer*);
int plyNodeAny();
u8 plyNodeFrom26(int);
u8 plyNodeTo25(int);
int plyOnGround(TChrPlayer*);
int plyOnSlipperyGround(TChrPlayer*);
int plyOnSteepGround(TChrPlayer*);
int plyPickUpEgg(TChrPlayer*, TObjAction*);
void plyPlaceOnMount(TChrPlayer*);
void plyPlaySe(TChrPlayer*, int);
void plyPlayVoice(TChrPlayer*, int, int);
void plyPosFromMatrix(TChrPlayer*);
void plyPoseAtFrame(TChrPlayer*, f32);
void plyProjectOnPlane(Vec3f*, Vec3f*, Vec3f*);
int plyPushEgg(TChrPlayer*, TObjAction*);
int plyPushEggChecked(TChrPlayer*, TObjAction*);
void plyPushFromWalls(TChrPlayer*);
int plyRayCrossSegment(Vec3f*, Vec3f*, Vec3f*, Vec3f*, Vec3f*, Vec3f*);
int plyReadLinkNodes(NJS_OBJECT*, void*, int, void*);
void plyReadMotion(NJS_OBJECT*, MotionPlayer*, void*, void*);
void plyReadMotionLink(NJS_OBJECT*, void*, void*, void*, f32);
int plyReadMotionPlain(NJS_OBJECT*, void*, void*, void*, f32);
int plyReadNodes(NJS_OBJECT*, void*, int, void*);
void plyReleaseEgg(TChrPlayer*, TObjAction*);
void plyReleaseKickedEgg(TChrPlayer*);
void plyReleaseRef(TChrPlayer*, ObjRef*);
void plyReportToGame(TChrPlayer*);
void plyRideMove(TChrPlayer*);
void plyRollMove(TChrPlayer*);
void plyRumble(TChrPlayer*, int);
int plySearchDamage(TChrPlayer*);
void plySendMountOff(TChrPlayer*);
void plySetAngle(TChrPlayer*, int);
void plySetEyelidNodes(f32, PlyPose*, PlyPose*);
int plySetFlagBounce(TChrPlayer*);
void plySetGroundVel(TChrPlayer*, f32, int);
int plySetHangObject(TChrPlayer*, u16, u16);
void plySetHeldEffect(TChrPlayer*, const ItemSpec*);
void plySetMoveFromInput(TChrPlayer*, PlyInput*);
int plySetMoveIfPushed(TChrPlayer*, f32, int);
int plySetMoveIfPushed2(TChrPlayer*, f32, int);
void plySetShadow(TChrPlayer*, int);
void plyShowShadowTask(TChrPlayer*, int);
void plySlamEgg(TChrPlayer*);
void plySlideDownSlope(TChrPlayer*);
void plySlideMove(TChrPlayer*);
void plySlideOnEgg(TChrPlayer*);
void plySpawnCrow(TChrPlayer*);
void plyStandOnMobile(TChrPlayer*);
int plyStartAim(TChrPlayer*);
void plyStartEvent(TChrPlayer*);
void plyStartStatusEffect(TChrPlayer*, TObject*);
void* plyStateSlotDtor(void*, s16);
int plyStateVfn2C(const TChrPlayer*);
int plyStateVfn30(const TChrPlayer*);
void plyStopAim(TChrPlayer*);
int plyStopIfGrounded(TChrPlayer*);
int plyStopIfMounted(TChrPlayer*);
void plyTakeItem(TChrPlayer*, const ItemSpec*);
int plyTakePickup(TChrPlayer*, u8*);
int plyThrowWornItem(TChrPlayer*);
void plyThrownOff(TChrPlayer*, const Vec3f*);
void plyToStartPoint(TChrPlayer*);
int plyTryBounce(TChrPlayer*);
int plyTryBounceJump(TChrPlayer*);
int plyTryCatchLedge(TChrPlayer*);
int plyTryChangeMount(TChrPlayer*);
int plyTryEggJump(TChrPlayer*, f32, int);
int plyTryEndBrake(TChrPlayer*, f32, int);
int plyTryHop(TChrPlayer*);
int plyTryJump(TChrPlayer*, f32, int);
u16 plyTryLandOnEgg(TChrPlayer*);
int plyTryLaunchEgg(TChrPlayer*, f32, int);
int plyTryLeaveMount(TChrPlayer*);
int plyTryLetGoEgg(TChrPlayer*, f32, int);
int plyTryPickUpInFront(TChrPlayer*, f32, int);
int plyTryRollDash(TChrPlayer*);
int plyTrySpinBounce(TChrPlayer*);
int plyTrySpinDash(TChrPlayer*, f32, int);
int plyTrySpinStand(TChrPlayer*, f32);
int plyTryTalk(TChrPlayer*);
int plyTryThrow(TChrPlayer*);
void plyTurnAboutNormal(Vec3f*, Vec3f*, Vec3f*, int, int);
void plyUnloadShared();
void plyUpdateMatrixPose(TChrPlayer*);
void plyUpdateSpeakCursor(TChrPlayer*);
void plyUpdateStatus(TChrPlayer*);
void plyUseHeldItem(TChrPlayer*);
int plyUseWornItem(TChrPlayer*);
void plyWalkMove(TChrPlayer*);
int plyWallProbe(TChrPlayer*, f32, void*, Vec3f*, Vec3f*, Vec3f*, PlyRider*);
void* riderArrayCtor(void*);
void* riderArrayDtor(void*, s16);
void runPlyState(void**, TChrPlayer*);
}

extern "C" {
void flagByteSet(void*, int);
}

extern "C" {
int plyGlobalsInit();
TObjAction* plyGetHangObject(TChrPlayer*);
void plyPlayCopyMotion(TChrPlayer*, MotionPlayer*, int);
int plyDeathRayHit(f32, MeshColli*, u16, const Vec3f*, PlyRayHit*);
void plyPlayMotionSynced(MotionPlayer*, int);
int plySetMoveIfAny(TChrPlayer*, f32, int);
void plyEggSearchHit(ColliContact*, PlyEggSearch*, int*);
void plyDamageSearchHit(ColliContact*, PlyDamageSearch*, int*);
void plyEdgeSearchHit(MeshColli*, u16, PlyEdgeSearch*);
void plyGrabSearchHit(ColliContact*, PlyGrabSearch*, int*);
void plyRollSearchHit(ColliContact*, PlyRollSearch*, int*);
void plyTalkSearchHit(ColliContact*, PlyTalkSearch*, int*);
void plySlamSearchHit(ColliContact*, PlySlamSearch*, int*);
u16 plyGiveEgg(TChrPlayer*);
int plyMoveRayHit(f32, MeshColli*, u16, const Vec3f*, PlyMoveHit*);
void* plyGetProbe(const TChrPlayer*, int, u16*, Vec3f*, Vec3f*);
int plyGroundRayHit(f32, MeshColli*, u16, const Vec3f*, PlyGroundHit*);
int plyAnyRayHit(f32, MeshColli*, u16, const Vec3f*, PlyGroundHit*);
void* changePlyState(void**, TChrPlayer*, void*);
void jointArrayInit(PlyArray*);
PlyArray* jointArrayDtor(PlyArray*, s16);
void jointArrayResize(PlyArray*, int);
void poseArrayInit(PlyArray*);
PlyArray* poseArrayDtor(PlyArray*, s16);
void poseArrayResize(PlyArray*, int);
void poseArrayReserve(PlyArray*, int);
void jointArrayReserve(PlyArray*, int);
void jointArrayAlloc(PlyArray*, int);
void poseArrayAlloc(PlyArray*, int);
void poseArrayCopy(PlyArray*, PlyPose*, PlyPose*, int);
void jointArrayCopy(PlyArray*, PlyJoint*, PlyJoint*, int);
}

extern "C" {
int plyHasNormalModels(TChrPlayer*);
int plyIsAiming(TChrPlayer*);
bool plyTryPickUpBehind(TChrPlayer*);
int plyTryGrabEdge(TChrPlayer*, f32, int);
int plyBounceFlag(TChrPlayer*);
}

extern "C" {
int plyHasMobile(TChrPlayer*);
int plyStopInAir(TChrPlayer*);
}

extern "C" {
extern Vec3f gAxisZ;
extern f32 sPlySpeedLimit;
extern NJS_MOTION** sPlayerMotions;
extern NJS_TEXLIST** sPlayerTexLists;
// Only what the on-foot states read of the settings at gPlyParams
struct PlyParams {
	u8 _00[0x2];
	u16 mDamageTime;         // _02 frames of the Damage state
	u16 _04;                 // _04
	u8 _06[0x6];
	u16 _0C;                 // _0C
	u16 _0E;                 // _0E
	u8 _10[0x2];
	u16 mCrowRecover;        // _12 frames of Crow's getting-up phase
	u16 mCrowHold;           // _14 frames before Crow escapes on its own
	u8 _16[0x12];
	int _28;                 // _28 an angle
	u8 _2C[0x10];
	f32 _3C;                 // _3C
	f32 mSteepSlope;         // _40
	u8 _44[0x4];
	f32 mDrag;               // _48 how much of the speed is lost each frame
	f32 _4C;                 // _4C
	f32 mRunSpeed;           // _50
	f32 mWalkMotionSpeed;    // _54
	f32 mTurnSpeed;          // _58 per frame at 60 frames a second
	f32 _5C;                 // _5C
	f32 mAccel;              // _60 how fast the speed approaches its target
	f32 mRunMotionSpeed;     // _64
	u8 _68[0x4];
	f32 _6C;                 // _6C
	f32 _70;                 // _70
	f32 _74;                 // _74
	f32 _78;                 // _78
	f32 _7C;                 // _7C
	u8 _80[0x10];
	f32 _90;                 // _90
	u8 _94[0x4];
	f32 _98;                 // _98
	f32 _9C;                 // _9C
	f32 _A0;                 // _A0
	f32 _A4;                 // _A4
	f32 mSlipNormal;         // _A8 ground normal Y Below this Slip keeps its timer full
	f32 _AC;                 // _AC
	f32 _B0;                 // _B0
	f32 _B4;                 // _B4
	u8 _B8[0xC];
	f32 _C4;                 // _C4
};
extern PlyParams* gPlyParams;
extern int lbl_802C4D68;
extern int lbl_802C4D6C;
extern int lbl_802C4D70;
extern int lbl_802C4D74;
extern int lbl_802C4D78;
extern int lbl_802C4D7C;
extern int lbl_802C4D80;
extern int lbl_8027FAC0[];
extern Vec3f sPlayerScreenPos;
extern int sPlayerStartAngle[]; // sPlayerStartPos is a HitVec array declared in Player.cpp
extern MotionEntry gPlayerBodyMotions[66];
extern MotionEntry sPlayerFaceMotions[8];
extern MotionEntry sPlayerLeftHandMotions[13];
extern MotionEntry sPlayerRightHandMotions[13];
extern u8 sPlayerFormPairs[];
extern int sPlayerFormFlags[][2];
extern const char* gTPlyStateName;
}

#endif
