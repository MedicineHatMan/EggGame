#ifndef _OBJECT_GREEN_TOBJGREENWINDMILL_H_
#define _OBJECT_GREEN_TOBJGREENWINDMILL_H_
#include "Ninja/ninja.h"
struct DrawTask;
#include "engine/MeshColli.h"
#include "engine/ModelDraw.h"
#include "sound/SoundHandle.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TObjGreenWindmill;
struct WindmillMesh;
struct WindmillNode;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjGreenWindmill(TObject*, SetDesc*);
}

struct WindmillMesh {
	u8 _00[0x14];
	Vec3f mCentre; // _14
	f32 mRadius;   // _20
};

struct WindmillNode {
	u8 _00[4];
	WindmillMesh* mMesh; // _04
	Vec3f mPos;          // _08
	u8 _14[0x18];
	WindmillNode* mChild; // _2C
};

class TObjGreenWindmill : public TSetActor {
public:
	TObjGreenWindmill(TObject* parent, SetDesc* desc);
	virtual ~TObjGreenWindmill();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual void vfn_74();

	void updateIdle();
	void updateSpinUp();
	void updateSpin();

	WindmillNode* mModel; // _64
	void* mCollData;      // _68
	DrawTask* mDrawTask;  // _6C
	ModelDraw mDraw;      // _70
	MotionTrack mTrack;   // _8C
	f32 mMatrix[12];      // _A8
	f32 mFanMatrix[12];   // _D8 node 0's matrix which the collision follows
	MeshColli* mColli;    // _108
	f32 mSpeed;           // _10C
	u16 mKey;             // _110
	SoundHandle mSound;   // _114
	Vec3f mCentre;        // _140
	f32 mRadius;          // _14C
	int mAct;             // _150 0 idle 1 speeding up 2 at full speed
};

extern "C" {
extern const char* gTObjGreenWindmillName;
extern f32 gGreenWindmillSetRange;
}

#endif
