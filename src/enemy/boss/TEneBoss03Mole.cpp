#include "enemy/boss/TEneBoss03Mole.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "enemy/boss/BossParts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/boss/TEneBoss03Model.h"
#include "enemy/boss/TEneBoss03Dino.h"
#include "enemy/boss/TEneBoss03_EffBlur.h"
#include "engine/MotionPose.h"
#include "enemy/EnemyCommon.h"
#include "core/Pad.h"
#include "io/ResBank.h"
#include "enemy/EnemyHit.h"
#include "enemy/ObjEnemy.h"
#include "enemy/boss/TEneBoss03_VolRock.h"
#include "object/blue/TObjBlueCaptain.h"
#include "enemy/boss/TEneBoss03_Magma.h"
#include "enemy/boss/TEneBoss03_Beam_Pera.h"
#include "camera/TCameraCtrl.h"
#include "object/ObjAction.h"
#include "Ninja/ninja.h"
#include "enemy/EnemyTexFx.h"
#include "object/ObjShadow.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "engine/TParticle.h"
#include "enemy/EnemyFx.h"
#include "core/TMainTask.h"
#include "animal/RacerMover.h"
struct ColliShape;
struct GjActor;
struct ObjShadow;

extern "C" {
extern const int sMoleTrackNodes[]          = { 2, 36, 40, 44, 48, -1 };
const char* gTEneBoss03MoleName             = "TEneBoss03Mole";
extern const EnemyFrameKey sMoleFrameKey0[] = { { 12.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sMoleFrameKey1[] = { { 12.0f, 0 }, { 19.0f, 5 }, { -1.0f, 0 } };
extern const EnemyFrameKey sMoleFrameKey2[] = { { 18.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sMoleFrameKey3[] = { { 11.0f, 0 }, { 18.0f, 4 }, { -1.0f, 0 } };

MotionEntry gMoleMotionTable[] = {
	{ MOTION_LOOP, 12, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 }, { MOTION_LOOP, 13, -1, 0, 0.0f, 20.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 10, -1, 0, 0.0f, 20.0f, 0.5f, 0.0f, 0 }, { MOTION_ONCE, 7, -1, 0, 0.0f, 14.0f, 0.25f, 0.0f, 0 },
	{ MOTION_LOOP, 9, -1, 0, 0.0f, 20.0f, 0.2f, 0.0f, 0 },  { MOTION_ONCE, 8, -1, 0, 0.0f, 19.0f, 0.6f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 8.0f, 22.0f, 0.25f, 0.0f, 0 }, { MOTION_ONCE, 6, -1, 0, 0.0f, 34.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 10.0f, 0.3f, 0.0f, 0 },  { MOTION_LOOP, 5, -1, 0, 0.0f, 20.0f, 0.7f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 34.0f, 0.3f, 0.0f, 0 },  { MOTION_ONCE, 11, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 14, -1, 0, 0.0f, 34.0f, 0.3f, 0.0f, 0 }, { MOTION_ONCE, 2, -1, 0, 0.0f, 419.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 539.0f, 1.0f, 0.0f, 0 },
};
const EnemyFrameKey* sMoleFrameKeys[4]     = { sMoleFrameKey0, sMoleFrameKey1, sMoleFrameKey2, sMoleFrameKey3 };
extern const ColliShape sMoleAttackShape[] = {
	{ { 0.0f, 15.0f, 0.0f }, 0x0, 0, 0x0, 15.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
extern const ColliShape sMoleHurtShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 35.0f, 15.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};
}

// 0x8007FD70
extern "C" void initTEneBoss03Mole()
{
	resBankLoad(&gBoss03Bank, "ar_ene_red_boss.arc");
	resBankAddTextures(&gBoss03Bank, 1, "ene_red_boss_mole.gvm");
	resBankAddTextures(&gBoss03Bank, 0, "ene_red_boss_dino.gvm");
	resBankAddTextures(&gBoss03Bank, 2, "ene_red_boss_volrock.gvm");
	sMoleModels   = resBankGetModels(&gBoss03Bank);
	sMoleTextures = resBankGetTextures(&gBoss03Bank);
	sMoleMotions  = resBankGetMotion(&gBoss03Bank, 2);
	gMoleParams   = (MoleParams*)enemyParamGet(3);
	initBoss03Model();
	volRockLoad();
	initCaptainRing__Fv();
}

// 0x8007FE34
extern "C" void shutdownTEneBoss03Mole()
{
	freeCaptainRing__Fv();
	volRockFree();
	freeBoss03Model();
	gMoleParams = 0;
	resBankRemoveTextures(&gBoss03Bank, 2);
	resBankRemoveTextures(&gBoss03Bank, 0);
	resBankRemoveTextures(&gBoss03Bank, 1);
	resBankRelease(&gBoss03Bank);
	sMoleModels   = 0;
	sMoleTextures = 0;
	sMoleMotions  = 0;
}

// 0x8007FEB0
extern "C" void createBoss03Magma()
{
	new TEneBoss03_Magma(gTL_05);
}

// 0x8007FEE8
extern "C" NJS_OBJECT** modelTEneBoss03Mole()
{
	return sMoleModels;
}

// 0x8007FEF0
extern "C" NJS_TEXLIST** shapeTEneBoss03Mole()
{
	return sMoleTextures;
}

// 0x8007FEF8
extern "C" NJS_MOTION** motionTEneBoss03Mole(int index)
{
	return resBankGetMotion(&gBoss03Bank, index);
}

// 0x8007FF24
// Also the mole's dust puff
extern "C" void bindTEneBoss03Mole(const Vec3f* pos)
{
	particleOnceByIdDefault(pos, 1, 66);
	particleOnceByIdDefault(pos, 1, 67);
}

// 0x8007FF68
extern "C" TObject* createTEneBoss03Mole(TObject* parent, SetDesc* desc)
{
	return new TEneBoss03Mole(parent, desc);
}

// 0x8007FFBC
TEneBoss03Mole::TEneBoss03Mole(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00   = (u32)gTEneBoss03MoleName;
	mSize = 0x300;
	init(desc);
	initBossBar();
}

// 0x80080060
void TEneBoss03Mole::init(SetDesc* desc)
{
	motionCtrlInit(&mAnim, gMoleMotionTable, sMoleMotions);
	motionCtrlPlay(&mAnim, 0);
	mBody.mRot.x = 0;
	mHealth      = 500;
	setupColliders();
	mBrain = 0;
	readEnemyDesc(desc);
	mPose     = (GjActor*)allocPose(sMoleModels[1]);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mDino     = new TEneBoss03Dino(0, this);
	mDino->setPos(&mBody.mPos);
	mMoleFlags = 0;
	mPush.x    = 0.0f;
	mPush.y    = 0.0f;
	mPush.z    = 0.0f;
	mFrameKey  = 0;
	for (int i = 0; i < 5; i++) {
		mNodes[i].x = 0.0f;
		mNodes[i].y = 0.0f;
		mNodes[i].z = 0.0f;
	}
	mTarget.x  = 0.0f;
	mTarget.y  = 0.0f;
	mTarget.z  = 0.0f;
	mFrom.x    = 0.0f;
	mFrom.y    = 0.0f;
	mFrom.z    = 0.0f;
	mMaxHealth = 500;
	mHealth    = 500;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 30.0f;
	mShadow._34 = 30.0f;
	shadowDisable(&mShadow);
	mLight.reset();
	mTint.reset();
	mRate = 1.0f;
	mBlur = 0;
	_2A4  = 0;
	mDemo = 0;
	seCueClear(&mCue);
	seHolderStop(&mSe);
	mSe.setRange(700.0f);
	mStep   = 0;
	mTimer  = 0;
	mAngle  = 0;
	_2F0    = 0.0f;
	_2F4    = 0.0f;
	mFallAt = 0.0f;
	mFlight = 0.0f;
	createBrain();
	setBossView(10.0f, 33.0f, 150.0f);
}

// 0x80080260
void TEneBoss03Mole::setupColliders()
{
	mCollider = colliCreate(1, 1);
	colliCopyShapes(mCollider, (const ColliShape*)sMoleAttackShape);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0x30, 0, 0);
	colliShapeSetBit8(mCollider, 0);
	colliShapeSetTag(mCollider, 0, 0);
	mHurt = colliCreate(1, 1);
	colliCopyShapes(mHurt, (const ColliShape*)sMoleHurtShape);
	colliSetOwner(mHurt, (u16)mObjId);
	colliSetMasks(mHurt, 2, 0xFF, 0, 0);
	colliShapeSetBit8(mHurt, 0);
	colliShapeSetBit10(mHurt, 0);
	colliShapeSetTag(mHurt, 0, 1);
	collidersOff();
}

// 0x8008035C
TEneBoss03Mole::~TEneBoss03Mole()
{
	if (_2A4 != 0) {
		_2A4->mFlags |= 1;
		_2A4 = 0;
	}
	seHolderStop(&mSe);
	if (mDemo != 0) {
		mDemo->mFlags |= 1;
		mDemo = 0;
	}
	if (mDino != 0) {
		delete mDino;
		mDino = 0;
	}
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	unlock();
	removeDrawTask(mDrawTask);
	freePose((void**)&mPose);
	colliRetire(mHurt);
	colliRetire(mCollider);
	mHurt     = 0;
	mCollider = 0;
}

// 0x800804C4
// Slot _74
void TEneBoss03Mole::setPos(const Vec3f* pos)
{
	f32 x        = pos->x;
	f32 y        = pos->y;
	mBody.mPos.x = x;
	f32 z        = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x80080508
// Slot _40
f32 TEneBoss03Mole::vfn_40()
{
	return 50.0f;
}

// 0x80080510
// Slot _78 Both colliders sit on node 0
void TEneBoss03Mole::syncCollider()
{
	if (colliIsDisabled(mCollider) == 0) {
		colliShapeSetPos(colliGetShapes(mCollider), &mNodes[0]);
	}
	if (colliIsDisabled(mHurt) == 0) {
		colliShapeSetPos(colliGetShapes(mHurt), &mNodes[0]);
		colliRefreshBounds(mHurt);
		colliBoundsOn(mHurt);
	}
}

// 0x80080588
// Slot _38
void TEneBoss03Mole::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
}

// 0x800805A8
// Slot _A4
int TEneBoss03Mole::getDamage(EnemyHit* hit)
{
	if (hit->mKind == 1) {
		return 0;
	}
	return hit->mInfo.mDamage;
}

// 0x800805C4
// Slot _A8
int TEneBoss03Mole::getHitKind(const void* touch, HitInfo* info)
{
	int kind = 1;
	if (mMoleFlags & MOLE_GUARD) {
		kind = 3;
	}
	return kind;
}

// 0x800805DC
// Slot _58 Players hurt it while it is exposed or always with arg 1
int TEneBoss03Mole::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit       = 0;

	if (isPlayerObj(other)) {
		info->mFlags.clear();
		info->mPos.z = info->mPos.y = info->mPos.x = 0.0f;
		if (arg == 0) {
			if (mMoleFlags & MOLE_EXPOSED) {
				hit           = 1;
				info->mDamage = 10;
			}
		} else if (arg == 1) {
			hit           = 1;
			info->mDamage = 20;
		}
	} else if (isEggObj(other)) {
		if (arg == 1) {
			hit = 1;
			info->mFlags.clear();
			info->mPos.z = info->mPos.y = info->mPos.x = 0.0f;
			info->mDamage                              = 1;
			info->mFlags.setBit(0xD);
		}
	} else if (other != this && isEnemyObj(other)) {
		if (arg == 1 || (mMoleFlags & MOLE_OFF_DINO)) {
			hit = 1;
			info->mFlags.clear();
			info->mPos.z = info->mPos.y = info->mPos.x = 0.0f;
			info->mDamage                              = 0;
		}
	}
	return hit;
}

// 0x80080718
// The frame of an action spent on the dinosaur's head
void TEneBoss03Mole::rideDino()
{
	updateDino();
	motionCtrlUpdate(&mAnim);
	followDino();
	pose();
}

// 0x8008075C
void TEneBoss03Mole::burstRocks(int count)
{
	Vec3f pos;
	getDinoPos(&pos);
	pos.y = 180.0f;
	createVolRockBurst(this, &pos, count);
}

// 0x800807A8
void TEneBoss03Mole::updateShadow()
{
	Vec3f ground;
	Vec3f normal;
	if (probeGround(&mBody.mPos, &ground, &normal) != 0) {
		colliEnable(mCollider);
		shadowSetGround(&mShadow, &ground, &normal);
		shadowEnable(&mShadow);
	} else {
		colliDisable(mCollider);
		shadowDisable(&mShadow);
	}
}

// 0x8008081C
// Turns towards mTarget and returns how far it still had to turn
int TEneBoss03Mole::turnToTarget(int rate, Vec3f* dir)
{
	int left;
	dir->x = mTarget.x;
	dir->y = mTarget.y;
	dir->z = mTarget.z;
	njSubVector(dir, &mBody.mPos);
	vecNormalizeXZ(dir, dir);
	mBody.mRot.y = angleApproachDiff(mBody.mRot.y, (int)(10430.38043493439 * (f32)atan2(dir->x, dir->z)), 0x200, &left);
	return left;
}

// 0x800808B4
// Speeds up while facing mTarget slows down while turned away
void TEneBoss03Mole::moveToTarget(f32 maxSpeed, f32 accel, int rate)
{
	Vec3f dir;
	f32 speed;
	int turn = turnToTarget(rate, &dir);
	speed    = mBody.mVel.z;
	if (turn > 0x4000) {
		speed -= accel;
		if (speed < 0.0f) {
			speed = 0.0f;
		}
	} else {
		speed += accel;
		if (speed > maxSpeed) {
			speed = maxSpeed;
		}
	}
	vecFromAngleXZ(speed, &dir, mBody.mRot.y);
	getPush(&dir);
	collideFiltered(15.0f, &mBody.mPos, &dir, &mShadow);
	mBody.mVel.z = vecLenXZ(&dir);
}

// A step scaled by the frame rate
static inline f32 rateScaled(f32 k, f32 rate)
{
	return k * rate;
}

// 0x8008097C
void TEneBoss03Mole::chaseTarget()
{
	f32 turn = rateScaled(768.0f, mRate);
	moveToTarget(2.0f * mRate, 0.1f * mRate, (int)turn);
}

// 0x800809C4
// Slides on slowing down but not below minSpeed
void TEneBoss03Mole::slowDown(f32 minSpeed)
{
	Vec3f dir;
	f32 drag  = 0.97f - 0.02f * mRate;
	f32 speed = mBody.mVel.z;
	if (drag < 0.0f) {
		drag = 0.0f;
	}
	speed *= drag;
	if (speed < minSpeed) {
		speed = minSpeed;
	}
	vecFromAngleXZ(speed, &dir, mBody.mRot.y);
	mBody.mVel.z = speed;
	getPush(&dir);
	collideFiltered(15.0f, &mBody.mPos, &dir, &mShadow);
}

// 0x80080A68
void TEneBoss03Mole::collidersOff()
{
	colliDisable(mCollider);
	colliDisable(mHurt);
}

// 0x80080AA0
void TEneBoss03Mole::collidersOn()
{
	colliEnable(mCollider);
	colliEnable(mHurt);
}

// 0x80080AD8
void TEneBoss03Mole::attackOnly()
{
	colliEnable(mCollider);
	colliDisable(mHurt);
}

// 0x80080B10
// Whether a volcano rock is just ahead
int TEneBoss03Mole::nearRock()
{
	Vec3f pos;
	njSinCos(mBody.mRot.y, &pos.x, &pos.z);
	pos.y = 0.0f;
	gjScaleVector(2.0f * mRate + 15.0f, &pos, &pos);
	njAddVector(&pos, &mBody.mPos);
	return volRockNear(&pos, 13.5f);
}

// 0x80080B84
void TEneBoss03Mole::biteFx()
{
	Vec3f pos;
	mDino->getMouthPos(&pos);
	createTEneBoss03_Beam_Pera(this, &pos);
	seStartAtFar(0x123, &pos);
}

// 0x80080BD0
void TEneBoss03Mole::startFx()
{
}

// 0x80080BD4
void TEneBoss03Mole::endFx()
{
}

// 0x80080BD8
void TEneBoss03Mole::killBlur()
{
	if (mBlur == 0) {
		return;
	}
	mBlur->mFlags |= 1;
	mBlur = 0;
}

// 0x80080BFC
void TEneBoss03Mole::updateBlur()
{
	Vec3f pos;
	getDinoMouth(&pos);
	if (mBlur == 0) {
		mBlur = createTEneBoss03_EffBlur(this, &pos);
	} else {
		mBlur->setPos(&pos);
	}
}

// 0x80080C54
void TEneBoss03Mole::endBlur()
{
	if (mBlur != 0) {
		mBlur->end();
	}
}

// 0x80080C80
void TEneBoss03Mole::setTarget(Vec3f* target)
{
	mTarget.x = target->x;
	mTarget.y = target->y;
	mTarget.z = target->z;
}

// 0x80080C9C
void TEneBoss03Mole::getDinoPos(Vec3f* out)
{
	Vec3f* pos = mDino->getPos();
	out->x     = pos->x;
	out->y     = pos->y;
	out->z     = pos->z;
}

// 0x80080CF0
void TEneBoss03Mole::setDinoRate(f32 rate)
{
	mDino->setMotionRate(rate);
}

// 0x80080D14
void TEneBoss03Mole::setMaxHealth(int health)
{
	mMaxHealth = health;
	mHealth    = health;
}

// 0x80080D20
void TEneBoss03Mole::flinch()
{
	enemyFxSparks(3.0f, &mContact.mSurface, &mContact.mCenter);
	seStartAt(0x11E, &mBody.mPos);
}

// 0x80080D64
void TEneBoss03Mole::knockOut()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
	hideBossBar();
	seStartAtFar(299, &mBody.mPos);
}

// 0x80080DB4
void TEneBoss03Mole::setGuard()
{
	mMoleFlags |= MOLE_GUARD;
}

// 0x80080DC4
void TEneBoss03Mole::clearGuard()
{
	mMoleFlags &= ~MOLE_GUARD;
}

// 0x80080DD4
int TEneBoss03Mole::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

// 0x80080DF8
int TEneBoss03Mole::dinoAtRing()
{
	return mDino->atRing();
}

// 0x80080E1C
int TEneBoss03Mole::dinoFaceInDone()
{
	return mDino->isFaceInDone();
}

// 0x80080E40
int TEneBoss03Mole::dinoFaceSideDone()
{
	return mDino->isFaceSideDone();
}

// 0x80080E64
int TEneBoss03Mole::dinoRoarDone()
{
	return mDino->isRoarDone();
}

// 0x80080E88
int TEneBoss03Mole::dinoRoarPeaked()
{
	return mDino->roarPeaked();
}

// 0x80080EAC
void TEneBoss03Mole::getDinoNode0(Vec3f* out)
{
	mDino->getNode0Pos(out);
}

// 0x80080ED0
int TEneBoss03Mole::dinoBreathing()
{
	return mDino->isBreathing();
}

// 0x80080EF4
int TEneBoss03Mole::dinoBreathEndDone()
{
	return mDino->isBreathEndDone();
}

// 0x80080F18
int TEneBoss03Mole::dinoSwipeDone()
{
	return mDino->isSwipeDone();
}

// 0x80080F3C
int TEneBoss03Mole::dinoBiteDone()
{
	return mDino->isBiteDone();
}

// 0x80080F60
int TEneBoss03Mole::hasLanded()
{
	return mMoleFlags & MOLE_LANDED;
}

// 0x80080F6C
int TEneBoss03Mole::isDiveDone()
{
	return isActionDone(ACT_DIVE);
}

// 0x80080F90
void TEneBoss03Mole::setRate(f32 rate)
{
	mRate = rate;
}

// 0x80080F98
void TEneBoss03Mole::getDinoMouth(Vec3f* out)
{
	mDino->getMouthPos(out);
}

// 0x80080FBC
int TEneBoss03Mole::isAtRock()
{
	return mMoleFlags & MOLE_AT_ROCK;
}

// 0x80080FC8
int TEneBoss03Mole::isPounceDone()
{
	return isActionDone(ACT_POUNCE);
}

// 0x80080FEC
int TEneBoss03Mole::isSurfaceDone()
{
	return isActionDone(ACT_SURFACE);
}

// 0x80081010
int TEneBoss03Mole::isDownDone()
{
	return isActionDone(ACT_DOWN);
}

// 0x80081034
// Slot _70
// The offset from b to a returned by value
static inline Vec3f moleOffset(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjSubVector(&r, a, b);
	return r;
}

void TEneBoss03Mole::updateBody()
{
	if (isBlocked() == 0 && takeHits(&mHit, &mContact) != 0) {
		mPush = moleOffset(&mBody.mPos, &mContact.mPos);
		if (enemyHitTestFlag(&mHit, 0xB) == 0) {
			applyDamage(&mHit);
		}
	}
	mBrain->update(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	mTint.update();
	syncCollider();
	setBossBar(mMaxHealth, mHealth);
}

// 0x80081150
// Slot _6C With button 0x400 held the dinosaur is put where the mole is
void TEneBoss03Mole::vfn_6C()
{
	if (returnZero() != 0) {
		trackBegin();
		gjPushUnitMatrix();
		modelMatrix();
		poseFirstFrame(sMoleMotions, 0xF, mPose);
		njPopMatrixEx();
		trackEnd();
		syncCollider();
		if (padGet(0)->mPressed & 0x400) {
			mDino->setPos(&mBody.mPos);
			mDino->mBody.mRot.y = mBody.mRot.y;
		}
		mDino->vfn_6C();
	}
}

// 0x80081214
void TEneBoss03Mole::createBrain()
{
	mBrain = createMoleBrain(this);
}

// 0x80081244
void TEneBoss03Mole::trackBegin()
{
	nodeTrackBeginOrigins(sMoleTrackNodes, mNodes);
}

// 0x80081274
// The node positions come out in model space
void TEneBoss03Mole::trackEnd()
{
	nodeTrackEnd();
	gjPushUnitMatrix();
	bodyMatrix();
	for (int i = 0; i < 5; i++) {
		njCalcPointEx(&mNodes[i], &mNodes[i]);
	}
	njPopMatrixEx();
}

// 0x800812D8
void TEneBoss03Mole::pose()
{
	trackBegin();
	gjPushUnitMatrix();
	modelMatrix();
	applyPose(mPose, &mAnim);
	njPopMatrixEx();
	trackEnd();
}

// 0x8008132C
// Takes the body's position from the motion's root node then poses about it
void TEneBoss03Mole::poseFromRoot()
{
	MotionNodePose root;
	NJS_MOTION* motion = motionCtrlGetMotion(&mAnim);
	motionGetNodePose(&root, *(NJS_OBJECT**)mPose, motion, motionCtrlFrame(&mAnim));
	motionPoseGetPos(&root, &mBody.mPos);
	trackBegin();
	gjPushUnitMatrix();
	gjTranslate(-mBody.mPos.x, -mBody.mPos.y, -mBody.mPos.z);
	modelMatrix();
	applyPose(mPose, &mAnim);
	njPopMatrixEx();
	trackEnd();
}

// 0x800813D8
void TEneBoss03Mole::setDinoAction(int action)
{
	mDino->setAction(action);
}

// 0x800813FC
void TEneBoss03Mole::updateDino()
{
	mDino->draw();
}

// 0x8008142C
void TEneBoss03Mole::setDinoRing(int action)
{
	f32 radius = getDinoRingRadius();
	if (action == ACT_WALK) {
		radius += 200.0f;
	} else if (action == ACT_WALK_OUT) {
		radius += 300.0f;
	}
	mDino->setRingRadius(radius);
}

// 0x80081488
void TEneBoss03Mole::followDino()
{
	mDino->getAim(&mBody.mPos, &mBody.mRot.x);
}

// 0x800814B8
void TEneBoss03Mole::turnWithDino()
{
	int rot[3];
	mDino->getAim(&mBody.mPos, rot);
	mBody.mRot.x = angleApproach(mBody.mRot.x, rot[0], 0x80);
	mBody.mRot.y = angleApproach(mBody.mRot.y, rot[1], 0x80);
	mBody.mRot.z = angleApproach(mBody.mRot.z, rot[2], 0x80);
}

// 0x8008152C
// The cut-in as it appears 1 1 and as it dies 0 0
void TEneBoss03Mole::startDemo(int model, int motion)
{
	void* obj            = resBankGetObjects(&gBoss03Bank)[model];
	NJS_MOTION** motions = resBankGetMotion(&gBoss03Bank, 0);
	mDemo                = createCamPath(0, 3, obj, motions[motion], 0, 1.0f, 1.0f);
	if (mDemo != 0) {
		((TCamera*)mDemo)->setCut();
	}
}

// 0x800815BC
// Slot _7C
void TEneBoss03Mole::onActionPhase(int action, int phase)
{
	switch (action) {
	case ACT_APPEAR:
		actAppear(phase);
		break;
	case ACT_WALK:
	case ACT_WALK_IN:
	case ACT_WALK_OUT:
		actWalk(phase, action);
		break;
	case ACT_FACE_IN:
	case ACT_FACE_SIDE:
		actFace(phase, action);
		break;
	case ACT_ROAR:
		actRoar(phase);
		break;
	case ACT_BREATH:
		actBreath(phase);
		break;
	case ACT_BREATH_END:
		actBreathEnd(phase);
		break;
	case ACT_SWIPE:
		actSwipe(phase);
		break;
	case ACT_BITE:
		actBite(phase);
		break;
	case ACT_IDLE:
		actIdle(phase);
		break;
	case ACT_LEAP:
		actLeap(phase);
		break;
	case ACT_DIVE:
		actDive(phase);
		break;
	case ACT_DIG:
		actDig(phase);
		break;
	case ACT_POUNCE:
		actPounce(phase);
		break;
	case ACT_SURFACE:
		actSurface(phase);
		break;
	case ACT_DOWN:
		actDown(phase);
		break;
	case ACT_DIE:
		actDie(phase);
		break;
	}
}

// 0x800816C0
// Rises out of the volcano on the dinosaur's head with the cut-in playing
void TEneBoss03Mole::actAppear(int phase)
{
	switch (phase) {
	case 0:
		setDinoAction(0);
		setCantAct();
		clearFrameKey();
		mBody.mPos.z = mBody.mPos.y = mBody.mPos.x = 0.0f;
		mBody.mRot.x                               = 0;
		mBody.mRot.y                               = 0;
		mBody.mRot.z                               = 0;
		motionCtrlPlay(&mAnim, 0xE);
		killBlur();
		mStep = 0;
		break;
	case 1: {
		updateDino();
		if (mDino->isNeckDown()) {
			updateBlur();
		} else {
			endBlur();
		}
		if (mStep == 0) {
			mStep = 1;
			startDemo(1, 1);
			seStartArg(0x2A6, 0x21C);
		}
		lockOn();
		motionCtrlUpdate(&mAnim);
		waitMotion();
		poseFromRoot();
		f32 frame = motionCtrlFrame(&mAnim);
		Vec3f pos;
		switch (mStep) {
		case 1:
			if (frame >= 10.0f) {
				mStep = 2;
				pos   = mBody.mPos;
				pos.y = 200.0f;
				particleGenTimedByIdDefault(&pos, 1, 0x48, 0, 0x37);
				particleGenTimedByIdDefault(&pos, 1, 0x50, 0, 0x37);
			}
			break;
		case 2:
			if (frame >= 65.0f) {
				mStep = 3;
				pos   = mBody.mPos;
				pos.y = 200.0f;
				bindTEneBoss03Mole(&pos);
			}
			break;
		case 3:
			if (frame >= 86.0f) {
				mStep = 4;
				particleOnceByIdDefault(&mBody.mPos, 1, 0x4A);
			}
			break;
		}
		break;
	}
	case 2:
		killBlur();
		clearCantAct();
		unlock();
		if (mDemo != 0) {
			mDemo->mFlags |= 1;
			mDemo = 0;
		}
		camSetLookFrom(0, &mBody.mPos);
		break;
	}
}

// 0x80081928
void TEneBoss03Mole::actWalk(int phase, int action)
{
	switch (phase) {
	case 0:
		setDinoRing(action);
		setDinoAction(2);
		motionCtrlPlay(&mAnim, 1);
		clearFrameKey();
		break;
	case 1:
		rideDino();
		break;
	case 2:
		break;
	}
}

// 0x8008199C
void TEneBoss03Mole::actFace(int phase, int action)
{
	switch (phase) {
	case 0:
		if (action == ACT_FACE_IN) {
			setDinoAction(3);
		} else {
			setDinoAction(4);
		}
		motionCtrlPlay(&mAnim, 1);
		clearFrameKey();
		break;
	case 1:
		rideDino();
		break;
	case 2:
		break;
	}
}

// 0x80081A18
void TEneBoss03Mole::actRoar(int phase)
{
	switch (phase) {
	case 0:
		setDinoAction(5);
		motionCtrlPlay(&mAnim, 1);
		clearFrameKey();
		break;
	case 1:
		rideDino();
		break;
	case 2:
		break;
	}
}

// 0x80081A80
void TEneBoss03Mole::actBreath(int phase)
{
	switch (phase) {
	case 0:
		setDinoAction(6);
		motionCtrlPlay(&mAnim, 1);
		clearFrameKey();
		break;
	case 1:
		rideDino();
		break;
	case 2:
		break;
	}
}

// 0x80081AE8
void TEneBoss03Mole::actBreathEnd(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 1);
		setDinoAction(7);
		clearFrameKey();
		break;
	case 1:
		rideDino();
		break;
	case 2:
		break;
	}
}

// 0x80081B54
// The swipe lights the rocks it passes with the blur on its mouth while it lasts
void TEneBoss03Mole::actSwipe(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 1);
		setDinoAction(8);
		clearFrameKey();
		mStep = 0;
		break;
	case 1:
		rideDino();
		switch (mStep) {
		case 0:
			if (mDino->swipeHits()) {
				Vec3f pos;
				getDinoMouth(&pos);
				lightVolRocksFrom(&pos);
				mStep = 1;
			}
			break;
		case 1:
			updateBlur();
			if (!mDino->swipeHits()) {
				endBlur();
				mStep = 2;
			}
			break;
		}
		break;
	case 2:
		killBlur();
		lightVolRocksInOrder();
		break;
	}
}

