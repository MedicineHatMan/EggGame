#include "enemy/TEneAm02.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyCommon.h"
#include "egg/ObjEgg.h"
#include "engine/MotionPose.h"
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/ObjEnemy.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "engine/Colli.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "enemy/EnemyFx.h"
#include "engine/GroundQuery.h"
#include "io/ResBank.h"
#include "enemy/EnemyHit.h"
#include "camera/TCameraCtrl.h"
#include "engine/TParticle.h"
#include "enemy/TEneAm01.h"
#include "core/MathUtil.h"
struct EnemyFx;
struct Vec3f;
struct ColliShape;
struct ResBank;
struct ObjShadow;

extern "C" {
// 0x80240BA0
// Nothing in this file reads it
extern const f32 sAm02Rates[4] = { 0.3f, 0.4f, 0.2f, 0.1f };
extern const int sAm02Bones[]  = {
	17, 24, 28, 38, 51, -1,
}; // the nodes nodeTrackBeginOrigins records
MotionEntry sAm02Motions[] = {
	{ MOTION_LOOP, 6, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },  { MOTION_LOOP, 5, -1, 0, 0.0f, 25.0f, 1.2f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 39.0f, 0.25f, 0.0f, 0 }, { MOTION_ONCE, 1, -1, 0, 0.0f, 49.0f, 0.25f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 9.0f, 0.6f, 0.0f, 0 },   { MOTION_ONCE, 3, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },
};
}

static const char* sClassName = "TEneAm02";

