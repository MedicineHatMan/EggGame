#include "stage/Stage.h"
#include "engine/PortalVis.h"
#include "egg/EggCreature.h"
#include "core/Mem.h"
#include "Ninja/ninja.h"

#include "engine/SceneMeshes.h"
#include "engine/MeshPool.h"

#pragma dont_inline on
extern "C" void sceneStepScrolls(SceneMeshes* scene)
{
	if (0) {
		((DirLight*)scene)->DirLight::~DirLight();
		((AmbientLight*)scene)->AmbientLight::~AmbientLight();
	}

	for (SceneScroll* it = scene->mScrolls; it != scene->mScrolls + scene->mScrollCount; it++) {
		it->mTime += it->mSpeed;
		if (it->mTime >= it->mPeriod) {
			it->mTime -= it->mPeriod;
		}
	}
}

#pragma dont_inline reset

// 0x8000EBC4
// Sorts the meshes inside the view's depth range into the lists by camera depth
extern "C" void sceneSortMeshes(SceneMeshes* scene, PortalVis*, Frustum* view, f32* matrix)
{
	SceneModel* model = *scene->mModel;
	CombVec<u8>::fill(((WorldVector<u8>*)&scene->mSeen)->begin(), ((WorldVector<u8>*)&scene->mSeen)->end(), sSceneFillByte);
	meshU16VecClear((MeshVector<u16>*)&scene->mFar);
	meshU16VecClear((MeshVector<u16>*)&scene->mNear);
	meshU16VecClear((MeshVector<u16>*)&scene->mFlagged);
	meshU16VecClear((MeshVector<u16>*)&scene->mExtra);
	njPushMatrix(matrix);
	njInvertMatrix(0);
	f32 far = view->mFar;
	for (int i = 0; i < model->mCount; i++) {
		SceneMesh* mesh = &model->mMeshes[i];
		Vec3f at;
		njCalcPointEx(&mesh->mCentre, &at);
		f32 front;
		f32 back;
		f32 r;
		r     = mesh->mRadius;
		front = at.z - r;
		back  = at.z + r;
		if (front > far || back < view->mNear) {
			continue;
		}
		if (mesh->mFlags & 1) {
			u16 index = i;
			meshU16VecPush((MeshVector<u16>*)&scene->mFlagged, &index);
		} else if (front < 100.0f) {
			u16 index = i;
			meshU16VecPush((MeshVector<u16>*)&scene->mNear, &index);
		} else {
			u16 index = i;
			meshU16VecPush((MeshVector<u16>*)&scene->mFar, &index);
		}
	}
	njPopMatrixEx();
	if (0) {
		((CombVec<u8>*)scene)->CombVec<u8>::~CombVec();
		((CombVec<u8>*)scene)->reserve(0);
	}
}
