#ifndef _TOBJEGGRING_H
#define _TOBJEGGRING_H
struct SetDesc;
struct TObject;
struct NJS_OBJECT;
struct Fx;
struct CharaColli;

#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "sound/SoundHandle.h"
#include "engine/RingDraw.h"

// TObjEggRing class 0x2D and what it holds one of six ring kinds chosen by the set record's first int

class TObjEggRing;

extern "C" void mtxCopy(f32* out, const f32* in);

/** A message that carries a vector 0x22 and 0x23 throw the holder 0x43 asks for its hand */
struct RingMsgVec : public ObjMsg {
	Vec3f mVec; // _04
};

/** Message 0x16 sent to whoever touches a ring to ask them to take hold of it */
struct RingMsgGrab : public ObjMsg {
	u8 _02[2];
	u16 mFrom; // _04
	u16 _06;   // _06
};

enum EggRingKind {
	EGGRING_TRANSFER = 0,
	EGGRING_SPIN     = 1,
	EGGRING_SPRING   = 2,
	EGGRING_ROCKET   = 3,
	EGGRING_TRAPEZE  = 4,
	EGGRING_SWING    = 5,
};

enum EggRingState {
	EGGRING_IDLE    = 0, // waiting for someone to touch it
	EGGRING_HELD    = 1,
	EGGRING_RELEASE = 2, // let go going back home
};

/**
 * What a ring holds. Its data comes first, so the vtable is at _1AC.
 *
 * @size 0x1B0
 */
class EggRingContent {
public:
	EggRingContent(TObjEggRing* owner); // 0x801371E8

	static void init();                                         // 0x80137068
	static void exit();                                         // 0x80137074
	static int getHeldCount();                                  // 0x80137080 read by TObjChickenBoss
	static EggRingContent* create(TObjEggRing* owner, int kind); // 0x80137088

	int hasHolder();                                 // 0x8013738C
	int launch(f32 x, f32 y, f32 z, int playSe);     // 0x801373E0 message 0x22
	int launchCounted(f32 x, f32 y, f32 z, int playSe); // 0x801374E0 message 0x23
	void initColli();                                // 0x8013762C
	void placeColli();                               // 0x801376BC
	void freeColli();                                // 0x801376F8
	void touch();                                    // 0x80137730
	void update();                                   // 0x80137B3C
	void spawnFx(int id, f32* mtx);                  // 0x80138048
	void startFx(int id, f32* mtx);                  // 0x80138078
	void stopFx();                                   // 0x801380C4
	void checkVisibleAt(f32 radius, Vec3f* pos);     // 0x80138104

	int getKind() { return mKind; }
	f32 getSize() { return 2.0f * RingDraw::getWidth() + mRing.mRadius; }

	int mDirty;          // _00 recalc before the next draw
	int mKind;           // _04 EggRingKind
	RingDraw mRing;      // _08
	CharaColli* mColli;        // _18
	TObjEggRing* mOwner; // _1C
	ObjRef mHolder;      // _20
	Vec3f mHome;         // _2C
	SetRot mHomeRot;     // _38
	Vec3f mPos;          // _44
	SetRot mRot;         // _50
	f32 mRadius;         // _5C the hoop's radius shrinks to the holder
	Vec3f mShadowPos;    // _60
	u8 _6C;              // _6C
	f32 mMtx[12];        // _70
	f32 mShadowMtx[12];  // _A0
	f32 mGlowMtx[12];    // _D0
	f32 mPower;          // _100 how hard it throws
	u16 mPhase;          // _104
	Fx* mFx;           // _108
	f32 mScaledMtx[12];  // _10C
	f32 mFxMtx[12];      // _13C
	void* mModels[3];    // _16C one per lod
	void* mTex;          // _178
	DrawState mDrawState;    // _17C
	int mDescFlag;       // _1A0
	u32 mFlags;          // _1A4 1 = counted as held 2 = visible 4 = shadow visible 8 = no ground
	int mState;          // _1A8 EggRingState

	// _1AC = vtable
	virtual void setState(int state);      // _08
	virtual void idle();                   // _0C
	virtual void hold();                   // _10
	virtual void release();                // _14
	virtual void calc();                   // _18
	virtual void calcMatrix();             // _1C
	virtual int calcShadow();              // _20
	virtual void calcGlow();               // _24
	virtual void onCatch(const Vec3f* hand); // _28
	virtual ~EggRingContent();             // _2C
	virtual void readDesc(SetDescObj* desc); // _30
	virtual void debugPrint();             // _34
	virtual int onMessage(ObjMsg* msg);    // _38
	virtual void getMatrix(int node, f32* out); // _3C
	virtual void getPos(Vec3f* out) const { *out = mPos; }    // _40
	virtual void getHome(Vec3f* out) const { *out = mHome; }  // _44
	virtual void getEnd(Vec3f* out) const { *out = mHome; }   // _48
	virtual void getRot(SetRot* out) const { *out = mRot; }   // _4C
	virtual f32 getRadius() { return mRadius; } // _50
	virtual void getRingMatrix(f32* out) { mtxCopy(out, mMtx); } // _54
	virtual void checkVisible();           // _58
	virtual void draw();                   // _5C
	virtual void drawShadow();             // _60
	virtual void drawDebug();              // _64
};

/** Moves to and fro between its home and an end point. @size 0x264 */
class EggRingTransfer : public EggRingContent {
public:
	EggRingTransfer(TObjEggRing* owner);

	int move(); // 0x80138840

	virtual void setState(int state);
	virtual void idle();
	virtual void hold();
	virtual void release();
	virtual void calc();
	virtual void onCatch(const Vec3f* hand);
	virtual ~EggRingTransfer();
	virtual void readDesc(SetDescObj* desc);
	virtual void debugPrint();
	virtual int onMessage(ObjMsg* msg);
	virtual void getEnd(Vec3f* out) const { *out = mEnd; }
	virtual void checkVisible();
	virtual void draw();