// 0x80081C50
void TEneBoss03Mole::actBite(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 1);
		setDinoAction(9);
		clearFrameKey();
		break;
	case 1:
		rideDino();
		if (mDino->biteHits()) {
			biteFx();
		}
		break;
	case 2:
		break;
	}
}

// 0x80081CD4
void TEneBoss03Mole::actIdle(int phase)
{
	switch (phase) {
	case 0:
		setDinoAction(1);
		motionCtrlPlay(&mAnim, 7);
		clearFrameKey();
		break;
	case 1:
		rideDino();
		setFrameKey(sMoleFrameKeys[3]);
		waitMotion();
		break;
	case 2:
		clearFrameKey();
		break;
	}
}

// 0x80081D68
// Jumps from the dinosaur's head to mTarget flipping over on the way and lands in the ground
void TEneBoss03Mole::actLeap(int phase)
{
	switch (phase) {
	case 0: {
		attackOnly();
		motionCtrlPlay(&mAnim, 8);
		mBody.mRot.z = 0;
		_2F0         = 0.0f;
		mStep        = 0;
		mMoleFlags &= ~MOLE_LANDED;
		mMoleFlags |= MOLE_OFF_DINO;
		mFrameKey = 3;
		Vec3f vel;
		f32 rise;
		f32 gravity;
		f32 time = flatVelocityTo(&mBody.mPos, &mTarget, &vel, 3.0f);
		ballisticSolve(time, 80.0f, mTarget.y - mBody.mPos.y, &rise, &gravity);
		mFrom.x        = mBody.mPos.x;
		mFrom.y        = mBody.mPos.y;
		mFrom.z        = mBody.mPos.z;
		mBody.mVel.x   = vel.x;
		mBody.mVel.y   = rise;
		mBody.mVel.z   = vel.z;
		mBody.mForce.y = gravity;
		mAngle         = mBody.mRot.x;
		_2F4           = (0x28000 - (int)mAngle) / (1.5f + time);
		seStartAtFar(0x124, &mBody.mPos);
		break;
	}
	case 1:
		updateDino();
		motionCtrlUpdate(&mAnim);
		switch (mStep) {
		case 0:
			_2F0 += 1.0f;
			parabolaPoint(&mFrom, &mBody.mVel, &mBody.mPos, mBody.mForce.y, _2F0);
			mBody.mRot.x = (int)mAngle + (int)(_2F4 * _2F0);
			if (mBody.mPos.y < mTarget.y) {
				mBody.mPos.x = mTarget.x;
				mBody.mPos.y = mTarget.y;
				mBody.mPos.z = mTarget.z;
				mTarget.x += mBody.mVel.x;
				mTarget.z += mBody.mVel.z;
				mStep = 1;
				_2F0  = 0.0f;
				motionCtrlPlay(&mAnim, 9);
				mMoleFlags |= MOLE_LANDED;
				startFx();
				bindTEneBoss03Mole(&mBody.mPos);
				seStartAtFar(0x125, &mBody.mPos);
			}
			mBody.mPos.y += 10.0f;
			break;
		case 1:
			mBody.mRot.x = 0;
			mBody.mPos.x = mTarget.x;
			mBody.mPos.y = mTarget.y;
			mBody.mPos.z = mTarget.z;
			break;
		}
		pose();
		updateShadow();
		break;
	case 2:
		mMoleFlags &= ~(MOLE_LANDED | MOLE_OFF_DINO);
		shadowDisable(&mShadow);
		collidersOff();
		endFx();
		clearFrameKey();
		break;
	}
}

