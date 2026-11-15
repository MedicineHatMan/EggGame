#include "stage/SetManager.h"
#include "enemy/EnemyCommon.h"
#include "core/dolphin.h"
#include "io/ResBank.h"
#include "egg/EggCreature.h"
#include "enemy/EnemyHit.h"
#include "enemy/TEneAm02.h"
#include "enemy/TEneAm06.h"
#include "enemy/TEneAm07.h"
#include "enemy/TEneAm08.h"
#include "enemy/TEneAntsQueen.h"
#include "enemy/TEnePenguin.h"
#include "enemy/TEneBat.h"
#include "enemy/TEneFrog.h"
#include "enemy/TEneArmadillo.h"
#include "enemy/boss/TEneBoss02.h"
#include "enemy/boss/TEneBoss03Mole.h"
#include "enemy/boss/TEnePurpleBoss.h"
#include "enemy/boss/TEneOrangeBoss.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "enemy/boss/TEneLastBoss.h"
#include "enemy/boss/TEneLastExBoss.h"
#include "enemy/boss/TEneGreenBoss.h"
#include "enemy/TEneGreenDemo.h"
#include "enemy/TEneAm01.h"
#include "enemy/Enemy.h"
#include "core/Mem.h"
#include "enemy/TEneGroupCollision.h"
#include "camera/TCamera.h"
struct ResBank;

static const char* sZakoArcs[] = {
	"ar_ene_zako_cat.arc",
	"ar_ene_zako_snake.arc",
	"ar_ene_zako_fugu.arc",
	"ar_ene_zako_ant.arc",
	"ar_ene_zako_bat.arc",
};

static const char* sZakoModels[] = {
	"ene_zako_cat.gvm",
	"ene_zako_snake.gvm",
	"ene_zako_fugu.gvm",
	"ene_zako_ant.gvm",
	"ene_zako_bat.gvm",
};

#include "enemy/TEneAm01.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "enemy/TEneAm01Parts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyCommon.h"
#include "egg/ObjEgg.h"
#include "enemy/TEneAm02_Rock.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "enemy/ObjEnemy.h"
#include "enemy/Enemy.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjShadow.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "stl/math.h"
#include "engine/GroundQuery.h"
#include "enemy/EnemyGround.h"
#include "enemy/EnemyHit.h"
#include "enemy/EnemyFx.h"
struct Vec3f;
struct ColliShape;
struct ObjShadow;

static const char* sClassName = "TEneAm01";

extern "C" {
MotionEntry sAm01Motions[] = {
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, 0.8f, 0.0f, 0 },
	{ MOTION_LOOP, 2, -1, 0, 0.0f, 30.0f, 1.4f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 19.0f, 0.6f, 0.0f, 0 },
	{ MOTION_LOOP, 0, -1, 0, 0.0f, 10.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 0.0f, 1.0f, 0.0f, 0 },
};
extern const ColliShape sAm01Shape[] = {
	{ { 0.0f, 7.0f, 0.0f }, 0x0, 0, 0x0, 7.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};

}

// 0x8001D038
// Five empty archive slots plus the shared motion archive
extern "C" void initZakoMotions()
{
	sEnemyArcSlots = new ResBank[5];
	gEnemyModelSlots = new NJS_OBJECT*[5];
	gEnemyTexSlots = new NJS_TEXLIST*[5];
	for (int i = 0; i < 5; i++) {
		resBankCtor(&sEnemyArcSlots[i]);
		gEnemyModelSlots[i] = 0;
		gEnemyTexSlots[i] = 0;
	}
	resBankLoad(&sEnemyBank, "ar_ene_zako_mtn.arc");
	gEnemyMotion = resBankGetMotion(&sEnemyBank, 0);
}

// 0x8001D0EC
extern "C" void shutdownZakoMotions()
{
	for (int i = 4; i >= 0; i--) {
		if (sEnemyArcSlots[i].mArc != 0) {
			resBankRemoveTextures(&sEnemyArcSlots[i], 0);
			resBankRelease(&sEnemyArcSlots[i]);
		}
		gEnemyModelSlots[i] = 0;
		gEnemyTexSlots[i] = 0;
	}
	if (sEnemyArcSlots != 0) {
		delete[] sEnemyArcSlots;
		sEnemyArcSlots = 0;
	}
	if (gEnemyModelSlots != 0) {
		delete[] gEnemyModelSlots;
		gEnemyModelSlots = 0;
	}
	if (gEnemyTexSlots != 0) {
		delete[] gEnemyTexSlots;
		gEnemyTexSlots = 0;
	}
	resBankRelease(&sEnemyBank);
	gEnemyMotion = 0;
}

