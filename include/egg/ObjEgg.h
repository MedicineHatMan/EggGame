#ifndef _OBJEGG_H
#define _OBJEGG_H
struct SetDesc;
struct TObject;
struct Fx;
struct CharaColli;

#include "core/types.h"
struct NJS_MOTION;
struct NJS_OBJECT;
struct NJS_TEXLIST;
struct DrawTask;
#include "engine/MotionPlayer.h"
#include "player/PlyInput.h"
#include "object/ObjCommon.h"
#include "stage/SetManager.h"
#include "engine/Rail.h"
#include "animal/AniRider.h"

// The eggs Billy rolls around

// What a state function is being asked to do
enum EggPhase {
	EGG_ENTER  = 0,
	EGG_EXIT   = 1,
	EGG_DRAW   = 2,
	EGG_UPDATE = 3,
};

enum EggState {
	EGG_STATE_NONE      = -1,
	EGG_STATE_IDLE      = 0,
	EGG_STATE_HELD      = 1,
	EGG_STATE_SLAM      = 2, // thrown at the ground and makes a shock wave when it lands
	EGG_STATE_HATCH     = 3,
	EGG_STATE_ROLLED    = 4,
	EGG_STATE_THROWN    = 5,
	EGG_STATE_BOUNCE    = 6,
	EGG_STATE_SLOW      = 7,
	EGG_STATE_EXPIRE    = 8, // left idle too long
	EGG_STATE_APPEAR    = 9,
	EGG_STATE_FALL      = 10,
	EGG_STATE_BOUNCE_UP = 11,
	EGG_STATE_ROLLED_P  = 12, // rolled tied to a player
	EGG_STATE_ROLLED_P2 = 13,
};

struct EggAxis {
	f32 v[3];
};

// A quaternion w first
struct EggRot {
	inline void operator=(const EggRot& other)
	{
		mW    = other.mW;
		mAxis = other.mAxis;
	}

	f32 mW;        // _00
	EggAxis mAxis; // _04
};

// What a holder sends to pick it up and what it gets back
struct EggHoldMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	void* mHandle; // _08
	Vec3f mPos;    // _0C
	EggRot mRot;   // _18
	f32 mRadius;   // _28
};

// Message 0x64 and 0x71 let go with this speed
struct EggThrowMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	Vec3f mVel; // _08
};

// Message 0x6B throw it along a path
struct EggPathMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	void* mPath;  // _08
	void* mPoint; // _0C
	f32 mSpeed;   // _10
};

// Message 0x18
struct EggFeedMsg {
	u16 mType; // _00
	u8 _02[2];
	u8 mFood;   // _04
	u8 mAmount; // _05
};

// Message 0x2A
struct EggKickMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mAngle; // _04
};

// Message 0x76 asks how grown it is and how many hits it has taken
struct EggLevelMsg {
	u16 mType; // _00
	u8 _02[2];
	f32 mProgress; // _04
	u8 mHits;      // _08
};

// An element of EggHit's array
struct HitVec : public Vec3f {
	HitVec() { }
	HitVec& operator=(const Vec3f& v)
	{
		Vec3f::operator=(v);
		return *this;
	}
};

struct EggHit { // size 0x4C
	EggHit();

	HitVec _00[3]; // _00
	Vec3f mPos;    // _24
	Vec3f mNormal; // _30
	u8 _3C[0x10];
};

// EggHit's layout with the implicit constructor so a local builds its three HitVecs with __construct_array
struct EggProbe {
	HitVec mPoints[3]; // _00
	Vec3f mPos;        // _24
	Vec3f mNormal;     // _30
	u8 _3C[0x10];
};

// What meshColliProbe fills in
struct EggHits {
	EggHit mHits[3]; // _00
	u32 mMask;       // _E4
};

/**
 * One row of egglevel.bin's size table.
 * @size 0x4
 */
struct EggSize {
	u16 mRadius; // _00 20 times the radius
	u16 mPower;  // _02
};

/**
 * One type of egg in egglevel.bin.
 * @size 0x10
 */
struct EggType {
	u8 mId; // _00
	u8 _01[2];
	u8 mMaxPoints; // _03
	u8 mMinSize;   // _04
	u8 mMaxSize;   // _05
	u8 mMask[10];  // _06 [0] marks the foods worth double [n] is the points size step n needs 0 = no such step
};

