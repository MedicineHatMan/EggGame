#ifndef _CAMERA_TCAMPATH_H_
#define _CAMERA_TCAMPATH_H_
struct DrawState;
struct EnemyLight;

#include "core/types.h"
struct NJS_TEXLIST;

struct SetTransform;

extern "C" {
void loadRedMagmaBoss();
void loadRedMagmaStage();
NJS_TEXLIST* redMagmaTexList();
void releaseRedMagma();
void releaseRedMagmaBoss();
f32 scrollModelRadius(SetTransform*);
}

extern "C" {
void setDrawAmbient(const EnemyLight*, DrawState*);
}

#include "io/ResBank.h"
extern "C" {
extern ResBank gRedMagmaBank;
extern const char* gTObjRedMagmaName;
extern const char* gTObjRedMagmaSceneName;
extern const char* gTObjRedMagmaObjName;
}

#endif