// 0x8001D1C4
// The requested slot if it is loaded otherwise the first loaded one
extern "C" int zakoSlot(int index)
{
	int slot = index < 0 ? 0 : (index > 4 ? 4 : index);
	if (gEnemyModelSlots[slot] != 0) {
		return slot;
	}
	for (int i = 0; i < 5; i++) {
		if (gEnemyModelSlots[i] != 0) {
			return i;
		}
	}
	return 0;
}

// 0x8001D230
extern "C" void loadZako(int index)
{
	index = index < 0 ? 0 : (index > 4 ? 4 : index);
	ResBank* slot = &sEnemyArcSlots[index];
	resBankLoad(slot, sZakoArcs[index]);
	resBankAddTextures(slot, 0, sZakoModels[index]);
	gEnemyModelSlots[index] = resBankGetModels(slot)[0];
	gEnemyTexSlots[index] = resBankGetTextures(slot)[0];
}

// 0x8001D2D8
// cat 0x8001D2FC snake 0x8001D320 fugu 0x8001D344 ant 0x8001D368 bat
extern "C" void loadZakoCat() { loadZako(0); }
extern "C" void loadZakoSnake() { loadZako(1); }
extern "C" void loadZakoFugu() { loadZako(2); }
extern "C" void loadZakoAnt() { loadZako(3); }
extern "C" void loadZakoBat() { loadZako(4); }

// 0x8001D38C
// Object size 0x264 The constructor at 0x8001D43C stores "TEneAm01"
extern "C" TObject* createTEneAm01(TObject* parent, SetDesc* desc)
{
	return new TEneAm01(parent, desc);
}

// Radians to angle units
static inline int toAngle(f32 x, f32 z)
{
	return (int)(10430.38043493439 * (f32)atan2(x, z));
}

// 0 to 1 from the game's random number
static inline f32 random01()
{
	return 3.0517578e-5f * rand();
}

static inline Vec3f vec3f(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

static inline void copyVec(Vec3f* out, Vec3f v)
{
	out->x = v.x;
	out->y = v.y;
	out->z = v.z;
}

static inline void crushStart(EnemyCrush* crush, Vec3f* pos, Vec3f* hit, Vec3f ground)
{
	crush->start(pos, hit, &ground);
}

static inline Vec3f subVec(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjSubVector(&r, a, b);
	return r;
}

static inline f32 randomRange(f32 range, f32 base)
{
	return range * random01() + base;
}

static inline f32 fastSqrt(f32 x)
{
	if (x > 0.0f) {
		f64 g = __frsqrte(x);
		g = (0.5 * g) * (3.0 - x * (g * g));
		g = (0.5 * g) * (3.0 - x * (g * g));
		g = (0.5 * g) * (3.0 - x * (g * g));
		volatile f32 y = (f32)(x * g);
		return y;
	}
	return x;
}

inline void TEneAm01::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x8001D3E0
// Builds one with a brain chosen by the caller
extern "C" TEneAm01* createTEneAm01WithBrain(TObject* parent, int model, EnemySeekBrain* brain)
{
	return new TEneAm01(parent, model, brain);
}

// 0x8001D43C
TEneAm01::TEneAm01(TObject* parent, SetDesc* desc) : TObjEnemy(parent, desc)
{
	_00 = (u32)sClassName;
	mSize = 0x264;
	mVariant = 0;
	mBrain = 0;
	mPose = 0;
	mTexture = 0;
	readEnemyDesc(desc);
	updateArea();
	init();
	createBrain();
}

// 0x8001D4E8
TEneAm01::TEneAm01(TObject* parent, int model, EnemySeekBrain* brain) : TObjEnemy(parent)
{
	_00 = (u32)sClassName;
	mSize = 0x264;
	mVariant = 0;
	mBrain = brain;
	mPose = 0;
	setModel(model);
	init();
	mBrain = brain;
	if (mBrain != 0) {
		mBrain->attach(this);
	}
}

// 0x8001D59C
void TEneAm01::init()
{
	motionCtrlInit(&mAnim, sAm01Motions, gEnemyMotion);
	motionCtrlPlay(&mAnim, 4);
	mCollider = colliCreate(1, 1);
	colliCopyShapes(mCollider, (const ColliShape*)sAm01Shape);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 1, 0, 0);
	mModel = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mFlags2 = 0;
	mTopSpeed = 0.0f;
	mSpeed = 0.0f;
	mTarget.x = 0.0f;
	mTarget.y = 0.0f;
	mTarget.z = 0.0f;
	mLeapTarget.x = 0.0f;
	mLeapTarget.y = 0.0f;
	mLeapTarget.z = 0.0f;
	mHitPos.x = 0.0f;
	mHitPos.y = 0.0f;
	mHitPos.z = 0.0f;
	mSquash = 1.0f;
	mFrameKey = 0;
	_1A0 = 0;
	mLastDamage = 0;
	mThrow = 0;
	mCrush.clear();
	mBreak = 0;
	mTint.reset();
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 14.0f;
	mShadow._34 = 14.0f;
	shadowEnable(&mShadow);
	enemyEffectSetSmall(16.0f, &mEffect, &mBody.mPos);
	resetCollider();
	mStep = 0;
	mTimer = 0;
	mAngle = 0;
	mDist = 0.0f;
}

