#include "animal/RacerMover.h"
#include "io/ResBank.h"
#include "enemy/boss/TEneOrangeBossBall.h"
#include "enemy/boss/TEneOrangeBoss.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "enemy/boss/BossParts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyAppear.h"
#include "engine/MotionPose.h"
#include "stage/SetManager.h"
#include "Ninja/ninja.h"
#include "engine/CharaColli.h"
#include "enemy/EnemyHit.h"
#include "enemy/ObjEnemy.h"
#include "stl/math.h"
#include "enemy/boss/TEneBoss03_VolRock.h"
#include "object/ObjAction.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "enemy/EnemyFx.h"
#include "core/MathUtil.h"
#include "stage/TGameGoal.h"
#include "enemy/EnemyTexFx.h"
#include "object/ObjShadow.h"
#include "enemy/EnemyCommon.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "engine/TParticle.h"
#include "enemy/boss/TEneOrangeBossPunch.h"
#include "core/TMainTask.h"
#include "enemy/TEneAm01.h"

extern "C" void goalScalePos(Vec3f* out, const Vec3f* pos, f32 scale)
{
	out->x = pos->x * scale;
	out->y = 50.0f;
	out->z = pos->z * scale;
}
struct GjActor;
struct Vec3f;
struct ColliShape;
struct CharaColli;
struct NJS_TEXLIST;

