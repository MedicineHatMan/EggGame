#include "enemy/TEneBat.h"
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
#include "enemy/ObjEnemy.h"
#include "enemy/EnemyFx.h"
#include "Ninja/ninja.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjShadow.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "enemy/EnemyHit.h"
#include "animal/RacerMover.h"
#include "enemy/TEneAm01.h"
struct Vec3f;
struct ColliShape;
struct ObjShadow;

extern "C" {

// The looping sound in mSound start with an id move it stop it

MotionEntry sBatMotions[] = {
	{ MOTION_LOOP, 9, -1, 0, 0.0f, 20.0f, 0.8f, 0.0f, 0 }, { MOTION_LOOP, 5, -1, 0, 0.0f, 20.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 }, { MOTION_LOOP, 7, -1, 0, 0.0f, 20.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 39.0f, 0.4f, 0.0f, 0 }, { MOTION_CHAIN, 0, -1, 6, 0.0f, 19.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 20.0f, 0.8f, 0.0f, 0 }, { MOTION_ONCE, 2, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 }, { MOTION_ONCE, 4, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
};
extern const int sBatBones[] = {
	12,
	60,
	-1,
}; // the bones mParts follow
}

extern const EnemyFrameKey sBatFrameKey0[] = { { 4.0f, 0 }, { -1.0f, 4 } };
extern const EnemyFrameKey sBatFrameKey1[] = { { 4.0f, 4 }, { 30.0f, 0 }, { 34.0f, 4 }, { -1.0f, 0 } };
extern const EnemyFrameKey sBatFrameKey2[] = { { 14.0f, 0 }, { -1.0f, 5 } };
extern const EnemyFrameKey sBatFrameKey3[] = { { 9.0f, 5 }, { -1.0f, 0 } };
extern const EnemyFrameKey sBatFrameKey4[] = { { 9.0f, 5 }, { -1.0f, 0 } };
extern const EnemyFrameKey sBatFrameKey5[] = { { 2.0f, 4 }, { 18.0f, 3 }, { -1.0f, 4 } };
const EnemyFrameKey* sBatFrameKeys[6]      = { sBatFrameKey0, sBatFrameKey1, sBatFrameKey2, sBatFrameKey3, sBatFrameKey4, sBatFrameKey5 };

extern "C" {
extern const ColliShape sBatShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 9.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 6.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

static const char* sClassName = "TEneBat";

// Radians to angle units
static inline int toAngle(f32 x, f32 z)
{
	return (int)(10430.38043493439 * (f32)atan2(x, z));
}

inline void* TEneBat::operator new(u32 size)
{
	return objHeapAlloc(gObjHeap, size);
}

inline void TEneBat::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x800D6A9C
// The motion is stored before the shape
extern "C" void initTEneBat()
{
	resBankLoad(&sBatBank, "ar_ene_bat.arc");
	resBankAddTextures(&sBatBank, 0, "ene_bat.gvm");
	sBatModels   = resBankGetModels(&sBatBank);
	sBatMotion   = resBankGetMotion(&sBatBank, 0);
	sBatTextures = resBankGetTextures(&sBatBank);
}

// 0x800D6B18
extern "C" void shutdownTEneBat()
{
	resBankRemoveTextures(&sBatBank, 0);
	resBankRelease(&sBatBank);
	sBatModels   = 0;
	sBatTextures = 0;
	sBatMotion   = 0;
}

// 0x800D6B60
extern "C" TObject* createTEneBat(TObject* parent, SetDesc* desc)
{
	return new TEneBat(parent, desc);
}

// 0x800D6BB4
TEneBat::TEneBat(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00   = (u32)sClassName;
	mSize = 0x2FC;
	init(desc);
}

// 0x800D6C6C
void TEneBat::init(SetDesc* desc)
{
	motionCtrlInit(&mAnim, sBatMotions, sBatMotion);
	motionCtrlPlay(&mAnim, 0);
	mBody.mRot.x = 0;
	mCollider    = colliCreate(2, 1);
	colliCopyShapes(mCollider, (const ColliShape*)sBatShape);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0x14, 0, 0x10);
	int shape = 0;
	for (int i = 0; i < 2; i++) {
		colliShapeSetTag(mCollider, i, shape);
		shape++;
	}
	mBrain = 0;
	readEnemyDesc(desc);
	mPose     = allocPose(*(void**)sBatModels);
	mModel    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags2   = 0;
	mKnock.x  = 0.0f;
	mKnock.y  = 0.0f;
	mKnock.z  = 0.0f;
	mFrameKey = 0;
	for (int i = 0; i < 2; i++) {
		mParts[i].x = 0.0f;
		mParts[i].y = 0.0f;
		mParts[i].z = 0.0f;
	}
	mTarget.x = 0.0f;
	mTarget.y = 0.0f;
	mTarget.z = 0.0f;
	mAim.x    = 0.0f;
	mAim.y    = 0.0f;
	mAim.z    = 0.0f;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 18.0f;
	mShadow._34 = 18.0f;
	shadowEnable(&mShadow);
	mBar = new TEnemyLifeBar(this);
	mBar->setMax((s16)mHealth);
	mBar->setMark(0);
	mBar->setValue((s16)mHealth);
	mBar->setSize(40.0f, 20.0f);
	mLight.reset();
	mLight.setFadeDist(-15.0f);
	mTint.reset();
	enemyEffectSetLarge(15.0f, &mEffect, &mBody.mPos);
	mStep   = 0;
	mTimer  = 0;
	mAngle  = 0;
	mHeight = 0.0f;
	mRise   = 1.0f;
	createBrain();
}

// 0x800D6EB0
TEneBat::~TEneBat()
{
	seLoopStop(&mSound);
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

// 0x800D6FC4
void TEneBat::setPos(const Vec3f* pos)
{
	f32 x        = pos->x;
	f32 y        = pos->y;
	mBody.mPos.x = x;
	f32 z        = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x800D7008
f32 TEneBat::vfn_40()
{
	return 60.0f;
}

// 0x800D7010
// The shapes sit on the two posed points
void TEneBat::syncCollider()
{
	Vec3f* part = (Vec3f*)colliGetShapes(mCollider);
	part[4]     = mParts[0];
	part[0]     = mParts[1];
	colliRefreshBounds(mCollider);
	colliBoundsOn(mCollider);
}

// 0x800D7080
// Health defaults to 200
void TEneBat::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	int health = desc->getInt(0);
	mHealth    = health <= 0 ? 200 : health;
}

// 0x800D70DC
// Hurts only while armed An egg takes 1 a player 20
int TEneBat::onHit(const TObjAction* other, int shape, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit       = 0;
	if ((mFlags2 & BAT_ARMED) != 0) {
		if (isPlayerOrKind4(other) != 0) {
			info->mDamage = 20;
			hit           = 1;
			info->mFlags.clear();
			mFlags2 |= BAT_ARMED;
		} else if (isEggObj(other) != 0) {
			info->mDamage = 1;
			hit           = 1;
			info->mFlags.clear();
			info->mFlags.setBit(0xD);
		}
	}
	return hit;
}

// 0x800D7184
void TEneBat::setTarget(const Vec3f* target)
{
	mTarget = *target;
}

// 0x800D71A0
void TEneBat::setAim(const Vec3f* pos, int angle)
{
	mAim      = *pos;
	mAimAngle = angle;
}

// 0x800D71C0
void TEneBat::noteDeath()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
}

// 0x800D7200
int TEneBat::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

// 0x800D7224
int TEneBat::hasArrived()
{
	return mFlags2 & BAT_ARRIVED;
}

// 0x800D7230
int TEneBat::isRiseDone()
{
	return isActionDone(ACT_RISE);
}

// 0x800D7254
int TEneBat::isDiveDone()
{
	return isActionDone(ACT_DIVE);
}

// 0x800D7278
// A hard hit flips it only low in a dive
int TEneBat::canFlip()
{
	u8 flip = 0;
	if (mAction == ACT_DIVE && (mFlags2 & BAT_LOW) != 0) {
		flip = 1;
	}
	return flip;
}

// 0x800D72A0
int TEneBat::isStunDone()
{
	return isActionDone(ACT_STUN);
}

// 0x800D72C4
int TEneBat::isFlipDone()
{
	return isActionDone(ACT_FLIP);
}

// 0x800D72E8
void TEneBat::noteStun()
{
	noteDeath();
	seStartAt(0xFE, &mBody.mPos);
}

// 0x800D7320
void TEneBat::noteDie()
{
	noteDeath();
	seStartAt(0xFF, &mBody.mPos);
}

// 0x800D7358
// The brain asks which death to use
int TEneBat::deathAction()
{
	return ACT_DIE;
}

// 0x800D7360
void TEneBat::stopEffect()
{
	enemyEffectStart(&mEffect);
}

// 0x800D7384
void TEneBat::updateBody()
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
	syncCollider();
	updateSound();
	mBar->setPos(&mBody.mPos);
	mBar->setValue((s16)mHealth);
	enemyEffectUpdate(&mEffect);
}

