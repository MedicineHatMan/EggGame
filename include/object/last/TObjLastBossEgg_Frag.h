#ifndef _OBJECT_LAST_TOBJLASTBOSSEGG_FRAG_H_
#define _OBJECT_LAST_TOBJLASTBOSSEGG_FRAG_H_
#include "stage/SetManager.h"
struct NJS_OBJECT;
struct DrawTask;
#include "stage/Stage.h"
struct TObjLastBossEgg_Frag;
struct Vec3f;

#include "core/types.h"

extern "C" {
void yellowBossRingExit();
void yellowBossRingInit();
}

extern "C" {
void spawnTObjLastBossEgg_Frag(void*, const Vec3f*, const Vec3f*, int);
void spawnTObjLastBossEgg_FragShell(void*, const Vec3f*, const Vec3f*, int);
}

class TObjLastBossEgg_Frag : public TObject {
public:
	TObjLastBossEgg_Frag(TObject* parent, int kind, void* model, const Vec3f* pos, Vec3f dir, int angle);
	virtual ~TObjLastBossEgg_Frag();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	DrawTask* mDraw;     // _20
	SetTransform mXform; // _24 mVel is the flight mRot spins
	int mSpinX;          // _68
	int mSpinY;          // _6C
	NJS_OBJECT* mModel;  // _70
	f32 mRadius;         // _74
	int mKind;           // _78 1 stays put
	int mLife;           // _7C
};

#endif
