#ifndef _OBJECT_TOBJEGGYOLK_H_
#define _OBJECT_TOBJEGGYOLK_H_
#include "Ninja/ninja.h"
#include "engine/DrawTask.h"
#include "engine/MotionPlayer.h"
#include "stage/Stage.h"
struct TObjEggYolk;

#include "core/types.h"

class TAeObjStageSelectE3;
class TMode;
class TModeAllocator;
struct Vec3f;

extern "C" {
TMode* createModeStageSelectE3(TModeAllocator*);
TAeObjStageSelectE3* createStageSelectE3();
void freeEggYolk();
void freeStageSelectE3();
void loadEggYolk();
void loadStageSelectE3();
void spawnEggYolk(f32, const Vec3f*);
}

class TObjEggYolk : public TObject {
public:
	enum State {
		YOLK_PROBE  = 0, // find the ground under it
		YOLK_FALL   = 1,
		YOLK_SPLASH = 2, // the motion then a 30 frame hold
		YOLK_MISS   = 3, // no ground fall for 30 frames and go
		YOLK_FADE   = 4,
	};

	TObjEggYolk(TObject* parent);
	virtual ~TObjEggYolk();
	virtual void draw();
	virtual void render();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	inline void fall()
	{
		mFallSpeed += -6.666667f;
		mFallSpeed *= 0.99f;
		mPos.y = 1.0f / 60.0f * mFallSpeed + mPos.y;
	}

	u8 mState;            // _20 State
	u16 mLife;            // _22
	f32 mScale;           // _24
	f32 mGroundY;         // _28
	f32 mFallSpeed;       // _2C
	f32 mAlpha;           // _30
	Vec3f mPos;           // _34
	MotionPlayer mMotion; // _40
	GjActor* mNode;       // _7C the actor
	DrawTask* mDraw;      // _80
};

#include "Dolphin/GX/GXTypes.h"
extern "C" {
extern GXColor sEggYolkClearColor;
extern TAeObjStageSelectE3* sAeStageSelectE3;
}

#endif