// 0x8001D700
TEneAm01::~TEneAm01()
{
	if (mBreak != 0) {
		delete mBreak;
		mBreak = 0;
	}
	if (mThrow != 0) {
		delete mThrow;
		mThrow = 0;
	}
	removeDrawTask(mModel);
	freePose((void**)&mPose);
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x8001D800
// Slot _74
void TEneAm01::setPos(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	mBody.mPos.x = x;
	f32 z = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x8001D844
// Slot _40
f32 TEneAm01::vfn_40()
{
	return 23.0f;
}

// 0x8001D84C
// Slot _58 Thrown it hurts other enemies
int TEneAm01::onHit(const TObjAction* other, int, int* result)
{
	int hit = 0;
	if (isThrown() != 0) {
		if (isEnemyObj(other) != 0) {
			fillAttack(mThrow, (HitInfo*)result);
			hit = 1;
		}
	} else if (isPlayerOrKind4(other) != 0) {
		HitInfo* info = (HitInfo*)result;
		info->mDamage = 10;
		hit = 1;
		info->mFlags.clear();
	}
	return hit;
}

// 0x8001D8E8
// Slot _78
void TEneAm01::syncCollider()
{
	colliPlaceShapes(mCollider, &mBody.mPos, sAm01Shape);
}

// 0x8001D91C
// Slot _38 Int 0 picks the variant 1 the health 10 if not above 0 2 the model
void TEneAm01::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	mVariant = pickVariant(desc);
	int health = desc->getInt(1);
	int value = 10;
	if (health > 0) {
		value = health;
	}
	mHealth = value;
	int model = desc->getInt(2);
	setModel(model);
}

// 0x8001D9B0
// Slot _70 which slot _0C calls each frame
void TEneAm01::updateBody()
{
	if (isBlocked() == 0 && isInRange(this, &mBody.mPos, 500.0f) == 0) {
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
		creditKiller();
	}

	checkFellOut();
	mBrain->update(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	mTint.update();
	applyPoseFaded(mPose, 18.0f, &mAnim, &mAppearTimer);
	enemyEffectUpdate(&mEffect);
}

// 0x8001DB04
// Slot _6C returnZero makes this do nothing
void TEneAm01::vfn_6C()
{
	if (returnZero() != 0) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		gjPushUnitMatrix();
		njPopMatrixEx();
	}
}

// 0x8001DB4C
int TEneAm01::isAppearDone()
{
	return isActionDone(ACT_APPEAR);
}

// 0x8001DB70
int TEneAm01::isAppearAimedDone()
{
	return isActionDone(ACT_APPEAR_AIMED);
}

// 0x8001DB94
int TEneAm01::isStaggerDone()
{
	return isActionDone(ACT_STAGGER);
}

// 0x8001DBB8
void TEneAm01::setTarget(const Vec3f* target)
{
	mTarget = *target;
	mFlags2 &= ~AM_ARRIVED;
}

// 0x8001DBE0
int TEneAm01::hasArrived()
{
	return mFlags2 & AM_ARRIVED;
}

// 0x8001DBEC
int TEneAm01::wasTouched()
{
	return mFlags2 & AM_TOUCHED;
}

// 0x8001DBF8
void TEneAm01::setLeapAimed()
{
	mFlags2 |= AM_LEAP_AIMED;
}

// 0x8001DC08
void TEneAm01::clearLeapAimed()
{
	mFlags2 &= ~AM_LEAP_AIMED;
}

// 0x8001DC18
int TEneAm01::isFlipDone()
{
	return mFlags2 & AM_FLIP_DONE;
}

// 0x8001DC24
// Which death fits the last hit
int TEneAm01::pickDeath()
{
	int action = ACT_DIE;
	if (enemyHitTestFlag(&mHit, 2) != 0) {
		action = ACT_DIE_THROWN;
	} else if (enemyHitTestFlag(&mHit, 0) != 0 && mBody.mPos.y - mShadow.mGround.y < 7.0f) {
		action = ACT_DIE_CRUSHED;
	}
	return action;
}

// 0x8001DCA0
int TEneAm01::pickVariant(SetDesc* desc)
{
	int value = desc->getInt(0);
	if (value < 0) {
		value = 0;
	} else if (value >= 2) {
		value = 1;
	}
	return value;
}

// 0x8001DCF0
void TEneAm01::createBrain()
{
	if (mVariant != 1) {
		mBrain = new Am01WanderBrain;
	} else {
		mBrain = new Am01LeapBrain;
	}
	if (mBrain != 0) {
		mBrain->attach(this);
	}
}

// 0x8001DD80
// Moves by a direction times mSpeed plus gravity
int TEneAm01::moveStep(f32 dirX, f32 dirZ)
{
	int free;
	Vec3f move;
	move.x = dirX * mSpeed;
	move.y = mBody.mVel.y - 0.12f;
	move.z = dirZ * mSpeed;
	getPush(&move);

	f32 d = move.x * move.x;
	d += move.z * move.z;
	if (d <= 0.01f) {
		moveVertical(move.y, &mBody.mPos, &mShadow);
		free = 1;
	} else if ((mFlags2 & AM_SIMPLE_HIT) != 0) {
		free = collide(7.0f, &mBody.mPos, &move, &mShadow) == 0;
	} else {
		SlopeFilter filter;
		filter.mUpNormal   = -2.0f;
		filter.mDownNormal = -2.0f;
		if ((mFlags2 & AM_STAGGERED) != 0) {
			filter.mStep = 1000.0f;
		} else {
			filter.mStep       = 10.0f;
			filter.mUpNormal   = 0.939f;
			filter.mDownNormal = 0.707f;
		}
		free = collide(7.0f, &mBody.mPos, &move, &filter, &mShadow) == 0;
	}

	if (isAirborne() == 0) {
		mBody.mVel.y = 0.0f;
	} else {
		mBody.mVel.y = move.y;
	}
	return free;
}

// 0x8001DEEC
void TEneAm01::checkPlayerTouch()
{
	for (ColliContact* t = mCollider->mContacts; t != 0; t = t->mNext) {
		TObjAction* other = findObjById((u16)colliContactOwner(t));
		if (other != 0 && isPlayerObj(other) != 0) {
			mFlags2 |= AM_TOUCHED;
		}
	}
}

// 0x8001DF5C
void TEneAm01::resetCollider()
{
	colliShapeSetBit8(mCollider, 0);
}

// 0x8001DF84
void TEneAm01::noOp() { }

// 0x8001DF88
void TEneAm01::beginDeath()
{
	mFrameKey = 3;
	mEnemyFlags |= ENEMY_BLOCKED;
	colliDisable(mCollider);
	shadowDisable(&mShadow);
	killed(1);
}

// 0x8001DFE0
void TEneAm01::setModel(int index)
{
	int slot = zakoSlot(index);
	if (mSlot != slot && mPose != 0) {
		freePose((void**)&mPose);
	}
	mSlot = slot;
	if (mPose == 0) {
		mPose = allocPose(gEnemyModelSlots[mSlot]);
	}
	mTexture = gEnemyTexSlots[mSlot];
}

// 0x8001E078
void TEneAm01::setVelocity(const Vec3f* vel)
{
	f32 x = vel->x;
	f32 y = vel->y;
	f32 z = vel->z;
	mBody.mVel.x = x;
	mBody.mVel.y = y;
	mBody.mVel.z = z;
	mBody.mRot.y = toAngle(x, z);
}

// 0x8001E0D8
void TEneAm01::dropItem()
{
	if ((mFlags2 & AM_NO_DROP) == 0) {
		spawnDropA(&mBody.mPos);
	}
}

// 0x8001E108
void TEneAm01::startEffect()
{
	enemyEffectStart(&mEffect);
}

// 0x8001E12C
// Slot _7C
void TEneAm01::onActionPhase(int action, int phase)
{
	switch (action) {
	case ACT_APPEAR:
		actAppear(phase);
		break;
	case ACT_APPEAR_AIMED:
		actAppearAimed(phase);
		break;
	case ACT_STAGGER:
		actStagger(phase);
		break;
	case ACT_IDLE:
		actIdle(phase);
		break;
	case ACT_WALK:
	case ACT_CHASE:
		actWalk(phase);
		break;
	case ACT_LEAP:
		actLeap(phase);
		break;
	case ACT_8:
		act8(phase);
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
	case ACT_DIE_CRUSHED:
		actDieCrushed(phase);
		break;
	}
}

// 0x8001E1E8
void TEneAm01::actAppear(int phase)
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
		break;
	case 2:
		endAppear(step);
		break;
	}
}

