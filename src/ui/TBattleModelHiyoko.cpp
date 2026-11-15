#include "ui/TBattleModelHiyoko.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "ui/AeArc.h"
#include "object/TObjLauncher.h"
#include "core/Mem.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"

MotionEntry sBattleModelHiyokoMotions[4] = {
	{ 0, 0, -1, 0, 0.0f, 49.0f, 0.25f, 0.0f, 0 },
	{ 0, 1, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },
	{ 2, 2, -1, 3, 0.0f, 24.0f, 0.25f, 0.0f, 0 },
	{ 0, 3, -1, 0, 0.0f, 39.0f, 0.5f, 0.0f, 0 },
};

const char* gTBattleModelHiyokoName = "TBattleModelHiyoko";
struct NJS_TEXLIST;
struct GjActor;
struct NJS_OBJECT;

extern "C" {
TArc* sBattleModelHiyokoArc; // battlemodel_hiyoko.arc
void* sBattleHiyokoBlock;    // the block at arc+0x4C
int sBattleHiyokoNode;       // node counter for the motion callback
}

// Block inside the archive
struct HiyokoBlock {
	u8 _00[8];      // _00
	void** mModels; // _08
	void* mMotions; // _0C
	void** mTex;    // _10
	void** mGvm;    // _14
};

// The block is read again at each use
#define HIYOKO_BLOCK ((HiyokoBlock*)sBattleHiyokoBlock)

typedef char TBattleModelHiyoko_size_check[(sizeof(TBattleModelHiyoko) == 0xB0) ? 1 : -1];

static void hiyokoNodeCallback();

// 0x80198754
// Name is ours
extern "C" void loadBattleModelHiyoko()
{
	loadArc("battlemodel_hiyoko.arc", &sBattleModelHiyokoArc);
	HiyokoBlock* block = (HiyokoBlock*)sBattleModelHiyokoArc->mData;
	sBattleHiyokoBlock = block;
	bindGvmTextures((u8*)*block->mGvm, (NJS_TEXLIST*)*block->mTex);
}

// 0x801987A0
// Name is ours
extern "C" void freeBattleModelHiyoko()
{
	HiyokoBlock* block = (HiyokoBlock*)sBattleHiyokoBlock;
	releaseTextures((NJS_TEXLIST*)*block->mTex);
	releaseResource(sBattleModelHiyokoArc);
}

// 0x801987D4
extern "C" TBattleModelHiyoko* createTBattleModelHiyoko(TObject* parent, const Vec3f* pos, const int* angle)
{
	return new TBattleModelHiyoko(parent, pos, (int*)angle);
}

// 0x80198830
TBattleModelHiyoko::TBattleModelHiyoko(TObject* parent, const Vec3f* pos, int* angle)
    : TObject(parent)
    , mPos(*pos)
    , mAngle(*(const BattleAngle*)angle)
{
	_00   = (u32)gTBattleModelHiyokoName;
	mSize = 0xB0;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	setup();
}

// 0x801988E0
TBattleModelHiyoko::~TBattleModelHiyoko()
{
	if (mActor != 0) {
		memFree(mActor);
		mActor = 0;
	}
	removeDrawTask(mDraw);
}

// 0x80198960
void TBattleModelHiyoko::setup()
{
	void* model = *HIYOKO_BLOCK->mModels;
	mActor      = (GjActor*)(memAlloc(gjActorSize((NJS_OBJECT*)model)));
	gjCreateActor((NJS_OBJECT*)model, (u8*)mActor);
	mMotion.init(sBattleModelHiyokoMotions, HIYOKO_BLOCK->mMotions);
	mMotionId = 0;
	mMotion.play(mMotionId);
	stepMotion();
	updateMatrix();
}

// 0x801989EC
void TBattleModelHiyoko::draw()
{
	stepMotion();
	updateMatrix();
}

// 0x80198A20
void TBattleModelHiyoko::vfn_10()
{
}