// 0x8008203C
void TEneBoss03Mole::actDive(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0xC);
		mBody.mPos.x = mTarget.x;
		mBody.mPos.y = mTarget.y;
		mBody.mPos.z = mTarget.z;
		mBody.mRot.x = 0;
		mBody.mRot.z = 0;
		attackOnly();
		startFx();
		break;
	case 1:
		updateDino();
		motionCtrlUpdate(&mAnim);
		waitMotion();
		pose();
		updateShadow();
		setFrameKey(sMoleFrameKeys[2]);
		break;
	case 2:
		shadowDisable(&mShadow);
		collidersOff();
		endFx();
		clearFrameKey();
		break;
	}
}

// 0x8008212C
// Digs towards mTarget marking when it has been by a rock long enough
void TEneBoss03Mole::actDig(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 2);
		motionCtrlSetSpeed(&mAnim, mRate);
		clearFrameKey();
		mBody.mVel.z = 0.0f;
		mBody.mRot.x = 0;
		mBody.mRot.z = 0;
		attackOnly();
		startFx();
		mMoleFlags |= MOLE_AT_ROCK | MOLE_EXPOSED;
		_2F0 = 0.0f;
		break;
	case 1:
		updateDino();
		motionCtrlUpdate(&mAnim);
		chaseTarget();
		mMoleFlags &= ~MOLE_AT_ROCK;
		if (nearRock() != 0) {
			_2F0 += mRate;
			if (_2F0 >= 24.0f) {
				mMoleFlags |= MOLE_AT_ROCK;
			}
		} else {
			_2F0 = 0.0f;
		}
		seHolderPlay(&mSe, 0x126, &mBody.mPos);
		pose();
		updateShadow();
		break;
	case 2:
		mMoleFlags &= ~(MOLE_AT_ROCK | MOLE_EXPOSED);
		shadowDisable(&mShadow);
		motionCtrlSetSpeed(&mAnim, 1.0f);
		collidersOff();
		endFx();
		seHolderStop(&mSe);
		break;
	}
}