extern "C" {

inline void TEneOrangeBoss::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

extern const int sOrangeBossNodes[] = { 1, 9, 14, 17, 20, 25, 28, 33, 36, 46, 58, 77, 82, 93, 98, 99, -1 };
const char* gTEneOrangeBossName = "TEneOrangeBoss";
extern const EnemyFrameKey sOrangeBossFrameKey0[] = { { 2.0f, 0 }, { 15.0f, 3 }, { 21.0f, 2 }, { -1.0f, 3 } };
extern const EnemyFrameKey sOrangeBossFrameKey1[] = { { 24.0f, 0 }, { -1.0f, 3 } };
extern const EnemyFrameKey sOrangeBossFrameKey2[] = { { 2.0f, 3 }, { 19.0f, 2 }, { -1.0f, 3 } };
extern const EnemyFrameKey sOrangeBossFrameKey3[] = { { 7.0f, 0 }, { 38.0f, 4 }, { -1.0f, 0 } };
MotionEntry sOrangeBossMotionKeys[32] = {
	{ MOTION_LOOP, 29, -1, 0, 0.0f, 30.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 26, -1, 0, 0.0f, 25.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 27, -1, 0, 0.0f, 0.7f, 20.0f, 0.0f, 0 },
	{ MOTION_ONCE, 21, -1, 0, 0.0f, 24.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 21, -1, 0, 16.0f, 24.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 22, -1, 0, 0.0f, 20.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 23, -1, 0, 0.0f, 24.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 18, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 19, -1, 0, 0.0f, 10.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 20, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 16, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 17, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 12, -1, 0, 0.0f, 24.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 13, -1, 0, 0.0f, 20.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 14, -1, 0, 0.0f, 24.0f, 0.6f, 0.0f, 0 },
	{ MOTION_LOOP, 28, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 10, -1, 0, 0.0f, 19.0f, 0.6f, 0.0f, 0 },
	{ MOTION_ONCE, 25, -1, 0, 0.0f, 34.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 24, -1, 0, 0.0f, 20.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 0.0f, 54.0f, 0.4f, 0.0f, 0 },
	{ MOTION_BLEND, 0, -1, 21, 0.0f, 9.0f, 0.3f, 0.1f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 24.0f, 0.4f, 0.0f, 0 },
	{ MOTION_BLEND, 3, -1, 24, 0.0f, 14.0f, 0.3f, 0.1f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 24.0f, 0.4f, 0.0f, 0 },
	{ MOTION_BLEND, 6, -1, 27, 0.0f, 11.0f, 0.3f, 0.1f, 0 },
	{ MOTION_LOOP, 7, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 22.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 30, -1, 0, 0.0f, 34.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 15, -1, 0, 0.0f, 479.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 239.0f, 1.0f, 0.0f, 0 },
};
const EnemyFrameKey* sOrangeBossFrameKeys[4] = {
	sOrangeBossFrameKey0, sOrangeBossFrameKey1, sOrangeBossFrameKey2, sOrangeBossFrameKey3,
};
extern const ColliShape sOrangeBossShapes[7] = {
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 8.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 8.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 8.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 6.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 6.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 6.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
	{ { 0.0f, 0.0f, 0.0f }, 0x0, 0, 0x0, 6.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};
extern "C" {
NJS_OBJECT** sOrangeBossModels;
NJS_TEXLIST** sOrangeBossTextures;
NJS_MOTION** sOrangeBossMotion;
OrangeBossParams* gOrangeBossParams;
FxBankFile sOrangeBossFx;
ResBank sOrangeBossBank;
}
}

// 0x8011462C
extern "C" void initTEneOrangeBoss()
{
	resBankLoad(&sOrangeBossBank, "ar_ene_orange_boss.arc");
	resBankAddTextures(&sOrangeBossBank, 0, "ene_orange_boss.gvm");
	sOrangeBossModels = resBankGetModels(&sOrangeBossBank);
	sOrangeBossTextures = resBankGetTextures(&sOrangeBossBank);
	sOrangeBossMotion = resBankGetMotion(&sOrangeBossBank, 0);
	gOrangeBossParams = (OrangeBossParams*)enemyParamGet(5);
	fxBankLoadDefault(&sOrangeBossFx, "pa_orangeboss.pad");
	orangeBallLoad();
}

// 0x801146CC
extern "C" void shutdownTEneOrangeBoss()
{
	orangeBallRelease();
	fxBankRelease(&sOrangeBossFx);
	gOrangeBossParams = 0;
	resBankRemoveTextures(&sOrangeBossBank, 0);
	resBankRelease(&sOrangeBossBank);
	sOrangeBossModels = 0;
	sOrangeBossTextures = 0;
	sOrangeBossMotion = 0;
}

// 0x80114728
extern "C" TObject* createTEneOrangeBoss(TObject* parent, SetDesc* desc)
{
	return new TEneOrangeBoss(parent, desc);
}

// 0x8011477C
extern "C" void orangeBossDrawReflections(TEneOrangeBoss* boss)
{
	boss->drawReflections();
}

// 0x8011479C
// Model-table word for an index below 3
extern "C" NJS_OBJECT* modelAtTEneOrangeBoss(int index)
{
	if (index < 0 || index >= 3) {
		return 0;
	}
	return sOrangeBossModels[index];
}

// 0x801147C4
extern "C" NJS_TEXLIST* texListTEneOrangeBoss()
{
	return sOrangeBossTextures[0];
}

// 0x801147D0
extern "C" void* paramsTEneOrangeBoss()
{
	return (int*)sOrangeBossNodes;
}

// 0x801147DC
TEneOrangeBoss::TEneOrangeBoss(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	_00 = (u32)gTEneOrangeBossName;
	mSize = 0x5AC;
	init(desc);
	initBossBar();
}

// 0x801148AC
void TEneOrangeBoss::init(SetDesc* desc)
{
	int i;

	motionCtrlSetPlayer(&mAnim, &mMotion);
	motionCtrlInit(&mAnim, sOrangeBossMotionKeys, sOrangeBossMotion);
	motionCtrlPlay(&mAnim, 0);
	mBody.mRot.x = 0;
	motionCtrlSetPlayer(&mAnim2, &mMotion2);
	motionCtrlInit(&mAnim2, sOrangeBossMotionKeys, sOrangeBossMotion);
	motionCtrlPlay(&mAnim2, 0);
	mCollider = (CharaColli*)orangeBossMakeCollider(mObjId);
	mBodyCyl  = colliCreate(1, 1);
	colliSetOwner(mBodyCyl, (u16)mObjId);
	colliSetMasks(mBodyCyl, 2, 0xFF, 0, 0x14);
	colliShapeMakeCylinder(colliGetShapes(mBodyCyl), &mBody.mPos, 1, 10.0f, 10.0f);
	colliShapeSetBit10(mBodyCyl, 0);
	colliDisable(mBodyCyl);
	mBrain = 0;
	readEnemyDesc(desc);
	mPose  = allocPose(sOrangeBossModels[1]);
	mPose2 = allocPose(sOrangeBossModels[1]);
	mtxIdentity(_1A4);
	mDrawTask   = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mMirrorTask = addDrawTask(this, (void (*)(void*))orangeBossDrawReflections, 0x19, 0);
	mBossFlags  = 0;
	mHitDir.x   = 0.0f;
	mHitDir.y   = 0.0f;
	mHitDir.z   = 0.0f;
	mFrameKey   = 0;
	for (i = 0; i < 16; i++) {
		mNodes[i].x = 0.0f;
		mNodes[i].y = 0.0f;
		mNodes[i].z = 0.0f;
	}
	mMoveTarget.x = 0.0f;
	mMoveTarget.y = 0.0f;
	mMoveTarget.z = 0.0f;
	mMoveArg      = 0;
	mLookTarget.x = 0.0f;
	mLookTarget.y = 0.0f;
	mLookTarget.z = 0.0f;
	mFacingArg    = 0;
	mFacingId     = 0xFFFF;
	mJumpFrom.x   = 0.0f;
	mJumpFrom.y   = 0.0f;
	mJumpFrom.z   = 0.0f;
	mMaxHealth    = 500;
	mHealth       = 500;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 44.0f;
	mShadow._34 = 44.0f;
	shadowEnable(&mShadow);
	mLight.reset();
	mTint.reset();
	mFx = 0;
	seCueClear(&mSe);
	mSlot = 0;
	mirrorSlotPos(mSlot, &mSlotPos, &mSlotAngle);
	mAimNode  = 0;
	mAimSide  = 0.0f;
	mBall     = 0;
	mFootY    = 0.0f;
	mRange[0] = 0.0f;
	mRange[1] = 0.0f;
	mRange[2] = 0.0f;
	mRange[3] = 0.0f;
	for (i = 0; i < 3; i++) {
		mGlow[i] = 0;
	}
	for (i = 0; i < 9; i++) {
		mSparks[i] = 0;
	}
	mLand     = 0;
	mActStep  = 0;
	mActTimer = 0;
	mBaseAngle      = 0;
	_594      = 0;
	mActTime  = 0.0f;
	mActEnd   = 0.0f;
	_5A0      = 0.0f;
	_5A4      = 0.0f;
	_5A8      = 0.0f;
	makeBrain();
	setBossView(10.0f, 33.0f, 150.0f);
}

// 0x80114BC0
// The seven spheres the body is hit with
extern "C" void* orangeBossMakeCollider(u16 objId)
{
	CharaColli* colli = colliCreate(7, 1);
	int tag;
	int i;

	colliCopyShapes(colli, (const ColliShape*)sOrangeBossShapes);
	colliSetOwner(colli, objId);
	colliSetMasks(colli, 2, 0xFF, 0, 0x10);
	tag = 0;
	for (i = 0; i < 7; i++, tag++) {
		colliShapeSetTag(colli, i, tag);
	}
	return colli;
}

// 0x80114C54
extern "C" void orangeBossColliderBit8(void* colli)
{
	int i;

	for (i = 0; i < 7; i++) {
		colliShapeSetBit8((CharaColli*)colli, i);
	}
}

// 0x80114C98
TEneOrangeBoss::~TEneOrangeBoss()
{
	int i;

	killBalls();
	for (i = 2; i >= 0; i--) {
		if (mGlow[i] != 0) {
			mGlow[i]->mFlags |= 1;
			mGlow[i] = 0;
		}
	}
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
	unlock();
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	removeDrawTask(mMirrorTask);
	removeDrawTask(mDrawTask);
	if (mBall != 0) {
		mBall->mFlags |= 1;
		mBall = 0;
	}
	freePose((void**)&mPose2);
	freePose((void**)&mPose);
	colliRetire(mBodyCyl);
	mBodyCyl = 0;
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x80114E2C
void TEneOrangeBoss::setPos(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	mBody.mPos.x = x;
	f32 z = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x80114E70
f32 TEneOrangeBoss::vfn_40()
{
	return 45.0f;
}

// 0x80114E78
// Puts the seven spheres on the tracked nodes
extern "C" void orangeBossFitCollider(void* colli, Vec3f* nodes)
{
	Vec3f mid;
	Vec3f* shapes = (Vec3f*)colliGetShapes((CharaColli*)colli);

	shapes[0] = nodes[0];
	shapes[4] = nodes[9];
	shapes[8] = nodes[10];
	mid       = nodes[8];
	njAddVector(&mid, &nodes[7]);
	gjScaleVector(0.5f, &mid, &mid);
	shapes[12] = mid;
	mid        = nodes[6];
	njAddVector(&mid, &nodes[5]);
	gjScaleVector(0.5f, &mid, &mid);
	shapes[16] = mid;
	shapes[20] = nodes[11];
	shapes[24] = nodes[13];
	colliRefreshBounds((CharaColli*)colli);
	colliBoundsOn((CharaColli*)colli);
}

// 0x80114FCC
void TEneOrangeBoss::syncCollider()
{
	orangeBossFitCollider(mCollider, mNodes);
	if (mBossFlags & (BOSS_HURT | BOSS_DOWN)) {
		colliDisable(mCollider);
	} else {
		colliEnable(mCollider);
	}
}

// 0x80115020
void TEneOrangeBoss::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
	desc->getInt(0);
}

// 0x80115064
int TEneOrangeBoss::onHit(const TObjAction* other, int arg, int* result)
{
	return 0;
}

// 0x8011506C
void TEneOrangeBoss::setLookTarget(const Vec3f* pos)
{
	mLookTarget = *pos;
}

// 0x80115088
void TEneOrangeBoss::setSlot(int slot)
{
	mSlot = slot;
	mirrorSlotPos(mSlot, &mSlotPos, &mSlotAngle);
}

// 0x801150B8
void TEneOrangeBoss::setMoveTarget(const Vec3f* pos, int arg)
{
	mMoveTarget = *pos;
	mMoveArg    = arg;
}

// 0x801150D8
void TEneOrangeBoss::setHealth(int health)
{
	mMaxHealth = health;
	mHealth    = health;
}

// 0x801150E4
void TEneOrangeBoss::clearHold()
{
	mBossFlags &= ~BOSS_HOLD;
}

// 0x801150F4
void TEneOrangeBoss::setHold()
{
	mBossFlags |= BOSS_HOLD;
}

// 0x80115104
void TEneOrangeBoss::stageBeaten()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
	hideBossBar();
	seStartAtFar(0x14F, &mBody.mPos);
}

// 0x80115154
void TEneOrangeBoss::hurtReact()
{
	enemyFxSparks(0.0f, &mContact.mSurface, &mContact.mCenter);
	seStartAtFar(0x142, &mBody.mPos);
}

// 0x80115198
void TEneOrangeBoss::footDust()
{
	Vec3f at;
	at.x = mNodes[8].x;
	at.y = mNodes[8].y;
	at.z = mNodes[8].z;

	njAddVector(&at, &mNodes[6]);
	gjScaleVector(0.5f, &at, &at);
	particleOnceByIdDefault(&at, 1, 0x40);
}

// 0x801151FC
void TEneOrangeBoss::landSplash()
{
	Vec3f at;

	at.x = mNodes[9].x;
	at.y = mNodes[9].y;
	at.z = mNodes[9].z;
	at.y = 50.0f;
	particleOnceByIdDefault(&at, 1, 0x41);
}

// 0x80115248
void TEneOrangeBoss::clearLand()
{
	mLand = 0;
}

// 0x80115254
int TEneOrangeBoss::isLandClear()
{
	return mLand == 0;
}

// 0x80115264
void TEneOrangeBoss::setLand()
{
	mLand = 1;
}

// 0x80115270
int TEneOrangeBoss::takeLand()
{
	if (mLand == 1) {
		mLand = 2;
		return 1;
	}
	return 0;
}

// 0x80115294
void TEneOrangeBoss::killBalls()
{
	int i;

	for (i = 8; i >= 0; i--) {
		if (mSparks[i] != 0) {
			mSparks[i]->mFlags |= 1;
			mSparks[i] = 0;
		}
	}
}

// 0x801152D0
int TEneOrangeBoss::isAction0Done()
{
	return isActionDone(0);
}

// 0x801152F4
int TEneOrangeBoss::isAction4Done()
{
	return isActionDone(4);
}

// 0x80115318
int TEneOrangeBoss::isAction2Done()
{
	return isActionDone(2);
}

// 0x8011533C
u32 TEneOrangeBoss::isReady()
{
	return mBossFlags & BOSS_READY;
}

// 0x80115348
int TEneOrangeBoss::isAction5Done()
{
	return isActionDone(5);
}

// 0x8011536C
int TEneOrangeBoss::isAction6Done()
{
	return isActionDone(6);
}

// 0x80115390
int TEneOrangeBoss::isAction8Done()
{
	return isActionDone(8);
}

// 0x801153B4
int TEneOrangeBoss::isAction10Done()
{
	return isActionDone(10);
}

// 0x801153D8
void TEneOrangeBoss::setFacing(const Vec3f* pos, int arg, u16 id)
{
	mLookTarget = *pos;
	mFacingArg  = arg;
	mFacingId   = id;
}

// 0x801153FC
int TEneOrangeBoss::isAction12Done()
{
	return isActionDone(12);
}

// 0x80115420
int TEneOrangeBoss::isAction13Done()
{
	return isActionDone(13);
}

// 0x80115444
int TEneOrangeBoss::isAction14Done()
{
	return isActionDone(14);
}

// 0x80115468
// Sends a punch out of the middle mirror
void TEneOrangeBoss::throwPunch()
{
	Vec3f from;
	Vec3f dir;
	TObjAction* center;

	if (isPunchOut()) {
		return;
	}
	center = findObjById((u16)mirrorCenterId());
	if (center != 0) {
		from = *center->getPos();
	} else {
		from.x = 0.0f;
		from.y = 50.0f;
		from.z = 0.0f;
	}
	njSinCos(mBody.mRot.y + 0x7000, &dir.x, &dir.z);
	dir.y = 0.0f;
	gjMulAddVector(-50.0f, &from, &from, &dir);
	mBossFlags |= BOSS_PUNCH_OUT;
	new TEneOrangeBossPunch(gTL_04, &from, &dir);
}

// 0x80115550
void TEneOrangeBoss::clearPunchOut()
{
	mBossFlags &= ~BOSS_PUNCH_OUT;
}

// 0x80115560
int TEneOrangeBoss::isPunchOut()
{
	return mBossFlags & BOSS_PUNCH_OUT;
}

// 0x8011556C
int TEneOrangeBoss::isAction15Done()
{
	return isActionDone(15);
}

// 0x80115590
int TEneOrangeBoss::isAction17Done()
{
	return isActionDone(17);
}

// 0x801155B4
int TEneOrangeBoss::isAction18Done()
{
	return isActionDone(18);
}

// 0x801155D8
int TEneOrangeBoss::dieAction()
{
	return 19;
}

// 0x801155E0
void TEneOrangeBoss::updateBody()
{
	if (!isBlocked() && takeHits(&mHit, &mContact)) {
		enemyHitPushAway((Vec3f*)&mHit, &mBody.mPos, (const Vec3f*)&mContact, &mHitDir);
		applyDamage(&mHit);
	}
	mBrain->update(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	mTint.update();
	if (mBossFlags & 0x100) {
		Vec3f flat = mNodes[1];
		flat.y = 0.0f;
		if (njScalor(&flat) < 220.0f) {
			particleKeepAt((void**)&mGlow[2], (const Vec3f*)&mNodes[1], 1, 0x47);
		} else {
			if (mGlow[2] != 0) {
				mGlow[2]->mFlags |= 1;
				mGlow[2] = 0;
			}
		}
	} else {
		if (mGlow[2] != 0) {
			mGlow[2]->mFlags |= 1;
			mGlow[2] = 0;
		}
	}
	syncCollider();
	setBossBar(mMaxHealth, mHealth);
}

// 0x80115758
void TEneOrangeBoss::vfn_6C()
{
	mEnemyFlags &= ~ENEMY_APPEARING;
	beginNodes();
	gjPushUnitMatrix();
	localMatrix();
	poseFirstFrame(sOrangeBossMotion, 0x1F, mPose);
	njPopMatrixEx();
	endNodes();
	calcPoints();
	syncCollider();
}

// 0x801157D0
void TEneOrangeBoss::makeBrain()
{
	mBrain = createOrangeBrain(this);
}

// 0x80115800
void TEneOrangeBoss::beginNodes()
{
	int i;

	for (i = 0; i < 16; i++) {
		mNodes[i].x = 0.0f;
		mNodes[i].y = 0.0f;
		mNodes[i].z = 0.0f;
	}
	mNodes[12].x = 0.0f;
	mNodes[12].y = -10.0f;
	mNodes[12].z = 0.0f;
	mNodes[14].x = 0.0f;
	mNodes[14].y = -10.0f;
	mNodes[14].z = 0.0f;
	nodeTrackBeginPoints(sOrangeBossNodes, mNodes);
}

// 0x80115878
void TEneOrangeBoss::endNodes()
{
	nodeTrackEnd();
}

// 0x80115898
void TEneOrangeBoss::updatePose()
{
	beginNodes();
	gjPushUnitMatrix();
	localMatrix();
	applyPose(mPose, &mAnim);
	njPopMatrixEx();
	if (hatchActive(&mAppearTimer)) {
		hatchPose(&mAppearTimer, (GjActor*)mPose, 25.0f);
	}
	endNodes();
}

// 0x80115908
void TEneOrangeBoss::updatePoseRooted()
{
	MotionNodePose root;
	NJS_MOTION* node = motionCtrlGetMotion(&mAnim);

	motionGetNodePose(&root, *(NJS_OBJECT**)mPose, node, motionCtrlFrame(&mAnim));
	motionPoseGetPos(&root, &mBody.mPos);
	mBody.mRot.x = root.mAng[0];
	beginNodes();
	gjPushUnitMatrix();
	localMatrix();
	gjRotateX(-mBody.mRot.x);
	gjTranslate(-mBody.mPos.x, -mBody.mPos.y, -mBody.mPos.z);
	applyPose(mPose, &mAnim);
	njPopMatrixEx();
	endNodes();
}

// 0x801159C4
void TEneOrangeBoss::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 16; i++) {
		njCalcPointEx(&mNodes[i], &mNodes[i]);
	}
	njPopMatrixEx();
}

