#include "io/ResBank.h"
#include "enemy/boss/TEnePurpleBoss.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "enemy/boss/BossParts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "engine/MotionPose.h"
#include "stage/SetManager.h"
#include "Ninja/ninja.h"
#include "object/ObjModel.h"
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "enemy/EnemyHit.h"
#include "enemy/ObjEnemy.h"
#include "enemy/boss/TEnePurpleBoss_IceBall.h"
#include "core/MathUtil.h"
#include "object/ObjShadow.h"
#include "camera/TCameraCtrl.h"
#include "enemy/EnemyTexFx.h"
#include "object/TObjLauncher.h"
#include "enemy/EnemyGround.h"
#include "enemy/EnemyCommon.h"
#include "engine/DrawState.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "ui/AeArc.h"
#include "camera/TCamPath.h"
#include "stl/math.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "enemy/EnemyFx.h"
#include "engine/TParticle.h"
#include "enemy/TEneAm01.h"
#include "animal/RacerMover.h"
struct NJS_QUATERNION;
struct NJS_OBJECT;
struct DrawState;
struct Vec3f;
struct ColliShape;
struct GjActor;
struct DrawStateSlot;
struct NJS_TEXLIST;
struct GroundFilter;
struct Quat;
struct EnemyLight;

extern "C" {
extern const int sPurpleBossNodes[] = { 1, 15, 24, 34, 36, 59, -1 };
const char* gTEnePurpleBossName = "TEnePurpleBoss";
extern const EnemyFrameKey sPurpleBossFrameKey0[] = { { 18.0f, 0 }, { 28.0f, 5 }, { 41.0f, 0 }, { 47.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sPurpleBossFrameKey1[] = { { 10.0f, 4 }, { 27.0f, 0 }, { 37.0f, 4 }, { -1.0f, 0 } };
extern const EnemyFrameKey sPurpleBossFrameKey2[] = { { 12.0f, 3 }, { 23.0f, 0 }, { 30.0f, 4 }, { -1.0f, 0 } };
extern const EnemyFrameKey sPurpleBossFrameKey3[] = { { 54.0f, 0 }, { -1.0f, 4 } };
extern const EnemyFrameKey sPurpleBossFrameKey4[] = { { 12.0f, 4 }, { 23.0f, 0 }, { 28.0f, 4 }, { -1.0f, 0 } };
extern const ColliShape sPurpleBossShapes[1] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 25.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
MotionEntry sPurpleBossMotionKeys[] = {
	{ MOTION_LOOP, 11, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 12, -1, 0, 0.0f, 35.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 0.0f, 59.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 8, -1, 0, 0.0f, 30.0f, 1.4f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 59.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 6, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 59.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 69.0f, 0.45f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 14.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 14.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 419.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 10, -1, 0, 0.0f, 539.0f, 1.0f, 0.0f, 0 },
};
PurpleBossParams* gPurpleBossParams;
}

// 0x800C7DC4
extern "C" void initTEnePurpleBoss()
{
	resBankLoad(&sPurpleBossBank, "ar_ene_purple_boss.arc");
	resBankAddTextures(&sPurpleBossBank, 0, "ene_purple_boss.gvm");
	sPurpleBossModels   = resBankGetModels(&sPurpleBossBank);
	sPurpleBossTextures = resBankGetTextures(&sPurpleBossBank);
	sPurpleBossMotion   = resBankGetMotion(&sPurpleBossBank, 0);
	gPurpleBossParams   = (PurpleBossParams*)enemyParamGet(4);
	fxBankLoadDefault(&sPurpleBossFx, "pa_purpleboss.pad");
	iceBallLoad();
}

// 0x800C7E64
extern "C" void shutdownTEnePurpleBoss()
{
	iceBallFree();
	fxBankRelease(&sPurpleBossFx);
	gPurpleBossParams = 0;
	resBankRemoveTextures(&sPurpleBossBank, 0);
	resBankRelease(&sPurpleBossBank);
	sPurpleBossModels   = 0;
	sPurpleBossTextures = 0;
	sPurpleBossMotion   = 0;
}

// 0x800C7EC0
extern "C" TObject* createTEnePurpleBoss(TObject* parent, SetDesc* desc)
{
	return new TEnePurpleBoss(parent, desc);
}

// 0x800C7F14
// Model-table word for an index below 33
extern "C" NJS_OBJECT* modelAtTEnePurpleBoss(int index)
{
	if (index < 0 || index >= 33) {
		return 0;
	}
	return sPurpleBossModels[index];
}

// 0x800C7F3C
extern "C" NJS_TEXLIST* shapeTEnePurpleBoss()
{
	return *sPurpleBossTextures;
}

// 0x800C7F48
extern "C" void* paramsTEnePurpleBoss()
{
	return (int*)sPurpleBossNodes;
}

// 0x800C7F54
TEnePurpleBoss::TEnePurpleBoss(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00 = (u32)gTEnePurpleBossName;
	mSize = 0x318;
	init(desc);
	initBossBar();
}

// 0x800C7FF8
void TEnePurpleBoss::init(SetDesc* desc)
{
	int i;

	motionCtrlInit(&mAnim, sPurpleBossMotionKeys, sPurpleBossMotion);
	motionCtrlPlay(&mAnim, 0);
	mBody.mRot.x = 0;
	mCollider = colliCreate(1, 1);
	colliCopyShapes(mCollider, (const ColliShape*)sPurpleBossShapes);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0xFF, 0, 0x10);
	colliShapeSetTag(mCollider, 0, 0);
	colliShapeSetBit8(mCollider, 0);
	colliOn();
	mBrain = 0;
	readEnemyDesc(desc);
	mPose      = (GjActor*)allocPose(sPurpleBossModels[32]);
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mBossFlags = 0;
	mHitDir.x  = 0.0f;
	mHitDir.y  = 0.0f;
	mHitDir.z  = 0.0f;
	mFrameKey  = 0;
	for (i = 0; i < 6; i++) {
		mNodes[i].x = 0.0f;
		mNodes[i].y = 0.0f;
		mNodes[i].z = 0.0f;
	}
	_1BC.x = 0.0f;
	_1BC.y = 0.0f;
	_1BC.z = 0.0f;
	_1C8.x = 0.0f;
	_1C8.y = 0.0f;
	_1C8.z = 0.0f;
	mTargetAngle = 0;
	mMaxHealth   = 500;
	mHealth      = 500;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 50.0f;
	mShadow._34 = 50.0f;
	shadowEnable(&mShadow);
	mFx = 0;
	seHolderStop(&mSe);
	mSe.setRange(700.0f);
	seCueClear(&mCue);
	mDust = 0;
	mLight.reset();
	mTint.reset();
	mSpeed     = 0.0f;
	mSpinSpeed = 5.0f;
	for (i = 0; i < 4; i++) {
		(&mTrail)[i] = 0;
	}
	mIceScale = 0.0f;
	mIceY     = 40.0f;
	njCreateQuaternion(0, 0.0f, 0.0f, 1.0f, (NJS_QUATERNION*)mQuat);
	mSkid      = 0;
	mActStep   = 0;
	mActTimer  = 0;
	mTurnRate  = 0;
	mTurnSpeed = 0.0f;
	makeBrain();
	setBossView(10.0f, 33.0f, 150.0f);
}

// 0x800C825C
TEnePurpleBoss::~TEnePurpleBoss()
{
	int i;

	for (i = 3; i >= 0; i--) {
		if ((&mTrail)[i] != 0) {
			(&mTrail)[i]->mFlags |= 1;
			(&mTrail)[i] = 0;
		}
	}
	seHolderStop(&mSe);
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	unlock();
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
	removeDrawTask(mDrawTask);
	if (mDust != 0) {
		mDust->mFlags |= 1;
		mDust = 0;
	}
	freePose((void**)&mPose);
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x800C83C4
void TEnePurpleBoss::setPos(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	mBody.mPos.x = x;
	f32 z = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x800C8408
f32 TEnePurpleBoss::vfn_40()
{
	return 80.0f;
}

// 0x800C8410
// One sphere the body's middle node or the ice block around it while it spins
void TEnePurpleBoss::syncCollider()
{
	Vec3f* shape = (Vec3f*)colliGetShapes(mCollider);

	if (mBossFlags & BOSS_SPINNING) {
		*shape = mBody.mPos;
		shape->y += 40.0f;
		colliShapeSetRadius((ColliShape*)shape, 40.0f);
	} else {
		*shape = mNodes[0];
		colliShapeSetRadius((ColliShape*)shape, 25.0f);
	}
}

// 0x800C849C
void TEnePurpleBoss::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	desc->getInt(0);
}

// 0x800C84E0
// Players bounce off it for 20
int TEnePurpleBoss::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit = 0;

	if (!(mBossFlags & BOSS_CLOSED)) {
		if (isPlayerOrKind4(other)) {
			info->mDamage = 20;
			hit = 1;
			info->mFlags.clear();
			if (mBossFlags & BOSS_HITTABLE) {
				mBossFlags |= BOSS_BOUNCED;
			}
		} else if (isEggObj(other) && (mBossFlags & (BOSS_HITTABLE | BOSS_ICE))) {
			info->mDamage = 1;
			hit = 1;
			info->mFlags.clear();
			info->mFlags.setBit(13);
		}
	}
	return hit;
}

// 0x800C859C
void TEnePurpleBoss::setTargetAngle(int angle)
{
	mTargetAngle = angle;
}

// 0x800C85A4
void TEnePurpleBoss::setHealth(int health)
{
	mMaxHealth = health;
	mHealth    = health;
}

// 0x800C85B0
int TEnePurpleBoss::isAction0Done()
{
	return isActionDone(0);
}

// 0x800C85D4
int TEnePurpleBoss::isAction1Done()
{
	return isActionDone(1);
}

// 0x800C85F8
int TEnePurpleBoss::wasBounced()
{
	return mBossFlags & BOSS_BOUNCED;
}

// 0x800C8604
int TEnePurpleBoss::hitWall()
{
	return mBossFlags & BOSS_WALLED;
}

// 0x800C8610
int TEnePurpleBoss::isAction3Done()
{
	return isActionDone(3);
}

// 0x800C8634
int TEnePurpleBoss::isAction4Done()
{
	return isActionDone(4);
}

// 0x800C8658
int TEnePurpleBoss::isAction6Done()
{
	return isActionDone(6);
}

// 0x800C867C
int TEnePurpleBoss::isAction7Done()
{
	return isActionDone(7);
}

// 0x800C86A0
void TEnePurpleBoss::setSkateSpeed(f32 speed)
{
	mSpinSpeed = speed;
}

// 0x800C86A8
int TEnePurpleBoss::wasBumped()
{
	return mBossFlags & BOSS_BUMPED;
}

// 0x800C86B4
void TEnePurpleBoss::throwBalls(int count)
{
	Vec3f at;

	at.x = mBody.mPos.x;
	at.y = mBody.mPos.y;
	at.z = mBody.mPos.z;
	at.y += 40.0f;
	throwIceBalls(this, &at, gPurpleBossParams->mBallSpeeds, 3, count, gPurpleBossParams->mBallRadius);
}

// 0x800C8710
int TEnePurpleBoss::isAction10Done()
{
	return isActionDone(10);
}

// 0x800C8734
int TEnePurpleBoss::isAction11Done()
{
	return isActionDone(11);
}

// 0x800C8758
void TEnePurpleBoss::stageBeaten()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
	hideBossBar();
	seStartAtFar(0x13E, &mBody.mPos);
}

