#include "enemy/TEneFrog.h"
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
#include "egg/EggCreature.h"
#include "animal/AniMount.h"
#include "enemy/boss/EneBoss.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "engine/Colli.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "enemy/EnemyFx.h"
#include "engine/TParticle.h"
#include "enemy/TEneAm01.h"
struct Vec3f;
struct ColliShape;
struct GjActor;
struct ObjShadow;

extern "C" {

MotionEntry sFrogMotions[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 22.0f, 0.3f, 0.0f, 0 },  { MOTION_LOOP, 1, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },  { MOTION_ONCE, 3, -1, 0, 0.0f, 24.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 22.0f, 0.4f, 0.0f, 0 },  { MOTION_ONCE, 5, -1, 0, 0.0f, 39.0f, 0.3f, 0.0f, 0 },
	{ MOTION_BLEND, 7, -1, 7, 0.0f, 9.0f, 0.5f, 0.2f, 0 },  { MOTION_ONCE, 6, -1, 0, 0.0f, 14.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 13.0f, 29.0f, 0.5f, 0.0f, 0 }, { MOTION_LOOP, 8, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
};
MotionEntry sFrogTongueMotions[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 29.0f, 1.5f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 30.0f, 1.5f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 29.0f, 1.5f, 0.0f, 0 },
};
extern const int sFrogBones[] = {
	1, 8, 19, 44, 46, -1,
}; // the bones mParts follow
extern const int sFrogTongueBones[] = {
	4, 6, 8, 10, 12, 14, 16, 18, -1,
}; // the bones mTongue follows
const char* gTEneFrogName                   = "TEneFrog";
extern const EnemyFrameKey sFrogFrameKey0[] = { { 8.0f, 0 }, { -1.0f, 4 } };
extern const EnemyFrameKey sFrogFrameKey1[] = { { 10.0f, 4 }, { -1.0f, 3 } };
extern const EnemyFrameKey sFrogFrameKey2[] = { { 1.0f, 3 }, { 11.0f, 0 }, { 25.0f, 4 }, { -1.0f, 0 } };
extern const EnemyFrameKey sFrogFrameKey3[] = { { 1.0f, 0 }, { 5.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sFrogFrameKey4[] = { { 1.0f, 0 }, { 6.0f, 3 }, { -1.0f, 2 } };
const EnemyFrameKey* sFrogFrameKeys[5]      = { sFrogFrameKey0, sFrogFrameKey1, sFrogFrameKey2, sFrogFrameKey3, sFrogFrameKey4 };
extern const ColliShape sFrogShape[]        = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 12.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
}

// Radians to angle units
static inline int toAngle(f32 x, f32 z)
{
	return (int)(10430.38043493439 * (f32)atan2(x, z));
}

inline void TEneFrog::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x800EED54
extern "C" void initTEneFrog()
{
	resBankLoad(&sFrogBank, "ar_ene_frog.arc");
	resBankAddTextures(&sFrogBank, 0, "ene_frog.gvm");
	sFrogModels       = resBankGetModels(&sFrogBank);
	sFrogTextures     = resBankGetTextures(&sFrogBank);
	sFrogMotion       = resBankGetMotion(&sFrogBank, 0);
	sFrogTongueMotion = resBankGetMotion(&sFrogBank, 1);
	gFrogParams       = (int*)enemyParamGet(10);
}

// 0x800EEDF0
extern "C" void shutdownTEneFrog()
{
	gFrogParams = 0;
	resBankRemoveTextures(&sFrogBank, 0);
	resBankRelease(&sFrogBank);
	sFrogModels       = 0;
	sFrogTextures     = 0;
	sFrogMotion       = 0;
	sFrogTongueMotion = 0;
}

// 0x800EEE44
extern "C" TObject* createTEneFrog(TObject* parent, SetDesc* desc)
{
	return new TEneFrog(parent, desc);
}

// 0x800EEE98
TEneFrog::TEneFrog(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00   = (u32)gTEneFrogName;
	mSize = 0x38C;
	init(desc);
}

// 0x800EEF5C
// The body has one shape
void TEneFrog::init(SetDesc* desc)
{
	motionCtrlInit(&mAnim, sFrogMotions, sFrogMotion);
	motionCtrlPlay(&mAnim, 9);
	mBody.mRot.x = 0;
	mCollider    = colliCreate(1, 1);
	colliCopyShapes(mCollider, sFrogShape);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0x14, 0, 0x10);
	colliShapeSetTag(mCollider, 0, 0);
	mTongueCollider = colliCreate(1, 1);
	colliShapeMakeBox(colliGetShapes(mTongueCollider), &mBody.mPos, 0, 10.0f, 10.0f, 10.0f, 0);
	colliSetOwner(mTongueCollider, (u16)mObjId);
	colliSetMasks(mTongueCollider, 2, 0x14, 0, 0x10);
	colliShapeSetTag(mTongueCollider, 0, 1);
	colliShapeSetBit8(mTongueCollider, 0);
	mBrain = 0;
	readEnemyDesc(desc);
	mPose     = (GjActor*)(allocPose(*(void**)sFrogModels));
	mModel    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags2   = 0;
	mKnock.x  = 0.0f;
	mKnock.y  = 0.0f;
	mKnock.z  = 0.0f;
	mFrameKey = 0;
	for (int i = 0; i < 5; i++) {
		mParts[i].x = 0.0f;
		mParts[i].y = 0.0f;
		mParts[i].z = 0.0f;
	}
	mAim.x    = 0.0f;
	mAim.y    = 0.0f;
	mAim.z    = 0.0f;
	mAimAngle = 0;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 24.0f;
	mShadow._34 = 24.0f;
	shadowEnable(&mShadow);
	mBar = new TEnemyLifeBar(this);
	mBar->setMax((s16)mHealth);
	mBar->setMark(0);
	mBar->setValue((s16)mHealth);
	mBar->setSize(40.0f, 20.0f);
	mLight.reset();
	mLight.setFadeDist(-15.0f);
	mTint.reset();
	enemyEffectSetLarge(33.0f, &mEffect, &mBody.mPos);
	motionCtrlSetPlayer(&mTongueAnim, &mTongueMotion);
	motionCtrlInit(&mTongueAnim, sFrogTongueMotions, sFrogTongueMotion);
	motionCtrlPlay(&mTongueAnim, 0);
	mTonguePose  = (GjActor*)(allocPose(sFrogModels[1]));
	mTonguePitch = 0;
	mTongueYaw   = 0;
	for (int i = 0; i < 8; i++) {
		mTongue[i].x = 0.0f;
		mTongue[i].y = 0.0f;
		mTongue[i].z = 0.0f;
		mTongueFx[i] = 0;
	}
	mStep     = 0;
	mSubStep  = 0;
	mHopAngle = 0;
	mBestDist = 0.0f;
	createBrain();
}