// 0x80115A24
int TEneOrangeBoss::getHitKind(const void* touch, HitInfo* info)
{
	return (mBossFlags & BOSS_HOLD) ? 3 : 1;
}

// 0x80115A3C
int TEneOrangeBoss::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x80115A54
void TEneOrangeBoss::localMatrix()
{
	njScale(0, 0.7f, 0.7f, 0.7f);
}

// 0x80115A84
void TEneOrangeBoss::bodyMatrix()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
	gjRotateX(mBody.mRot.x);
}

// 0x80115AC4
// On the floor below it
void TEneOrangeBoss::updateShadow()
{
	Vec3f at;
	Vec3f up;

	at = mBody.mPos;
	at.y = 50.0f;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	shadowSetGround(&mShadow, &at, &up);
	if (mBossFlags & BOSS_HURT) {
		shadowDisable(&mShadow);
	} else if (mBossFlags & BOSS_DOWN) {
		Vec3f d;
		f32 s;
		f32 c;

		d = mBody.mPos;
		njSubVector(&d, &mSlotPos);
		njSinCos(mSlotAngle, &s, &c);
		if (d.x * s + d.z * c <= 0.0f) {
			shadowDisable(&mShadow);
		} else {
			f32 len     = vecLenXZ(&d);
			len         = 2.0f * (len > 22.0f ? 22.0f : len);
			mShadow._30 = len;
			mShadow._34 = len;
			shadowEnable(&mShadow);
		}
	} else {
		mShadow._30 = 44.0f;
		mShadow._34 = 44.0f;
		shadowEnable(&mShadow);
	}
}

// 0x80115C04
void TEneOrangeBoss::gravity(Vec3f* vel)
{
	vel->y += -0.12f;
}

// 0x80115C18
// The move a node made with this frame's pose plus gravity
void TEneOrangeBoss::moveByNode(f32 rate, int node, Vec3f* out)
{
	Vec3f from;
	from.x = mNodes[node].x;
	from.y = mNodes[node].y;
	from.z = mNodes[node].z;
	Vec3f to;

	updatePose();
	gjPushUnitMatrix();
	bodyMatrix();
	njCalcPointEx(&mNodes[node], &to);
	njPopMatrixEx();
	moveAlong(rate, &from, &to, out);
	out->y += -0.12f;
}

// 0x80115CC4
int TEneOrangeBoss::moveBody(Vec3f* vel)
{
	Vec3f push;

	push.x = 0.0f;
	push.y = 0.0f;
	push.z = 0.0f;
	getPush(&push);
	push.y = 0.0f;
	njAddVector(&push, vel);
	njAddVector(&mBody.mPos, &push);
	if (mBody.mPos.y <= 50.0f) {
		mBody.mPos.y = 50.0f;
		mEnemyFlags |= 1;
	}
	updateShadow();
	if (!isAirborne()) {
		vel->y = 0.0f;
	}
	return 0;
}

// 0x80115D7C
void TEneOrangeBoss::fallMove(Vec3f* vel)
{
	gravity(vel);
	moveBody(vel);
}

// 0x80115DBC
void TEneOrangeBoss::slideMove(f32 drag)
{
	gravity(&mBody.mVel);
	mBody.mVel.x = mBody.mVel.x * drag;
	mBody.mVel.z = mBody.mVel.z * drag;
	if (moveBody(&mBody.mVel)) {
		mBody.mVel.x = 0.0f;
		mBody.mVel.z = 0.0f;
	}
}

// 0x80115E38
void TEneOrangeBoss::jumpAt(f32 t)
{
	parabolaPoint(&mJumpFrom, &mBody.mVel, &mBody.mPos, -0.12f, t);
	updateShadow();
}

// 0x80115E80
// Slides from mJumpFrom along its velocity slowing at decel negative until it stops
void TEneOrangeBoss::slideTo(f32 t, f32 decel)
{
	Vec3f dir;
	f32 speed;
	f32 stop;

	dir.x = mBody.mVel.x;
	dir.y = mBody.mVel.y;
	dir.z = mBody.mVel.z;
	dir.y = 0.0f;
	speed = njUnitVector(&dir);
	if (speed < gUnitVectorMin) {
		dir = gVecZero;
	}
	stop = -speed / decel;
	t = t > stop ? stop : t;
	gjMulAddVector(speed * t + t * (0.5f * decel * t), &mBody.mPos, &mJumpFrom, &dir);
	updateShadow();
}

// 0x80115F68
void TEneOrangeBoss::playIdle()
{
	motionCtrlPlay(&mAnim, 7);
	motionCtrlUpdate(&mAnim);
	motionCtrlSetFrame(17.0f, &mAnim);
}

// 0x80115FB0
// How many whole steps of about *step cover dist the step that fits exactly and the launch speed
f32 TEneOrangeBoss::planJump(f32 dist, f32* step, f32* speed)
{
	f32 n = floor(dist / *step);

	*step  = dist / n;
	n = n; // a no-op but the DOL moves n to a saved register here
	*speed = launchSpeedFor(n, 0.0f, -0.12f);
	return n;
}

extern "C" {
extern const int gOrangeSparkNodes[] = {
	1, 10, 9, 11, 13, 8, 6, 2,
	3, 4, 0,
};
}

// 0x80116020
// Walk in then land
void TEneOrangeBoss::keysWalk(f32 total)
{
	f32 fade;
	f32 end;

	end  = total - (f32)ceil(5.0);
	fade = (f32)floor(5.0);
	MotionStep* keys = mKeys;
	motionStepFromEntry(0.0f, &keys[0], 8, sOrangeBossMotionKeys);
	motionStepBlendOut(end - fade, 0.2f, &keys[1], &keys[0]);
	motionStepFromEntry(end, &keys[2], 9, sOrangeBossMotionKeys);
	motionStepEnd(&keys[3]);
}

// 0x801160D8
// Crouch jump land
void TEneOrangeBoss::keysJump(f32 total)
{
	f32 fade;
	f32 fade2;
	f32 crouch;
	f32 land;
	MotionStep* keys;

	fade = (f32)floor(6.6666665f);
	crouch = fade;
	fade = (f32)floor(5.0);
	land = total - 5.0f;
	fade2 = (f32)floor(5.0);
	keys = mKeys;
	motionStepOnce(0.0f, 17.0f, 19.0f, 0.3f, &keys[0], 7);
	motionStepBlendOut(crouch, 0.2f, &keys[1], &keys[0]);
	motionStepFromEntry(crouch + fade, &keys[2], 8, sOrangeBossMotionKeys);
	motionStepBlendOut(land - fade2, 0.2f, &keys[3], &keys[2]);
	motionStepFromEntry(land, &keys[4], 9, sOrangeBossMotionKeys);
	motionStepEnd(&keys[5]);
}

// 0x801161F4
// Like keysJump but a short one skips the flight
void TEneOrangeBoss::keysHop(f32 total)
{
	f32 fade2;
	f32 fade;
	f32 crouch;
	MotionStep* keys;

	fade   = (f32)floor(6.6666665f);
	crouch = fade;
	fade   = (f32)floor(5.0);
	fade2  = (f32)floor(5.0);
	keys   = mKeys;
	motionStepOnce(0.0f, 17.0f, 19.0f, 0.3f, &keys[0], 7);
	fade = crouch + fade;
	if (5.0f + (fade2 + fade) > total) {
		motionStepBlendOut(crouch, 1.0f / ((total - crouch) - 5.0f), &keys[1], &keys[0]);
		keys += 2;
	} else {
		motionStepBlendOut(crouch, 0.2f, &keys[1], &keys[0]);
		motionStepFromEntry(fade, &keys[2], 8, sOrangeBossMotionKeys);
		motionStepBlendOut((total - 5.0f) - fade2, 0.2f, &keys[3], &keys[2]);
		keys += 4;
	}
	motionStepFromEntry(total - 5.0f, &keys[0], 10, sOrangeBossMotionKeys);
	motionStepEnd(&keys[1]);
}

// 0x80116358
// Wind up leap come down
void TEneOrangeBoss::keysThrow(f32 total, f32* t0, f32* t1)
{
	f32 fade;
	f32 up;
	MotionStep* keys;

	fade = (f32)floor(16.666666f);
	up = fade;
	fade = (f32)ceil(10.0);
	keys = mKeys;
	motionStepOnce(0.0f, 19.0f, 24.0f, 0.3f, &keys[0], 3);
	motionStepBlendOut(up, 0.1f, &keys[1], &keys[0]);
	motionStepFromEntry(up + fade, &keys[2], 5, sOrangeBossMotionKeys);
	motionStepBlendOut((total - fade) - 26.666666f, 0.1f, &keys[3], &keys[2]);
	motionStepFromEntry(total - 26.666666f, &keys[4], 6, sOrangeBossMotionKeys);
	motionStepEnd(&keys[5]);
	mRange[0] = 6.6666665f;
	mRange[1] = up;
	mRange[2] = total - 26.666666f;
	mRange[3] = total;
	mFootY    = 0.0f;
	*t0       = up;
	*t1       = total - 26.666666f;
}

