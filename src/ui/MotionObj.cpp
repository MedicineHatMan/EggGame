#include "ui/TAeLayer.h"
#include "engine/MotionPose.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "ui/AeUtil.h"
struct NJS_QUATERNION;

// 0x800D33B0
extern "C" void motionObjInit(NJS_OBJECT* obj)
{
	obj->evalflags = 0;
	obj->model     = 0;
	obj->pos[0]    = 0.0f;
	obj->pos[1]    = 0.0f;
	obj->pos[2]    = 0.0f;
	obj->ang[0]    = 0;
	obj->ang[1]    = 0;
	obj->ang[2]    = 0;
	obj->scl[0]    = 1.0f;
	obj->scl[1]    = 1.0f;
	obj->scl[2]    = 1.0f;
	obj->child     = 0;
	obj->sibling   = 0;
	obj->quatRe    = 0.0f;
}

// 0x800D33F8
extern "C" void motionGetNodePose(MotionNodePose* out, NJS_OBJECT* obj, void* motion, f32 frame)
{
	njStartMotionObj(motion, frame);
	gjPushUnitMatrix();
	njGetMotionNodeData(obj, &out->mPos.x, (s32*)out->mAng, &out->mScale.x, (NJS_QUATERNION*)out->mQuat);
	njPopMatrixEx();
}

// 0x800D344C
extern "C" void motionPoseGetPos(const MotionNodePose* pose, Vec3f* out)
{
	*out = pose->mPos;
}
