#include "enemy/TEneArmadillo.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyAppear.h"
#include "enemy/EnemyCommon.h"
#include "engine/MotionPose.h"
#include "io/ResBank.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/EnemyHit.h"
#include "enemy/ObjEnemy.h"
#include "core/MathUtil.h"
#include "enemy/EnemyFx.h"
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "enemy/boss/EneBoss.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "engine/Colli.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "engine/TParticle.h"
#include "enemy/TEneAm01.h"
struct Vec3f;
struct ObjShadow;

extern "C" {

// The looping sound in mSound start with an id move it stop it

MotionEntry gArmadilloMotionTable[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 19.0f, 0.45f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 10.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 9.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 19.0f, 0.35f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 29.0f, 0.35f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 9.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 8, -1, 0, 0.0f, 10.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 10, -1, 0, 0.0f, 20.0f, 0.35f, 0.0f, 0 },
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 59.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 12, -1, 0, 0.0f, 30.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 29.0f, 0.25f, 0.0f, 0 },
};
extern const ColliShape sArmadilloShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 14.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

// Radians to angle units
static inline int toAngle(f32 x, f32 z)
{
	return (int)(10430.38043493439 * (f32)atan2(x, z));
}

inline void* TEneArmadillo::operator new(u32 size)
{
	return objHeapAlloc(gObjHeap, size);
}

inline void TEneArmadillo::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x800F5E94
extern "C" void initTEneArmadillo()
{
	resBankLoad(&gArmadilloBank, "ar_ene_armadillo.arc");
	resBankAddTextures(&gArmadilloBank, 0, "ene_armadillo.gvm");
	sArmadilloModels = resBankGetModels(&gArmadilloBank);
	sArmadilloTextures = resBankGetTextures(&gArmadilloBank);
	sArmadilloMotions = resBankGetMotion(&gArmadilloBank, 0);
	sArmadilloParams = (int*)enemyParamGet(11);
}

// 0x800F5F1C
extern "C" void shutdownTEneArmadillo()
{
	sArmadilloParams = 0;
	resBankRemoveTextures(&gArmadilloBank, 0);
	resBankRelease(&gArmadilloBank);
	sArmadilloModels = 0;
	sArmadilloTextures = 0;
	sArmadilloMotions = 0;
}

// 0x800F5F6C
extern "C" TObject* createTEneArmadillo(TObject* parent, SetDesc* desc)
{
	return new TEneArmadillo(parent, desc);
}

// 0x800F5FC0
TEneArmadillo::TEneArmadillo(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00 = (u32)sArmadilloClassName;
	mSize = 0x300;
	init(desc);
}

// 0x800F6070
void TEneArmadillo::init(SetDesc* desc)
{
	motionCtrlInit(&mAnim, gArmadilloMotionTable, sArmadilloMotions);
	motionCtrlPlay(&mAnim, 9);
	mBody.mRot.x = 0;
	mCollider = colliCreate(1, 1);
	colliCopyShapes(mCollider, sArmadilloShape);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0x14, 0, 0x10);
	colliShapeSetTag(mCollider, 0, 0);
	mBrain = 0;
	readEnemyDesc(desc);
	mPose = allocPose(*(void**)sArmadilloModels);
	mModel = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags2 = 0;
	mKnock.x = 0.0f;
	mKnock.y = 0.0f;
	mKnock.z = 0.0f;
	mFrameKey = 0;
	for (int i = 0; i < 3; i++) {
		mParts[i].x = 0.0f;
		mParts[i].y = 0.0f;
		mParts[i].z = 0.0f;
	}
	mAim.x = 0.0f;
	mAim.y = 0.0f;
	mAim.z = 0.0f;
	mAimAngle = 0;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 28.0f;
	mShadow._34 = 28.0f;
	shadowEnable(&mShadow);
	mBar = new TEnemyLifeBar(this);
	mBar->setMax((s16)mHealth);
	mBar->setMark(0);
	mBar->setValue((s16)mHealth);
	mBar->setSize(40.0f, 20.0f);
	mLight.reset();
	mLight.setFadeDist(-15.0f);
	mTint.reset();
	enemyEffectSetLarge(30.0f, &mEffect, &mBody.mPos);
	for (int i = 0; i < 2; i++) {
		mDust[i] = 0;
	}
	mStep = 0;
	mTimer = 0;
	mSpin = 0;
	_2FC = 0.0f;
	createBrain();
}