// 0x800C87A8
void TEnePurpleBoss::hurtReact()
{
	enemyFxSparks(3.0f, &mContact.mSurface, &mContact.mCenter);
	seStartAtFar(0x131, &mBody.mPos);
}

// 0x800C87EC
int TEnePurpleBoss::dieAction()
{
	return 12;
}

// 0x800C87F4
void TEnePurpleBoss::updateBody()
{
	if (!isBlocked() && takeHits(&mHit, &mContact)) {
		enemyHitPushAway((Vec3f*)&mHit, &mBody.mPos, (const Vec3f*)&mContact, &mHitDir);
		applyDamage(&mHit);
	}
	mBrain->update(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	mTint.update();
	syncCollider();
	updateTrails();
	setBossBar(mMaxHealth, mHealth);
}

// 0x800C88D8
void TEnePurpleBoss::vfn_6C()
{
	if (returnZero()) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginNodes();
		drawFirstFrame(sPurpleBossMotion, 13, mPose);
		endNodes();
		calcPoints();
		syncCollider();
	}
}

// 0x800C8954
void TEnePurpleBoss::makeBrain()
{
	mBrain = createPurpleBrain(this);
}

// 0x800C8984
void TEnePurpleBoss::beginNodes()
{
	nodeTrackBeginOrigins(sPurpleBossNodes, mNodes);
}

