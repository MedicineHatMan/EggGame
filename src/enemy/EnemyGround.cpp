#include "enemy/EnemyGround.h"
#include "stage/SetManager.h"
#include "engine/CharaColli.h"
#include "enemy/boss/EneBoss.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "core/MathUtil.h"
#include "object/ObjShadow.h"
#include "animal/RacerMover.h"

// 0x80146FF8
int GroundFilter::accept(int hit, f32 y, f32 groundY, f32 normalY)
{
	return true;
}

// 0x80147000
int StepFilter::accept(int hit, f32 y, f32 groundY, f32 normalY)
{
	int ok = false;
	if (hit) {
		f32 rise = groundY - y;
		ok       = -rise <= mStep;
	}
	return ok;
}

// 0x80147030
int SlopeFilter::accept(int hit, f32 y, f32 groundY, f32 normalY)
{
	int ok = false;
	if (hit) {
		f32 rise = groundY - y;
		if (rise < 0.0f) {
			u8 down = 0;
			if (-rise <= mStep && normalY > mDownNormal) {
				down = 1;
			}
			ok = down;
		} else {
			ok = normalY > mUpNormal;
		}
	}
	return ok;
}

// 0x80147094
void bodyTurnTo(SetTransform* body, int angle, int rate)
{
	body->mRot.y = angleApproach(body->mRot.y, angle, rate);
}

// 0x801470C8
void bodyFace(SetTransform* body, const Vec3f* target, Vec3f* from, int rate)
{
	Vec3f dir;
	dir = *target;
	njSubVector(&dir, from);
	bodyTurnTo(body, (int)(10430.38043493439 * (f32)atan2(dir.x, dir.z)), rate);
}

// 0x80147148
void colliSumPush(CharaColli* colli, Vec3f* out)
{
	for (ColliContact* contact = colli->mContacts; contact != NULL; contact = contact->mNext) {
		if ((colliContactShape(contact)->mFlags & 0x10) == 0) {
			njAddVector(out, &contact->mPush);
		}
	}
}

// 0x801471A4
void colliFlatPush(CharaColli* colli, Vec3f* out)
{
	f32 zero = 0.0f;
	out->x   = zero;
	out->y   = zero;
	out->z   = zero;
	colliSumPush(colli, out);
	out->y = 0.0f;
}

// 0x801471E8
void flatSteer(f32 scale, const Vec3f* to, Vec3f* from, Vec3f* out)
{
	f32 y  = out->y;
	out->x = to->x;
	out->y = to->y;
	out->z = to->z;
	njSubVector(out, from);
	gjScaleVector(scale, out, out);
	out->y = y;
}

// 0x8014726C
// Returns 1 when the body is above the ground
int clampAboveGround(f32 groundY, f32* y)
{
	if (groundY > *y) {
		*y = groundY;
		return 0;
	}
	return 1;
}

// 0x8014728C
void shadowFollowGround(int hit, ObjShadow* shadow, Vec3f* pos, Vec3f* normal)
{
	if (hit) {
		shadowEnable(shadow);
		shadowSetGround(shadow, pos, normal);
	} else {
		shadowDisable(shadow);
	}
}

// 0x801472E8
int nearXZ(const Vec3f* a, Vec3f* b, void*, void*)
{
	Vec3f d;
	d = *a;
	njSubVector(&d, b);
	return vecLenXZ(&d) < 5.0f;
}

// 0x8014733C
u32 dropToGround(f32 dy, Vec3f* pos, ObjShadow* shadow)
{
	Vec3f probe;
	Vec3f groundPos;
	Vec3f groundNormal;
	int hit;
	int airborne;
	u32 result;

	probe = *pos;
	probe.y += 5.0f;
	pos->y += dy;
	hit = probeGround(&probe, &groundPos, &groundNormal);
	if (hit) {
		airborne = clampAboveGround(groundPos.y, &pos->y);
	} else {
		airborne = 1;
	}
	if (shadow != NULL) {
		shadowFollowGround(hit, shadow, &groundPos, &groundNormal);
	}
	result = 0;
	if (airborne) {
		result |= GROUND_AIRBORNE;
	} else {
		result &= ~GROUND_AIRBORNE;
	}
	if (hit) {
		result |= GROUND_HIT;
	} else {
		result &= ~GROUND_HIT;
	}
	return result;
}

