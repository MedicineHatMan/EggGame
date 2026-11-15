#ifndef _ENEMY_BOSS_TENEBOSS02_POOLPOW_H_
#define _ENEMY_BOSS_TENEBOSS02_POOLPOW_H_
#include "stage/Stage.h"
struct TEneBoss02_PoolPow;
struct TObject;

#include "core/types.h"

class TStage;
struct BossRipple;
struct BossRippleBase;
struct Vec3f;

extern "C" {
void bossRippleExit();
void bossRippleInit();
void bossRippleSetPos(BossRippleBase*, const Vec3f*);
void bossRippleSetRadius(BossRippleBase*, f32);
TStage* createBlueBossStage();
BossRipple* createBossRipple();
void createPoolPow(const Vec3f*, f32, f32, f32);
void createPoolWatcher();
BossRipple* newBossRipple();
}

extern "C" {
void* createPoolPowGen(TObject*, const Vec3f*, int);
}

/**
 * @size 0x40
 */
class TEneBoss02_PoolPow : public TObject {
public:
	TEneBoss02_PoolPow(TObject* parent, const Vec3f* pos, f32 speed, f32 maxRadius, f32 height);
	virtual ~TEneBoss02_PoolPow();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	Vec3f mPos;     // _20
	f32 mPrev;      // _2C
	f32 mRadius;    // _30
	f32 mSpeed;     // _34
	f32 mHeight;    // _38 falls by 1% a frame
	f32 mMaxRadius; // _3C
};

extern "C" {
extern const char* gTEneBoss02_PoolPowName;
extern const char* gTEneBoss02_PoolPowGen_LocalName;
}

#endif