// 0x800C89B4
void TEnePurpleBoss::endNodes()
{
	nodeTrackEnd();
}

// 0x800C89D4
void TEnePurpleBoss::updatePose()
{
	beginNodes();
	applyPoseFaded(mPose, 25.0f, &mAnim, &mAppearTimer);
	endNodes();
}

// 0x800C8A20
// The motion moves the root so the body follows it and the pose is drawn back at the origin
void TEnePurpleBoss::updatePoseRooted()
{
	MotionNodePose root;
	NJS_MOTION* node = motionCtrlGetMotion(&mAnim);

	motionGetNodePose(&root, *(NJS_OBJECT**)mPose, node, motionCtrlFrame(&mAnim));
	motionPoseGetPos(&root, &mBody.mPos);
	mBody.mRot.x = root.mAng[0];
	beginNodes();
	gjPushUnitMatrix();
	gjRotateX(-mBody.mRot.x);
	gjTranslate(-mBody.mPos.x, -mBody.mPos.y, -mBody.mPos.z);
	applyPose(mPose, &mAnim);
	njPopMatrixEx();
	endNodes();
}

// 0x800C8AD8
void TEnePurpleBoss::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 6; i++) {
		njCalcPointEx(&mNodes[i], &mNodes[i]);
	}
	njPopMatrixEx();
}

// 0x800C8B38
int TEnePurpleBoss::getHitKind(const void* touch, HitInfo* info)
{
	return (mBossFlags & BOSS_DASH) ? 3 : 1;
}

// 0x800C8B50
int TEnePurpleBoss::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x800C8B68
void TEnePurpleBoss::bodyMatrix()
{
	if (mBossFlags & BOSS_SPINNING) {
		njTranslateEx(&mBody.mPos);
		njTranslate(0, 0.0f, 40.0f, 0.0f);
		njQuaternionEx((NJS_QUATERNION*)mQuat);
		njTranslate(0, 0.0f, -40.0f, 0.0f);
	} else {
		njTranslateEx(&mBody.mPos);
		gjRotateY(mBody.mRot.y);
		gjRotateX(mBody.mRot.x);
	}
}

// 0x800C8BF0
void TEnePurpleBoss::gravity(Vec3f* vel)
{
	vel->y += -0.12f;
}

// 0x800C8C04
// Returns the wall bits
int TEnePurpleBoss::moveBody(Vec3f* vel)
{
	Vec3f to;
	SlopeFilter filter;
	u32 result;

	getFlatPush(&to);
	njAddVector(&to, vel);
	initEnemyFilter(&filter);
	result = ::moveBody(25.0f, &mBody.mPos, &to, &filter, &mShadow);
	setAirborne(result);
	if (!isAirborne()) {
		vel->y = 0.0f;
	}
	return result & 0xC;
}

// 0x800C8CB8
int TEnePurpleBoss::fallMove(Vec3f* vel)
{
	gravity(vel);
	return moveBody(vel);
}

// 0x800C8CF8
void TEnePurpleBoss::skateMove(f32 drag)
{
	gravity(&mBody.mVel);
	mBody.mVel.x *= drag;
	mBody.mVel.z *= drag;
	if (moveBody(&mBody.mVel)) {
		mBody.mVel.x = 0.0f;
		mBody.mVel.z = 0.0f;
	}
}

// 0x800C8D74
// Spinning on the ice bounces off the walls and rolls the quaternion along the way it went
void TEnePurpleBoss::spinMove()
{
	EnemyFilter filter;
	Vec3f out;
	Vec3f normal;
	Vec3f hit;
	Vec3f dir;
	Vec3f from;
	Vec3f start;
	f32 speed;
	f32 vy;

	mBody.mVel.y += -0.12f;
	dir.x = mBody.mVel.x;
	dir.y = mBody.mVel.y;
	vy    = dir.y;
	dir.z = mBody.mVel.z;
	dir.y = 0.0f;
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	speed  = mSpinSpeed;
	start  = mBody.mPos;
	from   = mBody.mPos;
	from.y = start.y + 40.0f;
	if (probeWall(40.0f, &from, &dir, &speed, &hit, &normal)) {
		mBossFlags |= BOSS_BUMPED;
		particleOnceByIdRotDefault(&hit, 1, 0x48, 0x4000, (int)(10430.38043493439 * (f32)atan2(normal.x, normal.z)),
		                           0.0f);
	} else {
		mBossFlags &= ~BOSS_BUMPED;
	}
	gjMulAddVector(speed, &hit, &from, &dir);
	hit.y = 10.0f + mBody.mPos.y;
	out.x = -mBody.mPos.x;
	out.y = 0.0f;
	out.z = -mBody.mPos.z;
	if (vecDotXZ(&out, &dir) >= 0.0f) {
		GroundFilter simple;
		moveOnGround(vy, &mBody.mPos, &hit, &simple, &mShadow);
	} else {
		filter.mVtable = __vt__11SlopeFilter;
		initEnemyFilter((SlopeFilter*)&filter);
		if (moveOnGround(vy, &mBody.mPos, &hit, (GroundFilter*)&filter, &mShadow) & 8) {
			mBossFlags |= BOSS_BUMPED;
			normal.x = -out.x;
			normal.y = 0.0f;
			normal.z = -out.z;
		}
	}
	njSubVector(&start, &mBody.mPos);
	gjScaleVector(-1.0f, &start, &start);
	rollQuat(&start, (Quat*)mQuat, 40.0f);
	if (mBossFlags & BOSS_BUMPED) {
		normal.y = 0.0f;
		if (njUnitVector(&normal) < gUnitVectorMin) {
			normal = gVecZero;
		}
		vecReflect(&dir, &normal, &dir);
		speed = mSpinSpeed;
		seStartAtFar(0x13C, &mBody.mPos);
	}
	mBody.mVel.x = dir.x * speed;
	mBody.mVel.y = vy;
	mBody.mVel.z = dir.z * speed;
	skidMark((int)(10430.38043493439 * (f32)atan2(dir.x, dir.z)));
}