// 0x80147408
// Casts from along dir for *dist plus radius
u32 probeWall(f32 radius, Vec3f* from, Vec3f* dir, f32* dist, Vec3f* outPos, Vec3f* outNormal)
{
	Vec3f to;
	Vec3f hitPos;
	Vec3f hitNormal;

	gjMulAddVector(*dist + radius, &to, from, dir);
	to.y = from->y;
	if (probeGroundLine(from, &to, &hitPos, &hitNormal)) {
		to.x = hitPos.x;
		to.y = hitPos.y;
		to.z = hitPos.z;
		njSubVector(&to, from);
		to.y  = 0.0f;
		*dist = njScalor(&to) - radius;
		if (outPos != NULL) {
			outPos->x = hitPos.x;
			outPos->y = hitPos.y;
			outPos->z = hitPos.z;
		}
		if (outNormal != NULL) {
			outNormal->x = hitNormal.x;
			outNormal->y = hitNormal.y;
			outNormal->z = hitNormal.z;
		}
		return GROUND_WALL;
	}
	return 0;
}

// 0x80147514
u32 moveOnGround(f32 dy, Vec3f* pos, const Vec3f* to, GroundFilter* filter, ObjShadow* shadow)
{
	Vec3f groundPos;
	Vec3f groundNormal;
	int ok;
	int hit;
	int airborne;
	u32 result;

	hit = probeGround((Vec3f*)to, &groundPos, &groundNormal);
	ok  = filter->accept(hit, pos->y, groundPos.y, groundNormal.y);
	if (ok) {
		pos->x = to->x;
		pos->z = to->z;
	} else {
		hit = probeGround(pos, &groundPos, &groundNormal);
	}
	pos->y += dy;
	if (hit) {
		airborne = clampAboveGround(groundPos.y, &pos->y);
	} else {
		airborne = 1;
	}
	if (shadow != NULL) {
		shadowFollowGround(hit, shadow, &groundPos, &groundNormal);
	}
	result = 0;
	if (airborne) {
		result |= GROUND_AIRBORNE;
	} else {
		result &= ~GROUND_AIRBORNE;
	}
	if (hit) {
		result |= GROUND_HIT;
	} else {
		result &= ~GROUND_HIT;
	}
	if (ok) {
		result &= ~GROUND_REFUSED;
	} else {
		result |= GROUND_REFUSED;
	}
	return result;
}

// 0x80147644
u32 moveBodyStep(f32 radius, f32 step, Vec3f* pos, Vec3f* move, ObjShadow* shadow)
{
	StepFilter filter(step);
	return moveBody(radius, pos, move, &filter, shadow);
}

// 0x80147688
u32 moveBody(f32 radius, Vec3f* pos, const Vec3f* move, GroundFilter* filter, ObjShadow* shadow)
{
	Vec3f from;
	Vec3f dir;
	Vec3f to;
	f32 dist;
	u32 wall;
	f32 dy = move->y;

	dir.x   = move->x;
	dir.y   = move->y;
	dir.z   = move->z;
	dir.y   = 0.0f;
	f32 len = njUnitVector(&dir);
	if (len < gUnitVectorMin) {
		dir.x = gVecZero.x;
		dir.y = gVecZero.y;
		dir.z = gVecZero.z;
	}
	dist   = len;
	from.x = pos->x;
	from.y = pos->y;
	from.z = pos->z;
	from.y += 10.0f;
	wall = probeWall(radius, &from, &dir, &dist, NULL, NULL);
	gjMulAddVector(dist, &to, &from, &dir);
	return wall | moveOnGround(dy, pos, &to, filter, shadow);
}