// The frame keys sAm02FrameKeys points at
extern const EnemyFrameKey sAm02KeysA[3] = { { 2.0f, 0 }, { 34.0f, 5 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAm02KeysB[3] = { { 3.0f, 0 }, { 34.0f, 4 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAm02KeysC[3] = { { 1.0f, 0 }, { 19.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAm02KeysD[3] = { { 1.0f, 0 }, { 19.0f, 3 }, { -1.0f, 2 } };

// 0x80254674
// after sAm02Motions
extern "C" const EnemyFrameKey* sAm02FrameKeys[] = { sAm02KeysA, sAm02KeysB, sAm02KeysC, sAm02KeysD };

// The body's two shapes as they start out copied into sAm02Shapes at load
static const ColliShape sAm02ShapeData[2] = {
	{ { 0.0f, 27.0f, 0.0f }, 0x0, 0, 0x0, 15.0f, 27.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 13.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};

extern const ColliShape sAm02SwingShapes[1] = {
	{ { 0.0f, 5.0f, 0.0f }, 0x0, 0, 0x0, 80.0f, 10.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};

// Radians to angle units
static inline int toAngle(f32 x, f32 z)
{
	return (int)(10430.38043493439 * (f32)atan2(x, z));
}

// The world's objects are freed by the task system once this flag is up
template <typename T>
static inline void killFx(T*& fx)
{
	if (fx != 0) {
		fx->mFlags |= 1;
		fx = 0;
	}
}

inline void* TEneAm02::operator new(u32 size)
{
	return objHeapAlloc(gObjHeap, size);
}

inline void TEneAm02::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

static ResBank sAm02Bank;           // 0x8027CB08
static ColliShape sAm02Shapes[2];   // 0x8027CB14
static ColliShape sAm02Bounds;      // 0x8027CB74
static ColliShape sAm02SwingBounds; // 0x8027CBA4

// 0x8001F7C0
extern "C" void initTEneAm02()
{
	resBankLoad(&sAm02Bank, "ar_ene_am02.arc");
	resBankAddTextures(&sAm02Bank, 0, "ene_am02.gvm");
	sAm02Models   = resBankGetModels(&sAm02Bank);
	sAm02Textures = resBankGetTextures(&sAm02Bank);
	sAm02Motion   = resBankGetMotion(&sAm02Bank, 0);

	for (int i = 0; i < 2; i++) {
		((u32*)&sAm02Shapes[i].mPos)[0] = ((const u32*)&sAm02ShapeData[i].mPos)[0];
		((u32*)&sAm02Shapes[i].mPos)[1] = ((const u32*)&sAm02ShapeData[i].mPos)[1];
		((u32*)&sAm02Shapes[i].mPos)[2] = ((const u32*)&sAm02ShapeData[i].mPos)[2];
		sAm02Shapes[i]._0C              = sAm02ShapeData[i]._0C;
		sAm02Shapes[i].mRotY            = sAm02ShapeData[i].mRotY;
		sAm02Shapes[i]._14              = sAm02ShapeData[i]._14;
		sAm02Shapes[i].mSizeX           = sAm02ShapeData[i].mSizeX;
		sAm02Shapes[i].mSizeY           = sAm02ShapeData[i].mSizeY;
		sAm02Shapes[i].mSizeZ           = sAm02ShapeData[i].mSizeZ;
		sAm02Shapes[i].mFlags           = sAm02ShapeData[i].mFlags;
		sAm02Shapes[i].mType            = sAm02ShapeData[i].mType;
		sAm02Shapes[i].mTag             = sAm02ShapeData[i].mTag;
	}
	colliCalcBounds(&sAm02Bounds, sAm02Shapes, 2);
	colliCalcBounds(&sAm02SwingBounds, (ColliShape*)sAm02SwingShapes, 1);
}

// 0x8001F8DC
extern "C" void shutdownTEneAm02()
{
	resBankRemoveTextures(&sAm02Bank, 0);
	resBankRelease(&sAm02Bank);
	sAm02Models   = 0;
	sAm02Textures = 0;
	sAm02Motion   = 0;
}

// 0x8001F924
extern "C" TObject* createTEneAm02(TObject* parent, SetDesc* desc)
{
	return new TEneAm02(parent, desc);
}

// 0x8001F978
// One word of the shape table
extern "C" NJS_TEXLIST* shapeAtTEneAm02(int index)
{
	return sAm02Textures[index];
}

// 0x8001F988
extern "C" void* shapeTEneAm02()
{
	return shapeAtTEneAm02(0);
}

// 0x8001F9AC
// One word of the model table
extern "C" NJS_OBJECT* modelAtTEneAm02(int index)
{
	return sAm02Models[index];
}

// 0x8001F9BC
extern "C" void* modelTEneAm02()
{
	return modelAtTEneAm02(0);
}

// 0x8001F9E0
TEneAm02::TEneAm02(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00   = (u32)sClassName;
	mSize = 0x2C0;
	init(desc);
}

// 0x8001FA78
void TEneAm02::init(SetDesc* desc)
{
	setAction(ACT_WALK);
	motionCtrlInit(&mAnim, sAm02Motions, sAm02Motion);
	motionCtrlPlay(&mAnim, 3);

	mCollider = colliCreate(2, 1);
	colliCopyShapesTagged(mCollider, sAm02Shapes, 0);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 1, 0, 0);
	colliSetBounds(mCollider, &sAm02Bounds);

	mSwing = colliCreate(1, 1);
	colliSetOwner(mSwing, (u16)mObjId);
	colliCopyShapesTagged(mSwing, sAm02SwingShapes, 1);
	colliSetMasks(mSwing, 2, 1, 8, 6);
	colliSetBounds(mSwing, &sAm02SwingBounds);
	colliDisable(mSwing);
	colliShapeSetBit8(mSwing, 0);
	colliShapeSetBit10(mSwing, 0);

	mBrain = 0;
	readEnemyDesc(desc);
	mPose      = allocPose(sAm02Models[1]);
	mModelTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags2    = 0;
	mTarget.x  = 0.0f;
	mTarget.y  = 0.0f;
	mTarget.z  = 0.0f;
	mHitPos.x  = 0.0f;
	mHitPos.y  = 0.0f;
	mHitPos.z  = 0.0f;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 45.0f;
	mShadow._34 = 30.0f;
	mRock       = 0;
	mTrailA     = 0;
	mTrailB     = 0;
	mFrameKey   = 0;
	mLastDamage = 0;
	mThrow      = 0;

	mBar = new TEnemyLifeBar(this);
	mBar->setMax((s16)mHealth);
	mBar->setMark((s16)mHealthStep);
	mBar->setValue((s16)mHealth);
	mBar->setSize(60.0f, 25.0f);

	mLight.reset();
	mTint.reset();
	for (int i = 0; i < 5; i++) {
		mParts[i].x = 0.0f;
		mParts[i].y = 0.0f;
		mParts[i].z = 0.0f;
	}
	enemyEffectSetLarge(60.0f, &mEffect, &mBody.mPos);
	mStep  = 0;
	mTimer = 0;
	mAngle = 0;
	mSpeed = 0.0f;
	createBrain();
}

// 0x8001FD14
TEneAm02::~TEneAm02()
{
	killFx(mBar);
	if (mThrow != 0) {
		delete mThrow;
		mThrow = 0;
	}
	killFx(mTrailB);
	killFx(mTrailA);
	killFx(mRock);
	removeDrawTask(mModelTask);
	freePose((void**)&mPose);
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	colliRetire(mSwing);
	mSwing = 0;
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x8001FE94
// Slot _74
void TEneAm02::setPos(const Vec3f* pos)
{
	f32 x        = pos->x;
	f32 y        = pos->y;
	f32 z        = pos->z;
	mBody.mPos.x = x;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x8001FED8
// Slot _40
f32 TEneAm02::vfn_40()
{
	return 65.0f;
}

// 0x8001FEE0
// Slot _58 Thrown it hurts other enemies
int TEneAm02::onHit(const TObjAction* other, int shape, int* result)
{
	int hit;
	HitInfo* info;
	hit  = 0;
	info = (HitInfo*)result;
	if (isThrown() != 0) {
		if (isEnemyObj(other) != 0) {
			fillAttack(mThrow, info);
			hit = 1;
		}
	} else if (shape == 1) {
		if (isPlayerOrKind4(other) != 0) {
			info->mDamage = 20;
			hit           = 1;
			info->mFlags.clear();
		} else if (isEggObj(other) != 0) {
			info->mDamage = 1;
			hit           = 1;
			info->mFlags.clear();
			info->mFlags.setBit(13);
		}
	}
	return hit;
}

// 0x8001FFBC
// Slot _78 The swing's collider only moves while it's on
void TEneAm02::syncCollider()
{
	colliShapeSetPos(&sAm02Shapes[1], (const Vec3f*)&mParts[2]);
	colliPlaceShapesBounds(mCollider, &mBody.mPos, mBody.mRot.y, sAm02Shapes, (const Vec3f*)&sAm02Bounds);
	if (colliIsDisabled(mSwing) == 0) {
		colliPlaceShapesBounds(mSwing, &mBody.mPos, mBody.mRot.y, (const ColliShape*)sAm02SwingShapes, (const Vec3f*)&sAm02SwingBounds);
	}
}

// 0x80020040
// Slot _38 Int 0 picks the variant 1 the health 10 if not above 0 2 the weak flag
void TEneAm02::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	mVariant   = pickVariant(desc);
	int health = desc->getInt(1);
	int value  = 10;
	if (health > 0) {
		value = health;
	}
	mHealth     = value;
	mHealthStep = divideBy5(mHealth);
	if (desc->getInt(2) != 0) {
		mFlags2 |= AM_WEAK;
	} else {
		mFlags2 &= ~AM_WEAK;
	}
}

// 0x800200F4
// Slot _A4 A kind 1 hit does nothing to it
int TEneAm02::getDamage(EnemyHit* hit)
{
	if (hit->mKind == 1) {
		return 0;
	}
	return hit->mInfo.mDamage;
}

// 0x80020110
// Slot _A8 Always kind 3
int TEneAm02::getHitKind(const void* other, HitInfo*)
{
	Vec3f to;
	to = *(Vec3f*)((u8*)other + 0xC);
	njSubVector(&to, &mBody.mPos);
	angleDist(mBody.mRot.y, toAngle(to.x, to.z));
	return 3;
}

// 0x8002018C
// Slot _70 which slot _0C calls each frame
void TEneAm02::updateBody()
{
	if (isInRange(this, &mBody.mPos, 500.0f) == 0) {
		resetLink();
		mFlags |= 1;
		return;
	}

	if (isBlocked() == 0 && takeHits(&mHit, &mContact) != 0) {
		enemyHitPushAway((Vec3f*)&mHit, &mBody.mPos, (const Vec3f*)&mContact, &mHitPos);
		if (enemyHitTestFlag(&mHit, 2) != 0) {
			mLastDamage = mHit.mInfo.mDamage;
		}
		applyDamage(&mHit);
	}

	checkFellOut();
	mBrain->update(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	mTint.update();
	syncCollider();
	mBar->setPos(&mBody.mPos);
	mBar->setValue((s16)mHealth);
	enemyEffectUpdate(&mEffect);
}

// 0x800202D4
// Slot _6C returnZero makes this do nothing
void TEneAm02::vfn_6C()
{
	if (returnZero() != 0) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginBones();
		drawFirstFrame(sAm02Motion, 7, mPose);
		endBones();
	}
}

// 0x80020334
int TEneAm02::isAppearDone()
{
	return mHitFlags & HIT_DONE;
}

// 0x80020340
void TEneAm02::setTarget(const Vec3f* target)
{
	f32 x     = target->x;
	f32 y     = target->y;
	f32 z     = target->z;
	mTarget.x = x;
	mTarget.y = y;
	mTarget.z = z;
	mFlags2 &= ~AM_ARRIVED;
}

// 0x80020368
int TEneAm02::hasArrived()
{
	return mFlags2 & AM_ARRIVED;
}

// 0x80020374
int TEneAm02::isAttackDone()
{
	return mFlags2 & AM_ATTACK_DONE;
}

// 0x80020380
int TEneAm02::isStunDone()
{
	return mFlags2 & AM_STUN_DONE;
}

// 0x8002038C
void TEneAm02::hitEffects()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
}

// 0x800203CC
int TEneAm02::isFlipDone()
{
	return mFlags2 & AM_FLIP_DONE;
}

// 0x800203D8
void TEneAm02::hitSound()
{
	hitEffects();
	seStartAt(0xE7, &mBody.mPos);
}

// 0x80020410
// Not during a swing
int TEneAm02::canFlip()
{
	return !(mFlags2 & AM_BRACED);
}

// 0x80020424
void TEneAm02::deathSound()
{
	hitEffects();
	seStartAt(0xE8, &mBody.mPos);
}

// 0x8002045C
int TEneAm02::pickDeath()
{
	int action = ACT_DIE;
	if (enemyHitTestFlag(&mHit, 2) != 0) {
		action = ACT_DIE_THROWN;
	}
	return action;
}

// 0x800204A0
void TEneAm02::startEffect()
{
	enemyEffectStart(&mEffect);
}

// 0x800204C4
int TEneAm02::pickVariant(SetDesc* desc)
{
	int value = desc->getInt(0);
	if (value < 0) {
		value = 0;
	} else if (value >= 2) {
		value = 1;
	}
	return value;
}

// 0x80020514
void TEneAm02::createBrain()
{
	if (mVariant != 1) {
		mBrain = new Am02AttackBrain(this);
	} else {
		mBrain = new Am02ThrowBrain(this);
	}
}

// 0x8002058C
// Works out how far one part moved this frame in the model's own space and turns that into a move for the body so the
// feet don't slide
void TEneAm02::partMove(f32 dist, int part, Vec3f* out)
{
	Vec3f prev;
	prev.x = mParts[part].x;
	prev.y = mParts[part].y;
	prev.z = mParts[part].z;
	updatePose();
	gjPushUnitMatrix();
	applyTransform();
	Vec3f cur;
	njCalcPointEx(&mParts[part], &cur);
	njPopMatrixEx();
	moveAlong(dist, &prev, &cur, out);
	out->y += -0.12f;
}

// 0x80020638
int TEneAm02::move(Vec3f* vel)
{
	Vec3f push;
	getFlatPush(&push);
	njAddVector(&push, vel);
	int result = collideFiltered(15.0f, &mBody.mPos, &push, &mShadow);
	if (isAirborne() == 0) {
		vel->y = 0.0f;
	}
	return result;
}

// 0x800206C0
// Returns the angle still to turn
int TEneAm02::turnToTarget(int rate)
{
	Vec3f to;
	to.x = mTarget.x;
	to.y = mTarget.y;
	to.z = mTarget.z;
	njSubVector(&to, &mBody.mPos);
	int diff;
	mBody.mRot.y = angleApproachDiff(mBody.mRot.y, toAngle(to.x, to.z), rate, &diff);
	return diff;
}

// 0x8002074C
// Moves with a foot or under a negative number just updates the pose
void TEneAm02::stepFoot(f32 dist, int foot)
{
	if (foot >= 0) {
		partMove(dist, foot, &mBody.mVel);
	} else {
		updatePose();
	}
	move(&mBody.mVel);
}

// 0x80020798
void TEneAm02::checkArrived(int rate)
{
	if (probeLine(&mTarget, &mBody.mPos, (void*)rate, &mBody.mVel) != 0) {
		mFlags2 |= AM_ARRIVED;
	} else {
		mFlags2 &= ~AM_ARRIVED;
	}
}

// 0x800207F8
void TEneAm02::coast(f32 drag)
{
	mBody.mVel.x *= drag;
	mBody.mVel.y += -0.12f;
	mBody.mVel.z *= drag;
	move(&mBody.mVel);
}

// 0x80020844
// Which foot is down in the walk
int TEneAm02::walkFoot()
{
	f32 t = motionCtrlFrame(&mAnim);
	if (t >= 5.0f && t < 20.0f) {
		return 0;
	}
	return 1;
}

// 0x80020890
// Which foot is down in the chase
int TEneAm02::chaseFoot()
{
	int foot;
	f32 t = motionCtrlFrame(&mAnim);
	if (t <= 2.0f) {
		foot = 1;
	} else if (t <= 6.0f) {
	} else if (t <= 13.0f) {
		foot = 0;
	} else if (t <= 19.0f) {
		foot = -1;
	} else {
		foot = 1;
	}
	return foot;
}

// 0x8002091C
// The model writes the five part positions while this is on
void TEneAm02::beginBones()
{
	nodeTrackBeginOrigins(sAm02Bones, mParts);
}

// 0x8002094C
void TEneAm02::endBones()
{
	nodeTrackEnd();
}

// 0x8002096C
void TEneAm02::updatePose()
{
	beginBones();
	applyPoseFaded(mPose, 60.0f, &mAnim, &mAppearTimer);
	endBones();
}

// 0x800209B8
// From the model's space to the world's
void TEneAm02::updateParts()
{
	gjPushUnitMatrix();
	applyTransform();
	for (int i = 0; i < 5; i++) {
		njCalcPointEx(&mParts[i], &mParts[i]);
	}
	njPopMatrixEx();
}

// 0x80020A18
void TEneAm02::applyTransform()
{
	njTranslateEx(&mBody.mPos);
	if (isThrown() != 0) {
		njTranslate(0, 0.0f, 20.0f, 0.0f);
		if (mThrow != 0) {
			mThrow->mBody.vfn_20();
		}
		njTranslate(0, 0.0f, -20.0f, 0.0f);
	} else {
		gjRotateY(mBody.mRot.y);
	}
}

// 0x80020AB4
void TEneAm02::followPart(EnemyFx** fx, int part)
{
	if (*fx == 0) {
		*fx = (EnemyFx*)particleGenByIdDefault(&mParts[part], 0, 0x52);
	} else {
		(*fx)->mPos = mParts[part];
	}
}

// 0x80020B2C
// Slot _7C
void TEneAm02::onActionPhase(int action, int phase)
{
	switch (action) {
	case ACT_APPEAR:
		actAppear(phase);
		break;
	case ACT_WALK:
		actWalk(phase);
		break;
	case ACT_CHASE:
		actChase(phase);
		break;
	case ACT_ATTACK:
		actAttack(phase);
		break;
	case ACT_THROW:
		actThrow(phase);
		break;
	case ACT_STUN:
		actStun(phase);
		break;
	case ACT_FLIP:
		actFlip(phase);
		break;
	case ACT_DIE:
		actDie(phase);
		break;
	case ACT_DIE_THROWN:
		actDieThrown(phase);
		break;
	}
}

// 0x80020BD0
void TEneAm02::actAppear(int phase)
{
	int* step = &mStep;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0);
		beginAppear(step);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		runAppear(step);
		updatePose();
		updateParts();
		break;
	case 2:
		endAppear(step);
		break;
	}
}

// 0x80020C6C
void TEneAm02::actWalk(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		turnToTarget(0x180);
		stepFoot(1.0f, walkFoot());
		checkArrived(0x180);
		updateParts();
		break;
	case 2:
		break;
	}
}

// 0x80020D0C
// mStep says it's braking for a sharp turn
void TEneAm02::actChase(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 1);
		mFrameKey = 5;
		mStep     = 0;
		mSpeed    = 0.2f;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		int diff = turnToTarget(0x180);
		if (mStep == 0) {
			if (diff >= 0x4000) {
				mStep = 1;
			}
		} else if (diff <= 0x300) {
			mStep = 0;
		}
		if (mStep == 0) {
			mSpeed += 0.01f;
			if (mSpeed > 0.35f) {
				mSpeed = 0.35f;
			}
		} else {
			mSpeed -= 0.02f;
			if (mSpeed < 0.0f) {
				mSpeed = 0.0f;
			}
		}
		stepFoot(mSpeed, chaseFoot());
		checkArrived(0x180);
		updateParts();
		break;
	}
	case 2:
		clearFrameKey();
		break;
	}
}

// 0x80020E54
// A swing with both hands
void TEneAm02::actAttack(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 2);
		mFlags2 &= ~AM_ATTACK_DONE;
		mFlags2 |= AM_BRACED;
		mStep  = 0;
		mTimer = 0;
		colliDisable(mSwing);
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		updatePose();
		updateParts();
		setFrameKey(sAm02FrameKeys[0]);
		coast(0.96f);
		f32 t = motionCtrlFrame(&mAnim);
		if (!(t < 8.0f)) {
			if (t <= 22.0f) {
				followPart(&mTrailA, 4);
				followPart(&mTrailB, 3);
			} else {
				killFx(mTrailA);
				killFx(mTrailB);
			}
		}
		switch (mTimer) {
		case 0:
			if (motionCtrlPassed(0.1f, &mAnim) != 0) {
				particleOnceByIdDefault(&mParts[0], 0, 0x57);
				particleOnceByIdDefault(&mParts[1], 0, 0x57);
			}
			if (t >= 16.0f) {
				mTimer = 1;
				fxSpawnFlat(5, (void*)5, (const Vec3f*)&mBody.mPos);
				colliEnable(mSwing);
				camShakeNearHard(150.0f, &mBody.mPos);
				seStartAt(0xE6, &mBody.mPos);
			}
			break;
		case 1:
			colliPlaceShapesBounds(mSwing, &mBody.mPos, mBody.mRot.y, (const ColliShape*)sAm02SwingShapes, (const Vec3f*)&sAm02SwingBounds);
			if (t >= 18.0f) {
				mTimer = 2;
				mFlags2 &= ~AM_BRACED;
				colliDisable(mSwing);
				mStep = 0;
			}
			break;
		case 2:
			if (motionCtrlIsDone(&mAnim) != 0) {
				mFlags2 |= AM_ATTACK_DONE;
			}
			break;
		}
		break;
	}
	case 2:
		colliDisable(mSwing);
		mFlags2 &= ~AM_BRACED;
		clearFrameKey();
		killFx(mTrailA);
		killFx(mTrailB);
		break;
	}
}

