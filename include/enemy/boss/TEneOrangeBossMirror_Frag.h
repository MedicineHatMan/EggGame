#ifndef _ENEMY_BOSS_TENEORANGEBOSSMIRROR_FRAG_H
#define _ENEMY_BOSS_TENEORANGEBOSSMIRROR_FRAG_H
#include "egg/ObjEgg.h"
struct DrawTask;
#include "stage/Stage.h"
struct MirrorShard;
struct TEneOrangeBossMirror_Frag;

#include "core/types.h"

/** One triangle. @size 0x48 */
struct MirrorShard {
	MirrorShard();

	void setVel(const Vec3f* from);
	void setSpin(int yaw);
	void setCorners(f32 sizeX, f32 sizeY);
	void step();

	Vec3f mPos;       // _00
	Vec3f mVel;       // _0C
	int mRotX;        // _18
	int mRotY;        // _1C
	int mSpin;        // _20
	HitVec mCorners[3]; // _24
};

/**
 * @size 0x1228
 */
class TEneOrangeBossMirror_Frag : public TObject {
public:
	TEneOrangeBossMirror_Frag(TObject* parent, const Vec3f* pos, int yaw);
	virtual ~TEneOrangeBossMirror_Frag();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	DrawTask* mDrawTask;        // _20
	MirrorShard mShards[64]; // _24
	int mAlpha;             // _1224
};

extern "C" {
extern const char* gTEneOrangeBossMirror_FragName;
}

#endif