// 0x800F62B0
TEneArmadillo::~TEneArmadillo()
{
	seLoopStop(&mSound);
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	if (mBar != 0) {
		mBar->mFlags |= 1;
		mBar = 0;
	}
	removeDrawTask(mModel);
	freePose((void**)&mPose);
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x800F63B8
void TEneArmadillo::setPos(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	mBody.mPos.x = x;
	f32 z = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x800F63FC
f32 TEneArmadillo::vfn_40()
{
	return 50.0f;
}

// 0x800F6404
// The one shape sits 15 over the body
void TEneArmadillo::syncCollider()
{
	Vec3f* shape = (Vec3f*)colliGetShapes(mCollider);
	*shape = mBody.mPos;
	shape->y += 15.0f;
}

// 0x800F645C
// Health defaults to 200
void TEneArmadillo::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	int health = desc->getInt(0);
	mHealth = health <= 0 ? 200 : health;
}

// 0x800F64B8
// Hurts only while armed A player takes 20 and is knocked aside away from the side it is on
int TEneArmadillo::onHit(const TObjAction* other, int shape, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit = 0;
	if ((mFlags2 & ARM_ARMED) != 0) {
		if (isPlayerOrKind4(other) != 0) {
			info->mDamage = 20;
			hit = 1;
			info->mFlags.clear();
			info->mFlags.setBit(0xC);
			Vec3f dir;
			dir.y = 0.0f;
			njSinCos(mBody.mRot.y, &dir.x, &dir.z);
			Vec3f delta;
			delta = *((TObjAction*)other)->getPos();
			njSubVector(&delta, &mBody.mPos);
			if (vecCrossXZ(&dir, &delta) > 0.0f) {
				info->mPos.x = 2.0f * dir.z;
				info->mPos.y = 2.0f;
				info->mPos.z = -2.0f * dir.x;
			} else {
				info->mPos.x = -2.0f * dir.z;
				info->mPos.y = 2.0f;
				info->mPos.z = 2.0f * dir.x;
			}
			gjScaleVector(60.0f, &info->mPos, &info->mPos);
		} else if (isEggObj(other) != 0) {
			info->mDamage = 1;
			hit = 1;
			info->mFlags.clear();
			info->mFlags.setBit(0xD);
		}
	}
	return hit;
}

// 0x800F6624
void TEneArmadillo::setTarget(const Vec3f* target)
{
	mTarget = *target;
}

// 0x800F6640
void TEneArmadillo::setAim(const Vec3f* pos, int angle)
{
	mAim = *pos;
	mAimAngle = angle;
}

// 0x800F6660
// A light hit a spark where it landed
void TEneArmadillo::noteHit()
{
	enemyFxSparks(3.0f, &mContact.mSurface, &mContact.mCenter);
	seEnemyCrush(&mBody.mPos);
}

// 0x800F66A0
void TEneArmadillo::noteDeath()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
}

// 0x800F66E0
int TEneArmadillo::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

// 0x800F6704
void TEneArmadillo::stopRoll()
{
	mFlags2 |= ARM_STOP;
}

// 0x800F6714
int TEneArmadillo::hasArrived()
{
	return mFlags2 & ARM_ARRIVED;
}

// 0x800F6720
int TEneArmadillo::isRollDone()
{
	return isActionDone(ACT_ROLL);
}

// 0x800F6744
int TEneArmadillo::isCurlDone()
{
	return isActionDone(ACT_CURL);
}

// 0x800F6768
int TEneArmadillo::isUncurlDone()
{
	return isActionDone(ACT_UNCURL);
}

// 0x800F678C
// Hit from in front it flips
int TEneArmadillo::hurtAction()
{
	Vec3f delta;
	f32 sin;
	f32 cos;
	delta = mContact.mPos;
	njSubVector(&delta, &mBody.mPos);
	njSinCos(mBody.mRot.y, &sin, &cos);
	if (sin * delta.x + cos * delta.z > 0.0f) {
		return ACT_FLIP;
	}
	return ACT_KNOCK;
}

