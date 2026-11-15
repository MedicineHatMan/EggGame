#ifndef _CAMERA_CAMCURVE_H_
#define _CAMERA_CAMCURVE_H_

#include "core/types.h"

struct CamCurve;
struct CamHead;
struct CamTrail;
struct Vec3f;

extern "C" {
int camCurveDrift(CamCurve*, int, const Vec3f*);
void camCurveGetPos(const CamCurve*, Vec3f*);
void camCurveHold(CamCurve*);
void camCurveReset(CamCurve*, const Vec3f*);
void camCurveResetLift(CamCurve*);
void camCurveSetDrifting(CamCurve*);
void camCurveStart(CamCurve*, const Vec3f*);
void camCurveUpdate(CamCurve*, CamHead*);
void camHeadGetDir(const CamHead*, Vec3f*);
u16 camHeadGetId(CamHead*);
void camHeadGetPos(const CamHead*, Vec3f*);
f32 camHeadGetRise(CamHead*);
int camHeadGetRot(CamHead*);
int camHeadGetStill(CamHead*);
int camHeadIsHidden(CamHead*);
void camHeadStart(CamHead*, u16);
int camHeadTurn(CamHead*, const Vec3f*);
void camHeadUpdate(CamHead*, u16);
f32 camSpring(f32, f32*, Vec3f*, const Vec3f*);
void camTrailClear(CamTrail*);
int camTrailIndex(const CamTrail*, int);
f32 camTrailPush(f32, CamTrail*, const Vec3f*);
}

extern "C" {
void camCurveSetHeight(f32, CamCurve*);
void camCurveSetLift(f32, CamCurve*);
}

#endif
