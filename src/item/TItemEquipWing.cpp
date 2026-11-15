#include "item/TItemEquipWing.h"
#include "engine/DrawTask.h"
#include "player/Player.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "animal/AniMount.h"
#include "engine/ActorMotion.h"

typedef char TItemEquipWing_size_check[(sizeof(TItemEquipWing) == 0x10C) ? 1 : -1];

struct WingCtx {
	TItemEquipWing* wing;
	int index;
};

// 0x801AF7FC
extern "C" void onWingNode(void* node, void* arg);

TItemEquipWing::TItemEquipWing(TObject* parent, const ItemSpec* spec)
    : TItemEquipment(parent, spec)
    , mGlow()
{
	_00   = gTItemEquipWingName;
	mSize = 0x10C;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
}

// 0x801AF874
TItemEquipWing::~TItemEquipWing()
{
	removeDrawTask(mDraw);
}

// 0x801AF8F4
void TItemEquipWing::vfn_84()
{
	TChrPlayer* owner = (TChrPlayer*)getOwner();

	owner->vfn_A8(0x38, mMtx);
	mNodes.calc(mMtx);
}

// 0x801AF944
void TItemEquipWing::vfn_10()
{
}

// 0x801AF948
void TItemEquipWing::render()
{
	f32 mtx[12];

	if (mOn != 0) {
		mtxFromView(mtx, mMtx);
		mGlow.draw(&mHolder, mtx);
	}
}

// 0x801AF998
void TItemEquipWing::vfn_7C()
{
	TChrPlayer* owner = (TChrPlayer*)getOwner();
	void* data;
	ItemModelRec* rec;
	WingCtx ctx;

	owner->vfn_A8(0x38, mMtx);
	data = mHolder.mNode.mList != 0 ? mHolder.mEntry->mData.mArc : 0;
	if (data == 0) {
		return;
	}
	rec = *(ItemModelRec**)((u8*)data + 0x4C);
	mNodes.init(rec);
	ctx.wing  = this;
	ctx.index = 0;
	njPushMatrixEx();
	njUnitMatrix(0);
	walkPoseNodes((NJS_OBJECT*)(*(void**)rec->root), (NodeCallback)onWingNode, (void*)&ctx);
	njPopMatrixEx();
}
// 0x801AFA44
// One call per node from walkPoseNodes
extern "C" void onWingNode(void* node, void* arg)
{
	WingCtx* ctx = (WingCtx*)arg;
	f32 mtx[12];

	(void)node;
	if (ctx->wing->mNodes.has(ctx->index) != 0) {
		njGetMatrix(mtx);
		ctx->wing->mNodes.set(ctx->index, mtx);
	}
	ctx->index += 1;
}
