#ifndef _TENEBOSS03DINO_H
#define _TENEBOSS03DINO_H
struct DrawTask;

#include "enemy/ObjEnemy.h"
#include "egg/ObjEgg.h"
#include "enemy/boss/TEneBoss03Model.h"

struct EnemyFx;

// TEneBoss03Dino the red boss's dinosaur

/**
 * @size 0x260
 */
class TEneBoss03Dino : public TObjEnemy {
public:
	enum {
		ACT_RISE       = 0,
		ACT_IDLE       = 1,
		ACT_WALK       = 2,
		ACT_FACE_IN    = 3,
		ACT_FACE_SIDE  = 4,
		ACT_ROAR       = 5,
		ACT_BREATH     = 6,
		ACT_BREATH_END = 7,
		ACT_SWIPE      = 8,
		ACT_BITE       = 9,
		ACT_DEAD       = 10,
	};

	enum {
		NODE_FOOT_R = 1,
		NODE_HIP    = 2,
		NODE_FOOT_L = 3,
		NODE_HEAD   = 8,
		NODE_NECK   = 11,
		NODE_MOUTH  = 12,
	};

	TEneBoss03Dino(TObject* parent, void* owner);
	virtual ~TEneBoss03Dino();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void vfn_10();
	virtual void render();
	virtual f32 vfn_40();
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void vfn_6C();
	virtual void updateBody();
	virtual void setPos(const Vec3f* pos);
	virtual void syncCollider();
	virtual void onActionPhase(int action, int phase);
	virtual int getHitKind(const void* touch, HitInfo* info);

	void setupCollider();
	void colliderOn();
	void colliderNoTouch();
	void colliderOff();
	void showFx(int slot, int node, int kind);
	void updateFx();
	void killFx();
	void puffAtFeet();
	void getHeadPos(Vec3f* out) const;
	void getAim(Vec3f* pos, int* rot);
	void setRingRadius(f32 radius);
	void getNode0Pos(Vec3f* out);
	void getMouthPos(Vec3f* out) const;
	void setMotionRate(f32 rate);
	int isNeckDown();
	int atRing();
	int isFaceInDone();
	int isFaceSideDone();
	int isRoarDone();
	int roarPeaked();
	int isBreathing();
	int isBreathEndDone();
	int isSwipeDone();
	int swipeHits();
	int biteHits();
	int isBiteDone();
	int angleToCentre();
	int angleSideways();
	void findNodes();
	void pose();
	void poseAt(void* motion, f32 frame);
	void placeNodes();
	void stomp(int node);
	void quake();
	void bigQuake();
	void turn(int delta, int final);
	void applyMatrix();

	void actRise(int phase);
	void actIdle(int phase);
	void actWalk(int phase);
	void actFaceIn(int phase);
	void actFaceSide(int phase);
	void actRoar(int phase);
	void actBreath(int phase);
	void actBreathEnd(int phase);
	void actSwipe(int phase);
	void actBite(int phase);
	void actDead(int phase);

	void* mOwner;          // _158
	u8 _15C[4];            // _15C
	DrawTask* mDrawTask;   // _160
	MotionLinkHooks mPose; // _164
	f32 mRingRadius;       // _16C
	Vec3f mLastStomp;      // _170
	f32 mMotionRate;       // _17C
	u32 mStateFlags;       // _180 1 on the ring 2 breath out 4 roared 8 swiping 0x10 biting 0x20 no effects
	HitVec mNodes[13];     // _184
	HitVec mDirs[2];       // _220
	EnemyFx* mFx[4];       // _238
	int mStep;             // _248
	int mTimer;            // _24C
	int mTurn;             // _250
	int mTurnTo;           // _254
	f32 mStepSpeed;        // _258
	f32 mDrift;            // _25C
};

typedef char TEneBoss03Dino_size_check[(sizeof(TEneBoss03Dino) == 0x260) ? 1 : -1];

extern "C" {
extern const char* gTEneBoss03DinoName;
}

#endif
