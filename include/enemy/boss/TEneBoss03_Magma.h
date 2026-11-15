#ifndef _ENEMY_BOSS_TENEBOSS03_MAGMA_H_
#define _ENEMY_BOSS_TENEBOSS03_MAGMA_H_
struct TObject;
#include "egg/ObjEgg.h"
struct DrawTask;
#include "stage/Stage.h"
struct MagmaGrid;
struct TEneBoss03_Magma;

#include "core/types.h"

struct Vec3f;

extern "C" {
void createMagmaPow(const Vec3f*, f32, f32, f32);
void rippleMagma(const Vec3f*, f32, f32, f32);
}

/**
 * One grid. The name is ours.
 *
 * @size 0x38
 */
struct MagmaGrid {
	MagmaGrid(int cols, int rows, const Vec3f* pos, f32 width, f32 depth);
	~MagmaGrid();

	void setWarp();
	void push(const Vec3f* pos, f32 inner, f32 outer, f32 amount);
	void flush();
	void markMoving(int index);
	void markMoving2(int index);
	int update();
	void cull();
	void draw();

	u32 mFlags;       // _00 1 = warps what is behind it 2 = off screen
	f32 mDepth;       // _04
	f32 mWidth;       // _08
	int mRows;        // _0C
	int mCols;        // _10
	Vec3f mPos;       // _14
	HitVec* mVerts;   // _20
	HitVec* mNormals; // _24
	f32* mSpeeds;     // _28
	u32* mCells;      // _2C
	int mScrollA;     // _30
	int mScrollB;     // _34
};

/**
 * @size 0x48
 */
class TEneBoss03_Magma : public TObject {
public:
	TEneBoss03_Magma(TObject* parent);
	virtual ~TEneBoss03_Magma();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void push(const Vec3f* pos, f32 inner, f32 outer, f32 amount);

	MagmaGrid* mGrids[9]; // _20
	DrawTask* mDrawTask;  // _44
};

extern "C" {
extern const char* gTEneBoss03_MagmaPowName;
extern const char* gTEneBoss03_MagmaName;
extern TObject* sBoss03MagmaObj;
}

#endif
