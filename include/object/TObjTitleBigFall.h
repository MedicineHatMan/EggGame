#ifndef _TOBJTITLEBIGFALL_H
#define _TOBJTITLEBIGFALL_H
struct SetDesc;
struct TObject;

#include "engine/DrawTask.h"
struct NJS_OBJECT;
struct NJS_TEXLIST;
#include "engine/WaterSurface.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The title-screen waterfall. The model is slot 7 of the title archive. Same shape as the lake.
 *
 * @size 0x100
 */
class TObjTitleBigFall : public TSetActor {
public:
	TObjTitleBigFall(TObject* parent, SetDesc* desc);
	virtual ~TObjTitleBigFall();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void vfn_74();                    // _74 empty

	NJS_OBJECT* mModel;        // _64
	NJS_TEXLIST* mTexList;      // _68
	DrawTask* mDraw;     // _6C
	WaterSurface mWater; // _70
};

extern "C" {
TObject* createTObjTitleBigFall(TObject*, SetDesc*);
}

extern "C" {
extern u32 gTObjTitleBigFallName;
}

#endif
