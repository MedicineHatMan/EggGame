#ifndef _TOBJGOLDENEGG_H
#define _TOBJGOLDENEGG_H
struct GoldenEggWalk;
struct SetDesc;
struct TObject;
struct Fx;

#include "object/ObjAction.h"
struct NJS_OBJECT;
struct NJS_TEXLIST;
#include "egg/ObjEgg.h"
#include "engine/DrawTask.h"
#include "engine/CharaColli.h"
#include "sound/SoundHandle.h"

class TParticleGenerator;

class TObjGoldenEgg;
extern "C" int goldenEggOnMessage(TObjGoldenEgg* egg, ObjMsg* msg); // 0x800BF568

/** The handle other objects talk to while they hold the golden egg */
struct GoldenEggHandle : public EggHandleBase {
	virtual int onMessage(ObjMsg* msg) { return goldenEggOnMessage((TObjGoldenEgg*)mOwner, msg); } // 0x800C1A64

	u32 _10; // _10
};

/**
 * The golden egg, class 0x1C. See TObjGoldenEgg.cpp. Its states are GoldenEggState.
 *
 * @size 0x314
 */
class TObjGoldenEgg : public TObjAction {
public:
	TObjGoldenEgg(TObject* parent, const SetDesc* desc);
	virtual ~TObjGoldenEgg();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();     // _0C 0x800BF800
	virtual void vfn_10();   // _10 0x800BF9B4 hides the draw task off screen
	virtual void render();   // _1C 0x800BFA1C
	virtual Vec3f* getPos(); // _20 0x800C1A20
	virtual int getKind();   // _2C 0x800C1A18 an egg
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);                            // _4C 0x800BF694
	virtual f32 getRadiusV();                                         // _54 0x800C1A30
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x800BFB9C
	virtual SetLinkMember* getLink();                                 // _6C 0x800C1A28

	void setPos(const Vec3f* pos);       // 0x800BF7D8
	void runState(int state, int phase); // 0x800BFCE4
	void setState(int state);            // 0x800BFD88
	int isHolder(ObjMsg* msg);           // 0x800BFDE8
	int isHandleBusy();                  // 0x800BFE00 always 0
	int takeBy(ObjMsg* msg);             // 0x800BFE08
	void clearHolder();                  // 0x800BFEC8
	void onSizeChange(int size);         // 0x800BFED8
	void applySize();                    // 0x800BFF8C
	void setRadius(f32 radius);          // 0x800BFFC4
	void updateMatrix();                 // 0x800C0038
	void getGrowScale(f32* mtx);         // 0x800C0104
	int checkHatchTouch();               // 0x800C0184
	int checkThrown();                   // 0x800C01F0
	void die();                          // 0x800C024C

	void stateIdle(int phase);   // 0x800C087C
	void stateHeld(int phase);   // 0x800C0298
	void stateSlam(int phase);   // 0x800C0490
	void stateHatch(int phase);  // 0x800C062C
	void stateRolled(int phase); // 0x800C0364
	void stateThrown(int phase); // 0x800C0990
	void stateAsleep(int phase); // 0x800C0A00
	void stateBounce(int phase); // 0x800C0A74
	void stateSlow(int phase);   // 0x800C0B40

	int msgPush(ObjMsg* msg);      // 0x800C0C58 0x01
	int msgFeed(ObjMsg* msg);      // 0x800C0C84 0x18
	int msgRoll(ObjMsg* msg);      // 0x800C0CE4 0x2A
	int msgGetLevel(ObjMsg* msg);  // 0x800C0D78 0x76
	int msgAbilities(ObjMsg* msg); // 0x800C0DB4 0x40
	int msgGetFlags(ObjMsg* msg);  // 0x800C0DBC 0x41
	int msgGetVel(ObjMsg* msg);    // 0x800C0E10 0x43
	int msgWake(ObjMsg* msg);      // 0x800C0E30 0x1002
	int msgRollBy(ObjMsg* msg);    // 0x800C0E88 0x64
	int msgTake(ObjMsg* msg);      // 0x800C0F0C 0x6A
	int msgRelease(ObjMsg* msg);   // 0x800C0FE0 0x69
	int msgThrow(ObjMsg* msg);     // 0x800C1048 0x6B
	int msgSlam(ObjMsg* msg);      // 0x800C10C8 0x71

	FlagByte mEggFlags; // _30 4 = burst 8 = hatching or asleep so nothing breaks it
	s8 mState;          // _31 see GoldenEggState
	s8 mMode;           // _32 0 awake 1 asleep until message 0x1002
	u8 _33;
	u16 mOwnerId;                 // _34 0xFFFF = none
	s16 mTimer;                   // _36
	int mRotY;                    // _38
	CharaColli* mShape;           // _3C
	DrawTask* mDraw;              // _40
	EggBody mBody;                // _44
	EggCollide mCollide;          // _15C
	SetLinkMember mLink;          // _178
	f32 mRotMatrix[12];           // _184 where the particle generator follows
	f32 mMatrix[12];              // _1B4 what render draws with
	f32 mModelMatrix[12];         // _1E4 the glow's
	GoldenEggHandle mHandle;      // _214
	Vec3f mPos;                   // _228 what getPos returns
	EggRoll mRoll;                // _234
	TParticleGenerator* mSparkle; // _264
	ObjShadow mShadow;            // _268
	EggLevel mLevel;              // _2A0
	EggGrow mGrow;                // _2A8
	EggGlow mGlow;                // _2B4
	SoundHandle mSound;           // _2BC the hum while it waits to hatch
	Fx* mFx;                      // _2E8
	MotionPlayer* mHatchModel;    // _2EC made while it hatches
	EggAura mAuraRoll;            // _2F0
	EggAura mAuraSlow;            // _300
	void* mThrowTag;              // _310
};

// The states by runState's switch
enum GoldenEggState {
	GOLDEN_EGG_NONE   = -1,
	GOLDEN_EGG_IDLE   = 0,
	GOLDEN_EGG_HELD   = 1,
	GOLDEN_EGG_SLAM   = 2,
	GOLDEN_EGG_HATCH  = 3,
	GOLDEN_EGG_ROLLED = 4,
	GOLDEN_EGG_THROWN = 5,
	GOLDEN_EGG_ASLEEP = 6, // humming until message 0x1002 wakes it
	GOLDEN_EGG_BOUNCE = 7,
	GOLDEN_EGG_SLOW   = 8,
};

extern "C" {
extern TArc* sGoldenEggArc;                              // 0x802C51D8 egg_gold.arc
extern NJS_OBJECT** sGoldenEggModels;                    // 0x802C51DC its models the egg then the shards TGoldenEggBreak throws
extern NJS_TEXLIST** sGoldenEggTexLists;                 // 0x802C51E0 its texture lists
int goldenEggOnMessage(TObjGoldenEgg* egg, ObjMsg* msg); // 0x800BF568
int goldenEggTick(s16* timer);                           // 0x800C1A38 true on the frame it runs out
}

class TArc;

extern "C" {
int eventCmdLoadGold(void*);
TArc* goldenEggGetArc();
void goldenEggTexNum(int, int, int, int);
void initGoldenEgg();
void termGoldenEgg();
}

extern "C" {
void goldenEggTev(int);
void goldenEggHatchWalk(void*, GoldenEggWalk*);
TObject* createTObjGoldenEgg(TObject*, SetDesc*);
}

extern "C" {
void goldenEggTexGen(u8);
}

extern "C" {
extern const char* gGoldenEggModeNames[2];
}

#endif
