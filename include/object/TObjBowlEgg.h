#ifndef _OBJECT_TOBJBOWLEGG_H_
#define _OBJECT_TOBJBOWLEGG_H_
#include "egg/ObjEgg.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "engine/dlist.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct BowlEggHandle;
struct BowlGrabMsg;
struct BowlThrowMsg;
struct SetDesc;
struct TObjBowlEgg;
struct TObject;

#include "core/types.h"

class TObjAction;
class TObjGameBowling;
struct Vec3f;

extern "C" {
void bowlEggInit();
void bowlEggTerm();
int tickTimer32(int*);
}

extern "C" {
TObject* createTObjBowlEgg(TObject*, SetDesc*);
}

// The bowling egg's handle Its vtable is 0x8025884C
struct BowlEggHandle : public EggHandleBase {
	inline BowlEggHandle() { }
	virtual ~BowlEggHandle() { }               // _08 0x8009E2B0
	virtual inline int onMessage(ObjMsg* msg); // _0C 0x8009F6C0

	u32 _10; // _10
};

struct BowlGrabMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	void* mHandle; // _08
	Vec3f mPos;    // _0C
	EggRot mRot;   // _18
	f32 mRadius;   // _28
};

struct BowlThrowMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	Vec3f mVel; // _08
};

class TObjBowlEgg : public TObjAction {
public:
	TObjBowlEgg(TObject* parent, const SetDesc* desc);
	virtual ~TObjBowlEgg();
	virtual void draw();   // _0C 0x8009E79C
	virtual void vfn_10(); // _10 0x8009E8A8
	virtual void render(); // _1C 0x8009E910

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual Vec3f* getPos();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual int sendToHandle(ObjMsg* msg);
	virtual f32 getRadiusV();
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8009E6FC
	virtual void setPos(const Vec3f* pos);
	virtual SetLink* getLink();

	EggRoll* roll() { return &mRoll; }

	void carryOn(TObjAction* carrier, u16 node); // 0x8009E444 called by the lane
	void launch(Vec3f* vel);                     // 0x8009E50C
	void kill();                                 // 0x8009E548
	int handleMessage(ObjMsg* msg);              // 0x8009E57C
	void runState(int state, int phase);         // 0x8009E9F8
	void setState(int state);                    // 0x8009EA90
	int getMsgFlags();                           // 0x8009EAEC
	int isHolder(BowlThrowMsg* msg);             // 0x8009EB30
	int attachTo(BowlGrabMsg* msg);              // 0x8009EB48
	void release();                              // 0x8009EBBC
	void updateMatrix();                         // 0x8009EBCC
	int tryPath();                               // 0x8009EC0C
	void breakEgg();                             // 0x8009EC68
	void stateHeld(int phase);                   // 0x8009ECB0
	void stateSlam(int phase);                   // 0x8009ED4C
	void stateRolling(int phase);                // 0x8009EE38
	void stateBroken(int phase);                 // 0x8009EF30
	void stateGround(int phase);                 // 0x8009EFD0
	void statePath(int phase);                   // 0x8009F024
	void stateCarried(int phase);                // 0x8009F094
	void stateBounce(int phase);                 // 0x8009F15C
	int onPush(ObjMsg* msg);                     // 0x8009F1D8 0x01
	int onKick(ObjMsg* msg);                     // 0x8009F204 0x2A
	int onGetType();                             // 0x8009F2A4 0x40
	int onGetFlags();                            // 0x8009F2AC 0x41
	int onGetVel(ObjMsg* msg);                   // 0x8009F2EC 0x43
	int onThrow(ObjMsg* msg);                    // 0x8009F30C 0x64
	int onPickUp(ObjMsg* msg);                   // 0x8009F384 0x6A
	int onDrop(ObjMsg* msg);                     // 0x8009F430 0x69
	int onPath(ObjMsg* msg);                     // 0x8009F498 0x6B
	int onSlam(ObjMsg* msg);                     // 0x8009F518 0x71

	u16 mOwner;            // _30 who holds it
	u16 mLane;             // _32 the lane it came from
	u16 mCarrier;          // _34
	u16 mNode;             // _36
	int mTimer;            // _38
	FlagWord mBowlFlags;   // _3C
	CharaColli* mShape;    // _40
	DrawTask* mDraw;       // _44
	EggBody mBody;         // _48
	EggCollide mCollide;   // _160
	SetLinkMember mLink;   // _17C
	int mState;            // _188
	f32 mBodyMtx[12];      // _18C
	f32 mMtx[12];          // _1BC what it is drawn with
	BowlEggHandle mHandle; // _1EC
	Vec3f mPos;            // _200
	EggRot mCarryRot;      // _20C its turn against the carrier's node
	EggRoll mRoll;         // _21C
};

#endif
