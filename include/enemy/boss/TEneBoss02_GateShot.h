#ifndef _TENEBOSS02_GATESHOT_H
#define _TENEBOSS02_GATESHOT_H
#include "engine/DrawTask.h"

#include "stage/Stage.h"

class TEneBoss02_WaterShot_Visual;

/**
 * The water shot of TEneBoss02's entrance. It rides the root node of a motion; at frame 30 it opens a gate where
 * it is, at 170 it starts the splash and pool, and at 320 it closes the arena's gates and goes. The name is ours.
 *
 * @size 0x44
 */
class TEneBoss02_GateShot : public TObject {
public:
	TEneBoss02_GateShot(TObject* parent, void* motion, f32 frame, f32 rate);
	virtual ~TEneBoss02_GateShot();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	void getPos(Vec3f* out);

	void* mMotion;                        // _20
	f32 mFrame;                           // _24
	f32 mRate;                            // _28
	int mState;                           // _2C 0 flying 1 gate open 2 splashing 4 done
	Vec3f mPos;                           // _30
	u32 _3C;                              // _3C
	TEneBoss02_WaterShot_Visual* mVisual; // _40
};

/** See TEneBoss02_Pool.cpp */
class TEneBoss02_WaterShot_Visual : public TObject {
public:
	TEneBoss02_WaterShot_Visual(TObject* parent, const Vec3f* pos);
	virtual ~TEneBoss02_WaterShot_Visual();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void setPos(const Vec3f* pos);

	Vec3f mPos;      // _20
	DrawTask* mDrawTask; // _2C
	TObject* mFx;    // _30
	TObject* mFx2;   // _34
};

#endif
