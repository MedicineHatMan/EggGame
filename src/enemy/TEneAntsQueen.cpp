#include "enemy/TEneAntsQueen.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/TEneAntsBomb.h"
#include "enemy/AntBrain.h"
#include "enemy/EnemyCommon.h"
#include "engine/MotionPose.h"
#include "io/ResBank.h"
#include "object/ObjAction.h"
#include "enemy/EnemyHit.h"
#include "enemy/ObjEnemy.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "enemy/EnemyFx.h"
#include "engine/TParticle.h"
#include "core/TMainTask.h"
#include "enemy/TEneAm01.h"
#include "animal/RacerMover.h"
struct Vec3f;
struct ColliShape;
struct ColliContact;
struct ObjShadow;

extern "C" {

MotionEntry sAntsQueenMotions[] = {
	{ MOTION_LOOP, 10, -1, 0, 0.0f, 20.0f, 0.25f, 0.0f, 0 },
	{ MOTION_LOOP, 10, -1, 0, 20.0f, 0.0f, -2.0f, 0.0f, 0 },
	{ MOTION_LOOP, 8, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 9, -1, 0, 0.0f, 20.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 49.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 19.0f, 1.2f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 39.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 29.0f, 0.6f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 44.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 49.0f, 0.35f, 0.0f, 0 },
	{ MOTION_ONCE, 4, -1, 0, 0.0f, 9.0f, 1.0f, 0.0f, 0 },
};
extern const int sAntsQueenBones[] = {
	9, 12, 16, 19, 65, 68, 69, -1,
}; // the bones mParts follow
const char* gTEneAntsQueenName = "TEneAntsQueen";
extern const EnemyFrameKey sAntsQueenFrameKey0[] = { { 13.0f, 0 }, { -1.0f, 3 } };
extern const EnemyFrameKey sAntsQueenFrameKey1[] = { { 9.0f, 0 }, { 14.0f, 5 }, { 26.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAntsQueenFrameKey2[] = { { 1.0f, 0 }, { 16.0f, 3 }, { -1.0f, 0 } };
extern const EnemyFrameKey sAntsQueenFrameKey3[] = { { 1.0f, 0 }, { 31.0f, 3 }, { -1.0f, 4 } };
extern const EnemyFrameKey sAntsQueenFrameKey4[] = { { 1.0f, 0 }, { -1.0f, 3 } };
const EnemyFrameKey* sAntsQueenFrameKeys[5] = { sAntsQueenFrameKey0, sAntsQueenFrameKey1, sAntsQueenFrameKey2, sAntsQueenFrameKey3, sAntsQueenFrameKey4 };
extern const ColliShape sAntsQueenShape[] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 13.0f, 19.0f, 0.0f, 0x0, COLLISHAPE_CYLINDER, 0x0 },
};
}

// Radians to angle units
static inline int toAngle(f32 x, f32 z)
{
	return (int)(10430.38043493439 * (f32)atan2(x, z));
}

// 0x800923E0
AntLink::AntLink()
{
	mPrev = 0;
	mNext = 0;
	mSlot = -1;
	mBrain = 0;
	mOwner = 0;
}

// 0x80092400
AntLink::~AntLink()
{
	if (mPrev != 0) {
		mPrev->mNext = mNext;
	}
	if (mNext != 0) {
		mNext->mPrev = mPrev;
	}
}

// 0x80092468
void AntLink::setOwner(TEneAntsQueen* owner)
{
	mOwner = owner;
}

// 0x80092470
TEneAntsQueen* AntLink::getOwner()
{
	return mOwner;
}

// 0x80092478
// When the queen goes her ants stop knowing her
void AntLink::clearOwners()
{
	for (AntLink* link = this; link != 0; link = link->mNext) {
		link->mOwner = 0;
	}
}

// 0x80092494
void AntLink::insert(AntLink* head, int slot, void* brain)
{
	AntLink* at = head;
	AntLink* next;
	while ((next = at->mNext) != 0) {
		if (next->mSlot > slot) {
			next->mPrev = this;
			break;
		}
		at = next;
	}
	mNext = at->mNext;
	at->mNext = this;
	mPrev = at;
	mSlot = slot;
	mBrain = brain;
	mOwner = at->mOwner;
}

