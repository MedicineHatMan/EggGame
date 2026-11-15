#ifndef _OBJECT_MISSION_TMSNOBJRING_H_
#define _OBJECT_MISSION_TMSNOBJRING_H_
#include "Ninja/ninja.h"
struct RingShape;
#include "enemy/EnemyLight.h"
#include "engine/DrawTask.h"
#include "object/ObjCommon.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TMsnObjRing;
struct SetDesc;
struct TObject;

#include "core/types.h"

struct TArc;

extern "C" {
void initRingArc();
void releaseRingArc();
int scriptLoadRing(void*);
}

extern "C" {
void ringShineTev(int);
void ringShineTexGen(int);
}

extern "C" {
TObject* createTMsnObjRing(TObject*, SetDesc*);
}

// Vtable slot _30 is 0x80107360 and slot _34 is 0x8010754C
class TMsnObjRing : public TSetActor {
public:
	TMsnObjRing(TObject* parent, SetDesc* desc);
	virtual ~TMsnObjRing();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C the update
	virtual void vfn_10(); // _10 the visibility
	virtual void render(); // _1C
	virtual int getKind(); // _2C
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);

	void setup();
	void checkHits();
	void floatAway();
	void hop();

	NJS_TEXLIST* mTexList; // _64
	void* mShineTex;       // _68
	ObjDrawState mState;   // _6C
	ObjPlace mPlace;       // _90
	EnemyLight mLight;     // _EC
	ObjShadow mShadow;     // _12C
	DrawTask* mDrawTask;   // _164
	int mMode;             // _168
	int mSwitch;           // _16C
	int mKind;             // _170 0 or 1
	f32 mScale;            // _174
	int mSpin;             // _178 added to mRot.y each frame
	int mLife;             // _17C
	f32 mSpeed;            // _180 up
	int mOnGround;         // _184 whether the shadow found ground
};

// A ring's shape and size, by kind. @size 0x30
struct RingShape {
	u8 _00[0x18];
	f32 mRadius; // _18
	u8 _1C[0x14];
};

extern "C" {
extern struct RingShape sRingShapes[];
}

extern "C" {
extern TArc* gRingArc;
extern void* gRingAsset0;
extern void* gRingAsset1;
extern void* gRingAsset2;
extern void* gRingAsset3;
extern NJS_OBJECT* gRingShineModel;
}

#endif
