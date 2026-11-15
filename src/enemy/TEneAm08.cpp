#include "enemy/TEneAm08.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyCommon.h"
#include "engine/MotionPose.h"
#include "engine/CharaColli.h"
#include "io/ResBank.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/ObjEnemy.h"
#include "enemy/EnemyFx.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "enemy/EnemyHit.h"
#include "engine/TParticle.h"
#include "enemy/TEneAm01.h"
struct ObjShadow;

extern "C" {
static ResBank sAm08Bank;
NJS_OBJECT** sAm08Models;
NJS_TEXLIST** sAm08Textures;
NJS_MOTION** sAm08Motion;

MotionEntry sAm08Motions[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },  { MOTION_LOOP, 1, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },  { MOTION_ONCE, 3, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },  { MOTION_ONCE, 5, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 14.0f, 0.4f, 0.0f, 0 },  { MOTION_ONCE, 8, -1, 0, 0.0f, 24.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 24.0f, 0.3f, 0.0f, 0 },  { MOTION_LOOP, 9, -1, 0, 0.0f, 20.0f, 0.6f, 0.0f, 0 },
	{ MOTION_LOOP, 10, -1, 0, 0.0f, 20.0f, 0.3f, 0.0f, 0 }, { MOTION_LOOP, 11, -1, 0, 0.0f, 30.0f, 0.4f, 0.0f, 0 },
};
extern const int sAm08Nodes[] = {
	11, 15, 16, 22, 23, 53, 55, 64, -1,
};
const char* sClassName                      = "TEneAm08";
extern const EnemyFrameKey sAm08FrameKey0[] = { { 10.0f, 0 }, { -1.0f, 5 } };
extern const EnemyFrameKey sAm08FrameKey1[] = { { 7.0f, 5 }, { 12.0f, 0 }, { 18.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAm08FrameKey2[] = { { 15.0f, 0 }, { 25.0f, 5 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAm08FrameKey3[] = { { 1.0f, 0 }, { 7.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAm08FrameKey4[] = { { 9.0f, 3 }, { -1.0f, 2 } };
extern const EnemyFrameKey sAm08FrameKey5[] = { { 14.0f, 3 }, { -1.0f, 2 } };
const EnemyFrameKey* sAm08FrameKeys[6] = { sAm08FrameKey0, sAm08FrameKey1, sAm08FrameKey2, sAm08FrameKey3, sAm08FrameKey4, sAm08FrameKey5 };
extern const ColliShape sAm08Shapes[]  = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 13.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 10.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 4.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 4.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 12.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 13.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
extern const ColliShape sAm08BiteShape = { { 0.0f, 10.0f, 40.0f }, 0x0, 0, 0x0, 25.0f, 10.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 };
}

inline void* TEneAm08::operator new(u32 size)
{
	return objHeapAlloc(gObjHeap, size);
}

inline void TEneAm08::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x80073138
extern "C" void initTEneAm08()
{
	resBankLoad(&sAm08Bank, "ar_ene_lizard.arc");
	resBankAddTextures(&sAm08Bank, 0, "ene_lizard.gvm");
	sAm08Models   = resBankGetModels(&sAm08Bank);
	sAm08Textures = resBankGetTextures(&sAm08Bank);
	sAm08Motion   = resBankGetMotion(&sAm08Bank, 0);
}

extern "C" void shutdownTEneAm08()
{
	resBankRemoveTextures(&sAm08Bank, 0);
	resBankRelease(&sAm08Bank);
	sAm08Models   = 0;
	sAm08Textures = 0;
	sAm08Motion   = 0;
}

extern "C" TObject* createTEneAm08(TObject* parent, SetDesc* desc)
{
	return new TEneAm08(parent, desc);
}

TEneAm08::TEneAm08(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00   = (u32)sClassName;
	mSize = 0x318;
	init(desc);
}

// 0x800732F0
// Shapes 4 and 5 of the body are the jaws
void TEneAm08::init(SetDesc* desc)
{
	motionCtrlInit(&mAnim, sAm08Motions, sAm08Motion);
	motionCtrlPlay(&mAnim, 10);
	mBody.mRot.x = 0;
	mCollider    = colliCreate(6, 1);
	colliCopyShapes(mCollider, sAm08Shapes);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0x37, 0x14, 0x10);
	mBite = colliCreate(1, 1);
	colliCopyShapes(mBite, &sAm08BiteShape);
	colliSetOwner(mBite, (u16)mObjId);
	colliSetMasks(mBite, 2, 0xFF, 0xFF, 0xFE);
	colliShapeSetBit10(mBite, 0);
	int id = 0;
	for (int i = 0; i < 6; i++, id++) {
		colliShapeSetTag(mCollider, i, id);
	}
	colliShapeSetTag(mBite, 0, id);
	colliShapeSetBit8(mCollider, 4);
	colliShapeSetBit8(mCollider, 5);
	colliDisable(mBite);
	mBrain = 0;
	readEnemyDesc(desc);
	mPose     = allocPose(*(void**)sAm08Models);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags2   = 0;
	mPush.x   = 0.0f;
	mPush.y   = 0.0f;
	mPush.z   = 0.0f;
	mFrameKey = 0;
	for (int i = 0; i < 8; i++) {
		mPoints[i].x = 0.0f;
		mPoints[i].y = 0.0f;
		mPoints[i].z = 0.0f;
	}
	mRadius    = 0.0f;
	mAnchor.x  = 0.0f;
	mAnchor.y  = 0.0f;
	mAnchor.z  = 0.0f;
	mGoal.x    = 0.0f;
	mGoal.y    = 0.0f;
	mGoal.z    = 0.0f;
	mGoalAngle = 0;
	_200       = 0.0f;
	mBiteZ     = 40.0f;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 52.0f;
	mShadow._34 = 26.0f;
	shadowEnable(&mShadow);
	mBar = new TEnemyLifeBar(this);
	mBar->setMax((s16)mHealth);
	mBar->setMark((s16)mHealthStep);
	mBar->setValue((s16)mHealth);
	mBar->setSize(50.0f, 25.0f);
	mLight.reset();
	mLight.setFadeDist(-15.0f);
	mTint.reset();
	enemyEffectSetLarge(18.0f, &mEffect, &mPoints[5]);
	mStep = 0;
	_310  = 0;
	_314  = 0.0f;
	createBrain();
}

// 0x800735C8
TEneAm08::~TEneAm08()
{
	removeDrawTask(mDrawTask);
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
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x800736D8
void TEneAm08::setPos(const Vec3f* pos)
{
	f32 x        = pos->x;
	f32 y        = pos->y;
	mBody.mPos.x = x;
	f32 z        = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x8007371C
f32 TEneAm08::vfn_40()
{
	return 50.0f;
}

// 0x80073724
// 30% of the way from point b to point a
void TEneAm08::lerpPoints(int a, int b, Vec3f* out)
{
	out->x = 0.3f * mPoints[a].x + 0.7f * mPoints[b].x;
	out->y = 0.3f * mPoints[a].y + 0.7f * mPoints[b].y;
	out->z = 0.3f * mPoints[a].z + 0.7f * mPoints[b].z;
}

// 0x8007377C
// While AM_BITE is set mBite steps out in front of the mouth each frame from 40 to 90
void TEneAm08::syncCollider()
{
	ColliShape* part = colliGetShapes(mCollider);
	part[0].mPos     = mPoints[0];
	lerpPoints(1, 2, &part[2].mPos);
	lerpPoints(3, 4, &part[3].mPos);
	part[1].mPos = mPoints[5];
	part[5].mPos = mPoints[6];
	Vec3f tail;
	tail.x = mPoints[7].x;
	tail.y = mPoints[7].y;
	tail.z = mPoints[7].z;
	njAddVector(&tail, &mPoints[5]);
	gjScaleVector(0.5f, &tail, &tail);
	part[4].mPos = tail;
	colliRefreshBounds(mCollider);
	colliBoundsOn(mCollider);
	if ((mFlags2 & AM_BITE) != 0) {
		ColliShape bite = sAm08BiteShape;
		bite.mPos.z     = mBiteZ;
		mBiteZ += 12.5f;
		if (mBiteZ >= 90.0f) {
			mBiteZ = 40.0f;
		}
		colliEnable(mBite);
		colliPlaceShapesRotY(mBite, &mBody.mPos, mBody.mRot.y, &bite);
	} else {
		colliDisable(mBite);
	}
}

// 0x8007390C
// Health defaults to 50
void TEneAm08::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	int health  = desc->getInt(0);
	mHealth     = health > 0 ? health : 50;
	mHealthStep = divideBy5(mHealth);
}

// 0x80073974
// Only the jaws hurt and only while biting
int TEneAm08::isBiting(int shape)
{
	u8 biting = (mFlags2 & AM_BITING) && (shape == 4 || shape == 5);
	return biting;
}

// 0x800739B8
// A player takes 20 an egg 1 and another enemy 20 and a push along the lunge
int TEneAm08::onHit(const TObjAction* other, int shape, int* result)
{
	int hit       = 0;
	HitInfo* info = (HitInfo*)result;
	if (isBiting(shape) != 0) {
		if (isPlayerOrKind4(other) != 0) {
			info->mDamage = 20;
			hit           = 1;
			info->mFlags.clear();
		} else if (isEggObj(other) != 0) {
			info->mDamage = 1;
			hit           = 1;
			info->mFlags.clear();
			info->mFlags.setBit(0xD);
		} else if (isEnemyObj(other) != 0) {
			info->mDamage = 20;
			info->mFlags.clear();
			info->mFlags.setBit(1);
			info->mFlags.setBit(0xC);
			hit        = 1;
			info->mPos = mBody.mVel;
			info->_14  = -1;
		}
	}
	return hit;
}

// 0x80073AB8
void TEneAm08::setAnchor(const Vec3f* pos)
{
	mAnchor = *pos;
}

// 0x80073AD4
void TEneAm08::setGoal(const Vec3f* pos, int angle)
{
	mGoal      = *pos;
	mGoalAngle = angle;
}

// 0x80073AF4
int TEneAm08::isLunging()
{
	return mFlags2 & AM_LUNGING;
}

// 0x80073B00
void TEneAm08::noteDeath()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
}

// 0x80073B40
void TEneAm08::stopEffect()
{
	enemyEffectStart(&mEffect);
}

int TEneAm08::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

// 0x80073B88
int TEneAm08::hasArrived()
{
	return mFlags2 & AM_ARRIVED;
}

int TEneAm08::isThreatDone()
{
	return isActionDone(ACT_THREAT);
}

// 0x80073BB8
// mBite is touching something
int TEneAm08::canBite()
{
	u8 bite = 0;
	if ((mFlags2 & AM_BITE) != 0 && colliGetContactCount(mBite) > 0) {
		bite = 1;
	}
	return bite;
}

int TEneAm08::isLungeDone()
{
	return isActionDone(ACT_LUNGE);
}

int TEneAm08::isSlideDone()
{
	return isActionDone(ACT_SLIDE);
}

int TEneAm08::isRecoverDone()
{
	return isActionDone(ACT_RECOVER);
}

// 0x80073C70
int TEneAm08::canFlip()
{
	return !(mFlags2 & AM_GUARDING);
}

// 0x80073C84
void TEneAm08::noteFlip()
{
	noteDeath();
	seStartAt(0xEB, &mBody.mPos);
}

int TEneAm08::isFlipLungingDone()
{
	return isActionDone(ACT_FLIP_LUNGING);
}

int TEneAm08::isFlipDone()
{
	return isActionDone(ACT_FLIP);
}

// 0x80073D04
void TEneAm08::noteDie()
{
	noteDeath();
	seStartAt(0xEC, &mBody.mPos);
}

// 0x80073D3C
int TEneAm08::deathAction()
{
	return isLunging() != 0 ? ACT_DIE_LUNGING - 1 : ACT_DIE_LUNGING;
}

// 0x80073D68
void TEneAm08::updateBody()
{
	if (isBlocked() == 0 && isInRange(this, &mBody.mPos, 500.0f) == 0) {
		resetLink();
		mFlags |= 1;
		return;
	}
	if (isBlocked() == 0 && takeHits(&mHit, &mContact) != 0) {
		enemyHitPushAway((Vec3f*)&mHit, &mBody.mPos, (Vec3f*)&mContact, &mPush);
		applyDamage(&mHit);
	}
	mBrain->update(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	mTint.update();
	if ((mFlags2 & AM_TRAIL) != 0) {
		Vec3f pos;
		njSinCos(mBody.mRot.y, &pos.x, &pos.z);
		pos.y = 0.0f;
		gjScaleVector(35.0f, &pos, &pos);
		njAddVector(&pos, &mPoints[5]);
		mFx.start(5, 10, &pos, 0, mBody.mRot.y);
	} else {
		fxHolderStop(&mFx);
	}
	syncCollider();
	mBar->setPos(&mBody.mPos);
	mBar->setValue((s16)mHealth);
	enemyEffectUpdate(&mEffect);
}

// 0x80073F10
// returnZero is 0 so this never runs
void TEneAm08::vfn_6C()
{
	if (returnZero() != 0) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginPoints();
		drawFirstFrame(sAm08Motion, 0xC, mPose);
		endPoints();
		placePoints();
		syncCollider();
	}
}

// 0x80073F8C
void TEneAm08::createBrain()
{
	mBrain = Am08LizardBrain::create(this);
}

// 0x80073FBC
void TEneAm08::pose()
{
	beginPoints();
	applyPoseFaded(mPose, 50.0f, &mAnim, &mAppearTimer);
	endPoints();
}

// 0x80074008
void TEneAm08::beginPoints()
{
	nodeTrackBeginOrigins(sAm08Nodes, mPoints);
}

// 0x80074038
void TEneAm08::endPoints()
{
	nodeTrackEnd();
}

// 0x80074058
// Takes the points into the world and measures how far the tail reaches
void TEneAm08::placePoints()
{
	gjPushUnitMatrix();
	bodyMatrix();
	for (int i = 0; i < 8; i++) {
		njCalcPointEx(&mPoints[i], &mPoints[i]);
	}
	njPopMatrixEx();
	Vec3f d;
	d.x = mPoints[7].x;
	d.y = mPoints[7].y;
	d.z = mPoints[7].z;
	njSubVector(&d, &mBody.mPos);
	d.y     = 0.0f;
	mRadius = njScalor(&d);
}

// 0x800740F0
int TEneAm08::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x80074108
void TEneAm08::bodyMatrix()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
}

// 0x80074140
// Root motion how far the point moved in this frame's pose becomes the velocity plus gravity
void TEneAm08::stepAlong(int point, Vec3f* vel)
{
	Vec3f from;
	from.x = mPoints[point].x;
	from.y = mPoints[point].y;
	from.z = mPoints[point].z;
	pose();
	gjPushUnitMatrix();
	bodyMatrix();
	Vec3f to;
	njCalcPointEx(&mPoints[point], &to);
	njPopMatrixEx();
	moveAlong(1.0f, &from, &to, vel);
	vel->y += -0.12f;
}

// 0x800741D8
void TEneAm08::drag(f32 rate, Vec3f* vel)
{
	vel->x *= rate;
	vel->y += -0.12f;
	vel->z *= rate;
}

// 0x80074204
void TEneAm08::fall(Vec3f* vel)
{
	vel->y += -0.12f;
}

// 0x80074218
// On the ground the fall stops
int TEneAm08::move(Vec3f* vel)
{
	Vec3f push;
	getFlatPush(&push);
	njAddVector(&push, vel);
	int hit = collideFiltered(mRadius, &mBody.mPos, &push, &mShadow);
	if (isAirborne() == 0) {
		vel->y = 0.0f;
	}
	return hit;
}

// 0x800742A0
void TEneAm08::setForward(f32 speed)
{
	njSinCos(mBody.mRot.y, &mBody.mVel.x, &mBody.mVel.z);
	mBody.mVel.x *= speed;
	mBody.mVel.z *= speed;
}

// 0x80074304
f32 TEneAm08::flatSpeed()
{
	f32 x = mBody.mVel.x;
	f32 z = mBody.mVel.z;
	x *= x;
	z *= z;
	x += z;
	return __fres(__frsqrte(x));
}

// 0x80074328
void TEneAm08::onActionPhase(int action, int phase)
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
	case ACT_THREAT:
		actThreat(phase);
		break;
	case ACT_APPROACH:
		actApproach(phase);
		break;
	case ACT_LUNGE:
		actLunge(phase);
		break;
	case ACT_SLIDE:
		actSlide(phase);
		break;
	case ACT_RECOVER:
		actRecover(phase);
		break;
	case ACT_FLIP_LUNGING:
		actFlipLunging(phase);
		break;
	case ACT_FLIP:
		actFlip(phase);
		break;
	case ACT_DIE:
		actDie(phase);
		break;
	case ACT_DIE_LUNGING:
		actDieLunging(phase);
		break;
	}
}

// 0x800743F0
void TEneAm08::actAppear(int phase)
{
	int* step = &mStep;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 10);
		beginAppear(step);
		mFrameKey = 2;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		runAppear(step);
		pose();
		placePoints();
		break;
	case 2:
		endAppear(step);
		mFrameKey = 0;
		break;
	}
}

