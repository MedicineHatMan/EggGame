#ifndef _OBJECT_LAST_TOBJLASTBOSSEGG_H_
#define _OBJECT_LAST_TOBJLASTBOSSEGG_H_
#include "engine/FxHolder.h"
struct NJS_OBJECT;
struct DrawTask;
#include "engine/ScrollModel.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct SetDesc;
struct TObjLastBossEgg;

#include "core/types.h"

extern "C" {
TObject* createTObjLastBossEgg(TObject*, SetDesc*);
}

class TObjLastBossEgg : public TSetObj {
public:
	TObjLastBossEgg(TObject* parent, SetDesc* desc);
	virtual ~TObjLastBossEgg();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	ScrollModel mSurface; // _90
	NJS_OBJECT* mModel;   // _B0
	f32 mRadius;          // _B4
	DrawTask* mDraw;      // _B8
	int mOpen;            // _BC 1 once gLastBossEggOpen has been set
	FxHolder mGlow;       // _C0
};

extern "C" {
extern int gLastBossEggYaw;
extern Vec3f gLastBossEggPos;
}

#endif
