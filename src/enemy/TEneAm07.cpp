#include "enemy/TEneAm07.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyCommon.h"
#include "enemy/EnemyAppear.h"
#include "engine/MotionPose.h"
#include "io/ResBank.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/EnemyHit.h"
#include "enemy/ObjEnemy.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "enemy/EnemyFx.h"
#include "Ninja/ninja.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "engine/TParticle.h"
#include "animal/RacerMover.h"
#include "enemy/TEneAm01.h"
struct Vec3f;
struct ColliShape;
struct ObjShadow;

extern "C" {
// 0x80242A40
// the nodes nodeTrackBeginPoints records
extern const int sAm07Nodes[] = { 0, 18, 35, 43, 44, 44, 45, 47, 55, 56, -1 };
}

static const char* sClassName = "TEneAm07";

extern "C" {
// The frame keys sAm07FrameKeys points at
extern const EnemyFrameKey sAm07KeyRise[2]    = { { 21.0f, 0 }, { -1.0f, 5 } };
extern const EnemyFrameKey sAm07KeyLand[2]    = { { 15.0f, 5 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAm07KeyTug[2]     = { { 15.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAm07KeyPull[3]    = { { 10.0f, 0 }, { 15.0f, 4 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAm07KeyAim[2]     = { { 15.0f, 0 }, { -1.0f, 5 } };
extern const EnemyFrameKey sAm07KeyRecover[2] = { { 9.0f, 5 }, { -1.0f, 0 } };

MotionEntry sAm07Motions[] = {
	{ MOTION_LOOP, 13, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 }, { MOTION_ONCE, 6, -1, 0, 0.0f, 24.0f, 0.4f, 0.0f, 0 },
	{ MOTION_BLEND, 7, -1, 3, 0.0f, 4.0f, 0.4f, 0.1f, 0 },  { MOTION_LOOP, 8, -1, 0, 0.0f, 10.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },  { MOTION_LOOP, 10, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 13.0f, 0.3f, 0.0f, 0 }, { MOTION_ONCE, 12, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },  { MOTION_LOOP, 1, -1, 0, 0.0f, 20.0f, 0.6f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 24.0f, 0.4f, 0.0f, 0 },  { MOTION_ONCE, 3, -1, 0, 0.0f, 24.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },  { MOTION_ONCE, 4, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },
};
const EnemyFrameKey* sAm07FrameKeys[] = { sAm07KeyRise, sAm07KeyLand, sAm07KeyTug, sAm07KeyPull, sAm07KeyAim, sAm07KeyRecover };
extern const ColliShape sAm07Shapes[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 13.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 7.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 14.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 8.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

inline void* TEneAm07::operator new(u32 size)
{
	return objHeapAlloc(gObjHeap, size);
}

inline void TEneAm07::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// Not in the DOL the linker stripped it
extern "C" f32 am07Double(int x)
{
	return 2.0f * (f32)x;
}

// 0x8006C548
extern "C" void initTEneAm07()
{
	resBankLoad(&sAm07Bank, "ar_ene_bee.arc");
	resBankAddTextures(&sAm07Bank, 0, "ene_bee.gvm");
	sAm07Models   = resBankGetModels(&sAm07Bank);
	sAm07Textures = resBankGetTextures(&sAm07Bank);
	sAm07Motion   = resBankGetMotion(&sAm07Bank, 0);
	sAm07Param    = (int*)enemyParamGet(12);
}

extern "C" void shutdownTEneAm07()
{
	sAm07Param = 0;
	resBankRemoveTextures(&sAm07Bank, 0);
	resBankRelease(&sAm07Bank);
	sAm07Models   = 0;
	sAm07Textures = 0;
	sAm07Motion   = 0;
}

extern "C" TObject* createTEneAm07(TObject* parent, SetDesc* desc)
{
	return new TEneAm07(parent, desc);
}

TEneAm07::TEneAm07(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00   = (u32)sClassName;
	mSize = 0x360;
	init(desc);
}

// 0x8006C714
void TEneAm07::init(SetDesc* desc)
{
	motionCtrlInit(&mAnim, sAm07Motions, sAm07Motion);
	motionCtrlPlay(&mAnim, 0);
	mBody.mRot.x = 0;
	mCollider    = colliCreate(4, 1);
	colliCopyShapes(mCollider, (const ColliShape*)sAm07Shapes);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 255, 0, 0);
	for (int i = 0; i < 4; i++) {
		colliShapeSetBit8(mCollider, i);
		colliShapeSetTag(mCollider, i, i);
	}
	mBrain = 0;
	readEnemyDesc(desc);
	mPose      = allocPose(*(void**)sAm07Models);
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags2    = 0;
	mPush.x    = 0.0f;
	mPush.y    = 0.0f;
	mPush.z    = 0.0f;
	mFrameKey  = 0;
	mHover     = 46.0f;
	mTurnVel   = 0.0f;
	mRadius    = 1.0f;
	mCylRadius = mRadius;
	for (int i = 0; i < 10; i++) {
		mPoints[i].x = 0.0f;
		mPoints[i].y = 0.0f;
		mPoints[i].z = 0.0f;
	}
	mCenter.x       = 0.0f;
	mCenter.y       = 0.0f;
	mCenter.z       = 0.0f;
	mAnchor         = mBody.mPos;
	mGoal.x         = 0.0f;
	mGoal.y         = 0.0f;
	mGoal.z         = 0.0f;
	mGroundPos      = mBody.mPos;
	mGroundNormal.x = 0.0f;
	mGroundNormal.y = 1.0f;
	mGroundNormal.z = 0.0f;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 18.0f * 1.4f;
	mShadow._34 = 18.0f * 1.4f;
	shadowEnable(&mShadow);
	mFx  = 0;
	mBar = new TEnemyLifeBar(this);
	mBar->setMax((s16)mHealth);
	mBar->setMark((s16)mHealthStep);
	mBar->setValue((s16)mHealth);
	mBar->setSize(10.0f, 25.0f);
	mLight.reset();
	mTint.reset();
	enemyEffectSetLarge(10.0f, &mEffect, &mBody.mPos);
	mSound     = 0;
	mPrevSound = mSound;
	seHolderStop(&mSe);
	mStep  = 0;
	mTimer = 0;
	mPhase = 0;
	_358   = 0;
	mDist  = 0.0f;
	createBrain();
}

// 0x8006C9CC
TEneAm07::~TEneAm07()
{
	removeDrawTask(mDrawTask);
	seHolderStop(&mSe);
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
	if (mBar != 0) {
		mBar->mFlags |= 1;
		mBar = 0;
	}
	freePose((void**)&mPose);
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x8006CAF4
void TEneAm07::setPos(const Vec3f* pos)
{
	f32 x        = pos->x;
	f32 y        = pos->y;
	mBody.mPos.x = x;
	f32 z        = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x8006CB38
f32 TEneAm07::vfn_40()
{
	return 60.0f;
}

// 0x8006CB40
// Shapes 0 and 1 sit on the first two points 2 is the body and 3 the tail
void TEneAm07::syncCollider()
{
	Vec3f* part = (Vec3f*)colliGetShapes(mCollider);
	Vec3f* body = &part[8];
	part[0]     = mPoints[0];
	part[4]     = mPoints[1];
	Vec3f v;
	v       = mPoints[5];
	mRadius = 14.0f * njScalor(&v);
	if ((mFlags2 & AM_CYLINDER) != 0) {
		*body = mPoints[6];
		colliShapeSetCylinder((ColliShape*)body);
		colliShapeSetCylRadius((ColliShape*)body, mCylRadius);
		colliShapeSetCylHeight((ColliShape*)body, 0.8f * (mPoints[4].y - mPoints[6].y));
	} else {
		*body = mCenter;
		colliShapeSetSphere((ColliShape*)body);
		colliShapeSetRadius((ColliShape*)body, mRadius);
	}
	v = mPoints[8];
	njAddVector(&v, &mPoints[9]);
	gjScaleVector(0.5f, &v, &v);
	part[12] = v;
	colliRefreshBounds(mCollider);
	colliBoundsOn(mCollider);
	f32 size    = 2.1f * mRadius;
	mShadow._30 = size;
	mShadow._34 = size;
}

// 0x8006CCC8
// Health defaults to 100
void TEneAm07::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	int health  = desc->getInt(0);
	mHealth     = health > 0 ? health : 100;
	mHealthStep = divideBy5(mHealth);
}

// 0x8006CD30
// Shape 3 is the stinger
int TEneAm07::stingDamage(int shape)
{
	int damage = 0;
	if ((mFlags2 & AM_STINGER) != 0) {
		if (shape == 3 || shape == 2) {
			damage = 20;
		}
	} else if (shape == 3 && (mFlags2 & AM_GUARDED) == 0) {
		damage = 20;
	}
	return damage;
}

// 0x8006CD70
// A player takes the sting's damage an egg 1
int TEneAm07::onHit(const TObjAction* other, int shape, int* result)
{
	int hit       = 0;
	HitInfo* info = (HitInfo*)result;
	if (isPlayerOrKind4(other) != 0) {
		int damage = stingDamage(shape);
		if (damage > 0) {
			info->mDamage = damage;
			hit           = 1;
			info->mFlags.clear();
		}
	} else if (isEggObj(other) != 0 && stingDamage(shape) > 0) {
		info->mDamage = 1;
		hit           = 1;
		info->mFlags.clear();
		info->mFlags.setBit(0xD);
	}
	return hit;
}

// 0x8006CE28
void TEneAm07::getHome(Vec3f* out) const
{
	*out = mHome;
}

// 0x8006CE44
void TEneAm07::setAnchor(const Vec3f* pos)
{
	mAnchor = *pos;
}

// 0x8006CE60
void TEneAm07::setGoal(const Vec3f* pos)
{
	mGoal = *pos;
}

// 0x8006CE7C
void TEneAm07::noteDeath()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
}

int TEneAm07::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

int TEneAm07::isDiveDone()
{
	return isActionDone(ACT_DIVE);
}

int TEneAm07::isStingDone()
{
	return isActionDone(ACT_STING);
}

int TEneAm07::isPullDone()
{
	return isActionDone(ACT_PULL);
}

// 0x8006CF4C
int TEneAm07::canFlip()
{
	return !(mFlags2 & AM_STINGING);
}

// 0x8006CF60
void TEneAm07::noteFlip()
{
	noteDeath();
	seStartAt(0xF4, &mBody.mPos);
}

int TEneAm07::isFlipDone()
{
	return isActionDone(ACT_FLIP);
}

// 0x8006CFBC
void TEneAm07::noteDie()
{
	noteDeath();
	seStartAt(0xF5, &mBody.mPos);
}

// 0x8006CFF4
// Stuck in the ground it sinks there
int TEneAm07::deathAction()
{
	return ((mFlags2 & 0x40) ? -1 : 0) + ACT_FALL;
}

// 0x8006D008
void TEneAm07::stopEffect()
{
	enemyEffectStart(&mEffect);
}

// Not in the DOL the linker stripped it
extern "C" int am07Passed3(TEneAm07* am)
{
	return motionCtrlPassed(3.0f, &am->mAnim);
}

// 0x8006D02C
void TEneAm07::updateBody()
{
	if (isBlocked() == 0 && isInRange(this, &mBody.mPos, 500.0f) == 0) {
		resetLink();
		mFlags |= 1;
		return;
	}
	if (isBlocked() == 0 && takeHits(&mHit, &mContact) != 0) {
		applyDamage(&mHit);
	}
	mBrain->update(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	syncCollider();
	mTint.update();
	if (mSound != mPrevSound) {
		seHolderStop(&mSe);
	}
	if (mSound == SOUND_BUZZ) {
		seHolderPlay(&mSe, 0xF0, &mBody.mPos);
	} else if (mSound == SOUND_DIVE) {
		seHolderPlay(&mSe, 0xF1, &mBody.mPos);
	} else {
		seHolderStop(&mSe);
	}
	mPrevSound = mSound;
	mBar->setPos(&mBody.mPos);
	mBar->setValue((s16)mHealth);
	enemyEffectUpdate(&mEffect);
}

// 0x8006D1BC
// returnZero is 0 so this never runs
void TEneAm07::vfn_6C()
{
	if (returnZero() != 0) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		mHover = 46.0f;
		beginPoints();
		gjPushUnitMatrix();
		hoverMatrix();
		poseFirstFrame(sAm07Motion, 0xE, mPose);
		njPopMatrixEx();
		endPoints();
		placePoints();
		syncCollider();
	}
}

// 0x8006D250
void TEneAm07::createBrain()
{
	mBrain = Am07BeeBrain::create(this);
}

// 0x8006D280
// Point 5 starts as a unit axis so its posed length is the model's scale
void TEneAm07::beginPoints()
{
	for (int i = 0; i < 10; i++) {
		mPoints[i].x = 0.0f;
		mPoints[i].y = 0.0f;
		mPoints[i].z = 0.0f;
	}
	mPoints[5].x = 0.0f;
	mPoints[5].y = 0.0f;
	mPoints[5].z = 1.0f;
	nodeTrackBeginPoints(sAm07Nodes, mPoints);
}

// 0x8006D2EC
void TEneAm07::endPoints()
{
	nodeTrackEnd();
}

// 0x8006D30C
void TEneAm07::pose()
{
	beginPoints();
	void* pose = mPose;
	gjPushUnitMatrix();
	hoverMatrix();
	applyPose(pose, &mAnim);
	njPopMatrixEx();
	if (hatchActive(&mAppearTimer) != 0) {
		hatchPose(&mAppearTimer, (GjActor*)pose, 60.0f);
	}
	endPoints();
}

// 0x8006D384
// Moves the points into the world
void TEneAm07::placePoints()
{
	Vec3f axis;
	axis.x = mPoints[5].x;
	axis.y = mPoints[5].y;
	axis.z = mPoints[5].z;
	njSubVector(&axis, &mPoints[4]);
	gjPushUnitMatrix();
	bodyMatrix();
	njCalcPoints(0, mPoints, mPoints, 10);
	njPopMatrixEx();
	mPoints[5] = axis;
	mCenter    = mPoints[4];
	njAddVector(&mCenter, &mPoints[6]);
	gjScaleVector(0.5f, &mCenter, &mCenter);
}

// 0x8006D440
void TEneAm07::animate()
{
	pose();
	placePoints();
}

// 0x8006D474
// Only a kind-3 hit carries its damage
int TEneAm07::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x8006D48C
void TEneAm07::bodyMatrix()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
	gjRotateZ(mBody.mRot.z);
	if ((mFlags2 & AM_PITCH) != 0) {
		gjRotateX(mBody.mRot.x);
	}
}

// 0x8006D4E0
void TEneAm07::hoverMatrix()
{
	gjTranslate(0.0f, mHover, 0.0f);
}

// 0x8006D50C
// Moves across the ground by vel
int TEneAm07::moveFlat(f32 bounce, Vec3f* vel)
{
	Vec3f dir;
	Vec3f start;
	start.x = mCenter.x;
	start.y = mCenter.y;
	start.z = mCenter.z;
	dir.x   = vel->x;
	dir.y   = vel->y;
	dir.z   = vel->z;
	dir.y   = 0.0f;
	f32 len = njUnitVector(&dir);
	if (len < gUnitVectorMin) {
		dir = gVecZero;
	}
	Vec3f end;
	gjMulAddVector(len + mRadius, &end, &start, &dir);
	Vec3f hit;
	Vec3f normal;
	if (probeGroundLine(&start, &end, &hit, &normal) != 0) {
		hit.x   = hit.x - dir.x * mRadius;
		hit.z   = hit.z - dir.z * mRadius;
		start.x = start.x - hit.x;
		start.z = start.z - hit.z;
		mBody.mPos.x += start.x;
		mBody.mPos.z += start.z;
		normal.y = 0.0f;
		if (njUnitVector(&normal) < gUnitVectorMin) {
			normal = gVecZero;
		}
		f32 dot = normal.x * dir.x + normal.z * dir.z;
		dot *= 2.0f;
		gjMulAddVector(dot, vel, &dir, &normal);
		gjScaleVector(bounce, vel, vel);
		return 1;
	}
	mBody.mPos.x += vel->x;
	mBody.mPos.z += vel->z;
	return 0;
}

// 0x8006D6E4
void TEneAm07::getCenter(Vec3f* out)
{
	*out = mCenter;
}

// 0x8006D700
void TEneAm07::getHead(Vec3f* out)
{
	*out = mPoints[2];
}

// 0x8006D71C
void TEneAm07::getTail(Vec3f* out)
{
	*out = mPoints[9];
}

// 0x8006D738
// Finds the ground under the body and the shadow goes there
int TEneAm07::updateGround()
{
	Vec3f center;
	getCenter(&center);
	Vec3f pos;
	Vec3f normal;
	int hit = probeGround(&center, &pos, &normal);
	if (hit == 0) {
		shadowDisable(&mShadow);
	} else {
		mGroundPos    = pos;
		mGroundNormal = normal;
		shadowEnable(&mShadow);
		shadowSetGround(&mShadow, &pos, &normal);
	}
	return hit;
}

// 0x8006D7D8
// Moves up or down by dy stopping the head at a ceiling and the tail at the ground
int TEneAm07::moveVertical(f32 damp, const Vec3f* head, const Vec3f* tail, f32* dy)
{
	Vec3f top;
	Vec3f bottom;
	Vec3f stop;
	Vec3f center;
	Vec3f up;
	if (head == 0) {
		getHead(&top);
	} else {
		top = *head;
	}
	if (tail == 0) {
		getTail(&bottom);
	} else {
		bottom = *tail;
	}
	top.y += *dy;
	bottom.y += *dy;
	getCenter(&center);
	up          = center;
	up.y        = top.y;
	stop        = top;
	int ceiling = probeGroundLine(&center, &up, &stop, 0);
	f32 down    = stop.y - top.y;
	int floor   = updateGround();
	f32 lift    = mGroundPos.y - bottom.y;
	if (floor != 0) {
		if (lift > 0.0f) {
			floor = 1;
		} else {
			floor = 0;
		}
	} else {
		lift  = 0.0f;
		floor = 0;
	}
	if (ceiling != 0 && floor != 0) {
		mBody.mPos.y += 0.5f * (down + lift);
		*dy = 0.0f;
	} else if (ceiling != 0) {
		mBody.mPos.y += down;
		if (*dy > 0.0f) {
			*dy *= damp;
		}
	} else if (floor != 0) {
		mBody.mPos.y += lift;
		if (*dy < 0.0f) {
			*dy *= damp;
		}
	} else {
		mBody.mPos.y += *dy;
	}
	return ceiling != 0 || floor != 0;
}

// 0x8006DA04
int TEneAm07::moveVertical(f32 damp, f32* dy)
{
	return moveVertical(damp, 0, 0, dy);
}

// 0x8006DA30
void TEneAm07::move()
{
	moveFlat(-0.8f, &mBody.mVel);
	moveVertical(-0.8f, &mBody.mVel.y);
}

// 0x8006DA74
// Turns toward angle through a spring on mTurnVel
int TEneAm07::turnToward(int angle, f32 a, f32 b, f32 c, f32 d, f32 e)
{
	int yaw      = mBody.mRot.y;
	int diff     = angleDiff(yaw, angle);
	mTurnVel     = approachSpeed(mTurnVel, (f32)diff, a, b, c, d, e);
	mBody.mRot.y = yaw + (int)mTurnVel;
	return diff;
}

// 0x8006DB68
int TEneAm07::turnSlow(int angle)
{
	return turnToward(angle, 0.05f, 30.0f, 40.0f, 512.0f, 10.0f);
}

// 0x8006DB9C
int TEneAm07::goalAngle()
{
	Vec3f to;
	to.x = mGoal.x;
	to.y = mGoal.y;
	to.z = mGoal.z;
	njSubVector(&to, &mBody.mPos);
	to.y = 0.0f;
	return (int)(10430.38043493439 * (f32)atan2(to.x, to.z));
}

// 0x8006DC0C
int TEneAm07::turnToGoal()
{
	int angle = goalAngle();
	return turnSlow(angle);
}

// 0x8006DC48
int TEneAm07::turnToGoalFast()
{
	return turnToward(goalAngle(), 0.3f, 50.0f, 60.0f, 1300.0f, 10.0f);
}

// 0x8006DC94
// Leans into the turn
void TEneAm07::bank()
{
	mBody.mRot.z = (int)(-10.0f * mTurnVel);
}

// 0x8006DCBC
void TEneAm07::levelOut()
{
	mBody.mRot.z = angleApproach(mBody.mRot.z, 0, 0x100);
}

// 0x8006DCF8
// Picks a side of the goal to slide to 1 or 2 and turns back at a wall
int TEneAm07::strafe(int* side, f32* dist, Vec3f* out)
{
	Vec3f dir;
	if (*side <= 0) {
		*dist = 0.0f;
		if (3.0517578e-5f * rand() < 0.5f) {
			*side = 1;
		} else {
			*side = 2;
		}
	}
	Vec3f from;
	from.x = mBody.mPos.x;
	from.y = 0.5f * (mPoints[0].y + mPoints[8].y);
	from.z = mBody.mPos.z;
	dir    = mGoal;
	njSubVector(&dir, &mBody.mPos);
	dir.y = 0.0f;
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 x = dir.x;
	dir.x = dir.z;
	dir.z = -x;
	if (*side == 2) {
		gjScaleVector(-1.0f, &dir, &dir);
	}
	Vec3f to;
	gjMulAddVector(50.0f, &to, &from, &dir);
	int blocked = 0;
	Vec3f hit;
	if (probeGroundLine(&from, &to, &hit, 0) != 0) {
		njSubVector(&hit, &from);
		f32 d = njScalor2(&hit);
		if (*dist <= 0.0f) {
			*dist   = d;
			blocked = 1;
			if (*side == 1) {
				*side = 2;
			} else {
				*side = 1;
			}
		} else if (!(*dist >= d)) {
			gjScaleVector(-1.0f, &dir, &dir);
		}
	}
	if (blocked != 0) {
		return 0;
	}
	*out = dir;
	return 1;
}

// 0x8006DF0C
// Drifts to mAnchor and bobs between two heights over it
void TEneAm07::wander(int* angle, int* rising)
{
	Vec3f to;
	Vec3f vel;
	vel   = mBody.mVel;
	vel.y = 0.0f;
	to    = mAnchor;
	njSubVector(&to, &mBody.mPos);
	to.y = 0.0f;
	if (springVelocity(0.08f, 0.9f, &vel, 0.1f, &to, 0.1f, 5.0f, 0.05f) < 2.0f) {
		mFlags2 |= AM_STALLED;
	} else {
		mFlags2 &= ~AM_STALLED;
		*angle = (int)(10430.38043493439 * (f32)atan2(to.x, to.z));
	}
	mBody.mVel.x = vel.x;
	mBody.mVel.z = vel.z;
	int wall     = moveFlat(-0.8f, &mBody.mVel);
	turnSlow(*angle);
	bank();
	Vec3f tail;
	getTail(&tail);
	f32 height = 5.0f;
	if (*rising == 0) {
		height += 7.5f;
	}
	height -= tail.y - mAnchor.y;
	f32 vy = approachSpeed(mBody.mVel.y, height, 0.05f, 0.02f, 0.02f, 0.8f, 0.02f);
	if ((f32)__fabs(height) < 1.0f) {
		*rising = *rising ? 0 : 1;
	}
	mBody.mVel.y = vy;
	if (wall != 0) {
		if (mAnchor.y - tail.y > 0.0f) {
			mBody.mVel.y += 5.0f;
		} else {
			mBody.mVel.y -= 5.0f;
		}
	}
	if (moveVertical(-0.8f, &mBody.mVel.y) != 0) {
		*rising = *rising ? 0 : 1;
	}
}

// 0x8006E12C
// Sways side to side above mAnchor
void TEneAm07::hover(u32* phase)
{
	*phase += 0x300;
	*phase &= 0xFFFF;
	f32 side   = 25.0f * njSin(*phase);
	f32 height = 8.0f * (0.8f + njSin(*phase << 1)) + 46.0f;
	f32 s;
	f32 c;
	njSinCos(mBody.mRot.y, &s, &c);
	Vec3f to;
	to.x = side * c;
	to.y = height;
	to.z = side * -s;
	njAddVector(&to, &mAnchor);
	njSubVector(&to, &mBody.mPos);
	springVelocity(0.1f, 0.9f, &mBody.mVel, 0.08f, &to, 0.08f, 2.0f, 1.0f);
	move();
}

// 0x8006E234
// Charges at mGoal and down to it
int TEneAm07::dive(int* timer)
{
	int done = 0;
	Vec3f to;
	to = mGoal;
	njSubVector(&to, &mBody.mPos);
	to.y         = 0.0f;
	mBody.mRot.y = angleApproach(mBody.mRot.y, (int)(10430.38043493439 * (f32)atan2(to.x, to.z)), 0x200);
	if (njUnitVector(&to) < gUnitVectorMin) {
		to = gVecZero;
	}
	Vec3f vel;
	vel       = mBody.mVel;
	vel.y     = 0.0f;
	f32 along = njInnerProduct(&vel, &to);
	gjMulAddVector(-along, &vel, &vel, &to);
	along += 0.12f;
	if (along >= 8.0f) {
		along = 8.0f;
	}
	if (njScalor2(&vel) < 2.25f) {
		gjScaleVector(0.9f, &vel, &vel);
	} else {
		gjScaleVector(0.96f, &vel, &vel);
	}
	gjMulAddVector(along, &vel, &vel, &to);
	(*timer)--;
	if (along < 0.0f) {
		if (*timer <= 0) {
			done = 1;
		}
	} else {
		mBody.mRot.x = (int)(1500.0f * along);
	}
	mBody.mVel.x = vel.x;
	mBody.mVel.z = vel.z;
	moveFlat(-0.8f, &mBody.mVel);
	Vec3f tail;
	getTail(&tail);
	tail.y += 10.0f;
	f32 vy   = mBody.mVel.y;
	f32 gap  = mGoal.y - tail.y;
	f32 dist = gap;
	if (gap < 0.0f) {
		dist *= -1.0f;
		vy *= -1.0f;
	}
	if (dist > 10.0f) {
		vy += 0.15f;
		if (vy > 7.0f) {
			vy = 7.0f;
		}
	} else {
		vy *= 0.8f;
	}
	if (gap < 0.0f) {
		vy *= -1.0f;
	}
	mBody.mVel.y = vy;
	moveVertical(-0.8f, 0, &tail, &mBody.mVel.y);
	return done;
}

// 0x8006E4BC
// Flies at mGoal slowing down while it turns
void TEneAm07::chase()
{
	Vec3f to;
	Vec3f vel;
	vel       = mBody.mVel;
	vel.y     = 0.0f;
	f32 speed = njUnitVector(&vel);
	if (speed < gUnitVectorMin) {
		vel = gVecZero;
	}
	to = mGoal;
	njSubVector(&to, &mBody.mPos);
	to.y     = 0.0f;
	f32 dist = njUnitVector(&to);
	if (dist < gUnitVectorMin) {
		to = gVecZero;
	}
	f32 reach = dist;
	reach += 25.0f;
	int angle = (int)(10430.38043493439 * (f32)atan2(to.x, to.z));
	turnSlow(angle);
	int left;
	int heading = angleApproachDiff((int)(10430.38043493439 * (f32)atan2(vel.x, vel.z)), angle, 0x180, &left);
	if (left < 0x1800) {
		reach *= 0.02f;
		speed += 0.1f;
		if (reach > 2.0f) {
			reach = 2.0f;
		}
		if (speed > reach) {
			speed = reach;
		}
	} else {
		speed -= 0.3f;
		if (speed < 0.0f) {
			speed = 0.0f;
		}
	}
	njSinCos(heading, &vel.x, &vel.z);
	mBody.mVel.x = vel.x * speed;
	mBody.mVel.z = vel.z * speed;
	moveFlat(-0.8f, &mBody.mVel);
}

// 0x8006E6AC
void TEneAm07::onActionPhase(int action, int phase)
{
	switch (action) {
	case ACT_APPEAR:
		actAppear(phase);
		break;
	case ACT_WANDER:
		actWander(phase);
		break;
	case ACT_HOVER:
		actHover(phase);
		break;
	case ACT_DIVE:
		actDive(phase);
		break;
	case ACT_STING:
		actSting(phase);
		break;
	case ACT_STUCK:
		actStuck(phase);
		break;
	case ACT_PULL:
		actPull(phase);
		break;
	case ACT_FLIP:
		actFlip(phase);
		break;
	case ACT_SINK:
		actSink(phase);
		break;
	case ACT_FALL:
		actFall(phase);
		break;
	}
}

// 0x8006E75C
void TEneAm07::actAppear(int phase)
{
	int* step = &mStep;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0);
		beginAppear(step);
		mHover = 46.0f;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		runAppear(step);
		animate();
		break;
	case 2:
		endAppear(step);
		mFrameKey = 0;
		mBody.mPos.y += mHover;
		mHover = 0.0f;
		break;
	}
}

// 0x8006E818
void TEneAm07::actWander(int phase)
{
	int* angle  = &mPhase;
	int* rising = &mStep;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0);
		updateGround();
		*angle  = 0;
		*rising = 0;
		mFlags2 &= ~AM_STALLED;
		clearFrameKey();
		mSound = SOUND_BUZZ;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		wander(angle, rising);
		animate();
		break;
	case 2:
		mFlags2 &= ~AM_STALLED;
		mSound = SOUND_NONE;
		break;
	}
}

// 0x8006E8E4
// Hovers over the ground it found facing the goal
void TEneAm07::actHover(int phase)
{
	u32* sway = (u32*)&mPhase;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0);
		updateGround();
		mAnchor   = mGroundPos;
		mFrameKey = 5;
		mSound    = SOUND_BUZZ;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		turnToGoal();
		bank();
		hover(sway);
		animate();
		break;
	case 2:
		clearFrameKey();
		mSound = SOUND_NONE;
		break;
	}
}