// 0x80021130
// Throws a TEneAm02_Rock mStep 0 winding up 1 holding it 75 above itself 2 thrown
void TEneAm02::actThrow(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 3);
		mFlags2 &= ~AM_ATTACK_DONE;
		mStep = 0;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		updatePose();
		updateParts();
		setFrameKey(sAm02FrameKeys[1]);
		if (motionCtrlIsDone(&mAnim) != 0) {
			mFlags2 |= AM_ATTACK_DONE;
		}
		Vec3f to;
		to.x = mTarget.x;
		to.y = mTarget.y;
		to.z = mTarget.z;
		njSubVector(&to, &mBody.mPos);
		turnTo(toAngle(to.x, to.z), 0x90);
		coast(0.96f);
		switch (mStep) {
		case 0:
			if (motionCtrlFrame(&mAnim) >= 10.0f) {
				mStep = 1;
				Vec3f hold;
				hold.x = mBody.mPos.x;
				hold.y = mBody.mPos.y;
				hold.z = mBody.mPos.z;
				hold.y += 75.0f;
				mRock = new TEneAm02_Rock(this, &hold, mBody.mRot.y);
			}
			break;
		case 1: {
			Vec3f hold;
			hold.x = mBody.mPos.x;
			hold.y = mBody.mPos.y;
			hold.z = mBody.mPos.z;
			hold.y += 75.0f;
			if (mRock != 0) {
				mRock->hold(&hold, mBody.mRot.y);
			}
			if (mTrailA != 0) {
				mTrailA->mPos = hold;
			}
			if (motionCtrlFrame(&mAnim) >= 23.0f) {
				mStep = 2;
				if (mRock != 0) {
					mRock->throwAt(&mTarget);
					mRock = 0;
				}
				killFx(mTrailA);
			}
			break;
		case 2:
			break;
		}
		}
		break;
	}
	case 2:
		colliDisable(mSwing);
		killFx(mRock);
		killFx(mTrailA);
		clearFrameKey();
		break;
	}
}