// 0x800924E0
int AntLink::count()
{
	int n = 0;
	AntLink* link = this;
	while ((link = link->mNext) != 0) {
		n++;
	}
	return n;
}

// 0x80092500
extern "C" void initTEneAntsQueen()
{
	resBankLoad(&sAntsQueenBank, "ar_ene_ants_queen.arc");
	resBankAddTextures(&sAntsQueenBank, 0, "ene_ants_queen.gvm");
	sAntsQueenModels = resBankGetModels(&sAntsQueenBank);
	sAntsQueenTextures = resBankGetTextures(&sAntsQueenBank);
	sAntsQueenMotion = resBankGetMotion(&sAntsQueenBank, 0);
}

extern "C" void shutdownTEneAntsQueen()
{
	resBankRemoveTextures(&sAntsQueenBank, 0);
	resBankRelease(&sAntsQueenBank);
	sAntsQueenModels = 0;
	sAntsQueenTextures = 0;
	sAntsQueenMotion = 0;
}

extern "C" TObject* createTEneAntsQueen(TObject* parent, SetDesc* desc)
{
	return new TEneAntsQueen(parent, desc);
}

TEneAntsQueen::TEneAntsQueen(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00 = (u32)gTEneAntsQueenName;
	mSize = 0x2E0;
	init(desc);
}

// 0x800926B8
void TEneAntsQueen::init(SetDesc* desc)
{
	motionCtrlInit(&mAnim, sAntsQueenMotions, sAntsQueenMotion);
	motionCtrlPlay(&mAnim, 2);
	mBody.mRot.x = 0;
	mCollider = colliCreate(1, 1);
	colliCopyShapes(mCollider, (const ColliShape*)sAntsQueenShape);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0x14, 0, 0x10);
	colliShapeSetBit8(mCollider, 0);
	colliShapeSetTag(mCollider, 0, 0);
	mBrain = 0;
	readEnemyDesc(desc);
	mPose = allocPose(*(void**)sAntsQueenModels);
	mModelTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags2 = 0;
	mKnock.x = 0.0f;
	mKnock.y = 0.0f;
	mKnock.z = 0.0f;
	mFrameKey = 0;
	for (int i = 0; i < 7; i++) {
		mParts[i].x = 0.0f;
		mParts[i].y = 0.0f;
		mParts[i].z = 0.0f;
	}
	mAim.x = 0.0f;
	mAim.y = 0.0f;
	mAim.z = 0.0f;
	mAimAngle = 0;
	mMaxAnts = 10;
	mSlot = -1;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 26.0f;
	mShadow._34 = 26.0f;
	shadowEnable(&mShadow);
	mBar = new TEnemyLifeBar(this);
	mBar->setMax((s16)mHealth);
	mBar->setMark(0);
	mBar->setValue((s16)mHealth);
	mBar->setSize(50.0f, 25.0f);
	mLight.reset();
	mLight.setFadeDist(-15.0f);
	mTint.reset();
	mAnts.setOwner(this);
	enemyEffectSetLarge(38.0f, &mEffect, &mBody.mPos);
	mStep = 0;
	mTurnStep = 0;
	mAppearTimer2 = 0.0f;
	createBrain();
}

// 0x800928FC
TEneAntsQueen::~TEneAntsQueen()
{
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	mAnts.clearOwners();
	removeDrawTask(mModelTask);
	if (mBar != 0) {
		mBar->mFlags |= 1;
		mBar = 0;
	}
	freePose((void**)&mPose);
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x80092A04
void TEneAntsQueen::setPos(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	mBody.mPos.x = x;
	f32 z = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x80092A48
f32 TEneAntsQueen::vfn_40()
{
	return 38.0f;
}

// 0x80092A50
// The body's shape follows part 4 19 above the feet
void TEneAntsQueen::syncCollider()
{
	Vec3f* body = (Vec3f*)colliGetShapes(mCollider);
	*body = mParts[4];
	body->y = 19.0f + mBody.mPos.y;
	colliRefreshBounds(mCollider);
	colliBoundsOn(mCollider);
}

// 0x80092AB8
// Health defaults to 200
void TEneAntsQueen::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	int health = desc->getInt(0);
	mHealth = health <= 0 ? 200 : health;
}

// 0x80092B14
// It hurts nothing by touch
int TEneAntsQueen::onHit(const TObjAction* other, int shape, int* result)
{
	return 0;
}

// 0x80092B1C
void TEneAntsQueen::setTarget(const Vec3f* target)
{
	mTarget = *target;
}

// 0x80092B38
void TEneAntsQueen::setAim(const Vec3f* pos, int angle)
{
	mAim = *pos;
	mAimAngle = angle;
}

// 0x80092B58
AntLink* TEneAntsQueen::getAnts()
{
	return &mAnts;
}

// 0x80092B60
void TEneAntsQueen::hitEffects()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
}

