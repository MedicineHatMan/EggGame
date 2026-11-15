#include "engine/ActorMotion.h"
#include "stage/Stage.h"
#include "Ninja/ninja.h"
#include "engine/CharaColli.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "object/TObjLauncher.h"
#include "core/MathUtil.h"
#include "core/Mem.h"
struct Vec3f;
struct NJS_VECTOR;

extern "C" {

// 0x8010D8B4
// An actor for the object tree or 0 when there is no memory
GjActor* createActor(NJS_OBJECT* object)
{
	GjActor* actor = (GjActor*)memAlloc(gjActorSize(object));
	if (actor != 0) {
		gjCreateActor(object, (u8*)actor);
	}
	return actor;
}

// 0x8010D8FC
void beginActorDraw()
{
	gjLoadModelView(0);
	gjBeginDrawModel();
}

// 0x8010D924
void endActorDraw()
{
	gjEndDrawModel();
}

// 0x8010D944
// Draws an actor whose vertices are shorts position and normal 12 bytes apart
void drawActor(GjActor* actor)
{
	u8* verts = actor->verts;
	GXSetArray(GX_VA_POS, verts, 0xC);
	GXSetArray(GX_VA_NRM, verts + 6, 0xC);
	_gjDrawModel(actor->object->model);
}

// 0x8010D99C
// Poses the actor at a frame of the motion
void calcActorMotion(GjActor* actor, void* motion, NodeCallback fn, f32 frame, void* arg)
{
	njStartMotionObj(motion, frame);
	gjBeginCalcActor(actor);
	calcMotionNodes(actor->object, fn, arg);
	gjEndCalcActor(actor);
}

// 0x8010D9F4
void calcMotionNodes(NJS_OBJECT* node, NodeCallback fn, void* arg)
{
	do {
		njPushMatrixEx();
		njMotionTransformEx(node);
		if (fn != 0) {
			fn(node, arg);
		}
		if (node->model != 0 && !(node->evalflags & 8)) {
			gjCalcActor(node->model);
		}
		if (node->child != 0) {
			calcMotionNodes(node->child, fn, arg);
		}
		njPopMatrixEx();
		node = node->sibling;
	} while (node != 0);
}

// 0x8010DA90
// Poses the actor from a blend of two motions
void calcActorLink(GjActor* actor, void* link, NodeCallback fn, f32 blend, void* arg)
{
	njStartMotionLink((const NJS_MOTION_LINK*)link, blend);
	gjBeginCalcActor(actor);
	calcLinkNodes(actor->object, fn, arg);
	gjEndCalcActor(actor);
}

// 0x8010DAE8
void calcLinkNodes(NJS_OBJECT* node, NodeCallback fn, void* arg)
{
	do {
		njPushMatrixEx();
		njMotionLinkTransformEx(node);
		if (fn != 0) {
			fn(node, arg);
		}
		if (node->model != 0 && !(node->evalflags & 8)) {
			gjCalcActor(node->model);
		}
		if (node->child != 0) {
			calcLinkNodes(node->child, fn, arg);
		}
		njPopMatrixEx();
		node = node->sibling;
	} while (node != 0);
}

// 0x8010DB84
// Poses the actor from its nodes' own transforms
void calcActorPose(GjActor* actor, NodeCallback fn, void* arg)
{
	gjBeginCalcActor(actor);
	calcPoseNodes(actor->object, fn, arg);
	gjEndCalcActor(actor);
}

static inline void nodeTransform(NJS_OBJECT* node)
{
	u32 flags = node->evalflags;
	if (!(flags & 1)) {
		njTranslateEx((const Vec3f*)node->pos);
	}
	if (!(flags & 2)) {
		if (flags & 0x400) {
			njQuaternionEx2(node->quatRe, (const Vec3f*)((const NJS_VECTOR*)node->ang));
		} else {
			njRotateEx(node->ang, flags & 0x20);
		}
	}
	if (!(flags & 4)) {
		njScaleEx((const Vec3f*)node->scl);
	}
}

// 0x8010DBD0
void calcPoseNodes(NJS_OBJECT* node, NodeCallback fn, void* arg)
{
	do {
		njPushMatrixEx();
		nodeTransform(node);
		if (fn != 0) {
			fn(node, arg);
		}
		if (node->model != 0 && !(node->evalflags & 8)) {
			gjCalcActor(node->model);
		}
		if (node->child != 0) {
			calcPoseNodes(node->child, fn, arg);
		}
		njPopMatrixEx();
		node = node->sibling;
	} while (node != 0);
}

// 0x8010DCB4
// Calls fn for each contact of the collider until it sets *stop
void forEachContact(CharaColli* colli, ContactCallback fn, void* arg)
{
	int stop = 0;
	for (ColliContact* contact = colli->mContacts; contact != 0 && stop == 0; contact = contact->mNext) {
		fn(contact, arg, &stop);
	}
}

// 0x8010DD20
void walkMotionNodes(NJS_OBJECT* node, NodeCallback fn, void* arg)
{
	do {
		njPushMatrixEx();
		njMotionTransformEx(node);
		if (fn != 0) {
			fn(node, arg);
		}
		if (node->child != 0) {
			walkMotionNodes(node->child, fn, arg);
		}
		njPopMatrixEx();
		node = node->sibling;
	} while (node != 0);
}

// 0x8010DDA0
// Walks an object tree at a frame of the motion without an actor
void walkMotion(f32 frame, NJS_OBJECT* object, void* motion, NodeCallback fn, void* arg)
{
	njStartMotionObj(motion, frame);
	walkMotionNodes(object, fn, arg);
}

// 0x8010DDE8
// out = the current view matrix * in
void mtxFromView(f32* out, const f32* in)
{
	mtxConcat(out, (const f32*)gjGetMatrix(), in);
}

// 0x8010DE28
void walkPoseNodes(NJS_OBJECT* node, NodeCallback fn, void* arg)
{
	do {
		njPushMatrixEx();
		nodeTransform(node);
		if (fn != 0) {
			fn(node, arg);
		}
		if (node->child != 0) {
			walkPoseNodes(node->child, fn, arg);
		}
		njPopMatrixEx();
		node = node->sibling;
	} while (node != 0);
}
}