// 0x800D74CC
// returnZero is 0 so this never runs
void TEneBat::vfn_6C()
{
	if (returnZero() != 0) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginBones();
		drawFirstFrame(sBatMotion, 10, mPose);
		endBones();
		placeParts();
		syncCollider();
	}
}

// 0x800D7548
void TEneBat::createBrain()
{
	mBrain = BatHuntBrain::create(this);
}

// 0x800D7578
void TEneBat::beginBones()
{
	nodeTrackBeginOrigins(sBatBones, mParts);
}

// 0x800D75A8
void TEneBat::endBones()
{
	nodeTrackEnd();
}

// 0x800D75C8
void TEneBat::updatePose()
{
	beginBones();
	applyPoseFaded(mPose, 40.0f, &mAnim, &mAppearTimer);
	endBones();
}

// 0x800D7614
// Moves the posed points into the world
void TEneBat::placeParts()
{
	gjPushUnitMatrix();
	applyTransform();
	for (int i = 0; i < 2; i++) {
		njCalcPointEx(&mParts[i], &mParts[i]);
	}
	njPopMatrixEx();
}

// 0x800D7674
int TEneBat::getHitKind(const void*, HitInfo*)
{
	return 3;
}

// 0x800D767C
// Only a kind-3 hit carries its damage
int TEneBat::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x800D7694
void TEneBat::applyTransform()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
}