// 0x80092BA0
// Room for one more
int TEneAntsQueen::canSummon()
{
	return mAnts.count() < 10;
}

// 0x80092BDC
int TEneAntsQueen::getMaxAnts()
{
	return mMaxAnts;
}

// 0x80092BE4
// Sets every ant after the player who hit her
void TEneAntsQueen::alertAnts(u16 player)
{
	for (AntLink* link = mAnts.mNext; link != 0; link = link->mNext) {
		if (link->mBrain != 0) {
			((Am01AntBrain*)link->mBrain)->setChase(player);
		}
	}
}

// 0x80092C34
int TEneAntsQueen::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

// 0x80092C58
int TEneAntsQueen::hasArrived()
{
	return mFlags2 & QUEEN_ARRIVED;
}

// 0x80092C64
int TEneAntsQueen::isSummonDone()
{
	return isActionDone(ACT_SUMMON);
}

// 0x80092C88
int TEneAntsQueen::hasSummoned()
{
	return mFlags2 & QUEEN_SUMMONED;
}

// 0x80092C94
void TEneAntsQueen::setSlot(int angle, int slot)
{
	mAimAngle = angle;
	mSlot = slot;
}

// 0x80092CA0
int TEneAntsQueen::isThrowDone()
{
	return isActionDone(ACT_THROW);
}

// 0x80092CC4
int TEneAntsQueen::isStunDone()
{
	return isActionDone(ACT_STUN);
}

// 0x80092CE8
int TEneAntsQueen::isNotThrowing()
{
	return !(mFlags2 & QUEEN_THROWING);
}

// 0x80092CFC
int TEneAntsQueen::isFlipDone()
{
	return isActionDone(ACT_FLIP);
}

// 0x80092D20
void TEneAntsQueen::stunEffects()
{
	hitEffects();
	seStartAt(0xF9, &mBody.mPos);
}

// 0x80092D58
void TEneAntsQueen::dieEffects()
{
	hitEffects();
	seStartAt(0xFA, &mBody.mPos);
}

// 0x80092D90
int TEneAntsQueen::deathAction()
{
	return ACT_DIE;
}

// 0x80092D98
void TEneAntsQueen::startEffect()
{
	enemyEffectStart(&mEffect);
}

// 0x80092DBC
// Slot _70 which slot _0C calls each frame
void TEneAntsQueen::updateBody()
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
	mBar->setPos(&mBody.mPos);
	mBar->setValue((s16)mHealth);
	enemyEffectUpdate(&mEffect);
}

// 0x80092EFC
// Slot _6C returnZero makes this do nothing
void TEneAntsQueen::vfn_6C()
{
	if (returnZero() != 0) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginBones();
		drawFirstFrame(sAntsQueenMotion, 0xB, mPose);
		endBones();
		updateParts();
		syncCollider();
	}
}

// 0x80092F78
void TEneAntsQueen::createBrain()
{
	mBrain = AntsQueenHuntBrain::create(this);
}

// 0x80092FA8
// The model writes the seven part positions while this is on
void TEneAntsQueen::beginBones()
{
	nodeTrackBeginOrigins(sAntsQueenBones, mParts);
}

// 0x80092FD8
void TEneAntsQueen::endBones()
{
	nodeTrackEnd();
}

// 0x80092FF8
void TEneAntsQueen::updatePose()
{
	beginBones();
	applyPoseFaded(mPose, 38.0f, &mAnim, &mAppearTimer);
	endBones();
}

