#ifndef _TOBJBOSS02FENCE_H
#define _TOBJBOSS02FENCE_H
struct SetDesc;
struct TObject;

#include "stage/SetManager.h"
struct NJS_TEXLIST;
struct NJS_OBJECT;
struct DrawTask;
#include "enemy/TEneAm02.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A piece of fence in the blue boss room. Model 0 of ar_obj_blue_boss.arc, lit by an EnemyLight.
 * It has no collision of its own here.
 *
 * @size 0xD4
 */
class TObjBoss02Fence : public TSetObj {
public:
	TObjBoss02Fence(TObject* parent, SetDesc* desc);
	virtual ~TObjBoss02Fence();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	DrawTask* mDrawTask;   // _90
	EnemyLight mLight; // _94
};

extern "C" {
NJS_OBJECT** boss02GetFenceModels();
NJS_TEXLIST** boss02GetFenceTextures();
void initBlueBoss();
void termBlueBoss();
}

extern "C" {
TObject* createTObjBoss02Fence(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern u32 gTObjBoss02FenceName;
extern ResBank sBoss02FenceBank;
extern NJS_OBJECT** sBoss02FenceModels;
extern NJS_TEXLIST** sBoss02FenceTextures;
}

#endif
