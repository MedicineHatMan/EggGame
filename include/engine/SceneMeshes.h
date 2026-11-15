#ifndef _ENGINE_SCENEMESHES_H_
#define _ENGINE_SCENEMESHES_H_
#include "Ninja/ninja.h"
#include "stage/Stage.h"
struct SceneMesh;
struct SceneModel;
struct SceneScroll;
struct Frustum;
struct PortalVis;
struct SceneMeshes;

#include "core/types.h"
#include "engine/WorldVector.h"
#include "engine/CombVec.h"

extern "C" {
void sceneStepScrolls(SceneMeshes*);
void sceneSortMeshes(SceneMeshes*, PortalVis*, Frustum*, f32*);
}

template <typename T>
struct SceneVector {
	T* mData;  // _00
	int mSize; // _04
	int mCap;  // _08
};

/** One mesh of the landscape model. @size 0x40 */
struct SceneMesh {
	u32 mFlags;    // _00 bit 0 = drawn in the third list
	u32 _04;       // _04
	Vec3f mCentre; // _08
	f32 mRadius;   // _14
	u8 _18[0x28];
};

struct SceneModel {
	u16 mCount;         // _00
	SceneMesh* mMeshes; // _04
};

/** A texture scroll. @size 0x10 */
struct SceneScroll {
	u32 _00;     // _00
	f32 mSpeed;  // _04
	f32 mPeriod; // _08
	f32 mTime;   // _0C
};

struct SceneMeshes {
	SceneModel** mModel;       // _00
	SceneVector<u8> mSeen;     // _04 cleared every frame
	SceneVector<u16> mFar;     // _10 100 or more in front
	SceneVector<u16> mNear;    // _1C nearer
	SceneVector<u16> mFlagged; // _28 meshes with bit 0 set
	SceneVector<u16> mExtra;   // _34
	SceneScroll* mScrolls;     // _40
	int mScrollCount;          // _44
};

extern "C" {
extern const int sSceneFillByte;
}

#endif