// 0x800213F0
// Turns to face where the hit came from
void TEneAm02::actStun(int phase)
{
	switch (phase) {
	case 0: {
		motionCtrlPlay(&mAnim, 6);
		mFlags2 &= ~AM_STUN_DONE;
		mFrameKey = 4;
		mStep     = 0;
		Vec3f dir;
		dir.x = mContact.mSurface.x;
		dir.y = mContact.mSurface.y;
		dir.z = mContact.mSurface.z;
		njSubVector(&dir, &mContact.mCenter);
		mAngle  = toAngle(dir.x, dir.z);
		f32 len = njUnitVector(&dir);
		if (len < gUnitVectorMin) {
			dir.x = gVecZero.x;
			dir.y = gVecZero.y;
			dir.z = gVecZero.z;
		}
		gjMulAddVector(8.0f + len, &dir, &mContact.mCenter, &dir);
		enemyFxSparks(3.0f, &dir, &mContact.mCenter);
		break;
	}
	case 1:
		mBody.mRot.y = angleApproach(mBody.mRot.y, mAngle, 0x800);
		motionCtrlUpdate(&mAnim);
		updatePose();
		updateParts();
		coast(0.96f);
		if (motionCtrlIsDone(&mAnim) != 0) {
			mFlags2 |= AM_STUN_DONE;
		}
		break;
	case 2:
		clearFrameKey();
		mEnemyFlags &= ~ENEMY_INTANGIBLE;
		mFlags2 &= ~AM_STUN_DONE;
		break;
	}
}

