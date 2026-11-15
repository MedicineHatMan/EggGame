#include "animal/AniMount.h"
#include "io/Resource.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "object/TObjEggRing.h"
#include "core/MathUtil.h"
#include "object/TObjLauncher.h"
#include "object/ObjShadow.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "engine/ActorMotion.h"
#include "animal/AniModel.h"
struct DrawStateSlot;
struct GjActor;

// What poseNode walks the nodes with
struct AniPoseCursor {
	AniModel* mModel; // _00
	int mNode;        // _04
};

static void poseNode(void* node, AniPoseCursor* cursor);

// 0x801C0C1C
AniModel::AniModel()
    : mState(0)
    , mNodeSlots(NULL)
    , mNodeCount(0)
    , mMatrices(NULL)
    , mCount(0)
    , mActor(NULL)
{
}

// 0x801C0C48
AniModel::~AniModel()
{
	if (mActor != NULL) {
		memFree(mActor);
	}
	if (mNodeSlots != NULL) {
		memFree(mNodeSlots);
	}
	if (mMatrices != NULL) {
		memFree(mMatrices);
	}
}

// 0x801C0CC8
void AniModel::setNodes(const AniNode* nodes, int count)
{
	int last = 0;
	int i;
	for (i = 0; i < count; i++) {
		if (last < nodes[i].mNode) {
			last = nodes[i].mNode;
		}
	}
	mMatrices  = (f32(*)[12])memAlloc(count * sizeof(f32[12]));
	mNodeSlots = (s16*)memAlloc((last + 1) * sizeof(s16));
	for (i = 0; i < last + 1; i++) {
		mNodeSlots[i] = -1;
	}
	for (i = 0; i < count; i++) {
		mNodeSlots[nodes[i].mNode] = nodes[i].mSlot;
	}
	mNodeCount = last + 1;
	mCount     = count;
}

// 0x801C0DA0
void AniModel::getMatrix(int slot, f32* out)
{
	mtxCopy(out, mMatrices[slot]);
}

// 0x801C0DD0
void AniModel::update(AniAsset* asset)
{
	switch (mState) {
	case 0:
		if (asset->mLink.mList != NULL && asset->mFile->mArc != NULL) {
			build(asset);
			mState = 1;
		}
		break;
	case 1:
		if (asset->mLink.mList == NULL) {
			mState = 2;
		}
		break;
	}
}

// 0x801C0E58
void AniModel::pose(AniAsset* asset, const f32* matrix, MotionPlayer* motion)
{
	if (mActor != NULL) {
		AniPoseCursor cursor;
		cursor.mModel = this;
		cursor.mNode  = 0;
		njPushMatrixEx();
		njUnitMatrix(NULL);
		if (motion->isBlending() != 0) {
			calcActorLink(mActor, motion->getBlend(), (NodeCallback)poseNode, motion->mBlendTime, &cursor);
		} else {
			int cur = motion->mCurrent;
			f32 frame    = motion->mFrame;
			NJS_MOTION* data   = motion->getMotion(cur);
			calcActorMotion(mActor, data, (NodeCallback)poseNode, frame, &cursor);
		}
		njPopMatrixEx();
		for (int i = 0; i < mCount; i++) {
			mtxConcat(mMatrices[i], matrix, mMatrices[i]);
		}
	}
}

// 0x801C0F58
void* AniModel::find(const AniAsset* asset)
{
	return findArcExport(asset->mFile->mArc, "AniModel");
}

// 0x801C0F88
void AniModel::draw(const AniAsset* asset, const f32* matrix, DrawState* state)
{
	if (asset->mLink.mList != NULL && asset->mFile->mArc != NULL) {
		AniModelFile* file = (AniModelFile*)find(asset);
		DrawStateSlot* slot         = getDrawStateSlot();
		shadowAlphaBegin();
		njPushMatrix(matrix);
		beginDrawState(slot, state);
		if (file->mTextures != NULL && file->mTextures->mList != NULL) {
			njSetTexture(file->mTextures->mList);
		}
		gjDrawActor(mActor);
		endDrawState((DrawStateSlot*)slot);
		njPopMatrixEx();
		shadowAlphaEnd();
	}
}

// 0x801C1030
void AniModel::build(AniAsset* asset)
{
	AniModelObject* object = *((AniModelFile*)find(asset))->mObject;
	if (object->mFlags & 0x2000) {
		mActor = (GjActor*)(memAlloc(gjActorSize((NJS_OBJECT*)object)));
		gjCreateActor((NJS_OBJECT*)object, (u8*)mActor);
		njPushMatrixEx();
		njUnitMatrix(NULL);
		gjBuildActorVertices(mActor);
		njPopMatrixEx();
	}
}

// 0x801C10A4
// Called for each node as the model is posed
static void poseNode(void* node, AniPoseCursor* cursor)
{
	AniModel* model = cursor->mModel;
	if (cursor->mNode < model->mNodeCount) {
		int slot = model->mNodeSlots[cursor->mNode];
		if (slot >= 0) {
			njGetMatrix(model->mMatrices[slot]);
		}
	}
	cursor->mNode++;
}
