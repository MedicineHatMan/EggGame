#ifndef _OBJECT_MAP_TOBJMAPFOOTHOLD01_H_
#define _OBJECT_MAP_TOBJMAPFOOTHOLD01_H_
#include "enemy/EnemyLight.h"
#include "engine/DrawTask.h"
#include "engine/MeshColli.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjMapFoothold01;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjMapFoothold01(TObject*, SetDesc*);
}

class TObjMapFoothold01 : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjMapFoothold01(TObject* parent, SetDesc* desc);
	virtual ~TObjMapFoothold01();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual SetLink* getLink();

	void setup();
	void updateColliBox();
	void placeOffCentre();
	void stepWaiting();
	void stepFadeIn();
	void stepFadeOut();
	void stepWaitTouch();
	void stepFallDelay();
	void stepFall();
	void waitThen(int step);
	int isTouched();
	void updateTouchShape();

	ObjModel mModel;     // _64
	ObjPlace mPlace;     // _84
	DrawTask* mDrawTask; // _E0
	MeshObject* mColli;  // _E4
	ObjDrawState mState; // _E8
	int mType;           // _10C FootholdType
	int mPhase;          // _110 an angle
	int mSpeed;          // _114
	f32 mRange;          // _118
	f32 mOffset;         // _11C
	u32 mWait;           // _120
	Vec3f mVel;          // _124
	u32 mTimer;          // _130
	int mVisible;        // _134
	int mStep;           // _138
	u8 _13C[0xC];        // _13C
	EnemyLight mLight;   // _148
};

#endif
