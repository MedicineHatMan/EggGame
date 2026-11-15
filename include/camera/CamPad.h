#ifndef _CAMERA_CAMPAD_H_
#define _CAMERA_CAMPAD_H_

#include "core/types.h"

struct CamPath;

extern "C" {
void camPadClear(CamPath*);
int camPadDown(CamPath*);
int camPadHardHeld(CamPath*);
int camPadHardPressed(CamPath*);
void camPadInit(CamPath*);
int camPadLeft(CamPath*);
int camPadRight(CamPath*);
int camPadStickMoved(CamPath*);
void camPadUpdate(CamPath*, int, u16, int, u16, s16, s16);
f32 camPadX(CamPath*);
f32 camPadY(CamPath*);
}

#endif
