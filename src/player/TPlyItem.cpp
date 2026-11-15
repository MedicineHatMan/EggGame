#include "player/TPlyItem.h"
#include "engine/DrawTask.h"
#include "item/TItemManage.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "engine/ActorMotion.h"

typedef char TPlyItem_size_check[(sizeof(TPlyItem) == 0xA0) ? 1 : -1];

// 0x80189768
TPlyItem::TPlyItem(TObject* parent, int kind, f32* mtx)
    : TObject(parent)
{
	_00   = gTPlyItemName;
	mSize = 0xA0;
	mMtx  = mtx;
	itemHolderAttach(kind, &mHolder);
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
}

// 0x801897F8
TPlyItem::~TPlyItem()
{
	if (mHolder.mNode.mList != 0) {
		itemHolderRelease(&mHolder);
	}
	removeDrawTask(mTask);
}

// 0x80189880
void TPlyItem::draw()
{
	mGlow.update(&mHolder);
	mGlow.calc(&mHolder, mMtx);
}

// 0x801898C4
void TPlyItem::render()
{
	f32 mtx[12];

	mtxFromView(mtx, mMtx);
	mGlow.draw(&mHolder, mtx);
}
