#ifndef _ENEMY_BOSS_TENEPURPLEBOSS_ICEBALL_H_
#define _ENEMY_BOSS_TENEPURPLEBOSS_ICEBALL_H_
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TEnePurpleBoss_IceBall;

#include "core/types.h"

class TObject;
struct Vec3f;
struct NJS_OBJECT;

extern "C" {
void breakAllIceBalls();
void enforceIceBallLimit();
void iceBallLoad();
void iceBallFree();
void setIceBallLimit(int);
void throwIceBalls(TObject*, const Vec3f*, const f32*, int, int, f32);
}

/** @size 0x28 */
struct IceChunk {
	IceChunk();

	Vec3f mPos; // _00
	Vec3f mVel; // _0C
	int mRotX;  // _18
	int mRotY;  // _1C
	int mSpinX; // _20
	int mSpinY; // _24
};

/**
 * @size 0x4B4
 */
class TEnePurpleBoss_BigIceFragment : public TObject {
public:
	TEnePurpleBoss_BigIceFragment(TObject* parent, const Vec3f* pos, const f32* rot, f32 offset);
	virtual ~TEnePurpleBoss_BigIceFragment();

	void* operator new(unsigned long size) { return objHeapAlloc(gObjHeap, size); }
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	IceChunk mChunks[26];    // _20
	NJS_OBJECT* mModels[26]; // _430
	f32 mRot[4];             // _498 a quaternion
	int mWait;               // _4A8
	u32 mColor;              // _4AC
	DrawTask* mDrawTask;     // _4B0
};

/**
 * @size 0x78
 */
class TEnePurpleBoss_IceBall : public TObjAction {
public:
	TEnePurpleBoss_IceBall(TObject* parent, const Vec3f* pos, const Vec3f* vel, f32 radius);
	virtual ~TEnePurpleBoss_IceBall();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual int getKind();
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void vfn_6C(const Vec3f* pos);

	void link();
	void unlink();
	void syncCollider();
	void markBroken();
	void roll();
	void checkHits();
	void bounceEffect(const Vec3f* at, const Vec3f* normal);
	void shatter();

	TEnePurpleBoss_IceBall* mPrev; // _30
	TEnePurpleBoss_IceBall* mNext; // _34
	Vec3f mPos;                    // _38
	f32 mRot[4];                   // _44 a quaternion
	f32 mSpeed;                    // _54
	Vec3f mVel;                    // _58
	f32 mRadius;                   // _64
	CharaColli* mColli;            // _68
	DrawTask* mDrawTask;           // _6C
	int mState;                    // _70 0 not yet solid 1 rolling 2 breaking
	int mWait;                     // _74
};

extern "C" {
extern const char* gTEnePurpleBoss_BigIceFragmentName;
extern const char* gTEnePurpleBoss_IceBallName;
extern int sIceBallLimit;
extern TEnePurpleBoss_IceBall* sIceBallHead;
}

#endif
