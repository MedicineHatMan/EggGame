#ifndef _ANIMOUNT_H
#define _ANIMOUNT_H
struct Fx;
struct ObjShadow;
struct CharaColli;
struct GjActor;

#include "egg/EggCreature.h"
#include "egg/TEggCreature.h"
#include "animal/AniAsset.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "player/PlyInput.h"
#include "sound/SoundHandle.h"
#include "engine/CharaColli.h"
#include "animal/AniRider.h"

class TCamPath;

// The large animals the player rides TAniCamelOstrich TAniLionKangaroo and TAniHorseshoeCrabShark

struct AniBody;
struct AniHitPart;

struct AniHitList;
struct AniLife;
struct AniModel;
struct AniMotions;
struct AniRider;
struct MeshList;
class MeshColli;
struct AniMove;
struct AniWalk;
struct AniJump;
struct AniDash;
struct AniBlast;

extern "C" {
}

// The riders of an AniBody one for each way it probes
enum AniRiderSlot {
	ANIRIDER_FRONT = 0,
	ANIRIDER_BACK  = 1,
	ANIRIDER_RIGHT = 2,
	ANIRIDER_LEFT  = 3,
	ANIRIDER_4     = 4, // not probed by the body itself
	ANIRIDER_FLOOR = 5, // what it stands on
	ANIRIDER_BELOW = 6, // whatever is under it for the shadow
};

/**
 * The mount's physics: it moves the body by its velocity, stops it at walls, and keeps it on the ground. update steps
 * it once a frame. See AniBody.cpp.
 *
 * @size 0x1A0
 */
struct AniBody {
	AniBody();          // 0x801C1110
	virtual ~AniBody(); // 0x801C1258

	void setMask(u32 mask);                                                   // 0x801C12C0
	void update();                                                            // 0x801C12D4
	MeshColli* touching(int slot, u16* tri, Vec3f* pos, Vec3f* normal) const; // 0x801C133C what that rider stands on
	void detach(int slot);                                                    // 0x801C13F0
	void move();                                                              // 0x801C14AC
	int wall(MeshList* list, const Vec3f* from, const Vec3f* dir, f32 radius, Vec3f* push, AniRider* rider); // 0x801C164C
	void probeWalls();                                                                                       // 0x801C17EC
	void probeGround(f32 snap);                                                                              // 0x801C19AC
	void checkFall();                                                                                        // 0x801C1B98
	void followGround();                 // 0x801C1BE8 moves with a moving floor
	void followTurn();                   // 0x801C1C78 turns with a moving floor
	void setRotY(int angle);             // 0x801C1CEC
	void setState(int state);            // 0x801C1D24
	void runState(int state, int phase); // 0x801C1D84
	void runGround(int phase);           // 0x801C1DCC state 0
	void runAir(int phase);              // 0x801C1E48 state 1

	s8 mState;           // _04 0 on the ground 1 in the air 2 none
	u32 mMask;           // _08 the surfaces it can stand on
	u32 mStopMask;       // _0C the surfaces that stop it and set flags
	FlagWord mFlags;     // _10 Bit 0 on the ground 2 stopped 3/4/5/6 by the surface that stopped it
	Vec3f mPos;          // _14
	Quat mQuat;          // _20 which way it faces
	Vec3f mVel;          // _30
	Vec3f mPushOut;      // _3C how far what it touches pushes it this frame
	Vec3f mWallPush;     // _48 how far the walls push it this frame
	AniRider mRiders[7]; // _54 by AniRiderSlot
	f32 mFloorMtx[12];   // _150 the floor's matrix when it landed
	Vec3f mFloorPos;     // _180 where it stands in the floor's space
	Vec3f mFloorDir;     // _18C the way it faces in the floor's space
	f32 mStep;           // _198 how high a step it walks up
	u8 _19C[4];
};

struct AniHitLinks;

// A part's place in its AniHitList
struct AniHitLink {
	inline AniHitLink()
	    : mNext(NULL)
	    , mPrev(NULL)
	    , mList(NULL)
	{
	}

	AniHitLink* mNext;  // _00
	AniHitLink* mPrev;  // _04
	AniHitPart* mPart;  // _08
	AniHitLinks* mList; // _0C null while the part is in no list
};

