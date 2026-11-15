#include "enemy/TEnePenguin.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyAppear.h"
#include "enemy/EnemyCommon.h"
#include "engine/MotionPose.h"
#include "engine/CharaColli.h"
#include "io/ResBank.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/EnemyHit.h"
#include "enemy/ObjEnemy.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "animal/AniMount.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "enemy/EnemyFx.h"
#include "engine/TParticle.h"
#include "animal/RacerMover.h"
#include "enemy/TEneAm01.h"
struct Vec3f;
struct ObjShadow;

extern "C" {

// The looping sound in mSound start with an id move it stop it

MotionEntry sPenguinMotions[] = {
	{ MOTION_LOOP, 5, -1, 0, 0.0f, 20.0f, 0.5f, 0.0f, 0 },  { MOTION_LOOP, 7, -1, 0, 0.0f, 30.0f, 0.7f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 19.0f, 0.35f, 0.0f, 0 }, { MOTION_LOOP, 4, -1, 0, 0.0f, 10.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },  { MOTION_LOOP, 8, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },  { MOTION_ONCE, 0, -1, 0, 0.0f, 19.0f, 0.2f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 29.0f, 0.2f, 0.0f, 0 },
};
}

// What getHitKind is handed about the other side
struct PenguinTouch {
	u8 _00[0xC];
	Vec3f mPos; // _0C
	int mLive;  // _18
};

// Radians to angle units
static inline int toAngle(f32 x, f32 z)
{
	return (int)(10430.38043493439 * (f32)atan2(x, z));
}

static inline void flatUnit(Vec3f* v)
{
	if (njUnitVector(v) < gUnitVectorMin) {
		*v = gVecZero;
	}
}

inline void* TEnePenguin::operator new(u32 size)
{
	return objHeapAlloc(gObjHeap, size);
}

inline void TEnePenguin::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

extern const int sPenguinBones[] = {
	1, 28, 34, 52, 59, 68, 75, -1,
}; // the bones mParts follow

const char* gTEnePenguinName = "TEnePenguin";

extern const EnemyFrameKey sPenguinFrameKey0[] = { { 8.0f, 0 }, { -1.0f, 5 } };
extern const EnemyFrameKey sPenguinFrameKey1[] = { { 10.0f, 5 }, { -1.0f, 3 } };
extern const EnemyFrameKey sPenguinFrameKey2[] = { { 1.0f, 3 }, { 11.0f, 0 }, { 25.0f, 4 }, { -1.0f, 0 } };
extern const EnemyFrameKey sPenguinFrameKey3[] = { { 1.0f, 0 }, { 5.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sPenguinFrameKey4[] = { { 1.0f, 0 }, { 6.0f, 3 }, { -1.0f, 2 } };

const EnemyFrameKey* sPenguinFrameKeys[] = {
	sPenguinFrameKey0, sPenguinFrameKey1, sPenguinFrameKey2, sPenguinFrameKey3, sPenguinFrameKey4,
};

extern const ColliShape sPenguinShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 8.0f, 10.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 7.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};

// 0x800AA1D4
extern "C" void initTEnePenguin()
{
	resBankLoad(&sPenguinBank, "ar_ene_penguin.arc");
	resBankAddTextures(&sPenguinBank, 0, "ene_penguin.gvm");
	sPenguinModels   = resBankGetModels(&sPenguinBank);
	sPenguinTextures = resBankGetTextures(&sPenguinBank);
	sPenguinMotion   = resBankGetMotion(&sPenguinBank, 0);
	gPenguinParams   = (PenguinParams*)enemyParamGet(9);
}

// 0x800AA25C
extern "C" void shutdownTEnePenguin()
{
	gPenguinParams = 0;
	resBankRemoveTextures(&sPenguinBank, 0);
	resBankRelease(&sPenguinBank);
	sPenguinModels   = 0;
	sPenguinTextures = 0;
	sPenguinMotion   = 0;
}

// 0x800AA2AC
extern "C" TObject* createTEnePenguin(TObject* parent, SetDesc* desc)
{
	return new TEnePenguin(parent, desc);
}

// 0x800AA300
TEnePenguin::TEnePenguin(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00   = (u32)gTEnePenguinName;
	mSize = 0x314;
	init(desc);
}

// 0x800AA3A8
void TEnePenguin::init(SetDesc* desc)
{
	motionCtrlInit(&mAnim, sPenguinMotions, sPenguinMotion);
	motionCtrlPlay(&mAnim, 0);
	mBody.mRot.x = 0;
	mCollider    = colliCreate(2, 1);
	colliCopyShapes(mCollider, sPenguinShape);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0x14, 0, 0x10);
	int shape = 0;
	for (int i = 0; i < 2; i++) {
		colliShapeSetTag(mCollider, i, shape);
		shape++;
	}
	mBrain = 0;
	readEnemyDesc(desc);
	mPose     = allocPose(*(void**)sPenguinModels);
	mModel    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags2   = 0;
	mKnock.x  = 0.0f;
	mKnock.y  = 0.0f;
	mKnock.z  = 0.0f;
	mFrameKey = 0;
	for (int i = 0; i < 7; i++) {
		mParts[i].x = 0.0f;
		mParts[i].y = 0.0f;
		mParts[i].z = 0.0f;
	}
	mAim.x = 0.0f;
	mAim.y = 0.0f;
	mAim.z = 0.0f;
	mDir.x = 0.0f;
	mDir.y = 0.0f;
	mDir.z = 0.0f;
	mSpin  = 0;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 16.0f;
	mShadow._34 = 16.0f;
	shadowEnable(&mShadow);
	mBar = new TEnemyLifeBar(this);
	mBar->setMax((s16)mHealth);
	mBar->setMark(0);
	mBar->setValue((s16)mHealth);
	mBar->setSize(40.0f, 20.0f);
	mLight.reset();
	mLight.setFadeDist(-15.0f);
	mTint.reset();
	enemyEffectSetLarge(35.0f, &mEffect, &mBody.mPos);
	mFxMode = FX_NONE;
	mFxNext = -1;
	for (int i = 0; i < 4; i++) {
		mFx[i] = 0;
	}
	mStep   = 0;
	mAngle  = 0;
	mUnk310 = 0.0f;
	createBrain();
}

