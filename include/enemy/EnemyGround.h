#ifndef _ENEMYGROUND_H
#define _ENEMYGROUND_H

#include "stage/Stage.h"

struct SetTransform;
struct CharaColli;
struct ObjShadow;

/** Decides whether a body may stand where a move would put it */
class GroundFilter {
public:
	GroundFilter() { }
	virtual int accept(int hit, f32 y, f32 groundY, f32 normalY); // _08
};

/** Takes ground no more than mStep below the body */
class StepFilter : public GroundFilter {
public:
	StepFilter(f32 step)
	    : mStep(step)
	{
	}
	virtual int accept(int hit, f32 y, f32 groundY, f32 normalY); // _08

	f32 mStep; // _04
};

/** Takes ground up to mStep below the body when its normal points up past mDownNormal and ground above the body when
 * its normal points up past mUpNormal */
class SlopeFilter : public GroundFilter {
public:
	virtual int accept(int hit, f32 y, f32 groundY, f32 normalY); // _08

	f32 mStep;       // _04
	f32 mUpNormal;   // _08
	f32 mDownNormal; // _0C
};

// moveBody and friends return these bits
enum {
	GROUND_HIT      = 1, // ground was found under the body
	GROUND_AIRBORNE = 2, // the body is above the ground
	GROUND_WALL     = 4, // the move ran into a wall
	GROUND_REFUSED  = 8, // the filter refused the new spot
};

extern "C" {
void bodyTurnTo(SetTransform* body, int angle, int rate);
void bodyFace(SetTransform* body, const Vec3f* target, Vec3f* from, int rate);
void colliSumPush(CharaColli* colli, Vec3f* out);
void colliFlatPush(CharaColli* colli, Vec3f* out);
void flatSteer(f32 scale, const Vec3f* to, Vec3f* from, Vec3f* out);
int clampAboveGround(f32 groundY, f32* y);
void shadowFollowGround(int hit, ObjShadow* shadow, Vec3f* pos, Vec3f* normal);
int nearXZ(const Vec3f* a, Vec3f* b, void*, void*);
u32 dropToGround(f32 dy, Vec3f* pos, ObjShadow* shadow);
u32 probeWall(f32 radius, Vec3f* from, Vec3f* dir, f32* dist, Vec3f* outPos, Vec3f* outNormal);
u32 moveOnGround(f32 dy, Vec3f* pos, const Vec3f* to, GroundFilter* filter, ObjShadow* shadow);
u32 moveBodyStep(f32 radius, f32 step, Vec3f* pos, Vec3f* move, ObjShadow* shadow);
u32 moveBody(f32 radius, Vec3f* pos, const Vec3f* move, GroundFilter* filter, ObjShadow* shadow);
}

extern "C" {
extern u8 __vt__11SlopeFilter[];
}

#endif