// The parts of an AniHitList in the order they were added
struct AniHitLinks {
	AniHitLinks(); // 0x801CC6EC

	void add(AniHitLink* link);    // 0x801CC748
	void remove(AniHitLink* link); // 0x801CC788
	void clear();                  // 0x801CC7E8 unlinks every part

	AniHitLink* mTail; // _00
	AniHitLink* mHead; // _04
	int mCount;        // _08
};

/**
 * A hit sphere an animal owns. It sits at the body plus mOffset (mKind 0) or on a model node (mKind 1), and follows
 * it each frame. See AniHitList.cpp and AniHitParts.cpp.
 *
 * @size 0x34
 */
struct AniHitPart {
	AniHitPart();                       // 0x801CC3EC
	virtual ~AniHitPart() { }           // 0x801CC824
	virtual void attach();              // _0C when it joins a list
	virtual void detach();              // _10 when it leaves one
	virtual void hit(AniLife* life);    // _14 hurts what it touches
	virtual void update(AniBody* body); // _18 follows the body

	void getPos(Vec3f* out) const; // 0x801CC44C

	AniHitList* mHits; // _04 the list it is in
	int mKind;         // _08 0 at the body 1 on a model node
	u16 mOwnerId;      // _0C the animal's object id
	AniHitLink mLink;  // _10
	Vec3f mOffset;     // _20 from the body for kind 0
	f32 mRadius;       // _2C
	s16 mNode;         // _30 the model node for kind 1
};

// The sphere around the body
struct AniBodyHit : public AniHitPart {
	AniBodyHit();                       // 0x801CC884
	virtual ~AniBodyHit();              // 0x801CC8C8
	virtual void attach();              // 0x801CC93C
	virtual void detach();              // 0x801CC9F0
	virtual void hit(AniLife* life);    // 0x801CCAE0
	virtual void update(AniBody* body); // 0x801CCB70

	void pushOut(AniBody* body); // 0x801CCBBC

	CharaColli* mColli; // _34
};

// A sphere that reaches out for an attack
struct AniReachHit : public AniHitPart {
	AniReachHit();                      // 0x801CCC88
	virtual ~AniReachHit();             // 0x801CCCD0
	virtual void attach();              // 0x801CCD44
	virtual void detach();              // 0x801CCE0C
	virtual void hit(AniLife* life);    // 0x801CCE4C
	virtual void update(AniBody* body); // 0x801CCE50

	CharaColli* mColli; // _34
	u16 mActive;        // _38 written to the shape's _2C
	u8 _3A[2];
};

// The hit spheres an animal owns
struct AniHitList { // size 0x18
	AniHitList();   // 0x801CC4E4
	~AniHitList();  // 0x801CC700

	void add(AniHitPart* part);    // 0x801CC524
	void addAt(AniHitPart* part);  // 0x801CC57C and moves it to the body at once
	void remove(AniHitPart* part); // 0x801CC5EC
	void hitAll();                 // 0x801CC644
	void updateAll();              // 0x801CC698

	AniHitLinks mParts; // _00
	AniBody* mBody;     // _0C
	AniModel* mModel;   // _10
	AniLife* mLife;     // _14
};

// The mounts hold their hit list through this
struct AniHits {
	~AniHits() { }

	AniHitList mList; // _00
};

// Where the body hit sits on a mount
struct AniBodyOffset : public Vec3f {
	inline AniBodyOffset(f32 ax, f32 ay, f32 az)
	{
		x = ax;
		y = ay;
		z = az;
	}
};

// A node of a mount's model whose matrix AniModel keeps
struct AniNode {
	s16 mSlot; // _00 the matrix it goes in
	s16 mNode; // _02 the node in the model
};

/**
 * The model of a mount, made from the loaded file by update, posed each frame by pose. It keeps the matrices of the
 * nodes setNodes names, for what hangs off them. See AniModel.cpp.
 *
 * @size 0x1C
 */
struct AniModel {
	AniModel();          // 0x801C0C1C
	virtual ~AniModel(); // 0x801C0C48