// 0x800C90A4
void TEnePurpleBoss::openHit()
{
	mBossFlags |= BOSS_HITTABLE;
	mBossFlags &= ~(BOSS_BOUNCED | BOSS_WALLED);
}

// 0x800C90C0
void TEnePurpleBoss::closeHit()
{
	mBossFlags &= ~(BOSS_HITTABLE | BOSS_BOUNCED | BOSS_WALLED);
}

// 0x800C90D0
void TEnePurpleBoss::colliOn()
{
	colliAddMasks(mCollider, 0xFF, 0, 0);
}

// 0x800C9100
void TEnePurpleBoss::colliOff()
{
	colliClearMasks(mCollider, 9, 0, 0);
}

// 0x800C9130
void TEnePurpleBoss::resetSpeed()
{
	mSpeed = 550.0f;
}

// 0x800C913C
// How fast it has to go to stay inside the rink from where it is and which way it goes
#pragma opt_common_subs off
void TEnePurpleBoss::aimSpeed()
{
	f32 t[2];
	f32 pz = mBody.mPos.z;
	f32 vx;
	f32 vz = mBody.mVel.z;
	f32 px = mBody.mPos.x;
	vx = mBody.mVel.x;
	pz *= 1.2f;
	vz *= 1.2f;
	f32 x;
	f32 z;

	t[1] = 1.2f;
	quadRoots(vx * vx + vz * vz, 2.0f * (px * vx + pz * vz), (px * px + pz * pz) - 90000.0f, &t[0],
	            &t[1]);
	if (t[1] > t[0]) {
		t[0] = t[1];
	}
	vx *= t[0];
	vz *= 0.8333333f * t[0];
	t[1] = vx * vx + vz * vz;
	z = __frsqrte(t[1]);
	mSpeed = __fres(z);
}
#pragma opt_common_subs reset

// 0x800C921C
// Away from the way it was going or toward the middle when it stands still
void TEnePurpleBoss::dashBack(f32 scale)
{
	f32 speed = 2.0f;
	speed = -speed;
	speed *= scale;
	f32 x;
	f32 z = mBody.mVel.z;
	x = mBody.mVel.x;
	f32 len = x * x + z * z;

	if (len <= 1e-8f) {
		x   = -mBody.mPos.x;
		z   = -mBody.mPos.z;
		len = x * x + z * z;
	}
	len = speed * (f32)__frsqrte(len);
	x *= len;
	z *= len;
	mBody.mVel.x = x;
	mBody.mVel.z = z;
	mBossFlags |= BOSS_CLOSED | BOSS_DASH;
}

// 0x800C928C
void TEnePurpleBoss::dashStep(int* step)
{
	f32 frame = motionCtrlFrame(&mAnim);

	switch (*step) {
	case 0:
		if (frame >= 16.0f) {
			mBody.mVel.x *= 0.7f;
			mBody.mVel.z *= 0.7f;
			bigDustBurst(&mBody.mPos);
			particleOnceByIdDefault(&mBody.mPos, 1, 0x4A);
			*step = 1;
			seStartAtFar(0x133, &mBody.mPos);
		}
		fallMove(&mBody.mVel);
		break;
	case 1:
		if (frame >= 21.0f) {
			*step = 2;
		}
		fallMove(&mBody.mVel);
		break;
	case 2:
		skateMove(0.8f);
		break;
	}
}

// 0x800C9388
void TEnePurpleBoss::endDash()
{
	mBossFlags &= ~(BOSS_CLOSED | BOSS_DASH);
}

// 0x800C939C
void TEnePurpleBoss::playSpin()
{
	motionCtrlPlay(&mAnim, 8);
	motionCtrlUpdate(&mAnim);
	motionCtrlSetFrame(69.0f, &mAnim);
}

// 0x800C93E4
void TEnePurpleBoss::showIce()
{
	mBossFlags |= BOSS_SPINNING | BOSS_ICE;
	mIceScale = 1.0f;
	mIceY     = 40.0f;
}

// 0x800C9404
void TEnePurpleBoss::hideIce()
{
	mBossFlags &= ~(BOSS_SPINNING | BOSS_ICE);
	mIceScale = 0.0f;
}

// 0x800C941C
void TEnePurpleBoss::dustBurst(Vec3f* pos)
{
	particleOnceByIdDefault(pos, 1, 0x48);
	particleOnceByIdDefault(pos, 1, 0x49);
}

// 0x800C9464
void TEnePurpleBoss::bigDustBurst(Vec3f* pos)
{
	dustBurst(pos);
	particleOnceByIdDefault(pos, 1, 0x4A);
}

// 0x800C94A0
void TEnePurpleBoss::clearSkid()
{
	mSkid = 0;
}

// 0x800C94AC
// A skid mark every eighth frame
void TEnePurpleBoss::skidMark(int angle)
{
	if ((mSkid & 7) == 0) {
		fxSpawnScaled(3.0f, 1.0f, 3.0f, 5, (void*)3, &mBody.mPos, 0, angle);
	}
	mSkid++;
}