/** The contents of egglevel.bin */
struct EggTable {
	int mSizeCount;   // _00
	EggSize* mSizes;  // _04
	int mTypeCount;   // _08
	EggType* mTypes;  // _0C
	int mRandomCount; // _10
	u8* mRandom;      // _14 512 type IDs to pick from when a record asks for type 99
};

/**
 * Which type of egg this is, and how far it has grown.
 * @size 0x8
 */
struct EggLevel {
	EggLevel();

	static int pickRandomType(f32 random);
	void setType(int id);
	int getType();
	int getMaxSize();
	int getMinSize();
	f32 getRadius() const;
	int getPower();
	int sizeForPoints(int points);
	int feed(int food, int amount);
	void setPoints(int points);
	void getProgress(void* msg);

	s8 mSize;   // _00 a step in EggTable::mSizes from 1
	s8 mPoints; // _01
	u8 _02[2];
	EggType* mType; // _04
};

// One of an EggBody's riders 0 is what it rests on 2 the last mesh its move ran into 1 is unused here
struct EggBodyItem {                              // size 0x24 laid out like an AniRider
	EggBodyItem() { riderInit((AniRider*)this); } // 0x8008FBB8
	~EggBodyItem();                               // 0x8008B2B8 TMobileObject.cpp

	AniRider* mNext;  // _00
	AniRider* mPrev;  // _04
	AniRider* mSelf;  // _08
	void* mList;      // _0C
	MeshColli* mMesh; // _10 the mesh it stands on or null
	u16 mTri;         // _14 the triangle it touched
	u8 _16[2];
	Vec3f mPos; // _18 where it touched in the mesh's space
};

class MeshColli;
struct MeshList;

// What move calls for the face it stopped against and for each face the ball touches on the way nobody passes one of
// those where the ball touched the face's normal the caller's argument and how far along the sweep it was
typedef void (*EggFaceFn)(MeshColli* mesh, u16 tri, Vec3f* pos, Vec3f* normal, void* arg, f32 dist);

/**
 * Position and motion, and a ball's collision with the meshes. The shared rolling code in EggRoll::pushOff and friends
 * works on this. See EggBody.cpp.
 * @size 0x118
 */
// A particle generator as the aura sees it
struct EggParticle {
	u8 _00[4];
	u16 mFlags; // _04 1 = finish and free
	u8 _06[0x1E];
	Vec3f mPos; // _24
};

struct EggBody {
	EggBody();                                                           // 0x8008FADC
	virtual ~EggBody() { }                                               // _08 inline everywhere its copy is at 0x8008FA48
	virtual void setPos(const Vec3f* pos);                               // _0C 0x80090C5C
	virtual f32 getBounce(MeshColli* mesh, u16 tri, void* arg);          // _10 0x8008FC30 how much of the speed into a face comes back
	virtual f32 getFriction(MeshColli* mesh, u16 tri, void* arg);        // _14 0x8008FC38
	virtual int canStop(MeshColli* mesh, u16 tri, u32 flags, void* arg); // _18 0x8008FC40 whether a face stops the ball

	void makeMatrix(f32* mtx);                                                                             // 0x8008FBE8
	void stepRoll(f32 dt, void* arg, u32 mask, EggFaceFn graze, EggFaceFn face);                           // 0x8008FC48
	void stepSlide(f32 dt, void* arg, u32 mask, EggFaceFn graze, EggFaceFn face);                          // 0x8008FC90
	void move(f32 dt, void* arg, u32 mask, EggFaceFn graze, EggFaceFn face, int roll);                     // 0x8008FCD8
	void spin(f32 dt);                                                                                     // 0x80090234
	void followMesh(Vec3f* pos);                                                                           // 0x80090318
	MeshColli* sweep(MeshList* list, const Vec3f* from, const Vec3f* to, Vec3f* pos, f32* dist, u16* tri); // 0x8009043C
	MeshColli* touching(int slot, u16* tri, Vec3f* pos, Vec3f* normal) const;                              // 0x80090670

