#ifndef _OBJECT_GREEN_TOBJGREENBOSSFENCE_H_
#define _OBJECT_GREEN_TOBJGREENBOSSFENCE_H_
#include "enemy/EnemyLight.h"
struct DrawTask;
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjGreenBossFence;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjGreenBossFence(TObject*, SetDesc*);
}

class TObjGreenBossFence : public TSetObj {
public:
	TObjGreenBossFence(TObject* parent, SetDesc* desc);
	virtual ~TObjGreenBossFence();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	DrawTask* mDrawTask;  // _90
	EnemyLight mLight; // _94
};

extern "C" {
extern u32 gTObjGreenBossFenceName;
}

#endif
