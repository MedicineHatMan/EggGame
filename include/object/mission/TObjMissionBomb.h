#ifndef _OBJECT_MISSION_TOBJMISSIONBOMB_H_
#define _OBJECT_MISSION_TOBJMISSIONBOMB_H_
struct TArc;
class TParticleGenerator;
#include "Ninja/ninja.h"
struct DrawTask;
#include "egg/ObjEgg.h"
#include "engine/CharaColli.h"
#include "io/Resource.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct BombHandle;
struct BombHoldMsg;
struct BombRope;
struct BombRopeNode;
struct BombThrowMsg;
struct SetDesc;
struct TObjMissionBomb;
struct TObject;
struct Vec3f;

#include "core/types.h"

extern "C" {
void bombUnused1();
void bombUnused2();
void initMissionBomb();
void termMissionBomb();
}

extern "C" {
TObjMissionBomb* spawnTObjMissionBomb(u16, const Vec3f*);
TObject* createTObjMissionBomb(TObject*, SetDesc*);
}

extern "C" {
int loadTObjMissionBomb();
}

/** The holder's side of the bomb */
struct BombHandle : public EggHandleBase {
	inline BombHandle() { }
	virtual ~BombHandle() { }           // _08 0x801084C0
	virtual int onMessage(ObjMsg* msg); // _0C

	u32 _10; // _10
};

struct BombRopeNode { // size 0x24
	BombRopeNode() { }

	Vec3f mPos;  // _00
	Vec3f mPrev; // _0C
	Vec3f mVel;  // _18
};

struct BombRope { // size 0x124
	BombRope();

	BombRopeNode mNodes[8]; // _00
	int _120;               // _120
};

/** Sent to a bomb when something picks it up and filled in by the bomb */
struct BombHoldMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	void* mHandle; // _08
	Vec3f mPos;    // _0C
	EggRot mRot;   // _18
	f32 mRadius;   // _28
};

struct BombThrowMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	Vec3f mVel; // _08
};

class TObjMissionBomb : public TObjAction {
public:
	TObjMissionBomb(TObject* parent, u16 id);
	virtual ~TObjMissionBomb();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);
	virtual f32 getRadiusV();
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual SetLink* getLink(); // _6C

	EggRoll* roll() { return &mRoll; }

	void loadModels(TArc* arc);
	void start();
	void setPos(const Vec3f* pos);
	void updateRope(f32 groundY);
	int handleMessage(ObjMsg* msg);
	void runState(int state, int phase);
	void setState(int state);
	int isHolder(BombThrowMsg* msg);
	int getMsgFlags();
	int attachTo(BombHoldMsg* msg);
	void release();
	void updateMatrix();
	int checkDamage();
	void stateHeld(int phase);
	void stateRolling(int phase);
	void stateSlam(int phase);
	void stateGround(int phase);
	void stateExplode(int phase);
	int onPush(ObjMsg* msg);
	int onGetType();
	int onGetFlags();
	int onGetVel(ObjMsg* msg);
	int onThrow(ObjMsg* msg);
	int onPickUp(ObjMsg* msg);
	int onDrop(ObjMsg* msg);
	int onSlam(ObjMsg* msg);

	u16 mOwner;                // _30
	FlagByte mFlagBits;        // _32
	s8 mState;                 // _33
	s16 mTimer;                // _34
	s16 mSwitch;               // _36 -1 = none
	u32 mTimeLeft;             // _38 milliseconds
	u32 mLimit;                // _3C
	CharaColli* mColli;        // _40
	DrawTask* mDrawTask;       // _44
	TParticleGenerator* mFuse; // _48 the spark effect
	EggBody mBody;             // _4C
	SetLinkMember mLink;       // _164
	f32 mMatrix[12];           // _170
	BombHandle mHandle;        // _1A0
	Vec3f mPos;                // _1B4
	// An EggRoll without the fields only TObjEgg uses the next member here starts 0x30 in
	EggRoll mRoll;     // _1C0
	ObjShadow mShadow; // _1F0
	BombRope mRope;    // _228
	int mRopeSeg;      // _34C where the spark is
	f32 mRopeFrac;     // _350
	void** mModels;    // _354
	void** mTexLists;  // _358
	ItemHolder mItem;  // _35C
};

extern "C" {
extern const char* gTObjMissionBombName;
extern TArc* sMissionBombArc;
}

#endif