// 0x8006E9B0
// Winds up facing the goal slides to one side then charges with a trail behind it
void TEneAm07::actDive(int phase)
{
	int* timer = &mTimer;
	f32* dist  = &mDist;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 8);
		mFlags2 |= AM_PITCH;
		updateGround();
		mStep  = 0;
		*timer = 0;
		*dist  = 0.0f;
		mBody.mVel.y += 0.2f;
		mBody.mRot.x = 0;
		clearFrameKey();
		mSound = SOUND_BUZZ;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		switch (mStep) {
		case 0: {
			setFrameKey(sAm07FrameKeys[4]);
			int left = turnToGoalFast();
			levelOut();
			if (motionCtrlIsDone(&mAnim) != 0 && left < 0x400 && left > -0x400) {
				motionCtrlPlay(&mAnim, 9);
				mStep     = 1;
				*timer    = 0;
				*dist     = 0.0f;
				mFrameKey = 4;
			}
			gjScaleVector(0.92f, &mBody.mVel, &mBody.mVel);
			move();
			animate();
			break;
		}
		case 1: {
			turnToGoalFast();
			levelOut();
			gjScaleVector(0.92f, &mBody.mVel, &mBody.mVel);
			Vec3f dir;
			if (strafe(timer, dist, &dir) != 0) {
				mStep  = 2;
				*timer = 60;
				*dist  = 1000000.0f;
				gjScaleVector(6.0f, &dir, &dir);
				mBody.mVel.x = dir.x;
				mBody.mVel.z = dir.z;
				mFlags2 |= AM_PITCH;
				mSound = SOUND_DIVE;
				seStartAt(0xF2, &mBody.mPos);
			}
			move();
			animate();
			break;
		}
		case 2: {
			levelOut();
			int done = dive(timer);
			animate();
			Vec3f tail;
			tail.x = mPoints[9].x;
			tail.y = mPoints[9].y;
			tail.z = mPoints[9].z;
			tail.y += 10.0f;
			int heading = (int)(10430.38043493439 * (f32)atan2(mBody.mVel.x, mBody.mVel.z));
			particleKeepAtRot((void**)&mFx, &tail, -0x4000, heading, 0, 0x6A);
			if ((*timer & 7) == 0) {
				tail.x = 2.5f * mBody.mVel.x + tail.x;
				tail.z = 2.5f * mBody.mVel.z + tail.z;
				fxSpawnScaled(2.0f, 1.0f, 2.0f, 5, (void*)0x14, (const Vec3f*)&tail, 0, heading);
			}
			if (done != 0) {
				mStep  = 3;
				*timer = 5;
				if (mFx != 0) {
					mFx->mFlags |= 1;
					mFx = 0;
				}
			}
			break;
		}
		case 3:
			levelOut();
			mBody.mRot.x = angleApproach(mBody.mRot.x, 0, 0x180);
			(*timer)--;
			if (*timer == 0) {
				motionCtrlPlayBlend(0.1f, &mAnim, 10);
				motionCtrlUpdate(&mAnim);
			}
			if (*timer <= 0) {
				setFrameKey(sAm07FrameKeys[5]);
			}
			gjScaleVector(0.9f, &mBody.mVel, &mBody.mVel);
			move();
			waitMotion();
			animate();
			break;
		}
		break;
	case 2:
		mFlags2 &= ~AM_PITCH;
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		clearFrameKey();
		mSound = SOUND_NONE;
		break;
	}
}

