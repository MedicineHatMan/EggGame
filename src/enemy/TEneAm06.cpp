#include "enemy/TEneAm06.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyCommon.h"
#include "engine/MotionPose.h"
#include "io/ResBank.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/ObjEnemy.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "enemy/EnemyFx.h"
#include "engine/TParticle.h"
#include "enemy/TEneAm01.h"
struct EnemyFx;
struct ColliShape;
struct ColliContact;
struct ObjShadow;

extern "C" {

MotionEntry sAm06Motions[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 29.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 19.0f, 0.2f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 22.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 16.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 16.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 10, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 12, -1, 0, 0.0f, 39.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 13, -1, 0, 0.0f, 20.0f, 0.2f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 14, -1, 0, 0.0f, 20.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 15, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 16, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
};
extern const int sAm06Bones[] = {
	17, 25, 26, 27, 29, 46, 49, -1,
}; // the nodes the segments follow
extern const ColliShape sAm06BodyShapes[4] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 10.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 13.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 13.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 13.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
extern const ColliShape sAm06AttackShapes[3] = {
	{ { 0.0f, 30.0f, 80.0f }, 0x0, 0, 0x0, 45.0f, 15.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 20.0f, 10.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 20.0f, 10.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};
}

static const char* sClassName = "TEneAm06";

inline void* TEneAm06::operator new(u32 size)
{
	return objHeapAlloc(gObjHeap, size);
}

inline void TEneAm06::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x80046590
extern "C" void initTEneAm06()
{
	resBankLoad(&sAm06Bank, "ar_ene_snake.arc");
	resBankAddTextures(&sAm06Bank, 0, "ene_snake.gvm");
	sAm06Models = resBankGetModels(&sAm06Bank);
	sAm06Textures = resBankGetTextures(&sAm06Bank);
	sAm06Motion = resBankGetMotion(&sAm06Bank, 0);
	sAm06SegAngle = 0;
}

extern "C" void shutdownTEneAm06()
{
	resBankRemoveTextures(&sAm06Bank, 0);
	resBankRelease(&sAm06Bank);
	sAm06Models = 0;
	sAm06Textures = 0;
	sAm06Motion = 0;
}

// 0x8004665C
extern "C" TObject* createTEneAm06(TObject* parent, SetDesc* desc)
{
	return new TEneAm06(parent, desc);
}

// 0x800466B0
TEneAm06::TEneAm06(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00 = (u32)sClassName;
	mSize = 0x2E4;
	init(desc);
}

// 0x80046748
void TEneAm06::init(SetDesc* desc)
{
	motionCtrlInit(&mAnim, sAm06Motions, sAm06Motion);
	motionCtrlPlay(&mAnim, 7);
	mBody.mRot.x = 0;
	mCollider = colliCreate(4, 1);
	colliCopyShapes(mCollider, (const ColliShape*)sAm06BodyShapes);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 255, 0, 0);
	for (int i = 0; i < 4; i++) {
		colliShapeSetBit8(mCollider, i);
		colliShapeSetTag(mCollider, i, i);
	}
	mHead = colliCreate(1, 1);
	colliCopyShapes(mHead, (const ColliShape*)sAm06AttackShapes);
	colliSetOwner(mHead, (u16)mObjId);
	colliSetMasks(mHead, 2, 255, 255, 0xF3);
	colliShapeSetBit10(mHead, 0);
	mBite = colliCreate(2, 1);
	colliCopyShapes(mBite, sAm06AttackShapes + 1);
	colliSetOwner(mBite, (u16)mObjId);
	colliSetMasks(mBite, 2, 255, 255, 3);
	*(u32*)((u8*)colliGetShape(mBite, 0) + 0x24) |= 0x10;
	*(u32*)((u8*)colliGetShape(mBite, 1) + 0x24) |= 0x10;
	hideHead();
	hideBite();
	mBrain = 0;
	mYaw = 0;
	readEnemyDesc(desc);
	mPose = allocPose(*(void**)sAm06Models);
	mModel = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags2 = 0;
	mTarget.x = 0.0f;
	mTarget.y = 0.0f;
	mTarget.z = 0.0f;
	mFrameKey = 0;
	for (int i = 0; i < 7; i++) {
		mSeg[i].x = 0.0f;
		mSeg[i].y = 0.0f;
		mSeg[i].z = 0.0f;
	}
	mAim = 0;
	mTurnMode = 0;
	mTurn = 0;
	mHold = 0;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 26.0f;
	mShadow._34 = 26.0f;
	shadowEnable(&mShadow);
	mGrabId = -1;
	mGrab = 0;
	mFx = 0;
	mBar = new TEnemyLifeBar(this);
	mBar->setMax((s16)mHealth);
	mBar->setMark((s16)mHealthStep);
	mBar->setValue((s16)mHealth);
	mBar->setSize(60.0f, 25.0f);
	mLight.reset();
	mTint.reset();
	enemyEffectSetLarge(10.0f, &mEffect, &mSeg[3]);
	mStep = 0;
	mWindup = 0;
	mBitten = 0;
	_2DC = 0;
	_2E0 = 0.0f;
	createBrain();
}

