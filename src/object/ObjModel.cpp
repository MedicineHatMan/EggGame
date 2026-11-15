#include "object/ObjModel.h"
#include "engine/MotionPlayer.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "core/Mem.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
struct GXQuaternion;
struct GjActor;
struct NJS_OBJECT;
struct DrawStateSlot;

typedef char ObjModel_size_check[(sizeof(ObjModel) == 0x20) ? 1 : -1];
typedef char ObjPlace_size_check[(sizeof(ObjPlace) == 0x5C) ? 1 : -1];
typedef char DrawState_size_check[(sizeof(DrawState) == 0x24) ? 1 : -1];

// What objModelNodeCallback writes through set just before a motion runs
#define sCallbackNodes sModelCallbackNodes
#define sCallbackIndex sModelCallbackIndex

/*
 * --INFO--
 * Address:	8005097C
 * Size:	00006C
 */
// 0x8005097C
ObjPlace::ObjPlace()
{
	mFlags   = 0;
	mPos.x   = 0.0f;
	mPos.y   = 0.0f;
	mPos.z   = 0.0f;
	mScale.x = 1.0f;
	mScale.y = 1.0f;
	mScale.z = 1.0f;
	njCreateQuaternion(0, 0.0f, 0.0f, 1.0f, (NJS_QUATERNION*)mRot);
	njUnitMatrix(mMatrix);
}

/*
 * --INFO--
 * Address:	800509E8
 * Size:	0000D0
 */
// 0x800509E8
void ObjPlace::update()
{
	f32 rot[12];
	f32 scale[12];

	f32* mtx = mMatrix;

	PSMTXIdentity(mtx);
	if (mFlags & 1) {
		PSMTXTransApply((const float*)mtx, (float*)mtx, mPos.x, mPos.y, mPos.z);
	}
	if (mFlags & 2) {
		f32 q[4];
		q[3] = mRot[0];
		q[0] = mRot[1];
		q[1] = mRot[2];
		q[2] = mRot[3];
		PSMTXQuat((f32*)rot, (const GXQuaternion*)q);
		PSMTXConcat(mtx, rot, mtx);
	}
	if (mFlags & 4) {
		PSMTXScale(scale, mScale.x, mScale.y, mScale.z);
		PSMTXConcat(mtx, scale, mtx);
	}
}

/*
 * --INFO--
 * Address:	80050AB8
 * Size:	000040
 *
 * The motion callback for an actor: stores each node's matrix in turn.
 */
extern "C" void objModelNodeCallback()
{
	njGetMatrix(sCallbackNodes[sCallbackIndex].mMatrix);
	sCallbackIndex++;
}

/*
 * --INFO--
 * Address:	80050AF8
 * Size:	000024
 *
 * mModel is left alone. setModel always comes next.
 */
ObjModel::ObjModel()
{
	mActor   = 0;
	mState   = 0;
	mMotion  = 0;
	mPlace   = 0;
	mNodes   = 0;
	mTexList = 0;
	mFlags   = 0;
}

/*
 * --INFO--
 * Address:	80050B1C
 * Size:	000048
 */
ObjModel::~ObjModel()
{
	release();
}

/*
 * --INFO--
 * Address:	80050B64
 * Size:	000074
 */
void ObjModel::setModel(void* model)
{
	release();
	mModel = (NJS_OBJECT*)model;
	if (*(u32*)model & 0x2000) {
		mActor = (GjActor*)(memAlloc(gjActorSize((NJS_OBJECT*)model)));
		gjCreateActor((NJS_OBJECT*)model, (u8*)mActor);
		njPushMatrixEx();
		njUnitMatrix(0);
		gjBuildActorVertices(mActor);
		njPopMatrixEx();
	}
}

/*
 * --INFO--
 * Address:	80050BD8
 * Size:	0000AC
 */
void ObjModel::setModelNodes(void* model, int nodes)
{
	release();
	mFlags |= 1;
	mNodes = new ObjPlace[nodes];
	mModel = (NJS_OBJECT*)model;
	if (*(u32*)model & 0x2000) {
		mActor = (GjActor*)(memAlloc(gjActorSize((NJS_OBJECT*)model)));
		gjCreateActor((NJS_OBJECT*)model, (u8*)mActor);
		njPushMatrixEx();
		njUnitMatrix(0);
		gjBuildActorVertices(mActor);
		njPopMatrixEx();
	}
}

