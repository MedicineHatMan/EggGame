#include "stage/Stage.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "stage/SetManager.h"
#include "enemy/EnemyAppear.h"
#include "enemy/TEneAm01Parts.h"
#include "Ninja/ninja.h"
#include "engine/Viewport.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/TParticle.h"
#include "enemy/EnemyCommon.h"
#include "stl/math.h"
#include "enemy/EnemyFx.h"
#include "core/Mem.h"
#include "core/dolphin.h"
#include "core/MathUtil.h"
#include "enemy/ObjEnemy.h"
#include "egg/EggCreature.h"
#include "enemy/TEneDeathCrow.h"
#include "core/VecMath.h"
#include "object/TObjEggFeed.h"
#include "enemy/Enemy.h"
#include "core/TMainTask.h"
#include "animal/RacerMover.h"
struct GjActor;
struct NJS_QUATERNION;
struct NJS_OBJECT;
struct TObjEggFeed;

// 0x8001AD10
void* enemyDescGetKind(void* owner, s16* a, s16* b)
{
	u8* data = *(u8**)((u8*)owner + 0xC);
	if (data != NULL) {
		*a = *(s16*)(data + 0x1C);
		*b = *(s16*)(data + 0x1E);
	}
	return data;
}

// 0x8001AD30
void alphaTestBegin()
{
	GXSetZCompLoc(GX_FALSE);
	GXSetAlphaCompare(GX_GREATER, 1, GX_AOP_AND, GX_LEQUAL, 0xFF);
}

// 0x8001AD6C
void alphaTestEnd()
{
	GXSetZCompLoc(GX_TRUE);
	GXSetAlphaCompare(GX_GREATER, 1, GX_AOP_AND, GX_LEQUAL, 0xFF);
}

// 0x8001ADA8
// Speeds speed up toward a gap by accel while it heads toward it by brake while it heads away then caps it at gain of
// the gap kept between least and most and at the gap itself
f32 approachSpeed(f32 speed, f32 gap, f32 gain, f32 accel, f32 brake, f32 most, f32 least)
{
	f32 dist = gap;
	if (gap < 0.0f) {
		speed *= -1.0f;
		dist *= -1.0f;
	}
	if (speed >= 0.0f) {
		speed += accel;
	} else {
		speed += brake;
	}
	if (speed >= 0.0f) {
		f32 cap = dist * gain;
		if (cap > most) {
			cap = most;
		} else if (cap < least) {
			cap = least;
		}
		if (speed > cap) {
			speed = cap;
		}
		if (speed > dist) {
			speed = dist;
		}
	}
	if (gap < 0.0f) {
		speed *= -1.0f;
	}
	return speed;
}

