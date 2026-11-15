#include "ui/TMainMenuModel.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "core/Mem.h"
#include "Ninja/ninja.h"
#include "enemy/boss/EneBoss.h"
#include "object/TObjLauncher.h"
#include "core/MathUtil.h"

const char* gTMainMenuModelName = "TMainMenuModel";
struct NJS_TEXLIST;
struct GjActor;
struct NJS_OBJECT;

// 0x80267BA0
// Three motions for each of the three kinds
extern "C" MotionEntry sMainMenuModelMotions[9] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 1.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 29.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 24.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 1.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 30.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 24.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 1.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 7, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 24.0f, 0.3f, 0.0f, 0 },
};

extern "C" {
TArc* sMainMenuModelArc; // main_menu_model.arc
void* sMainMenuBlock; // block at arc+0x4C
}

struct MenuBlock {
	u8 _00[8];      // _00
	void** mModels; // _08
	void* mMotions; // _0C
	void** mTex;    // _10
	void** mGvm;    // _14
};

typedef char TMainMenuModel_size_check[(sizeof(TMainMenuModel) == 0xC0) ? 1 : -1];

static inline void buildMenuActor(GjActor** actor, void* model)
{
	if (*(u32*)model & 0x2000) {
		*actor = (GjActor*)memAlloc(gjActorSize((NJS_OBJECT*)model));
		gjCreateActor((NJS_OBJECT*)model, (u8*)*actor);
		njPushMatrixEx();
		njUnitMatrix(0);
		gjBuildActorVertices(*actor);
		njPopMatrixEx();
	} else {
		*actor = 0;
	}
}

// 0x801B7AAC
// Name is ours
extern "C" void loadMainMenuModel()
{
	loadArc("main_menu_model.arc", &sMainMenuModelArc);
	MenuBlock* block = (MenuBlock*)sMainMenuModelArc->mData;
	sMainMenuBlock     = block;
	bindGvmTextures((u8*)*block->mGvm, (NJS_TEXLIST*)*block->mTex);
}

// 0x801B7AF8
// Name is ours
extern "C" void freeMainMenuModel()
{
	MenuBlock* block = (MenuBlock*)sMainMenuBlock;
	releaseTextures((NJS_TEXLIST*)*block->mTex);
	releaseResource(sMainMenuModelArc);
}

// 0x801B7B2C
extern "C" TMainMenuModel* createTMainMenuModel(TObject* parent, const Vec3f* pos, const int* angle, int kind, int priority)
{
	return new TMainMenuModel(parent, pos, angle, kind, priority);
}

// 0x801B7B98
TMainMenuModel::TMainMenuModel(TObject* parent, const Vec3f* pos, const int* angle, int kind, int priority)
    : TObject(parent), mActor(0), mKind((s8)kind), mPos(*pos), mAngle(*(const MenuModelAngle*)angle)
{
	_00       = (u32)gTMainMenuModelName;
	mSize     = 0xC0;
	mDraw     = addDrawTask(this, (void (*)(void*))drawObjectTask, priority, 0);
	setup();
}

// 0x801B7C60
TMainMenuModel::~TMainMenuModel()
{
	if (mActor != 0) {
		memFree(mActor);
		mActor = 0;
	}
	removeDrawTask(mDraw);
}

// 0x801B7CE0
// Kinds 0 1 and 2 each own three MotionEntry rows
void TMainMenuModel::setup()
{
	mAlt               = mKind + 3;
	buildMenuActor(&mActor, ((MenuBlock*)sMainMenuBlock)->mModels[mKind]);
	buildMenuActor(&mActorB, ((MenuBlock*)sMainMenuBlock)->mModels[mAlt]);
	if (mKind < 3) {
		mMotion.init(&sMainMenuModelMotions[mKind * 3], ((MenuBlock*)sMainMenuBlock)->mMotions);
		mMotionId = 0;
		mMotion.play(mMotionId);
		stepMotion();
	}
	mState = 0;
	updateMatrix();
}

// 0x801B7E2C
void TMainMenuModel::draw()
{
	if (mState != 0 && mState >= 0 && mState < 3) {
		stepMotion();
	}
	updateMatrix();
}

// 0x801B7E78
void TMainMenuModel::vfn_10() { }

// 0x801B7E7C
void TMainMenuModel::render()
{
	if (sMainMenuBlock == 0) {
		return;
	}
	f32 view[12];
	mtxConcat(view, (const f32*)gjGetMatrix(), mMtx);
	njPushMatrix(view);
	njSetTexture(*((MenuBlock*)sMainMenuBlock)->mTex);
	if (mState == 0) {
		if (mActorB != 0) {
			gjDrawActor(mActorB);
		} else {
			gjDrawObject((NJS_OBJECT*)((MenuBlock*)sMainMenuBlock)->mModels[mAlt]);
		}
	} else if (mActor != 0) {
		gjDrawActor(mActor);
	} else {
		int cur = mMotion.mCurrent;
		f32 frame = mMotion.mFrame;
		NJS_MOTION* motion = mMotion.getMotion(cur);
		gjDrawMotion((NJS_OBJECT*)((MenuBlock*)sMainMenuBlock)->mModels[mKind], motion, frame);
	}
	njPopMatrixEx();
}

// 0x801B7F70
void TMainMenuModel::updateMatrix()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateZYX(mMtx, mMtx, mAngle.v);
}

// 0x801B7FB4
void TMainMenuModel::stepMotion()
{
	mMotion.update();
	if (mActor != 0) {
		gjPushUnitMatrix();
		if (mMotion.isBlending() != 0) {
			gjMotionLinkActor(mActor, (void*)mMotion.getBlend(), mMotion.mBlendTime);
		} else {
			int cur = mMotion.mCurrent;
			f32 frame = mMotion.mFrame;
			gjMotionActor(frame, mActor, mMotion.getMotion(cur));
		}
		njPopMatrixEx();
	}
}

// 0x801B8054
void TMainMenuModel::play(s16 id)
{
	if (mMotionId != id) {
		mMotionId = id;
		mMotion.play(id);
		stepMotion();
	}
}

// 0x801B80A0
void TMainMenuModel::setIdle()
{
	play(0);
	mState = 0;
}

// 0x801B80D8
void TMainMenuModel::setState1()
{
	play(1);
	mState = 1;
}

// 0x801B8110
void TMainMenuModel::setState2()
{
	play(2);
	mState = 2;
}