// 0x8007449C
void TEneAm08::actIdle(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 10);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		drag(0.6f, &mBody.mVel);
		move(&mBody.mVel);
		pose();
		placePoints();
		break;
	case 2:
		break;
	}
}

// 0x80074530
// Walks to mAnchor on the motion's own steps the front foot first
void TEneAm08::actWalk(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.05f, &mAnim, 11);
		mFlags2 &= ~AM_ARRIVED;
		clearFrameKey();
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		faceToward(&mAnchor, &mBody.mPos, (void*)0x100);
		int point;
		if (motionCtrlFrame(&mAnim) < 15.0f && motionCtrlIsBlending(&mAnim) == 0) {
			point = 2;
		} else {
			point = 4;
		}
		stepAlong(point, &mBody.mVel);
		move(&mBody.mVel);
		if (probeLine(&mAnchor, &mBody.mPos, (void*)0x100, &mBody.mVel) != 0) {
			mFlags2 |= AM_ARRIVED;
		} else {
			mFlags2 &= ~AM_ARRIVED;
		}
		pose();
		placePoints();
		break;
	}
	case 2:
		mFlags2 &= ~AM_ARRIVED;
		break;
	}
}

// 0x80074664
// Rears up and turns to the player
void TEneAm08::actThreat(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 4);
		mFlags2 |= AM_GUARDING;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		setFrameKey(sAm08FrameKeys[2]);
		waitMotion();
		if (motionCtrlIsBlending(&mAnim) == 0 && motionCtrlFrame(&mAnim) <= 5.0f) {
			turnTo(mGoalAngle, 0x800);
		}
		drag(0.6f, &mBody.mVel);
		move(&mBody.mVel);
		if (motionCtrlPassed(15.0f, &mAnim) != 0) {
			seStartAt(0xE9, &mBody.mPos);
		}
		pose();
		placePoints();
		break;
	case 2:
		mFlags2 &= ~(AM_ARRIVED | AM_BITING | AM_GUARDING);
		break;
	}
}

