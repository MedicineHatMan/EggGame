#ifndef _OBJECT_YELLOW_TOBJYELLOWSANDFLOOR_H_
#define _OBJECT_YELLOW_TOBJYELLOWSANDFLOOR_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "engine/MeshColli.h"
#include "engine/ScrollModel.h"
#include "object/ObjCommon.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjYellowSandFloor;

#include "core/types.h"

class SetDesc;
class TObject;

extern "C" {
TObject* createTObjYellowSandFloor(TObject*, SetDesc*);
}

class TObjYellowSandFloor : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjYellowSandFloor(TObject* parent, SetDesc* desc);
	virtual ~TObjYellowSandFloor();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int sendToHandle(ObjMsg* msg);
	virtual void vfn_74();

	void updatePlace();
	void updateColliBox();
	void stateWait();
	void stateRun();

	NJS_OBJECT* mModelData;     // _64
	NJS_TEXLIST* mTexList;       // _68
	void* mColliData;     // _6C
	NJS_OBJECT* mTopModel;      // _70
	void* mTopTexList;    // _74
	DrawTask* mDrawTask;      // _78
	MeshColli* mColli;         // _7C
	f32 mMatrix[12];      // _80
	ScrollModel mSurface; // _B0
	SetTransform mTrans;  // _D0
	u32 _114;             // _114
	Vec3f mCenter;        // _118
	f32 mRadius;          // _124
	int mAct;             // _128
};

extern "C" {
extern const char* gTObjYellowSandFloorName;
}

#endif