// 0x80046A68
TEneAm06::~TEneAm06()
{
	removeDrawTask(mModel);
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
	colliRetire(mBite);
	mBite = 0;
	colliRetire(mHead);
	mHead = 0;
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x80046B9C
void TEneAm06::setPos(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	mBody.mPos.x = x;
	f32 z = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x80046BE0
// A glow uses the smaller radius
f32 TEneAm06::vfn_40()
{
	if (mAction == ACT_GLOW) {
		return 30.0f;
	}
	return 60.0f;
}

// 0x80046BFC
void TEneAm06::syncCollider()
{
	placeBody();
	if ((mFlags2 & AM_HEAD) != 0) {
		placeHead();
	}
	if ((mFlags2 & AM_BITE) != 0) {
		placeBite();
	}
}

// 0x80046C50
// Health defaults to 30 and the stun length to 300
void TEneAm06::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	mYaw = mBody.mRot.y;
	mVariant = pickVariant(desc);
	int health = desc->getInt(0);
	mHealth = health <= 0 ? 30 : health;
	mHealthStep = divideBy5(mHealth);
	int stun = desc->getInt(3);
	mStunFrames = stun <= 0 ? 300 : stun;
}

// 0x80046CFC
// Hurts on shape 3 and only while the flag is set
int TEneAm06::onHit(const TObjAction* other, int shape, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit = 0;
	if ((mFlags2 & AM_HURTING) != 0 && shape == 3) {
		if (isPlayerOrKind4(other) != 0) {
			info->mDamage = 20;
			hit = 1;
			info->mFlags.clear();
		} else if (isEggObj(other) != 0) {
			info->mDamage = 1;
			hit = 1;
			info->mFlags.clear();
			info->mFlags.setBit(0xD);
		}
	}
	return hit;
}

// 0x80046D9C
int TEneAm06::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

int TEneAm06::isReared()
{
	return (mFlags2 & AM_REARED) != 0;
}

// 0x80046DCC
int TEneAm06::isCoiled()
{
	return mFlags2 & AM_COILED;
}

void TEneAm06::setAim(int angle)
{
	mAim = angle;
}

void* TEneAm06::headShape()
{
	return mHead;
}

void TEneAm06::setHold(int angle)
{
	mHold = angle;
}

// 0x80046DF0
int TEneAm06::isStruck()
{
	return mFlags2 & AM_STRUCK;
}

void TEneAm06::setGrabId(u16 id)
{
	mGrabId = id;
}

// 0x80046E04
int TEneAm06::isBitten()
{
	return mFlags2 & AM_BITTEN;
}

int TEneAm06::getGrab()
{
	return mGrab;
}

int TEneAm06::isStaggerDone()
{
	return isActionDone(ACT_STAGGER);
}

int TEneAm06::stunFrames()
{
	return mStunFrames;
}

void TEneAm06::grab()
{
	mFlags2 |= AM_GRABBED;
}

void TEneAm06::release()
{
	mFlags2 &= ~AM_GRABBED;
}

int TEneAm06::isFlinchDone()
{
	return isActionDone(ACT_FLINCH);
}

// 0x80046E88
void TEneAm06::noteDeath()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
}