// 0x800822A4
// Slows to a stop then jumps in an arc back onto the dinosaur's head turning to its heading
void TEneBoss03Mole::actPounce(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 3);
		clearFrameKey();
		attackOnly();
		mMoleFlags |= MOLE_EXPOSED;
		mStep = 0;
		break;
	case 1:
		updateDino();
		motionCtrlUpdate(&mAnim);
		switch (mStep) {
		case 0:
			slowDown(0.0f);
			if (motionCtrlIsDone(&mAnim)) {
				int rot[3];
				mDino->getAim(&mTarget, rot);
				mBody.mVel.x = mTarget.x;
				mBody.mVel.y = mTarget.y;
				mBody.mVel.z = mTarget.z;
				njSubVector(&mBody.mVel, &mBody.mPos);
				mBody.mVel.y = 0.0f;
				f32 dist     = njUnitVector(&mBody.mVel);
				if (dist < gUnitVectorMin) {
					mBody.mVel.x = gVecZero.x;
					mBody.mVel.y = gVecZero.y;
					mBody.mVel.z = gVecZero.z;
				}
				gjScaleVector(dist / 60.0f, &mBody.mVel, &mBody.mVel);
				f32 rise = mTarget.y - mBody.mPos.y;
				ballisticSolve(60.0f, 35.0f + rise, rise, &mBody.mVel.y, &mBody.mForce.y);
				mFrom.x  = mBody.mPos.x;
				mFrom.y  = mBody.mPos.y;
				mFrom.z  = mBody.mPos.z;
				int turn = angleDiff(mBody.mRot.y, rot[1]);
				if (turn < 0) {
					turn -= 0x10000;
				} else {
					turn += 0x10000;
				}
				mAngle  = mBody.mRot.y;
				_2F0    = turn / 60.0f;
				mFallAt = 10.0f;
				mFallAt += 5.0f;
				mFallAt = 60.0f - mFallAt;
				mStep   = 1;
				_2F4    = 0.0f;
				mFlight = 60.0f;
				motionCtrlPlayBlend(0.1f, &mAnim, 4);
			}
			break;
		case 1: {
			_2F4 += 1.0f;
			if (_2F4 >= mFallAt) {
				motionCtrlPlayBlend(0.1f, &mAnim, 5);
				mFallAt = 2.0f * mFlight;
			}
			f32 lastY = mBody.mPos.y;
			parabolaPoint(&mFrom, &mBody.mVel, &mBody.mPos, mBody.mForce.y, _2F4);
			f32 dy       = mBody.mPos.y - lastY;
			mBody.mRot.y = (int)mAngle + (int)(_2F0 * _2F4);
			if (dy < 0.0f) {
				mDino->getHeadPos(&mTarget);
				if (mTarget.y >= mBody.mPos.y) {
					mBody.mPos.x = mTarget.x;
					mBody.mPos.y = mTarget.y;
					mBody.mPos.z = mTarget.z;
					mStep        = 2;
				}
			}
			break;
		}
		case 2:
			turnWithDino();
			if (motionCtrlIsDone(&mAnim)) {
				motionCtrlPlay(&mAnim, 0xB);
				mStep = 3;
				_2F4  = 0.0f;
			}
			break;
		case 3:
			followDino();
			waitMotion();
			break;
		}
		pose();
		updateShadow();
		break;
	case 2:
		shadowDisable(&mShadow);
		collidersOff();
		mMoleFlags &= ~MOLE_EXPOSED;
		break;
	}
}