// 0x800F6818
int TEneArmadillo::isRecovered()
{
	return mFlags2 & ARM_RECOVERED;
}

// 0x800F6824
void TEneArmadillo::noteStun()
{
	noteDeath();
	seStartAt(0x102, &mBody.mPos);
}

// 0x800F685C
void TEneArmadillo::noteDie()
{
	noteDeath();
	seStartAt(0x103, &mBody.mPos);
}

// 0x800F6894
// The brain asks which death to use
int TEneArmadillo::deathAction()
{
	return ACT_DIE;
}

// 0x800F689C
void TEneArmadillo::stopEffect()
{
	enemyEffectStart(&mEffect);
}

// 0x800F68C0
void TEneArmadillo::updateBody()
{
	if (isBlocked() == 0 && isInRange(this, &mBody.mPos, 500.0f) == 0) {
		resetLink();
		mFlags |= 1;
		return;
	}
	if (isBlocked() == 0 && takeHits(&mHit, &mContact) != 0) {
		enemyHitPushAway((Vec3f*)&mHit, &mBody.mPos, (const Vec3f*)&mContact, &mKnock);
		applyDamage(&mHit);
	}
	mBrain->update(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	mTint.update();
	placeParts();
	syncCollider();
	mBar->setPos(&mBody.mPos);
	mBar->setValue((s16)mHealth);
	enemyEffectUpdate(&mEffect);
}

// 0x800F6A08
// returnZero is 0 so this never runs
void TEneArmadillo::vfn_6C()
{
	if (returnZero() != 0) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginBones();
		drawFirstFrame(sArmadilloMotions, 13, mPose);
		endBones();
		placeParts();
		syncCollider();
	}
}

// 0x800F6A84
void TEneArmadillo::createBrain()
{
	mBrain = ArmadilloHuntBrain::create(this);
}

// 0x800F6AB4
void TEneArmadillo::beginBones()
{
	nodeTrackBeginOrigins(sArmadilloTrackNodes, mParts);
}

// 0x800F6AE4
void TEneArmadillo::endBones()
{
	nodeTrackEnd();
}

// 0x800F6B04
void TEneArmadillo::updatePose()
{
	beginBones();
	applyPoseFaded(mPose, 30.0f, &mAnim, &mAppearTimer);
	endBones();
}

// 0x800F6B50
// Moves the posed points into the world
void TEneArmadillo::placeParts()
{
	gjPushUnitMatrix();
	applyTransform();
	for (int i = 0; i < 3; i++) {
		njCalcPointEx(&mParts[i], &mParts[i]);
	}
	njPopMatrixEx();
}

// 0x800F6BB0
// Guarded only a kind-1 hit counts
int TEneArmadillo::getHitKind(const void*, HitInfo*)
{
	if ((mFlags2 & ARM_GUARDED) != 0) {
		return 1;
	}
	return 3;
}

// 0x800F6BC8
// Only a kind-3 hit carries its damage
int TEneArmadillo::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x800F6BE0
// Rolled up the ball turns about a point 15 up
void TEneArmadillo::applyTransform()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
	if ((mFlags2 & ARM_BALL) != 0) {
		gjTranslate(0.0f, 15.0f, 0.0f);
		gjRotateX(mBody.mRot.x);
		gjTranslate(0.0f, -15.0f, 0.0f);
	}
}

// 0x800F6C4C
// How far the foot part moved this frame as a step along the ground with gravity
void TEneArmadillo::stepFoot(f32 dist, int part, Vec3f* out)
{
	Vec3f from;
	Vec3f to;
	from = mParts[part];
	updatePose();
	gjPushUnitMatrix();
	applyTransform();
	njCalcPointEx(&mParts[part], &to);
	njPopMatrixEx();
	moveAlong(dist, &from, &to, out);
	out->y += -0.12f;
}

// 0x800F6CF8
// Moves by vel and the push
int TEneArmadillo::move(Vec3f* vel)
{
	Vec3f dir;
	getFlatPush(&dir);
	njAddVector(&dir, vel);
	int hit = collideFiltered(14.0f, &mBody.mPos, &dir, &mShadow);
	if (isAirborne() == 0) {
		vel->y = 0.0f;
	}
	return hit;
}