// 0x80198A24
void TBattleModelHiyoko::render()
{
	if (sBattleHiyokoBlock == 0) {
		return;
	}
	f32 view[12];
	mtxConcat(view, (const f32*)gjGetMatrix(), mMtx);
	njPushMatrix(view);
	njSetTexture(*HIYOKO_BLOCK->mTex);
	if (mActor != 0) {
		gjDrawActor(mActor);
	}
	mtxConcat(view, view, sBattleHiyokoMtx);
	njPushMatrix(view);
	_nj_control_3d_flag_ |= 0x10;
	f32 color[4];
	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 0.0f;
	color[3] = 0.0f;
	njSetConstantMaterial(color);
	gjDrawObject((NJS_OBJECT*)HIYOKO_BLOCK->mModels[1]);
	_nj_control_3d_flag_ &= 0xFFFFFFEF;
	njPopMatrixEx();
	njPopMatrixEx();
}

// 0x80198B00
void TBattleModelHiyoko::updateMatrix()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateZYX(mMtx, mMtx, mAngle.v);
}

// 0x80198B44
void TBattleModelHiyoko::stepMotion()
{
	mMotion.update();
	if (mActor != 0) {
		gjPushUnitMatrix();
		sBattleHiyokoNode = 0;
		gjSetMotionCallback((void (*)(NJS_OBJECT*))hiyokoNodeCallback);
		if (mMotion.isBlending() != 0) {
			gjMotionLinkActor(mActor, mMotion.getBlend(), mMotion.mBlendTime);
		} else {
			int cur   = mMotion.mCurrent;
			f32 frame = mMotion.mFrame;
			gjMotionActor(frame, mActor, mMotion.getMotion(cur));
		}
		gjSetMotionCallback(0);
		njPopMatrixEx();
	}
}

// 0x80198C00
void TBattleModelHiyoko::play(s16 id)
{
	mMotionId = id;
	mMotion.playBlend(id, 0.05f);
}

// 0x80198C2C
// Copies the current matrix when the actor reaches node 0xE
static void hiyokoNodeCallback()
{
	if (sBattleHiyokoNode == 0xE) {
		njGetMatrix(sBattleHiyokoMtx);
	}
	sBattleHiyokoNode += 1;
}

// 0x80198C6C
// Name is ours
extern "C" TBattleModelHiyokoProp* createTBattleModelHiyokoProp(TObject* parent, const Vec3f* pos, const int* angle)
{
	return new TBattleModelHiyokoProp(parent, pos, (int*)angle);
}

// 0x80198CC8
TBattleModelHiyokoProp::TBattleModelHiyokoProp(TObject* parent, const Vec3f* pos, int* angle)
    : TObject(parent)
    , mPos(*pos)
    , mAngle(*(const BattleAngle*)angle)
{
	mScale = 1.0f;
	_00    = (u32)gTBattleModelHiyokoName;
	mSize  = 0x74;
	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	setup();
}

// 0x80198D78
TBattleModelHiyokoProp::~TBattleModelHiyokoProp()
{
	removeDrawTask(mDraw);
}

// 0x80198DE0
void TBattleModelHiyokoProp::setup()
{
	mBlock = sBattleHiyokoBlock;
	updateMatrix();
}

// 0x80198E08
void TBattleModelHiyokoProp::draw()
{
	updateMatrix();
}

// 0x80198E28
void TBattleModelHiyokoProp::vfn_10()
{
}

// 0x80198E2C
void TBattleModelHiyokoProp::render()
{
	if (mBlock != 0) {
		f32 view[12];
		mtxConcat(view, (const f32*)gjGetMatrix(), mMtx);
		njPushMatrix(view);
		HiyokoBlock* block = (HiyokoBlock*)mBlock;
		njSetTexture(*block->mTex);
		njPushMatrix(view);
		gjDrawObject((NJS_OBJECT*)(((HiyokoBlock*)mBlock)->mModels[2]));
		njPopMatrixEx();
		njPopMatrixEx();
	}
}

// 0x80198EAC
void TBattleModelHiyokoProp::updateMatrix()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateZYX(mMtx, mMtx, mAngle.v);
	mtxScale(mScale, mScale, mScale, mMtx, mMtx);
}

typedef char TBattleModelHiyokoProp_size_check[(sizeof(TBattleModelHiyokoProp) == 0x74) ? 1 : -1];