// 0x8001AE40
// Moves vel toward the gap the part along it by approachSpeed the part across it scaled by keep
// Returns the gap's length
f32 springVelocity(f32 gain, f32 keep, Vec3f* vel, f32 accel, const Vec3f* gap, f32 brake, f32 most, f32 least)
{
	Vec3f dir;
	dir.x    = gap->x;
	dir.y    = gap->y;
	dir.z    = gap->z;
	f32 dist = njUnitVector(&dir);
	if (dist < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 along = njInnerProduct(vel, &dir);
	Vec3f side;
	side.x = dir.x;
	side.y = dir.y;
	side.z = dir.z;
	gjScaleVector(-along, &side, &side);
	njAddVector(&side, vel);
	f32 speed = approachSpeed(along, dist, gain, accel, brake, most, least);
	gjScaleVector(keep, &side, &side);
	gjMulAddVector(speed, vel, &side, &dir);
	return dist;
}

// 0x8001AFE0
// A random point in a flat ring around a centre
void randomPointInRing(f32 inner, f32 outer, const Vec3f* center, Vec3f* out)
{
	f32 s, c;
	njSinCos((int)(65536.0f * (3.0517578125e-05f * rand())), &s, &c);
	f32 dist = (outer - inner) * (3.0517578125e-05f * rand()) + inner;
	out->x   = s * dist + center->x;
	out->y   = center->y;
	out->z   = c * dist + center->z;
}

static inline Vec3f negated(const Vec3f& v)
{
	Vec3f r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

static inline Quat quatTimes(const Quat& a, const Quat* r)
{
	Quat q = a;
	njMultiQuaternion((const NJS_QUATERNION*)&q, (const NJS_QUATERNION*)r, (NJS_QUATERNION*)&q);
	return q;
}

// Half of turn times rot
static inline Quat quatHalfTimes(const Quat& turn, const Quat* rot)
{
	Quat q = quatTimes(turn, rot);
	quatScale(&q, 0.5f);
	return q;
}

// 0x8001B0D4
// A random whole number from base to base + range
int randomInt(f32 base, f32 range)
{
	return (int)(range * (3.0517578125e-05f * rand()) + base);
}

// 0x8001B150
// Turns rot as a ball of that radius rolling with vel would turn
void rollQuat(const Vec3f* vel, Quat* rot, f32 radius)
{
	Vec3f axis;
	Vec3f spin;
	Quat turn;
	Vec3f down = negated(gVecUp);
	njOuterProduct(vel, &down, &axis);
	vecScale(1.0f / radius, &spin, &axis);
	turn.w    = 0.0f;
	turn.v    = spin;
	Quat half = quatHalfTimes(turn, rot);
	quatAdd(rot, &half);
	quatNormalize(rot);
}

// 0x8001B2A0
// Whether a value is inside a range
int rampInt(f32 value, f32 low, f32 high, int span)
{
	int result = 0;
	if (!(value < low)) {
		if (value <= high) {
			f32 t = value - low;
			t /= high - low;
			result = span * t;
		} else {
			result = span;
		}
	}
	return result;
}

// 0x8001B308
// Starts a particle generator or moves the one it has
void particleKeepAt(void** gen, const Vec3f* pos, int group, int id)
{
	if (*gen == NULL) {
		*gen = particleGenByIdDefault(pos, group, id);
	} else {
		((TParticleGenerator*)*gen)->mPos = *pos;
	}
}

// 0x8001B36C
void enemyFxKeepAt(void** gen, const Vec3f* pos, int id)
{
	particleKeepAt(gen, pos, 0, id);
}

// 0x8001B394
void particleKeepAtRot(void** gen, const Vec3f* pos, int rotX, int rotY, int group, int id)
{
	if (*gen == NULL) {
		*gen = particleGenByIdRotDefault(pos, group, id, rotX, rotY, 0.0f);
	} else {
		((TParticleGenerator*)*gen)->mPos = *pos;
		((TParticleGenerator*)*gen)->setRot(rotX, rotY);
	}
}

// 0x8001B41C
void enemyFxKeepAtRot(void** gen, const Vec3f* pos, int rotX, int rotY, int id)
{
	particleKeepAtRot(gen, pos, rotX, rotY, 0, id);
}

// 0x8001B444
// How many of the ascending zero-terminated thresholds a value is past
int thresholdIndex(int value, const int* table)
{
	int i;
	for (i = 0; table[i] > 0; i++) {
		if (value > ((volatile const int*)table)[i]) {
			break;
		}
	}
	return i;
}

// 0x8001B478
// Whether health fell from before to after by at least the drop for the step after is in
int healthDropReached(int before, int after, const int* steps, const int* drops)
{
	int i = thresholdIndex(after, steps);
	return before - after >= drops[i];
}

// 0x8001B4D4
// 1 near falling off with the distance
f32 distanceFalloff(f32 dist)
{
	return __fres(0.01f * dist + 1.0f);
}

// 0x8001B4E8
// Makes an actor for a model at its rest pose
void* actorCreate(void* model)
{
	GjActor* actor = (GjActor*)new u8[gjActorSize((NJS_OBJECT*)model)];
	gjCreateActor((NJS_OBJECT*)model, (u8*)actor);
	gjPushUnitMatrix();
	gjBuildActorVertices(actor);
	njPopMatrixEx();
	return actor;
}

// 0x8001B540
void actorFree(void** actor)
{
	if (*actor != NULL) {
		delete[] (u8*)*actor;
	}
	*actor = NULL;
}

// 0x8001B580
// Poses an actor from a motion player blending if it's in a blend
void actorApplyMotion(void* actor, void* motion)
{
	if (motionCtrlIsBlending(motion) != 0) {
		f32 frame = motionCtrlBlendTime(motion);
		gjMotionLinkActor((GjActor*)actor, motionCtrlGetBlend(motion), frame);
	} else {
		f32 frame = motionCtrlFrame(motion);
		gjMotionActor(frame, (GjActor*)actor, motionCtrlGetMotion(motion));
	}
}

// 0x8001B610
void enemyFxBurst4E(const Vec3f* pos)
{
	particleOnceByIdDefault(pos, 0, 0x4E);
	particleOnceByIdDefault(pos, 0, 0x64);
}

// 0x8001B654
void enemyFxBurst4F(const Vec3f* pos)
{
	particleOnceByIdDefault(pos, 0, 0x4F);
	particleOnceByIdDefault(pos, 0, 0x64);
}

// 0x8001B698
// Sparks facing away from a point
void enemyFxSparks(f32 scale, const Vec3f* pos, const Vec3f* from)
{
	int angle = (int)(10430.38043493439 * (f32)atan2(pos->x - from->x, pos->z - from->z));
	particleOnceByIdRotDefault(pos, 0, 0x53, 0, angle, scale);
	particleOnceByIdRotDefault(pos, 0, 0x54, 0, angle, scale);
}

// 0x8001B740
void enemyFxBurst50(const Vec3f* pos)
{
	particleOnceByIdDefault(pos, 0, 0x50);
}

// 0x8001B768
void enemyFxDeath44(const Vec3f* pos)
{
	particleOnceByIdDefault(pos, 0, 0x44);
	particleGenTimedByIdDefault(pos, 0, 0x43, 5, 1);
	particleGenTimedByIdDefault(pos, 0, 0x71, 0, 0xF);
	createDeathCrowLoud(gTL_04, pos);
}

// 0x8001B7D8
void enemyFxDeath4A(const Vec3f* pos)
{
	particleOnceByIdDefault(pos, 0, 0x4A);
	particleGenTimedByIdDefault(pos, 0, 0x70, 0, 0xF);
	createDeathCrowLoud(gTL_04, pos);
}

// 0x8001B830
void enemyFxDeath44Se(const Vec3f* pos)
{
	enemyFxDeath44(pos);
	seStartAt(0xDC, pos);
}

// 0x8001B868
void enemyFxDeath4ASe(const Vec3f* pos)
{
	enemyFxDeath4A(pos);
	seStartAt(0xDC, pos);
}

// 0x8001B8A0
void enemyFxSpawnAtPos(int kind, void* owner, const Vec3f* pos)
{
	f32 m[12];
	mtxSetTransV(m, pos);
	fxSpawnAt(1, kind, (int)owner, (const float*)m);
}

// 0x8001B8EC
void fxSpawnFlat(int kind, void* owner, const Vec3f* pos)
{
	f32 m[12];
	mtxSetTransV(m, pos);
	mtxRotateX(m, m, -0x4000);
	fxSpawnAt(1, kind, (int)owner, (const float*)m);
}

// 0x8001B948
void fxSpawnTurned(int kind, void* owner, const Vec3f* pos, int rotX, int rotY)
{
	f32 m[12];
	mtxSetTransV(m, pos);
	mtxRotateY(m, m, rotY);
	mtxRotateX(m, m, rotX);
	fxSpawnAt(1, kind, (int)owner, (const float*)m);
}

// 0x8001B9BC
void fxSpawnScaled(f32 sx, f32 sy, f32 sz, int kind, void* owner, const Vec3f* pos, int rotX, int rotY)
{
	f32 m[12];
	mtxSetTransV(m, pos);
	mtxRotateY(m, m, rotY);
	mtxRotateX(m, m, rotX);
	mtxScale(sx, sy, sz, m, m);
	fxSpawnAt(1, kind, (int)owner, (const float*)m);
}

// 0x8001BA64
void enemyFxLoadStub()
{
}

// 0x8001BA68
void enemyFxFreeStub()
{
}

// ---------------------------------------------------------------------------------------------------------------------
// 0x8001BA6C-0x8001CE24 the egg enemies appear from the hatch pose the crush and break deaths the hit tint and the drop
// spawner

// 0x8001BA6C
TEnemyAppear::TEnemyAppear(TObject* parent, Vec3f pos, int dropIn)
    : TObject(parent)
{
	mTimer     = 0;
	_22        = 0;
	mPos       = pos;
	mVelY      = 0.0f;
	mSpeed     = 0.0f;
	mDir.x     = 0.0f;
	mDir.y     = 0.0f;
	mDir.z     = 0.0f;
	mMoveFlags = 0;
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags |= 0x10;
	if (dropIn != 0) {
		mPos.y += 25.0f;
		mScale = 0.0f;
		mState = STATE_WAIT;
		particleGenTimedById(this, &mPos, 0, 0x40, 0, 8);
		particleGenTimedById(this, &mPos, 0, 0x41, 8, 1);
		particleGenTimedById(this, &mPos, 0, 0x42, 0x12, 4);
	} else {
		mPos.y -= 3.5f;
		mScale = 0.1f;
		mState = STATE_FALL;
	}
}

// 0x8001BBB8
TEnemyAppear::~TEnemyAppear()
{
	removeDrawTask(mDrawTask);
}

// 0x8001BC20
int TEnemyAppear::isLanded()
{
	return mState == STATE_DONE || mState == STATE_LAND;
}

// 0x8001BC44
void TEnemyAppear::getPos(Vec3f* out)
{
	*out = mPos;
}

// 0x8001BC60
void TEnemyAppear::launch(const Vec3f* vel)
{
	mVelY     = vel->y;
	mDir      = *vel;
	mDir.y    = 0.0f;
	f32 speed = njUnitVector(&mDir);
	if (speed < gUnitVectorMin) {
		mDir = gVecZero;
	}
	mSpeed = speed;
	if (mSpeed >= 0.001f) {
		mMoveFlags |= 1;
	} else {
		mMoveFlags &= ~1;
		mSpeed = 0.0f;
	}
}

// 0x8001BD18
void TEnemyAppear::draw()
{
	switch (mState) {
	case STATE_WAIT:
		mTimer++;
		if (mTimer >= 20) {
			mTimer = 0;
			_22    = 0;
			mState = STATE_FALL;
			seStartAt(0xDB, &mPos);
		}
		break;
	case STATE_FALL: {
		Vec3f hit;
		Vec3f normal;
		Vec3f to;
		mTimer++;
		mScale += 0.05f;
		if (mScale > 1.0f) {
			mScale = 1.0f;
		}
		int landed = 0;
		if (mMoveFlags & 1) {
			gjMulAddVector(5.0f + mSpeed, &to, &mPos, &mDir);
			if (probeGroundLine(&mPos, &to, &hit, &normal) != 0) {
				normal.y = 0.0f;
				if (njUnitVector(&normal) < gUnitVectorMin) {
					normal = gVecZero;
				}
				gjMulAddVector(-5.0f, &mPos, &hit, &normal);
				vecReflect(&mDir, &normal, &mDir);
			} else {
				gjMulAddVector(mSpeed, &mPos, &mPos, &mDir);
			}
		}
		mVelY += -0.05f;
		f32 y = mPos.y + mVelY;
		if (probeGround(&mPos, &hit, NULL) != 0 && y < hit.y) {
			y      = hit.y;
			landed = 1;
		}
		mPos.y = y;
		if (mTimer >= 180) {
			landed = 1;
		}
		if (landed != 0) {
			mState = STATE_LAND;
			mTimer = 0;
			Vec3f at;
			at = mPos;
			at.y += 5.0f;
			particleOnceByIdDefault(&at, 0, 0x4C);
		}
		break;
	}
	case STATE_LAND:
		mTimer++;
		mState = STATE_DONE;
		mTimer = 0;
		break;
	case STATE_DONE:
		break;
	}
}

// 0x8001BF64
void TEnemyAppear::vfn_10()
{
	if (mState == STATE_FALL || mState == STATE_LAND) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x8001BF98
void TEnemyAppear::render()
{
	enemyCommonSetTexture();
	njPushMatrixEx();
	njTranslateEx(&mPos);
	njScale(NULL, mScale, mScale, mScale);
	gjTranslate(0.0f, 7.0f, 0.0f);
	gjDrawObject(enemyBombModel());
	njPopMatrixEx();
}

// 0x8001C000
// clang-format off
asm void setGqrS16()
{
	nofralloc
	li r3, 0x807
	oris r3, r3, 0x807
	mtspr 0x396, r3
	blr
}
// clang-format on

// The usual inline sqrtf three Newton steps on frsqrte
static inline f32 sqrtfInline(f32 x)
{
	static const f64 _half  = .5;
	static const f64 _three = 3.0;
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		y         = (f32)(x * (_half * guess * (_three - guess * guess * x)));
		return y;
	}
	return x;
}

// 0x8001C010
// Pulls every vertex onto an egg of the model's height growing from the egg's own size as t falls
// The vertices are s16 with 8 fraction bits read and written with quantized paired-single loads
void poseEggShape(GjActor* pose, f32 t, f32 height)
{
	f32 stretch;
	f32 sum;
	register f32 x;
	register f32 y;
	register f32 z;
	f32 scaleY;
	int n;
	register s16* v;

	n          = pose->size / 12;
	v          = (s16*)pose->verts;
	f32 lift   = 0.35714287f * height;
	f32 radius = 4.5f * ((1.0f - t) * (height / 12.599999f) + t);
	setGqrS16();
	for (int i = 0; i < n; i++) {
		// clang-format off
		asm {
			psq_l x, 0(v), 1, 6
			psq_l y, 2(v), 1, 6
			psq_l z, 4(v), 1, 6
		}
		// clang-format on
		y -= lift;
		if (y >= 0.0f) {
			scaleY  = 0.5555556f;
			stretch = 1.7999998f;
		} else {
			stretch = scaleY = 1.0f;
		}
		y *= scaleY;
		sum = x * x;
		sum += y * y;
		sum += z * z;
		f32 len = sqrtfInline(sum);
		f32 s   = radius / len;
		y *= s;
		x *= s;
		z *= s;
		y *= stretch;
		y += radius;
		// clang-format off
		asm {
			psq_st x, 0(v), 1, 6
			psq_st y, 2(v), 1, 6
			psq_st z, 4(v), 1, 6
		}
		// clang-format on
		v += 6;
	}
	DCStoreRange(pose->verts, pose->size);
}

// 0x8001C174
// As above but blends from the egg toward the model's own shape as t falls below split
void poseEggBlend(GjActor* pose, f32 t, f32 height, f32 split)
{
	f32 lift;
	f32 center;
	f32 keep;
	f32 size;
	f32 k;
	f32 target;
	f32 stretch;
	f32 sum;
	register f32 x;
	register f32 y;
	register f32 z;
	f32 scaleY;
	int n;
	register s16* v;

	k      = t / split;
	n      = pose->size / 12;
	v      = (s16*)pose->verts;
	lift   = 0.35714287f * height;
	keep   = 1.0f - k;
	size   = (1.0f - split) * (height / 12.599999f) + split;
	center = k * (4.5f * size) + lift * keep;
	size *= 4.5f;
	setGqrS16();
	target = size * k;
	for (int i = 0; i < n; i++) {
		// clang-format off
		asm {
			psq_l x, 0(v), 1, 6
			psq_l y, 2(v), 1, 6
			psq_l z, 4(v), 1, 6
		}
		// clang-format on
		y -= lift;
		if (y >= 0.0f) {
			scaleY  = 0.5555556f;
			stretch = 1.7999998f;
		} else {
			stretch = scaleY = 1.0f;
		}
		y *= scaleY;
		sum = x * x;
		sum += y * y;
		sum += z * z;
		f32 len = sqrtfInline(sum);
		f32 s   = (len * keep + target) / len;
		y *= s;
		x *= s;
		z *= s;
		y *= stretch;
		y += center;
		// clang-format off
		asm {
			psq_st x, 0(v), 1, 6
			psq_st y, 2(v), 1, 6
			psq_st z, 4(v), 1, 6
		}
		// clang-format on
		v += 6;
	}
	DCStoreRange(pose->verts, pose->size);
}

// 0x8001C324
void poseHatch(GjActor* pose, f32 t, f32 height)
{
	if (t >= 0.7f) {
		poseEggShape(pose, t, height);
	} else {
		poseEggBlend(pose, t, height, 0.7f);
	}
}

// 0x8001C35C
void hatchClear(f32* t)
{
	*t = 0.0f;
}

// 0x8001C368
void hatchStart(f32* t)
{
	*t = 1.0f;
}

// 0x8001C374
int hatchActive(f32* t)
{
	return *t > 0.0f;
}

// 0x8001C38C
int hatchDone(f32* t)
{
	return *t <= 0.0f;
}

// 0x8001C3A8
void hatchStep(f32* t)
{
	*t -= 0.08f;
}

// 0x8001C3BC
void hatchPose(f32* t, GjActor* pose, f32 height)
{
	poseHatch(pose, *t, height);
}

// 0x8001C3E8
// Turns to the hit's direction tips over by rot's x and turns back to the body's own heading
void EnemyCrush::rotate(const SetRot* rot)
{
	int back = rot->y;
	back -= mAngle;
	gjRotateY(mAngle);
	gjRotateX(rot->x);
	gjRotateY(back);
}

// 0x8001C430
void EnemyCrush::apply(SetRot* rot)
{
	njScale(NULL, 1.0f, mTime, 1.0f);
	rotate(rot);
}

// 0x8001C47C
void EnemyCrush::clear()
{
	mTime  = 0.0f;
	mAngle = 0;
	mState = 0;
	mTimer = 0;
}

// 0x8001C498
void EnemyCrush::start(Vec3f* pos, const Vec3f* hit, const Vec3f* ground)
{
	enemyFxBurst50(pos);
	mTimer = 0;
	mState = 0;
	mTime  = 1.0f;
	mAngle = (int)(10430.38043493439 * (f32)atan2(hit->x, hit->z));
	pos->y = 0.5f + ground->y;
}

// 0x8001C520
int EnemyCrush::step(Vec3f* pos, Vec3f* vel, SetRot* rot)
{
	int done = 0;
	switch (mState) {
	case 0: {
		mTime -= 0.05f;
		if (mTime < 0.1f) {
			mTime = 0.1f;
		}
		int x = rot->x;
		x += 0xD00;
		if (x > 0x4000) {
			x = 0x4000;
		}
		rot->x = x;
		mTimer++;
		if (mTimer >= 120) {
			mState = 1;
			mTimer = 0;
			vel->y = 2.0f;
			mTime  = 0.1f;
			rot->x = 0x4000;
			seStartAt(0xE4, pos);
		}
		break;
	}
	case 1:
		mTimer++;
		if (mTimer >= 90) {
			mState = 2;
			done   = 1;
			break;
		}
		mTime += 0.1f;
		if (mTime > 1.0f) {
			mTime = 1.0f;
		}
		vel->y -= 0.12f;
		pos->y += vel->y;
		if (vel->y < -1.7f) {
			mState = 2;
			done   = 1;
		}
		break;
	}
	return done;
}

// 0x8001C678
// The burst at the top of the squashed body when it pops
void EnemyCrush::burst(f32 height, Vec3f* pos, SetRot* rot, Vec3f* out)
{
	out->x = 0.0f;
	out->y = 0.5f * height;
	out->z = 0.0f;
	gjPushUnitMatrix();
	rotate(rot);
	njCalcPointEx(out, out);
	njPopMatrixEx();
	njAddVector(out, pos);
	enemyFxDeath44Se(out);
}

// 0x8001C6F8
EnemyBreak::EnemyBreak(Vec3f dir, Vec3f* pos, Vec3f* vel)
{
	init(dir, pos, vel);
}

// 0x8001C744
EnemyBreak::~EnemyBreak()
{
	if (mSparks != NULL) {
		mSparks->mFlags |= 1;
		mSparks = NULL;
	}
	if (mSmoke != NULL) {
		mSmoke->mFlags |= 1;
		mSmoke = NULL;
	}
}

// 0x8001C7C4
// Flies up and away from the hit
void EnemyBreak::init(Vec3f dir, Vec3f* pos, Vec3f* vel)
{
	*vel = dir;
	if (vel->y < 0.0f) {
		vel->y *= -1.0f;
	}
	if (vel->y > 3.0f) {
		vel->y = 3.0f;
	}
	if (njUnitVector(vel) < gUnitVectorMin) {
		*vel = gVecZero;
	}
	gjScaleVector(2.0f, vel, vel);
	mTimer  = 0;
	mSmoke  = NULL;
	mSparks = NULL;
}

// 0x8001C888
// Waits 9 frames then flies bouncing off the ground for up to 120
static inline Vec3f vecSum(const Vec3f* a, const Vec3f* b)
{
	Vec3f s;
	gjAddVector(&s, a, b);
	return s;
}

int EnemyBreak::update(Vec3f* pos, Vec3f* vel)
{
	Vec3f hit;
	Vec3f normal;
	Vec3f to;
	mTimer++;
	if (mTimer < 9) {
		return 0;
	}
	if (mTimer == 9) {
		if (mSmoke == NULL) {
			mSmoke = particleGenByIdDefault(pos, 0, 0x5F);
		}
		if (mSparks == NULL) {
			mSparks = particleGenByIdDefault(pos, 0, 0x65);
		}
	} else if (mTimer >= 120) {
		return 1;
	}
	f32 speed = njUnitVector(vel);
	if (speed < gUnitVectorMin) {
		*vel = gVecZero;
	}
	speed += 1.0f;
	if (speed > 30.0f) {
		speed = 30.0f;
	}
	gjMulAddVector(speed, &to, pos, vel);
	if (probeGroundLine(pos, &to, &hit, &normal) != 0) {
		gjScaleVector(-2.0f * njInnerProduct(vel, &normal), &normal, &normal);
		*vel  = vecSum(&normal, vel);
		speed = 2.0f;
		*pos  = hit;
		seStartAt(0xE3, pos);
	} else {
		*pos = to;
	}
	gjScaleVector(speed, vel, vel);
	if (mSmoke != NULL) {
		mSmoke->mPos = *pos;
	}
	if (mSparks != NULL) {
		mSparks->mPos = *pos;
	}
	return 0;
}

// 0x8001CAB8
EnemyTint::EnemyTint()
{
	clear();
}

// 0x8001CAE8
void EnemyTint::reset()
{
	clear();
}

// 0x8001CB08
void EnemyTint::start()
{
	mLevel  = 0;
	mPhase  = 0;
	mActive = 1;
}

// 0x8001CB20
void EnemyTint::clear()
{
	mLevel  = 0;
	mPhase  = 0;
	mActive = 0;
}

// 0x8001CB34
int EnemyTint::update()
{
	if (isActive() == 0) {
		return 1;
	}
	u32 level = mLevel;
	if ((mPhase & 1) == 0) {
		level += 0x28;
		if (level >= 0xA0) {
			mPhase++;
			level = 0xA0;
		}
	} else {
		level -= 0x28;
		if (level == 0) {
			mPhase++;
			level = 0;
		}
	}
	mLevel = level;
	if (mPhase >= 6) {
		clear();
		return 1;
	}
	return 0;
}

// 0x8001CBD4
int EnemyTint::isActive()
{
	return mActive == 1;
}

// 0x8001CBE8
void EnemyTint::getPhase(u32* out)
{
	*out = mPhase << 24;
}

// 0x8001CBF8
// Turns on the added colour with the level as its alpha
void EnemyTint::addToLight(u32* state)
{
	u32 level = mLevel;
	if (level != 0) {
		state[0] |= 0x20;
		state[4] = level << 24;
	}
}

// 0x8001CC1C
void spawnDropRun(Vec3f* pos, int count)
{
	spawnDropRunEx(pos, count, 0);
}

// 0x8001CC40
void spawnDropRunEx(Vec3f* pos, int count, int big)
{
	new TEnemyDropSpawner(gTL_04, pos, count, big);
}

// 0x8001CC98
TEnemyDropSpawner::TEnemyDropSpawner(TObject* parent, const Vec3f* pos, int count, int big)
    : TObject(parent)
{
	mTimer = 0;
	mCount = count;
	mPos   = *pos;
	mKind  = (int)(7.0f * (3.0517578e-05f * rand()));
	mBig   = big;
}

// 0x8001CD48
TEnemyDropSpawner::~TEnemyDropSpawner()
{
}

// 0x8001CDA4
void TEnemyDropSpawner::draw()
{
	mTimer--;
	if (mTimer <= 0) {
		createTObjEggFeedAt(mKind, &mPos, (u8)mBig);
		mTimer = 6;
		mCount--;
		if (mCount <= 0) {
			mFlags |= 1;
		}
	}
}