// 0x8001E274
// Comes out in a random direction
void TEneAm01::actAppearAimed(int phase)
{
	int* step = &mStep;
	switch (phase) {
	case 0: {
		motionCtrlPlay(&mAnim, 0);
		f32 sin, cos;
		njSinCos((int)(65536.0f * (3.0517578e-5f * rand())), &sin, &cos);
		f32 speed = randomRange(1.5f, 0.5f);
		mBody.mVel.x = sin * speed;
		mBody.mVel.y = randomRange(1.0f, 0.0f);
		mBody.mVel.z = cos * speed;
		beginAppear(step);
		break;
	}
	case 1:
		motionCtrlUpdate(&mAnim);
		runAppearAimed(step);
		break;
	case 2:
		endAppear(step);
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		break;
	}
}

// 0x8001E3EC
// Knocked along until it lands with no collider for 60 frames
void TEneAm01::actStagger(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 3);
		mFlags2 |= AM_STAGGERED;
		colliDisable(mCollider);
		mStep = 60;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		mStep--;
		if (mStep <= 0) {
			colliEnable(mCollider);
		}
		moveStep(mBody.mVel.x, mBody.mVel.z);
		if (isAirborne() == 0) {
			mHitFlags |= HIT_DONE;
		}
		break;
	case 2:
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		colliEnable(mCollider);
		mFlags2 &= ~AM_STAGGERED;
		break;
	}
}

