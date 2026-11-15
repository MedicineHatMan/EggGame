#ifndef _OBJECT_TOBJANCHOR01_H_
#define _OBJECT_TOBJANCHOR01_H_
#include "enemy/EnemyLight.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "object/ObjCommon.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjAnchor01;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjAnchor01(TObject*, SetDesc*);
}

class TObjAnchor01 : public TSetActor {
public:
	TObjAnchor01(TObject* parent, SetDesc* desc);
	virtual ~TObjAnchor01();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void showHits();
	void swing();
	void fitShape();
	void placeShadow();
	virtual int vfn_48(TObjAction* other);                              // _48
	virtual int onHit(const TObjAction* other, int shape, int* result); // _58

	void* mModel;      // _64
	void* mTexture;    // _68
	DrawTask* mDraw;   // _6C
	ObjShadow mShadow; // _70
	f32 mScale;        // _A8
	f32 mRate;         // _AC 100
	int mSpeed;        // _B0
	int mStep;         // _B4
	int mLeft;         // _B8
	int mRange;        // _BC
	int mPhase;        // _C0 the swing's angle 0x10000 a turn
	u8 mLive;          // _C4
	u8 _C5[3];
	EnemyLight mLight; // _C8 an EnemyLight
	f32 mGroundY;      // _108
};

#endif