int TEneAm06::isFlipDone()
{
	return isActionDone(ACT_FLIP);
}

// 0x80046EEC
void TEneAm06::noteFlip()
{
	noteDeath();
	seStartAt(0x106, &mBody.mPos);
}

// 0x80046F24
int TEneAm06::isAimed()
{
	return mFlags2 & AM_AIMED;
}

// 0x80046F30
void TEneAm06::noteCrush()
{
	enemyFxSparks(3.0f, &mContact.mSurface, &mContact.mCenter);
	seEnemyCrush(&mBody.mPos);
}

int TEneAm06::isDownDone()
{
	return isActionDone(ACT_DOWN);
}

// 0x80046F94
void TEneAm06::noteDie()
{
	noteDeath();
	seStartAt(0x107, &mBody.mPos);
}

// 0x80046FCC
// The brain asks which death to use
int TEneAm06::deathAction()
{
	return ACT_DIE;
}

// 0x80046FD4
void TEneAm06::stopEffect()
{
	enemyEffectStart(&mEffect);
}

// 0x80046FF8
void TEneAm06::updateBody()
{
	if (isBlocked() == 0 && isInRange(this, &mBody.mPos, 500.0f) == 0) {
		resetLink();
		mFlags |= 1;
		return;
	}
	if (isBlocked() == 0 && takeHits(&mHit, &mContact) != 0) {
		applyDamage(&mHit);
	}
	checkFellOut();
	mBrain->update(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	mBody.mRot.y = mTurn + mYaw;
	mTint.update();
	applyMotion();
	applyPoseFaded(mPose, 60.0f, &mAnim, &mAppearTimer);
	placePoints();
	syncCollider();
	mBar->setPos(&mBody.mPos);
	mBar->setValue((s16)mHealth);
	enemyEffectUpdate(&mEffect);
}

// 0x80047164
// returnZero is 0 so this never runs
void TEneAm06::vfn_6C()
{
	if (returnZero() != 0) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		u32 saved = mFlags2;
		applyMotion();
		drawFirstFrame(sAm06Motion, 0x11, mPose);
		placePoints();
		syncCollider();
		mFlags2 = saved;
	}
}

// 0x800471E0
// The description is ignored Every snake is variant 0
int TEneAm06::pickVariant(SetDesc*)
{
	return 0;
}

// 0x800471E8
void TEneAm06::createBrain()
{
	mBrain = Am06SnakeBrain::create(this);
}

// 0x80047218
// Rotates a segment on two of the pose steps
static void poseSegments()
{
	nodeTrackOrigins();
	int step = nodeTrackIndex();
	if (step == 14 || step == 18) {
		gjRotateX(sAm06SegAngle);
	}
	nodeTrackNext();
}

// 0x80047258
void TEneAm06::applyMotion()
{
	nodeTrackSet(sAm06Bones, mSeg);
	sAm06SegAngle = mTurn / 2;
	nodeTrackSetCallback(poseSegments);
}

// 0x800472AC
// Poses the body points into the world and the bite points while that shape is live
void TEneAm06::placePoints()
{
	nodeTrackEnd();
	gjPushUnitMatrix();
	njTranslateEx(&mBody.mPos);
	gjRotateY(mYaw);
	if ((mFlags2 & AM_GLOW) != 0) {
		njCalcPointEx(&mSeg[1], &mSeg[1]);
	}
	njCalcPointEx(&mSeg[0], &mSeg[0]);
	njCalcPointEx(&mSeg[5], &mSeg[5]);
	njCalcPointEx(&mSeg[6], &mSeg[6]);
	njCalcPointEx(&mSeg[3], &mSeg[3]);
	if ((mFlags2 & AM_BITE) != 0) {
		njCalcPointEx(&mSeg[2], &mSeg[2]);
		njCalcPointEx(&mSeg[4], &mSeg[4]);
	}
	njPopMatrixEx();
}