// 0x8001E4EC
void TEneAm01::actIdle(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		moveStep(0.0f, 0.0f);
		syncCollider();
		break;
	case 2:
		break;
	}
}

// 0x8001E56C
// Turns toward mTarget and accelerates stopping once within a turning circle of it
void TEneAm01::actWalk(int phase)
{
	switch (phase) {
	case 0:
		if (mAction == ACT_WALK) {
			motionCtrlPlay(&mAnim, 1);
		} else {
			motionCtrlPlay(&mAnim, 2);
			mFrameKey = 5;
		}
		mFlags2 &= ~AM_TOUCHED;
		mStep = 0;
		if (mAction == ACT_CHASE) {
			resetCollider();
		}
		break;
	case 1: {
		motionCtrlUpdate(&mAnim);
		Vec3f toTarget;
				toTarget.x = mTarget.x;
				toTarget.y = mTarget.y;
				toTarget.z = mTarget.z;
		njSubVector(&toTarget, &mBody.mPos);
		int angle = toAngle(toTarget.x, toTarget.z);
		if (angle != mBody.mRot.y) {
			int diff;
			int rot = angleApproachDiff(mBody.mRot.y, angle, 0x100, &diff);
			if (mAction == ACT_CHASE) {
				if (mStep == 0) {
					if (diff >= 0x4000) {
						mStep = 1;
					}
				} else if (diff <= 0x200) {
					mStep = 0;
				}
			}
			mBody.mRot.y = rot;
		}

		if (hasArrived() == 0) {
			f32 radius = 5.0f + turnRadius(mSpeed, 0x100);
			toTarget.y = 0.0f;
			if (njScalor(&toTarget) < radius) {
				mFlags2 |= AM_ARRIVED;
			}
		}

		if (isAirborne() == 0) {
			if (mStep == 0) {
				mSpeed += 0.01f;
				if (mSpeed > mTopSpeed) {
					mSpeed = mTopSpeed;
				}
			} else {
				mSpeed -= 0.01f;
				if (mSpeed < 0.0f) {
					mSpeed = 0.0f;
				}
			}
		}

		njSinCos(mBody.mRot.y, &mBody.mVel.x, &mBody.mVel.z);
		moveStep(mBody.mVel.x, mBody.mVel.z);
		if (mAction == ACT_CHASE) {
			checkPlayerTouch();
		}
		syncCollider();
		break;
	}
	case 2:
		noOp();
		clearFrameKey();
		break;
	}
}

