#include "player/TPlyEffStDrain.h"
#include "engine/DrawTask.h"
#include "item/TItemManage.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "engine/ActorMotion.h"

const char* gTPlyEffStDrainName = "TPlyEffStDrain";
struct Fx;

typedef char TPlyEffStDrain_size_check[(sizeof(TPlyEffStDrain) == 0x108) ? 1 : -1];

// 0x801BC4F4
extern "C" TPlyEffStDrain* createTPlyEffStDrain(TObject* parent)
{
	return new TPlyEffStDrain(parent);
}

// 0x801BC540
TPlyEffStDrain::TPlyEffStDrain(TObject* parent)
    : TPlyEffSt(parent, 0xB)
    , mEffect(0)
    , mTask(0)
    , mHolder()
    , mGlow()
{
	_00   = (u32)gTPlyEffStDrainName;
	mSize = 0x108;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	itemHolderAttach(3, &mHolder);
}

// 0x801BC5EC
TPlyEffStDrain::~TPlyEffStDrain()
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

// 0x801BC6A0
void TPlyEffStDrain::vfn_20()
{
	Vec3f pos;

	pos = *mOwner->getPos();
	mtxSetTransV(mMtx, &pos);
	mtxTranslate((float*)mMtxB, (const float*)mMtx, 0.0f, 20.0f, 0.0f);
	mtxRotateY(mMtxB, mMtxB, mOwner->getRotY());
	mEffect = fxSpawnFollow(1, 0, 0x61, mMtx);
}

// 0x801BC754
void TPlyEffStDrain::vfn_24()
{
	if (mEffect != 0) {
		fxStop(mEffect);
		mEffect = 0;
	}
}

// 0x801BC794
void TPlyEffStDrain::draw()
{
	Vec3f pos;

	pos = *mOwner->getPos();
	mtxSetTransV(mMtx, &pos);
	mtxTranslate((float*)mMtxB, (const float*)mMtx, 0.0f, 20.0f, 0.0f);
	mtxRotateY(mMtxB, mMtxB, mOwner->getRotY());
	mGlow.update(&mHolder);
}

// 0x801BC83C
void TPlyEffStDrain::render()
{
	f32 mtx[12];

	mtxFromView(mtx, mMtxB);
	mGlow.draw(&mHolder, mtx);
}