// 0x800C9510
void TEnePurpleBoss::killSparks()
{
	if (mSpark[0] != 0) {
		mSpark[0]->mFlags |= 1;
		mSpark[0] = 0;
	}
	if (mSpark[1] != 0) {
		mSpark[1]->mFlags |= 1;
		mSpark[1] = 0;
	}
}

// 0x800C9554
void TEnePurpleBoss::keepSparks(Vec3f* pos, int angle)
{
	particleKeepAtRot((void**)&mSpark[0], pos, -0x4000, angle, 1, 0x4E);
	particleKeepAtRot((void**)&mSpark[1], pos, -0x4000, angle, 1, 0x4D);
}

// 0x800C95B8
void TEnePurpleBoss::trailOff()
{
	mBossFlags &= ~(BOSS_TRAIL | BOSS_SPRAY);
}

// 0x800C95C8
void TEnePurpleBoss::trailOn()
{
	mBossFlags |= BOSS_TRAIL | BOSS_SPRAY;
}

// 0x800C95D8
void TEnePurpleBoss::sprayOn()
{
	mBossFlags &= ~BOSS_TRAIL;
	mBossFlags |= BOSS_SPRAY;
}

// 0x800C95F4
void TEnePurpleBoss::updateTrails()
{
	if (mBossFlags & BOSS_TRAIL) {
		particleKeepAt((void**)&mTrail, &mBody.mPos, 1, 0x45);
	} else {
		if (mTrail != 0) {
			mTrail->mFlags |= 1;
			mTrail = 0;
		}
	}
	if (mBossFlags & BOSS_SPRAY) {
		particleKeepAt((void**)&mSpray, &mBody.mPos, 1, 0x46);
	} else {
		if (mSpray != 0) {
			mSpray->mFlags |= 1;
			mSpray = 0;
		}
	}
}

// 0x800C96A4
void TEnePurpleBoss::spawnFx(int model, int motion)
{
	void* object = resBankGetObjects(&sPurpleBossBank)[model];
	NJS_MOTION** motions = resBankGetMotion(&sPurpleBossBank, 1);

	mFx = createCamPath(0, 2, object, motions[motion], 0, 1.0f, 1.0f);
	if (mFx != 0) {
		((TCamera*)mFx)->setCut();
	}
}

// 0x800C9734
void TEnePurpleBoss::onActionPhase(int action, int phase)
{
	switch (action) {
	case 0:
		actAppear(phase);
		break;
	case 1:
		actTurn(phase);
		break;
	case 2:
		actSkate(phase);
		break;
	case 3:
		actBrake(phase);
		break;
	case 4:
		actDash(phase);
		break;
	case 5:
		actDazed(phase);
		break;
	case 6:
		actSlam(phase);
		break;
	case 7:
		actFreeze(phase);
		break;
	case 8:
		actSpinUp(phase);
		break;
	case 9:
		actSpin(phase);
		break;
	case 10:
		actDrop(phase);
		break;
	case 11:
		actHurt(phase);
		break;
	case 12:
		actDie(phase);
		break;
	}
}

// 0x800C9808
// Action 0 rises out of the ice sheds the big ice block and skates off
void TEnePurpleBoss::actAppear(int phase)
{
	switch (phase) {
	case 0:
		mBody.mPos.x = 0.0f;
		mBody.mPos.y = 0.0f;
		mBody.mPos.z = 0.0f;
		mBody.mRot.x = 0;
		mBody.mRot.y = 0;
		mBody.mRot.z = 0;
		motionCtrlPlay(&mAnim, 12);
		clearFrameKey();
		killSparks();
		clearSkid();
		mBossFlags |= BOSS_CLOSED;
		mActStep = 0;
		break;
	case 1: {
		Vec3f at;
		f32 frame;

		motionCtrlUpdate(&mAnim);
		updatePoseRooted();
		at.x  = mBody.mPos.x;
		at.y  = mBody.mPos.y;
		at.z  = mBody.mPos.z;
		at.y  = 50.0f;
		frame = motionCtrlFrame(&mAnim);
		lockOn();
		switch (mActStep) {
		case 0:
			spawnFx(1, 1);
			seStartArg(0x2A8, 0x230);
			mIceScale = 1.0f;
			mIceY     = 30.0f;
			mActStep  = 1;
			break;
		case 1:
			if (frame >= 110.0f && frame < 139.0f) {
				particleKeepAt((void**)&mSpark[0], &mBody.mPos, 1, 0x45);
			} else {
				if (mSpark[0] != 0) {
					mSpark[0]->mFlags |= 1;
					mSpark[0] = 0;
				}
			}
			if (frame >= 138.0f) {
				mActStep = 2;
				dustBurst(&at);
			}
			break;
		case 2:
			if (frame < 138.0f) {
				break;
			}
			if (frame < 220.0f) {
				keepSparks(&at, mBody.mRot.y + 0x8000);
			} else {
				killSparks();
				mActStep = 3;
			}
			break;
		case 3:
			if (frame >= 294.0f) {
				mIceScale = 0.0f;
				njXYZAngleToQuaternion((const s32*)&mBody.mRot.x, (NJS_QUATERNION*)mQuat);
				new TEnePurpleBoss_BigIceFragment(this, &at, mQuat, 30.0f);
				mActStep = 4;
			}
			break;
		case 4:
			if (frame >= 350.0f) {
				bigDustBurst(&at);
				mActStep = 5;
			}
			break;
		}
		waitMotion();
		calcPoints();
		break;
	}
	case 2:
		clearFrameKey();
		hideIce();
		mBody.mRot.x = 0;
		unlock();
		mBossFlags &= ~BOSS_CLOSED;
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		camSetLookFrom(0, &mBody.mPos);
		break;
	}
}