// 0x800EF274
TEneFrog::~TEneFrog()
{
	stopTongueFx();
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	freePose((void**)&mTonguePose);
	if (mBar != 0) {
		mBar->mFlags |= 1;
		mBar = 0;
	}
	removeDrawTask(mModel);
	freePose((void**)&mPose);
	colliRetire(mTongueCollider);
	mTongueCollider = 0;
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x800EF388
void TEneFrog::setPos(const Vec3f* pos)
{
	f32 x        = pos->x;
	f32 y        = pos->y;
	mBody.mPos.x = x;
	f32 z        = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x800EF3CC
f32 TEneFrog::vfn_40()
{
	return 45.0f;
}

// 0x800EF3D4
// The body's shape sits on the body's point
void TEneFrog::syncCollider()
{
	Vec3f* body = (Vec3f*)colliGetShapes(mCollider);
	*body       = mParts[0];
	if ((mFlags2 & FROG_TONGUE_HURT) != 0) {
		colliEnable(mTongueCollider);
		ColliShape* box = colliGetShapes(mTongueCollider);
		Vec3f root;
		root.x = mTongue[0].x;
		root.y = mTongue[0].y;
		root.z = mTongue[0].z;
		Vec3f tip;
		tip.x = mTongue[7].x;
		tip.y = mTongue[7].y;
		tip.z = mTongue[7].z;
		Vec3f mid;
		f32 half = 0.5f * (root.y - mBody.mPos.y);
		mid.x    = root.x;
		mid.y    = root.y;
		mid.z    = root.z;
		njAddVector(&mid, &tip);
		gjScaleVector(0.5f, &mid, &mid);
		mid.y = mBody.mPos.y + half;
		njSubVector(&tip, &root);
		f32 len = vecLenXZ(&tip);
		colliShapeSetPos(box, &mid);
		colliShapeSetBoxX((ColliShape*)box, 5.0f);
		colliShapeSetBoxY((ColliShape*)box, half);
		colliShapeSetBoxZ((ColliShape*)box, 0.5f * len);
		colliShapeSetBoxRot((ColliShape*)box, toAngle(tip.x, tip.z));
		colliRefreshBounds(mTongueCollider);
	} else {
		colliDisable(mTongueCollider);
	}
}

// 0x800EF55C
// Health defaults to 200
void TEneFrog::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	int health = desc->getInt(0);
	mHealth    = health <= 0 ? 200 : health;
}

// 0x800EF5B8
// Only the tongue hurts An egg takes 1
int TEneFrog::onHit(const TObjAction* other, int shape, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit       = 0;
	if ((mFlags2 & FROG_TONGUE_HURT) != 0 && shape == 1) {
		if (isPlayerOrKind4(other) != 0) {
			f32 s;
			f32 c;
			info->mDamage = 20;
			hit           = 1;
			info->mFlags.clear();
			njSinCos(mTongueYaw, &s, &c);
			info->mFlags.setBit(0xC);
			if (angleDiff(mTongueYawPrev, mTongueYaw) >= 0) {
				info->mPos.x = 2.0f * c;
				info->mPos.y = 2.0f;
				info->mPos.z = -2.0f * s;
			} else {
				info->mPos.x = -2.0f * c;
				info->mPos.y = 2.0f;
				info->mPos.z = 2.0f * s;
			}
			gjScaleVector(60.0f, &info->mPos, &info->mPos);
		} else if (isEggObj(other) != 0) {
			info->mDamage = 1;
			hit           = 1;
			info->mFlags.clear();
			info->mFlags.setBit(0xD);
		}
	}
	return hit;
}

// 0x800EF6E8
void TEneFrog::setTarget(const Vec3f* target)
{
	mTarget = *target;
}

// 0x800EF704
void TEneFrog::setAim(const Vec3f* pos, int angle)
{
	mAim      = *pos;
	mAimAngle = angle;
}

// 0x800EF724
void TEneFrog::noteHurt()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
}