	void setNodes(const AniNode* nodes, int count);                        // 0x801C0CC8
	void getMatrix(int slot, f32* out);                                    // 0x801C0DA0
	void update(AniAsset* asset);                                          // 0x801C0DD0 makes the model once the file is in
	void pose(AniAsset* asset, const f32* matrix, MotionPlayer* motion);   // 0x801C0E58
	void* find(const AniAsset* asset);                                     // 0x801C0F58
	void draw(const AniAsset* asset, const f32* matrix, DrawState* state); // 0x801C0F88
	void build(AniAsset* asset);                                           // 0x801C1030

	s8 mState;            // _04 0 waiting for the file 1 made 2 the file went away
	s16* mNodeSlots;      // _08 for each node up to the last named its matrix or -1
	int mNodeCount;       // _0C
	f32 (*mMatrices)[12]; // _10
	int mCount;           // _14
	GjActor* mActor;      // _18
};

/** The mount's motions binds the MotionPlayer to the loaded file */
struct AniMotions { // size 0x10
	AniMotions();   // 0x801C0AEC
	virtual ~AniMotions() { }

	void set(MotionPlayer* player, const MotionEntry* table); // 0x801C0B0C
	void update(AniAsset* asset);                             // 0x801C0B18 starts the player once the file is in
	void* find(const AniAsset* asset);                        // 0x801C0BA0
	void load(AniAsset* asset);                               // 0x801C0BD0

	s8 mState;                 // _04 0 waiting for the file 1 loaded 2 the file went away
	MotionPlayer* mPlayer;     // _08
	const MotionEntry* mTable; // _0C
};

// Hit points at _37C in the camel
struct AniLife {
	AniLife();  // 0x801CD190
	~AniLife(); // 0x801CD1D0

	void damage(int damage); // 0x801CD20C does nothing while bit 0 is set

	int mHp;         // _00
	int mMax;        // _04
	FlagWord mFlags; // _08 1 = just hurt
};

// Flashes the animal after a hit and reads whether it died or was thrown
struct AniBlink {
	AniBlink(); // 0x801CD280
	virtual ~AniBlink() { }

	void update();                    // 0x801CD2D8
	int tint(DrawState* state) const; // 0x801CD3DC true while it blinks
	int isHidden();                   // 0x801CD444 the off frames of the blink
	int getResult();                  // 0x801CD474 0 alive 1 dead 2 thrown

	s8 mResult;      // _04 1 dead 2 thrown
	s16 mBlink;      // _06 frames of blinking left
	int mFlash;      // _08
	FlagWord mFlags; // _0C bit 0 just hurt bit 1 done
	AniLife* mLife;  // _10
	AniBody* mBody;  // _14
};

/** A way a mount moves */
struct AniMove { // size 0x10
	AniMove();   // 0x801C5C9C
	virtual ~AniMove() { }
	virtual void enter() { }         // _0C
	virtual void update(f32 dt) = 0; // _10

	void setBody(AniBody* body);          // 0x801C5CBC
	void setStick(f32 stick, int angle);  // 0x801C5CC4
	int getStick(f32* stick, int* angle); // 0x801C5CD0 nonzero when the stick is pushed
	void step();                          // 0x801C5D08 one frame at 60 a second
	void setRotY(int angle);              // 0x801C5D4C turns the body to face angle
	int getRotY() const;                  // 0x801C5DB0 the way the body faces

	AniBody* mBody;  // _04
	f32 mStick;      // _08 how far the stick is pushed cleared each frame
	int mStickAngle; // _0C the way it is pushed
};

f32 aniLerp(f32 rate, f32 from, f32 to); // 0x801C5E18 moves from toward to by rate a second
int aniTurn(f32 rate, int from, int to); // 0x801C5E44 the same for an angle the short way round

#pragma push
#pragma sym off
/** Walking and running state() reads it 0 still 1 walking 2 jogging 3 running 4 off the ground */
struct AniWalk : public AniMove { // size 0x34
	AniWalk();                    // 0x801C5EDC
	virtual void enter();         // 0x801C5F44
	virtual void update(f32 dt);  // 0x801C5F84

	int state();                          // 0x801C62B8
	void face(int rotY, const Vec3f* up); // 0x801C6314

