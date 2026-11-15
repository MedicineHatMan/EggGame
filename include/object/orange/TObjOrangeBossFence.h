#ifndef _TOBJORANGEBOSSFENCE_H
#define _TOBJORANGEBOSSFENCE_H
struct SetDesc;
struct TObject;

#include "stage/SetManager.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The orange boss fence. The model and texture live in gOrangeBossObjModels and gOrangeBossObjTextures, filled in
 * somewhere else. It deletes itself past gBossFenceRange of mTrans.mPos, and the light hides the draw
 * task past gBossFenceVisibleDist.
 *
 * @size 0xD4
 */
class TObjOrangeBossFence : public TSetObj {
public:
	TObjOrangeBossFence(TObject* parent, SetDesc* desc);
	virtual ~TObjOrangeBossFence();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	DrawTask* mDrawTask; // _90
	EnemyLight mLight;   // _94
};

extern "C" {
void loadOrangeBossObj();
void releaseOrangeBossObj();
}

extern "C" {
TObject* createTObjOrangeBossFence(TObject*, SetDesc*);
}

#include "io/ResBank.h"
extern "C" {
extern const char* gTObjOrangeBossFenceName;
extern ResBank gOrangeBossObjBank;
extern f32 gBossFenceRange;
extern f32 gBossFenceVisibleDist;
}

#endif