// 0x80021580
// Pushed along the hit and plays the hit effects twice as the original does
void TEneAm02::actFlip(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 4);
		mEnemyFlags |= ENEMY_FLIPPING;
		hitSound();
		mFlags2 &= ~AM_FLIP_DONE;
		mStep        = 0;
		mBody.mVel.x = mHitPos.x;
		mBody.mVel.y = mHitPos.y;
		mBody.mVel.z = mHitPos.z;
		mBody.mVel.y = 0.0f;
		hitSound();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlIsDone(&mAnim) != 0) {
			mFlags2 |= AM_FLIP_DONE;
		}
		updatePose();
		updateParts();
		setFrameKey(sAm02FrameKeys[2]);
		coast(0.7f);
		break;
	case 2:
		clearFrameKey();
		mEnemyFlags &= ~ENEMY_FLIPPING;
		break;
	}
}

// 0x80021694
// mStep 0 falling over 1 lying there for 90 frames 2 unused since step 1 removes it
void TEneAm02::actDie(int phase)
{
	switch (phase) {
	case 0:
		mEnemyFlags |= ENEMY_BLOCKED;
		motionCtrlPlayBlend(0.2f, &mAnim, 5);
		mFlags2 |= AM_DYING;
		mStep  = 0;
		mTimer = 0;
		colliDisable(mCollider);
		mBar->hide();
		killed(1);
		deathSound();
		break;
	case 1:
		switch (mStep) {
		case 0:
			motionCtrlUpdate(&mAnim);
			updatePose();
			updateParts();
			setFrameKey(sAm02FrameKeys[3]);
			coast(0.96f);
			if (motionCtrlIsDone(&mAnim) != 0) {
				mStep  = 1;
				mTimer = 0;
			}
			if (motionCtrlPassed(7.0f, &mAnim) != 0) {
				particleOnceByIdDefault(&mBody.mPos, 0, 0x46);
				seEnemyPuff(&mBody.mPos);
			}
			break;
		case 1:
			mTimer++;
			if (mTimer >= 90) {
				mStep     = 2;
				mTimer    = 0;
				Vec3f pos = mBody.mPos;
				pos.y += 8.0f;
				enemyFxDeath4ASe(&pos);
				spawnDropB(&mBody.mPos);
				mFlags |= 1;
			}
			break;
		case 2:
			mTimer++;
			if (mTimer >= 15) {
				spawnDropB(&mBody.mPos);
				mFlags |= 1;
			}
			break;
		}
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_BLOCKED;
		clearFrameKey();
		break;
	}
}