// 0x800EF764
int TEneFrog::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

// 0x800EF788
int TEneFrog::hasArrived()
{
	return mFlags2 & FROG_ARRIVED;
}

// 0x800EF794
int TEneFrog::isGrounded()
{
	return isAirborne() == 0;
}

// 0x800EF7C8
int TEneFrog::isRetractDone()
{
	return isActionDone(ACT_RETRACT);
}

// 0x800EF7EC
// Stunned with the tongue out if it was out and hurting
int TEneFrog::stunAction()
{
	return ((mFlags2 >> 3) & 1) + ACT_STUN;
}

// 0x800EF7FC
int TEneFrog::isStunDone()
{
	u8 done = 0;
	if (isActionDone(ACT_STUN) != 0 || isActionDone(ACT_STUN_LICK) != 0) {
		done = 1;
	}
	return done;
}

// 0x800EF854
void TEneFrog::noteStun()
{
	noteHurt();
	seStartAt(0x10B, &mBody.mPos);
}

// 0x800EF88C
void TEneFrog::noteDie()
{
	noteHurt();
	seStartAt(0x10C, &mBody.mPos);
}

// 0x800EF8C4
// The brain asks which death to use
int TEneFrog::deathAction()
{
	return ACT_DIE;
}

// 0x800EF8CC
void TEneFrog::stopEffect()
{
	enemyEffectStart(&mEffect);
}