// 0x8007478C
// Closes in with the jaws open
void TEneAm08::actApproach(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 9);
		mFlags2 |= AM_BITE | AM_BITING | AM_GUARDING;
		mFrameKey = 5;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		int point;
		if (motionCtrlFrame(&mAnim) < 10.0f && motionCtrlIsBlending(&mAnim) == 0) {
			point = 2;
		} else {
			point = 4;
		}
		turnTo(mGoalAngle, 0x100);
		stepAlong(point, &mBody.mVel);
		f32 s;
		f32 c;
		njSinCos(mBody.mRot.y, &s, &c);
		mBody.mVel.x = 1.2f * s + mBody.mVel.x;
		mBody.mVel.z = 1.2f * c + mBody.mVel.z;
		move(&mBody.mVel);
		pose();
		placePoints();
		break;
	}
	case 2:
		mFlags2 &= ~(AM_BITE | AM_BITING | AM_GUARDING);
		clearFrameKey();
		break;
	}
}

// 0x800748C0
// Speeds up through the wind-up then leaps at 4.2
void TEneAm08::actLunge(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0);
		mFlags2 &= ~(AM_LUNGING | AM_TRAIL);
		mFlags2 |= AM_BITING | AM_GUARDING;
		mStep = 0;
		setForward(3.0f);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		setFrameKey(sAm08FrameKeys[0]);
		switch (mStep) {
		case 0: {
			turnTo(mGoalAngle, 0x40);
			f32 speed;
			if (motionCtrlFrame(&mAnim) >= 15.0f) {
				speed = 4.2f;
				mStep = 1;
				mFlags2 |= AM_LUNGING | AM_TRAIL;
				seStartAt(0xEA, &mBody.mPos);
			} else {
				speed = flatSpeed();
				speed *= 0.9f;
			}
			setForward(speed);
			fall(&mBody.mVel);
			Vec3f vel;
			vel = mBody.mVel;
			move(&vel);
			break;
		}
		case 1: {
			Vec3f vel;
			vel = mBody.mVel;
			move(&vel);
			waitMotion();
			break;
		}
		}
		pose();
		placePoints();
		if (mStep == 0 && motionCtrlPassed(0.1f, &mAnim) != 0) {
			particleOnceByIdDefault(&mPoints[1], 0, 0x57);
			particleOnceByIdDefault(&mPoints[3], 0, 0x57);
		}
		break;
	case 2:
		mFlags2 &= ~(AM_BITING | AM_LUNGING | AM_GUARDING | AM_TRAIL);
		break;
	}
}