	s8 mState; // _10
	u8 _11[3];
	int mTilt;      // _14 nonzero to lean with the ground
	f32 mSpeed;     // _18 how fast it goes now
	f32 mRunSpeed;  // _1C top speed with the stick all the way
	f32 mRunFrom;   // _20 how fast it must go before it runs
	f32 mWalkSpeed; // _24 top speed while walking
	int mRotY;      // _28 the way it heads
	f32 mAccel;     // _2C
	f32 mDecel;     // _30
};

/** A jump state() reads it 0 rising 1 falling 2 not yet launched 3 landed */
struct AniJump : public AniMove { // size 0x2C
	AniJump();                    // 0x801CB714
	virtual void enter();         // 0x801CB768
	virtual void update(f32 dt);  // 0x801CB854

	void launch();     // 0x801CB7B0 leaves the ground the way the stick points
	int state() const; // 0x801CBA4C

	s8 mState; // _10 0 standing 2 rising 3 falling 4 landed
	u8 _11[3];
	int mRotY;      // _14 the way it heads
	f32 mGravity;   // _18
	f32 mJumpSpeed; // _1C
	f32 mAirSpeed;  // _20
	u8 _24[4];
	f32 mAirControl; // _28 how hard the stick pushes in the air
};

/** A charge state() reads it 0 going 1 hit a wall 2 left the ground */
struct AniDash : public AniMove { // size 0x24
	AniDash();                    // 0x801CB4A0
	virtual void enter();         // 0x801CB4F4
	virtual void update(f32 dt);  // 0x801CB53C

	int state(); // 0x801CB6D8

	s8 mState; // _10 0 going 1 hit a wall 2 on the ground again
	u8 _11[3];
	int mRotY;  // _14 the way it heads
	f32 mBrake; // _18 its fall speed
	u8 _1C[4];
	f32 mSpeed; // _20
};

/** Being blown along by its own breath */
struct AniBlast : public AniMove { // size 0x28
	AniBlast();                    // 0x801CAF7C
	virtual void enter();          // 0x801CAFD4
	virtual void update(f32 dt);   // 0x801CB024

	int state();                           // 0x801CB20C
	void push(int angle, const Vec3f* up); // 0x801CB254 sets the velocity along the ground

	s8 mState; // _10
	u8 _11[3];
	int mRotY;  // _14 the way it heads
	f32 mPush;  // _18 the speed it heads for
	f32 mSpeed; // _1C how fast it goes now
	f32 mDrag;  // _20 how quickly it gets there
	u8 _24[4];
};
#pragma pop

/** Diving under and coming back up */
struct AniDive : public AniMove { // size 0x28
	AniDive();                    // 0x801CBA98
	virtual ~AniDive() { }
	virtual void enter();        // 0x801CBAF4
	virtual void update(f32 dt); // 0x801CBB6C

	int resume();                                   // 0x801CBB30 starts again while swimming
	int state() const;                              // 0x801CBBB8
	void heading(f32 speed, Vec3f* out, int angle); // 0x801CBC70 along the ground
	void drift(f32 drag, const Vec3f* accel);       // 0x801CBD30
	int slowed();                                   // 0x801CBDC4
	int fellOff();                                  // 0x801CBE34
	int hitWall();                                  // 0x801CBE94
	void setState(int state);                       // 0x801CBF48
	void runState(int state, int phase);            // 0x801CBFA8
	void runStart(int phase);                       // 0x801CC014 state 3
	void runSwim(int phase);                        // 0x801CC0DC state 0
	void runBounce(int phase);                      // 0x801CC304 state 1
	void runStop(int phase);                        // 0x801CC3C0 state 2

	s8 mState; // _10
	u8 _11;
	s16 mTimer;     // _12
	int mRotY;      // _14 the way it heads
	f32 mDrag;      // _18
	f32 mSwim;      // _1C how fast it swims now
	f32 mSpeed;     // _20 faster than this is fast
	f32 mStopSpeed; // _24 slower than this has stopped
};

/** Slamming down state() reads it 0 falling 1 hit the ground */
struct AniSlam : public AniMove { // size 0x20
	AniSlam();                    // 0x801CB2E4
	virtual ~AniSlam() { }
	virtual void enter();        // 0x801CB340
	virtual void update(f32 dt); // 0x801CB374

	int state(); // 0x801CB470

