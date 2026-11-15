#ifndef _ENGINE_ACTORMOTION_H_
#define _ENGINE_ACTORMOTION_H_
#include "Ninja/ninja.h"

#include "core/types.h"

#include "Ninja/ninja.h"

typedef void (*NodeCallback)(NJS_OBJECT* node, void* arg);
struct GjActor;
struct NJS_OBJECT;

extern "C" {
void beginActorDraw();
void calcActorPose(GjActor*, NodeCallback, void*);
void calcLinkNodes(NJS_OBJECT*, NodeCallback, void*);
void calcMotionNodes(NJS_OBJECT*, NodeCallback, void*);
void calcPoseNodes(NJS_OBJECT*, NodeCallback, void*);
void endActorDraw();
void walkMotionNodes(NJS_OBJECT*, NodeCallback, void*);
}

extern "C" {
void mtxFromView(f32*, const f32*);
}

extern "C" {
GjActor* createActor(NJS_OBJECT*);
void drawActor(GjActor*);
}

extern "C" {
void calcActorMotion(GjActor*, void*, NodeCallback, f32, void*);
void calcActorLink(GjActor*, void*, NodeCallback, f32, void*);
void walkMotion(f32, NJS_OBJECT*, void*, NodeCallback, void*);
void walkPoseNodes(NJS_OBJECT*, NodeCallback, void*);
}

struct ColliContact;
class CharaColli;
typedef void (*ContactCallback)(ColliContact* contact, void* arg, int* stop);
extern "C" {
void forEachContact(CharaColli* colli, ContactCallback fn, void* arg);
}

#endif