// 0x800F6D80
// Walks toward mTarget on whichever foot is down
void TEneArmadillo::walk(f32 dist, void* arg)
{
	faceToward(&mTarget, &mBody.mPos, arg);
	f32 frame = motionCtrlFrame(&mAnim);
	int part;
	if (frame >= 4.0f && frame < 18.0f && motionCtrlIsBlending(&mAnim) == 0) {
		part = 1;
	} else {
		part = 2;
	}
	stepFoot(dist, part, &mBody.mVel);
	move(&mBody.mVel);
	if (probeLine(&mTarget, &mBody.mPos, arg, &mBody.mVel) != 0) {
		mFlags2 |= ARM_ARRIVED;
	} else {
		mFlags2 &= ~ARM_ARRIVED;
	}
}

// 0x800F6E68
void TEneArmadillo::drag(f32 rate)
{
	mBody.mVel.x *= rate;
	mBody.mVel.y += -0.12f;
	mBody.mVel.z *= rate;
	move(&mBody.mVel);
}

// 0x800F6EB4
// Returns how far it still has to turn
int TEneArmadillo::turnToAim(int rate)
{
	int diff;
	mBody.mRot.y = angleApproachDiff(mBody.mRot.y, mAimAngle, rate, &diff);
	return diff;
}

// 0x800F6EF8
void TEneArmadillo::arm()
{
	mFlags2 |= ARM_ARMED;
	colliShapeSetBit8(mCollider, 0);
}

// 0x800F6F2C
void TEneArmadillo::disarm()
{
	mFlags2 &= ~ARM_ARMED;
	colliShapeClearBit8(mCollider, 0);
}

// 0x800F6F60
void TEneArmadillo::guard()
{
	mFlags2 |= ARM_GUARDED;
}

// 0x800F6F70
void TEneArmadillo::unguard()
{
	mFlags2 &= ~ARM_GUARDED;
}

// 0x800F6F80
// Dust under it while on the ground
void TEneArmadillo::kickDust()
{
	if (isAirborne() == 0) {
		enemyFxKeepAt((void**)&mDust[0], &mBody.mPos, 0x7A);
		enemyFxKeepAt((void**)&mDust[1], &mBody.mPos, 0x7C);
	} else {
		stopDust();
	}
}

// 0x800F6FEC
void TEneArmadillo::stopDust()
{
	for (int i = 1; i >= 0; i--) {
		if (mDust[i] != 0) {
			mDust[i]->mFlags |= 1;
			mDust[i] = 0;
		}
	}
}

// 0x800F7028
// A particle on the ground under the first part on the given frame
void TEneArmadillo::puffAt(f32 frame, int id)
{
	if (motionCtrlPassed(frame, &mAnim) != 0) {
		Vec3f pos;
		gjPushUnitMatrix();
		applyTransform();
		njCalcPointEx(&mParts[0], &pos);
		njPopMatrixEx();
		pos.y = mBody.mPos.y;
		particleOnceByIdDefault(&pos, 0, id);
	}
}

// 0x800F7098
void TEneArmadillo::onActionPhase(int action, int phase)
{
	switch (action) {
	case ACT_APPEAR:
		actAppear(phase);
		break;
	case ACT_IDLE:
		actIdle(phase);
		break;
	case ACT_WALK:
		actWalk(phase);
		break;
	case ACT_ROLL:
		actRoll(phase);
		break;
	case ACT_CURL:
		actCurl(phase);
		break;
	case ACT_HOLD:
		actHold(phase);
		break;
	case ACT_UNCURL:
		actUncurl(phase);
		break;
	case ACT_KNOCK:
		actKnock(phase);
		break;
	case ACT_FLIP:
		actFlip(phase);
		break;
	case ACT_DIE:
		actDie(phase);
		break;
	}
}

// 0x800F7148
void TEneArmadillo::actAppear(int phase)
{
	int* step = &mStep;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 9);
		beginAppear(step);
		mFrameKey = 2;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		runAppear(step);
		updatePose();
		break;
	case 2:
		endAppear(step);
		clearFrameKey();
		break;
	}
}

// 0x800F71EC
void TEneArmadillo::actIdle(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 9);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		drag(0.8f);
		break;
	case 2:
		break;
	}
}

