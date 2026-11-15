#ifndef _TOBJTITLERAINBOW_H
#define _TOBJTITLERAINBOW_H
struct SetDesc;
struct TObject;

#include "engine/DrawTask.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The title-screen rainbow. No motion. Set float 0 is the scale, clamped to -1..10 and then added to 1.
 * Hidden while gTitleScene is 3. The ambient alpha is 255 times one minus the fog mix, or 255 when debugEditActive is set.
 *
 * @size 0x8C
 */
class TObjTitleRainbow : public TSetActor {
public:
	TObjTitleRainbow(TObject* parent, SetDesc* desc);
	virtual ~TObjTitleRainbow();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34

	DrawTask* mDraw; // _64
	ObjModel mModel; // _68
	f32 mScale;      // _88 set float 0 plus 1
};

extern "C" {
void initTObjTitleRainbow();
void termTObjTitleRainbow();
}

extern "C" {
TObject* createTObjTitleRainbow(TObject*, SetDesc*);
}

extern "C" {
extern const char* gTObjTitleRainbowName;
}

#endif
