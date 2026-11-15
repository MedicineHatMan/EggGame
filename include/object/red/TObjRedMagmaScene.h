#ifndef _TOBJREDMAGMASCENE_H
#define _TOBJREDMAGMASCENE_H
struct SetDesc;
struct TObjRedMagmaObj;
struct TObject;

#include "object/red/TObjRedMagma.h"
struct NJS_TEXLIST;

/**
 * The magma placed as a scene object. Same size as TObjRedMagma: it only supplies the model
 * table and the two distances.
 *
 * @size 0xB8
 */
class TObjRedMagmaScene : public TObjRedMagma {
public:
	TObjRedMagmaScene(TObject* parent, SetDesc* desc);
	virtual ~TObjRedMagmaScene() { }

	virtual void initFromDesc(SetDesc* desc); // _30
	virtual f32 getRange();                   // _7C returns gRedMagmaSceneRange
	virtual f32 getFarAdd();                  // _80 returns gRedMagmaSceneFarAdd

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
};

extern "C" {
void setRedMagmaSceneModels(int, int*);
void setRedMagmaObjModels(int, int*);
TObject* createTObjRedMagmaScene(TObject*, SetDesc*);
TObject* createTObjRedMagmaObj(TObject*, SetDesc*);
}

extern "C" {
extern NJS_TEXLIST** gRedMagmaTexLists;
extern f32 gRedMagmaSceneRange;
extern f32 gRedMagmaSceneFarAdd;
extern f32 gRedMagmaObjRange;
extern f32 gRedMagmaObjFarAdd;
extern int sRedMagmaCount;
extern int* sRedMagmaIndices;
extern int sRedMagmaSceneCount;
extern int* sRedMagmaSceneIndices;
}

#endif