/*
 * --INFO--
 * Address:	80050C84
 * Size:	000060
 */
void ObjModel::release()
{
	if (mActor != 0) {
		memFree(mActor);
		mActor = 0;
	}
	if (mNodes != 0) {
		delete[] mNodes;
		mNodes = 0;
	}
}

/*
 * --INFO--
 * Address:	80050CE4
 * Size:	00016C
 *
 * Moves the actor to the motion's current frame, or fills mNodes when there is no actor.
 */
// 0x80050CE4
void ObjModel::calcMotion()
{
	if (mMotion == 0) {
		return;
	}
	if (mActor != 0) {
		if (mFlags & 1) {
			sCallbackNodes = mNodes;
			sCallbackIndex = 0;
			gjSetMotionCallback((void (*)(NJS_OBJECT*))objModelNodeCallback);
		}
		njPushMatrixEx();
		njUnitMatrix(0);
		if (mMotion->isBlending()) {
			f32 time          = mMotion->mBlendTime;
			MotionBlend* link = mMotion->getBlend();
			gjMotionLinkActor(mActor, link, time);
		} else {
			int cur            = mMotion->mCurrent;
			f32 frame          = mMotion->mFrame;
			NJS_MOTION* motion = mMotion->getMotion(cur);
			gjMotionActor(frame, mActor, motion);
		}
		njPopMatrixEx();
		if (mFlags & 1) {
			gjSetMotionCallback(0);
		}
	} else if (mFlags & 1) {
		njPushMatrixEx();
		njUnitMatrix(0);
		if (mMotion->isBlending()) {
			f32 time          = mMotion->mBlendTime;
			MotionBlend* link = mMotion->getBlend();
			njStartMotionLink((const NJS_MOTION_LINK*)link, time);
			objModelLinkNodes(mModel, mNodes, 0);
		} else {
			int cur            = mMotion->mCurrent;
			f32 frame          = mMotion->mFrame;
			NJS_MOTION* motion = mMotion->getMotion(cur);
			njStartMotionObj(motion, frame);
			objModelNodes(mModel, mNodes, 0);
		}
		njPopMatrixEx();
	}
}

/*
 * --INFO--
 * Address:	80050E50
 * Size:	000104
 */
// 0x80050E50
void ObjModel::draw()
{
	if (mModel == 0) {
		return;
	}
	njPushMatrixEx();
	if (mPlace != 0) {
		njMultiMatrix(0, mPlace->mMatrix);
	}
	DrawStateSlot* slot = getDrawStateSlot();
	beginDrawState(slot, mState);
	if (mTexList != 0) {
		njSetTexture(mTexList);
	}
	if (mActor != 0) {
		gjDrawActor(mActor);
	} else if (mMotion != 0) {
		if (mMotion->isBlending()) {
			f32 time          = mMotion->mBlendTime;
			MotionBlend* link = mMotion->getBlend();
			gjDrawMotionLink(mModel, link, time);
		} else {
			int cur            = mMotion->mCurrent;
			f32 frame          = mMotion->mFrame;
			NJS_MOTION* motion = mMotion->getMotion(cur);
			gjDrawMotion(mModel, motion, frame);
		}
	} else {
		gjDrawObject(mModel);
	}
	endDrawState(slot);
	njPopMatrixEx();
}

/*
 * --INFO--
 * Address:	80050F54
 * Size:	000078
 *
 * Walks the model tree: _2C is the first child and _30 the next sibling.
 */
extern "C" int objModelNodes(NJS_OBJECT* node, ObjPlace* nodes, int index)
{
	do {
		njMotionTransformEx(node);
		njGetMatrix(nodes[index].mMatrix);
		index++;
		NJS_OBJECT* child = node->child;
		if (child != 0) {
			index = objModelNodes(child, nodes, index);
		}
		node = node->sibling;
	} while (node != 0);
	return index;
}

/*
 * --INFO--
 * Address:	80050FCC
 * Size:	000078
 */
extern "C" int objModelLinkNodes(NJS_OBJECT* node, ObjPlace* nodes, int index)
{
	do {
		njMotionLinkTransformEx(node);
		njGetMatrix(nodes[index].mMatrix);
		index++;
		NJS_OBJECT* child = node->child;
		if (child != 0) {
			index = objModelLinkNodes(child, nodes, index);
		}
		node = node->sibling;
	} while (node != 0);
	return index;
}
