#include "item/ItemParts.h"
#include "object/ObjCommon.h"
#include "Ninja/ninja.h"
#include "object/ObjShadow.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "core/Mem.h"
#include "ui/AeArc.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"
#include "engine/TParticle.h"
#include "object/TObject.h"
#include "engine/ActorMotion.h"

struct ItemFx {
	u8 _00[6];
	u16 mSlot; // _06 a slot of ItemNodes
	u8 _08[4];
	ItemFx* mNext; // _0C
	u8 _10[0x14];
	Vec3f mPos; // _24
};

/** What the node walks hand the callback */
struct GlowWalk {
	ItemGlow* mGlow;
	int mIndex;
};

static inline TArc* holderArc(ItemHolder* holder)
{
	return holder->mNode.mList != NULL ? holder->mEntry->mData.mArc : NULL;
}

static inline ItemModelRec* arcModel(TArc* arc)
{
	return (ItemModelRec*)arc->mData;
}

// 0x801CD704
ItemGlow::ItemGlow()
    : mState(0)
    , mActor(NULL)
{
}

// 0x801CD74C
ItemGlow::~ItemGlow()
{
	if (mActor != NULL) {
		memFree(mActor);
	}
}

// 0x801CD7AC
void ItemGlow::stop(ItemHolder* holder)
{
	if (!holder->hasItem()) {
		return;
	}
}

// 0x801CD7B8
void ItemGlow::update(ItemHolder* holder)
{
	switch (mState) {
	case 0:
		bool has = holder->hasItem();
		if (has && (has ? holder->mEntry->mData.mArc : NULL) != NULL) {
			setup(holder);
			mState = 1;
		}
		break;
	case 1:
		tick(holder);
		break;
	}
}

static void glowNode(NJS_OBJECT* node, void* arg);

// 0x801CD848
void ItemGlow::calc(ItemHolder* holder, f32* mtx)
{
	TArc* arc = holderArc(holder);
	GlowWalk walk;

	if (arc != NULL) {
		ItemModelRec* rec = arcModel(arc);
		if (mActor != NULL) {
			walk.mGlow  = this;
			walk.mIndex = 0;
			if (rec->mMotions != NULL) {
				njPushMatrixEx();
				njUnitMatrix(NULL);
				if (mMotion.isBlending() != 0) {
					calcActorLink(mActor, (void*)mMotion.getBlend(), (NodeCallback)glowNode, mMotion.mBlendTime, (void*)&walk);
				} else {
					int cur            = mMotion.mCurrent;
					f32 frame          = mMotion.mFrame;
					NJS_MOTION* motion = mMotion.getMotion(cur);
					calcActorMotion(mActor, motion, (NodeCallback)glowNode, frame, (void*)&walk);
				}
				njPopMatrixEx();
			}
		}
		mNodes.calc(mtx);
	}
}

// 0x801CD950
// One call per node the node's matrix goes to its slot
static void glowNode(NJS_OBJECT* node, void* arg)
{
	GlowWalk* walk = (GlowWalk*)arg;
	ItemGlow* glow = walk->mGlow;
	f32 mtx[12];

	if (glow->mNodes.has(walk->mIndex) != 0) {
		njGetMatrix(mtx);
		glow->mNodes.set(walk->mIndex, mtx);
	}
	walk->mIndex++;
}

// 0x801CD9B4
void ItemGlow::draw(ItemHolder* holder, const f32* mtx)
{
	ItemModelRec* rec;

	int has = holder->hasItem();
	if (has && (has ? holder->mEntry->mData.mArc : NULL) != NULL) {
		rec = arcModel(has ? holder->mEntry->mData.mArc : NULL);
		if (rec->arg != NULL) {
			njSetTexture(rec->arg);
		}
		if (rec->root != NULL) {
			shadowAlphaBegin();
			if (mActor != NULL) {
				njPushMatrix(mtx);
				gjDrawActor(mActor);
				njPopMatrixEx();
			} else if (rec->mMotions != NULL) {
				njPushMatrix(mtx);
				int cur            = mMotion.mCurrent;
				f32 frame          = mMotion.mFrame;
				NJS_MOTION* motion = mMotion.getMotion(cur);
				gjDrawMotion(*(NJS_OBJECT**)rec->root, motion, frame);
				njPopMatrixEx();
			} else {
				njPushMatrix(mtx);
				gjDrawObject(*(NJS_OBJECT**)rec->root);
				njPopMatrixEx();
			}
			shadowAlphaEnd();
		}
	}
}