// 0x8002189C
// Becomes a projectile that hurts other enemies gone after 180 frames
void TEneAm02::actDieThrown(int phase)
{
	switch (phase) {
	case 0: {
		mEnemyFlags |= ENEMY_BLOCKED | ENEMY_THROWN;
		mFlags2 |= AM_THROWN;
		mThrow = new RockFlight(20.0f, &mBody);
		mThrow->setVel(&mHitPos);
		mThrow->mBody.mPos.y += 20.0f;
		mThrow->mBody.mVel.y *= 0.5f;
		gjScaleVector(2.0f, &mThrow->mBody.mVel, &mThrow->mBody.mVel);
		motionCtrlPlay(&mAnim, 4);
		motionCtrlUpdate(&mAnim);
		updatePose();
		colliShapeSetBit8(mCollider, 0);
		mBar->hide();
		killed(1);
		deathSound();
		seStartAt(0xE1, &mBody.mPos);
		spawnDropB(&mBody.mPos);
		mFrameKey = 3;
		break;
	}
	case 1: {
		mThrow->update(mCollider);
		mTimer++;
		if (mTimer == 120) {
			mThrow->setGroundFriction();
		}
		{
			RockFlight* rock = mThrow;
			mBody.mPos.x     = rock->mBody.mPos.x;
			mBody.mPos.y     = rock->mBody.mPos.y;
			mBody.mPos.z     = rock->mBody.mPos.z;
		}
		EggHits hits;
		if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&mBody.mPos, -1) != 0 && (hits.mMask & 1) != 0) {
			shadowSetFromContact(&mShadow, (u8*)&hits);
		}
		mBody.mPos.y -= 20.0f;
		updateParts();
		if (mTimer >= 180) {
			enemyFxDeath44Se(&mBody.mPos);
			mFlags |= 1;
		}
		break;
	}
	case 2:
		mEnemyFlags &= ~(ENEMY_BLOCKED | ENEMY_THROWN);
		mFlags2 &= ~AM_THROWN;
		clearFrameKey();
		if (mThrow != 0) {
			delete mThrow;
			mThrow = 0;
		}
		break;
	}
}