// 0x8006EDBC
// Faces the goal rises jumps over it in an arc and drives the stinger into the ground
void TEneAm07::actSting(int phase)
{
	f32 jump;
	Vec3f to;
	Vec3f tip;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0);
		mFlags2 |= AM_STINGING;
		updateGround();
		mStep  = 0;
		mTimer = 0;
		mSound = SOUND_DIVE;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		levelOut();
		switch (mStep) {
		case 0: {
			int left = turnToGoal();
			levelOut();
			if (left < 0x400 && left > -0x400) {
				mStep = 1;
				motionCtrlPlay(&mAnim, 1);
			}
			break;
		}
		case 1:
			turnToGoal();
			setFrameKey(sAm07FrameKeys[0]);
			gjScaleVector(0.92f, &mBody.mVel, &mBody.mVel);
			moveFlat(-0.8f, &mBody.mVel);
			moveVertical(-0.01f, &mBody.mVel.y);
			if (motionCtrlPassed(7.0f, &mAnim) != 0) {
				quadRoots(1.0f, -0.02f, -2.0f, &jump, 0);
				mStep        = 2;
				mBody.mVel.y = jump;
				mDist        = jump / 0.02f;
				to.x         = mGoal.x;
				to.y         = mGoal.y;
				to.z         = mGoal.z;
				njSubVector(&to, &mBody.mPos);
				to.y    = 0.0f;
				f32 len = njUnitVector(&to);
				if (len < gUnitVectorMin) {
					to = gVecZero;
				}
				f32 speed = len / mDist;
				speed *= 0.5f;
				if (speed > 2.0f) {
					speed = 2.0f;
				}
				gjScaleVector(speed, &to, &to);
				mBody.mVel.x = to.x;
				mBody.mVel.z = to.z;
			}
			if (motionCtrlPassed(3.0f, &mAnim) != 0) {
				seStartAt(0xF3, &mBody.mPos);
			}
			break;
		case 2:
			chase();
			mBody.mVel.y += -0.02f;
			moveVertical(-0.01f, &mBody.mVel.y);
			if (mBody.mVel.y <= 0.0f) {
				mStep = 3;
				motionCtrlPlay(&mAnim, 2);
				mFlags2 |= AM_STINGER | AM_CYLINDER;
				mCylRadius = mRadius;
				mAnchor    = mGoal;
				mFrameKey  = 4;
			}
			break;
		case 3: {
			chase();
			mBody.mVel.y += -0.02f;
			tip.x = mPoints[7].x;
			tip.y = mPoints[7].y;
			tip.z = mPoints[7].z;
			tip.y += 3.0f;
			tip.y += -0.02f;
			if (moveVertical(-0.01f, 0, &tip, &mBody.mVel.y) != 0) {
				mStep = 4;
				motionCtrlPlay(&mAnim, 4);
				mBody.mVel.y = 0.0f;
				mFlags2 |= AM_STUCK;
				particleOnceByIdDefault(&tip, 0, 0x46);
				mFlags2 &= ~AM_STINGER;
				mSound = SOUND_NONE;
				seStartAt(0x10E, &mBody.mPos);
			}
			break;
		}
		case 4:
			waitMotion();
			setFrameKey(sAm07FrameKeys[1]);
			break;
		}
		animate();
		break;
	case 2:
		mFlags2 &= ~(AM_STINGER | AM_CYLINDER | AM_STINGING | AM_STUCK);
		clearFrameKey();
		mSound = SOUND_NONE;
		break;
	}
}