// 0x80116490
// A full turn backwards spread over t0..t1
int TEneOrangeBoss::flipAngle(f32 t, f32 t0, f32 t1)
{
	return -rampInt(t, t0, t1, 0x10000);
}

// 0x801164B8
// In the two spans of mRange the jump's start sinks with the foot so the body stays on it
void TEneOrangeBoss::followFoot(f32 t)
{
	if ((mRange[0] <= t && t <= mRange[1]) || (mRange[2] <= t && t <= mRange[3])) {
		mJumpFrom.y -= mNodes[9].y - mFootY;
	}
	mFootY = mNodes[9].y;
}

// 0x8011651C
// On the ring in the direction mMoveArg facing it with the middle as its slot
void TEneOrangeBoss::placeAtSlot()
{
	Vec3f dir;

	mBody.mRot.y = mMoveArg;
	njSinCos(mMoveArg, &dir.x, &dir.z);
	dir.y        = 0.0f;
	mBody.mVel.x = dir.x;
	mBody.mVel.z = dir.z;
	goalScalePos(&mBody.mPos, &dir, -50.0f);
	mSlotPos.x = 0.0f;
	mSlotPos.y = 50.0f;
	mSlotPos.z = 0.0f;
	mSlotAngle = mMoveArg;
}

// 0x801165A0
// Just behind its mirror up off the floor
void TEneOrangeBoss::placeAtMirror()
{
	f32 s;
	f32 c;

	njSinCos(mBody.mRot.y, &s, &c);
	mBody.mPos.y = 100.0f;
	mBody.mPos.x = -5.0f * s;
	mBody.mPos.z = -5.0f * c;
	njAddVector(&mBody.mPos, &mSlotPos);
}

// 0x80116608
void TEneOrangeBoss::spawnFx(int model, int motion)
{
	void* obj = resBankGetObjects(&sOrangeBossBank)[model];
	NJS_MOTION** motions = resBankGetMotion(&sOrangeBossBank, 1);

	mFx = createCamPath(0, 2, obj, motions[motion], 0, 1.0f, 1.0f);
	if (mFx != 0) {
		((TCamera*)mFx)->setCut();
	}
}

// 0x80116698
void TEneOrangeBoss::setAimNode(int node)
{
	mAimSide = 0.0f;
	mAimNode = node;
}

// 0x801166A8
// Which side of its mirror the aimed node is on
f32 TEneOrangeBoss::aimDot()
{
	Vec3f fwd;
	Vec3f d;

	njSinCos(mSlotAngle, &fwd.x, &fwd.z);
	fwd.y = 0.0f;
	d     = mNodes[mAimNode];
	njSubVector(&d, &mSlotPos);
	d.y = 0.0f;
	return vecDotXZ(&fwd, &d);
}

// 0x80116730
// When the node passes through its ring mirror the mirror ripples there
void TEneOrangeBoss::aimRing()
{
	f32 dot = aimDot();

	if (dot * mAimSide < 0.0f) {
		mirrorAimAt(mSlot, &mNodes[mAimNode]);
		seStartAtFar(0x146, &mBody.mPos);
	}
	mAimSide = dot;
}

// 0x801167AC
// The same for the middle mirror
void TEneOrangeBoss::aimCenter()
{
	f32 dot = aimDot();

	if (dot * mAimSide < 0.0f) {
		mirrorCenterAimAt(&mNodes[mAimNode]);
		seStartAtFar(0x146, &mBody.mPos);
	}
	mAimSide = dot;
}

// 0x80116824
// The wide cylinder the spin pushes with
void TEneOrangeBoss::bodyCylOn()
{
	Vec3f* shape = (Vec3f*)colliGetShapes(mBodyCyl);

	shape->x = 0.0f;
	shape->y = 200.0f;
	shape->z = 0.0f;
	colliShapeSetCylRadius((ColliShape*)shape, 330.0f);
	colliShapeSetCylHeight((ColliShape*)shape, 300.0f);
	((u32*)shape)[9] &= ~8;
	colliEnable(mBodyCyl);
	colliRefreshBounds(mBodyCyl);
}

// 0x8011689C
void TEneOrangeBoss::bodyCylOff()
{
	colliDisable(mBodyCyl);
}

// 0x801168C0
void TEneOrangeBoss::onActionPhase(int action, int phase)
{
	switch (action) {
	case 0:
		actAppear(phase);
		break;
	case 1:
		actCircle(phase);
		break;
	case 2:
		actJumpIn(phase);
		break;
	case 3:
		actThrow(phase);
		break;
	case 4:
		actSettle(phase);
		break;
	case 5:
		actHop(phase);
		break;
	case 6:
		actDive(phase);
		break;
	case 7:
		actDaze(phase);
		break;
	case 8:
		actRecover(phase);
		break;
	case 9:
		actFall(phase);
		break;
	case 10:
		actToCenter(phase);
		break;
	case 11:
		actSpin(phase);
		break;
	case 12:
		actLeap(phase);
		break;
	case 13:
		actVault(phase);
		break;
	case 14:
		actPunch(phase);
		break;
	case 15:
		actTurn(phase);
		break;
	case 16:
		actDecoys(phase);
		break;
	case 17:
		actReturn(phase);
		break;
	case 18:
		actHurt(phase);
		break;
	case 19:
		actDie(phase);
		break;
	}
}

extern "C" int sOrangeMotionPicks[3] = {
	20, 23, 26,
};

// 0x801169E8
// Rises out of the middle mirror shows itself then sinks back
void TEneOrangeBoss::actAppear(int phase)
{
	switch (phase) {
	case 0:
		mBody.mPos.x = 0.0f;
		mBody.mPos.y = 0.0f;
		mBody.mPos.z = 0.0f;
		mBody.mRot.x = 0;
		mBody.mRot.y = 0;
		mBody.mRot.z = 0;
		motionCtrlPlay(&mAnim, 0x1E);
		clearFrameKey();
		mActStep  = 0;
		mActTimer = 0;
		shadowDisable(&mShadow);
		clearLand();
		break;
	case 1: {
		int lock = lockOn();
		if (lock != 0) {
			mirrorsHide();
		}
		switch (mActStep) {
		case 0:
			spawnFx(1, 1);
			seStartArg(0x2AA, 0x230);
			mActStep   = 1;
			mActTimer  = 0;
			mSlotPos.x = 0.0f;
			mSlotPos.y = 50.0f;
			mSlotPos.z = 0.0f;
			mSlotAngle = 0;
			mirrorCenterTurn(0);
			mEnemyFlags |= ENEMY_APPEARING;
			mirrorCenterRaise(0);
			break;
		case 1:
			mActTimer = mActTimer + 1;
			if (mActTimer >= 60) {
				mActStep = 2;
				motionCtrlUpdate(&mAnim);
				updatePoseRooted();
				mEnemyFlags &= ~ENEMY_APPEARING;
				mBossFlags |= 0x100;
			}
			break;
		case 2:
			mActTimer = mActTimer + 1;
			if (mActTimer >= 300) {
				mActStep = 3;
				mBossFlags |= 0x16;
				mBossFlags &= ~0x100;
				setAimNode(0);
			}
			motionCtrlUpdate(&mAnim);
			updatePoseRooted();
			if (mActTimer == 0xAF) {
				setLand();
			}
			break;
		case 3:
			mActTimer = mActTimer + 1;
			if (mActTimer >= 480) {
				mActStep = 4;
				mBossFlags &= ~BOSS_DOWN;
				mBossFlags |= BOSS_HURT;
				mirrorCenterLower(0);
			}
			motionCtrlUpdate(&mAnim);
			updatePoseRooted();
			break;
		case 4:
			motionCtrlUpdate(&mAnim);
			updatePoseRooted();
			waitMotion();
			break;
		}
		calcPoints();
		if (mActStep == 3) {
			aimCenter();
		}
		if (takeLand()) {
			footDust();
		}
		break;
	}
	case 2:
		clearFrameKey();
		unlock();
		mEnemyFlags &= ~ENEMY_APPEARING;
		mBossFlags &= ~0x11E;
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		break;
	}
}

// 0x80116CD0
// Runs round the ring
void TEneOrangeBoss::actCircle(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 1);
		clearFrameKey();
		mBossFlags |= 0xA;
		mActTime = 0.0f;
		break;
	case 1: {
		f32 frame;
		f32 x;
		f32 z;
		f32 step;
		int angle;
		f32 sn;
		f32 cs;

		motionCtrlUpdate(&mAnim);
		updatePose();
		frame = motionCtrlFrame(&mAnim);
		if (motionCtrlIsBlending(&mAnim) || (frame >= 10.0f && frame < 23.0f)) {
			step = mActTime - mNodes[6].z;
		} else {
			step = vecLenXZ(&mBody.mVel);
		}
		x     = mBody.mPos.x;
		z     = mBody.mPos.z;
		sn    = x;
		cs    = z;
		angle = (int)(10430.38043493439 * (f32)atan2(x, z));
		angle += (int)(10430.38043493439 * (step / 270.0f));
		njSinCos(angle, &sn, &cs);
		sn           = 270.0f * sn;
		cs           = 270.0f * cs;
		mBody.mRot.y = angle + 0x4000;
		mBody.mVel.x = sn - x;
		mBody.mVel.z = cs - z;
		mActTime     = mNodes[6].z;
		if (motionCtrlPassed(10.0f, &mAnim)) {
			seStartAtFar(0x144, &mBody.mPos);
		}
		moveBody(&mBody.mVel);
		calcPoints();
		break;
	}
	case 2:
		mBossFlags &= ~0xA;
		break;
	}
}

// 0x80116ECC
// Jumps in from the ring at its slot
void TEneOrangeBoss::actJumpIn(int phase)
{
	switch (phase) {
	case 0: {
		Vec3f dir;
		f32 step;
		int angle;

		motionCtrlPlay(&mAnim, 8);
		clearFrameKey();
		mBossFlags |= 0x116;
		clearLand();
		mActStep = 0;
		angle    = mirrorSlotDir(mSlot, &dir);
		goalScalePos(&mBody.mPos, &dir, 270.0f);
		mBody.mRot.y = angle + 0x8000;
		step         = 3.5f;
		mActEnd      = planJump(75.0f, &step, &mBody.mVel.y);
		step        *= -1.0f;
		mBody.mVel.x = dir.x * step;
		mBody.mVel.z = dir.z * step;
		keysWalk(mActEnd);
		mActTime  = -1.0f;
		mJumpFrom = mBody.mPos;
		break;
	}
	case 1:
		mActTime += 1.0f;
		if (motionStepsPlay(mActTime, mKeys, &mMotion)) {
			mHitFlags |= 0x20;
		}
		updatePose();
		switch (mActStep) {
		case 0:
			jumpAt(mActTime);
			if (mActTime >= mActEnd) {
				mActStep     = 1;
				mBody.mVel.y = 0.0f;
				seStartAtFar(0x145, &mBody.mPos);
			}
			break;
		case 1:
			if (isLandClear() && motionCtrlFrame(&mAnim) >= 4.0f) {
				setLand();
			}
			slideMove(0.85f);
			break;
		}
		calcPoints();
		if (takeLand()) {
			footDust();
		}
		break;
	case 2:
		mBossFlags &= ~0x116;
		break;
	}
}

