#ifndef _TOBJORANGEBALLOON_H
#define _TOBJORANGEBALLOON_H

#include "stage/Stage.h"
#include "engine/DrawTask.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One live balloon. The pool is 400 of these, free-listed through mNext. Ours.
 *
 * @size 0x14
 */
struct BalloonPuff {
	BalloonPuff();

	BalloonPuff* mNext; // _00
	u8 mKind;           // _04
	u8 _05;
	u16 mCell; // _06 index into the octree positions
	f32 mX;    // _08
	f32 mY;    // _0C
	f32 mZ;    // _10
};

/**
 * Balloons that rise out of the orange mirror octree. gBalloonPosExport is that tree. A byte per cell in
 * mGrid stops a cell at 8 balloons. The draw task culls them and render batches ten at a time.
 *
 * @size 0x40
 */
class TObjOrangeBalloon : public TObject {
public:
	TObjOrangeBalloon(TObject* parent);
	virtual ~TObjOrangeBalloon();

	inline void* operator new(u32 size) { return objHeapAlloc(gObjHeap, size); }
	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void render(); // _1C

	void spawn(u16 cell);
	static void bindTex(int kind);
	static void bindShape(int lod);

	DrawTask* mDrawTask; // _20
	u32 _24;
	BalloonPuff* mPool;   // _28
	BalloonPuff* mActive; // _2C
	BalloonPuff* mFree;   // _30
	int mCount;           // _34
	int mTimer;           // _38 counts down to a query then reloads to 60
	u8* mGrid;            // _3C balloons currently in each cell
};

extern "C" {
extern const char* gTObjOrangeBalloonName;
extern int sBalloonLastKind;
extern int sBalloonLastLod;
extern f32 gBalloonRandOffset;
extern const u32 sOrangeBalloonClearColor;
}

#endif