// 0x800AA614
TEnePenguin::~TEnePenguin()
{
	seLoopStop(&mSound);
	for (int i = 3; i >= 0; i--) {
		if (mFx[i] != 0) {
			mFx[i]->mFlags |= 1;
			mFx[i] = 0;
		}
	}
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	removeDrawTask(mModel);
	if (mBar != 0) {
		mBar->mFlags |= 1;
		mBar = 0;
	}
	freePose((void**)&mPose);
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x800AA748
void TEnePenguin::setPos(const Vec3f* pos)
{
	f32 x        = pos->x;
	f32 y        = pos->y;
	mBody.mPos.x = x;
	f32 z        = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x800AA78C
f32 TEnePenguin::vfn_40()
{
	return 40.0f;
}

// 0x800AA794
// The shapes sit on the first two posed points
void TEnePenguin::syncCollider()
{
	Vec3f* part = (Vec3f*)colliGetShapes(mCollider);
	part[0]     = mParts[0];
	part[4]     = mParts[1];
	colliRefreshBounds(mCollider);
	colliBoundsOn(mCollider);
}

// 0x800AA804
// Health defaults to 200
void TEnePenguin::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	int health = desc->getInt(0);
	mHealth    = health <= 0 ? 200 : health;
}

// 0x800AA860
// Hurts only while armed and only with its first shape
int TEnePenguin::onHit(const TObjAction* other, int shape, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit       = 0;
	if ((mFlags2 & PEN_ARMED) != 0 && shape == 0) {
		if (isPlayerOrKind4(other) != 0) {
			info->mDamage = 20;
			hit           = 1;
			info->mFlags.clear();
		} else if (isEggObj(other) != 0) {
			info->mDamage = 1;
			hit           = 1;
			info->mFlags.clear();
			info->mFlags.setBit(0xD);
		}
	}
	return hit;
}

// 0x800AA900
void TEnePenguin::setTarget(const Vec3f* target)
{
	mTarget = *target;
}

// 0x800AA91C
void TEnePenguin::setAim(const Vec3f* aim)
{
	mAim = *aim;
}

// 0x800AA938
void TEnePenguin::noteDeath()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
}

// 0x800AA978
void TEnePenguin::stopEffect()
{
	enemyEffectStart(&mEffect);
}

// 0x800AA99C
int TEnePenguin::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

// 0x800AA9C0
int TEnePenguin::hasArrived()
{
	return mFlags2 & PEN_ARRIVED;
}

// 0x800AA9CC
int TEnePenguin::isSlideDone()
{
	return isActionDone(ACT_SPIN);
}

// 0x800AA9F0
int TEnePenguin::isRestDone()
{
	return isActionDone(ACT_REST);
}

