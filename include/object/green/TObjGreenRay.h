#ifndef _OBJECT_GREEN_TOBJGREENRAY_H_
#define _OBJECT_GREEN_TOBJGREENRAY_H_
#include "engine/Fx.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjGreenRay;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjGreenRay(TObject*, SetDesc*);
}

class TObjGreenRay : public TSetActor {
public:
	TObjGreenRay(TObject* parent, SetDesc* desc);
	virtual ~TObjGreenRay();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void initFromDesc(SetDesc* desc);

	Fx* mFx;       // _64
	f32 mMatrix[12]; // _68
};

extern "C" {
extern const char* gTObjGreenRayName;
}

#endif
