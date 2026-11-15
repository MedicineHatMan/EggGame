#ifndef _TOBJBLUEFLAGSCENE_H
#define _TOBJBLUEFLAGSCENE_H

#include "engine/DrawTask.h"
#include "engine/FlagCloth.h"
#include "Ninja/ninja.h"
#include "egg/ObjEgg.h"
#include "stage/SetActor.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The same cloth as TObjBlueFlag, with no cylinder. Set int 0 pins the cloth, set int 1 picks
 * the texture, set int 2 turns the wind and set int 3 is how hard it wanders. The floats are
 * added to a reference of 0.5, a wind speed, a spring of 0.3 and an energy loss of 0.99.
 * The cloth is 10 by 8 quads, and it hangs in a wind of (0, -0.01, 0).
 *
 * @size 0x524
 */
class TObjBlueFlagScene : public TSetActor {
public:
	TObjBlueFlagScene(TObject* parent, SetDesc* desc);
	virtual ~TObjBlueFlagScene();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void buildNormals();

	DrawTask* mDraw;       // _64
	NJS_TEXLIST* mTexList; // _68 geobj texture 15
	int mAnchor;           // _6C set int 0 -1 until the first init
	s16 mTex;              // _70 set int 1 clamped to the list
	u8 _72[2];
	FlagCloth* mCloth; // _74
	u8 _78[4];
	f32 mRef;              // _7C set float 0 plus 0.5
	HitVec mNormal[9][11]; // _80 one per cloth point
};

extern "C" {
extern s8 sFlagSceneHitVecsGuard;
extern HitVec sFlagSceneHitVecs[8 * 10 * 2];
}

#endif
