#ifndef _OBJECT_MAP_TOBJMAPROCK01_H_
#define _OBJECT_MAP_TOBJMAPROCK01_H_
#include "engine/DrawTask.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjMapRock01;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjMapRock01(TObject*, SetDesc*);
}

class TObjMapRock01 : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TObjMapRock01(TObject* parent, SetDesc* desc);
	virtual ~TObjMapRock01();

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void checkTouch();
	void stateWait();
	void stateBroken();
	void setSize(f32 scale);

	ObjModel mModel;  // _64
	ObjPlace mPlace;  // _84
	DrawTask* mDraw;      // _E0
	int mAct;         // _E4 0 = whole 1 = broken
	f32 mScale;       // _E8
	int mKeyBit;      // _EC
};

extern "C" {
extern const char* gTObjMapRock01Name;
}

#endif