// 0x8006F198
void TEneAm07::actStuck(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 5);
		updateGround();
		mBody.mRot.x = 0;
		mBody.mRot.z = 0;
		mFlags2 |= AM_GUARDED | AM_CYLINDER | AM_STINGING | AM_STUCK;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		animate();
		break;
	case 2:
		mFlags2 &= ~(AM_GUARDED | AM_CYLINDER | AM_STINGING | AM_STUCK);
		clearFrameKey();
		break;
	}
}

// 0x8006F240
// Tugs then pulls the stinger free on frame 12
void TEneAm07::actPull(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 6);
		updateGround();
		mBody.mRot.x = 0;
		mBody.mRot.z = 0;
		mFlags2 |= AM_STINGING | AM_STUCK;
		mStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		switch (mStep) {
		case 0:
			if (motionCtrlIsDone(&mAnim) != 0) {
				mStep = 1;
				motionCtrlPlayBlend(0.1f, &mAnim, 7);
			}
			setFrameKey(sAm07FrameKeys[2]);
			if (motionCtrlPassed(5.0f, &mAnim) != 0) {
				seStartAt(0xF3, &mBody.mPos);
			}
			break;
		case 1:
			setFrameKey(sAm07FrameKeys[3]);
			if (motionCtrlFrame(&mAnim) >= 12.0f) {
				mFlags2 &= ~AM_STUCK;
			}
			waitMotion();
			break;
		}
		animate();
		break;
	case 2:
		mFlags2 &= ~(AM_STINGING | AM_STUCK);
		clearFrameKey();
		break;
	}
}

