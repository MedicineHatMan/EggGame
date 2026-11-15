#ifndef _MOTIONPOSE_H
#define _MOTIONPOSE_H

#include "core/types.h"
#include "stage/Stage.h"

struct NJS_OBJECT;

/**
 * One node's pose at a motion frame, as njGetMotionNodeData fills it.
 *
 * @size 0x34
 */
struct MotionNodePose {
	Vec3f mPos;   // _00
	Vec3f mScale; // _0C
	s32 mAng[3];  // _18
	f32 mQuat[4]; // _24
};

extern "C" {
// Resets an object to no model no children no offset or turn and a scale of 1
void motionObjInit(NJS_OBJECT* obj);
// Plays motion on obj at frame and reads the root node's pose
void motionGetNodePose(MotionNodePose* out, NJS_OBJECT* obj, void* motion, f32 frame);
// 0x800D344C
void motionPoseGetPos(const MotionNodePose* pose, Vec3f* out);

/* Node tracking While a model is drawn with a tracking callback installed each node whose number is next in the list
 * has a point taken through its matrix and written to the next Vec3f */
void nodeTrackSet(const int* nodes, Vec3f* out);           // 0x800D32F4 without a callback
void nodeTrackSetCallback(void (*callback)());             // 0x800D3314 any callback 0 for none
void nodeTrackBeginOrigins(const int* nodes, Vec3f* out);  // 0x800D3334 record each node's origin
void nodeTrackBeginPoints(const int* nodes, Vec3f* points); // 0x800D3360 move each point into world space
void nodeTrackEnd();                                       // 0x800D338C
void nodeTrackOrigins();                                   // 0x800D321C for a callback record this node
void nodeTrackPoints();                                    // 0x800D3284 for a callback move this node's points
int nodeTrackIndex();                                      // 0x800D32DC the node being drawn
void nodeTrackNext();                                      // 0x800D32E4 for a callback on to the next node
}

#endif
