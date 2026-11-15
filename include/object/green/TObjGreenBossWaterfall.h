#ifndef _OBJECT_GREEN_TOBJGREENBOSSWATERFALL_H_
#define _OBJECT_GREEN_TOBJGREENBOSSWATERFALL_H_
#include "engine/WaterSurface.h"
struct DrawTask;
struct NJS_OBJECT;
struct NJS_TEXLIST;
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjGreenBossWaterfall;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjGreenBossWaterfall(TObject*, SetDesc*);
}

class TObjGreenBossWaterfall : public TSetActor {
public:
	TObjGreenBossWaterfall(TObject* parent, SetDesc* desc);
	virtual ~TObjGreenBossWaterfall();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	NJS_OBJECT* mModel;         // _64
	NJS_TEXLIST* mTexList;       // _68
	DrawTask* mDrawTask;      // _6C
	GreenBossWater mWater; // _70
};

extern "C" {
extern u32 gTObjGreenBossWaterfallName;
}

#endif
