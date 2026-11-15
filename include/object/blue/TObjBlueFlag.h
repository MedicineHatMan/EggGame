#ifndef _TOBJBLUEFLAG_H
#define _TOBJBLUEFLAG_H
struct SetDesc;
struct TObjBlueFlagScene;
struct TObject;

#include "engine/DrawTask.h"
#include "engine/FlagCloth.h"
#include "Ninja/ninja.h"
#include "egg/ObjEgg.h"
#include "stage/SetActor.h"

extern "C" void objHeapFree(void*, void*);

/**
 * A cloth flag the egg can push. Set int 0 picks which points stay pinned, set int 1 is the
 * texture, set int 2 turns the wind and set int 3 is how hard that wind wanders. The floats
 * are added to a reference of 0.5, a wind speed, a spring of 0.3 and an energy loss of 0.99.
 * The cloth is 15 by 12 quads. A touch plays sound 0x209 once, until the cylinder is clear again.
 *
 * @size 0xA5C
 */
class TObjBlueFlag : public TSetActor {
public:
	TObjBlueFlag(TObject* parent, SetDesc* desc);
	virtual ~TObjBlueFlag();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void makeShape();
	void placeShape();
	void freeShape();
	void buildNormals();

	NJS_TEXLIST* mTexList;     // _64 geobj texture 15
	DrawTask* mDraw;           // _68
	int mAnchor;               // _6C set int 0 -1 until the first init
	s16 mTex;                  // _70 set int 1 clamped to the list
	u8 _72[2];
	FlagCloth* mCloth;         // _74
	u8 _78[4];
	f32 mRef;                  // _7C set float 0 plus 0.5
	HitVec mTouch[2];          // _80 where the egg is and where it was
	int mWasClear;             // _98 1 after a frame with no contact so the next one sounds
	HitVec mNormal[13][16];    // _9C one per cloth point
};

extern "C" {
void initBlueFlag();
void initBlueFlagSet();
void termBlueFlag();
void termBlueFlagSet();
}

extern "C" {
TObject* createTObjBlueFlag(TObject*, SetDesc*);
TObject* createTObjBlueFlagScene(TObject*, SetDesc*);
}

extern "C" {
extern s8 sFlagHitVecsGuard;
extern HitVec sFlagHitVecs[12 * 15 * 2];
}

#endif
