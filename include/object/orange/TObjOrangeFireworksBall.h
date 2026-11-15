#ifndef _TOBJORANGEFIREWORKSBALL_H
#define _TOBJORANGEFIREWORKSBALL_H
struct SetDesc;
struct TObject;
struct CharaColli;

#include "engine/TMobileObject.h"
struct NJS_OBJECT;
struct NJS_TEXLIST;
#include "stage/SetManager.h"
#include "engine/DrawTask.h"

extern "C" void objHeapFree(void*, void*);

/** The holder's side of the ball */
struct FireworksBallHandle : public EggHandleBase {
	inline FireworksBallHandle() { }
	virtual ~FireworksBallHandle() { }         // _08 0x8012F330
	virtual inline int onMessage(ObjMsg* msg); // _0C 0x80130380

	u32 _10; // _10
};

struct FireworksHoldMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	void* mHandle; // _08
	Vec3f mPos;    // _0C
	EggRot mRot;   // _18
	f32 mRadius;   // _28
};

struct FireworksThrowMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	Vec3f mVel; // _08
};

/**
 * A fireworks ball, which rolls and is carried like an egg. Loaded from obj_ms_fwball.arc. Below a height
 * of -40 it sends message 0x9F and deletes itself. An enemy's touch, or a hit with flag 13, sets it off.
 *
 * @size 0x278
 */
class TObjOrangeFireworksBall : public TObjAction {
public:
	enum {
		BALL_GROUND  = 0,
		BALL_HELD    = 1,
		BALL_SLAM    = 2,
		BALL_ROLLING = 3,
		BALL_BURST   = 4,
		BALL_BOUNCE  = 5,
	};

	TObjOrangeFireworksBall(TObject* parent, const SetDesc* desc);
	virtual ~TObjOrangeFireworksBall();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C runs every frame
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual Vec3f* getPos();                  // _20
	virtual int getKind();                    // _2C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual int sendToHandle(ObjMsg* msg);    // _4C
	virtual f32 getRadiusV();                 // _54
	virtual SetLink* getLink();               // _6C

	EggRoll* roll() { return &mRoll; }

	int handleMessage(ObjMsg* msg);
	void runState(int state, int phase);
	void setState(int state);
	int getHolderFlags();
	int isHolder(FireworksThrowMsg* msg);
	int attachTo(FireworksHoldMsg* msg);
	void release();
	void updateMatrices();
	void updateGround();
	int checkTouches();
	void remove();
	void stateHeld(int phase);
	void stateSlam(int phase);
	void stateRolling(int phase);
	void stateBurst(int phase);
	void stateGround(int phase);
	void stateBounce(int phase);
	int onPush(ObjMsg* msg);
	int onKick(ObjMsg* msg);
	int onGetType();
	int onGetFlags();
	int onGetVel(ObjMsg* msg);
	int onThrow(ObjMsg* msg);
	int onPickUp(ObjMsg* msg);
	int onDrop(ObjMsg* msg);
	int onSlam(ObjMsg* msg);

	s8 mState;      // _30
	FlagByte mBusy; // _31 8 once it has burst
	u16 mTimer;     // _32
	u16 mOwner;     // _34
	u8 _36[2];
	CharaColli* mShape;          // _38
	DrawTask* mDrawTask;         // _3C
	EggBody mBody;               // _40
	EggCollide mCollide;         // _158
	SetLinkMember mLink;         // _174
	f32 mPlace[12];              // _180
	f32 mDrawMtx[12];            // _1B0
	FireworksBallHandle mHandle; // _1E0
	Vec3f mPos;                  // _1F4
	u8 _200[0x10];
	// An EggRoll without the fields only TObjEgg uses the next member here starts 0x30 in
	EggRoll mRoll;     // _210
	ObjShadow mShadow; // _240
};

class TArc;

extern "C" {
TArc* getFireworksBallArc();
void loadFireworksBallArc();
void releaseFireworksBallArc();
}

extern "C" {
TObject* createTObjOrangeFireworksBall(TObject*, SetDesc*);
}

extern "C" {
extern NJS_OBJECT** sOrangeFireworksBallModels;
extern NJS_TEXLIST** sOrangeFireworksBallTexLists;
extern TArc* sFireworkBallArc;
}

#endif