// 0x8004735C
// 3 while grabbed otherwise 1
int TEneAm06::getHitKind(const void*, HitInfo*)
{
	if ((mFlags2 & AM_GRABBED) != 0) {
		return 3;
	}
	return 1;
}

// 0x80047374
// Only a kind-3 hit carries its damage
int TEneAm06::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x8004738C
// The result is signed so a value past 0x7FFF wraps
int TEneAm06::approachAngle(int from, int to, int rate)
{
	int next = angleApproach(from, to, rate);
	if (next >= 0x8000) {
		next -= 0x10000;
	}
	return next;
}

// 0x800473CC
void TEneAm06::easeTurn(int diff, int rate)
{
	mTurn = approachAngle(mTurn, diff, rate);
}

// 0x8004740C
void TEneAm06::sway()
{
	stepYaw(0x200);
}

// 0x80047430
// Walks mYaw toward eating mTurn by at most step per call
void TEneAm06::stepYaw(int step)
{
	if (mTurn == 0) {
		return;
	}
	if (mTurn < step) {
		if (mTurn > -step) {
			mYaw += mTurn;
			mTurn = 0;
		} else {
			mYaw -= step;
			mTurn += step;
		}
	} else {
		mYaw += step;
		mTurn -= step;
	}
}

// 0x800474A0
void TEneAm06::clearTurn()
{
	mTurnMode = 0;
}

// 0x800474AC
// Turns toward mAim and eases mTurn by the angle still left
void TEneAm06::track()
{
	int diff = angleDiff(mYaw, mAim);
	int gap = diff < 0 ? -diff : diff;
	if (mTurnMode == 1) {
		if (gap > 0x471C) {
			mTurnMode = 0;
		}
	} else {
		if (gap < 0x1555) {
			mTurnMode = 1;
		}
		mYaw = approachAngle(mYaw, mAim, 0x200);
		diff = angleDiff(mYaw, mAim);
	}
	easeTurn(diff, 0x400);
}

// 0x80047554
void TEneAm06::fall()
{
	mBody.mVel.y += -0.12f;
	moveVertical(mBody.mVel.y, &mBody.mPos, &mShadow);
	if (isAirborne() == 0) {
		mBody.mVel.y = 0.0f;
	}
}

// 0x800475C0
// Parts 0 and 3 of the body the ones the glow uses
void TEneAm06::armHead()
{
	colliShapeClearBit8(mCollider, 0);
	colliShapeSetBit10(mCollider, 0);
	colliShapeClearBit8(mCollider, 3);
	colliShapeSetBit10(mCollider, 3);
}

// 0x80047618
void TEneAm06::disarmHead()
{
	colliShapeSetBit8(mCollider, 0);
	colliShapeClearBit10(mCollider, 0);
	colliShapeSetBit8(mCollider, 3);
	colliShapeClearBit10(mCollider, 3);
}

// 0x80047670
void TEneAm06::showHead()
{
	colliEnable(mHead);
	colliClearContacts(mHead);
	mFlags2 |= AM_HEAD;
}

// 0x800476B4
void TEneAm06::hideHead()
{
	colliDisable(mHead);
	colliClearContacts(mHead);
	mFlags2 |= AM_HEAD;
}

// 0x800476F8
void TEneAm06::showBite()
{
	colliEnable(mBite);
	colliClearContacts(mBite);
	mFlags2 |= AM_BITE;
}

// 0x8004773C
void TEneAm06::hideBite()
{
	colliDisable(mBite);
	colliClearContacts(mBite);
	mFlags2 &= ~AM_BITE;
}

// 0x80047780
// Copies four body points into the four shapes
void TEneAm06::placeBody()
{
	Vec3f* part = (Vec3f*)colliGetShapes(mCollider);
	part[0] = mSeg[0];
	part[4] = mSeg[5];
	part[8] = mSeg[6];
	part[12] = mSeg[3];
	colliRefreshBounds(mCollider);
	colliBoundsOn(mCollider);
}

// 0x80047820
void TEneAm06::placeHead()
{
	colliPlaceShapesRotY(mHead, &mBody.mPos, mBody.mRot.y, sAm06AttackShapes);
	colliRefreshBounds(mHead);
	colliBoundsOn(mHead);
}