// 0x80074AC4
// Skids to a stop trailing dust while it is fast
void TEneAm08::actSlide(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.5f, &mAnim, 1);
		mFlags2 |= AM_BITING | AM_LUNGING | AM_GUARDING | AM_TRAIL;
		mStep     = 0;
		mFrameKey = 5;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		drag(0.965f, &mBody.mVel);
		Vec3f vel;
		vel.x = mBody.mVel.x;
		vel.y = mBody.mVel.y;
		vel.z = mBody.mVel.z;
		move(&vel);
		f32 speed = flatSpeed();
		if (mStep == 0 && speed <= 0.7f) {
			mStep = 1;
			mFlags2 &= ~(AM_BITING | AM_GUARDING);
			motionCtrlPlayBlend(0.1f, &mAnim, 2);
		}
		if (speed <= 0.1f) {
			mHitFlags |= 0x20;
		}
		pose();
		placePoints();
		if (speed > 1.3f) {
			mFlags2 |= AM_TRAIL;
		} else {
			mFlags2 &= ~AM_TRAIL;
		}
		break;
	}
	case 2:
		motionCtrlSetSpeed(&mAnim, 1.0f);
		clearFrameKey();
		mFlags2 &= ~(AM_BITING | AM_LUNGING | AM_GUARDING | AM_TRAIL);
		break;
	}
}