// 0x800D76CC
// The step from from to to held to limit either way
int TEneBat::clampStep(f32 from, f32 to, f32 limit, f32* out)
{
	f32 diff = to - from;
	if (diff >= 0.0f) {
		if (diff < limit) {
			*out = diff;
			return 1;
		}
	} else {
		limit *= -1.0f;
		if (diff > limit) {
			*out = diff;
			return 1;
		}
	}
	*out = limit;
	return 0;
}

// 0x800D771C
// Moves flat by vel and the push stopping at walls
int TEneBat::move(Vec3f* vel)
{
	Vec3f dir;
	getFlatPush(&dir);
	njAddVector(&dir, vel);
	dir.y   = 0.0f;
	f32 len = njUnitVector(&dir);
	if (len < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 dist     = len;
	int hit      = probeWall(9.0f, &mBody.mPos, &dir, &dist, 0, 0);
	mBody.mPos.x = dir.x * dist + mBody.mPos.x;
	mBody.mPos.z = dir.z * dist + mBody.mPos.z;
	return hit;
}

// 0x800D77E0
int TEneBat::drag(f32 rate)
{
	mBody.mVel.x *= rate;
	mBody.mVel.z *= rate;
	return move(&mBody.mVel);
}

// 0x800D781C
void TEneBat::hover(f32 rate)
{
	drag(rate);
	mBody.mVel.y *= rate;
	followGround(5.0f, 17.0f, &mBody.mPos, &mBody.mVel.y);
}

// 0x800D7880
// Moves pos up by vy keeping below a ceiling and down above the ground
u8 TEneBat::followGround(f32 up, f32 down, Vec3f* pos, f32* vy)
{
	Vec3f top;
	Vec3f hit;
	Vec3f normal;
	f32 lift;
	f32 rise;
	top = *pos;
	top.y += up;
	top.y += *vy;
	int ceiling = probeGroundLine(pos, &top, &hit, 0);
	if (ceiling != 0) {
		lift = *vy + (hit.y - top.y);
	}
	mFlags2 &= ~BAT_LOW;
	int ground = 0;
	if (probeGround(pos, &hit, &normal) != 0) {
		f32 base = pos->y;
		base -= down;
		base += *vy;
		f32 gap = hit.y - base;
		if (gap > 0.0f) {
			rise   = gap + *vy;
			ground = 1;
		} else if (gap > -2.5f) {
			mFlags2 |= BAT_LOW;
		}
		shadowEnable(&mShadow);
		shadowSetGround(&mShadow, &hit, &normal);
	} else {
		shadowDisable(&mShadow);
	}
	if (ceiling != 0) {
		if (ground != 0) {
			pos->y = 0.5f * (lift + rise) + pos->y;
			*vy    = 0.0f;
		} else {
			pos->y = pos->y + lift;
			*vy    = 0.0f;
		}
	} else if (ground != 0) {
		pos->y = pos->y + rise;
		*vy    = 0.0f;
	} else {
		pos->y = pos->y + *vy;
	}
	return (ceiling != 0 || ground != 0);
}

// 0x800D7A68
// Makes both shapes hurt
void TEneBat::arm()
{
	mFlags2 |= BAT_ARMED;
	colliShapeSetBit8(mCollider, 0);
	colliShapeSetBit8(mCollider, 1);
}

// 0x800D7AB4
void TEneBat::disarm()
{
	mFlags2 &= ~BAT_ARMED;
	colliShapeClearBit8(mCollider, 0);
	colliShapeClearBit8(mCollider, 1);
}

// 0x800D7B00
// Starts moves or stops the flapping as BAT_FLAP asks
void TEneBat::updateSound()
{
	u32 flags = mFlags2;
	if ((flags & BAT_SOUND) != 0) {
		if ((flags & BAT_FLAP) != 0) {
			seLoopMove(&mSound, &mBody.mPos);
		} else {
			seLoopStop(&mSound);
			mFlags2 &= ~BAT_SOUND;
		}
	} else if ((flags & BAT_FLAP) != 0) {
		seLoopStart(&mSound, 0xFD, &mBody.mPos);
		mFlags2 |= BAT_SOUND;
	}
}

// 0x800D7B88
void TEneBat::startFlap()
{
	mFlags2 |= BAT_FLAP;
}

// 0x800D7B98
void TEneBat::stopFlap()
{
	mFlags2 &= ~BAT_FLAP;
}

// 0x800D7BA8
void TEneBat::onActionPhase(int action, int phase)
{
	switch (action) {
	case ACT_APPEAR:
		actAppear(phase);
		break;
	case ACT_HOVER:
		actHover(phase);
		break;
	case ACT_FLY:
		actFly(phase);
		break;
	case ACT_RISE:
		actRise(phase);
		break;
	case ACT_AIM:
		actAim(phase);
		break;
	case ACT_DIVE:
		actDive(phase);
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
	}
}

// 0x800D7C4C
// Rises out of the ground drawn 17 up then moves up to match
void TEneBat::actAppear(int phase)
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
		beginBones();
		gjPushUnitMatrix();
		njTranslate(0, 0.0f, 17.0f, 0.0f);
		applyPose(mPose, &mAnim);
		njPopMatrixEx();
		if (hatchActive(&mAppearTimer) != 0) {
			hatchPose(&mAppearTimer, (GjActor*)mPose, 40.0f);
		}
		endBones();
		placeParts();
		break;
	case 2:
		endAppear(step);
		mBody.mPos.y += 17.0f;
		clearFrameKey();
		break;
	}
}

