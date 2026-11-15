#ifndef _TENEBOSS03MODEL_H
#define _TENEBOSS03MODEL_H

#include "core/types.h"
#include "stage/Stage.h"
#include "Ninja/ninja.h"

/**
 * @size 0x8
 */
struct MotionLinkHooks {
	void (*mTransform)(NJS_OBJECT* obj); // _00
	void (*mNextNode)();                 // _04
};

typedef int (*ModelNodeFn)(NJS_OBJECT* obj);

extern "C" {
extern NJS_OBJECT** gBoss03Models; // the archive's models 0 is the dinosaur
extern NJS_TEXLIST** gBoss03Textures;
extern NJS_MOTION** gBoss03Motions;
extern f32 gDinoRingRadius;  // how far from the middle the dinosaur walks
extern int gDinoRingAngle;   // which way it faces on the ring relative to the way out
extern Vec3f gDinoNode9Rest; // node 9 of the dinosaur at rest in model space
extern Vec3f gDinoHeadRest;  // its head node 8 at rest

void countNodes(NJS_OBJECT* obj, int* count);
void fillNodeDepths(NJS_OBJECT* obj, s8* out, int* index, int* depth);
void getNodeDepths(NJS_OBJECT* obj, s8* out);
void buildNodeMask(int count, const s8* depth, s8* mask, const int* nodes);
void motionHooksInit(MotionLinkHooks* hooks);
void motionHooksStart(MotionLinkHooks* hooks, void* motion, f32 frame);
void motionHooksStartLink(MotionLinkHooks* hooks, void* motion, f32 rate);
void motionHooksTransform(MotionLinkHooks* hooks, NJS_OBJECT* obj);
void motionHooksNextNode(MotionLinkHooks* hooks);
int walkNodes(NJS_OBJECT* obj);
void walkModel(NJS_OBJECT* obj, ModelNodeFn callback);
void setNodeTargets(const int* nodes, Vec3f* points, MotionLinkHooks* hooks, int dirNode, Vec3f* dirs);
void poseNode(NJS_OBJECT* obj);
int recordNode();
int nodesDone();
int findNodeStep(NJS_OBJECT* obj);
int findNodeDirStep(NJS_OBJECT* obj);
void initBoss03Model();
void freeBoss03Model();
f32 getDinoRingRadius();
int getDinoRingAngle();
}

extern "C" {
extern const int gBoss03DinoNodes[];
}

#endif