// 0x80047870
// The bite sits between the second and fourth points and the head point
void TEneAm06::placeBite()
{
	Vec3f* part = (Vec3f*)colliGetShapes(mBite);
	Vec3f mid;
	mid = mSeg[2];
	njAddVector(&mid, &mSeg[4]);
	gjScaleVector(0.5f, &mid, &mid);
	f32 height = 0.5f * (0.5f * (mSeg[2].y - mSeg[4].y));
	part[0] = mid;
	colliShapeSetCylHeight((ColliShape*)&part[0], height);
	Vec3f head;
	head = mSeg[0];
	head.y = mid.y;
	gjScaleVector(0.6f, &head, &head);
	gjScaleVector(0.4f, &mid, &mid);
	njAddVector(&head, &mid);
	part[4] = head;
	colliShapeSetCylHeight((ColliShape*)&part[4], height);
	colliRefreshBounds(mBite);
	colliBoundsOn(mBite);
}

// 0x800479A4
void TEneAm06::onActionPhase(int action, int phase)
{
	switch (action) {
	case ACT_APPEAR:
		actAppear(phase);
		break;
	case ACT_REAR:
		actGlow(phase);
		break;
	case ACT_GLOW:
		actCoil(phase);
		break;
	case ACT_COIL:
		actRear(phase);
		break;
	case ACT_SWAY:
		actSway(phase);
		break;
	case ACT_BITE:
		actStrike(phase);
		break;
	case ACT_STRIKE:
		actBite(phase);
		break;
	case ACT_STAGGER:
		actStagger(phase);
		break;
	case ACT_SPIT:
		actSpit(phase);
		break;
	case ACT_FLINCH:
		actFlinch(phase);
		break;
	case ACT_FLIP:
		actFlip(phase);
		break;
	case ACT_DIE:
		actDie(phase);
		break;
	case ACT_DOWN:
		actDown(phase);
		break;
	}
}

// 0x80047A78
void TEneAm06::actAppear(int phase)
{
	int* step = &mStep;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 10);
		beginAppear(step);
		mFrameKey = 2;
		mTurn = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		runAppear(step);
		break;
	case 2:
		endAppear(step);
		break;
	}
}

// 0x80047B14
void TEneAm06::actRear(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 9);
		mFlags2 &= ~AM_REARED;
		mFrameKey = 2;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlIsDone(&mAnim) != 0) {
			mFlags2 |= AM_REARED;
		}
		sway();
		fall();
		break;
	case 2:
		mFrameKey = 0;
		break;
	}
}

// 0x80047BC4
// A glow at the first body point for 60 frames
void TEneAm06::actGlow(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 10);
		mFrameKey = 2;
		mTurn = 0;
		mStep = 60;
		mFlags2 |= AM_GLOW;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (mStep <= 0) {
			if (mFx == 0) {
				mFx = (EnemyFx*)particleGenByIdDefault(&mSeg[1], 0, 0x51);
			} else {
				mFx->mPos = mSeg[1];
			}
		} else {
			mStep--;
		}
		fall();
		break;
	case 2:
		mFrameKey = 0;
		mFlags2 &= ~AM_GLOW;
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		break;
	}
}

// 0x80047CDC
void TEneAm06::actCoil(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xE);
		mFlags2 &= ~AM_COILED;
		mFrameKey = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlIsDone(&mAnim) != 0) {
			mFlags2 |= AM_COILED;
		}
		sway();
		break;
	case 2:
		break;
	}
}

// 0x80047D70
void TEneAm06::actSway(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.08f, &mAnim, 0xD);
		mFlags2 |= AM_AIMED | AM_SWAY;
		clearTurn();
		showHead();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		track();
		break;
	case 2:
		mFlags2 &= ~(AM_AIMED | AM_SWAY);
		hideHead();
		break;
	}
}

