#ifndef _OBJECT_GREEN_TOBJGREENBOSSLAKE_H_
#define _OBJECT_GREEN_TOBJGREENBOSSLAKE_H_
#include "engine/WaterSurface.h"
struct DrawTask;
struct NJS_OBJECT;
struct NJS_TEXLIST;
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjGreenBossLake;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjGreenBossLake(TObject*, SetDesc*);
}

class TObjGreenBossLake : public TSetActor {
public:
	TObjGreenBossLake(TObject* parent, SetDesc* desc);
	virtual ~TObjGreenBossLake();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	NJS_OBJECT* mModel;    // _64
	NJS_TEXLIST* mTexList; // _68
	DrawTask* mDrawTask;   // _6C
	GreenBossWater mWater; // _70
};

extern "C" {
extern const char* gTObjGreenBossLakeName;
}

#endif