	s8 mState; // _10
	u8 _11[3];
	f32 mMaxFall;  // _14
	f32 mFall;     // _18 how fast it falls now
	f32 mFallRate; // _1C how quickly it gets to mMaxFall
};

#include "animal/TAniMount.h"

// Parts the mount animals make

// The TMorphEggAnim2 a TAniBirth makes
struct AniBirthEgg {
	u8 _00[4];
	u16 mFlags; // _04
	u8 _06[0x1E];
	int mHatched; // _24
	u8 _28[4];
	int mFinished;   // _2C
	DrawState mDraw; // _30
};

// Anything that ends by setting bit 0 at _04 and says it is done at _24
struct AniBirthPart {
	u8 _00[4];
	u16 mFlags; // _04
	u8 _06[0x1E];
	int mDone; // _24
};

/**
 * @size 0x78
 */
class TAniBirth : public TObject {
public:
	TAniBirth(TObject* parent, TObjAction* owner, ItemHolder* holder, AniBody* body);
	virtual ~TAniBirth();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void getDrawState(DrawState* out) const;
	void setState(int state);
	void runState(int state, int phase);
	void runMakeEgg(int phase);
	void runEggEffect(int phase);
	void runWaitOwner(int phase);
	void runEggModel(int phase);
	void runAnimal(int phase);
	void runText(int phase);
	void runEnd(int phase);

	s8 mState;        // _20
	FlagByte mFlags2; // _21 1 = the egg opened 2 = the text started
	s8 mSubState;     // _22
	u8 _23;
	s16 mFxBank;         // _24
	s16 mTextId;         // _26
	int mWait;           // _28
	AniBirthEgg* mEgg;   // _2C
	ItemHolder* mHolder; // _30
	u8 _34[4];
	TCamPath* mModel;        // _38 rides the egg model's camera motion
	AniBirthPart* mTextBack; // _3C
	TObjAction* mOwner;      // _40
	Fx* mFx;                 // _44
	AniBody* mBody;          // _48
	int mHoldsOwner;         // _4C
	Vec3f mOffset;           // _50
	Vec3f mTextAtB;          // _5C
	Vec3f mTextAtA;          // _68
	AniBirthPart* mText;     // _74
};

typedef char TAniBirth_size_check[(sizeof(TAniBirth) == 0x78) ? 1 : -1];

/**
 * @size 0x88
 */
class TAniLionFireBall : public TObject {
public:
	TAniLionFireBall(TObject* parent, u16 ownerId, int fxBank, const Vec3f* pos, const Vec3f* vel);
	virtual ~TAniLionFireBall();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	u16 mOwnerId;       // _20
	u16 mLife;          // _22
	s16 mFxBank;        // _24
	void* mFx;          // _28
	CharaColli* mColli; // _2C
	Vec3f mPos;         // _30
	Vec3f mVel;         // _3C
	f32 mQuat[4];       // _48
	f32 mMatrix[12];    // _58
};

typedef char TAniLionFireBall_size_check[(sizeof(TAniLionFireBall) == 0x88) ? 1 : -1];

/**
 * @size 0x58
 */
class TAniLionFireBreath : public TObject {
public:
	TAniLionFireBreath(TObject* parent, u16 ownerId);
	virtual ~TAniLionFireBreath();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void setMatrix(const f32* matrix);

	u16 mOwnerId; // _20
	u8 _22[2];
	CharaColli* mColli; // _24
	f32 mMatrix[12];    // _28
};

typedef char TAniLionFireBreath_size_check[(sizeof(TAniLionFireBreath) == 0x58) ? 1 : -1];

/**
 * @size 0x40
 */
class TAniSharkShockWave : public TObject {
public:
	TAniSharkShockWave(TObject* parent, u16 ownerId, const Vec3f* pos, f32 radius, f32 finalRadius);
	virtual ~TAniSharkShockWave();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	u16 mOwnerId;     // _20
	s16 mLife;        // _22
	Vec3f mPos;       // _24
	f32 mRadius;      // _30
	f32 mFinalRadius; // _34
	u8 _38[4];
	CharaColli* mColli; // _3C
};

typedef char TAniSharkShockWave_size_check[(sizeof(TAniSharkShockWave) == 0x40) ? 1 : -1];

#endif
