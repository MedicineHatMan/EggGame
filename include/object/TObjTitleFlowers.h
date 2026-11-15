#ifndef _TOBJTITLEFLOWERS_H
#define _TOBJTITLEFLOWERS_H
struct SetDesc;
struct TObject;

#include "engine/DrawTask.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A title-screen clump of flowers. No motion. Set float 0 is the scale, clamped to -1..10 and then added to 1.
 *
 * @size 0x8C
 */
class TObjTitleFlowers : public TSetActor {
public:
	TObjTitleFlowers(TObject* parent, SetDesc* desc);
	virtual ~TObjTitleFlowers();

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
void initTObjTitleFlowers();
void termTObjTitleFlowers();
}

extern "C" {
TObject* createTObjTitleFlowers(TObject*, SetDesc*);
}

extern "C" {
extern const char* gTObjTitleFlowersName;
}

#endif