// 0x80047E14
// Winds up to face mHold bites then settles
void TEneAm06::actStrike(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0);
		mFlags2 &= ~(AM_STRUCK | AM_HURTING);
		mFrameKey = 4;
		mStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		switch (mStep) {
		case 0: {
			mTurn = approachAngle(mTurn, 0, 0x300);
			int aim = (mHold - 0x2A00) & 0xFFFF;
			mYaw = angleApproach(mYaw, aim, 0x400);
			if (mYaw == aim && mTurn == 0) {
				mStep = 1;
				mFlags2 |= AM_HURTING;
			}
			if (motionCtrlFrame(&mAnim) >= 9.0f) {
				motionCtrlSetFrame(9.0f, &mAnim);
			}
			break;
		}
		case 1:
			if (motionCtrlPassed(8.0f, &mAnim) != 0) {
				seStartAt(0x105, &mBody.mPos);
			}
			if (motionCtrlPassed(13.0f, &mAnim) != 0) {
				mFlags2 &= ~AM_HURTING;
			} else if (motionCtrlPassed(21.0f, &mAnim) != 0) {
				mStep = 2;
				clearTurn();
			}
			break;
		case 2:
			track();
			if (motionCtrlIsDone(&mAnim) != 0) {
				mFlags2 |= AM_STRUCK;
			}
			break;
		}
		break;
	case 2:
		mFlags2 &= ~AM_HURTING;
		mFrameKey = 0;
		break;
	}
}

// 0x80047FF0
void TEneAm06::actBite(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 4);
		mFlags2 &= ~AM_BITTEN;
		mFrameKey = 4;
		hideBite();
		mGrab = 0;
		mStep = 0;
		mWindup = 1;
		mBitten = 0;
		break;
	case 1:
		if (mWindup != 0 && motionCtrlFrame(&mAnim) > 4.0f) {
			motionCtrlSetFrame(8.0f, &mAnim);
			mWindup = 0;
		}
		motionCtrlUpdate(&mAnim);
		stepYaw(0x800);
		TObjAction* grabbed = findObjById((u16)mGrabId);
		if (grabbed != 0) {
			Vec3f* pos = grabbed->getPos();
			Vec3f delta;
			delta = *pos;
			njSubVector(&delta, &mBody.mPos);
			int angle = (int)(10430.38043493439 * (f32)atan2(delta.x, delta.z));
			mYaw = angleApproach(mYaw, angle, 0x200);
		}
		switch (mStep) {
		case 0:
			if (motionCtrlPassed(10.5f, &mAnim) != 0) {
				showBite();
				mStep = 1;
				mEnemyFlags |= ENEMY_INTANGIBLE;
				armHead();
			}
			break;
		case 1:
			if (mBitten == 0) {
				for (ColliContact* touch = mBite->mContacts; touch != 0; touch = touch->mNext) {
					if ((u16)colliContactOwner(touch) == (u16)mGrabId) {
						ColliShape* shape = colliContactShape(touch);
						if ((shape->mFlags & 0x10) == 0) {
							mBitten = 1;
							break;
						}
					}
				}
			}
			if (motionCtrlFrame(&mAnim) >= 13.5f && mBitten != 0) {
				if (grabbed != 0) {
					struct {
						u16 mType;
						u16 mFrom;
					} msg;
					msg.mType = 0x73;
					msg.mFrom = mObjId;
					if (grabbed->sendToHandle((ObjMsg*)&msg) != 0) {
						mGrab = 1;
					} else {
						mGrab = 2;
					}
					grabbed->mFlags |= 1;
				} else {
					mBitten = 0;
					mGrab = 0;
				}
				mStep = 2;
				hideBite();
				disarmHead();
			}
			if (motionCtrlPassed(15.0f, &mAnim) != 0) {
				mStep = 2;
				hideBite();
			}
			break;
		case 2:
			if (motionCtrlIsDone(&mAnim) != 0) {
				if (mBitten != 0) {
					motionCtrlPlay(&mAnim, 6);
				} else {
					motionCtrlPlay(&mAnim, 5);
					mGrab = 0;
				}
				mStep = 3;
			}
			break;
		case 3:
			if (mGrab != 0 && motionCtrlPassed(5.0f, &mAnim) != 0) {
				seStartAt(0x104, &mBody.mPos);
			}
			if (motionCtrlIsDone(&mAnim) != 0) {
				mFlags2 |= AM_BITTEN;
			}
			break;
		}
		break;
	case 2:
		hideBite();
		mEnemyFlags &= ~ENEMY_INTANGIBLE;
		disarmHead();
		mFrameKey = 0;
		break;
	}
}