// 0x800AAA14
// Not while it is getting going
int TEnePenguin::canStun()
{
	return !(mFlags2 & PEN_SLIDING);
}

// 0x800AAA28
int TEnePenguin::isStunDone()
{
	return isActionDone(ACT_STUN);
}

// 0x800AAA4C
void TEnePenguin::noteStun()
{
	noteDeath();
	seStartAt(0xEE, &mBody.mPos);
}

// 0x800AAA84
void TEnePenguin::noteDie()
{
	noteDeath();
	seStartAt(0xEF, &mBody.mPos);
}

// 0x800AAABC
// The brain asks which death to use
int TEnePenguin::deathAction()
{
	return ACT_DIE;
}

// 0x800AAAC4
void TEnePenguin::updateBody()
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
	updateSound();
	updateFx();
	mBar->setPos(&mBody.mPos);
	mBar->setValue((s16)mHealth);
	enemyEffectUpdate(&mEffect);
}

// 0x800AAC1C
// returnZero is 0 so this never runs
void TEnePenguin::vfn_6C()
{
	if (returnZero() != 0) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginBones();
		drawFirstFrame(sPenguinMotion, 9, mPose);
		endBones();
		placeParts();
		syncCollider();
	}
}

// 0x800AAC98
void TEnePenguin::createBrain()
{
	mBrain = PenguinHuntBrain::create(this);
}

// 0x800AACC8
void TEnePenguin::beginBones()
{
	nodeTrackBeginOrigins(sPenguinBones, mParts);
}

// 0x800AACF8
void TEnePenguin::endBones()
{
	nodeTrackEnd();
}

// 0x800AAD18
void TEnePenguin::updatePose()
{
	beginBones();
	applyPoseFaded(mPose, 25.0f, &mAnim, &mAppearTimer);
	endBones();
}

// 0x800AAD64
// Moves the posed points into the world
void TEnePenguin::placeParts()
{
	gjPushUnitMatrix();
	applyTransform();
	for (int i = 0; i < 7; i++) {
		njCalcPointEx(&mParts[i], &mParts[i]);
	}
	njPopMatrixEx();
}

// 0x800AADC4
// While armed a touch from below its second point does nothing
int TEnePenguin::getHitKind(const void* touch, HitInfo*)
{
	PenguinTouch* t = (PenguinTouch*)touch;
	if ((mFlags2 & PEN_ARMED) != 0) {
		if (t->mLive == 0) {
			return 0;
		}
		if (t->mPos.y < mParts[1].y) {
			return 0;
		}
	}
	return 3;
}

// 0x800AAE04
// Only a kind-3 hit carries its damage
int TEnePenguin::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x800AAE1C
// On its belly it leans over the way it slides
void TEnePenguin::applyTransform()
{
	njTranslateEx(&mBody.mPos);
	if ((mFlags2 & PEN_TILTED) != 0) {
		gjRotateY(mBody.mRot.z);
		gjRotateX(mBody.mRot.x);
	}
	gjRotateY(mBody.mRot.y);
}

// 0x800AAE70
// How far a foot moved this frame as a step along the ground
void TEnePenguin::walkStep(f32 rate, int part, Vec3f* out)
{
	Vec3f was;
	Vec3f moved;
	was = mParts[part];
	updatePose();
	gjPushUnitMatrix();
	applyTransform();
	njCalcPointEx(&mParts[part], &moved);
	njPopMatrixEx();
	moveAlong(rate, &was, &moved, out);
	out->y += -0.12f;
}

// 0x800AAF1C
// Moves by vel and the push
int TEnePenguin::move(Vec3f* vel)
{
	Vec3f push;
	getFlatPush(&push);
	njAddVector(vel, &push);
	int hit = collideFiltered(8.0f, &mBody.mPos, vel, &mShadow);
	if (isAirborne() == 0) {
		vel->y = 0.0f;
	}
	return hit;
}

// 0x800AAFA4
// The same with the push on its own
int TEnePenguin::slideMove(Vec3f* vel)
{
	Vec3f push;
	getFlatPush(&push);
	njAddVector(&push, vel);
	int hit = collideFiltered(8.0f, &mBody.mPos, &push, &mShadow);
	if (isAirborne() == 0) {
		vel->y = 0.0f;
	}
	return hit;
}