// 0x8001E7E8
// Crouches for 8 frames then jumps
void TEneAm01::actLeap(int phase)
{
	switch (phase) {
	case 0:
		if ((s16)motionCtrlCurrent(&mAnim) != 3) {
			motionCtrlPlay(&mAnim, 3);
			motionCtrlUpdate(&mAnim);
		}
		mFlags2 &= ~(AM_TOUCHED | AM_PASSED | AM_SQUASH);
		mFlags2 |= AM_STAGGERED;
		if (isAirborne() == 0) {
			mStep = 0;
		} else {
			mStep = 1;
		}
		mTimer = 0;
		mSquash = 1.0f;
		mAngle = mBody.mRot.y;
		njSinCos(mAngle, &mBody.mVel.x, &mBody.mVel.z);
		break;
	case 1:
		if ((mFlags2 & AM_LEAP_AIMED) != 0) {
			mFrameKey = 5;
		} else {
			clearFrameKey();
		}

		switch (mStep) {
		case 0:
			mFlags2 |= AM_SQUASH;
			if (isAirborne() == 0) {
				Vec3f toTarget;
				toTarget.x = mTarget.x;
				toTarget.y = mTarget.y;
				toTarget.z = mTarget.z;
				njSubVector(&toTarget, &mBody.mPos);
				mAngle = (u16)toAngle(toTarget.x, toTarget.z);
				f32 squash;
				if (mTimer < 4) {
					squash = mTimer + 1;
				} else {
					squash = 7 - mTimer;
				}
				mSquash = 1.0f - 0.125f * squash;
				mSpeed *= 0.9f;
				if (mSpeed < 0.0f) {
					mSpeed = 0.0f;
				}
				mTimer++;
				if (mTimer >= 8) {
					mTimer = 0;
					mStep = 1;
					mFlags2 &= ~AM_SQUASH;
					seStartAt(0xE4, &mBody.mPos);
					motionCtrlPlay(&mAnim, 3);
					motionCtrlUpdate(&mAnim);
					mLeapTarget = mTarget;
					toTarget.y = 0.0f;
					mDist = njScalor2(&toTarget);
					mFlags2 &= ~AM_PASSED;
					njSinCos(mAngle, &mBody.mVel.x, &mBody.mVel.z);
					mBody.mVel.y += 2.5f;
					if ((mFlags2 & AM_LEAP_AIMED) != 0) {
						mSpeed = fastSqrt(mDist) / (2.0f * (mBody.mVel.y / 0.12f));
						if (mSpeed > mTopSpeed) {
							mSpeed = mTopSpeed;
						}
						mStep = 2;
					} else {
						mSpeed = mTopSpeed;
					}
					mFlags2 &= ~AM_SPIN;
					mBody.mRot.x = 0;
				}
			} else {
				mTimer = 0;
			}
			break;
		case 1:
		case 2:
			if (mBody.mVel.y >= 0.0f) {
				if (motionCtrlIsBlending(&mAnim) != 0 || motionCtrlFrame(&mAnim) <= 5.0f) {
					motionCtrlUpdate(&mAnim);
				}
			} else {
				motionCtrlUpdate(&mAnim);
			}
			if ((mFlags2 & AM_PASSED) == 0) {
				Vec3f toTarget;
				copyVec(&toTarget, subVec(&mLeapTarget, &mBody.mPos));
				toTarget.y = 0.0f;
				f32 dist = njScalor2(&toTarget);
				if (dist > mDist) {
					mFlags2 |= AM_PASSED;
				}
				mDist = dist;
			}
			break;
		}

		int cur = mBody.mRot.y;
		int want = mAngle;
		if (want != cur) {
			mBody.mRot.y = angleApproach(cur, want, 0x400);
		}
		moveStep(mBody.mVel.x, mBody.mVel.z);

		if ((mStep == 2 || mStep == 1) && isAirborne() == 0) {
			mStep = 0;
			mTimer = 0;
			if ((mFlags2 & AM_PASSED) != 0) {
				mFlags2 |= AM_ARRIVED;
			}
		}

		if (mStep == 2 && mBody.mVel.y <= 0.0f) {
			checkPlayerTouch();
			resetCollider();
		} else {
			noOp();
		}
		syncCollider();
		break;
	case 2:
		mFlags2 &= ~(AM_STAGGERED | AM_SQUASH | AM_PASSED | AM_SPIN);
		mBody.mRot.x = 0;
		noOp();
		clearFrameKey();
		break;
	}
}