// 0x80021AD8
// Slot _3C
void TEneAm02::vfn_3C()
{
	int pos = (kEnemyInfoX << 16) | kEnemyInfoLine;
	if (mVariant == 0) {
		njPrintC(pos, ":TYPE A");
	} else if (mVariant == 1) {
		njPrintC(pos, ":TYPE B");
	}
	njPrint((kEnemyInfoX << 16) | (kEnemyInfoLine + 1), ":HP %3d", mHealth);
	pos = (kEnemyInfoX << 16) | (kEnemyInfoLine + 2);
	if ((mFlags2 & AM_WEAK) != 0) {
		njPrintC(pos, ":WEAK OFF");
	} else {
		njPrintC(pos, ":WEAK ON");
	}
}

// 0x80021B98
void TEneAm02::show()
{
	mFlags &= ~0x10;
	if (isBlocked() == 0) {
		mBar->show();
	}
}

// 0x80021BEC
void TEneAm02::hide()
{
	mFlags |= 0x10;
	mBar->hide();
}

// 0x80021C1C
// Slot _10
void TEneAm02::vfn_10()
{
	if ((mFlags2 & AM_HIDDEN) != 0 || isAppearing() != 0) {
		hide();
	} else if (mLight.isVisible(mModelTask, 500.0f, &mBody.mPos, 29.0f, 0) == 0) {
		hide();
	} else {
		show();
	}
}

// 0x80021CAC
// Slot _1C
void TEneAm02::render()
{
	njSetTexture(sAm02Textures[0]);
	enemyEyeSet(sAm02Textures[0], 0, mFrameKey);
	useArea();
	njPushMatrixEx();
	applyTransform();
	drawLit(&mLight, &mTint, mPose);
	njPopMatrixEx();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}

// 0x80021D2C
// Defined here after ~TEneAm02 first used it so it is generated here
inline Am02SeekBrain::~Am02SeekBrain()
{
}

// Never called Referencing update here keeps its body in place
static void am02BrainUpdate(Am02Brain* brain, TEneAm02* owner)
{
	brain->Am02Brain::update(owner);
}

// 0x80021D9C
// Runs the current state's phases then asks the owner to change action if the state chose one
inline void Am02Brain::update(TEneAm02* owner)
{
	mOwner = owner;
	mNext  = -1;
	if (checkInterrupt() == 0) {
		onPhase(mState, 2);
	}
	onPhase(mState, 1);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}