// 0x800F7268
void TEneArmadillo::actWalk(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.05f, &mAnim, 11);
		mFlags2 &= ~ARM_ARRIVED;
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		walk(1.0f, (void*)0x100);
		break;
	case 2:
		mFlags2 &= ~ARM_ARRIVED;
		break;
	}
}

// 0x800F7304
// mStep 0 curling up 1 and 2 spinning up in place while turning to the player 3 the roll 4 slowing down and uncurling
void TEneArmadillo::actRoll(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0);
		mFlags2 &= ~(ARM_STOP | ARM_BALL);
		mFlags2 |= ARM_20;
		clearFrameKey();
		disarm();
		unguard();
		stopDust();
		fxHolderStop(&mFx);
		mBody.mRot.x = 0;
		mStep = 0;
		mSpin = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		switch (mStep) {
		case 0:
			setFrameKey(sArmadilloFrameKeys[2]);
			if (motionCtrlIsDone(&mAnim) != 0) {
				motionCtrlPlayBlend(0.3f, &mAnim, 1);
				clearFrameKey();
				mStep = 1;
				mTimer = 600;
				arm();
				guard();
				mFlags2 |= ARM_BALL;
			}
			updatePose();
			drag(0.8f);
			break;
		case 1:
		case 2:
			if (mStep == 1) {
				if (mSpin >= 0x800) {
					seLoopStart(&mSound, 0x100, &mBody.mPos);
					mStep = 2;
				}
			} else {
				seLoopMove(&mSound, &mBody.mPos);
			}
			mSpin += 0x80;
			if (mSpin >= 0x1800) {
				mSpin = 0x1800;
				int diff = turnToAim(0x240);
				mTimer--;
				if (diff <= 0x240 || mTimer <= 0) {
					mStep = 3;
					mTimer = 10000;
					seLoopStop(&mSound);
					seStartAt(0x101, &mBody.mPos);
				}
			}
			mBody.mRot.x += mSpin;
			updatePose();
			drag(0.8f);
			break;
		case 3: {
			mBody.mRot.x += mSpin;
			updatePose();
			int diff = turnToAim(0x80);
			f32 sin;
			f32 cos;
			njSinCos(mBody.mRot.y, &sin, &cos);
			mBody.mVel.x = 5.0f * sin;
			mBody.mVel.z = 5.0f * cos;
			mBody.mVel.y += -0.12f;
			move(&mBody.mVel);
			mTimer--;
			if (mTimer >= 0) {
				Vec3f trail;
				int angle = mBody.mRot.y;
				vecFromAngleXZ(35.0f, &trail, angle);
				njAddVector(&trail, &mBody.mPos);
				trail.y += 14.0f;
				mFx.start(5, 13, &trail, 0, angle);
				if ((mTimer & 7) == 0) {
					fxSpawnScaled(2.0f, 1.0f, 2.0f, 5, (void*)0x11, (const Vec3f*)&mBody.mPos, 0, mBody.mRot.y);
				}
			} else {
				fxHolderStop(&mFx);
			}
			if (diff >= 0x5000 || (mFlags2 & ARM_STOP) != 0) {
				mStep = 4;
				mTimer = 90;
				mFlags2 |= ARM_20;
				fxHolderStop(&mFx);
			}
			kickDust();
			break;
		}
		case 4:
			mTimer--;
			if (mTimer <= 0) {
				disarm();
				motionCtrlPlay(&mAnim, 2);
				mFlags2 &= ~ARM_BALL;
				mBody.mRot.x = 0;
				mHitFlags |= HIT_DONE;
			} else {
				mBody.mRot.x += -0x1A00;
			}
			updatePose();
			drag(0.96f);
			if (vecLenXZ(&mBody.mVel) >= 1.0f && mTimer > 0) {
				kickDust();
			} else {
				stopDust();
			}
			break;
		}
		break;
	case 2:
		mFlags2 &= ~(ARM_STOP | ARM_20 | ARM_BALL);
		mBody.mRot.x = 0;
		disarm();
		unguard();
		clearFrameKey();
		stopDust();
		fxHolderStop(&mFx);
		seLoopStop(&mSound);
		break;
	}
}

// 0x800F7784
void TEneArmadillo::actCurl(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 6);
		clearFrameKey();
		guard();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		drag(0.8f);
		puffAt(3.0f, 0x7B);
		break;
	case 2:
		unguard();
		break;
	}
}