// 0x8006F3A4
void TEneAm07::actFlip(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 11);
		noteFlip();
		mEnemyFlags |= ENEMY_FLIPPING;
		mFrameKey = 3;
		updateGround();
		mBody.mRot.x = 0;
		mBody.mRot.z = 0;
		mBody.mVel.x = 0.4f * mPush.x + mBody.mVel.x;
		mBody.mVel.z = 0.4f * mPush.z + mBody.mVel.z;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		waitMotion();
		gjScaleVector(0.92f, &mBody.mVel, &mBody.mVel);
		move();
		animate();
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		clearFrameKey();
		break;
	}
}

// 0x8006F498
void TEneAm07::beginDeath()
{
	mEnemyFlags |= ENEMY_INTANGIBLE | ENEMY_BLOCKED;
	colliDisable(mCollider);
	mBar->hide();
	killed(1);
	noteDie();
	mFrameKey = 3;
	updateGround();
	mBody.mRot.x = 0;
	mBody.mRot.z = 0;
}

// 0x8006F50C
// Dies where it is stuck and leaves a drop a second after the motion ends
void TEneAm07::actSink(int phase)
{
	switch (phase) {
	case 0:
		beginDeath();
		motionCtrlPlayBlend(0.05f, &mAnim, 12);
		mStep = 60;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updateGround();
		if (motionCtrlIsDone(&mAnim) != 0) {
			mStep--;
			if (mStep <= 0) {
				enemyFxDeath4A(&mPoints[7]);
				spawnDropB(&mPoints[7]);
				mFlags |= 1;
			}
		}
		animate();
		break;
	case 2:
		mEnemyFlags &= ~(ENEMY_INTANGIBLE | ENEMY_BLOCKED);
		clearFrameKey();
		break;
	}
}