// 0x8001ED40
void TEneAm01::act8(int) { }

// 0x8001ED44
// Thrown back from the hit spinning then 30 frames lying down
void TEneAm01::actFlip(int phase)
{
	switch (phase) {
	case 0: {
		motionCtrlPlay(&mAnim, 4);
		mEnemyFlags |= ENEMY_FLIPPING;
		mFlags2 &= ~AM_FLIP_DONE;
		mFlags2 |= AM_SIMPLE_HIT | AM_SPIN;
		mStep = 0;
		int angle = toAngle(mHitPos.x, mHitPos.z);
		mBody.mVel.y = 3.0f;
		mBody.mRot.y = angle + 0x8000;
		mBody.mRot.x = 0;
		njSinCos(angle, &mBody.mVel.x, &mBody.mVel.z);
		mTopSpeed = 1.0f;
		mSpeed = mTopSpeed;
		enemyFxBurst4E(&mContact.mSurface);
		mTint.start();
		break;
	}
	case 1:
		moveStep(mBody.mVel.x, mBody.mVel.z);
		if (isAirborne() == 0) {
			mStep++;
			mSpeed *= 0.9f;
			motionCtrlPlay(&mAnim, 5);
			mBody.mRot.x = 0;
			if (mStep >= 30) {
				mStep = 0;
				mFlags2 |= AM_FLIP_DONE;
			}
			mFlags2 &= ~(AM_SIMPLE_HIT | AM_SPIN);
			mEnemyFlags &= ~ENEMY_INTANGIBLE; // not ENEMY_FLIPPING which stays set until the exit
		} else {
			mBody.mRot.x += 0x1A00;
		}
		motionCtrlUpdate(&mAnim);
		syncCollider();
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		mFlags2 &= ~(AM_SIMPLE_HIT | AM_SPIN);
		break;
	}
}

// 0x8001EF14
// Flies off in an arc
void TEneAm01::actDie(int phase)
{
	switch (phase) {
	case 0: {
		mEnemyFlags |= ENEMY_BLOCKED | ENEMY_AIRBORNE;
		mFlags2 |= AM_SPIN;
		motionCtrlPlay(&mAnim, 4);
		mBreak = new EnemyBreak(mHitPos, &mBody.mPos, &mBody.mVel);
		beginDeath();
		enemyFxBurst4F(&mContact.mSurface);
		seEnemyDie0(&mBody.mPos);
		dropItem();
		break;
	}
	case 1:
		if (mBreak->update(&mBody.mPos, &mBody.mVel) != 0) {
			mFlags |= 1;
		} else {
			mBody.mRot.x -= 0x1A00;
		}
		break;
	case 2:
		mEnemyFlags &= ~(ENEMY_BLOCKED | ENEMY_AIRBORNE);
		mFlags2 &= ~AM_SPIN;
		mHealth = 2;
		colliEnable(mCollider);
		setPos(&mHome);
		if (mBreak != 0) {
			delete mBreak;
			mBreak = 0;
		}
		break;
	}
}

// 0x8001F080
void TEneAm01::actDieCrushed(int phase)
{
	switch (phase) {
	case 0: {
		Vec3f ground;
		ground.set(mShadow.mGround.x, mShadow.mGround.y, mShadow.mGround.z);
		crushStart(&mCrush, &mBody.mPos, &mHitPos, ground);
		mFlags2 |= AM_SPIN | AM_CRUSHED;
		motionCtrlPlay(&mAnim, 4);
		motionCtrlUpdate(&mAnim);
		beginDeath();
		enemyFxBurst4F(&mContact.mSurface);
		seEnemyDie2(&mBody.mPos);
		dropItem();
		break;
	}
	case 1:
		if (mCrush.step(&mBody.mPos, &mBody.mVel, &mBody.mRot) != 0) {
			Vec3f out;
			mCrush.burst(18.0f, &mBody.mPos, &mBody.mRot, &out);
			mFlags |= 1;
		}
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_BLOCKED;
		mFlags2 &= ~(AM_SPIN | AM_CRUSHED);
		break;
	}
}