// 0x800F7830
void TEneArmadillo::actHold(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 7);
		clearFrameKey();
		guard();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		drag(0.8f);
		break;
	case 2:
		unguard();
		break;
	}
}

// 0x800F78C4
// mStep 0 the first motion 1 the second
void TEneArmadillo::actUncurl(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 8);
		mStep = 0;
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		setFrameKey(sArmadilloFrameKeys[3]);
		if (mStep == 0) {
			if (motionCtrlIsDone(&mAnim) != 0) {
				motionCtrlPlay(&mAnim, 10);
				mStep = 1;
			}
		} else {
			waitMotion();
		}
		drag(0.8f);
		break;
	case 2:
		clearFrameKey();
		break;
	}
}

// 0x800F79A8
// Takes the knock as its speed and returns the angle it goes
int TEneArmadillo::startKnock()
{
	mEnemyFlags |= ENEMY_FLIPPING;
	mBody.mVel.x = mKnock.x;
	mBody.mVel.z = mKnock.z;
	mFlags2 &= ~ARM_RECOVERED;
	mFlags2 |= ARM_20;
	noteStun();
	return toAngle(mBody.mVel.x, mBody.mVel.z);
}

// 0x800F7A2C
// Slides to a stop turning to angle faster the faster it goes
void TEneArmadillo::tumble(int angle)
{
	motionCtrlUpdate(&mAnim);
	updatePose();
	if (motionCtrlIsDone(&mAnim) != 0) {
		mFlags2 |= ARM_RECOVERED;
	}
	drag(0.7f);
	turnTo(angle, (int)(384.0f * vecLenXZ(&mBody.mVel)));
}

// 0x800F7AB8
void TEneArmadillo::endKnock()
{
	mEnemyFlags &= ~ENEMY_FLIPPING;
	clearFrameKey();
	mFlags2 &= ~(ARM_RECOVERED | ARM_20);
}

// 0x800F7AFC
// Faces the way it is knocked
void TEneArmadillo::actKnock(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 4);
		mFrameKey = 3;
		mSpin = startKnock();
		break;
	case 1:
		tumble(mSpin);
		puffAt(2.0f, 0x7B);
		setFrameKey(sArmadilloFrameKeys[1]);
		break;
	case 2:
		endKnock();
		break;
	}
}

// 0x800F7BA0
// Faces back the way it came
void TEneArmadillo::actFlip(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 5);
		mFrameKey = 3;
		mSpin = startKnock() + 0x8000;
		break;
	case 1:
		tumble(mSpin);
		puffAt(18.0f, 0x7B);
		break;
	case 2:
		endKnock();
		clearFrameKey();
		break;
	}
}

// 0x800F7C40
// Falls over then after 60 more loops of the motion goes in a puff leaving a drop
void TEneArmadillo::actDie(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 12);
		mEnemyFlags |= ENEMY_BLOCKED | ENEMY_INTANGIBLE;
		colliDisable(mCollider);
		mBar->hide();
		killed(1);
		noteDie();
		mFrameKey = 3;
		mStep = 60;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		puffAt(18.0f, 0x46);
		if (motionCtrlIsDone(&mAnim) != 0) {
			mStep--;
			if (mStep <= 0) {
				enemyFxDeath4A(&mBody.mPos);
				spawnDropB(&mBody.mPos);
				mFlags |= 1;
			}
		}
		break;
	case 2:
		mEnemyFlags &= ~(ENEMY_BLOCKED | ENEMY_INTANGIBLE);
		clearFrameKey();
		break;
	}
}

// 0x800F7D5C
void TEneArmadillo::vfn_3C()
{
	int x = kEnemyInfoX << 16;
	int line = kEnemyInfoLine;
	njPrint(x | line, ":HP %4d", mHealth);
}

