#define TBATTLEMODELPLAYER_CPP
#include "ui/TBattleModelPlayer.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "core/Mem.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"

MotionEntry sBattleModelPlayerMotions[12] = {
	{ 0, 0, -1, 0, 0.0f, 174.0f, 1.0f, 0.0f, 0 },
	{ 0, 4, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ 1, 8, -1, 0, 0.0f, 141.0f, 1.0f, 0.0f, 0 },
	{ 0, 1, -1, 0, 0.0f, 88.0f, 0.5f, 0.0f, 0 },
	{ 0, 5, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ 1, 9, -1, 0, 0.0f, 141.0f, 1.0f, 0.0f, 0 },
	{ 0, 2, -1, 0, 0.0f, 88.0f, 0.5f, 0.0f, 0 },
	{ 0, 6, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ 1, 10, -1, 0, 0.0f, 141.0f, 1.0f, 0.0f, 0 },
	{ 0, 3, -1, 0, 0.0f, 88.0f, 0.5f, 0.0f, 0 },
	{ 0, 7, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ 1, 11, -1, 0, 0.0f, 141.0f, 1.0f, 0.0f, 0 },
};

const char* gTBattleModelPlayerName = "TBattleModelPlayer";
struct NJS_TEXLIST;
struct GjActor;

extern "C" {
TArc* sBattleModelPlayerArc; // battlemodel_player.arc
void* sBattlePlayerBlock;    // block at arc+0x4C
}

struct PlayerBlock {
	u8 _00[8];      // _00
	void** mModels; // _08
	void* mMotions; // _0C
	void** mTex;    // _10
	void** mGvm;    // _14
};

typedef char TBattleModelPlayer_size_check[(sizeof(TBattleModelPlayer) == 0xB8) ? 1 : -1];

// 0x801AE634
// Name is ours
extern "C" void loadBattleModelPlayer()
{
	loadArc("battlemodel_player.arc", &sBattleModelPlayerArc);
	PlayerBlock* block = (PlayerBlock*)sBattleModelPlayerArc->mData;
	sBattlePlayerBlock = block;
	bindGvmTextures((u8*)*block->mGvm, (NJS_TEXLIST*)*block->mTex);
}

// 0x801AE680
// Name is ours
extern "C" void freeBattleModelPlayer()
{
	PlayerBlock* block = (PlayerBlock*)sBattlePlayerBlock;
	releaseTextures((NJS_TEXLIST*)*block->mTex);
	releaseResource(sBattleModelPlayerArc);
}

// 0x801AE6B4
extern "C" TBattleModelPlayer* createTBattleModelPlayer(TObject* parent, const Vec3f* pos, const int* angle, int kind)
{
	return new TBattleModelPlayer(parent, pos, (int*)angle, kind);
}

// 0x801AE718
TBattleModelPlayer::TBattleModelPlayer(TObject* parent, const Vec3f* pos, int* angle, int kind)
    : TObject(parent)
    , mPos(*pos)
    , mAngle(*(BattleAngle*)angle)
{
	_00   = (u32)gTBattleModelPlayerName;
	mSize = 0xB8;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	setup(kind);
}

// 0x801AE7D0
TBattleModelPlayer::~TBattleModelPlayer()
{
	if (mActor != 0) {
		memFree(mActor);
		mActor = 0;
	}
	removeDrawTask(mDraw);
}

// 0x801AE850
// Each kind owns three MotionEntry rows 0x54 bytes
void TBattleModelPlayer::setup(int kind)
{
	s8 k        = kind;
	mDrop       = 0.0f;
	void* model = ((PlayerBlock*)sBattlePlayerBlock)->mModels[k];
	mActor      = (GjActor*)(memAlloc(gjActorSize((NJS_OBJECT*)model)));
	gjCreateActor((NJS_OBJECT*)model, (u8*)mActor);
	mMotion.init(&sBattleModelPlayerMotions[k * 3], ((PlayerBlock*)sBattlePlayerBlock)->mMotions);
	mMotionId = 0;
	mMotion.play(mMotionId);
	stepMotion();
	mFalling = 0;
	updateMatrix();
}

// 0x801AE8FC
void TBattleModelPlayer::draw()
{
	switch (mFalling) {
	case 0:
		break;
	case 1:
		if (mDrop < 1.5f) {
			mDrop += 0.05f;
		}
		mPos.z -= mDrop;
		mAngle.v[1] = angleApproach(mAngle.v[1], 0x8000, 0x100);
		break;
	case 2:
		break;
	}
	stepMotion();
	updateMatrix();
}

// 0x801AE990
void TBattleModelPlayer::vfn_10()
{
}

// 0x801AE994
void TBattleModelPlayer::render()
{
	if (sBattlePlayerBlock == 0) {
		return;
	}
	f32 view[12];
	mtxConcat(view, (const f32*)gjGetMatrix(), mMtx);
	njPushMatrix(view);
	njSetTexture(*((PlayerBlock*)sBattlePlayerBlock)->mTex);
	if (mActor != 0) {
		gjDrawActor(mActor);
	}
	njPopMatrixEx();
}

// 0x801AEA08
void TBattleModelPlayer::updateMatrix()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateZYX(mMtx, mMtx, mAngle.v);
}

// 0x801AEA4C
void TBattleModelPlayer::stepMotion()
{
	mMotion.update();
	if (mActor != 0) {
		gjPushUnitMatrix();
		if (mMotion.isBlending() != 0) {
			gjMotionLinkActor(mActor, (void*)mMotion.getBlend(), mMotion.mBlendTime);
		} else {
			int cur   = mMotion.mCurrent;
			f32 frame = mMotion.mFrame;
			gjMotionActor(frame, mActor, mMotion.getMotion(cur));
		}
		njPopMatrixEx();
	}
}

// 0x801AEAEC
void TBattleModelPlayer::play(s16 id)
{
	if (mMotionId != id) {
		mMotionId = id;
		mMotion.playBlend(id, 0.05f);
	}
}

// 0x801AEB28
void TBattleModelPlayer::startFall()
{
	play(1);
	mFalling = 1;
}
