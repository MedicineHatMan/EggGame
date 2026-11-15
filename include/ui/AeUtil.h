#ifndef _UI_AEUTIL_H_
#define _UI_AEUTIL_H_

#include "core/types.h"

class TAeLayer;
struct MotionNodePose;
struct NJS_OBJECT;
struct Vec3f;

extern "C" {
void aeClipSetDigits3(TAeLayer*, int, int, u32, u32, u32);
void consoleSetBlink(TAeLayer*, int);
void consoleSetGauge(TAeLayer**, int);
void motionGetNodePose(MotionNodePose*, NJS_OBJECT*, void*, f32);
void motionObjInit(NJS_OBJECT*);
void motionPoseGetPos(const MotionNodePose*, Vec3f*);
void nodeTrackBeginPoints(const int*, Vec3f*);
void nodeTrackEnd();
int nodeTrackIndex();
void nodeTrackNext();
void nodeTrackOrigins();
void nodeTrackPoints();
void nodeTrackSet(const int*, Vec3f*);
}

extern "C" {
void aeClipSetDigit(TAeLayer*, int, int, u32);
void aeClipSetDigits2(TAeLayer*, int, int, u32, u32);
void lerp4f(f32, f32*, const f32*, const f32*);
}

extern "C" {
extern const u32 sAeGaugeCells[11][5];
}

#endif