// 0x801170F8
// The foot the throw walk steps with or -1
int throwFoot(void* anim)
{
	f32 frame = motionCtrlFrame(anim);

	if (motionCtrlIsBlending(anim)) {
		return -1;
	}
	if (frame < 7.0f) {
		return 6;
	}
	if (frame < 11.0f) {
		return -1;
	}
	if (frame < 17.0f) {
		return 8;
	}
	return -1;
}

// 0x80117190
// Where the ball sits through the throw the left hand then over to the right then let go
// Returns 1 when *step changes
int throwHand(int* step, void* anim, Vec3f* nodes, Vec3f* out)
{
	int changed = 0;
	f32 frame   = motionCtrlFrame(anim);

	switch (*step) {
	case 0:
		if (!motionCtrlIsBlending(anim)) {
			changed = 1;
			*step   = 1;
		}
		break;
	case 1:
		if (frame <= 18.0f && frame >= 5.0f) {
			changed = 1;
			*step   = 2;
			*out    = nodes[12];
		}
		break;
	case 2:
		*out = nodes[12];
		if (frame >= 13.0f) {
			changed = 1;
			*step   = 3;
		}
		break;
	case 3: {
		f32 t = frame - 13.0f;
		t /= 3.0f;
		if (t > 1.0f) {
			t = 1.0f;
		}
		vecLerp(t, &nodes[12], &nodes[14], out);
		if (frame >= 16.0f) {
			changed = 1;
			*step   = 4;
		}
		break;
	}
	case 4:
		*out = nodes[14];
		if (frame >= 18.0f || frame < 16.0f) {
			changed = 1;
			*step   = 1;
		}
		break;
	}
	return changed;
}

// 0x80117348
int throwStepFrame(void* anim)
{
	u8 hit = 0;

	if (motionCtrlPassed(10.0f, anim) || motionCtrlPassed(0.0f, anim)) {
		hit = 1;
	}
	return hit;
}

// 0x801173A0
// Walks out from its mirror throwing balls at the player
void TEneOrangeBoss::actThrow(int phase)
{
	int* act = &mActStep;

	switch (phase) {
	case 0: {
		Vec3f dir;

		motionCtrlPlayBlend(0.2f, &mAnim, 0x12);
		mBossFlags |= 0x100;
		mBossFlags &= ~BOSS_READY;
		clearFrameKey();
		mActTime = 1000000.0f;
		mirrorSlotDir(mSlot, &dir);
		goalScalePos(&mMoveTarget, &dir, 120.0f);
		*act = 0;
		mBall    = 0;
		break;
	}
	case 1: {
		Vec3f hand;
		Vec3f d;
		int node;
		int step;
		int changed;
		f32 len;

		motionCtrlUpdate(&mAnim);
		motionCtrlFrame(&mAnim);
		faceToward(&mMoveTarget, &mBody.mPos, (void*)0x300);
		node = throwFoot(&mAnim);
		if (node >= 0) {
			moveByNode(0.7f, node, &mBody.mVel);
		} else {
			updatePose();
		}
		moveBody(&mBody.mVel);
		if (throwStepFrame(&mAnim)) {
			seStartAtFar(0x144, &mBody.mPos);
		}
		calcPoints();
		step    = *act;
		changed = throwHand(act, &mAnim, mNodes, &hand);
		switch (step) {
		case 0:
			break;
		case 1:
			if (changed) {
				mBall = new TEneOrangeBossBall(this, &hand);
			}
			break;
		case 2:
			mBall->setPos(&hand);
			break;
		case 3:
			mBall->setPos(&hand);
			break;
		case 4:
			mBall->setPos(&hand);
			if (changed) {
				mBall->throwAt(&mLookTarget);
				mBall = 0;
				seStartAtFar(0x147, &hand);
			}
			break;
		}
		d = mMoveTarget;
		njSubVector(&d, &mBody.mPos);
		len = vecLenXZ(&d);
		if (len < 3.0f) {
			mBossFlags |= BOSS_READY;
		} else {
			mBossFlags &= ~BOSS_READY;
		}
		mActTime = len;
		break;
	}
	case 2:
		mBossFlags &= ~0x120;
		if (mBall != 0) {
			mBall->throwAt(&mLookTarget);
			mBall = 0;
		}
		break;
	}
}

// 0x8011762C
// Comes to a stop facing its mirror
void TEneOrangeBoss::actSettle(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.3f, &mAnim, 7);
		clearFrameKey();
		mBossFlags |= 0x116;
		mBody.mRot.y = mirrorSlotAngle(mSlot);
		seStartAtFar(0x145, &mBody.mPos);
		break;
	case 1: {
		f32 frame;

		motionCtrlUpdate(&mAnim);
		updatePose();
		slideMove(0.9f);
		frame = motionCtrlFrame(&mAnim);
		frame += 0.3f;
		if (!motionCtrlIsBlending(&mAnim) && frame >= 17.0f) {
			mHitFlags |= 0x20;
		}
		calcPoints();
		break;
	}
	case 2:
		mBossFlags &= ~0x116;
		break;
	}
}

// 0x8011773C
// Hops back out to the ring at its slot
void TEneOrangeBoss::actHop(int phase)
{
	switch (phase) {
	case 0: {
		Vec3f d;
		f32 dist;
		f32 step;

		playIdle();
		clearFrameKey();
		mBossFlags |= 0x116;
		mBody.mRot.y = mirrorSlotAngle(mSlot);
		mActStep     = 0;
		mirrorSlotDir(mSlot, &d);
		goalScalePos(&mMoveTarget, &d, 270.0f);
		d = mMoveTarget;
		njSubVector(&d, &mBody.mPos);
		d.y  = 0.0f;
		dist = njUnitVector(&d);
		if (dist < gUnitVectorMin) {
			d = gVecZero;
		}
		step         = 3.5f;
		mActEnd      = planJump(dist, &step, &mBody.mVel.y);
		mBody.mVel.x = d.x * step;
		mBody.mVel.z = d.z * step;
		keysJump(mActEnd);
		mActTime  = -1.0f;
		mJumpFrom = mBody.mPos;
		setAimNode(10);
		seStartAtFar(0x149, &mBody.mPos);
		break;
	}
	case 1: {
		if (mActStep != 2) {
			mActTime += 1.0f;
			if (motionStepsPlay(mActTime, mKeys, &mMotion)) {
				mHitFlags |= 0x20;
				motionCtrlPlayBlend(0.1f, &mAnim, 0);
				mActStep = 2;
			}
		} else {
			motionCtrlUpdate(&mAnim);
		}
		updatePose();
		switch (mActStep) {
		case 0:
			jumpAt(mActTime);
			if (mActTime >= mActEnd) {
				mActStep     = 1;
				mBody.mVel.y = 0.0f;
				seStartAtFar(0x145, &mBody.mPos);
			}
			break;
		case 1:
		case 2:
			slideMove(0.85f);
			break;
		}
		calcPoints();
		aimRing();
		break;
	}
	case 2:
		mBossFlags &= ~0x116;
		break;
	}
}

// 0x801179A8
// A low dive along the floor towards its slot
void TEneOrangeBoss::actDive(int phase)
{
	switch (phase) {
	case 0: {
		Vec3f d;
		f32 dist;
		f32 step;

		playIdle();
		mBossFlags |= 0x100;
		clearFrameKey();
		mBody.mRot.y = mirrorSlotAngle(mSlot);
		clearLand();
		mActStep = 0;
		mirrorSlotDir(mSlot, &d);
		goalScalePos(&mMoveTarget, &d, 220.0f);
		d = mMoveTarget;
		njSubVector(&d, &mBody.mPos);
		d.y  = 0.0f;
		dist = njUnitVector(&d);
		if (dist < gUnitVectorMin) {
			d = gVecZero;
		}
		dist        -= 29.0f;
		step         = 2.5f;
		mActEnd      = planJump(dist, &step, &mBody.mVel.y);
		mBody.mVel.x = d.x * step;
		mBody.mVel.z = d.z * step;
		_5A0         = (f32)ceil(2.5);
		_5A4         = (f32)ceil(25.0);
		_5A0         = _5A0 + mActEnd;
		_5A4         = _5A4 + mActEnd;
		keysHop(mActEnd);
		mActTime  = -1.0f;
		mJumpFrom = mBody.mPos;
		seStartAtFar(0x149, &mBody.mPos);
		break;
	}
	case 1: {
		mActTime += 1.0f;
		if (motionStepsPlay(mActTime, mKeys, &mMotion)) {
			mHitFlags |= 0x20;
		}
		updatePose();
		if ((s16)motionCtrlCurrent(&mAnim) == 10) {
			setFrameKey(sOrangeBossFrameKeys[0]);
		}
		switch (mActStep) {
		case 0:
			jumpAt(mActTime);
			if (mActTime >= mActEnd) {
				mActStep     = 1;
				mBody.mVel.y = 0.0f;
			}
			break;
		case 1:
			if (mActTime >= _5A0) {
				mActStep     = 2;
				mBody.mVel.y = 0.0f;
				if (njUnitVector(&mBody.mVel) < gUnitVectorMin) {
					mBody.mVel = gVecZero;
				}
				mBody.mVel.x *= -2.5f;
				mBody.mVel.z *= -2.5f;
				setLand();
				seStartAtFar(0x14B, &mBody.mPos);
			}
			break;
		case 2:
			fallMove(&mBody.mVel);
			if (mActTime >= _5A4) {
				mActStep = 3;
				setLand();
			}
			break;
		case 3:
			slideMove(0.9f);
			break;
		}
		calcPoints();
		if (takeLand()) {
			if (mActStep == 2) {
				Vec3f at;
				at.x = mNodes[12].x;
				at.y = mNodes[12].y;
				at.z = mNodes[12].z;
				njAddVector(&at, &mNodes[14]);
				gjScaleVector(0.5f, &at, &at);
				at.y = mNodes[10].y;
				particleOnceByIdDefault(&at, 1, 0x43);
				enemyFxBurst4E(&at);
			} else {
				landSplash();
			}
		}
		break;
	}
	case 2:
		mBossFlags &= ~0x100;
		clearFrameKey();
		break;
	}
}

