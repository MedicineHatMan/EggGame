#ifndef _EGG_TGOLDENEGGBREAK_H_
#define _EGG_TGOLDENEGGBREAK_H_
#include "stage/Stage.h"
struct DrawTask;
struct EggShard;
struct TGoldenEggBreak;

#include "core/types.h"

extern "C" {
int goldenEggTick(s16*);
}

extern "C" {
void* goldenEggBreakDemoNew();
}

/**
 * @size 0x68
 */
struct EggShard {
	EggShard();

	void* mModel;    // _00
	Vec3f mPos;      // _04
	Vec3f mVel;      // _10
	f32 mRot[4];     // _1C
	Vec3f mSpin;     // _2C
	f32 mMatrix[12]; // _38
};

/**
 * @size 0x38
 */
class TGoldenEggBreak : public TObject {
public:
	TGoldenEggBreak(TObject* parent, const Vec3f* pos, const f32* rot, f32 radius);
	virtual ~TGoldenEggBreak();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void render();

	u16 mLife;         // _20
	u16 mTime;         // _22
	u16 mCount;        // _24
	DrawTask* mTask;       // _28
	f32 mRadius;       // _2C
	f32 mFloor;        // _30
	EggShard* mShards; // _34
};

extern "C" {
extern const char* gTGoldenEggBreakName;
extern const char* gTGoldenEggBreakDemoName;
}

#endif