	int mTag;           // _1B0 0 = it moves by itself
	u16 mSpeed;         // _1B4
	Vec3f mOffset;      // _1B8
	Vec3f mEnd;         // _1C4
	int mDir;           // _1D0
	f32 mBlade[3][12];  // _1D4
};

/** Spins about its axis while held. @size 0x1F0 */
class EggRingSpin : public EggRingContent {
public:
	EggRingSpin(TObjEggRing* owner);

	int spin(); // 0x80138E30

	virtual void idle();
	virtual void hold();
	virtual void release();
	virtual void onCatch(const Vec3f* hand);
	virtual ~EggRingSpin();
	virtual void readDesc(SetDescObj* desc);
	virtual void debugPrint();
	virtual int onMessage(ObjMsg* msg);
	virtual void draw();
	virtual void drawDebug() { }

	NJS_OBJECT* mPole;       // _1B0
	NJS_OBJECT* mTop;        // _1B4
	void* mPoleTex;    // _1B8
	void* mTopTex;     // _1BC
	u16 mSpinSpeed;    // _1C0
	SoundHandle mSound; // _1C4
};

/** Hangs from four ropes and bounces. @size 0x280 */
class EggRingSpring : public EggRingContent {
public:
	EggRingSpring(TObjEggRing* owner);

	int bounce(); // 0x801394CC

	virtual void idle();
	virtual void hold();
	virtual void release();
	virtual void calc();
	virtual void onCatch(const Vec3f* hand);
	virtual ~EggRingSpring();
	virtual void readDesc(SetDescObj* desc);
	virtual void debugPrint();
	virtual int onMessage(ObjMsg* msg);
	virtual void checkVisible();
	virtual void draw();
	virtual void drawDebug() { }

	Vec3f mVel;        // _1B0
	f32 mStretch;      // _1BC
	f32 mRope[4][12];  // _1C0
};

/** Flies off along a line with two jets lit. @size 0x238 */
class EggRingRocket : public EggRingContent {
public:
	EggRingRocket(TObjEggRing* owner);

	int move(int back); // 0x80139BFC

	virtual void setState(int state);
	virtual void idle();
	virtual void hold();
	virtual void release();
	virtual void calc();
	virtual void onCatch(const Vec3f* hand);
	virtual ~EggRingRocket();
	virtual void readDesc(SetDescObj* desc);
	virtual void debugPrint();
	virtual int onMessage(ObjMsg* msg);
	virtual void getEnd(Vec3f* out) const { *out = mEnd; }
	virtual void checkVisible();
	virtual void draw();

	u16 mSpeed;        // _1B0
	Vec3f mOffset;     // _1B4
	Vec3f mEnd;        // _1C0
	int mDir;          // _1CC
	f32 mJet[2][12];   // _1D0
	void* mFlame[2];   // _230
};

/** Swings to and fro under a bar. @size 0x1F4 */
class EggRingTrapeze : public EggRingContent {
public:
	EggRingTrapeze(TObjEggRing* owner);

	int swing(); // 0x8013A2E8

	virtual void idle();
	virtual void hold();
	virtual void release();
	virtual void calc();
	virtual void onCatch(const Vec3f* hand);
	virtual ~EggRingTrapeze();
	virtual void readDesc(SetDescObj* desc);
	virtual void debugPrint();
	virtual int onMessage(ObjMsg* msg);
	virtual void checkVisible();
	virtual void draw();
	virtual void drawDebug() { }

	u16 mSpeed;       // _1B0
	Vec3f mPivot;     // _1B4
	f32 mMaxAngle;    // _1C0
	f32 mBarMtx[12];  // _1C4
};

/** Swings all the way round its pivot. @size 0x1F0 */
class EggRingSwing : public EggRingContent {
public:
	EggRingSwing(TObjEggRing* owner);

	int swing(); // 0x8013A954

	virtual void idle();
	virtual void hold();
	virtual void release();
	virtual void calc();
	virtual void onCatch(const Vec3f* hand);
	virtual ~EggRingSwing();
	virtual void readDesc(SetDescObj* desc);
	virtual void debugPrint();
	virtual int onMessage(ObjMsg* msg);
	virtual void checkVisible();
	virtual void draw();
	virtual void drawDebug() { }

	u16 mSpeed;       // _1B0
	Vec3f mPivot;     // _1B4
	f32 mBarMtx[12];  // _1C0
};

/** @size 0x98 */
class TObjEggRing : public TSetActor {
public:
	TObjEggRing(TObject* parent, SetDesc* desc);
	virtual ~TObjEggRing();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                            // _0C
	virtual void vfn_10();                          // _10
	virtual void render();                          // _1C
	virtual void getNodeMatrix(int node, f32* mtx); // _24
	virtual void initFromDesc(SetDesc* desc);       // _30
	virtual void debugPrint(void*);                 // _34
	virtual int sendToHandle(ObjMsg* msg);          // _4C
	virtual void findLight();                       // _74

	void drawShadow(); // 0x8013B1D8

	DrawTask* mDraw;  // _64
	DrawTask* mGlow;  // _68 the shadow's
	ObjModel mObj;    // _6C
	int mLightSet;    // _8C from lightSetFindArea
	EggRingContent* mHeld; // _90
	int mKind;        // _94 EggRingKind
};

extern "C" {
void eggRingExit();
void eggRingInit();
}

extern "C" {
TObject* createTObjEggRing(TObject*, SetDesc*);
}

extern "C" {
extern const char* gEggRingKindNames[6];
}

#endif