// 0x800C9B14
// Turns *angle toward target fast between frames 11 and 15 and easing off after
int TEnePurpleBoss::turnStep(f32 frame, int* angle, f32* rate, int target)
{
	int diff  = angleDiff(*angle, target);
	f32 left  = diff;
	int there;

	if (frame >= 11.0f && frame < 15.0f) {
		if (diff >= 0) {
			*rate = 512.0f;
		} else {
			*rate = -512.0f;
		}
	} else {
		*rate *= 0.98f;
	}
	if (left > 0.0f) {
		there = *rate > left;
	} else if (left < 0.0f) {
		there = *rate < left;
	} else {
		there = 1;
	}
	if (there) {
		*rate = left;
	}
	*angle += (int)*rate;
	return there;
}

// 0x800C9C34
// Action 1 turns to face the player
void TEnePurpleBoss::actTurn(int phase)
{
	f32* speed = &mTurnSpeed;
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 1);
		clearFrameKey();
		*speed = 0.0f;
		break;
	case 1: {
		int angle;

		motionCtrlUpdate(&mAnim);
		updatePose();
		angle = mBody.mRot.y;
		if (turnStep(motionCtrlFrame(&mAnim), &angle, speed, mTargetAngle)) {
			mHitFlags |= HIT_DONE;
		}
		mBody.mRot.y = angle;
		skateMove(0.8f);
		calcPoints();
		break;
	}
	case 2:
		break;
	}
}

// 0x800C9D00
// Turns toward angle and past frame 36 pushes off that way
int TEnePurpleBoss::pushOff(SetTransform* body, void* anim, int angle)
{
	int pushed;

	body->mRot.y = angleApproach(body->mRot.y, angle, 0x100);
	pushed = 0;
	if (!motionCtrlIsBlending(anim) && motionCtrlFrame(anim) > 36.0f) {
		pushed = 1;
		njSinCos(body->mRot.y, &body->mVel.x, &body->mVel.z);
		body->mVel.x *= 5.0f;
		body->mVel.z *= 5.0f;
	}
	return pushed;
}

// 0x800C9D9C
int TEnePurpleBoss::pastLaunch(void* anim)
{
	return motionCtrlFrame(anim) > 44.0f;
}

// 0x800C9DCC
// Action 2 pushes off and skates until it runs into something
void TEnePurpleBoss::actSkate(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 2);
		mActStep = 0;
		closeHit();
		resetSpeed();
		trailOff();
		clearFrameKey();
		seHolderStop(&mSe);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		switch (mActStep) {
		case 0:
			if (pushOff(&mBody, &mAnim, mTargetAngle)) {
				mActStep = 1;
				openHit();
				fallMove(&mBody.mVel);
				seStartAtFar(0x132, &mBody.mPos);
			} else {
				skateMove(0.8f);
				if (motionCtrlPassed(22.0f, &mAnim)) {
					seStartAtFar(0x134, &mBody.mPos);
				}
			}
			setFrameKey(sPurpleBossFrameKeys[0]);
			break;
		case 1:
			if (pastLaunch(&mAnim)) {
				mActStep = 2;
				dustBurst(&mBody.mPos);
				trailOn();
				seStartAtFar(0x133, &mBody.mPos);
			}
			setFrameKey(sPurpleBossFrameKeys[0]);
			break;
		case 2:
			setFrameKey(sPurpleBossFrameKeys[0]);
			if (motionCtrlIsDone(&mAnim)) {
				motionCtrlPlay(&mAnim, 3);
				mActStep  = 3;
				mFrameKey = 5;
			}
		case 3:
			if (fallMove(&mBody.mVel)) {
				mBossFlags |= BOSS_WALLED;
			}
			aimSpeed();
			seHolderPlay(&mSe, 0x135, &mBody.mPos);
			break;
		}
		calcPoints();
		break;
	case 2:
		closeHit();
		resetSpeed();
		trailOff();
		clearFrameKey();
		seHolderStop(&mSe);
		break;
	}
}

// 0x800CA018
f32 TEnePurpleBoss::brakeRate(f32 frame)
{
	if (frame >= 23.0f) {
		return 0.93f;
	}
	if (frame >= 28.0f) {
		return 0.75f;
	}
	return 0.98f;
}

// 0x800CA050
// Action 3 brakes throwing up ice
void TEnePurpleBoss::actBrake(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 4);
		closeHit();
		sprayOn();
		seStartAtFar(0x136, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		setFrameKey(sPurpleBossFrameKeys[1]);
		skateMove(brakeRate(motionCtrlFrame(&mAnim)));
		calcPoints();
		if (motionCtrlPassed(23.0f, &mAnim)) {
			particleOnceByIdDefault(&mNodes[1], 1, 0x48);
			particleOnceByIdDefault(&mNodes[2], 1, 0x48);
			trailOff();
		}
		break;
	case 2:
		closeHit();
		trailOff();
		break;
	}
}

// 0x800CA160
// Action 4 a short dash back
void TEnePurpleBoss::actDash(int phase)
{
	int* step = &mActStep;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 5);
		mFrameKey = 3;
		dashBack(1.0f);
		*step = 0;
		seStartAtFar(0x137, &mBody.mPos);
		seStartAtFar(0x138, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		dashStep(step);
		calcPoints();
		break;
	case 2:
		clearFrameKey();
		endDash();
		break;
	}
}

// 0x800CA230
// Action 5 dazed after running into the wall open to the eggs
void TEnePurpleBoss::actDazed(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 6);
		if (mSpark[0] != 0) {
			mSpark[0]->mFlags |= 1;
			mSpark[0] = 0;
		}
		mFrameKey = 3;
		mBossFlags |= BOSS_CLOSED | BOSS_DASH;
		colliOff();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		skateMove(0.8f);
		calcPoints();
		particleKeepAt((void**)&mSpark[0], (const Vec3f*)&mNodes[4], 1, 0x44);
		break;
	case 2:
		if (mSpark[0] != 0) {
			mSpark[0]->mFlags |= 1;
			mSpark[0] = 0;
		}
		clearFrameKey();
		mBossFlags &= ~(BOSS_CLOSED | BOSS_DASH);
		colliOn();
		break;
	}
}