	FlagByte mFlags;       // _04 what the last sweep ran into 2 = anything 3-6 = surface kinds
	EggBodyItem mItems[3]; // _08
	Vec3f mAccel;          // _74 cleared by every move
	Vec3f mPos;            // _80
	Vec3f mVel;            // _8C
	Vec3f mAngVel;         // _98
	Vec3f mPush;           // _A4 added to the next move
	EggRot mRot;           // _B0
	f32 mRadius;           // _C0
	union {
		f32 mBounce;     // _C4
		u32 mBounceBits; // _C4 for the callers that clear it as a word
	};
	f32 mFriction;    // _C8
	u32 mStopMask;    // _CC the surface kinds that set mFlags
	f32 mMeshMtx[12]; // _D0 the matrix of the mesh it rests on when it rested on it
	Vec3f mMeshPos;   // _100 where it rested in that mesh's space
	u8 _10C[0xC];
};

// The body TMobileObject uses Its vtable is at 0x80259DD8
struct EggSurface : public EggBody {
	EggSurface() { }
	virtual ~EggSurface();
	virtual f32 getBounce(MeshColli* mesh, u16 tri, void* arg);
	virtual f32 getFriction(MeshColli* mesh, u16 tri, void* arg);
	virtual int canStop(MeshColli* mesh, u16 tri, u32 flags, void* arg);
};

// A rider on a rail where it is and its link in the rail's list of riders
struct EggCollide { // size 0x1C
	EggCollide();
	~EggCollide();

	void getNormal(Vec3f* out) const;
	f32 getDistance() const;
	void getDir(Vec3f* out) const;
	void getPos(Vec3f* out) const;
	void setPoint(const RailPoint* point);
	void setPath(RailPath* path);
	void advance(f32 dist);

	int mSeg;        // _00
	f32 mDist;       // _04 past the start of the segment
	RailLink mLink;  // _08
	RailPath* mPath; // _18
};

// Flickers between two radii for a second when the egg changes size
struct EggGrow { // size 0xC
	EggGrow();

	int isActive();
	void start(f32 from, f32 to);
	void tick();
	f32 getTarget() const;
	void getScale(void* mtx) const;

	int mTimer; // _00
	f32 mFrom;  // _04
	f32 mTo;    // _08
};

class TMobileObject;
struct ObjMsg;

/**
 * What a holder keeps of the thing it holds. Its vtable is 0x8025813C: the destructor and a pure onMessage.
 * Each holdable thing derives its own. See TMobileObject.cpp.
 * @size 0x10
 */
struct EggHandleBase {
	EggHandleBase(); // 0x8008B014

	void release();        // 0x8008B094
	int send(ObjMsg* msg); // 0x8008B0D8 to the holder if there is one else to onMessage

	TMobileObject* mHolder; // _00
	void* mOwner;           // _04
	int mDamage;            // _08

	// _0C = vtable The members come first so it does too
	virtual ~EggHandleBase();               // _08 0x8008B030 lets go of the holder
	virtual int onMessage(ObjMsg* msg) = 0; // _0C
};

/**
 * The part of the egg that other objects talk to while they hold it.
 * @size 0x14
 */
struct EggHandle : public EggHandleBase {
	EggHandle() { }
	inline ~EggHandle();

	inline virtual int onMessage(ObjMsg* msg);

	u32 _10; // _10
};

// Collected while moving
struct EggHitInfo {
	FlagWord mBits; // _00
	u32 mMask;      // _04 the flags of every face touched
	void* mCb;      // _08
	void* mArg;     // _0C
};

class TObjEgg;

struct EggRoll { // size 0x30
	EggRoll();

	void setVel(const Vec3f* vel);
	void startSlam(const Vec3f* vel);
	void attachPath(void* path, void* point);
	void attachPathAt(void* path, void* point, f32 speed);
	void kick(u16 unused);
	void rebound();
	void kickHeld();
	void reboundHard();
	void stopRolling();
	void leavePath();
	u32 getMask();
	int hitWhileRolling(int* result, int power);
	int hitWhileHeld(int* result, int power);
	void pushOff(void* coll, void* first, void* end);
	void moveGround();
	void moveRolling();
	void movePath();
	void moveBounce();
	void moveSlow();
	void moveSlam();
	void checkBumps();
	int isRollDone();
	int isPathDone();
	int isSlamDone();
	int hasLanded();
	int findPath(void** path, void* point);
	int isPastHolder();
	int hitWall();
	void step();
	void stepSlow();
	int isGrounded();

