#ifndef _OBJECT_GREEN_TOBJGREENBOSSRAY_H_
#define _OBJECT_GREEN_TOBJGREENBOSSRAY_H_
#include "engine/Fx.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjGreenBossRay;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjGreenBossRay(TObject*, SetDesc*);
}

class TObjGreenBossRay : public TSetActor {
public:
	TObjGreenBossRay(TObject* parent, SetDesc* desc);
	virtual ~TObjGreenBossRay();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void vfn_74();

	void updateMatrix();

	Fx* mFx;       // _64
	f32 mMatrix[12]; // _68
};

extern "C" {
extern const char* gTObjGreenBossRayName;
}

#endif