// 0x80117D84
// Dazed with stars over its head
void TEneOrangeBoss::actDaze(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0xF);
		mFrameKey = 3;
		if (mGlow[0] != 0) {
			mGlow[0]->mFlags |= 1;
			mGlow[0] = 0;
		}
		break;
	case 1: {
		Vec3f at;

		motionCtrlUpdate(&mAnim);
		updatePose();
		slideMove(0.8f);
		calcPoints();
		at.x = mNodes[10].x;
		at.y = mNodes[10].y;
		at.z = mNodes[10].z;
		at.y += 15.0f;
		particleKeepAt((void**)&mGlow[0], &at, 1, 0x42);
		break;
	}
	case 2:
		if (mGlow[0] != 0) {
			mGlow[0]->mFlags |= 1;
			mGlow[0] = 0;
		}
		clearFrameKey();
		break;
	}
}

// 0x80117E94
// Gets back up
void TEneOrangeBoss::actRecover(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0x1D);
		mBaseAngle = mBody.mRot.y;
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		waitMotion();
		updatePose();
		fallMove(&mBody.mVel);
		calcPoints();
		if (motionCtrlPassed(14.0f, &mAnim)) {
			seStartAtFar(0x149, &mBody.mPos);
		} else if (motionCtrlPassed(18.0f, &mAnim)) {
			seStartAtFar(0x145, &mBody.mPos);
		}
		break;
	case 2:
		break;
	}
}

// 0x80117F6C
// Stands at a slot falling to the floor
void TEneOrangeBoss::actFall(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0);
		placeAtSlot();
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		clearFrameKey();
		mBossFlags |= 0xA;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		fallMove(&mBody.mVel);
		calcPoints();
		break;
	case 2:
		mBossFlags &= ~0xA;
		break;
	}
}

// 0x80118030
f32 TEneOrangeBoss::planWalkJump()
{
	f32 step;
	f32 n;

	placeAtSlot();
	mBody.mRot.x = 0;
	step         = 3.5f;
	n            = planJump(90.0f, &step, &mBody.mVel.y);
	mBody.mVel.x = mBody.mVel.x * step;
	mBody.mVel.z = mBody.mVel.z * step;
	keysJump(n);
	return n;
}

// 0x801180C0
f32 TEneOrangeBoss::keysLand()
{
	f32 b;
	f32 c;
	f32 ab;
	f32 a;
	MotionStep* keys;

	mBody.mVel.y = 0.0f;
	mJumpFrom    = mBody.mPos;
	b = (f32)floor(15.0);
	a = b;
	b = (f32)floor(10.0);
	c = (f32)ceil(10.0);
	keys = mKeys;
	motionStepOnce(0.0f, 2.0f, 8.0f, 0.4f, &keys[0], 9);
	motionStepBlendOut(a, 0.1f, &keys[1], &keys[0]);
	ab = a + b;
	motionStepOnce(ab, 19.0f - 0.3f * c, 19.0f, 0.3f, &keys[2], 3);
	motionStepEnd(&keys[3]);
	return c + ab;
}

// 0x801181C0
// A leap from the middle back over the ring
f32 TEneOrangeBoss::planLeap(f32* t0, f32* t1)
{
	f32 up;
	f32 a;
	f32 b;
	f32 speed;
	Vec3f flat;

	mJumpFrom    = mBody.mPos;
	mBody.mRot.x = 0;
	up           = speedForJumpHeight(-0.12f, 130.0f);
	mBody.mVel.y = up;
	ballisticRoots(-0.12f, up, 100.0f, &a, &b);
	a = a > b ? a : b;
	flat.x = mBody.mPos.x;
	flat.y = mBody.mPos.y;
	flat.z = mBody.mPos.z;
	flat.y = 0.0f;
	f32 d = vecLenXZ(&flat);
	d += 5.0f;
	speed = d / a;
	njSinCos(mBody.mRot.y, &mBody.mVel.x, &mBody.mVel.z);
	mBody.mVel.x = mBody.mVel.x * -speed;
	mBody.mVel.z = mBody.mVel.z * -speed;
	keysThrow(a, t0, t1);
	return a;
}

// 0x801182E4
// Jumps into the middle lands then leaps back out flipping over
void TEneOrangeBoss::actToCenter(int phase)
{
	f32* t0 = &_5A0;
	f32* t1 = &_5A4;

	switch (phase) {
	case 0:
		playIdle();
		clearFrameKey();
		mBossFlags |= 0x116;
		clearLand();
		setAimNode(10);
		mActStep  = 0;
		mActEnd   = planWalkJump();
		mActTime  = -1.0f;
		mActTimer = 0;
		mJumpFrom = mBody.mPos;
		seStartAtFar(0x149, &mBody.mPos);
		break;
	case 1: {
		int again;

		mActTime += 1.0f;
		do {
			again = 0;
			switch (mActStep) {
			case 0:
				if (mActTime >= mActEnd) {
					jumpAt(mActEnd);
					mActStep = 1;
					again    = 1;
					mBossFlags &= ~0x16;
					mActTime = mActTime - mActEnd;
					mActEnd  = keysLand();
					seStartAtFar(0x145, &mBody.mPos);
				} else {
					motionStepsPlay(mActTime, mKeys, &mMotion);
					updatePose();
					jumpAt(mActTime);
				}
				break;
			case 1:
				if (mActTime >= mActEnd) {
					slideTo(mActEnd, -0.2f);
					mActStep = 2;
					again    = 1;
					mActTime = mActTime - mActEnd;
					mActEnd  = planLeap(t0, t1);
					seStartAtFar(0x14A, &mBody.mPos);
				} else {
					motionStepsPlay(mActTime, mKeys, &mMotion);
					updatePose();
					slideTo(mActTime, -0.2f);
					if (isLandClear() && motionCtrlFrame(&mAnim) >= 4.0f) {
						setLand();
					}
				}
				break;
			case 2:
				if (mActTime >= mActEnd) {
					jumpAt(mActEnd);
					mActStep = 3;
					again    = 1;
					mActTime = mActTime - mActEnd;
					seStartAtFar(0x145, &mBody.mPos);
				} else {
					mBody.mRot.x = flipAngle(mActTime, *t0, *t1);
					motionStepsPlay(mActTime, mKeys, &mMotion);
					updatePose();
					followFoot(mActTime);
					jumpAt(mActTime);
				}
				break;
			case 3:
				motionCtrlUpdate(&mAnim);
				updatePose();
				waitMotion();
				break;
			}
		} while (again);
		calcPoints();
		if (mActStep == 0) {
			aimCenter();
		}
		if (takeLand()) {
			footDust();
		}
		break;
	}
	case 2:
		mBody.mRot.x = 0;
		mBossFlags &= ~0x116;
		break;
	}
}

// 0x8011863C
// Spins in front of the middle mirror pushing away whoever comes close in front
void TEneOrangeBoss::actSpin(int phase)
{
	switch (phase) {
	case 0: {
		int a;
		int b;

		pickMotions(&a, &b);
		motionCtrlPlayBlend(0.2f, &mAnim, (s16)a);
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		placeAtMirror();
		clearFrameKey();
		killBalls();
		bodyCylOn();
		mBaseAngle = 0;
		break;
	}
	case 1: {
		struct PushMsg {
			s16 mKind;
			Vec3f mPush;
		};
		PushMsg msg;
		Vec3f d;
		Vec3f fwd;
		f32 push;
		TObjAction* other;
		int hit;
		ColliContact* contact;
		int i;

		motionCtrlUpdate(&mAnim);
		updatePose();
		turnTo(mFacingArg, 0x180);
		mirrorCenterTurn(mBody.mRot.y);
		msg.mKind = 1;
		njSinCos(mBody.mRot.y, &fwd.x, &fwd.z);
		fwd.y = 0.0f;
		push  = -gOrangeBossParams->mPush;
		push *= 0.2f * njSin(mBaseAngle) + 1.0f;
		mBaseAngle += 0x600;
		for (contact = ((CharaColli*)mBodyCyl)->mContacts; contact != 0; contact = contact->mNext) {
			if (colliContactNoBit10(contact)) {
				other = findObjById(colliContactOwner(contact));
				if (other != 0 && (isPlayerObj(other) || isEggObj(other))) {
					d = *other->getPos();
					f32 along;
					njSubVector(&d, &mBody.mPos);
					d.y   = 0.0f;
					along = d.x * fwd.x + d.z * fwd.z;
					if (!(along <= 0.0f)) {
						Vec3f side;
						f32 dist;
						gjMulAddVector(-along, &side, &d, &fwd);
						dist = njUnitVector(&side);
						if (dist < gUnitVectorMin) {
							side = gVecZero;
						}
						hit = 0;
						if (dist <= 50.0f) {
							hit = 1;
						} else if (angleDist(mBody.mRot.y, (int)(10430.38043493439 * (f32)atan2(d.x, d.z))) <= 0xE38) {
							hit = 1;
						}
						if (hit) {
							if (njUnitVector(&d) < gUnitVectorMin) {
								d = gVecZero;
							}
							gjScaleVector(push, &d, &d);
							along = d.x * side.x + d.z * side.z;
							along *= -0.15f;
							gjMulAddVector(along, &msg.mPush, &d, &side);
							other->sendToHandle((ObjMsg*)&msg);
						}
					}
				}
			}
		}
		calcPoints();
		for (i = 0; i < 9; i++) {
			particleKeepAt((void**)&mSparks[i], (const Vec3f*)(&mNodes[gOrangeSparkNodes[i]]), 1, 0x4B);
			if (mSparks[i] != 0) {
				((TParticleGenerator*)mSparks[i])->setScale(10.0f);
			}
		}
		break;
	}
	case 2:
		bodyCylOff();
		killBalls();
		break;
	}
}