	EggBody* mBody;  // _00
	TObjEgg* mOwner; // _04
	u8 _08[4];
	FlagWord mGround;  // _0C
	u32 mMask;         // _10
	Vec3f _14;         // _14
	EggCollide* mPath; // _20 the owner's
	f32 mPathSpeed;    // _24
	void* mHitCb;      // _28
	void* mHitArg;     // _2C
};

struct EggGlow { // size 0x8
	EggGlow();
	~EggGlow();

	void stop();
	void startGrow(void* mtx);
	void startFull(void* mtx);
	void update();

	u16 mTimer; // _00
	u8 _02[2];
	Fx* mFx; // _04
};

// A squash and stretch when something bumps the egg
struct EggTilt { // size 0x10
	EggTilt();

	void wobble();
	void update();
	void setState(int state);
	void runState(int state, int phase);
	void stateWobble(int phase);
	int getScale(void* mtx);

	s8 mState; // _00 0 = wobbling 1 = still
	u8 _01;
	u16 mTimer; // _02
	f32 mXZ;    // _04
	f32 mY;     // _08
	f32 mVel;   // _0C
};

// The glow around an egg a looping particle that follows the body and every eighth frame a ring effect at its feet
// See EggAura.cpp
struct EggAura {          // size 0x10
	EggAura(void* owner); // 0x801CFF0C
	~EggAura();           // 0x801CFF24

	void start();  // 0x801CFF7C
	void stop();   // 0x801CFFCC
	void update(); // 0x801CFFF0

	TObject* mOwner;   // _00
	EggBody* mBody;    // _04
	EggParticle* mGen; // _08
	s16 mFrame;        // _0C
	u8 _0E[2];
};

/**
 * @size 0x3B0
 */
class TObjEgg : public TObjAction {
public:
	TObjEgg(TObject* parent, const SetDesc* desc);
	virtual ~TObjEgg();

	inline void* operator new(unsigned long size, void* pool);
	inline void operator delete(void* mem);

	virtual void draw();                                              // _0C the per-frame update
	virtual void vfn_10();                                            // _10
	virtual Vec3f* getPos();                                          // _20
	virtual int getRotY();                                            // _28
	virtual int getKind();                                            // _2C
	virtual void initFromDesc(SetDesc* desc);                         // _30
	virtual void debugPrint(void* arg);                               // _34
	virtual int sendToHandle(ObjMsg* msg);                            // _4C
	virtual f32 getRadiusV();                                         // _54
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58
	virtual void setPos(const Vec3f* pos);                            // _6C

	void setup();
	int onMessage(ObjMsg* msg);
	f32 getRadius();
	int checkHit(EggHits* hits, int index);
	void setDamageMul(s8 mul);
	void setDamageAdd(s8 add);
	void shatterSilently();
	void prepareContent();
	void setForm(int form);
	void spawnContent(int firstTime);
	void die();
	void shatter();
	void updateMatrix();
	int checkHatchTouch();
	int checkThrown();
	void limitVel(f32 limit);
	void setRadius(f32 radius);
	void playSe(int id);
	int takeBy(void* msg);
	void clearHolder();
	void onSizeChange(int size);
	void applySize();
	void render(int alt);
	int isHolder(ObjMsg* msg);
	void runState(int state, int phase);
	void setState(int state);
	int isHandleBusy();
	void getGrowScale(void* mtx);
	void updateWobble();
	void formFollow();
	void formFollowTop();
	void startEffect(int id);
	void enterFormA();
	void enterFormB();
	void enterFormC();
	void enterFormD();
	void enterFormE();
	void enterFormF();
	void enterFormG();
	void enterFormH();

	void stateIdle(int phase);
	void stateHeld(int phase);
	void stateSlam(int phase);
	void stateHatch(int phase);
	void stateRolled(int phase);
	void stateThrown(int phase);
	void stateBounce(int phase);
	void stateSlow(int phase);
	void stateExpire(int phase);
	void stateAppear(int phase);
	void stateFall(int phase);
	void stateBounceUp(int phase);
	void stateRolledP(int phase);
	void stateRolledP2(int phase);

