#include "enemy/boss/TEneBoss03Model.h"
#include "egg/ObjEgg.h"
#include "enemy/boss/TEneBoss03Mole.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "stl/math.h"
#include "animal/RacerMover.h"
#include "player/Player.h"
#include "enemy/Enemy.h"
#include "Ninja/ninja.h"

extern "C" {
extern const int gBoss03DinoNodes[] = {
	0, 7, 17, 28, 39, 72, 77, 78, 114, 115, 119, 120, 121, -1,
}; // the nodes to find in the model ending in -1
}

const char* gTEneBoss03DinoName = "TEneBoss03Dino";

extern "C" {

NJS_OBJECT** gBoss03Models;
NJS_TEXLIST** gBoss03Textures;
NJS_MOTION** gBoss03Motions;
f32 gDinoRingRadius;
int gDinoRingAngle;
ModelNodeFn sNodeCallback;
int sNodeIndex;
int sListPos;
const int* sNodeList;
Vec3f* sPoints;
MotionLinkHooks* sHooks;
int sDirNode;
Vec3f* sDirs;
int sNodeCount;
s8* sNodeMask;
s8* sNodeDepth;
Vec3f gDinoNode9Rest;
Vec3f gDinoHeadRest;
}

// 0x8007D664
void countNodes(NJS_OBJECT* obj, int* count)
{
	do {
		(*count)++;
		if (!(obj->evalflags & 0x10)) {
			countNodes(obj->child, count);
		}
		obj = obj->sibling;
	} while (obj != 0);
}

// 0x8007D6C0
void fillNodeDepths(NJS_OBJECT* obj, s8* out, int* index, int* depth)
{
	do {
		out[(*index)++] = *depth;
		if (!(obj->evalflags & 0x10)) {
			(*depth)++;
			fillNodeDepths(obj->child, out, index, depth);
			(*depth)--;
		}
		obj = obj->sibling;
	} while (obj != 0);
}

// 0x8007D74C
void getNodeDepths(NJS_OBJECT* obj, s8* out)
{
	int index = 0;
	int depth = 0;
	fillNodeDepths(obj, out, &index, &depth);
}

// 0x8007D780
// Marks each wanted node and the nodes above it 1 the rest 0 and everything past the last one 2
void buildNodeMask(int count, const s8* depth, s8* mask, const int* nodes)
{
	int next  = *nodes++;
	int start = 0;
	int i;
	for (i = 0; i < count; i++) {
		if (i == next) {
			int d = depth[i];
			for (int j = i; j >= start; j--) {
				if (depth[j] <= d) {
					mask[j] = 1;
					d--;
				} else {
					mask[j] = 0;
				}
			}
			next  = *nodes++;
			start = i + 1;
			if (next < 0) {
				break;
			}
		}
	}
	for (i = start; i < count; i++) {
		mask[i] = 2;
	}
}

// 0x8007D85C
void motionHooksInit(MotionLinkHooks* hooks)
{
	hooks->mTransform = 0;
	hooks->mNextNode  = 0;
}

// 0x8007D86C
void motionHooksStart(MotionLinkHooks* hooks, void* motion, f32 frame)
{
	njStartMotionObj(motion, frame);
	hooks->mTransform = njMotionTransformEx;
	hooks->mNextNode  = njSetNextMotionNodeEx;
}

// 0x8007D8B4
void motionHooksStartLink(MotionLinkHooks* hooks, void* motion, f32 rate)
{
	njStartMotionLink((const NJS_MOTION_LINK*)motion, rate);
	hooks->mTransform = njMotionLinkTransformEx;
	hooks->mNextNode  = njSetNextMotionLinkNode;
}

// 0x8007D8FC
void motionHooksTransform(MotionLinkHooks* hooks, NJS_OBJECT* obj)
{
	hooks->mTransform(obj);
}

// 0x8007D928
void motionHooksNextNode(MotionLinkHooks* hooks)
{
	hooks->mNextNode();
}

// 0x8007D950
// Runs sNodeCallback on each node depth first until it returns 0
int walkNodes(NJS_OBJECT* obj)
{
	int ok;
	do {
		njPushMatrixEx();
		ok = sNodeCallback(obj);
		if (ok != 0 && obj->child != 0) {
			ok = walkNodes(obj->child);
		}
		obj = obj->sibling;
		njPopMatrixEx();
	} while (ok != 0 && obj != 0);
	return ok;
}

// 0x8007D9C4
void walkModel(NJS_OBJECT* obj, ModelNodeFn callback)
{
	sNodeCallback = callback;
	walkNodes(obj);
}