// 0x80082638
// Bursts up out of the ground
void TEneBoss03Mole::actSurface(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f * mRate, &mAnim, 6);
		motionCtrlSetSpeed(&mAnim, mRate);
		attackOnly();
		mMoleFlags |= MOLE_EXPOSED;
		startFx();
		clearFrameKey();
		break;
	case 1: {
		updateDino();
		motionCtrlUpdate(&mAnim);
		slowDown(0.2f);
		f32 frame = motionCtrlFrame(&mAnim);
		if (12.0f <= frame && frame < 15.0f) {
			collidersOn();
		} else {
			attackOnly();
		}
		if (motionCtrlPassed(12.0f, &mAnim)) {
			seStartAtFar(0x127, &mBody.mPos);
		}
		waitMotion();
		pose();
		if (motionCtrlPassed(12.0f, &mAnim)) {
			Vec3f pos;
			pos.x = mBody.mPos.x;
			pos.y = mBody.mPos.y;
			pos.z = mBody.mPos.z;
			pos.y = mNodes[4].y - 2.0f;
			particleOnceByIdDefault(&pos, 1, 0x46);
		}
		setFrameKey(sMoleFrameKeys[1]);
		break;
	}
	case 2:
		collidersOff();
		endFx();
		clearFrameKey();
		mMoleFlags &= ~MOLE_EXPOSED;
		break;
	}
}