// 0x80074C54
void TEneAm08::actRecover(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.5f, &mAnim, 3);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		setFrameKey(sAm08FrameKeys[1]);
		drag(0.965f, &mBody.mVel);
		move(&mBody.mVel);
		waitMotion();
		pose();
		placePoints();
		break;
	case 2:
		break;
	}
}

// 0x80074CFC
// The flip keeps the push the hit gave it
void TEneAm08::beginFlip()
{
	mEnemyFlags |= ENEMY_FLIPPING;
	mFrameKey = 3;
	noteFlip();
	mBody.mVel.x = mPush.x;
	mBody.mVel.z = mPush.z;
}

// 0x80074D4C
void TEneAm08::flipped()
{
	motionCtrlUpdate(&mAnim);
	drag(0.7f, &mBody.mVel);
	move(&mBody.mVel);
	waitMotion();
	pose();
	placePoints();
}

// 0x80074DB0
void TEneAm08::endFlip()
{
	mEnemyFlags &= ~ENEMY_FLIPPING;
	clearFrameKey();
}

// 0x80074DDC
void TEneAm08::actFlipLunging(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 5);
		beginFlip();
		mFlags2 |= AM_LUNGING;
		mFrameKey = 3;
		break;
	case 1:
		flipped();
		break;
	case 2:
		endFlip();
		mFlags2 &= ~AM_LUNGING;
		break;
	}
}

