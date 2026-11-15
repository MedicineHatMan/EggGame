#include "enemy/EnemySight.h"
#include "enemy/ObjEnemy.h"
#include "object/ObjAction.h"
#include "egg/EggCreature.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "object/TObjPlayer.h"
#include "enemy/EnemyHit.h"
struct TObjAction;
struct TObjPlayer;

#define toAngle(x, z) ((int)(10430.38043493439 * (f32)atan2(x, z)))

static inline void copyVec(Vec3f& out, const Vec3f* in)
{
	out.x = in->x;
	out.y = in->y;
	out.z = in->z;
}

// 0x80090F64
extern "C" f32 flatLengthSq(Vec3f* v)
{
	f32 x = v->x;
	f32 z = v->z;
	x *= x;
	return z * z + x;
}

// 0x80090F78
EnemySightBase::EnemySightBase()
{
	reset();
}

// 0x80090FC0
EnemySightBase::~EnemySightBase()
{
}

// 0x80091008
// Slot _0C
void EnemySightBase::reset()
{
	mPlayerId = 0xFFFF;
	mTarget.x = 0.0f;
	mTarget.y = 0.0f;
	mTarget.z = 0.0f;
	mDelta.x  = 0.0f;
	mDelta.y  = 0.0f;
	mDelta.z  = 0.0f;
}

// 0x80091034
int EnemySightBase::canSee(TObjAction* player)
{
	if (player != 0) {
		return player->canAct();
	}
	return 0;
}

// 0x80091074
EnemySight::EnemySight()
{
	reset();
}

// 0x800910C4
EnemySight::~EnemySight()
{
}

// 0x8009111C
// Slot _0C
void EnemySight::reset()
{
	EnemySightBase::reset();
	mHeightLimit = 100.0f;
	mDistSq      = 0.0f;
	mAngle       = 0;
	mAngleDiff   = 0;
}

// 0x80091164
// Slot _10 Keeps the last player in range not the nearest since rangeSq doesn't shrink
// findPlayer takes the player number as a u16 here the DOL passes the loop counter without masking it
#define findPlayerU16(i) (((TObjPlayer * (*)(u16)) findPlayer)(i))

static inline Vec3f vecDiff(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjSubVector(&r, a, b);
	return r;
}

int EnemySight::findInRange(f32 rangeSq, Vec3f* pos)
{
	TObjAction* player;
	u16 best = 0xFFFF;
	for (u16 i = 0; i < 4; i++) {
		if (canSee(player = (TObjAction*)findPlayerU16(i)) != 0) {
			Vec3f target;
			Vec3f delta;
			copyVec(target, player->getPos());
			delta  = vecDiff(&target, pos);
			f64 dy = __fabs(delta.y);
			if ((f32)dy < mHeightLimit) {
				f32 dist = flatLengthSq(&delta);
				if (dist < rangeSq) {
					mPlayerId = i;
					best      = i;
					copyVec(mTarget, &target);
					copyVec(mDelta, &delta);
					mDistSq = dist;
				}
			}
		}
	}
	if (best != 0xFFFF) {
		mAngle     = toAngle(mDelta.x, mDelta.z);
		mAngleDiff = 0;
		return 1;
	}
	return 0;
}

// 0x800912FC
// Slot _14 The nearest player in range and within a full width of angle
int EnemySight::findInCone(f32 rangeSq, Vec3f* pos, int rotY, int angle)
{
	TObjAction* player;
	u16 best = 0xFFFF;
	for (u16 i = 0; i < 4; i++) {
		if (canSee(player = (TObjAction*)findPlayerU16(i)) != 0) {
			Vec3f delta;
			Vec3f target;
			copyVec(target, player->getPos());
			delta  = vecDiff(&target, pos);
			f64 dy = __fabs(delta.y);
			if ((f32)dy < mHeightLimit) {
				f32 dist = flatLengthSq(&delta);
				if (dist < rangeSq) {
					int to   = toAngle(delta.x, delta.z);
					int diff = angleDist(rotY, to);
					if (diff < ((u16)angle >> 1)) {
						mPlayerId = i;
						rangeSq   = dist;
						best      = i;
						copyVec(mTarget, &target);
						copyVec(mDelta, &delta);
						mDistSq    = dist;
						mAngle     = to;
						mAngleDiff = diff;
					}
				}
			}
		}
	}
	return best != 0xFFFF;
}

// 0x800914CC
// Slot _18 As findInCone with one angle out to farSq and a wider one inside nearSq
int EnemySight::search(f32 farSq, f32 nearSq, Vec3f* pos, int rotY, int farAngle, int nearAngle)
{
	TObjAction* player;
	int nearLimit;
	int farLimit;
	int limit;
	int to;
	u16 best;
	f32 range = farSq;
	best      = 0xFFFF;
	nearLimit = (u16)nearAngle >> 1;
	farLimit  = (u16)farAngle >> 1;
	for (u16 i = 0; i < 4; i++) {
		if (canSee(player = (TObjAction*)findPlayerU16(i)) != 0) {
			Vec3f delta;
			Vec3f target;
			copyVec(target, player->getPos());
			delta  = vecDiff(&target, pos);
			f64 dy = __fabs(delta.y);
			if ((f32)dy < mHeightLimit) {
				f32 dist = flatLengthSq(&delta);
				if (dist < range) {
					if (dist <= nearSq) {
						limit = nearLimit;
					} else {
						limit = farLimit;
					}
					to       = toAngle(delta.x, delta.z);
					int diff = angleDist(rotY, to);
					if (diff < limit) {
						mPlayerId = i;
						range     = dist;
						best      = i;
						copyVec(mTarget, &target);
						copyVec(mDelta, &delta);
						mDistSq    = dist;
						mAngle     = to;
						mAngleDiff = diff;
					}
				}
			}
		}
	}
	return best != 0xFFFF;
}

