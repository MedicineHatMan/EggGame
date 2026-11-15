#ifndef _TOBJTITLELAKE_H
#define _TOBJTITLELAKE_H
struct SetDesc;
struct TObject;

#include "engine/DrawTask.h"
struct NJS_OBJECT;
struct NJS_TEXLIST;
#include "engine/WaterSurface.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The title-screen lake. The model is slot 8 of the title archive. The water reads the same set record.
 *
 * @size 0x100
 */
class TObjTitleLake : public TSetActor {
public:
	TObjTitleLake(TObject* parent, SetDesc* desc);
	virtual ~TObjTitleLake();

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
TObject* createTObjTitleLake(TObject*, SetDesc*);
}

extern "C" {
extern u32 gTObjTitleLakeName;
}

#endif