// 0x80074E6C
void TEneAm08::actFlip(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 6);
		beginFlip();
		break;
	case 1:
		flipped();
		setFrameKey(sAm08FrameKeys[3]);
		break;
	case 2:
		endFlip();
		break;
	}
}

// 0x80074EF0
void TEneAm08::beginDeath(int* timer)
{
	mEnemyFlags |= ENEMY_INTANGIBLE | ENEMY_BLOCKED;
	colliDisable(mCollider);
	mBar->hide();
	killed(1);
	noteDie();
	mFrameKey = 3;
	*timer    = 60;
}

// 0x80074F5C
// A second after the motion ends it drops its item and goes
void TEneAm08::dying(int* timer)
{
	motionCtrlUpdate(&mAnim);
	if (motionCtrlIsDone(&mAnim) != 0) {
		(*timer)--;
		if (*timer <= 0) {
			enemyFxDeath4A(&mBody.mPos);
			spawnDropB(&mBody.mPos);
			mFlags |= 1;
		}
	}
	drag(0.7f, &mBody.mVel);
	move(&mBody.mVel);
	pose();
	placePoints();
}

// 0x80075000
void TEneAm08::endDeath()
{
	mEnemyFlags &= ~(ENEMY_INTANGIBLE | ENEMY_BLOCKED);
	clearFrameKey();
}