// 0x801CDADC
void ItemGlow::setup(ItemHolder* holder)
{
	ItemModelRec* rec;
	NJS_OBJECT* node;
	GlowWalk walk;

	int has = holder->hasItem();
	if ((has ? holder->mEntry->mData.mArc : NULL) != NULL) {
		rec = arcModel(has ? holder->mEntry->mData.mArc : NULL);
		mNodes.init(rec);
		if (rec->mTable != NULL) {
			mMotion.init(rec->mTable, rec->mMotions);
			mMotion.play(0);
		}
		if (rec->root != NULL) {
			node        = *(NJS_OBJECT**)rec->root;
			walk.mIndex = 0;
			walk.mGlow  = this;
			if ((node->evalflags & 0x2000) != 0) {
				mActor = createActor(node);
				njPushMatrixEx();
				njUnitMatrix(NULL);
				if (rec->mTable == NULL) {
					calcActorPose(mActor, glowNode, &walk);
				} else {
					int cur            = mMotion.mCurrent;
					f32 frame          = mMotion.mFrame;
					NJS_MOTION* motion = mMotion.getMotion(cur);
					calcActorMotion(mActor, motion, (NodeCallback)glowNode, frame, (void*)&walk);
				}
				njPopMatrixEx();
			} else {
				njPopMatrixEx(); // the game's slip a pop where the push belongs
				njUnitMatrix(NULL);
				walkPoseNodes(node, (NodeCallback)glowNode, (void*)&walk);
				njPopMatrixEx();
			}
		}
	}
}

// 0x801CDC48
void ItemGlow::tick(const ItemHolder* holder)
{
	bool has = holder->hasItem();
	if (has && (has ? holder->mEntry->mData.mArc : NULL) != NULL) {
		mMotion.update();
	}
}

// 0x801CDC9C
void ItemGlow::play(const ItemHolder* holder, int id)
{
	int has = holder->hasItem();
	if (has && (has ? holder->mEntry->mData.mArc : NULL) != NULL) {
		if (id < arcModel(has ? holder->mEntry->mData.mArc : NULL)->mMotionNum) {
			mMotion.play(id);
		}
	}
}

// 0x801CDD1C
ItemNodes::ItemNodes()
{
	mFx      = NULL;
	mOut     = NULL;
	mNode    = NULL;
	mNum     = 0;
	mSlot    = NULL;
	mSlotNum = 0;
}

// 0x801CDD48
ItemNodes::~ItemNodes()
{
	if (mFx != NULL) {
		delete mFx;
	}
	if (mOut != NULL) {
		memFree(mOut);
	}
	if (mNode != NULL) {
		memFree(mNode);
	}
	if (mSlot != NULL) {
		memFree(mSlot);
	}
}

// 0x801CDDEC
void ItemNodes::init(const ItemModelRec* rec)
{
	ItemFxNode* fx;
	int num = rec->mFxNum;
	int max;
	int i;
	int n;
	ItemFx* gen;

	if (num == 0) {
		return;
	}
	fx  = rec->mFx;
	max = 0;
	for (i = 0; i < num; i++) {
		if (max < fx[i].mNode) {
			max = fx[i].mNode;
		}
	}
	mSlot    = (s16*)memAlloc((max + 1) * 2);
	mSlotNum = max + 1;
	for (i = 0; i < max + 1; i++) {
		mSlot[i] = -1;
	}
	n = 0;
	for (i = 0; i < num; i++) {
		if (mSlot[fx[i].mNode] < 0) {
			mSlot[fx[i].mNode] = n++;
		}
	}
	mOut  = (f32*)memAlloc(n * 0x30);
	mNode = (f32*)memAlloc(n * 0x30);
	mNum  = n;
	if (num != 0) {
		mFx = new TObject(NULL);
	}
	for (i = 0; i < num; i++) {
		gen = (ItemFx*)particleGenById(mFx, &gVecZero, 0, fx[i].mId);
		if (gen != NULL) {
			gen->mSlot = mSlot[fx[i].mNode];
		}
	}
}

// 0x801CDF64
int ItemNodes::has(int node)
{
	bool result = false;

	if (node < mSlotNum && mSlot[node] >= 0) {
		result = true;
	}
	return result;
}

// 0x801CDF94
void ItemNodes::set(int node, f32* mtx)
{
	int slot;

	if (node < mSlotNum && (slot = mSlot[node]) >= 0) {
		mtxCopy(&mNode[slot * 12], mtx);
	}
}

// 0x801CDFE4
void ItemNodes::calc(f32* base)
{
	Vec3f pos;
	int i;
	ItemFx* gen;

	for (i = 0; i < mNum; i++) {
		mtxConcat(&mOut[i * 12], base, &mNode[i * 12]);
	}
	if (mFx != NULL) {
		for (gen = (ItemFx*)mFx->mChild; gen != NULL; gen = gen->mNext) {
			mtxGetPos(&mOut[gen->mSlot * 12], &pos);
			gen->mPos = pos;
		}
		objUpdateTree(mFx);
	}
}