// 0x80093044
// From the model's space to the world's
void TEneAntsQueen::updateParts()
{
	gjPushUnitMatrix();
	applyTransform();
	for (int i = 0; i < 7; i++) {
		njCalcPointEx(&mParts[i], &mParts[i]);
	}
	njPopMatrixEx();
}

// 0x800930A4
// Slot _A8
int TEneAntsQueen::getHitKind(const void* touch, HitInfo* info)
{
	return colliContactInGroup2((ColliContact*)touch) != 0 ? 2 : 3;
}

// 0x800930D4
// Slot _A4
int TEneAntsQueen::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x800930EC
void TEneAntsQueen::applyTransform()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
}

// 0x80093124
// How far one part moved this frame as a move for the body so the feet don't slide
void TEneAntsQueen::partMove(f32 dist, int part, Vec3f* out)
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

// 0x800931D0
int TEneAntsQueen::move(Vec3f* vel)
{
	Vec3f push;
	getFlatPush(&push);
	njAddVector(&push, vel);
	int result = collideFiltered(13.0f, &mBody.mPos, &push, &mShadow);
	if (isAirborne() == 0) {
		vel->y = 0.0f;
	}
	return result;
}

// 0x80093258
// Turns toward mTarget offset by an angle and steps with the foot that's down
void TEneAntsQueen::walk(f32 dist, int rate, int offset)
{
	int foot;
	f32 t = motionCtrlFrame(&mAnim);
	if (t >= 5.0f && t < 15.0f && motionCtrlIsBlending(&mAnim) == 0) {
		foot = 1;
	} else {
		foot = 3;
	}
	Vec3f to;
	to.x = mTarget.x;
	to.y = mTarget.y;
	to.z = mTarget.z;
	njSubVector(&to, &mBody.mPos);
	int a = (int)(10430.38043493439 * (f32)atan2(to.x, to.z));
	a += offset;
	turnTo(a, rate);
	partMove(dist, foot, &mBody.mVel);
	move(&mBody.mVel);
	if (probeLine(&mTarget, &mBody.mPos, (void*)rate, &mBody.mVel) != 0) {
		mFlags2 |= QUEEN_ARRIVED;
	} else {
		mFlags2 &= ~QUEEN_ARRIVED;
	}
}

// 0x80093390
void TEneAntsQueen::coast(f32 drag)
{
	mBody.mVel.x *= drag;
	mBody.mVel.y += -0.12f;
	mBody.mVel.z *= drag;
	move(&mBody.mVel);
}

// 0x800933DC
// The mouth and the way it faces
void TEneAntsQueen::getMouth(Vec3f* pos, Vec3f* dir)
{
	Vec3f base;
	base.x = mParts[5].x;
	base.y = mParts[5].y;
	base.z = mParts[5].z;
	*dir = mParts[6];
	*pos = *dir;
	njSubVector(dir, &base);
	if (njUnitVector(dir) < gUnitVectorMin) {
		*dir = gVecZero;
	}
}

// 0x80093488
// A puff at the mouth and a burst 2 along the way it faces
void TEneAntsQueen::throwEffects(Vec3f* pos, Vec3f* dir)
{
	particleOnceByIdRotDefault(pos, 0, 0x75, 0, 0, 5.0f);
	Vec3f at;
	gjMulAddVector(2.0f, &at, pos, dir);
	int rotY = mBody.mRot.y;
	f32 x = dir->x;
	f32 z = dir->z;
	x *= x;
	z *= z;
	x += z;
	f32 flat = __fres((f32)__frsqrte(x));
	int pitch = toAngle(dir->y, flat);
	particleOnceByIdRotDefault(&at, 0, 0x73, pitch, rotY, 0.0f);
	seStartAt(0xF6, &at);
}

// 0x80093550
void TEneAntsQueen::getHome(Vec3f* out)
{
	*out = mHome;
}

// 0x8009356C
// The slots are spread evenly on a ring 80 out
void TEneAntsQueen::getSlotPos(int slot, Vec3f* out)
{
	f32 sin;
	f32 cos;
	njSinCos((slot << 16) / mMaxAnts, &sin, &cos);
	*out = mBody.mPos;
	out->x = 80.0f * sin + out->x;
	out->z = 80.0f * cos + out->z;
}