// 0x80075030
void TEneAm08::actDie(int phase)
{
	int* timer = &mStep;
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 7);
		beginDeath(timer);
		break;
	case 1:
		dying(timer);
		setFrameKey(sAm08FrameKeys[4]);
		if (motionCtrlPassed(12.0f, &mAnim) != 0) {
			particleOnceByIdDefault(&mPoints[6], 0, 0x46);
		}
		break;
	case 2:
		endDeath();
		break;
	}
}

// 0x800750E8
// Falls head first dust under the head then the tail
void TEneAm08::actDieLunging(int phase)
{
	int* timer = &mStep;
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 8);
		beginDeath(timer);
		break;
	case 1:
		dying(timer);
		setFrameKey(sAm08FrameKeys[5]);
		if (motionCtrlPassed(10.0f, &mAnim) != 0) {
			Vec3f pos;
			pos.x = mPoints[0].x;
			pos.y = mPoints[0].y;
			pos.z = mPoints[0].z;
			pos.y = mBody.mPos.y;
			particleOnceByIdDefault(&pos, 0, 0x46);
		}
		if (motionCtrlPassed(14.0f, &mAnim) != 0) {
			particleOnceByIdDefault(&mPoints[6], 0, 0x46);
		}
		break;
	case 2:
		endDeath();
		break;
	}
}

// 0x800751E4
void TEneAm08::vfn_3C()
{
	njPrint((kEnemyInfoX << 16) | kEnemyInfoLine, ":HP %4d", mHealth);
}

// 0x80075220
// Hidden while appearing or out of view
void TEneAm08::vfn_10()
{
	if (isAppearing() != 0) {
		mFlags |= 0x10;
	} else if (mLight.isVisible(mDrawTask, 500.0f, &mBody.mPos, 70.0f, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x800752B0
void TEneAm08::render()
{
	njSetTexture(*(void**)sAm08Textures);
	enemyEyeSet(*(NJS_TEXLIST**)sAm08Textures, 0, mFrameKey);
	useArea();
	njPushMatrixEx();
	bodyMatrix();
	drawLit(&mLight, &mTint, mPose);
	njPopMatrixEx();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}