// 0x800D7D5C
void TEneBat::actHover(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0);
		clearFrameKey();
		startFlap();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		hover(0.8f);
		placeParts();
		break;
	case 2:
		stopFlap();
		break;
	}
}

// 0x800D7DF8
// Turns toward mTarget then flies there
void TEneBat::actFly(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 1);
		mFlags2 &= ~BAT_ARRIVED;
		clearFrameKey();
		startFlap();
		mStep = 0;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		updatePose();
		Vec3f to;
		Vec3f dir;
		int diff;
		f32 vy;
		to = mTarget;
		njSubVector(&to, &mBody.mPos);
		mBody.mRot.y = angleApproachDiff(mBody.mRot.y, toAngle(to.x, to.z), 0x180, &diff);
		if (diff <= 0x600) {
			mStep = 1;
		}
		dir      = to;
		dir.y    = 0.0f;
		f32 dist = njUnitVector(&dir);
		if (dist < gUnitVectorMin) {
			dir = gVecZero;
		}
		f32 speed = vecLenXZ(&mBody.mVel);
		if (mStep == 1) {
			speed = approachSpeed(speed, dist, 0.05f, 0.03f, 0.06f, 1.0f, 0.01f);
		} else {
			speed *= 0.8f;
		}
		gjScaleVector(speed, &dir, &dir);
		move(&dir);
		int close    = dist < 1.0f;
		mBody.mVel.x = dir.x;
		mBody.mVel.z = dir.z;
		int level;
		if (mStep == 1) {
			level = clampStep(mBody.mPos.y, mTarget.y, 0.3f, &vy);
		} else {
			vy = mBody.mVel.y;
			vy *= 0.8f;
		}
		followGround(5.0f, 17.0f, &mBody.mPos, &vy);
		mBody.mVel.y = vy;
		if (level != 0 && close != 0) {
			mFlags2 |= BAT_ARRIVED;
		} else {
			mFlags2 &= ~BAT_ARRIVED;
		}
		placeParts();
		break;
	}
	case 2:
		mFlags2 &= ~BAT_ARRIVED;
		stopFlap();
		break;
	}
}

