#ifndef _OBJECT_LAST_TOBJLASTEGGLIGHT_H_
#define _OBJECT_LAST_TOBJLASTEGGLIGHT_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "engine/ScrollModel.h"
#include "sound/SeHolder.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjLastEggLight;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjLastEggLight(TObject*, SetDesc*);
}

class TObjLastEggLight : public TSetObj {
public:
	TObjLastEggLight(TObject* parent, SetDesc* desc);
	virtual ~TObjLastEggLight();
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
	int mSpin;            // _BC added to the yaw each frame
	SeHolder mTone;       // _C0 the sound
};

extern "C" {
extern NJS_OBJECT* gLastBossFeatherObject;
extern NJS_MOTION** gLastBossFeatherMotion;
extern NJS_TEXLIST* gLastBossFeatherTexList;
extern NJS_OBJECT** gLastEggLightModels;
extern NJS_TEXLIST** gLastEggLightTexLists;
}

#endif
