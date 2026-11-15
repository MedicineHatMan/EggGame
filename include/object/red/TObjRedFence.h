#ifndef _TOBJREDFENCE_H
#define _TOBJREDFENCE_H

#include "stage/SetManager.h"
#include "object/ObjModel.h"
#include "enemy/TEneAm02.h"
#include "engine/DrawTask.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One leaf of the red fence. TObjRedFenceCtrl builds four of these and places them.
 * The controller writes mPos and mRot. This object turns that into the drawn place.
 *
 * @size 0x114
 */
class TObjRedFence : public TObjAction {
public:
	TObjRedFence(TObject* parent);
	virtual ~TObjRedFence();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();        // _0C
	virtual void vfn_10();      // _10
	virtual void render();      // _1C
	virtual Vec3f* getPos();    // _20
	virtual void setPos(const Vec3f* pos); // _6C

	// Names are ours
	void setup();
	void startFx();
	void stopFx();
	void pointOn(Vec3f* out, u16 index);
	void pointOff(Vec3f* out, u16 index);

	DrawTask* mDrawTask; // _30
	EnemyLight mLight;   // _34
	ObjModel mModel;     // _74
	ObjPlace mPlace;     // _94
	Vec3f mPos;          // _F0 what getPos returns
	SetRot mRot;         // _FC angles turned into mPlace.mRot
	TObject* mFx[3];     // _108
};

#include "stage/Stage.h"
extern "C" {
extern Vec3f sRedFenceTop[3];
extern Vec3f sRedFenceBottom[3];
extern const char* gTObjRedFenceName;
}

#endif