// 0x8007D9E8
// points gets one position per entry of nodes
void setNodeTargets(const int* nodes, Vec3f* points, MotionLinkHooks* hooks, int dirNode, Vec3f* dirs)
{
	sNodeList  = nodes;
	sNodeIndex = 0;
	sListPos   = 0;
	sPoints    = points;
	sHooks     = hooks;
	sDirNode   = dirNode;
	sDirs      = dirs;
}

// 0x8007DA0C
void poseNode(NJS_OBJECT* obj)
{
	if (sNodeMask[sNodeIndex] == 1) {
		motionHooksTransform(sHooks, obj);
	} else {
		motionHooksNextNode(sHooks);
	}
}

// 0x8007DA58
int recordNode()
{
	if (sNodeIndex == sNodeList[sListPos]) {
		njCalcPointEx(sPoints, sPoints);
		sListPos++;
		sPoints++;
		return 1;
	}
	return 0;
}

// 0x8007DAC0
int nodesDone()
{
	return sNodeList[sListPos] < 0;
}

// 0x8007DAD8
int findNodeStep(NJS_OBJECT* obj)
{
	poseNode(obj);
	recordNode();
	sNodeIndex++;
	return !nodesDone();
}

// 0x8007DB14
int findNodeDirStep(NJS_OBJECT* obj)
{
	poseNode(obj);
	if (recordNode() != 0 && sNodeIndex == sDirNode) {
		njCalcVectorEx(&gAxisZ, sDirs++);
		njCalcVectorEx(&gVecUp, sDirs);
	}
	sNodeIndex++;
	return !nodesDone();
}

// 0x8007DB90
void initBoss03Model()
{
	Vec3f away;
	Vec3f ring;
	Vec3f head;
	Vec3f node9;
	MotionLinkHooks hooks;

	gBoss03Models   = modelTEneBoss03Mole();
	gBoss03Textures = shapeTEneBoss03Mole();
	gBoss03Motions  = motionTEneBoss03Mole(1);
	sNodeCallback   = 0;
	sNodeIndex      = 0;
	sListPos        = 0;
	sNodeList       = 0;
	sPoints         = 0;
	sHooks          = 0;
	sDirNode        = 0;
	sDirs           = 0;

	NJS_OBJECT* root = gBoss03Models[0];
	sNodeCount       = 0;
	countNodes(root, &sNodeCount);
	sNodeMask  = new s8[sNodeCount];
	sNodeDepth = new s8[sNodeCount];
	getNodeDepths(root, sNodeDepth);
	buildNodeMask(sNodeCount, sNodeDepth, sNodeMask, gBoss03DinoNodes);

	HitVec points[13];

	motionHooksStart(&hooks, gBoss03Motions[1], 0.0f);
	for (int i = 0; i < 13; i++) {
		points[i].x = 0.0f;
		points[i].y = 0.0f;
		points[i].z = 0.0f;
	}
	setNodeTargets(gBoss03DinoNodes, points, &hooks, -1, 0);
	gjPushUnitMatrix();
	walkModel(gBoss03Models[0], findNodeStep);
	njPopMatrixEx();

	gDinoNode9Rest = points[9];
	gDinoHeadRest  = points[8];
	away           = gDinoNode9Rest;
	head           = gDinoHeadRest;
	njSubVector(&away, &head);
	away.y = 0.0f;
	if (njUnitVector(&away) < gUnitVectorMin) {
		away = gVecZero;
	}
	node9 = gDinoNode9Rest;
	gjMulAddVector(90.0f, &ring, &node9, &away);
	ring.y          = 0.0f;
	gDinoRingRadius = njScalor(&ring);
	gDinoRingAngle  = (int)(10430.38043493439 * (f32)atan2(ring.x, ring.z)) + 0x4000;
}

// 0x8007DDE8
void freeBoss03Model()
{
	if (sNodeDepth != 0) {
		delete[] sNodeDepth;
		sNodeDepth = 0;
	}
	if (sNodeMask != 0) {
		delete[] sNodeMask;
		sNodeMask = 0;
	}
	gBoss03Models   = 0;
	gBoss03Textures = 0;
	gBoss03Motions  = 0;
}

// 0x8007DE44
f32 getDinoRingRadius()
{
	return gDinoRingRadius;
}

// 0x8007DE4C
int getDinoRingAngle()
{
	return gDinoRingAngle;
}