// 0x800D807C
// Flaps up over the player while spinning then turns to face them
void TEneBat::actRise(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 2);
		clearFrameKey();
		startFlap();
		mStep   = 0;
		mHeight = 0.0f;
		mRise   = 1.8f;
		mTimer  = 0;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		updatePose();
		s16 motion = motionCtrlCurrent(&mAnim);
		if (motion == 2) {
			setFrameKey(sBatFrameKeys[0]);
		} else if (motion == 4) {
			setFrameKey(sBatFrameKeys[1]);
		} else {
			mFrameKey = 4;
		}
		drag(0.8f);
		switch (mStep) {
		case 0:
			mBody.mVel.y *= 0.8f;
			if (motionCtrlFrame(&mAnim) > 16.0f) {
				f32 y = mParts[0].y;
				f32 dy;
				dy = y - mHeight;
				if (dy < mRise) {
					dy = mRise - dy;
				} else {
					mRise = dy;
					dy    = 0.0f;
				}
				mHeight = y;
				dy      = dy + mBody.mVel.y;
				followGround(5.0f, 17.0f, &mBody.mPos, &dy);
				mBody.mVel.y *= 0.8f;
				mBody.mRot.y += 0x1900;
				mTimer = 1;
			} else {
				mHeight = mParts[0].y;
				followGround(5.0f, 17.0f, &mBody.mPos, &mBody.mVel.y);
				turnTo(mAimAngle, 0x200);
			}
			if (motionCtrlIsDone(&mAnim) != 0) {
				mBody.mVel.y = mRise;
				mHeight      = mAim.y;
				mRise        = 0.0f;
				mStep        = 1;
				motionCtrlPlay(&mAnim, 3);
			}
			if (motionCtrlPassed(14.0f, &mAnim) != 0) {
				seStartAt(0xFB, &mBody.mPos);
				stopFlap();
			}
			break;
		case 1:
		case 2: {
			f32 diff = mHeight;
			diff += 67.0f;
			diff -= mBody.mPos.y;
			f32 vy = mBody.mVel.y;
			vy     = approachSpeed(vy, diff, 0.15f, 0.2f, 0.2f, 10.0f, 0.01f);
			followGround(5.0f, 17.0f, &mBody.mPos, &vy);
			if (mStep == 1) {
				if (vy < mBody.mVel.y) {
					mStep  = 2;
					mTimer = 0;
					motionCtrlPlay(&mAnim, 4);
					startFlap();
				}
				mBody.mRot.y += 0x1900;
			} else {
				if (mRise <= 0.0f) {
					int diff = angleDiff(mBody.mRot.y, mAimAngle);
					if (diff >= 0 && diff < 0x1900) {
						mBody.mRot.y = mAimAngle;
						mRise        = 1.0f;
					} else {
						mBody.mRot.y += 0x1900;
					}
				} else {
					turnTo(mAimAngle, 0x200);
				}
				waitMotion();
			}
			mBody.mVel.y = vy;
			break;
		}
		}
		placeParts();
		break;
	}
	case 2:
		mFlags2 &= ~BAT_ARRIVED;
		clearFrameKey();
		stopFlap();
		break;
	}
}