// MSL's fabsf findInBox hands it the comparison not the value local.z < depth as a float
static inline f32 sightAbs(f32 x)
{
	return __fabs(x);
}

// 0x800916CC
// Slot _1C The nearest player in a box ahead of the enemy
// The depth test is written as fabs(z < depth) where fabs(z) < depth must have been meant
int EnemySight::findInBox(Vec3f* pos, int rotY, f32 halfWidth, f32 depth)
{
	TObjAction* player;
	u16 best;
	u16 i;
	int angle   = -rotY;
	f32 limit   = halfWidth * halfWidth + depth * depth;
	f32 nearest = limit;
	best        = 0xFFFF;
	for (i = 0; i < 4; i++) {
		if (canSee(player = (TObjAction*)findPlayerU16(i)) != 0) {
			Vec3f delta;
			Vec3f target;
			copyVec(target, player->getPos());
			delta  = vecDiff(&target, pos);
			f64 dy = __fabs(delta.y);
			if ((f32)dy < mHeightLimit) {
				f32 dist = flatLengthSq(&delta);
				if (dist < limit) {
					Vec3f local;
					vecRotateY(&delta, &local, angle);
					if ((f32)__fabs(local.x) < halfWidth) {
						if (sightAbs(local.z < depth)) {
							if (dist < nearest) {
								mPlayerId = i;
								nearest   = dist;
								best      = i;
								copyVec(mTarget, &target);
								copyVec(mDelta, &delta);
								mDistSq    = dist;
								mAngle     = toAngle(delta.x, delta.z);
								mAngleDiff = 0;
							}
						}
					}
				}
			}
		}
	}
	return best != 0xFFFF;
}

// 0x80091930
// Slot _20 One given player wherever it is
static inline Vec3f sightSub(const Vec3f* a, const Vec3f* b)
{
	Vec3f d;
	gjSubVector(&d, a, b);
	return d;
}

int EnemySight::track(u16 id, Vec3f* pos, int rotY)
{
	if (isPlayerId(id) != 0) {
		TObjAction* player = (TObjAction*)findPlayerU16(id);
		if (canSee(player) != 0) {
			Vec3f delta;
			Vec3f target;
			copyVec(target, player->getPos());
			delta     = sightSub(&target, pos);
			f32 dist  = flatLengthSq(&delta);
			int to    = toAngle(delta.x, delta.z);
			int diff  = angleDist(rotY, to);
			mPlayerId = id;
			copyVec(mTarget, &target);
			copyVec(mDelta, &delta);
			mDistSq    = dist;
			mAngle     = to;
			mAngleDiff = diff;
			return 1;
		}
	}
	return 0;
}

// 0x80091A9C
EnemyMemory::EnemyMemory()
{
	reset();
}

// 0x80091ACC
void EnemyMemory::reset()
{
	mCooldown = 0;
	mChase    = 0;
	mPlayer   = 0xFFFF;
}

// 0x80091AE8
void EnemyMemory::tick()
{
	if (mCooldown > 0) {
		mCooldown--;
	}
	if (mChase > 0) {
		mChase--;
		if (mChase <= 0) {
			mPlayer = 0xFFFF;
		}
	}
}

// 0x80091B2C
int EnemyMemory::isReady()
{
	return (u8)__rlwnm(1, __cntlzw(mCooldown), 31, 31);
}

// 0x80091B44
int EnemyMemory::isChasing()
{
	return mChase > 0;
}

// 0x80091B58
void EnemyMemory::startCooldown()
{
	mCooldown = gEnemyParams->mCooldown;
}

// 0x80091B68
void EnemyMemory::setChase(u16 player)
{
	mChase  = gEnemyParams->mChase;
	mPlayer = player;
}

// 0x80091B7C
void EnemyMemory::clearChase()
{
	mChase  = 0;
	mPlayer = 0xFFFF;
}

// 0x80091B94
u16 EnemyMemory::getPlayer()
{
	return mPlayer;
}

// 0x80091B9C
// Slot _10
int EnemySightBase::findInRange(f32, Vec3f*)
{
	return 0;
}

// 0x80091BA4
// Slot _14
int EnemySightBase::findInCone(f32, Vec3f*, int, int)
{
	return 0;
}

// 0x80091BAC
// Slot _18
int EnemySightBase::search(f32, f32, Vec3f*, int, int, int)
{
	return 0;
}

// 0x80091BB4
// Slot _1C
int EnemySightBase::findInBox(Vec3f*, int, f32, f32)
{
	return 0;
}

// 0x80091BBC
// Slot _20
int EnemySightBase::track(u16, Vec3f*, int)
{
	return 0;
}