// 0x800CA344
// Action 6 jumps round to face the middle and slams the ice which breaks every ice ball
void TEnePurpleBoss::actSlam(int phase)
{
	switch (phase) {
	case 0: {
		motionCtrlPlayBlend(0.1f, &mAnim, 7);
		int diff  = angleDiff(mBody.mRot.y,
		                        (int)(10430.38043493439 * (f32)atan2(-mBody.mPos.x, -mBody.mPos.z)));
		mTurnRate = (int)((f32)diff / 20.0f);
		break;
	}
	case 1: {
		f32 frame;

		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		setFrameKey(sPurpleBossFrameKeys[2]);
		frame = motionCtrlFrame(&mAnim);
		if (frame > 14.0f && frame < 20.0f) {
			mBody.mRot.y += mTurnRate;
		}
		skateMove(0.8f);
		calcPoints();
		if (motionCtrlPassed(12.0f, &mAnim)) {
			particleOnceByIdDefault(&mBody.mPos, 1, 0x49);
			seStartAtFar(0x132, &mBody.mPos);
		}
		if (motionCtrlPassed(22.0f, &mAnim)) {
			bigDustBurst(&mBody.mPos);
			pulseBossView(&mBody.mPos, 600.0f);
			breakAllIceBalls();
			seStartAtFar(0x133, &mBody.mPos);
		}
		break;
	}
	case 2:
		clearFrameKey();
		break;
	}
}

// 0x800CA4F0
// Action 7 freezes into the ice block it spins in
void TEnePurpleBoss::actFreeze(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 8);
		mBossFlags &= ~BOSS_SPINNING;
		mIceScale = 0.0f;
		seHolderStop(&mSe);
		if (mSpark[0] != 0) {
			mSpark[0]->mFlags |= 1;
			mSpark[0] = 0;
		}
		if (mSpark[1] != 0) {
			mSpark[1]->mFlags |= 1;
			mSpark[1] = 0;
		}
		break;
	case 1: {
		Vec3f at;
		f32 frame;

		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		skateMove(0.8f);
		calcPoints();
		setFrameKey(sPurpleBossFrameKeys[3]);
		frame = motionCtrlFrame(&mAnim);
		if (frame > 58.0f && frame <= 69.0f) {
			frame -= 58.0f;
			mIceScale = frame / 11.0f;
		}
		at.x = mBody.mPos.x;
		at.y = mBody.mPos.y;
		at.z = mBody.mPos.z;
		at.y += 40.0f;
		if (frame < 58.0f) {
			particleKeepAt((void**)&mSpark[0], &at, 1, 0x40);
			particleKeepAt((void**)&mSpark[1], &mBody.mPos, 1, 0x41);
		} else {
			if (mSpark[0] != 0) {
				mSpark[0]->mFlags |= 1;
				mSpark[0] = 0;
			}
			if (mSpark[1] != 0) {
				mSpark[1]->mFlags |= 1;
				mSpark[1] = 0;
			}
		}
		seHolderPlay(&mSe, 0x139, &mBody.mPos);
		break;
	}
	case 2:
		mBossFlags &= ~BOSS_SPINNING;
		mIceScale = 0.0f;
		if (mSpark[0] != 0) {
			mSpark[0]->mFlags |= 1;
			mSpark[0] = 0;
		}
		if (mSpark[1] != 0) {
			mSpark[1]->mFlags |= 1;
			mSpark[1] = 0;
		}
		clearFrameKey();
		seHolderStop(&mSe);
		break;
	}
}

// 0x800CA730
// Action 8 spins on the spot inside the ice block turning toward the player
void TEnePurpleBoss::actSpinUp(int phase)
{
	switch (phase) {
	case 0:
		playSpin();
		showIce();
		mFrameKey = 5;
		killSparks();
		clearSkid();
		seHolderStop(&mSe);
		mBody.mRot.x = 0;
		njXYZAngleToQuaternion((const s32*)&mBody.mRot.x, (NJS_QUATERNION*)mQuat);
		seStartAtFar(0x140, &mBody.mPos);
		break;
	case 1:
		updatePose();
		skateMove(0.8f);
		turnTo(mTargetAngle, 0x100);
		mBody.mRot.x += 0x1000;
		njXYZAngleToQuaternion((const s32*)&mBody.mRot.x, (NJS_QUATERNION*)mQuat);
		calcPoints();
		keepSparks(&mBody.mPos, mBody.mRot.y);
		skidMark(mBody.mRot.y);
		seHolderPlay(&mSe, 0x13A, &mBody.mPos);
		break;
	case 2:
		hideIce();
		mBody.mRot.x = 0;
		clearFrameKey();
		killSparks();
		seHolderStop(&mSe);
		break;
	}
}

// 0x800CA860
// Action 9 spins across the rink bouncing off the walls
void TEnePurpleBoss::actSpin(int phase)
{
	switch (phase) {
	case 0:
		playSpin();
		showIce();
		mFrameKey = 5;
		mBossFlags &= ~BOSS_BUMPED;
		njXYZAngleToQuaternion((const s32*)&mBody.mRot.x, (NJS_QUATERNION*)mQuat);
		njSinCos(mBody.mRot.y, &mBody.mVel.x, &mBody.mVel.z);
		mBody.mVel.x *= mSpinSpeed;
		mBody.mVel.z *= mSpinSpeed;
		trailOn();
		clearSkid();
		seHolderStop(&mSe);
		break;
	case 1:
		updatePose();
		spinMove();
		calcPoints();
		seHolderPlay(&mSe, 0x13B, &mBody.mPos);
		break;
	case 2:
		hideIce();
		mBody.mRot.y = (int)(10430.38043493439 * (f32)atan2(mBody.mVel.x, mBody.mVel.z));
		mBossFlags &= ~BOSS_BUMPED;
		trailOff();
		clearFrameKey();
		seHolderStop(&mSe);
		break;
	}
}

