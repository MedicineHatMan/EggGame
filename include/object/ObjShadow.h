#ifndef _OBJECT_OBJSHADOW_H_
#define _OBJECT_OBJSHADOW_H_
struct ObjShadow;
struct Vec3f;

#include "core/types.h"

extern "C" {
void shadowAlphaOff();
void shadowAlphaOn();
void shadowDraw();
void shadowTevStub();
void shadowSetTevPass();
void shadowSetNoTexGen();
void shadowSetChans();
void shadowBlendAdd();
void shadowBlendSub();
void shadowListInit();
void shadowListStub();
void shadowVolumesDisable();
void shadowAlphaBegin();
void shadowAlphaEnd();
void shadowDarkenScreen();
void shadowDrawAll();
void shadowDrawDecals();
void shadowDrawVolumes();
void* shadowGetModel();
void shadowRestoreAlpha();
void shadowSystemExit();
void shadowSystemInit();
void shadowVolumeAdd();
void shadowVolumeBegin();
void shadowVolumeEnd();
void shadowVolumeSubtract();
}

extern "C" {
void shadowDisable(ObjShadow*);
void shadowSetGround(ObjShadow*, const Vec3f*, const Vec3f*);
void shadowEnable(ObjShadow*);
}

extern "C" {
void shadowSetFromContact(ObjShadow*, u8*);
int shadowSetAngle(ObjShadow*, int);
int shadowIsEnabled(ObjShadow*);
}

extern "C" {
extern u32 sShadowCull;
extern u32 sShadowFlags;
extern int sShadowVolumesOn;
extern ObjShadow* sShadowList;
extern void* sShadowVolume;
extern const void* sShadowVolumeModel[];
}

#endif