// 0x800EF8F0
void TEneFrog::updateBody()
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
	if ((mFlags2 & FROG_TONGUE_FX) != 0) {
		keepTongueFx();
	} else if (mTongueFx[0] != 0) {
		stopTongueFx();
	}
	syncCollider();
	mBar->setPos(&mBody.mPos);
	mBar->setValue((s16)mHealth);
	enemyEffectUpdate(&mEffect);
}

// 0x800EFA64
// returnZero is 0 so this never runs
void TEneFrog::vfn_6C()
{
	if (returnZero() != 0) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginBones();
		drawFirstFrame(sFrogMotion, 9, mPose);
		endBones();
		placeParts();
		syncCollider();
	}
}

// 0x800EFAE0
void TEneFrog::createBrain()
{
	mBrain = FrogHuntBrain::create(this);
}

// 0x800EFB10
void TEneFrog::beginBones()
{
	nodeTrackBeginOrigins(sFrogBones, mParts);
}

// 0x800EFB40
void TEneFrog::endBones()
{
	nodeTrackEnd();
}

// 0x800EFB60
void TEneFrog::updatePose()
{
	beginBones();
	applyPoseFaded(mPose, 25.0f, &mAnim, &mAppearTimer);
	endBones();
}

// 0x800EFBAC
// Moves the posed points into the world
void TEneFrog::placeParts()
{
	gjPushUnitMatrix();
	applyTransform();
	for (int i = 0; i < 5; i++) {
		njCalcPointEx(&mParts[i], &mParts[i]);
	}
	njPopMatrixEx();
	Vec3f dir;
	dir.x = mParts[4].x;
	dir.y = mParts[4].y;
	dir.z = mParts[4].z;
	njSubVector(&dir, &mParts[3]);
	mTongueYawPrev = mTongueYaw;
	vectorToAngles(&dir, &mTonguePitch, &mTongueYaw);
	gjPushUnitMatrix();
	applyTongueTransform();
	for (int i = 0; i < 8; i++) {
		njCalcPointEx(&mTongue[i], &mTongue[i]);
	}
	njPopMatrixEx();
}

// 0x800EFC80
void TEneFrog::beginTongueBones()
{
	nodeTrackBeginOrigins(sFrogTongueBones, mTongue);
}

// 0x800EFCB0
void TEneFrog::endTongueBones()
{
	nodeTrackEnd();
}

// 0x800EFCD0
void TEneFrog::updateTonguePose()
{
	beginTongueBones();
	gjPushUnitMatrix();
	applyPose(mTonguePose, &mTongueAnim);
	njPopMatrixEx();
	endTongueBones();
}

// 0x800EFD1C
// Only a kind-3 hit carries its damage
int TEneFrog::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x800EFD34
void TEneFrog::applyTransform()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
}

// 0x800EFD6C
void TEneFrog::applyTongueTransform()
{
	njTranslateEx(&mParts[4]);
	gjRotateY(mTongueYaw);
}

// 0x800EFDA4
// Moves by vel and the push
int TEneFrog::moveFiltered(Vec3f* vel)
{
	Vec3f dir;
	getFlatPush(&dir);
	njAddVector(&dir, vel);
	int hit = collideFiltered(12.0f, &mBody.mPos, &dir, &mShadow);
	if (isAirborne() == 0) {
		vel->y = 0.0f;
	}
	return hit;
}

// 0x800EFE2C
// As moveFiltered but the push is worked out and not used
int TEneFrog::move(Vec3f* vel)
{
	Vec3f dir;
	getFlatPush(&dir);
	njAddVector(&dir, vel);
	int hit = collide(12.0f, &mBody.mPos, vel, &mShadow);
	if (isAirborne() == 0) {
		vel->y = 0.0f;
	}
	return hit;
}

// 0x800EFEB4
// Slows flat by rate and falls
void TEneFrog::drag(f32 rate)
{
	mBody.mVel.x *= rate;
	mBody.mVel.y += -0.12f;
	mBody.mVel.z *= rate;
	moveFiltered(&mBody.mVel);
}