// 0x800D8410
// Hangs 67 over the player facing them
void TEneBat::actAim(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0);
		mFrameKey = 5;
		startFlap();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		turnTo(mAimAngle, 0x180);
		drag(0.8f);
		clampStep(mBody.mPos.y, 17.0f + (50.0f + mAim.y), 0.3f, &mBody.mVel.y);
		followGround(5.0f, 17.0f, &mBody.mPos, &mBody.mVel.y);
		placeParts();
		break;
	case 2:
		clearFrameKey();
		stopFlap();
		break;
	}
}

// 0x800D8500
// mStep 0 the wind-up 1 the dive with a trail 2 pulling up
void TEneBat::actDive(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 5);
		mFlags2 &= ~BAT_HIT;
		disarm();
		fxHolderStop(&mFx);
		mStep   = 0;
		mTimer  = 480;
		mHeight = 0.0f;
		break;
	case 1: {
		switch (mStep) {
		case 0:
			motionCtrlUpdate(&mAnim);
			updatePose();
			hover(0.8f);
			turnTo(mAimAngle, 0x180);
			if (motionCtrlFrame(&mAnim) >= 12.0f) {
				mStep = 1;
				arm();
				mBody.mVel.y = -1.7f;
				mHeight      = mAim.y;
				seStartAt(0xFC, &mBody.mPos);
			}
			break;
		case 1: {
			motionCtrlUpdate(&mAnim);
			updatePose();
			turnTo(mAimAngle, 0x40);
			f32 sin;
			f32 cos;
			njSinCos(mBody.mRot.y, &sin, &cos);
			f32 dx       = mAim.x - mBody.mPos.x;
			f32 dz       = mAim.z - mBody.mPos.z;
			mBody.mVel.x = 3.0f * sin;
			mBody.mVel.z = 3.0f * cos;
			int wall     = move(&mBody.mVel);
			f32 dy       = 13.0f + mHeight - mBody.mPos.y;
			if (dy > 0.0f) {
				dy *= 0.2f;
			}
			mBody.mVel.y = approachSpeed(mBody.mVel.y, dy, 0.08f, 0.1f, 0.005f, 10.0f, 0.01f);
			followGround(5.0f, 13.0f, &mBody.mPos, &mBody.mVel.y);
			mTimer--;
			if (wall != 0 || mTimer <= 0 || ((mFlags2 & BAT_LOW) != 0 && (dx * sin + dz * cos <= 0.0f || (mFlags2 & BAT_HIT) != 0))) {
				motionCtrlPlay(&mAnim, 7);
				disarm();
				mStep   = 2;
				mHeight = 7.0f + (17.0f + mBody.mPos.y);
				fxHolderStop(&mFx);
			} else {
				Vec3f trail;
				trail   = mBody.mPos;
				trail.x = 40.0f * sin + trail.x;
				trail.z = 40.0f * cos + trail.z;
				mFx.start(5, 10, &trail, 0, mBody.mRot.y);
			}
			break;
		}
		case 2: {
			motionCtrlUpdate(&mAnim);
			f32 rate;
			if (motionCtrlFrame(&mAnim) >= 10.0f) {
				rate = 0.92f;
				if (vecLenXZ(&mBody.mVel) >= 1.2f) {
					motionCtrlSetFrame(10.0f, &mAnim);
				} else {
					startFlap();
				}
			} else {
				rate = 0.98f;
				updatePose();
			}
			updatePose();
			if (drag(rate) != 0) {
				mBody.mVel.x = 0.0f;
				mBody.mVel.z = 0.0f;
			}
			clampStep(mBody.mPos.y, mHeight, 0.12f, &mBody.mVel.y);
			followGround(5.0f, 13.0f, &mBody.mPos, &mBody.mVel.y);
			waitMotion();
			break;
		}
		}
		s16 motion = motionCtrlCurrent(&mAnim);
		if (motion == 5) {
			setFrameKey(sBatFrameKeys[2]);
		} else if (motion == 7) {
			setFrameKey(sBatFrameKeys[3]);
		} else {
			mFrameKey = 5;
		}
		placeParts();
		break;
	}
	case 2:
		disarm();
		clearFrameKey();
		stopFlap();
		fxHolderStop(&mFx);
		break;
	}
}