// 0x800CA99C
// Action 10 dashes back and leaves the ice block behind
void TEnePurpleBoss::actDrop(int phase)
{
	int* step = &mActStep;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 5);
		mFrameKey = 3;
		dashBack(-1.0f);
		*step = 0;
		seStartAtFar(0x13D, &mBody.mPos);
		seStartAtFar(0x138, &mBody.mPos);
		new TEnePurpleBoss_BigIceFragment(this, &mBody.mPos, mQuat, 40.0f);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		dashStep(step);
		calcPoints();
		break;
	case 2:
		clearFrameKey();
		endDash();
		break;
	}
}

// 0x800CAA94
// Action 11 hurt
void TEnePurpleBoss::actHurt(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 10);
		mEnemyFlags |= ENEMY_FLIPPING;
		mFrameKey = 3;
		mBossFlags |= BOSS_CLOSED | BOSS_DASH;
		colliOff();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		calcPoints();
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		clearFrameKey();
		mBossFlags &= ~(BOSS_CLOSED | BOSS_DASH);
		colliOn();
		break;
	}
}

// 0x800CAB64
// Action 12 flashes then goes back to the middle and melts away
void TEnePurpleBoss::actDie(int phase)
{
	switch (phase) {
	case 0:
		mActStep  = 0;
		mActTimer = 18;
		mTint.start();
		mEnemyFlags |= ENEMY_BLOCKED;
		colliDisable(mCollider);
		break;
	case 1: {
		f32 frame;

		if (mActStep == 0) {
			mActTimer--;
			if (mActTimer >= 0) {
				break;
			}
			mBody.mPos.x = 0.0f;
			mBody.mPos.y = 0.0f;
			mBody.mPos.z = 0.0f;
			mBody.mRot.x = 0;
			mBody.mRot.y = 0;
			mBody.mRot.z = 0;
			motionCtrlPlayBlend(0.2f, &mAnim, 11);
			mEnemyFlags |= ENEMY_INTANGIBLE | ENEMY_BLOCKED;
			colliDisable(mCollider);
			mFrameKey = 3;
			if (mSpark[0] != 0) {
				mSpark[0]->mFlags |= 1;
				mSpark[0] = 0;
			}
			if (mSpark[1] != 0) {
				mSpark[1]->mFlags |= 1;
				mSpark[1] = 0;
			}
			mBossFlags &= ~BOSS_MELT;
			shadowDisable(&mShadow);
			mActStep = 1;
			spawnFx(0, 0);
			seStartArg(0x2A9, 0x1B8);
			seCueSet(&mCue, 0x69, 0xA5);
			bossSuitSetFade(140.0f, 260.0f, &mSuit);
			bossSuitSetSink(200.0f, 400.0f, 70.0f, -20.0f, 10.0f, &mSuit);
			bossSuitSetFx(&mSuit, 1, 0x51, 0x50);
			breakAllIceBalls();
		}
		motionCtrlUpdate(&mAnim);
		updatePoseRooted();
		calcPoints();
		lockOn();
		frame = motionCtrlFrame(&mAnim);
		if (bossSuitUpdate(frame, &mSuit, &mBody.mPos, sPurpleBossModels[29])) {
			mBossFlags |= BOSS_MELT;
		} else {
			mBossFlags &= ~BOSS_MELT;
		}
		seCueUpdate(&mCue, (int)frame);
		if (motionCtrlIsDone(&mAnim)) {
			slain();
			sendBossDefeated(this);
			mFlags |= 1;
		}
		break;
	}
	case 2:
		mEnemyFlags &= ~(ENEMY_INTANGIBLE | ENEMY_BLOCKED);
		clearFrameKey();
		unlock();
		break;
	}
}

// 0x800CADF0
void TEnePurpleBoss::vfn_3C()
{
	njPrint((kEnemyInfoX << 16) | kEnemyInfoLine, ":HP %4d", mHealth);
}

// 0x800CAE2C
void TEnePurpleBoss::vfn_10()
{
	if (isAppearing()) {
		mFlags |= 0x10;
	} else {
		if (!mLight.isVisible(mDrawTask, 3000.0f, &mBody.mPos, 40.0f, 0)) {
			mFlags |= 0x10;
		} else {
			mFlags &= ~0x10;
			if (mBossFlags & BOSS_MELT) {
				moveDrawTask(mDrawTask, 0x22);
			}
		}
	}
}

// 0x800CAED4
void TEnePurpleBoss::render()
{
	if (mBossFlags & BOSS_MELT) {
		texFxSuitLoadTex(2);
	}
	njSetTexture(*sPurpleBossTextures);
	enemyEyeSet((NJS_TEXLIST*)*sPurpleBossTextures, 0, mFrameKey);
	if (mBossFlags & BOSS_MELT) {
		gjSetBlendOverride(1, 4, 5, 3);
		_nj_control_3d_flag_ |= 0x8000;
		texFxSuitBegin(0, *(u32*)&mSuit);
	}
	njPushMatrixEx();
	bodyMatrix();
	shadowAlphaBegin();
	if (mBossFlags & BOSS_MELT) {
		texFxSuitScroll(10.0f + mSuit.mHeight, mSuit.mHeight, 1);
		gjDrawActor(mPose);
	} else {
		ObjDrawState state;
		DrawStateSlot* slot = getDrawStateSlot();
		state = *(DrawState*)getDefaultDrawState();
		setDrawAmbient(&mLight, &state);
		mTint.addToLight(&state.mFlags);
		beginDrawState(slot, (const DrawState*)&state);
		gjDrawActor(mPose);
		if (mIceScale > 0.0f) {
			njPushMatrixEx();
			njTranslate(0, 0.0f, mIceY, 0.0f);
			njScale(0, mIceScale, mIceScale, mIceScale);
			gjDrawObject(sPurpleBossModels[30]);
			njPopMatrixEx();
		}
		endDrawState((DrawStateSlot*)slot);
	}
	shadowAlphaEnd();
	njPopMatrixEx();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}