// 0x800EFF00
void TEneFrog::keepTongueFx()
{
	for (int i = 0; i < 8; i++) {
		particleKeepAt((void**)&mTongueFx[i], &mTongue[i], 0, 0x56);
	}
}

// 0x800EFF64
void TEneFrog::stopTongueFx()
{
	for (int i = 7; i >= 0; i--) {
		if (mTongueFx[i] != 0) {
			mTongueFx[i]->mFlags |= 1;
			mTongueFx[i] = 0;
		}
	}
}

// 0x800EFFA0
void TEneFrog::onActionPhase(int action, int phase)
{
	switch (action) {
	case ACT_APPEAR:
		actAppear(phase);
		break;
	case ACT_IDLE:
		actIdle(phase);
		break;
	case ACT_HOP:
		actHop(phase);
		break;
	case ACT_LICK:
		actLick(phase);
		break;
	case ACT_RETRACT:
		actRetract(phase);
		break;
	case ACT_STUN:
		actStun(phase);
		break;
	case ACT_STUN_LICK:
		actStunLick(phase);
		break;
	case ACT_DIE:
		actDie(phase);
		break;
	}
}

// 0x800F0038
void TEneFrog::actAppear(int phase)
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

// 0x800F00DC
void TEneFrog::actIdle(int phase)
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

// 0x800F0158
// Hops toward mTarget once each time the crouch passes frames 8 to 10
void TEneFrog::actHop(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.05f, &mAnim, 6);
		mFlags2 &= ~(FROG_ARRIVED | FROG_OVERSHOT);
		clearFrameKey();
		mStep     = 0;
		mHopAngle = 0;
		mBestDist = 100000.0f;
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		f32 frame = motionCtrlFrame(&mAnim);
		mFlags2 &= ~FROG_ARRIVED;
		switch (mStep) {
		case 0:
			updatePose();
			if (motionCtrlIsBlending(&mAnim) == 0) {
				mStep = 1;
			}
			drag(0.8f);
			break;
		case 1:
			updatePose();
			if (frame >= 8.0f && frame <= 10.0f) {
				int angle = angleApproach(mBody.mRot.y, toAngle(mTarget.x - mBody.mPos.x, mTarget.z - mBody.mPos.z), 0x4000);
				njSinCos(angle, &mBody.mVel.x, &mBody.mVel.z);
				mBody.mVel.x *= 1.3f;
				mBody.mVel.z *= 1.3f;
				mBody.mVel.y += 2.2f;
				mStep     = 2;
				mHopAngle = angle;
				move(&mBody.mVel);
				seStartAt(0x108, &mBody.mPos);
			} else {
				drag(0.8f);
			}
			break;
		case 2:
			turnTo(mHopAngle, 0x800);
			updatePose();
			mBody.mVel.y += -0.12f;
			if (mBody.mVel.y <= 0.0f) {
				mStep = 3;
				motionCtrlPlayBlend(0.25f, &mAnim, 8);
			}
			move(&mBody.mVel);
			break;
		case 3:
			if (frame > 20.0f) {
				motionCtrlSetFrame(20.0f, &mAnim);
			}
			updatePose();
			mBody.mVel.y += -0.12f;
			if (isAirborne() == 0) {
				mStep = 4;
				drag(0.8f);
			} else {
				move(&mBody.mVel);
			}
			break;
		case 4:
			updatePose();
			if (motionCtrlIsDone(&mAnim) != 0) {
				motionCtrlPlay(&mAnim, 6);
				mStep = 1;
				if ((mFlags2 & FROG_OVERSHOT) != 0) {
					mFlags2 |= FROG_ARRIVED;
				}
			}
			drag(0.8f);
			break;
		}
		f32 dist = distXZ(&mBody.mPos, &mTarget);
		if (dist <= mBestDist) {
			mBestDist = dist;
			mFlags2 &= ~FROG_OVERSHOT;
		} else {
			mFlags2 |= FROG_OVERSHOT;
		}
		break;
	}
	case 2:
		mFlags2 &= ~(FROG_ARRIVED | FROG_OVERSHOT);
		break;
	}
}