// 0x801189D0
// A fall from up behind the mirror forward through it
f32 TEneOrangeBoss::planFall(f32* t0, f32* t1)
{
	f32 a;
	f32 b;
	f32 crouch;
	f32 fade;
	MotionStep* keys;

	placeAtMirror();
	mJumpFrom    = mBody.mPos;
	mBody.mVel.x = 0.0f;
	mBody.mVel.y = 0.0f;
	mBody.mVel.z = 0.0f;
	njSinCos(mBody.mRot.y, &mBody.mVel.x, &mBody.mVel.z);
	mBody.mForce = mBody.mVel;
	gjScaleVector(-1.0f, &mBody.mForce, &mBody.mForce);
	mBody.mVel.x *= -0.8f;
	mBody.mVel.z *= -0.8f;
	ballisticRoots(-0.12f, 0.0f, 50.0f - mBody.mPos.y, &a, &b);
	a = a > b ? a : b;
	*t0    = 55.0f;
	*t1    = 57.5f;
	a      = a + *t0;
	fade   = (f32)floor(60.0);
	crouch = fade;
	fade   = (f32)floor(5.0);
	keys = mKeys;
	motionStepFromEntry(0.0f, &keys[0], 0xC, sOrangeBossMotionKeys);
	motionStepBlendOut(crouch, 0.2f, &keys[1], &keys[0]);
	motionStepFromEntry(crouch + fade, &keys[2], 0xD, sOrangeBossMotionKeys);
	motionStepBlendOut((a - 3.3333333f) - fade, 0.2f, &keys[3], &keys[2]);
	motionStepFromEntry(a - 3.3333333f, &keys[4], 0xE, sOrangeBossMotionKeys);
	motionStepEnd(&keys[5]);
	return a;
}

// 0x80118BAC
// Climbs out of its mirror and drops to the floor
void TEneOrangeBoss::actLeap(int phase)
{
	f32* t0 = &_5A0;
	f32* t1 = &_5A4;

	switch (phase) {
	case 0:
		mBossFlags |= 0x100;
		motionCtrlPlayBlend(0.2f, &mAnim, 0xC);
		clearFrameKey();
		clearLand();
		mActStep = 0;
		_5A8     = planFall(t0, t1);
		mActEnd  = -1.0f;
		mActTime = 0.0f;
		break;
	case 1: {
		s16 motion;

		mActEnd += 1.0f;
		if (motionStepsPlay(mActEnd, mKeys, &mMotion)) {
			mHitFlags |= 0x20;
		}
		updatePose();
		motion = (s16)motionCtrlCurrent(&mAnim);
		switch (motion) {
		case 0xC:
			setFrameKey(sOrangeBossFrameKeys[1]);
			break;
		case 0xD:
			mFrameKey = 3;
			break;
		case 0xE:
			setFrameKey(sOrangeBossFrameKeys[2]);
			break;
		}
		switch (mActStep) {
		case 0:
			if (mActEnd >= *t0 && mActEnd <= *t1) {
				f32 d = mNodes[8].z - mActTime;
				mJumpFrom.x = mBody.mForce.x * d + mJumpFrom.x;
				mJumpFrom.z = mBody.mForce.z * d + mJumpFrom.z;
			}
			if (*t0 <= mActEnd && mActEnd <= 1.0f + *t0) {
				seStartAtFar(0x14B, &mBody.mPos);
			}
			if (mActEnd >= *t0) {
				jumpAt(mActEnd - *t0);
			}
			if (mActEnd >= _5A8) {
				mActStep     = 1;
				mBody.mPos.y = 50.0f;
				mBody.mVel.y = 0.0f;
				mJumpFrom    = mBody.mPos;
				setLand();
			}
			break;
		case 1:
			slideTo(mActEnd - _5A8, -0.05f);
			break;
		}
		mActTime = mNodes[8].z;
		calcPoints();
		if (takeLand()) {
			landSplash();
		}
		break;
	}
	case 2:
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		mBossFlags &= ~0x100;
		clearFrameKey();
		break;
	}
}

// 0x80118E74
f32 TEneOrangeBoss::planCrouch()
{
	f32 b;
	f32 a;
	MotionStep* keys;

	placeAtMirror();
	mJumpFrom    = mBody.mPos;
	mBody.mRot.x = 0;
	b = (f32)floor(10.0);
	a = b;
	b = (f32)floor(63.333332f);
	keys = mKeys;
	motionStepBlend(0.0f, 0.1f, &keys[0], &mMotion);
	motionStepFromEntry(a, &keys[1], 3, sOrangeBossMotionKeys);
	motionStepEnd(&keys[2]);
	return a + b;
}

// 0x80118F38
f32 TEneOrangeBoss::planVault(f32* t0, f32* t1)
{
	f32 up;
	f32 a;
	f32 b;
	f32 speed;

	up           = speedForJumpHeight(-0.12f, 30.0f);
	mBody.mVel.y = up;
	ballisticRoots(-0.12f, up, 50.0f - mJumpFrom.y, &a, &b);
	a = a > b ? a : b;
	speed = 50.0f / a;
	njSinCos(mBody.mRot.y, &mBody.mVel.x, &mBody.mVel.z);
	mBody.mVel.x = mBody.mVel.x * speed;
	mBody.mVel.z = mBody.mVel.z * speed;
	keysThrow(a, t0, t1);
	return a;
}

// 0x80119010
// Vaults out over the top of its mirror flipping and turning round
void TEneOrangeBoss::actVault(int phase)
{
	f32* t0 = &_5A0;
	f32* t1 = &_5A4;

	switch (phase) {
	case 0:
		mBossFlags |= 0x100;
		motionCtrlPlayBlend(0.2f, &mAnim, 0xC);
		clearFrameKey();
		clearLand();
		mActStep   = 0;
		mActEnd    = planCrouch();
		mActTime   = -1.0f;
		mBaseAngle = mBody.mRot.y;
		break;
	case 1: {
		int again;

		mActTime += 1.0f;
		do {
			again = 0;
			switch (mActStep) {
			case 0:
				if (mActTime >= mActEnd) {
					mActStep   = 1;
					again      = 1;
					mActTime   = mActTime - mActEnd;
					mActEnd    = planVault(t0, t1);
					mBaseAngle = mBody.mRot.y;
					seStartAtFar(0x14A, &mBody.mPos);
				} else {
					motionStepsPlay(mActTime, mKeys, &mMotion);
					updatePose();
				}
				break;
			case 1:
				if (mActTime >= mActEnd) {
					mActStep     = 2;
					mBody.mVel.y = 0.0f;
					again        = 1;
					mBody.mPos.y = 50.0f;
					mBody.mRot.y = mBaseAngle + 0x8000;
					mBody.mRot.x = 0;
					seStartAtFar(0x145, &mBody.mPos);
				} else {
					int turn;
					motionStepsPlay(mActTime, mKeys, &mMotion);
					updatePose();
					jumpAt(mActTime);
					mBody.mRot.x = flipAngle(mActTime, *t0, *t1);
					turn         = flipAngle(mActTime, *t0, mRange[3]);
					mBody.mRot.y = mBaseAngle + turn / 2;
					followFoot(mActTime);
				}
				break;
			case 2:
				if (motionStepsPlay(mActTime, mKeys, &mMotion)) {
					mHitFlags |= 0x20;
				}
				updatePose();
				if (isLandClear() && motionCtrlFrame(&mAnim) >= 8.0f) {
					setLand();
				}
				break;
			}
		} while (again);
		calcPoints();
		if (takeLand()) {
			footDust();
		}
		break;
	}
	case 2:
		mBossFlags &= ~0x100;
		break;
	}
}

// 0x801192C4
// Punches through the middle mirror
void TEneOrangeBoss::actPunch(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0x13);
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		clearFrameKey();
		if (mGlow[0] != 0) {
			mGlow[0]->mFlags |= 1;
			mGlow[0] = 0;
		}
		if (mGlow[1] != 0) {
			mGlow[1]->mFlags |= 1;
			mGlow[1] = 0;
		}
		break;
	case 1: {
		f32 frame;

		motionCtrlUpdate(&mAnim);
		waitMotion();
		updatePose();
		fallMove(&mBody.mVel);
		calcPoints();
		setFrameKey(sOrangeBossFrameKeys[3]);
		frame = motionCtrlFrame(&mAnim);
		if (frame >= 8.0f && frame <= 24.0f) {
			particleKeepAt((void**)&mGlow[0], (const Vec3f*)&mNodes[15], 1, 0x45);
		} else {
			if (mGlow[0] != 0) {
				mGlow[0]->mFlags |= 1;
				mGlow[0] = 0;
			}
		}
		if (frame >= 16.0f && frame <= 30.0f) {
			particleKeepAt((void**)&mGlow[1], (const Vec3f*)&mNodes[15], 1, 0x46);
		} else {
			if (mGlow[1] != 0) {
				mGlow[1]->mFlags |= 1;
				mGlow[1] = 0;
			}
		}
		if (motionCtrlPassed(28.0f, &mAnim)) {
			seStartAtFar(0x14C, &mNodes[15]);
		}
		if (motionCtrlPassed(30.0f, &mAnim)) {
			throwPunch();
			mirrorCenterSetFlag40();
		}
		break;
	}
	case 2:
		clearFrameKey();
		if (mGlow[0] != 0) {
			mGlow[0]->mFlags |= 1;
			mGlow[0] = 0;
		}
		if (mGlow[1] != 0) {
			mGlow[1]->mFlags |= 1;
			mGlow[1] = 0;
		}
		break;
	}
}

// 0x80119510
// Turns to face out of its mirror
void TEneOrangeBoss::actTurn(int phase)
{
	switch (phase) {
	case 0:
		mBossFlags |= 0x100;
		motionCtrlPlayBlend(0.2f, &mAnim, 0x11);
		mBaseAngle = mBody.mRot.y;
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		waitMotion();
		updatePose();
		if (!motionCtrlIsBlending(&mAnim)) {
			f32 frame    = motionCtrlFrame(&mAnim);
			mBody.mRot.y = mBaseAngle + rampInt(frame, 12.0f, 19.0f, angleDiff(mBaseAngle, mSlotAngle));
		}
		fallMove(&mBody.mVel);
		calcPoints();
		if (motionCtrlPassed(13.0f, &mAnim)) {
			seStartAtFar(0x149, &mBody.mPos);
		} else if (motionCtrlPassed(20.0f, &mAnim)) {
			seStartAtFar(0x145, &mBody.mPos);
		}
		break;
	case 2:
		mBody.mRot.y = mSlotAngle + 0x8000;
		mBossFlags &= ~0x100;
		break;
	}
}

// 0x80119670
// Two different motions of the three in sOrangeMotionPicks at random
void TEneOrangeBoss::pickMotions(int* a, int* b)
{
	int i;

	i = (int)(3.0f * ((1.0f / 32768.0f) * (f32)rand()));
	if (i >= 3) {
		i = 2;
	}
	*a = i;
	i  = (int)(3.0f * ((1.0f / 32768.0f) * (f32)rand()));
	if (i >= 3) {
		i = 2;
	}
	*b = i;
	if (*a == *b) {
		*b = *b + 1;
		*b = *b % 3;
	}
	*a = sOrangeMotionPicks[*a];
	*b = sOrangeMotionPicks[*b];
}