// 0x800AB02C
// Turns toward the target and steps with whichever foot is down
void TEnePenguin::walk(f32 rate, void* arg)
{
	faceToward(&mTarget, &mBody.mPos, arg);
	f32 frame = motionCtrlFrame(&mAnim);
	int foot;
	if (frame >= 0.0f && frame < 15.0f && motionCtrlIsBlending(&mAnim) == 0) {
		foot = 5;
	} else {
		foot = 6;
	}
	walkStep(rate, foot, &mBody.mVel);
	slideMove(&mBody.mVel);
	if (probeLine(&mTarget, &mBody.mPos, arg, &mBody.mVel) != 0) {
		mFlags2 |= PEN_ARRIVED;
	} else {
		mFlags2 &= ~PEN_ARRIVED;
	}
}

// 0x800AB114
void TEnePenguin::drag(f32 rate)
{
	mBody.mVel.x *= rate;
	mBody.mVel.y += -0.12f;
	mBody.mVel.z *= rate;
	slideMove(&mBody.mVel);
}

// 0x800AB160
// The belly shape grows and hurts
void TEnePenguin::arm()
{
	mFlags2 |= PEN_ARMED;
	ColliShape* shape = colliGetShape(mCollider, 0);
	colliShapeSetCylRadius(shape, 21.0f);
	shape->mFlags |= 8;
	colliShapeSetRadius(colliGetShape(mCollider, 1), 10.0f);
}

// 0x800AB1CC
void TEnePenguin::disarm()
{
	mFlags2 &= ~PEN_ARMED;
	ColliShape* shape = colliGetShape(mCollider, 0);
	colliShapeSetCylRadius(shape, 8.0f);
	shape->mFlags &= ~8;
	colliShapeSetRadius(colliGetShape(mCollider, 1), 7.0f);
}

// 0x800AB238
// Leans into the slide more the faster it goes
void TEnePenguin::tilt(f32 speed)
{
	mBody.mRot.z = toAngle(mDir.x, mDir.z);
	mBody.mRot.x = (int)(4681.143f * speed);
}

// 0x800AB2B4
// Starts moves or stops the sliding sound as PEN_SLIDENOW asks
void TEnePenguin::updateSound()
{
	u32 flags = mFlags2;
	if ((flags & PEN_SOUND) != 0) {
		if ((flags & PEN_SLIDENOW) != 0) {
			seLoopMove(&mSound, &mBody.mPos);
		} else {
			seLoopStop(&mSound);
			mFlags2 &= ~PEN_SOUND;
		}
	} else if ((flags & PEN_SLIDENOW) != 0) {
		seLoopStart(&mSound, 0xED, &mBody.mPos);
		mFlags2 |= PEN_SOUND;
	}
}

// 0x800AB33C
void TEnePenguin::startSlideSound()
{
	mFlags2 |= PEN_SLIDENOW;
}

// 0x800AB34C
void TEnePenguin::stopSlideSound()
{
	mFlags2 &= ~PEN_SLIDENOW;
}

// 0x800AB35C
// Switches the particles over when asked then keeps them on the body
void TEnePenguin::updateFx()
{
	if (mFxNext != -1) {
		if (mFxMode != mFxNext) {
			killFx();
		}
		mFxMode = mFxNext;
		mFxNext = -1;
	}
	if (mFxMode == FX_NONE) {
	} else if (mFxMode == FX_SLIDE) {
		enemyFxKeepAtRot((void**)&mFx[0], (const Vec3f*)&mParts[0], mBody.mRot.x, mBody.mRot.y, 0x69);
		enemyFxKeepAt((void**)&mFx[1], &mBody.mPos, 0x6D);
		enemyFxKeepAt((void**)&mFx[2], (const Vec3f*)&mParts[3], 0x55);
		enemyFxKeepAt((void**)&mFx[3], (const Vec3f*)&mParts[4], 0x55);
	} else if (mFxMode == FX_DUST) {
		Vec3f pos;
		pos = mParts[1];
		pos.y += 6.0f;
		enemyFxKeepAt((void**)&mFx[0], &pos, 0x47);
	}
}

// 0x800AB44C
void TEnePenguin::setFx(int mode)
{
	mFxNext = mode;
}

// 0x800AB454
void TEnePenguin::killFx()
{
	for (int i = 3; i >= 0; i--) {
		if (mFx[i] != 0) {
			mFx[i]->mFlags |= 1;
			mFx[i] = 0;
		}
	}
}

// 0x800AB490
void TEnePenguin::onActionPhase(int action, int phase)
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
	case ACT_SLIDE:
		actSlide(phase);
		break;
	case ACT_SPIN:
		actSpin(phase);
		break;
	case ACT_GETUP:
		actGetUp(phase);
		break;
	case ACT_REST:
		actRest(phase);
		break;
	case ACT_STUN:
		actStun(phase);
		break;
	case ACT_DIE:
		actDie(phase);
		break;
	}
}

