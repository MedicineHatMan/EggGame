#ifndef _TOBJREDMAGMA_H
#define _TOBJREDMAGMA_H

#include "stage/SetManager.h"
#include "engine/ScrollModel.h"
#include "engine/DrawTask.h"

extern "C" {
void objHeapFree(void*, void*);
extern void* gObjHeap;
}

/**
 * Scrolling magma surface. The record's first int picks a model index, and that index
 * selects a model from gRedMagmaModels. TObjRedMagmaScene and TObjRedMagmaObj are the placed objects.
 *
 * @size 0xB8
 */
class TObjRedMagma : public TSetObj {
public:
	TObjRedMagma(TObject* parent, SetDesc* desc);
	virtual ~TObjRedMagma();

	virtual void draw();       // _0C
	virtual void vfn_10();     // _10
	virtual void render();     // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34

	// Ours The base vtable leaves both slots 0
	virtual f32 getRange()  = 0; // _7C distance passed to isInRange
	virtual f32 getFarAdd() = 0; // _80 added to the view distance in vfn_10

	// Clamps the record's first int and binds that model
	void bindModel(SetDesc* desc, int count, int* indices);

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	int mModelIndex;      // _90 index into gRedMagmaModels
	ScrollModel mSurface; // _94
	DrawTask* mDraw;      // _B4
};

#endif