#include "enemy/boss/EneBoss.h"
#include "engine/DrawTask.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/TEneAm02.h"
#include "enemy/boss/TEneBoss03Model.h"
#include "enemy/boss/TEneBoss03Dino.h"
#include "enemy/boss/TEneBoss03_Magma.h"
#include "egg/EggCreature.h"
#include "enemy/ObjEnemy.h"
#include "Ninja/ninja.h"
#include "object/ObjAction.h"
#include "engine/Colli.h"
#include "stl/math.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "engine/TParticle.h"
#include "enemy/EnemyFx.h"
struct ColliShape;
struct CharaColli;
struct NJS_OBJECT;

extern "C" {
MotionEntry sBoss03DinoMotions[] = {
	{ MOTION_LOOP, 11, -1, 0, 0.0f, 30.0f, 0.25f, 0.0f, 0 }, { MOTION_LOOP, 12, -1, 0, 0.0f, 50.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 10, -1, 0, 0.0f, 30.0f, 0.4f, 0.0f, 0 },  { MOTION_ONCE, 9, -1, 0, 0.0f, 49.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 49.0f, 0.5f, 0.0f, 0 },   { MOTION_ONCE, 7, -1, 0, 0.0f, 49.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 59.0f, 0.3f, 0.0f, 0 },   { MOTION_ONCE, 3, -1, 0, 0.0f, 59.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 29.0f, 0.2f, 0.0f, 0 },   { MOTION_LOOP, 1, -1, 0, 0.0f, 20.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },   { MOTION_ONCE, 5, -1, 0, 0.0f, 419.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 539.0f, 1.0f, 0.0f, 0 },
};
extern const ColliShape sBoss03DinoShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 60.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

// 0x8007DE54
TEneBoss03Dino::TEneBoss03Dino(TObject* parent, void* owner)
    : TObjEnemy(parent)
{
	_00   = (u32)gTEneBoss03DinoName;
	mSize = 0x260;
	motionCtrlInit(&mAnim, sBoss03DinoMotions, gBoss03Motions);
	motionCtrlPlay(&mAnim, 0);
	mOwner = owner;
	setupCollider();
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mDirs[0].x = 0.0f;
	mDirs[0].y = 0.0f;
	mDirs[0].z = 1.0f;
	mDirs[1].x = 0.0f;
	mDirs[1].y = 1.0f;
	mDirs[1].z = 0.0f;
	motionHooksInit(&mPose);
	mRingRadius  = getDinoRingRadius();
	mLastStomp.x = 0.0f;
	mLastStomp.y = 0.0f;
	mLastStomp.z = 0.0f;
	mMotionRate  = 1.0f;
	mStateFlags  = 0;
	for (int i = 0; i < 13; i++) {
		mNodes[i].x = 0.0f;
		mNodes[i].y = 0.0f;
		mNodes[i].z = 0.0f;
	}
	for (int i = 0; i < 2; i++) {
		mDirs[i].x = 0.0f;
		mDirs[i].y = 0.0f;
		mDirs[i].z = 0.0f;
	}
	for (int i = 0; i < 4; i++) {
		mFx[i] = 0;
	}
	mStep      = 0;
	mTimer     = 0;
	mTurn      = 0;
	mTurnTo    = 0;
	mStepSpeed = 0.0f;
	mDrift     = 0.0f;
	setCantAct();
}

// 0x8007E014
void TEneBoss03Dino::setupCollider()
{
	mCollider = (CharaColli*)colliCreate(1, 1);
	colliCopyShapes(mCollider, sBoss03DinoShape);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0xFF, 0, 6);
	colliShapeSetTag(mCollider, 0, 0);
	colliDisable(mCollider);
}

