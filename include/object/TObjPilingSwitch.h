#ifndef _TOBJPILINGSWITCH_H
#define _TOBJPILINGSWITCH_H
struct SetDesc;
struct TObject;
struct MeshObject;
struct DrawTask;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "enemy/EnemyLight.h"

/**
 * A post that a stomp drives into the ground.
 *
 * @size 0x1A4
 */
class TObjPilingSwitch : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjPilingSwitch(TObject* parent, SetDesc* desc);
	virtual ~TObjPilingSwitch();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void updateCylinder();
	void updateMatrices();
	void updateShapePos();
	void updateColli();
	void stateWait();
	void stateIn();
	void stateDone();

	MeshObject* mPileColli;  // _64
	MeshObject* mBaseColli;  // _68
	ObjModel mPile;    // _6C
	ObjModel mBase;    // _8C
	ObjDrawState mState;  // _AC
	f32 mPileMtx[12];  // _D0
	f32 mBaseMtx[12];  // _100
	DrawTask* mDrawTask;   // _130
	u16 mSwitchId;     // _134
	u16 mEventId;      // _136
	u16 mAct;          // _138 0 = wait 1 = driven in 2 = done
	Vec3f mPileOffset; // _13C
	s16 mPause;        // _148
	EnemyLight mLight; // _14C
	Vec3f mCenter;     // _18C
	f32 mRadius;       // _198
	int mInMirror;     // _19C
	int  mLightSet;   // _1A0
};

extern "C" {
void pilingSwitchSetExit();
void pilingSwitchSetInit();
}

extern "C" {
TObject* createTObjPilingSwitch(TObject*, SetDesc*);
}

#include "stage/Stage.h"
extern "C" {
extern const char* gTObjPilingSwitchName;
struct PileDownOffset : public Vec3f {
	PileDownOffset(f32 ax, f32 ay, f32 az)
	{
		x = ax;
		y = ay;
		z = az;
	}
};
extern PileDownOffset gPileDownOffset;
}

#endif