// 0x800F7D98
void TEneArmadillo::vfn_10()
{
	if (isAppearing() != 0) {
		mFlags |= 0x10;
	} else if (mLight.isVisible(mModel, 500.0f, &mBody.mPos, 60.0f, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x800F7E28
void TEneArmadillo::render()
{
	njSetTexture(*(void**)sArmadilloTextures);
	enemyEyeSet(*(NJS_TEXLIST**)sArmadilloTextures, 0, mFrameKey);
	useArea();
	njPushMatrixEx();
	applyTransform();
	drawLit(&mLight, &mTint, mPose);
	njPopMatrixEx();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}

// 0x800F7EA8
void ArmadilloBrain::update(TEneArmadillo* owner)
{
	mOwner = owner;
	mNext = -1;
	if (checkInterrupt() == 0) {
		onPhase(mState, 2);
	}
	onPhase(mState, 1);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x800F7F40
ArmadilloBrainBase::ArmadilloBrainBase()
{
	mOwner = 0;
}

// 0x800F7F74
ArmadilloBrainBase::~ArmadilloBrainBase() { }

// 0x800F7FD0
ArmadilloHuntBrain* ArmadilloHuntBrain::create(TEneArmadillo* owner)
{
	return new ArmadilloHuntBrain(owner);
}

// 0x800F8018
ArmadilloHuntBrain::ArmadilloHuntBrain(TEneArmadillo* owner)
{
	mSight.reset();
	mMemory.reset();
	mCharging = 0;
	mCharge = *sArmadilloParams;
	mTimer = 0;
	mResting = 0;
	mOwner = owner;
	mNext = -1;
	mState = STATE_APPEAR;
	onPhase(mState, 0);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x800F80E0
ArmadilloHuntBrain::~ArmadilloHuntBrain() { }

// 0x800F8148
// A hard hit hurts it and it goes after who hit it
int ArmadilloHuntBrain::checkInterrupt()
{
	if (mOwner->isDead() != 0 && mState != STATE_DIE) {
		changeState(STATE_DIE);
		return 1;
	}
	if (mOwner->wasHitHard() != 0) {
		changeState(STATE_HURT);
		mMemory.setChase(mOwner->getHitPlayer());
	} else if (mOwner->wasHitKind1() != 0) {
		mOwner->noteHit();
	}
	return 0;
}

// 0x800F8254
// A random point 25 to 35 from home
void ArmadilloHuntBrain::pickTarget()
{
	Vec3f home;
	Vec3f target;
	home = mOwner->mHome;
	randomPointInRing(25.0f, 35.0f, &home, &target);
	mOwner->setTarget(&target);
}

// 0x800F82B8
// Follows the player it remembers or looks in a cone
int ArmadilloHuntBrain::look(f32 rangeSq, int angle)
{
	mMemory.tick();
	if (mMemory.isReady() != 0) {
		if (mMemory.isChasing() != 0) {
			return mSight.track(mMemory.getPlayer(), mOwner->getPos(), mOwner->getRotY());
		}
		return mSight.findInCone(rangeSq, mOwner->getPos(), mOwner->getRotY(), angle);
	}
	return 0;
}
// 0x800F83CC
// 160 ahead
int ArmadilloHuntBrain::lookNear()
{
	return look(25600.0f, 0xAAAA);
}

// 0x800F83F8
// 300 ahead
int ArmadilloHuntBrain::lookFar()
{
	return look(90000.0f, 0xD555);
}

// 0x800F8424
void ArmadilloHuntBrain::aim()
{
	mOwner->setAim(&mSight.mTarget, mSight.mAngle);
}

// 0x800F8454
void ArmadilloHuntBrain::resetCharge()
{
	mCharging = 0;
	mCharge = 0;
}

// 0x800F8464
void ArmadilloHuntBrain::startCharge()
{
	if (mCharging != 0) {
		return;
	}
	mCharging = 1;
	mCharge = *sArmadilloParams;
}

// 0x800F8488
// 1 once the count has run out
int ArmadilloHuntBrain::chargeDone()
{
	if (mCharging == 1) {
		if (mCharge <= 0) {
			return 1;
		}
		mCharge--;
	}
	return 0;
}

// 0x800F84B8
// Slot _0C
void ArmadilloHuntBrain::onPhase(int state, int phase)
{
	switch (state) {
	case STATE_APPEAR:
		phaseAppear(phase);
		break;
	case STATE_WANDER:
		phaseWander(phase);
		break;
	case STATE_ROLL:
		phaseRoll(phase);
		break;
	case STATE_CURL:
		phaseCurl(phase);
		break;
	case STATE_HOLD:
		phaseHold(phase);
		break;
	case STATE_UNCURL:
		phaseUncurl(phase);
		break;
	case STATE_HURT:
		phaseHurt(phase);
		break;
	case STATE_DIE:
		phaseDie(phase);
		break;
	}
}

// 0x800F8550
void ArmadilloHuntBrain::phaseAppear(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneArmadillo::ACT_APPEAR;
		break;
	case 2:
		if (mOwner->isAppearDone() != 0) {
			changeState(STATE_WANDER);
		}
		break;
	case 3:
		break;
	}
}

// 0x800F85F0
// Walks between random points resting on arrival
void ArmadilloHuntBrain::phaseWander(int phase)
{
	switch (phase) {
	case 0:
		pickTarget();
		mNext = TEneArmadillo::ACT_WALK;
		mTimer = randomInt(300.0f, 120.0f);
		mResting = 0;
		break;
	case 2:
		if (chargeDone() != 0) {
			changeState(STATE_CURL);
		} else if (lookNear() != 0) {
			mOwner->stopEffect();
			changeState(STATE_ROLL);
		}
		break;
	case 1:
		mTimer--;
		if (mTimer < 0) {
			pickTarget();
			mTimer = randomInt(300.0f, 120.0f);
			if (mResting != 0) {
				mResting = 0;
				mNext = TEneArmadillo::ACT_WALK;
			}
		} else if (mOwner->hasArrived() != 0) {
			mResting = 1;
			mNext = TEneArmadillo::ACT_IDLE;
		}
		break;
	case 3:
		break;
	}
}

// 0x800F8774
// Rolls at the player for up to 600 frames while it can still see them
void ArmadilloHuntBrain::phaseRoll(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneArmadillo::ACT_ROLL;
		mMemory.clearChase();
		aim();
		mTimer = 600;
		break;
	case 2:
		if (mOwner->isRollDone() != 0) {
			mMemory.startCooldown();
			mMemory.clearChase();
			resetCharge();
			if (lookNear() != 0) {
				changeState(STATE_ROLL);
			} else {
				changeState(STATE_WANDER);
			}
		}
		break;
	case 1:
		mTimer--;
		if (lookFar() == 0 || mTimer <= 0) {
			mOwner->stopRoll();
		} else {
			aim();
		}
		break;
	case 3:
		break;
	}
}

// 0x800F88D8
void ArmadilloHuntBrain::phaseCurl(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneArmadillo::ACT_CURL;
		break;
	case 2:
		if (mOwner->isCurlDone() != 0) {
			changeState(STATE_HOLD);
		}
		break;
	case 3:
		break;
	}
}

// 0x800F8978
// Holds 180 frames then rolls at a player within 300 or uncurls
void ArmadilloHuntBrain::phaseHold(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneArmadillo::ACT_HOLD;
		mTimer = 180;
		break;
	case 2:
		mTimer--;
		if (mTimer <= 0) {
			if (lookNear() != 0 && mSight.mDistSq <= 90000.0f) {
				changeState(STATE_ROLL);
			} else {
				changeState(STATE_UNCURL);
			}
		}
		break;
	case 3:
		break;
	}
}

// 0x800F8A8C
void ArmadilloHuntBrain::phaseUncurl(int phase)
{
	switch (phase) {
	case 0:
		mNext = TEneArmadillo::ACT_UNCURL;
		break;
	case 2:
		if (mOwner->isUncurlDone() != 0) {
			resetCharge();
			changeState(STATE_WANDER);
		}
		break;
	case 3:
		break;
	}
}

// 0x800F8B34
// Knocked or flipped and the charge count starts
void ArmadilloHuntBrain::phaseHurt(int phase)
{
	switch (phase) {
	case 0:
		mNext = mOwner->hurtAction();
		startCharge();
		break;
	case 2:
		if (chargeDone() != 0) {
			changeState(STATE_CURL);
		} else if (mOwner->isRecovered() != 0) {
			changeState(STATE_WANDER);
		}
		break;
	case 3:
		break;
	}
}

// 0x800F8C30
void ArmadilloHuntBrain::phaseDie(int phase)
{
	switch (phase) {
	case 0:
		mNext = mOwner->deathAction();
		break;
	case 1:
		break;
	case 2:
		break;
	}
}
