#ifndef _OBJECT_GREEN_TOBJGREENWATERFALL_H_
#define _OBJECT_GREEN_TOBJGREENWATERFALL_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "engine/WaterSurface.h"
#include "object/ObjAction.h"
#include "sound/SoundHandle.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjGreenWaterfall;
struct WaterfallNode;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjGreenWaterfall(TObject*, SetDesc*);
}

struct WaterfallNode {
	u8 _00[8];
	Vec3f mPos;             // _08
	u8 _14[4];
	int mRotY;              // _18
	u8 _1C[4];
	f32 mSizeX;             // _20
	f32 mSizeY;             // _24
	f32 mSizeZ;             // _28
	WaterfallNode* mChild;  // _2C
	WaterfallNode* mNext;   // _30
};

class TObjGreenWaterfall : public TSetActor {
public:
	TObjGreenWaterfall(TObject* parent, SetDesc* desc);
	virtual ~TObjGreenWaterfall();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void vfn_74();

	void wake();
	void updateFlow();
	void updateBroken();

	WaterfallNode* mModel; // _64
	NJS_TEXLIST* mTexList;        // _68
	void* mBreakModel;     // _6C
	void* mBreakTexList;   // _70
	DrawTask* mDrawTask;       // _74
	WaterSurface mWater;   // _78
	SoundHandle mSound;    // _108
	FlagByte mDrawFlags;   // _134 1 = draw the water
	u8 _135[3];
	int mAct;              // _138 0 flowing 1 broken
};

extern "C" {
extern const char* gTObjGreenWaterfallName;
extern f32 gGreenWaterfallSetRange;
}

#endif
