#ifndef _TOBJLASTBOSSSHOT_H
#define _TOBJLASTBOSSSHOT_H
struct Fx;
struct CharaColli;

#include "engine/TMobileObject.h"
#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "object/ObjClassName.h"
#include "egg/ObjEgg.h"
#include "object/ObjCommon.h"
#include "engine/CharaColli.h"
#include "sound/SoundHandle.h"

// TObjLastBossShot the ball the last boss drops

enum ShotState {
	SHOT_WAIT    = 0,     // just dropped
	SHOT_GROUND  = 2,
	SHOT_HELD    = 3,
	SHOT_SLAM    = 4,
	SHOT_THROWN  = 5,
	SHOT_SLOW    = 7,     // the roll's last stretch
	SHOT_AIM     = 8,     // rising to the player's height
	SHOT_FLY     = 0x101, // flying at the player
	SHOT_BOUNCE  = 0x106,
};

enum ShotPhase {
	SHOT_ENTER  = 0,
	SHOT_EXIT   = 1,
	SHOT_UPDATE = 2,
	SHOT_CHECK  = 3,
};

/** The holder's side of the shot */
struct ShotHandle : public EggHandleBase {
	inline ShotHandle() { }
	virtual ~ShotHandle() { }           // _08 0x8018D98C
	virtual inline int onMessage(ObjMsg* msg); // _0C 0x8018ED34

	u32 _10; // _10
};

/** What picking the shot up sends and the shot fills in */
struct ShotGrabMsg {
	u16 mType;     // _00
	u8 _02[2];
	u16 mHolder;   // _04
	u8 _06[2];
	void* mHandle; // _08
	Vec3f mPos;    // _0C
	EggRot mRot;   // _18
	f32 mRadius;   // _28
};

struct ShotThrowMsg {
	u16 mType;   // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	Vec3f mVel;  // _08
};

struct ShotVecMsg {
	u16 mType;  // _00
	u8 _02[2];
	Vec3f mVec; // _04
};

/**
 * @size 0x24C
 */
class TObjLastBossShot : public TObjAction {
public:
	TObjLastBossShot(TObject* parent, const Vec3f* pos);
	virtual ~TObjLastBossShot();
	virtual void draw();                                        // _0C 0x8018DBD8
	virtual void vfn_10();                                      // _10 0x8018DCA8
	virtual void render();                                      // _1C 0x8018DCAC
	virtual Vec3f* getPos();                                    // _20 0x8018ED24
	virtual int getKind();                                      // _2C 0x8018ED1C
	virtual int sendToHandle(ObjMsg* msg);                      // _4C 0x8018DBB4
	virtual f32 getRadiusV();                                   // _54 0x8018ED2C
	virtual int onHit(const TObjAction* other, int arg, int* result); // _58 0x8018DD20

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	int handleMessage(ObjMsg* msg);          // 0x8018DAC8
	void runState(int state, int phase);     // 0x8018DE68
	void setState(int state);                // 0x8018DF48
	int isHolder(ShotThrowMsg* msg);         // 0x8018DFA4
	int attachTo(ShotGrabMsg* msg);          // 0x8018DFBC
	void release();                          // 0x8018E068
	void updateMatrix();                     // 0x8018E078
	void stateWait(int phase);               // 0x8018E10C
	void stateHeld(int phase);               // 0x8018E178
	void stateSlam(int phase);               // 0x8018E214
	void stateThrown(int phase);             // 0x8018E324
	void stateAim(int phase);                // 0x8018E41C
	void stateGround(int phase);             // 0x8018E53C
	void stateBounce(int phase);             // 0x8018E590
	void stateFly(int phase);                // 0x8018E62C
	void stateSlow(int phase);               // 0x8018E6EC
	int onPush(ObjMsg* msg);                 // 0x8018E7F0 0x01
	int onKick(ObjMsg* msg);                 // 0x8018E81C 0x2A
	int onGetType();                         // 0x8018E8E0 0x40
	int onGetFlags();                        // 0x8018E8E8 0x41
	int onGetVel(ObjMsg* msg);               // 0x8018E918 0x43
	int onThrow(ObjMsg* msg);                // 0x8018E9C0 0x64
	int onStrike(ObjMsg* msg);               // 0x8018EA38 0x65
	int onPickUp(ObjMsg* msg);               // 0x8018EB30 0x6A
	int onDrop(ObjMsg* msg);                 // 0x8018EC10 0x69
	int onSlam(ObjMsg* msg);                 // 0x8018EC78 0x71
	static f32 approach(f32 rate, f32 from, f32 to); // 0x8018ECF0

	EggRoll* roll() { return &mRoll; }

	s16 mState;          // _30
	s16 mTimer;          // _32
	u16 mOwner;          // _34
	u8 _36[2];
	CharaColli* mColli;        // _38
	EggBody mBody;       // _3C
	f32 mMatrix[12];     // _154
	ShotHandle mHandle;  // _184
	Vec3f mPos;          // _198
	int mAngle;          // _1A4 of the flight
	f32 mSpeed;          // _1A8
	EggRoll mRoll;       // _1AC
	SoundHandle mSnd;    // _1DC
	Fx* mFx;           // _208
	DrawTask* mDraw;         // _20C
	ObjShadow mShadow;   // _210
	int mDone;           // _248 set when it hit the player
};

#endif
