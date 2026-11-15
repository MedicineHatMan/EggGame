#include "player/TPlyEffStBlind.h"
#include "engine/DrawTask.h"
#include "item/TItemManage.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "engine/ActorMotion.h"

const char* gTPlyEffStBlindName = "TPlyEffStBlind";
struct Fx;

typedef char TPlyEffStBlind_size_check[(sizeof(TPlyEffStBlind) == 0x108) ? 1 : -1];

// 0x801BC168
extern "C" TPlyEffStBlind* createTPlyEffStBlind(TObject* parent)
{
	return new TPlyEffStBlind(parent);
}

// 0x801BC1B4
TPlyEffStBlind::TPlyEffStBlind(TObject* parent)
    : TPlyEffSt(parent, 0xA)
    , mEffect(0)
    , mTask(0)
    , mHolder()
    , mGlow()
{
	_00   = (u32)gTPlyEffStBlindName;
	mSize = 0x108;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	itemHolderAttach(4, &mHolder);
}

// 0x801BC260
TPlyEffStBlind::~TPlyEffStBlind()
{
	if (mHolder.mNode.mList != 0) {
		itemHolderRelease(&mHolder);
	}
	if (mEffect != 0) {
		fxStop((Fx*)mEffect);
	}
	if (mTask != 0) {
		removeDrawTask(mTask);
	}
}

// 0x801BC314
void TPlyEffStBlind::vfn_20()
{
	Vec3f pos;

	pos = *mOwner->getPos();
	mtxSetTransV(mMtx, &pos);
	mtxTranslate((float*)mMtxB, (const float*)mMtx, 0.0f, 20.0f, 0.0f);
	mtxRotateY(mMtxB, mMtxB, mOwner->getRotY());
	mEffect = fxSpawnFollow(1, 0, 99, mMtx);
}

// 0x801BC3C8
void TPlyEffStBlind::vfn_24()
{
	if (mEffect != 0) {
		fxStop(mEffect);
		mEffect = 0;
	}
}

// 0x801BC408
void TPlyEffStBlind::draw()
{
	Vec3f pos;

	pos = *mOwner->getPos();
	mtxSetTransV(mMtx, &pos);
	mtxTranslate((float*)mMtxB, (const float*)mMtx, 0.0f, 20.0f, 0.0f);
	mtxRotateY(mMtxB, mMtxB, mOwner->getRotY());
	mGlow.update(&mHolder);
}

// 0x801BC4B0
void TPlyEffStBlind::render()
{
	f32 mtx[12];

	mtxFromView(mtx, mMtxB);
	mGlow.draw(&mHolder, mtx);
}