// 0x800827D4
// Knocked out of the ground and flipped over
void TEneBoss03Mole::actDown(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f * mRate, &mAnim, 10);
		motionCtrlSetSpeed(&mAnim, mRate);
		mEnemyFlags |= ENEMY_FLIPPING;
		mMoleFlags |= MOLE_OFF_DINO;
		knockOut();
		attackOnly();
		startFx();
		clearFrameKey();
		break;
	case 1:
		updateDino();
		motionCtrlUpdate(&mAnim);
		if (motionCtrlFrame(&mAnim) >= 17.0f) {
			mFrameKey = 0;
		}
		slowDown(0.0f);
		waitMotion();
		pose();
		if (motionCtrlPassed(4.0f, &mAnim)) {
			Vec3f pos;
			pos.x = mNodes[0].x;
			pos.y = mNodes[0].y;
			pos.z = mNodes[0].z;
			pos.y = mBody.mPos.y;
			particleOnceByIdDefault(&pos, 0, 0x46);
			seStartAtFar(0x128, &pos);
		}
		setFrameKey(sMoleFrameKeys[0]);
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		mMoleFlags &= ~MOLE_OFF_DINO;
		mFrameKey = 0;
		collidersOff();
		endFx();
		clearFrameKey();
		break;
	}
}