	int msgPush(ObjMsg* msg);
	int msgHit(ObjMsg* msg);
	int msgFeed(ObjMsg* msg);
	int msgRoll(ObjMsg* msg);
	int msgFeedMax(ObjMsg* msg);
	int msgGetLevel(ObjMsg* msg);
	int msgAbilities(ObjMsg* msg);
	int msgGetFlags(ObjMsg* msg);
	int msgGetVel(ObjMsg* msg);
	int msgRollBy(ObjMsg* msg);
	int msgStop(ObjMsg* msg);
	int msgTake(ObjMsg* msg);
	int msgRelease(ObjMsg* msg);
	int msgThrow(ObjMsg* msg);
	int msgRollByPlayer(ObjMsg* msg);
	int msgTakeIdle(ObjMsg* msg);
	int msgSlam(ObjMsg* msg);

	u8 _30;             // _30
	u8 mSavedPoints;    // _31 what msgFeedMax restores when mFeedTimer runs out
	s8 mHits;           // _32 0 to 2 A hit has a 30% chance of breaking it and a third hit always does
	s8 mState;          // _33 see EggState
	s8 mContentKind;    // _34 0 = aniSpawn 1 = spawnItem
	s8 mContentId;      // _35 -1 = none
	s8 mDamageMul;      // _36
	s8 mDamageAdd;      // _37
	FlagByte mEggFlags; // _38
	u8 _39[3];
	u16 mOwnerId;              // _3C 0xFFFF = none
	u16 mHitTimer;             // _3E
	u16 mTick;                 // _40
	u16 mFeedTimer;            // _42
	u16 mTimer;                // _44
	u16 mPulse;                // _46
	Vec3f mHome;               // _48
	f32 mWobble;               // _54
	f32 mWobbleVal;            // _58
	f32 mWobbleVel;            // _5C
	SetLinkMember mLink;       // _60
	EggLevel mLevel;           // _6C
	CharaColli* mColl;         // _74
	f32 _78;                   // _78
	DrawTask* mDrawObj;        // _7C
	DrawTask* mAuraDrawObj;    // _80
	u32 _84;                   // _84
	ObjShadow mShadow;         // _88
	u16 _C0;                   // _C0
	u16 _C2;                   // _C2
	void* mEggParam;           // _C4
	void* mPlayerParam;        // _C8
	EggBody mBody;             // _CC
	EggCollide mCollide;       // _1E4
	EggGrow mGrow;             // _200
	f32 mMatrix[12];           // _20C
	f32 mRotMatrix[12];        // _23C
	f32 mModelMatrix[12];      // _26C
	f32 mFxMatrix[12];         // _29C
	Vec3f mDrawPos;            // _2CC
	EggHandle mHandle;         // _2D8
	EggRoll mRoll;             // _2EC
	MotionPlayer* mHatchModel; // _31C made while it hatches
	u8 _320[0x10];
	EggGlow mGlow;       // _330
	EggTilt mTilt;       // _338
	void* mThrowTag;     // _348
	void* mTexture;      // _34C
	int mForm;           // _350
	Fx* mFormFx;         // _354
	Fx* mPlayerFx;       // _358
	PlyInput mPush;      // _35C
	int mPushAngle;      // _374
	EggAura mAuraA;      // _378
	EggAura mAuraB;      // _388
	ItemHolder mContent; // _398
};

class TArc;

/** What eggHatchCallback keeps while the hatch model is walked a node count and the matrix at node 1 */
struct EggHatchWalk {
	int mCount;   // _00
	f32 mMtx[12]; // _04
};

extern "C" {
int eggContentA(int);
int eggContentB(int);
void eggDrawAlt(TObjEgg*);
void eggDrawAura(TObjEgg*);
void eggDrawNormal(TObjEgg*);
void eggHatchCallback(void*, EggHatchWalk*);
void eggSetTexMtx(int);
void eggTevAlphaOff(int);
void eggTevAlphaOn(int);
void eggTevCrack(int);
void eggTexAlphaOff(u8);
void eggTexAlphaOn(u8);
extern TArc* gEggArc;
extern int gEggAssetsLoaded;
void loadEggAssets();
extern NJS_OBJECT** sEggHatchModels;
extern NJS_MOTION** sEggHatchMotionData;
extern NJS_TEXLIST** sEggTexLists;
void shatterEgg(TObjEgg*);
void unloadEggAssets();
}

extern "C" {
TObject* createEgg(TObject*, SetDesc*);
extern const char* gTObjEggName;
extern int sEggItemIdsLow[41];
}

#endif