// 0x800935EC
// Slot _7C
void TEneAntsQueen::onActionPhase(int action, int phase)
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
	case ACT_RETREAT:
		actRetreat(phase);
		break;
	case ACT_SUMMON:
		actSummon(phase);
		break;
	case ACT_THROW:
		actThrow(phase);
		break;
	case ACT_STUN:
	case ACT_FLIP:
		actStun(phase);
		break;
	case ACT_DIE:
		actDie(phase);
		break;
	}
}

// 0x80093684
void TEneAntsQueen::actAppear(int phase)
{
	int* step = &mStep;
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 2);
		beginAppear(step);
		mFrameKey = 2;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		runAppear(step);
		updatePose();
		updateParts();
		break;
	case 2:
		endAppear(step);
		mFrameKey = 0;
		break;
	}
}

// 0x80093730
void TEneAntsQueen::actIdle(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 2);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		coast(0.2f);
		updateParts();
		break;
	case 2:
		break;
	}
}

// 0x800937B4
void TEneAntsQueen::actWalk(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.05f, &mAnim, 0);
		mFlags2 &= ~QUEEN_ARRIVED;
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		walk(1.0f, 0x100, 0);
		updateParts();
		break;
	case 2:
		mFlags2 &= ~QUEEN_ARRIVED;
		break;
	}
}

// 0x8009385C
// Backs away with a cry every 25 frames
void TEneAntsQueen::actRetreat(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.05f, &mAnim, 1);
		mFlags2 &= ~QUEEN_ARRIVED;
		clearFrameKey();
		mStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (motionCtrlIsBlending(&mAnim) == 0) {
			mStep--;
			if (mStep <= 0) {
				mStep = 25;
				seStartAt(0xF8, &mBody.mPos);
			}
		}
		walk(0.3f, 0x400, 0x8000);
		updateParts();
		break;
	case 2:
		mFlags2 &= ~QUEEN_ARRIVED;
		break;
	}
}

// 0x8009394C
// Turns to the free slot over the wind-up and at frame 12.5 calls an ant out of her mouth
void TEneAntsQueen::actSummon(int phase)
{
	switch (phase) {
	case 0: {
		mFlags2 &= ~QUEEN_SUMMONED;
		mStep = 0;
		mBody.mVel.x = 0.0f;
		mBody.mVel.z = 0.0f;
		int frames;
		if ((s16)motionCtrlCurrent(&mAnim) == 6) {
			motionCtrlPlay(&mAnim, 6);
			frames = 0;
		} else {
			motionCtrlPlayBlend(0.1f, &mAnim, 6);
			frames = (int)(f32)ceil(10.0);
		}
		frames += (int)(f32)ceil(1.6666666269302368);
		mStep = frames;
		mTurnStep = angleDiff(mBody.mRot.y, mAimAngle);
		mTurnStep = mTurnStep / mStep;
		clearFrameKey();
		break;
	}
	case 1:
		motionCtrlUpdate(&mAnim);
		waitMotion();
		updatePose();
		setFrameKey(sAntsQueenFrameKeys[0]);
		if (mStep > 0) {
			mStep--;
			mBody.mRot.y = mAimAngle - mTurnStep * mStep;
		} else {
			mBody.mRot.y = mAimAngle;
		}
		coast(0.2f);
		updateParts();
		if (motionCtrlPassed(12.5f, &mAnim) != 0) {
			TEneAm01* ant = createTEneAm01WithBrain(gTL_04, 3, new Am01AntBrain(&mAnts, mSlot));
			if (ant != 0) {
				Vec3f dir;
				Vec3f pos;
				getMouth(&pos, &dir);
				throwEffects(&pos, &dir);
				gjScaleVector(1.5f, &dir, &dir);
				ant->setPos(&pos);
				ant->setVelocity(&dir);
				ant->updateArea();
			}
			mFlags2 |= QUEEN_SUMMONED;
		}
		break;
	case 2:
		mFlags2 &= ~QUEEN_SUMMONED;
		clearFrameKey();
		break;
	}
}