// 0x800F04B8
// Turns to the player and shoots the tongue out
void TEneFrog::actLick(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0);
		motionCtrlPlay(&mTongueAnim, 1);
		mFlags2 &= ~(FROG_TONGUE_SHOW | FROG_TONGUE_HURT | FROG_TONGUE_FX);
		mStep    = 0;
		mSubStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		switch (mStep) {
		case 0:
			if (motionCtrlPassed(20.0f, &mAnim) != 0) {
				motionCtrlPlay(&mTongueAnim, 0);
				mSubStep = 1;
				mFlags2 |= FROG_TONGUE_SHOW | FROG_TONGUE_FX;
				seStartAt(0x10A, &mBody.mPos);
			}
			if (motionCtrlIsDone(&mAnim) != 0) {
				motionCtrlPlayBlend(0.2f, &mAnim, 1);
				mStep = 1;
			}
			turnTo(mAimAngle, 0x200);
			break;
		case 1:
			turnTo(mAimAngle, 0x80);
			break;
		}
		switch (mSubStep) {
		case 0:
			break;
		case 1:
			motionCtrlUpdate(&mTongueAnim);
			if (motionCtrlIsDone(&mTongueAnim) != 0) {
				motionCtrlPlayBlend(0.3f, &mTongueAnim, 1);
				mSubStep = 2;
				mFlags2 |= FROG_TONGUE_HURT;
			}
			break;
		case 2:
			motionCtrlUpdate(&mTongueAnim);
			break;
		}
		updateTonguePose();
		drag(0.8f);
		break;
	case 2:
		mFlags2 &= ~(FROG_TONGUE_SHOW | FROG_TONGUE_HURT | FROG_TONGUE_FX);
		break;
	}
}

// 0x800F0680
// Draws the tongue back in slowing until the body's motion reaches frame 12
void TEneFrog::actRetract(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 2);
		motionCtrlPlay(&mTongueAnim, 1);
		mFlags2 |= FROG_TONGUE_SHOW | FROG_TONGUE_HURT | FROG_TONGUE_FX;
		mStep     = 0;
		mBestDist = 1.0f;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		motionCtrlUpdate(&mTongueAnim);
		updateTonguePose();
		if (mStep == 0) {
			mBestDist -= 0.012f;
			if (mBestDist < 0.1f) {
				mBestDist = 0.1f;
			}
			motionCtrlSetSpeed(&mTongueAnim, mBestDist);
			if (motionCtrlFrame(&mAnim) >= 12.0f) {
				motionCtrlPlayBlend(0.1f, &mTongueAnim, 2);
				motionCtrlSetSpeed(&mTongueAnim, 1.0f);
				mFlags2 &= ~(FROG_TONGUE_HURT | FROG_TONGUE_FX);
				mStep = 1;
				seStartAt(0x109, &mBody.mPos);
			}
		} else if (motionCtrlIsDone(&mTongueAnim) != 0) {
			mFlags2 &= ~FROG_TONGUE_SHOW;
		}
		drag(0.8f);
		break;
	case 2:
		mFlags2 &= ~(FROG_TONGUE_SHOW | FROG_TONGUE_HURT | FROG_TONGUE_FX);
		motionCtrlSetSpeed(&mTongueAnim, 1.0f);
		break;
	}
}

// 0x800F0808
void TEneFrog::actStun(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 3);
		mEnemyFlags |= ENEMY_FLIPPING;
		mFrameKey = 3;
		noteStun();
		mBody.mVel.x = mKnock.x;
		mBody.mVel.z = mKnock.z;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		setFrameKey(sFrogFrameKeys[3]);
		waitMotion();
		drag(0.7f);
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		clearFrameKey();
		break;
	}
}

