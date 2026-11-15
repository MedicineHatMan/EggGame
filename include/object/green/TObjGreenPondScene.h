#ifndef _OBJECT_GREEN_TOBJGREENPONDSCENE_H_
#define _OBJECT_GREEN_TOBJGREENPONDSCENE_H_
#include "engine/WaterSurface.h"
struct DrawTask;
struct NJS_TEXLIST;
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjGreenPondScene;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
void waterTevStage(int);
void waterTexGen0(int, int, int);
void waterTexGen1();
}

extern "C" {
TObject* createTObjGreenPondScene(TObject*, SetDesc*);
}

class TObjGreenPondScene : public TSetActor {
public:
	TObjGreenPondScene(TObject* parent, SetDesc* desc);
	virtual ~TObjGreenPondScene();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void* mModel;          // _64
	NJS_TEXLIST* mTexList; // _68
	DrawTask* mDrawTask;   // _6C
	WaterSurface mWater;   // _70
	int mIndex;            // _100 0 to 3 though the tables hold three
	u8 _104[0x10];
};

extern "C" {
extern const char* gTObjGreenPondSceneName;
extern f32 gGreenPondSceneSetRange;
}

#endif