// 0x80093B90
// A bomb at frame 13 with a cry at 2 and sparks at 7
void TEneAntsQueen::actThrow(int phase)
{
	Vec3f dir;
	Vec3f pos;
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 7);
		mFlags2 |= QUEEN_THROWING;
		mBody.mVel.x = 0.0f;
		mBody.mVel.z = 0.0f;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		waitMotion();
		updatePose();
		setFrameKey(sAntsQueenFrameKeys[1]);
		turnTo(mAimAngle, 0x1000);
		coast(0.2f);
		updateParts();
		if (motionCtrlPassed(2.0f, &mAnim) != 0) {
			getMouth(&pos, &dir);
			seStartAt(0xF7, &pos);
		}
		if (motionCtrlPassed(7.0f, &mAnim) != 0) {
			getMouth(&pos, &dir);
			particleGenTimedByIdDefault(&pos, 0, 0x74, 0, 2);
		}
		if (motionCtrlPassed(13.0f, &mAnim) != 0) {
			getMouth(&pos, &dir);
			new TEneAntsBomb(gTL_04, &pos, &mAim, &dir);
			throwEffects(&pos, &dir);
		}
		break;
	case 2:
		clearFrameKey();
		mFlags2 &= ~QUEEN_THROWING;
		break;
	}
}

// 0x80093D34
// Knocked along the hit The flip goes half as far
void TEneAntsQueen::actStun(int phase)
{
	switch (phase) {
	case 0:
		mBody.mVel.x = mKnock.x;
		mBody.mVel.z = mKnock.z;
		if (mAction == ACT_STUN) {
			motionCtrlPlay(&mAnim, 8);
			mBody.mVel.x *= 0.5f;
			mBody.mVel.z *= 0.5f;
		} else {
			motionCtrlPlay(&mAnim, 9);
		}
		mEnemyFlags |= ENEMY_FLIPPING;
		mFrameKey = 3;
		stunEffects();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		coast(0.7f);
		if (mAction == ACT_STUN) {
			setFrameKey(sAntsQueenFrameKeys[2]);
		} else {
			setFrameKey(sAntsQueenFrameKeys[3]);
		}
		updateParts();
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		clearFrameKey();
		break;
	}
}

// 0x80093E6C
// Falls and is gone 60 frames after the motion ends leaving a drop
void TEneAntsQueen::actDie(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 10);
		mEnemyFlags |= ENEMY_INTANGIBLE | ENEMY_BLOCKED;
		colliDisable(mCollider);
		mBar->hide();
		killed(1);
		dieEffects();
		mFrameKey = 3;
		mStep = 60;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		if (motionCtrlIsDone(&mAnim) != 0) {
			mStep--;
			if (mStep <= 0) {
				enemyFxDeath4A(&mBody.mPos);
				spawnDropB(&mBody.mPos);
				mFlags |= 1;
			}
		}
		updateParts();
		if (motionCtrlPassed(7.5f, &mAnim) != 0) {
			Vec3f pos;
			pos.x = mParts[4].x;
			pos.y = mParts[4].y;
			pos.z = mParts[4].z;
			pos.y = mBody.mPos.y;
			particleOnceByIdDefault(&pos, 0, 0x46);
		}
		break;
	case 2:
		mEnemyFlags &= ~(ENEMY_INTANGIBLE | ENEMY_BLOCKED);
		clearFrameKey();
		break;
	}
}

// 0x80093FC4
// Slot _3C
void TEneAntsQueen::vfn_3C()
{
	int x = kEnemyInfoX << 16;
	int line = kEnemyInfoLine;
	njPrint(x | line, ":HP %4d", mHealth);
}

// 0x80094000
// Slot _10
void TEneAntsQueen::vfn_10()
{
	if (isAppearing() != 0) {
		mFlags |= 0x10;
	} else if (mLight.isVisible(mModelTask, 500.0f, &mBody.mPos, 60.0f, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x80094090
// Slot _1C
void TEneAntsQueen::render()
{
	njSetTexture(*(void**)sAntsQueenTextures);
	enemyEyeSet(*(NJS_TEXLIST**)sAntsQueenTextures, 0, mFrameKey);
	useArea();
	njPushMatrixEx();
	applyTransform();
	drawLit(&mLight, &mTint, mPose);
	njPopMatrixEx();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}
