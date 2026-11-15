#ifndef _TOBJORANGEBATTERY_H
#define _TOBJORANGEBATTERY_H
struct SetDesc;
struct TObject;

#include "stage/SetActor.h"
#include "Ninja/ninja.h"
#include "object/ObjModel.h"
#include "egg/ObjEgg.h"
#include "engine/DrawTask.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One of the two lightning bolts. Sixteen points run from mFrom to mTo, jittered in reset. Ours.
 *
 * @size 0x114
 */
class BatteryBolt {
public:
	BatteryBolt();
	~BatteryBolt();

	void aim(const Vec3f* from, const Vec3f* to);
	void reset();
	void draw();

	f32 mMatrix[12];    // _00
	Vec3f mFrom;        // _30
	Vec3f mTo;          // _3C
	f32 mSpread;        // _48
	f32 mWidth;         // _4C shrinks as the bolt gets further from the camera
	u32 mColor;         // _50
	Vec3f mPoints[16];  // _54
};

/**
 * A battery the player strikes to set a switch. Set int 0 is that switch. Act 0 waits for the hit,
 * act 1 plays MOTION_BATTERY and throws the two bolts, act 2 leaves the open model up. A switch that
 * is already set starts in act 2.
 *
 * @size 0x3FC
 */
class TObjOrangeBattery : public TSetActor {
public:
	TObjOrangeBattery(TObject* parent, SetDesc* desc);
	virtual ~TObjOrangeBattery();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void vfn_74();                    // _74

	void copyNodes(NJS_OBJECT* model);
	void stateWait();
	void stateCharge();
	void stateDone();

	NJS_OBJECT* mModelData; // _64 MODEL_BATTERY
	NJS_OBJECT mNodes[3];  // _68 three copied model nodes siblings cleared
	void* mTex;           // _110 TLS_MODEL_BATTERY
	void* mMotion;        // _114 MOTION_BATTERY
	DrawTask* mDrawTask;  // _118
	ObjModel mModel;      // _11C
	f32 mMatrix[12];      // _13C
	u16 mSwitch;          // _16C
	u16 _16E;
	HitVec mEnds[2];      // _170 where the bolts rest once the motion finishes
	f32 mFrame;           // _188
	f32 mAnim[12];        // _18C the motion node for this frame
	Vec3f mCenter;        // _1BC
	f32 mRadius;          // _1C8
	BatteryBolt mBolts[2]; // _1CC
	void* mMesh;          // _3F4
	int mAct;             // _3F8 0 waiting 1 charging 2 open
};

extern "C" {
TObject* createTObjOrangeBattery(TObject*, SetDesc*);
}

#include "stage/Stage.h"
extern "C" {
extern s8 sBatteryOffsetAGuard;
extern s8 sBatteryOffsetBGuard;
extern Vec3f sBatteryOffsetA;
extern Vec3f sBatteryOffsetB;
}

#endif