// 0x8006F5EC
// Pops up falls to the ground and leaves a drop
void TEneAm07::actFall(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 13);
		beginDeath();
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		mStep        = 0;
		mDist        = mBody.mPos.y;
		mBody.mVel.y = 2.0f;
		break;
	case 1:
		switch (mStep) {
		case 0:
			motionCtrlUpdate(&mAnim);
			if (motionCtrlPassed(17.0f, &mAnim) != 0) {
				mStep = 1;
			}
		case 1: {
			f32 drop;
			f32 vy = mBody.mVel.y;
			vy += -0.1f;
			drop = mCenter.y - 18.0f;
			if (updateGround() != 0) {
				f32 floor = mGroundPos.y - drop;
				if (vy < floor) {
					mStep  = 2;
					vy     = floor;
					mTimer = 60;
					particleOnceByIdDefault(&mGroundPos, 0, 0x46);
					seEnemyPuff(&mBody.mPos);
				}
			} else if (mBody.mPos.y - mDist < 100.0f) {
				mFlags |= 1;
			}
			mBody.mVel.y = vy;
			mBody.mPos.y += vy;
			break;
		}
		case 2:
			motionCtrlUpdate(&mAnim);
			if (motionCtrlIsDone(&mAnim) != 0) {
				mTimer--;
				if (mTimer <= 0) {
					enemyFxDeath4A(&mPoints[7]);
					spawnDropB(&mPoints[7]);
					mFlags |= 1;
				}
			}
			break;
		}
		animate();
		break;
	case 2:
		mEnemyFlags &= ~(ENEMY_INTANGIBLE | ENEMY_BLOCKED);
		clearFrameKey();
		break;
	}
}

// 0x8006F7F0
void TEneAm07::vfn_3C()
{
	njPrint((kEnemyInfoX << 16) | kEnemyInfoLine, ":HP %4d", mHealth);
}

// 0x8006F82C
// Hidden while appearing or out of view
void TEneAm07::vfn_10()
{
	if (isAppearing() != 0) {
		mFlags |= 0x10;
	} else if (mLight.isVisible(mDrawTask, 500.0f, &mBody.mPos, 70.0f, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x8006F8BC
void TEneAm07::render()
{
	njSetTexture(*(void**)sAm07Textures);
	enemyEyeSet(*(NJS_TEXLIST**)sAm07Textures, 0, mFrameKey);
	useArea();
	njPushMatrixEx();
	bodyMatrix();
	drawLit(&mLight, &mTint, mPose);
	njPopMatrixEx();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}
