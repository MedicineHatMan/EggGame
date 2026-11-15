#include "enemy/boss/EneBoss.h"
#include "engine/DrawTask.h"
#include "engine/PortalVis.h"
#include "core/TSceneManager.h"
#include "Ninja/ninja.h"
#include "engine/SceneMeshes.h"
#include "camera/TCameraCtrl.h"
struct DrawTask;

typedef char TSceneManager_size_check[(sizeof(TSceneManager) == 0x2C) ? 1 : -1];

// 0x80057920
SceneRecord::SceneRecord()
{
	mCamera = 0;
	mVis    = 0;
	_08     = 0;
	mLights = 0;
}

// 0x80057938
extern "C" SceneRecord* getSceneRecord()
{
	return sSceneRecord;
}

// 0x80057940
extern "C" void setSceneRecord(SceneRecord* record)
{
	sSceneRecord = record;
}

const char* gTSceneManagerName = "TSceneManager";

// 0x80057948
TSceneManager::TSceneManager(TObject* parent)
    : TObject(parent)
{
	mRecord = 0;
	_00     = (u32)gTSceneManagerName;
	mSize   = 0x2C;
	mTask1  = addDrawTask(this, (void (*)(void*))sceneDrawPass1, 0x21, 0);
	mTask2  = addDrawTask(this, (void (*)(void*))sceneDrawPass2, 0x22, 0);
}

// 0x800579D4
TSceneManager::~TSceneManager()
{
	removeDrawTask((DrawTask*)mTask1);
	removeDrawTask((DrawTask*)mTask2);
}

typedef void (*ScenePassFn)(void*, int);

/** The land scene passes reach */
struct SceneLand {
	u8 _00[0x4C];
	virtual void drawPass(int pass);
};

static inline void scenePass(TSceneManager* scene, int pass)
{
	u8* land = (u8*)scene->mRecord->_08;
	if (land != 0) {
		((SceneLand*)land)->drawPass(pass);
	}
}

// 0x80057A44
extern "C" void sceneDrawPass1(void* task)
{
	scenePass((TSceneManager*)task, 1);
}

// 0x80057A84
extern "C" void sceneDrawPass2(void* task)
{
	scenePass((TSceneManager*)task, 2);
}

// 0x80057AC4
void TSceneManager::draw()
{
	if (mRecord->_08 != 0) {
		sceneStepScrolls((SceneMeshes*)mRecord->_08);
	}
}

// 0x80057AF4
void TSceneManager::vfn_10()
{
	CamView* camera = (CamView*)camGetView(mRecord->mCamera);
	Frustum view;
	view.set(camera->mFov, camera->mAspect, 1.0f, 5000.0f);
	f32 matrix[12];
	njUnitMatrix(matrix);
	njTranslate(matrix, camera->mPos.x, camera->mPos.y, camera->mPos.z);
	njRotateY(matrix, camera->mYaw);
	PortalVis* target = mRecord->mVis;
	if (target != 0) {
		target->update(&view, matrix, &camera->mTarget);
	}
	if (mRecord->_08 != 0 && target != 0) {
		sceneSortMeshes((SceneMeshes*)mRecord->_08, target, &view, (float*)matrix);
	}
}
