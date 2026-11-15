#ifndef _TOBJBOSS02KATANA_H
#define _TOBJBOSS02KATANA_H
struct SetDesc;
struct TObject;

#include "stage/SetManager.h"
struct DrawTask;
#include "enemy/TEneAm02.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The katana in the blue boss room. Model 1 is lit, model 2 is drawn with a fixed grey tev colour.
 * Both come from ar_obj_blue_boss.arc, which TObjBoss02Fence loads.
 *
 * @size 0xD4
 */
class TObjBoss02Katana : public TSetObj {
public:
	TObjBoss02Katana(TObject* parent, SetDesc* desc);
	virtual ~TObjBoss02Katana();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	DrawTask* mDrawTask;   // _90
	EnemyLight mLight; // _94
};

extern "C" {
TObject* createTObjBoss02Katana(TObject*, SetDesc*);
}

extern "C" {
extern const char* gTObjBoss02KatanaName;
}

#endif
