#ifndef _ENEMY_TENEDEATHCROW_H_
#define _ENEMY_TENEDEATHCROW_H_
#include "stage/Stage.h"
struct DrawTask;
struct TEneDeathCrow;
struct TObject;

#include "core/types.h"

struct BossSuit;
struct Vec3f;

extern "C" {
void bossSuitClearFx(BossSuit*);
int bossSuitFade(f32, BossSuit*);
void bossSuitKeepFx(BossSuit*, const Vec3f*);
void bossSuitSetFade(f32, f32, BossSuit*);
void bossSuitSetFx(BossSuit*, int, int, int);
void bossSuitSetSink(f32, f32, f32, f32, f32, BossSuit*);
int bossSuitSink(f32, BossSuit*, const Vec3f*, void*);
int bossSuitUpdate(f32, BossSuit*, const Vec3f*, void*);
int bossSuitUpdateNoFx(f32, BossSuit*);
}

extern "C" {
void* createDeathCrowLoud(TObject*, const Vec3f*);
}

/**
 * @size 0x48
 */
class TEneDeathCrow : public TObject {
public:
	TEneDeathCrow(TObject* parent, const Vec3f* pos, int quiet);
	virtual ~TEneDeathCrow();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	Vec3f mPos;          // _20
	Vec3f mVel;          // _2C
	int mRotY;           // _38
	f32 mFrame;          // _3C loops at 20
	DrawTask* mDrawTask; // _40
	int mLife;           // _44
};

extern "C" {
extern const char* gTEneDeathCrowName;
extern const char* gTEneDeathCrowGenName;
}

#endif