// 0x8007E098
TEneBoss03Dino::~TEneBoss03Dino()
{
	killFx();
	removeDrawTask(mDrawTask);
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x8007E114
void TEneBoss03Dino::setPos(const Vec3f* pos)
{
	f32 x        = pos->x;
	f32 y        = pos->y;
	mBody.mPos.x = x;
	f32 z        = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x8007E158
f32 TEneBoss03Dino::vfn_40()
{
	return 50.0f;
}

// 0x8007E160
void TEneBoss03Dino::syncCollider()
{
	if (colliIsDisabled(mCollider) == 0) {
		ColliShape* shapes = colliGetShapes(mCollider);
		Vec3f at;
		at.x = mNodes[NODE_MOUTH].x;
		at.y = mNodes[NODE_MOUTH].y;
		at.z = mNodes[NODE_MOUTH].z;
		njSubVector(&at, &mNodes[NODE_NECK]);
		at.y = 0.0f;
		if (njUnitVector(&at) < gUnitVectorMin) {
			at = gVecZero;
		}
		gjScaleVector(10.0f, &at, &at);
		njAddVector(&at, &mNodes[NODE_NECK]);
		colliShapeSetPos(shapes, &at);
	}
}

// 0x8007E228
void TEneBoss03Dino::colliderOn()
{
	colliEnable(mCollider);
	colliShapeSetBit8(mCollider, 0);
}

// 0x8007E264
void TEneBoss03Dino::colliderNoTouch()
{
	colliShapeClearBit8(mCollider, 0);
}

// 0x8007E28C
void TEneBoss03Dino::colliderOff()
{
	colliDisable(mCollider);
}

// 0x8007E2B0
void TEneBoss03Dino::showFx(int slot, int node, int kind)
{
	EnemyFx* fx = mFx[slot];
	if (fx == 0) {
		mFx[slot] = (EnemyFx*)particleGenByIdDefault(&mNodes[node], 1, kind);
	} else {
		fx->mPos.x = mNodes[node].x;
		fx->mPos.y = mNodes[node].y;
		fx->mPos.z = mNodes[node].z;
	}
}

// 0x8007E32C
void TEneBoss03Dino::updateFx()
{
	if ((mStateFlags & 0x20) != 0) {
		killFx();
	} else {
		showFx(0, 5, 0x44);
		showFx(1, 6, 0x44);
		showFx(2, 7, 0x44);
		showFx(3, 0xB, 0x45);
	}
}

// 0x8007E3B4
void TEneBoss03Dino::killFx()
{
	for (int i = 3; i >= 0; i--) {
		if (mFx[i] != 0) {
			mFx[i]->mFlags |= 1;
			mFx[i] = 0;
		}
	}
}

// 0x8007E3F0
void TEneBoss03Dino::puffAtFeet()
{
	Vec3f at;
	at.x = mNodes[0].x;
	at.y = mNodes[0].y;
	at.z = mNodes[0].z;
	at.y = 180.0f;
	particleOnceByIdDefault(&at, 1, 0x41);
	particleOnceByIdDefault(&at, 1, 0x51);
}

// 0x8007E44C
int TEneBoss03Dino::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int took     = 0;
	if (isPlayerObj(other) != 0) {
		hit->mFlags.clear();
		took         = 1;
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 0x14;
	} else if (isEggObj(other) != 0) {
		took = 1;
		hit->mFlags.clear();
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 1;
		hit->mFlags.setBit(0xD);
	}
	return took;
}

// 0x8007E4F8
void TEneBoss03Dino::getHeadPos(Vec3f* out) const
{
	*out = mNodes[NODE_HEAD];
}

// 0x8007E514
// The head's place and the rotation that faces along mDirs[0]
void TEneBoss03Dino::getAim(Vec3f* pos, int* rot)
{
	f32 x   = mDirs[0].x;
	f32 z   = mDirs[0].z;
	f32 ang = (f32)atan2(x, z);
	x *= x;
	z *= z;
	z         = z + x;
	f32 flat  = __fres(__frsqrte(z));
	int yaw   = (int)(10430.38043493439 * ang);
	int pitch = (int)(10430.38043493439 * (f32)atan2(-mDirs[0].y, flat));
	gjPushUnitMatrix();
	gjRotateX(-pitch);
	gjRotateY(-yaw);
	Vec3f up;
	njCalcVectorEx(&mDirs[1], &up);
	atan2(up.y, -up.x);
	njPopMatrixEx();
	getHeadPos(pos);
	rot[0] = pitch;
	rot[1] = yaw;
	rot[2] = 0;
}

// 0x8007E61C
void TEneBoss03Dino::setRingRadius(f32 radius)
{
	mRingRadius = radius;
}

// 0x8007E624
void TEneBoss03Dino::getNode0Pos(Vec3f* out)
{
	gjPushUnitMatrix();
	applyMatrix();
	njCalcPointEx(&gDinoHeadRest, out);
	njPopMatrixEx();
}

// 0x8007E670
void TEneBoss03Dino::getMouthPos(Vec3f* out) const
{
	*out = mNodes[NODE_MOUTH];
}

// 0x8007E68C
void TEneBoss03Dino::setMotionRate(f32 rate)
{
	mMotionRate = rate;
	motionCtrlSetSpeed(&mAnim, rate);
}

// 0x8007E6B4
int TEneBoss03Dino::isNeckDown()
{
	f32 frame = motionCtrlFrame(&mAnim);
	u8 yes    = 0;
	if (405.0f <= frame && frame < 460.0f) {
		yes = 1;
	}
	return yes;
}

// 0x8007E700
int TEneBoss03Dino::atRing()
{
	return mStateFlags & 1;
}

// 0x8007E70C
int TEneBoss03Dino::isFaceInDone()
{
	return isActionDone(ACT_FACE_IN);
}

// 0x8007E730
int TEneBoss03Dino::isFaceSideDone()
{
	return isActionDone(ACT_FACE_SIDE);
}

// 0x8007E754
int TEneBoss03Dino::isRoarDone()
{
	return isActionDone(ACT_ROAR);
}

// 0x8007E778
int TEneBoss03Dino::roarPeaked()
{
	return mStateFlags & 4;
}

// 0x8007E784
int TEneBoss03Dino::isBreathing()
{
	return mStateFlags & 2;
}

// 0x8007E790
int TEneBoss03Dino::isBreathEndDone()
{
	return isActionDone(ACT_BREATH_END);
}

// 0x8007E7B4
int TEneBoss03Dino::isSwipeDone()
{
	return isActionDone(ACT_SWIPE);
}

// 0x8007E7D8
int TEneBoss03Dino::swipeHits()
{
	return mStateFlags & 8;
}

// 0x8007E7E4
int TEneBoss03Dino::biteHits()
{
	return mStateFlags & 0x10;
}

// 0x8007E7F0
int TEneBoss03Dino::isBiteDone()
{
	return isActionDone(ACT_BITE);
}

// 0x8007E814
void TEneBoss03Dino::updateBody()
{
	clearHitFlags();
	if (takeHits(&mHit, &mContact) != 0) {
		enemyFxSparks(3.0f, &mContact.mSurface, &mContact.mCenter);
		seStartAtFar(0x11E, (u8*)this + 0x12C);
	}
	onActionPhase((s16)mAction, 1);
	syncCollider();
	updateFx();
}

// 0x8007E8AC
void TEneBoss03Dino::vfn_6C()
{
	f32 frame;
	NJS_MOTION* motion = firstMotionFrame(gBoss03Motions, 0xD, &frame);
	poseAt(motion, frame);
	placeNodes();
}

// 0x8007E8FC
int TEneBoss03Dino::getHitKind(const void* touch, HitInfo* info)
{
	return 1;
}

// 0x8007E904
int TEneBoss03Dino::angleToCentre()
{
	return (int)(10430.38043493439 * (f32)atan2(-mBody.mPos.x, -mBody.mPos.z));
}

// 0x8007E94C
int TEneBoss03Dino::angleSideways()
{
	return angleToCentre() + 0x4000;
}

// 0x8007E970
// Finds the nodes in the model as it is posed now each offset from its bone by what is set here
void TEneBoss03Dino::findNodes()
{
	for (int i = 0; i < 13; i++) {
		mNodes[i].x = 0.0f;
		mNodes[i].y = 0.0f;
		mNodes[i].z = 0.0f;
	}
	mNodes[5].y  = -45.0f;
	mNodes[6].y  = -38.0f;
	mNodes[7].y  = -35.0f;
	mNodes[11].y = -15.0f;
	mNodes[11].z = 32.0f;
	setNodeTargets(gBoss03DinoNodes, mNodes, &mPose, 0x72, mDirs);
	gjPushUnitMatrix();
	walkModel(*gBoss03Models, findNodeDirStep);
	njPopMatrixEx();
}

// 0x8007EA18
void TEneBoss03Dino::pose()
{
	if (motionCtrlIsBlending(&mAnim) != 0) {
		f32 rate = motionCtrlBlendTime(&mAnim);
		motionHooksStartLink(&mPose, motionCtrlGetBlend(&mAnim), rate);
	} else {
		f32 frame = motionCtrlFrame(&mAnim);
		motionHooksStart(&mPose, motionCtrlGetMotion(&mAnim), frame);
	}
	findNodes();
}

// 0x8007EAAC
void TEneBoss03Dino::poseAt(void* motion, f32 frame)
{
	motionHooksStart(&mPose, motion, frame);
	findNodes();
}

// 0x8007EAE4
void TEneBoss03Dino::placeNodes()
{
	gjPushUnitMatrix();
	applyMatrix();
	njCalcPoints(0, mNodes, mNodes, 0xD);
	njCalcVectors(0, mDirs, mDirs, 2);
	njPopMatrixEx();
}

// 0x8007EB44
// A stomp shakes the pool but not twice within 50
void TEneBoss03Dino::stomp(int node)
{
	Vec3f at;
	at.x = mNodes[node].x;
	at.y = mNodes[node].y;
	at.z = mNodes[node].z;
	Vec3f delta;
	delta.x = mLastStomp.x;
	delta.y = mLastStomp.y;
	delta.z = mLastStomp.z;
	njSubVector(&delta, &at);
	if (vecLenXZ(&delta) >= 50.0f) {
		mLastStomp.x = at.x;
		mLastStomp.y = at.y;
		mLastStomp.z = at.z;
		createMagmaPow(&mLastStomp, 2.0f, 280.0f, -0.2f);
	}
}

// 0x8007EBF4
void TEneBoss03Dino::quake()
{
	createMagmaPow(&mNodes[0], 2.0f, 500.0f, 0.6f);
}

// 0x8007EC24
void TEneBoss03Dino::bigQuake()
{
	createMagmaPow(&mNodes[0], 3.0f, 800.0f, -2.3f);
}

// 0x8007EC54
void TEneBoss03Dino::onActionPhase(int action, int phase)
{
	switch (action) {
	case ACT_RISE:
		actRise(phase);
		break;
	case ACT_IDLE:
		actIdle(phase);
		break;
	case ACT_WALK:
		actWalk(phase);
		break;
	case ACT_FACE_IN:
		actFaceIn(phase);
		break;
	case ACT_FACE_SIDE:
		actFaceSide(phase);
		break;
	case ACT_ROAR:
		actRoar(phase);
		break;
	case ACT_BREATH:
		actBreath(phase);
		break;
	case ACT_BREATH_END:
		actBreathEnd(phase);
		break;
	case ACT_SWIPE:
		actSwipe(phase);
		break;
	case ACT_BITE:
		actBite(phase);
		break;
	case ACT_DEAD:
		actDead(phase);
		break;
	}
}

// 0x8007ED10
void TEneBoss03Dino::actRise(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xC);
		mBody.mPos.x = 0.0f;
		mBody.mPos.y = 0.0f;
		mBody.mPos.z = 0.0f;
		mBody.mRot.y = 0;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		waitMotion();
		pose();
		placeNodes();
		f32 frame = motionCtrlFrame(&mAnim);
		switch (mStep) {
		case 0:
			if (frame >= 250.0f) {
				mStep = 1;
				puffAtFeet();
				quake();
			}
			break;
		case 1:
			if (frame >= 300.0f) {
				mStep = 2;
				puffAtFeet();
				bigQuake();
			}
			break;
		}
		break;
	}
	case 2:
		mBody.mPos.x = mNodes[0].x;
		mBody.mPos.y = mNodes[0].y;
		mBody.mPos.z = mNodes[0].z;
		mBody.mRot.y = (int)(10430.38043493439 * (f32)atan2(-mBody.mPos.x, -mBody.mPos.z));
		motionCtrlPlay(&mAnim, 0);
		motionCtrlUpdate(&mAnim);
		break;
	}
}