// 0x80048388
void TEneAm06::actStagger(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xB);
		mFrameKey = 3;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlIsDone(&mAnim) != 0) {
			mHitFlags |= HIT_DONE;
		}
		break;
	case 2:
		mFrameKey = 0;
		break;
	}
}

// 0x8004841C
// The same glow as actGlow lifted on motion 12
void TEneAm06::actSpit(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.08f, &mAnim, 0xC);
		mFrameKey = 3;
		mFlags2 |= AM_GLOW;
		mFrameKey = 3;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (mFx == 0) {
			mFx = (EnemyFx*)particleGenByIdDefault(&mSeg[1], 0, 0x47);
		} else {
			Vec3f at = mSeg[1];
			at.y += -13.0f;
			mFx->mPos = at;
		}
		break;
	case 2:
		mFrameKey = 0;
		mFlags2 &= ~AM_GLOW;
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		break;
	}
}

// 0x80048530
void TEneAm06::actFlinch(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 8);
		mFrameKey = 3;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlIsDone(&mAnim) != 0) {
			mHitFlags |= HIT_DONE;
		}
		break;
	case 2:
		mFrameKey = 0;
		break;
	}
}

// 0x800485C4
void TEneAm06::actFlip(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 2);
		mEnemyFlags |= ENEMY_FLIPPING;
		mFrameKey = 3;
		noteFlip();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlIsDone(&mAnim) != 0) {
			mHitFlags |= HIT_DONE;
		}
		sway();
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		mFrameKey = 0;
		break;
	}
}

// 0x80048680
void TEneAm06::actDie(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 3);
		mEnemyFlags |= ENEMY_BLOCKED | ENEMY_INTANGIBLE;
		colliDisable(mCollider);
		hideBite();
		hideHead();
		mBar->hide();
		killed(1);
		mFrameKey = 3;
		noteDie();
		mStep = 0x5A;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlPassed(13.0f, &mAnim) != 0) {
			particleOnceByIdDefault(&mSeg[3], 0, 0x46);
			seEnemyPuff(&mBody.mPos);
		}
		if (motionCtrlIsDone(&mAnim) != 0) {
			mStep--;
			if (mStep <= 0) {
				enemyFxDeath4A(&mBody.mPos);
				spawnDropB(&mBody.mPos);
				mFlags |= 1;
			}
		}
		sway();
		break;
	case 2:
		mFrameKey = 0;
		break;
	}
}

// 0x800487B8
void TEneAm06::actDown(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xF);
		noteCrush();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlIsDone(&mAnim) != 0) {
			mHitFlags |= HIT_DONE;
		}
		sway();
		break;
	case 2:
		break;
	}
}

// 0x80048840
void TEneAm06::vfn_3C()
{
	int x = kEnemyInfoX << 16;
	int line = kEnemyInfoLine;
	njPrint(x | line, ":HP %4d", mHealth);
	njPrint(x | (line + 3), ":StunFrm %4d", mStunFrames);
}

// 0x800488A4
// Hidden while appearing or out of view
void TEneAm06::vfn_10()
{
	if (isAppearing() != 0) {
		mFlags |= 0x10;
	} else if (mLight.isVisible(mModel, 500.0f, &mBody.mPos, 70.0f, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x80048934
void TEneAm06::render()
{
	njSetTexture(*(void**)sAm06Textures);
	enemyEyeSet(*(NJS_TEXLIST**)sAm06Textures, 0, mFrameKey);
	useArea();
	njPushMatrixEx();
	njTranslateEx(&mBody.mPos);
	gjRotateY(mYaw);
	drawLit(&mLight, &mTint, mPose);
	njPopMatrixEx();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mYaw);
}
