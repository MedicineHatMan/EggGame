#ifndef _ENEMY_BOSS_TENEBOSS02_POOL_H_
#define _ENEMY_BOSS_TENEBOSS02_POOL_H_
#include "enemy/boss/TEneBoss02.h"
struct NJS_TEXLIST;
struct NJS_OBJECT;
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TEneBoss02_Pool;
struct TEneBoss02_Gate;

#include "core/types.h"

class TObject;
struct Vec3f;

extern "C" {
void aimWaterShot(const Vec3f*, const Vec3f*, Vec3f*, f32, f32);
void clearWaterShotAssets();
void closeBossGates();
void* createWaterShot(TObject*, const Vec3f*, const Vec3f*);
void* createWaterShotGate(TObject*, const Vec3f*, const Vec3f*, int);
void findTakenGateSlot(Vec3f*);
void freeBossGates();
void initBossGates();
void resetBossGates();
void rippleBossGates(const Vec3f*, f32, f32, f32);
void setWaterShotAssets(NJS_OBJECT*, NJS_TEXLIST*);
int takeFreeGateSlot(Vec3f*);
}

extern "C" {
TEneBoss02_Gate* makeBossGate(TObject*, Vec3f*);
}

/**
 * @size 0x54
 */
class TEneBoss02_Pool : public TObjAction {
public:
	TEneBoss02_Pool(TObject* parent, const Vec3f* pos);
	virtual ~TEneBoss02_Pool();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual int onHit(const TObjAction* other, int arg, int* result);

	Vec3f mPos;           // _30
	int mTime;            // _3C
	f32 mRadius;          // _40
	f32 mAlpha;           // _44
	BossRipple* mSurface; // _48
	DrawTask* mDrawTask;  // _4C
	int mState;           // _50 0 growing 1 waiting 2 fading
};

/**
 * @size 0x54
 */
class TEneBoss02_Gate : public TObjAction {
public:
	TEneBoss02_Gate(TObject* parent, int slot, const Vec3f* pos);
	virtual ~TEneBoss02_Gate();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual int onHit(const TObjAction* other, int arg, int* result);

	void close();

	Vec3f mPos;           // _30
	int _3C;              // _3C
	f32 mRadius;          // _40
	BossRipple* mSurface; // _44
	DrawTask* mDrawTask;  // _48
	int mSlot;            // _4C
	int mState;           // _50 0 growing 1 open 2 closing
};

extern "C" {
extern NJS_OBJECT** gRedMagmaModels;
extern const char* gTEneBoss02_PoolName;
extern const char* gTEneBoss02_GateName;
extern const char* gTEneBoss02_WaterShot_VisualName;
extern const char* gTEneBoss02_WaterShotName;
extern const char* gTEneBoss02_WaterShot_GateName;
extern NJS_OBJECT* sBoss02ShotModel;
extern NJS_TEXLIST* sBoss02ShotTextures;
extern Vec3f* sBoss02GatePlaces;
extern int* sBoss02GateTaken;
extern TEneBoss02_Gate** sBoss02Gates;
extern int sBoss02GateCount;
void* createBossPool(TObject* parent, const Vec3f* pos);
TEneBoss02_Gate* makeBossGateAt(TObject* parent, int slot);
}

#endif