// 0x8001F1B4
// Becomes a projectile that hurts other enemies gone after 180 frames
void TEneAm01::actDieThrown(int phase)
{
	switch (phase) {
	case 0: {
		mEnemyFlags |= ENEMY_THROWN;
		mFlags2 |= AM_THROWN;
		mThrow = new RockFlight(7.0f, &mBody);
		mThrow->setVel(&mHitPos);
		mThrow->mBody.mPos.y += 7.0f;
		mThrow->mBody.mVel.y *= 0.1f;
		gjScaleVector(1.5f, &mThrow->mBody.mVel, &mThrow->mBody.mVel);
		motionCtrlPlay(&mAnim, 4);
		motionCtrlUpdate(&mAnim);
		resetCollider();
		mStep = 0;
		beginDeath();
		colliEnable(mCollider);
		shadowEnable(&mShadow);
		enemyFxBurst4F(&mContact.mSurface);
		seEnemyDie1(&mBody.mPos);
		dropItem();
		break;
	}
	case 1: {
		mThrow->update(mCollider);
		mStep++;
		if (mStep == 120) {
			mThrow->setGroundFriction();
		}
		mBody.mPos = mThrow->mBody.mPos;
		EggHits hits;
		if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&mBody.mPos, -1) != 0 && (hits.mMask & 1) != 0) {
			shadowSetFromContact(&mShadow, (u8*)&hits);
		}
		mBody.mPos.y -= 7.0f;
		syncCollider();
		if (mStep >= 180) {
			enemyFxDeath44Se(&mBody.mPos);
			mFlags |= 1;
		}
		break;
	}
	case 2:
		mEnemyFlags &= ~(ENEMY_BLOCKED | ENEMY_THROWN);
		mFlags2 &= ~AM_THROWN;
		if (mThrow != 0) {
			delete mThrow;
			mThrow = 0;
		}
		noOp();
		break;
	}
}

// 0x8001F3F4
// Slot _3C
void TEneAm01::vfn_3C()
{
	int x = kEnemyInfoX << 16;
	int line = kEnemyInfoLine;
	int pos = x | line;
	if (mVariant == 0) {
		njPrintC(pos, ":TYPE A");
	} else if (mVariant == 1) {
		njPrintC(pos, ":TYPE B");
	}
	njPrint(x | (line + 1), ":HP %3d", mHealth);
	njPrint(x | (line + 2), ":MODEL");
}

// 0x8001F480
// Slot _10 Hidden while appearing or out of view
void TEneAm01::vfn_10()
{
	if (isAppearing() != 0) {
		mFlags |= 0x10;
	} else if (isInView(500.0f, 12.0f, &mBody.mPos, 0) == 0) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
	}
}

// 0x8001F50C
// Slot _1C
void TEneAm01::render()
{
	njSetTexture(mTexture);
	enemyEyeSet((NJS_TEXLIST*)mTexture, 0, mFrameKey);
	useArea();
	njPushMatrixEx();
	njTranslateEx(&mBody.mPos);
	if ((mFlags2 & AM_CRUSHED) != 0) {
		mCrush.apply(&mBody.mRot);
	} else if (isThrown() != 0) {
		njTranslate(0, 0.0f, 8.0f, 0.0f);
		if (mThrow != 0) {
			mThrow->mBody.vfn_20();
		}
		njTranslate(0, 0.0f, -8.0f, 0.0f);
	} else {
		gjRotateY(mBody.mRot.y);
		if ((mFlags2 & AM_SQUASH) != 0) {
			f32 width = 0.8f * (1.0f - mSquash) + 1.0f;
			njScale(0, width, mSquash, width);
		}
		if ((mFlags2 & AM_SPIN) != 0) {
			njTranslate(0, 0.0f, 8.0f, 0.0f);
			gjRotateX(mBody.mRot.x);
			njTranslate(0, 0.0f, -8.0f, 0.0f);
		}
	}
	drawLit(0, &mTint, mPose);
	njPopMatrixEx();
	enemyEyeRestore();
}

#include "enemy/EnemyBrainDtors.h"

// 0x8001F6D8
// Runs the current state's phases then asks the owner to change action if the state chose one
inline void EnemyBrain::update(TEneAm01* owner)
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

// 0x8001F7B8
// Slot _AC
inline int TEneAm01::showsHitEffect()
{
	return 0;
}