// 0x800D894C
// Knocked back a little
void TEneBat::actStun(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 8);
		mEnemyFlags |= ENEMY_FLIPPING;
		mFrameKey    = 3;
		mBody.mVel.x = 0.4f * mKnock.x;
		mBody.mVel.z = 0.4f * mKnock.z;
		noteStun();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		hover(0.95f);
		placeParts();
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		clearFrameKey();
		break;
	}
}

// 0x800D8A20
// Knocked down then back up
void TEneBat::actFlip(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 9);
		mEnemyFlags |= ENEMY_FLIPPING;
		clearFrameKey();
		mBody.mVel.x = mKnock.x;
		mBody.mVel.z = mKnock.z;
		mBody.mVel.y = -2.0f;
		noteStun();
		mStep   = 0;
		mHeight = 0.0f;
		mAngle  = toAngle(mKnock.x, mKnock.z);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		waitMotion();
		setFrameKey(sBatFrameKeys[4]);
		switch (mStep) {
		case 0:
			turnTo(mAngle, 0x280);
			if ((mFlags2 & BAT_LOW) != 0) {
				mStep = 1;
			} else if (motionCtrlFrame(&mAnim) >= 1.0f) {
				motionCtrlSetFrame(1.0f, &mAnim);
			}
			updatePose();
			drag(0.9f);
			break;
		case 1:
			turnTo(mAngle, 0x280);
			if (motionCtrlFrame(&mAnim) >= 13.0f) {
				mStep   = 2;
				mHeight = 7.0f + (4.0f + mBody.mPos.y);
			}
			updatePose();
			drag(0.7f);
			break;
		case 2:
			clampStep(mBody.mPos.y, mHeight, 0.12f, &mBody.mVel.y);
			updatePose();
			drag(0.9f);
			break;
		}
		followGround(5.0f, 13.0f, &mBody.mPos, &mBody.mVel.y);
		placeParts();
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		clearFrameKey();
		break;
	}
}

// 0x800D8C58
// Lets go of four TEneAm01 and waits
void TEneBat::actDie(int phase)
{
	switch (phase) {
	case 0:
		mEnemyFlags |= ENEMY_BLOCKED;
		colliDisable(mCollider);
		mBar->hide();
		noteDie();
		mList.spawn(&mBody.mPos, 4);
		mEnemyFlags |= ENEMY_APPEARING;
		shadowDisable(&mShadow);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (mList.isEmpty() != 0) {
			killed(1);
			mFlags |= 1;
		} else if (mList.hasAny() == 0) {
			mFlags |= 1;
		}
		break;
	case 2:
		mEnemyFlags &= ~(ENEMY_BLOCKED | ENEMY_INTANGIBLE);
		clearFrameKey();
		break;
	}
}

// 0x800D8D58
void TEneBat::vfn_3C()
{
	int x    = kEnemyInfoX << 16;
	int line = kEnemyInfoLine;
	njPrint(x | line, ":HP %4d", mHealth);
}

// 0x800D8D94
void TEneBat::vfn_10()
{
	if (isAppearing() != 0) {
		mFlags |= 0x10;
	} else if (mLight.isVisible(mModel, 500.0f, &mBody.mPos, 60.0f, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x800D8E24
void TEneBat::render()
{
	njSetTexture(*(void**)sBatTextures);
	enemyEyeSet(*(NJS_TEXLIST**)sBatTextures, 0, mFrameKey);
	useArea();
	njPushMatrixEx();
	applyTransform();
	drawLit(&mLight, &mTint, mPose);
	njPopMatrixEx();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}