// 0x80082964
// Waits 18 frames then plays the death cut-in with the suit of crows and ends the boss
void TEneBoss03Mole::actDie(int phase)
{
	switch (phase) {
	case 0:
		mStep  = 0;
		mTimer = 0x12;
		mTint.start();
		mEnemyFlags |= ENEMY_BLOCKED;
		colliDisable(mCollider);
		break;
	case 1: {
		updateDino();
		if (mStep == 0) {
			mTimer--;
			if (mTimer >= 0) {
				return;
			}
			setDinoAction(10);
			mBody.mPos.x = 0.0f;
			mBody.mPos.y = 0.0f;
			mBody.mPos.z = 0.0f;
			mBody.mRot.x = 0;
			mBody.mRot.y = 0;
			mBody.mRot.z = 0;
			motionCtrlPlay(&mAnim, 0xD);
			mMoleFlags &= ~MOLE_SUIT;
			mFrameKey = 3;
			shadowDisable(&mShadow);
			mStep = 1;
			startDemo(0, 0);
			seStartArg(0x2A7, 0x1E0);
			seCueSet(&mCue, 0x69, 0xA5);
			bossSuitSetFade(200.0f, 280.0f, &mSuit);
			bossSuitSetSink(240.0f, 380.0f, 20.0f, -5.0f, 5.0f, &mSuit);
			bossSuitSetFx(&mSuit, 1, 0x54, 0x53);
		}
		motionCtrlUpdate(&mAnim);
		poseFromRoot();
		lockOn();
		f32 frame = motionCtrlFrame(&mAnim);
		switch (mStep) {
		case 1:
			if (frame >= 50.0f) {
				mStep = 2;
				Vec3f pos;
				pos.x = mBody.mPos.x;
				pos.y = mBody.mPos.y;
				pos.z = mBody.mPos.z;
				pos.y = 200.0f;
				bindTEneBoss03Mole(&pos);
			}
			break;
		case 2:
			if (frame >= 155.0f) {
				mStep = 3;
				particleOnceByIdDefault(&mBody.mPos, 1, 0x4A);
			}
			break;
		case 3:
			if (bossSuitUpdate(frame, &mSuit, &mBody.mPos, sMoleModels[0x12])) {
				mMoleFlags |= MOLE_SUIT;
			} else {
				mMoleFlags &= ~MOLE_SUIT;
			}
			break;
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
		mMoleFlags &= ~MOLE_SUIT;
		unlock();
		break;
	}
}

// 0x80082C28
// Slot _3C
void TEneBoss03Mole::vfn_3C()
{
}

// 0x80082C2C
// Slot _10 Hidden while appearing or out of view
void TEneBoss03Mole::vfn_10()
{
	if (isAppearing()) {
		mFlags |= 0x10;
	} else if (!mLight.isVisible(mDrawTask, 1000.0f, &mBody.mPos, 70.0f, 0)) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
		if (mMoleFlags & MOLE_SUIT) {
			moveDrawTask(mDrawTask, 0x22);
		}
	}
}

// 0x80082CD4
void TEneBoss03Mole::bodyMatrix()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
	gjRotateX(mBody.mRot.x);
	gjRotateZ(mBody.mRot.z);
}

// 0x80082D1C
void TEneBoss03Mole::modelMatrix()
{
}

// 0x80082D20
// Slot _1C
void TEneBoss03Mole::render()
{
	int suit = mMoleFlags & MOLE_SUIT;
	if (suit) {
		texFxSuitLoadTex(2);
	}
	njSetTexture(sMoleTextures[1]);
	enemyEyeSet(sMoleTextures[1], 1, mFrameKey);
	if (suit) {
		gjSetBlendOverride(1, 4, 5, 3);
		_nj_control_3d_flag_ |= 0x8000;
		texFxSuitBegin(0, *(u32*)&mSuit);
	}
	njPushMatrixEx();
	bodyMatrix();
	if (suit) {
		texFxSuitScroll(5.0f + mSuit.mHeight, mSuit.mHeight, 1);
		gjDrawActor(mPose);
	} else {
		drawLit(&mLight, &mTint, mPose);
	}
	njPopMatrixEx();
	if (suit) {
		texFxSuitEnd();
		_nj_control_3d_flag_ &= ~0x8000;
	}
	enemyEyeRestore();
	shadowSetAngle(&mShadow, 0);
}