// 0x800AB534
void TEnePenguin::actAppear(int phase)
{
	int* step = &mStep;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0);
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

// 0x800AB5D8
void TEnePenguin::actIdle(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		drag(0.2f);
		break;
	case 2:
		break;
	}
}

// 0x800AB654
void TEnePenguin::actWalk(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.05f, &mAnim, 1);
		mFlags2 &= ~PEN_ARRIVED;
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		walk(1.0f, (void*)0x100);
		break;
	case 2:
		mFlags2 &= ~PEN_ARRIVED;
		break;
	}
}

// 0x800AB6F0
// Winds up throws itself down then slides at mAim
void TEnePenguin::actSlide(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 2);
		mStep  = 0;
		mAngle = 0;
		mSpin  = 0;
		disarm();
		mFlags2 &= ~PEN_TILTED;
		mFlags2 |= PEN_SLIDING;
		mDir.x       = 0.0f;
		mDir.y       = 0.0f;
		mDir.z       = 0.0f;
		mBody.mRot.z = 0;
		mBody.mRot.x = 0;
		setFx(FX_NONE);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		switch (mStep) {
		case 0:
			setFrameKey(sPenguinFrameKeys[0]);
			if (motionCtrlFrame(&mAnim) >= 13.0f) {
				mAngle = toAngle(mParts[2].x, mParts[2].z);
				mStep  = 1;
				setFx(FX_SLIDE);
			}
			break;
		case 1: {
			setFrameKey(sPenguinFrameKeys[0]);
			if (motionCtrlFrame(&mAnim) >= 15.5f) {
				startSlideSound();
			}
			int angle = toAngle(mParts[2].x, mParts[2].z);
			int turn  = angleDist(angle, mAngle);
			if (turn < mSpin) {
				mBody.mRot.y += mSpin - turn;
			} else {
				mSpin = turn;
			}
			mAngle = angle;
			if (motionCtrlIsDone(&mAnim) != 0) {
				mStep = 2;
				arm();
				mFlags2 |= PEN_TILTED;
				mFlags2 &= ~PEN_SLIDING;
				motionCtrlPlay(&mAnim, 3);
				mFrameKey = 5;
			}
			break;
		}
		case 2: {
			Vec3f to;
			Vec3f dir;
			Vec3f side;
			f32 speed;
			mBody.mRot.y += mSpin;
			to = mAim;
			njSubVector(&to, &mBody.mPos);
			to.y = 0.0f;
			flatUnit(&to);
			if (mDir.x * to.x + mDir.z * to.z < -0.2f) {
				gjScaleVector(0.92f, &mDir, &mDir);
				speed = njScalor(&mDir);
			} else {
				gjMulAddVector(0.04f, &mDir, &mDir, &to);
				dir   = mDir;
				speed = njUnitVector(&dir);
				if (speed < gUnitVectorMin) {
					dir = gVecZero;
				}
				if (speed > 1.0f) {
					mDir.x = dir.x;
					mDir.z = dir.z;
				}
			}
			tilt(speed);
			f32 along = mBody.mVel.x * to.x + mBody.mVel.z * to.z;
			gjMulAddVector(-along, &side, &mBody.mVel, &to);
			side.y = 0.0f;
			gjScaleVector(gPenguinParams->mSideDrag, &side, &side);
			if (along < 0.0f) {
				along += gPenguinParams->mSpinPull;
			} else {
				along += gPenguinParams->mSpinPush;
			}
			gjMulAddVector(along, &dir, &side, &to);
			dir.y   = 0.0f;
			f32 len = njUnitVector(&dir);
			if (len < gUnitVectorMin) {
				dir = gVecZero;
			}
			if (len > gPenguinParams->mMaxSpeed) {
				len = gPenguinParams->mMaxSpeed;
			}
			mBody.mVel.x = dir.x * len;
			mBody.mVel.z = dir.z * len;
			mBody.mVel.y += -0.12f;
			if (move(&mBody.mVel) != 0) {
				mBody.mVel.x *= 0.5f;
				mBody.mVel.z *= 0.5f;
			}
			break;
		}
		}
		break;
	case 2:
		disarm();
		mFlags2 &= ~(PEN_TILTED | PEN_SLIDING);
		setFx(FX_NONE);
		clearFrameKey();
		stopSlideSound();
		break;
	}
}