// 0x8011978C
// Itself at its own mirror and a copy at each of the other seven
void TEneOrangeBoss::setupDecoys()
{
	Vec3f dir;
	int a;
	int b;
	int k = 0;
	int slot;

	for (slot = 0; slot < 8; slot++) {
		int angle = mirrorSlotDir(slot, &dir);
		if (slot != mSlot) {
			goalScalePos(&mDecoyPos[k], &dir, 250.0f);
			mDecoyAngle[k] = angle + 0x8000;
			k++;
		} else {
			mBody.mRot.y = mSlotAngle;
			goalScalePos(&mBody.mPos, &dir, 250.0f);
		}
	}
	pickMotions(&a, &b);
	motionCtrlPlayBlend(0.2f, &mAnim, (s16)a);
	motionCtrlPlayBlend(0.2f, &mAnim2, (s16)b);
	setCantAct();
}

// 0x80119870
void TEneOrangeBoss::actDecoys(int phase)
{
	switch (phase) {
	case 0:
		clearFrameKey();
		mBossFlags |= 0x4A;
		setupDecoys();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		waitMotion();
		updatePose();
		calcPoints();
		motionCtrlUpdate(&mAnim2);
		gjPushUnitMatrix();
		localMatrix();
		applyPose(mPose2, &mAnim2);
		njPopMatrixEx();
		break;
	case 2:
		mBossFlags &= ~0x4A;
		clearCantAct();
		break;
	}
}

// 0x80119934
// A jump back out of the middle to its slot
f32 TEneOrangeBoss::planReturn()
{
	Vec3f dir;
	f32 a;
	f32 b;
	f32 speed;
	f32 fade;
	MotionStep* keys;

	mBody.mRot.y = mSlotAngle + 0x8000;
	njSinCos(mSlotAngle + 0x8000, &dir.x, &dir.z);
	dir.y = 0.0f;
	goalScalePos(&mBody.mPos, &dir, 260.0f);
	mJumpFrom = mBody.mPos;
	mJumpFrom.y += 20.0f;
	ballisticRoots(-0.12f, 0.0f, -20.0f, &a, &b);
	a = a > b ? a : b;
	speed = 100.0f;
	speed /= a;
	speed *= -1.0f;
	mBody.mVel.x = dir.x * speed;
	mBody.mVel.y = 0.0f;
	mBody.mVel.z = dir.z * speed;
	fade         = (f32)floor(5.0);
	keys = mKeys;
	motionStepFromEntry(0.0f, &keys[0], 0xD, sOrangeBossMotionKeys);
	motionStepBlendOut((a - 3.3333333f) - fade, 0.2f, &keys[1], &keys[0]);
	motionStepFromEntry(a - 3.3333333f, &keys[2], 0xE, sOrangeBossMotionKeys);
	motionStepEnd(&keys[3]);
	return a;
}

// 0x80119AAC
void TEneOrangeBoss::actReturn(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0xD);
		clearFrameKey();
		mBossFlags |= 0x116;
		clearLand();
		mActStep = 0;
		mActEnd  = planReturn();
		mActTime = -1.0f;
		seStartAtFar(0x14B, &mBody.mPos);
		break;
	case 1:
		mActTime += 1.0f;
		if (motionStepsPlay(mActTime, mKeys, &mMotion)) {
			mHitFlags |= 0x20;
		}
		updatePose();
		switch (mActStep) {
		case 0:
			if (mActTime < mActEnd) {
				jumpAt(mActTime);
			} else {
				mActStep     = 1;
				mBody.mPos.y = 50.0f;
				mBody.mVel.y = 0.0f;
				mJumpFrom    = mBody.mPos;
				setLand();
			}
			break;
		case 1:
		case 2:
			slideTo(mActTime - mActEnd, -0.2f);
			if (mActStep == 1 && motionCtrlFrame(&mAnim) >= 10.0f) {
				mActStep = 2;
				setLand();
			}
			break;
		}
		calcPoints();
		if (takeLand()) {
			landSplash();
		}
		break;
	case 2:
		mBossFlags &= ~0x116;
		mBody.mVel.x = 0.0f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z = 0.0f;
		break;
	}
}

// 0x80119C90
// Knocked down
void TEneOrangeBoss::actHurt(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x10);
		mEnemyFlags |= 8;
		stageBeaten();
		mFrameKey = 3;
		if (mGlow[0] != 0) {
			mGlow[0]->mFlags |= 1;
			mGlow[0] = 0;
		}
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		waitMotion();
		updatePose();
		slideMove(0.8f);
		calcPoints();
		break;
	case 2:
		mEnemyFlags &= ~8;
		clearFrameKey();
		break;
	}
}

// 0x80119D68
void TEneOrangeBoss::actDie(int phase)
{
	switch (phase) {
	case 0:
		mActStep  = 0;
		mActTimer = 0x12;
		mTint.start();
		mEnemyFlags |= 0x10;
		colliDisable(mCollider);
		break;
	case 1:
		if (mActStep == 0) {
			mActTimer--;
			if (mActTimer >= 0) {
				return;
			}
			mBody.mPos.x = 0.0f;
			mBody.mPos.y = 0.0f;
			mBody.mPos.z = 0.0f;
			mBody.mRot.x = 0;
			mBody.mRot.y = 0;
			mBody.mRot.z = 0;
			motionCtrlPlay(&mAnim, 0x1F);
			mEnemyFlags |= 0x14;
			colliDisable(mCollider);
			mFrameKey = 3;
			shadowDisable(&mShadow);
			mActStep = 1;
			mActTime = 0.0f;
			bossSuitSetFade(140.0f, 260.0f, &mSuit);
			bossSuitSetSink(200.0f, 320.0f, 60.0f, -15.0f, 5.0f, &mSuit);
			bossSuitSetFx(&mSuit, 1, 0x50, 0x4F);
			spawnFx(0, 0);
			seStartArg(0x2AB, 0x1B8);
			seCueSet(&mSe, 0x69, 0xA5);
		}
		motionCtrlUpdate(&mAnim);
		updatePoseRooted();
		if (lockOn()) {
			mirrorsHide();
		}
		calcPoints();
		mActTime += 1.0f;
		if (bossSuitUpdate(mActTime, &mSuit, &mBody.mPos, sOrangeBossModels[2])) {
			mBossFlags |= 1;
		} else {
			mBossFlags &= ~1;
		}
		seCueUpdate(&mSe, (int)mActTime);
		if (mActTime >= 420.0f) {
			slain();
			sendBossDefeated(this);
			mFlags |= 1;
		}
		break;
	case 2:
		mEnemyFlags &= ~0x14;
		clearFrameKey();
		unlock();
		break;
	}
}

// 0x80119FB4
void TEneOrangeBoss::vfn_3C()
{
	njPrint((kEnemyInfoX << 16) | kEnemyInfoLine, ":HP %4d", mHealth);
}

// 0x80119FF0
void TEneOrangeBoss::vfn_10()
{
	if (isAppearing()) {
		mFlags |= 0x10;
	} else {
		if (mBossFlags & 0x40) {
			mLight.place(0, mDrawTask, &mBody.mPos, 0);
		} else {
			if (!mLight.isVisible(mDrawTask, 3000.0f, &mBody.mPos, 22.0f, 0)) {
				mFlags |= 0x10;
				return;
			}
		}
		mFlags &= ~0x10;
		gjPushUnitMatrix();
		bodyMatrix();
		njGetMatrix(_1A4);
		njPopMatrixEx();
		if (mBossFlags & 1) {
			moveDrawTask(mDrawTask, 0x22);
		}
	}
}

// 0x8011A0D8
// Its reflection in its mirror and the copies when they are on the near side
void TEneOrangeBoss::drawReflections()
{
	Vec3f top;
	Vec3f at;
	Vec3f* pos;
	int angle;
	int i;

	top.x = 0.0f;
	top.y = 50.0f;
	top.z = 0.0f;
	njCalcPointEx(&top, &top);
	if (mBossFlags & 2) {
		njCalcPointEx(&mBody.mPos, &at);
		if (top.z > at.z) {
			angle  = mSlotAngle;
			mSlotAngle = angle + 0x8000;
			drawBody(mBossFlags & 4, _1A4, mPose);
			mSlotAngle = angle;
		}
	}
	if (mBossFlags & 0x40) {
		for (i = 0; i < 7; i++) {
			pos = &mDecoyPos[i];
			if (isInView(3000.0f, 22.0f, pos, &at) && top.z > at.z) {
				f32 mtx[12];
				gjPushUnitMatrix();
				njTranslateEx(pos);
				gjRotateY(mDecoyAngle[i]);
				njGetMatrix(mtx);
				njPopMatrixEx();
				drawBody(0, mtx, mPose2);
			}
		}
	}
}

// 0x8011A210
void TEneOrangeBoss::render()
{
	if (!(mBossFlags & BOSS_HURT)) {
		drawBody(mBossFlags & BOSS_DOWN, _1A4, mPose);
	}
}

// 0x8011A248
// mirrored draws it cut by the plane of its mirror
void TEneOrangeBoss::drawBody(int mirrored, f32* mtx, void* pose)
{
	u32 phase[2];

	if (mBossFlags & 1) {
		texFxSuitLoadTex(2);
	} else if (mirrored) {
		texFxTintLoadTex(1);
	}
	njSetTexture(*(void**)sOrangeBossTextures);
	enemyEyeSet((NJS_TEXLIST*)(*(void**)sOrangeBossTextures), 2, mFrameKey);
	if (mBossFlags & 1) {
		gjSetBlendOverride(1, 4, 5, 3);
		_nj_control_3d_flag_ |= 0x8000;
		texFxSuitBegin(0, *(u32*)&mSuit);
	} else if (mirrored) {
		njPushMatrix(mtx);
		texFxTintSetMtx(1, &mSlotPos, mSlotAngle);
		njPopMatrixEx();
		mTint.getPhase(phase);
		texFxTintBegin(0, phase);
	}
	njPushMatrixEx();
	njMultiMatrix(0, mtx);
	if (mirrored) {
		shadowAlphaBegin();
		gjDrawActor((GjActor*)pose);
		shadowAlphaEnd();
	} else if (mBossFlags & 1) {
		texFxSuitScroll(5.0f + mSuit.mHeight, mSuit.mHeight, 1);
		shadowAlphaBegin();
		gjDrawActor((GjActor*)pose);
		shadowAlphaEnd();
	} else {
		drawLit(&mLight, &mTint, pose);
	}
	njPopMatrixEx();
	if (mBossFlags & 1) {
		texFxSuitEnd();
	} else if (mirrored) {
		texFxTintEnd();
	}
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}
