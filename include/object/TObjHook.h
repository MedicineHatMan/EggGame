#ifndef _OBJECT_TOBJHOOK_H_
#define _OBJECT_TOBJHOOK_H_
#include "Ninja/ninja.h"
#include "engine/DrawTask.h"
#include "engine/dlist.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjHook;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjHook(TObject*, SetDesc*);
}

class TObjHook : public TSetActor {
public:
	TObjHook(TObject* parent, SetDesc* desc);
	virtual ~TObjHook();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                         // _0C
	virtual void vfn_10();                       // _10
	virtual void render();                       // _1C
	virtual void getNodeMatrix(int node, f32* mtx); // _24
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	int checkHit();
	void setShape(f32 radius);

	ObjModel mModel; // _64
	ObjPlace mPlace; // _84
	DrawTask* mDraw; // _E0
	u32 _E4;         // _E4
	f32 _E8;         // _E8 cleared by vfn_74
	int mHooked;     // _EC set while something touches the hook
	u32 _F0;         // _F0 cleared by vfn_74
};

extern "C" {
extern void* gTObjHookName;
}

#endif