// 0x800ABBC0
// The slide slows and spins down then it sits up
void TEnePenguin::actSpin(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 3);
		mFlags2 |= PEN_TILTED;
		setFx(FX_SLIDE);
		startSlideSound();
		mStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		setFrameKey(sPenguinFrameKeys[1]);
		gjScaleVector(0.95f, &mDir, &mDir);
		f32 speed = njScalor(&mDir);
		if (speed <= 0.01f) {
			mDir.x = 0.0f;
			mDir.y = 0.0f;
			mDir.z = 0.0f;
			mFlags2 &= ~PEN_TILTED;
			mBody.mRot.y += mBody.mRot.z;
			mBody.mRot.x = 0;
			mBody.mRot.z = 0;
		} else {
			tilt(speed);
		}
		mSpin -= 0x90;
		if (mStep == 0) {
			if (mSpin <= 0x1000) {
				mStep = 1;
				motionCtrlPlay(&mAnim, 4);
			}
			mBody.mVel.x *= 0.95f;
			mBody.mVel.z *= 0.95f;
		} else {
			f32 frame = motionCtrlFrame(&mAnim);
			if (frame >= 5.0f) {
				setFx(FX_NONE);
			}
			if (frame >= 3.5f) {
				stopSlideSound();
			}
			waitMotion();
			mBody.mVel.x *= 0.7f;
			mBody.mVel.z *= 0.7f;
		}
		if (mSpin < 0) {
			mSpin = 0;
		}
		mBody.mRot.y += mSpin;
		mBody.mVel.y += -0.12f;
		slideMove(&mBody.mVel);
		break;
	case 2:
		mFlags2 &= ~PEN_TILTED;
		setFx(FX_NONE);
		mBody.mRot.y += mBody.mRot.z;
		stopSlideSound();
		clearFrameKey();
		break;
	}
}

// 0x800ABE18
void TEnePenguin::actGetUp(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 5);
		mFrameKey = 3;
		setFx(FX_DUST);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		drag(0.2f);
		break;
	case 2:
		clearFrameKey();
		setFx(FX_NONE);
		break;
	}
}

// 0x800ABEBC
void TEnePenguin::actRest(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 6);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		setFrameKey(sPenguinFrameKeys[2]);
		waitMotion();
		drag(0.2f);
		break;
	case 2:
		clearFrameKey();
		break;
	}
}

// 0x800ABF5C
// Knocked back by the hit
void TEnePenguin::actStun(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 7);
		mEnemyFlags |= ENEMY_FLIPPING;
		mFrameKey = 3;
		noteStun();
		mBody.mVel.x = mKnock.x;
		mBody.mVel.z = mKnock.z;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		setFrameKey(sPenguinFrameKeys[3]);
		waitMotion();
		drag(0.7f);
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		clearFrameKey();
		break;
	}
}

// 0x800AC030
// Falls over then a second later bursts and drops its prize
void TEnePenguin::actDie(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 8);
		mEnemyFlags |= ENEMY_INTANGIBLE | ENEMY_BLOCKED;
		colliDisable(mCollider);
		mBar->hide();
		killed(1);
		noteDie();
		mFrameKey = 3;
		mStep     = 60;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlPassed(5.0f, &mAnim) != 0) {
			Vec3f pos;
			pos   = mParts[0];
			pos.y = mBody.mPos.y;
			particleOnceByIdDefault(&pos, 0, 0x46);
		}
		updatePose();
		setFrameKey(sPenguinFrameKeys[4]);
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
		mEnemyFlags &= ~(ENEMY_INTANGIBLE | ENEMY_BLOCKED);
		clearFrameKey();
		break;
	}
}

// 0x800AC194
void TEnePenguin::vfn_3C()
{
	int x    = kEnemyInfoX << 16;
	int line = kEnemyInfoLine;
	njPrint(x | line, ":HP %4d", mHealth);
}

// 0x800AC1D0
void TEnePenguin::vfn_10()
{
	if (isAppearing() != 0) {
		mFlags |= 0x10;
	} else if (mLight.isVisible(mModel, 500.0f, &mBody.mPos, 60.0f, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x800AC260
void TEnePenguin::render()
{
	njSetTexture(*(void**)sPenguinTextures);
	enemyEyeSet(*(NJS_TEXLIST**)sPenguinTextures, 0, mFrameKey);
	useArea();
	njPushMatrixEx();
	applyTransform();
	drawLit(&mLight, &mTint, mPose);
	njPopMatrixEx();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}
