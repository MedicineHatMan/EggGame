#ifndef _ANIMAL_TANIHORSESHOECRABSHARK_H_
#define _ANIMAL_TANIHORSESHOECRABSHARK_H_
#include "Ninja/ninja.h"
#include "animal/AniMount.h"
#include "engine/DrawTask.h"
#include "engine/Fx.h"
#include "engine/MotionPlayer.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "player/PlyInput.h"
#include "stage/Stage.h"
struct TAniHorseshoeCrabShark;

#include "core/types.h"

class TObject;

extern "C" {
void* createHorseshoeCrabShark(TObject*);
}

/**
 * @size 0x4D4
 */
class TAniHorseshoeCrabShark : public TAniMount {
public:
	TAniHorseshoeCrabShark(TObject* parent);
	virtual ~TAniHorseshoeCrabShark();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void vfn_10();
	virtual Vec3f* getPos();
	virtual int getRotY();
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void vfn_94(f32* out);
	virtual int canMount() const;
	virtual int canDismount();
	virtual void onMount();
	virtual void onDismount();
	virtual void onLoaded();
	virtual void runState(int state, int phase);
	virtual void preUpdate();
	virtual void postUpdate();

	static void drawTask(TAniHorseshoeCrabShark* self);

	void updateMatrix();
	void updateShadow();
	void feed(const PlyInput* input, AniMove* move);
	void sound(int id);
	void bump(const Vec3f* pos);
	void land();
	int inWater();
	void startDiveFx();
	void useMove(AniMove* move);

	void runBirth(int phase);
	void runIdle(int phase);
	void runStand(int phase);
	void runWalk(int phase);
	void runDive(int phase);
	void runSurface(int phase);
	void runJump(int phase);
	void runFall(int phase);
	void runSlam(int phase);
	void runStun(int phase);
	void runHurt(int phase);
	void runDead(int phase);

	FlagByte mFlags2;           // _9C a FlagByte 1 = its death is on the way
	u8 _9D;
	s16 mFxBank;          // _9E
	s16 mClock;           // _A0 counts frames Nothing here reads it
	s16 mWait;            // _A2
	s8 mDiveFxOff;        // _A4
	s8 mDives;            // _A5
	u8 _A6[2];
	int mRotY;            // _A8
	int mBob;             // _AC the bob's angle up 0x200 a frame
	Vec3f mPos;           // _B0
	f32 mMatrix[12];      // _BC
	f32 mWakeMatrix[12];  // _EC at mPos raised by 2 turned by mRotY
	MotionPlayer mMotion; // _11C
	u8 _158[4];
	DrawTask* mDrawTask;      // _15C
	AniHits mHits;        // _160
	AniBodyHit mBodyHit;  // _178
	AniReachHit mDiveHit; // _1B0
	AniBody mBody;        // _1EC
	AniModel mModel; // _38C
	AniMotions mMotions; // _3A8
	AniLife mLife;        // _3B8
	AniBlink mBlink;      // _3C4
	ObjShadow mShadow;    // _3DC
	AniWalk mWalk; // _414
	AniJump mJump; // _448
	AniDive mDive; // _474
	AniSlam mSlam; // _49C
	AniMove* mMove;          // _4BC
	Fx* mFx;            // _4C0
	Fx* mFx2;           // _4C4
	TObject* mShotParent; // _4C8 the shock waves hang from this
	TAniBirth* mBirth;    // _4CC
	int mHitValue;        // _4D0
};

extern "C" {
extern const char* gTAniHorseshoeCrabSharkName;
extern AniBodyOffset sHorseshoeBodyOffset;
extern const AniNode sHorseshoeCrabSharkModelNodes[2];
extern const MotionEntry sHorseshoeCrabSharkMotions[];
}

#endif