// 0x8007EE7C
void TEneBoss03Dino::actIdle(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		waitMotion();
		pose();
		placeNodes();
		break;
	case 2:
		break;
	}
}

// 0x8007EEF4
void TEneBoss03Dino::actWalk(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 2);
		mBody.mPos.y = 70.0f;
		mStateFlags &= ~1;
		mStepSpeed = 0.0f;
		mDrift     = 0.0f;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		f32 frame;
		if (motionCtrlIsBlending(&mAnim) != 0) {
			frame = 0.0f;
		} else {
			frame = motionCtrlFrame(&mAnim);
		}
		Vec3f now;
		Vec3f move;
		if (frame < 3.0f || (12.0f < frame && frame < 18.0f) || 27.0f < frame) {
			njSinCos(mBody.mRot.y, &move.x, &move.z);
			move.x *= mStepSpeed;
			move.y = 0.0f;
			move.z *= mStepSpeed;
			pose();
			mDrift = mStepSpeed;
		} else {
			int foot = frame < 15.0f ? 2 : 4;
			move.x   = mNodes[foot].x;
			move.y   = mNodes[foot].y;
			move.z   = mNodes[foot].z;
			pose();
			gjPushUnitMatrix();
			applyMatrix();
			njCalcPointEx(&mNodes[foot], &now);
			njPopMatrixEx();
			njSubVector(&move, &now);
			move.y  = 0.0f;
			f32 len = njUnitVector(&move);
			if (len < gUnitVectorMin) {
				move = gVecZero;
			}
			mStepSpeed = len;
			mDrift *= 0.8f;
			gjScaleVector(mStepSpeed + mDrift, &move, &move);
		}
		njAddVector(&mBody.mPos, &move);
		getDinoRingRadius();
		f32 len;
		f32 x;
		f32 z;
		z   = mBody.mPos.z;
		x   = mBody.mPos.x;
		len = __fres(__frsqrte(x * x + z * z));
		if (motionCtrlIsBlending(&mAnim) == 0) {
			f32 want  = mRingRadius;
			f32 limit = 1.5f * mMotionRate;
			f32 off   = len - want;
			if (off > limit) {
				want = len - limit;
				mStateFlags &= ~1;
			} else if (off < -limit) {
				want = len + limit;
				mStateFlags &= ~1;
			} else {
				mStateFlags |= 1;
			}
			x *= want / len;
			z *= want / len;
		}
		mBody.mPos.x = x;
		mBody.mPos.z = z;
		mBody.mRot.y = (int)(10430.38043493439 * (f32)atan2(x, z)) - 0x4000;
		placeNodes();
		if (motionCtrlPassed(3.0f, &mAnim) != 0) {
			seStartAtRange(0x11F, &mNodes[NODE_HIP], 0x78, 1400.0f);
		} else if (motionCtrlPassed(18.0f, &mAnim) != 0) {
			seStartAtRange(0x11F, &mNodes[NODE_HIP], 0x78, 1400.0f);
		}
		if (mNodes[NODE_FOOT_R].y > mNodes[NODE_FOOT_L].y) {
			stomp(NODE_FOOT_R);
		} else {
			stomp(NODE_FOOT_L);
		}
		break;
	}
	case 2:
		mStateFlags &= ~1;
		break;
	}
}