// 0x800F08DC
// As actStun with the tongue left out and drawn
void TEneFrog::actStunLick(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 4);
		mEnemyFlags |= ENEMY_FLIPPING;
		mFrameKey = 3;
		noteStun();
		mStep = 0;
		mFlags2 |= FROG_TONGUE_SHOW;
		mFlags2 &= ~FROG_TONGUE_FX;
		motionCtrlPlay(&mTongueAnim, 2);
		mBody.mVel.x = mKnock.x;
		mBody.mVel.z = mKnock.z;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		motionCtrlUpdate(&mTongueAnim);
		updateTonguePose();
		drag(0.7f);
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		clearFrameKey();
		mFlags2 &= ~(FROG_TONGUE_SHOW | FROG_TONGUE_FX);
		break;
	}
}

// 0x800F09E8
// Flips over and falls to the ground then waits and goes leaving a drop
void TEneFrog::actDie(int phase)
{
	Vec3f pos;
	Vec3f ground;
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 5);
		mEnemyFlags |= ENEMY_BLOCKED | ENEMY_INTANGIBLE;
		colliDisable(mCollider);
		mBar->hide();
		killed(1);
		noteDie();
		mFrameKey    = 3;
		mSubStep     = 0;
		mStep        = 0;
		mBody.mVel.y = 0.0f;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		switch (mSubStep) {
		case 0:
			if (motionCtrlFrame(&mAnim) >= 2.0f) {
				mSubStep = 1;
				mStep    = 240;
			}
			break;
		case 1:
			pos   = mParts[0];
			pos.y = 10.0f + mBody.mPos.y;
			mBody.mVel.y += -0.12f;
			mBody.mPos.y += mBody.mVel.y;
			if (probeGround(&pos, &ground, 0) != 0) {
				if (ground.y - mBody.mPos.y >= 0.0f) {
					mSubStep     = 2;
					mStep        = 60;
					mBody.mPos.y = ground.y;
				} else if (motionCtrlFrame(&mAnim) > 4.0f) {
					motionCtrlSetFrame(4.0f, &mAnim);
				}
			} else {
				shadowDisable(&mShadow);
				mStep--;
				if (mStep < 0) {
					mSubStep = 3;
				}
			}
			break;
		case 2:
			if (motionCtrlPassed(5.0f, &mAnim) != 0) {
				pos   = mParts[0];
				pos.y = mBody.mPos.y;
				particleOnceByIdDefault(&pos, 0, 0x46);
				seEnemyPuff(&pos);
			}
			break;
		}
		pos   = mParts[0];
		pos.y = mBody.mPos.y;
		shadowSetGround(&mShadow, &pos, 0);
		updatePose();
		setFrameKey(sFrogFrameKeys[4]);
		if (motionCtrlIsDone(&mAnim) != 0 || mSubStep == 3) {
			mStep--;
			if (mStep <= 0) {
				mSubStep = 3;
			}
		}
		if (mSubStep == 3) {
			enemyFxDeath4A(&mBody.mPos);
			spawnDropB(&mBody.mPos);
			mFlags |= 1;
		}
		break;
	case 2:
		mEnemyFlags &= ~(ENEMY_BLOCKED | ENEMY_INTANGIBLE);
		clearFrameKey();
		break;
	}
}

// 0x800F0CEC
void TEneFrog::vfn_3C()
{
	int x    = kEnemyInfoX << 16;
	int line = kEnemyInfoLine;
	njPrint(x | line, ":HP %4d", mHealth);
}

// 0x800F0D28
void TEneFrog::vfn_10()
{
	if (isAppearing() != 0) {
		mFlags |= 0x10;
	} else if (mLight.isVisible(mModel, 500.0f, &mBody.mPos, 60.0f, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x800F0DB8
// The tongue is drawn only while it is out
void TEneFrog::render()
{
	njSetTexture(*(void**)sFrogTextures);
	enemyEyeSet(*(NJS_TEXLIST**)sFrogTextures, 0, mFrameKey);
	useArea();
	beginLit(&mLight, &mTint);
	njPushMatrixEx();
	applyTransform();
	gjDrawActor(mPose);
	njPopMatrixEx();
	if ((mFlags2 & FROG_TONGUE_SHOW) != 0) {
		njPushMatrixEx();
		applyTongueTransform();
		gjDrawActor(mTonguePose);
		njPopMatrixEx();
	}
	endLit();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}