// 0x8007F284
void TEneBoss03Dino::turn(int delta, int final)
{
	if (motionCtrlIsBlending(&mAnim) == 0) {
		f32 frame = motionCtrlFrame(&mAnim);
		if (22.0f < frame && frame < 32.0f) {
			mBody.mRot.y += delta;
		} else if (frame >= 32.0f) {
			mBody.mRot.y = final;
		}
	}
	waitMotion();
}

// 0x8007F310
void TEneBoss03Dino::actFaceIn(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 3);
		mTurn = (int)((f32)angleDiff(mBody.mRot.y, mTurnTo = angleToCentre()) / 20.0f);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		turn(mTurn, mTurnTo);
		pose();
		placeNodes();
		stomp(NODE_FOOT_R);
		if (motionCtrlPassed(32.0f, &mAnim) != 0) {
			seStartAtRange(0x11F, &mNodes[NODE_HIP], 0x78, 1400.0f);
		}
		break;
	case 2:
		break;
	}
}

// 0x8007F410
void TEneBoss03Dino::actFaceSide(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 4);
		mTurn = (int)((f32)angleDiff(mBody.mRot.y, mTurnTo = angleSideways()) / 20.0f);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		turn(mTurn, mTurnTo);
		pose();
		placeNodes();
		stomp(NODE_FOOT_L);
		if (motionCtrlPassed(32.0f, &mAnim) != 0) {
			seStartAtRange(0x11F, &mNodes[NODE_HIP], 0x78, 1400.0f);
		}
		break;
	case 2:
		break;
	}
}

// 0x8007F510
void TEneBoss03Dino::actRoar(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 5);
		mStateFlags &= ~4;
		mStep = 0;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		waitMotion();
		if (motionCtrlPassed(26.0f, &mAnim) != 0) {
			mStateFlags |= 4;
		}
		f32 oldY = mNodes[NODE_HIP].y;
		pose();
		placeNodes();
		f32 rose;
		f32 y = mNodes[NODE_HIP].y;
		rose  = y - oldY;
		switch (mStep) {
		case 0:
			if (y > 160.0f) {
				mStep = 1;
				quake();
			}
			break;
		case 1:
			if (rose <= 0.0f) {
				mStep = 2;
			}
			break;
		case 2:
			if (y < 210.0f) {
				mStep = 3;
				puffAtFeet();
				bigQuake();
			}
			break;
		case 3:
			if (y < 182.0f) {
				mStep = 4;
				seStartAtRange(0x11F, &mBody.mPos, 0x78, 1400.0f);
			}
			break;
		}
		break;
	}
	case 2:
		mStateFlags &= ~4;
		break;
	}
}

// 0x8007F6A8
void TEneBoss03Dino::actBreath(int phase)
{
	switch (phase) {
	case 0:
		mStep  = 0;
		mTimer = 0x2A;
		mStateFlags &= ~0x22;
		motionCtrlPlayBlend(0.1f, &mAnim, 0);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (mStep == 0) {
			mTimer--;
			if (mTimer <= 0) {
				motionCtrlPlayBlend(0.1f, &mAnim, 8);
				int side = angleSideways();
				mTurn    = side - getDinoRingAngle();
				colliderOn();
				mStep = 1;
			}
		} else {
			if (mStep == 1) {
				if (motionCtrlFrame(&mAnim) >= 15.0f) {
					mStateFlags |= 0x22;
					mStep = 2;
					particleOnceByIdDefault(&mNodes[NODE_MOUTH], 1, 0x4E);
					particleOnceByIdDefault(&mNodes[NODE_MOUTH], 1, 0x4D);
					seStartAtFar(0x121, &mNodes[NODE_MOUTH]);
				}
			} else if (mStep == 2) {
				if (motionCtrlIsDone(&mAnim) != 0) {
					motionCtrlPlay(&mAnim, 9);
					mStep  = 3;
					mTimer = 0;
					colliderNoTouch();
				}
			} else if (mStep == 3) {
				mTimer--;
				if (mTimer <= 0) {
					mTimer = 0x708;
					colliDisable(mCollider);
				} else {
					colliEnable(mCollider);
				}
			}
			mBody.mRot.y = angleApproach(mBody.mRot.y, mTurn, 0x100);
		}
		pose();
		placeNodes();
		break;
	case 2:
		mStateFlags &= ~0x22;
		colliderOff();
		break;
	}
}

// 0x8007F8AC
void TEneBoss03Dino::actBreathEnd(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0xA);
		mTurn     = angleSideways();
		f32 ratio = (f32)getDinoRingAngle();
		ratio /= 16.666666f;
		mTurnTo = (int)(f32)__fabs(ratio);
		colliderOn();
		colliderNoTouch();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlIsBlending(&mAnim) == 0 && motionCtrlFrame(&mAnim) >= 13.0f) {
			mBody.mRot.y = angleApproach(mBody.mRot.y, mTurn, mTurnTo);
		}
		waitMotion();
		pose();
		placeNodes();
		break;
	case 2:
		colliderOff();
		break;
	}
}

// 0x8007F9CC
void TEneBoss03Dino::actSwipe(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 6);
		mStateFlags &= ~8;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		f32 frame = motionCtrlFrame(&mAnim);
		if (frame >= 15.0f && frame <= 30.0f) {
			mStateFlags |= 8;
		} else {
			mStateFlags &= ~8;
		}
		waitMotion();
		pose();
		placeNodes();
		if (motionCtrlPassed(15.0f, &mAnim) != 0) {
			seStartAtRange(0x120, &mNodes[NODE_MOUTH], 0x78, 1400.0f);
		}
		break;
	}
	case 2:
		mStateFlags &= ~8;
		break;
	}
}

// 0x8007FAD4
void TEneBoss03Dino::actBite(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 7);
		mStateFlags &= ~0x10;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlPassed(19.0f, &mAnim) != 0) {
			mStateFlags |= 0x10;
		} else {
			mStateFlags &= ~0x10;
		}
		waitMotion();
		pose();
		placeNodes();
		break;
	case 2:
		break;
	}
}

// 0x8007FB88
void TEneBoss03Dino::actDead(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xB);
		mStateFlags |= 0x20;
		mBody.mPos.x = 0.0f;
		mBody.mPos.y = 0.0f;
		mBody.mPos.z = 0.0f;
		mBody.mRot.x = 0;
		mBody.mRot.y = 0;
		mBody.mRot.z = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		waitMotion();
		pose();
		placeNodes();
		break;
	case 2:
		break;
	}
}

// 0x8007FC28
void TEneBoss03Dino::vfn_10()
{
	if (isInView(1000.0f, 70.0f, &mBody.mPos, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x8007FC88
void TEneBoss03Dino::applyMatrix()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
}

// 0x8007FCC0
void TEneBoss03Dino::render()
{
	njSetTexture(*gBoss03Textures);
	njPushMatrixEx();
	applyMatrix();
	if (motionCtrlIsBlending(&mAnim) != 0) {
		f32 rate = motionCtrlBlendTime(&mAnim);
		gjDrawMotionLink(*gBoss03Models, motionCtrlGetBlend(&mAnim), rate);
	} else {
		NJS_MOTION* motion = motionCtrlGetMotion(&mAnim);
		gjDrawMotion(*gBoss03Models, motion, motionCtrlFrame(&mAnim));
	}
	njPopMatrixEx();
}
