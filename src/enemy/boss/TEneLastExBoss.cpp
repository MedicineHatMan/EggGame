#include "io/ResBank.h"
struct DrawTask;
#include "enemy/ObjEnemy.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyAppear.h"
#include "enemy/EnemySight.h"
#include "engine/MotionPose.h"
#include "enemy/boss/BossParts.h"
#include "object/ObjCommon.h"
#include "enemy/EnemyLight.h"
#include "enemy/TEneAm01Parts.h"
#include "engine/FxHolder.h"
#include "egg/ObjEgg.h"
#include "sound/SeHolder.h"
#include "enemy/boss/TEneLastExBossCore.h"
#include "object/ObjAction.h"
#include "enemy/EnemyHit.h"
#include "object/TObjHelios.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "core/MathUtil.h"
#include "camera/TCamPath.h"
#include "object/ObjShadow.h"
#include "enemy/EnemyFx.h"
#include "engine/TFogCtrl.h"
#include "enemy/EnemyTexFx.h"
#include "object/last/TObjLastExBoss_BGCtrl.h"
#include "enemy/boss/TEneLastExBoss.h"
#include "enemy/boss/TEneLastExBossFeather.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "engine/DrawState.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "ui/AeArc.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "engine/Colli.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "engine/TParticle.h"
#include "camera/TCameraCtrl.h"
#include "animal/RacerMover.h"
#include "enemy/TEneAm01.h"
struct GjActor;
struct Vec3f;
struct DrawState;
struct DrawStateSlot;
struct ColliShape;
struct CharaColli;
struct CamView;

class TEneLastExBoss;

extern "C" {
MotionEntry sLastExBossMotions[] = {
	{ MOTION_LOOP, 11, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },
	{ MOTION_LOOP, 12, -1, 0, 0.0f, 20.0f, 0.7f, 0.0f, 0 },
	{ MOTION_LOOP, 13, -1, 0, 0.0f, 20.0f, 0.9f, 0.0f, 0 },
	{ MOTION_LOOP, 18, -1, 0, 0.0f, 30.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 39.0f, 0.6f, 0.0f, 0 },
	{ MOTION_LOOP, 16, -1, 0, 0.0f, 55.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 19.0f, 0.35f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 10.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 49.0f, 0.7f, 0.0f, 0 },
	{ MOTION_ONCE, 15, -1, 0, 0.0f, 59.0f, 0.75f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 24.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 15.0f, 0.9f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 34.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 6, -1, 0, 0.0f, 30.0f, 0.8f, 0.0f, 0 },
	{ MOTION_ONCE, 17, -1, 0, 0.0f, 59.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 49.0f, 0.7f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 0.0f, 14.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 14, -1, 0, 0.0f, 29.0f, 0.25f, 0.0f, 0 },
	{ MOTION_ONCE, 10, -1, 0, 0.0f, 299.0f, 1.0f, 0.0f, 0 },
};
extern const u32 sLastExFeatherIds0[] = { 0x0000006E, 0x0000006F, 0x00000070, 0x00000071 };
extern const u32 sLastExFeatherIds1[] = { 0x00000072, 0x00000073, 0x00000074, 0x00000075 };
extern const u32 sLastExFeatherIds2[] = { 0x00000076, 0x00000077, 0x00000078, 0x00000079 };
extern const u32 sLastExFeatherIds3[] = { 0x0000007A, 0x0000007A, 0x0000007A, 0x0000007A };
extern const int sLastExBossNodes[] = { 13, 18, 23, 28, 38, 43, 48, 53, 55, 56, 58, 59, 60, 64, 73, 79, 91, 97, -1 };
const char* gTEneLastExBossName = "TEneLastExBoss";
extern const EnemyFrameKey sLastExBossFrameKey0[] = { { 18.0f, 0 }, { 28.0f, 5 }, { 41.0f, 0 }, { 47.0f, 3 }, { -1.0f, 0 } };
}

typedef char LastExBossAi_size_check[(sizeof(LastExBossAi) == 0x5C) ? 1 : -1];

typedef char TEneLastExBoss_size_check[(sizeof(TEneLastExBoss) == 0x41C) ? 1 : -1];

// The arena is a disc about the origin with its floor at 20

// 0x80165968
// pos put on the floor
extern "C" void lastExOnFloor(const Vec3f* pos, Vec3f* out)
{
	out->x = pos->x;
	out->y = 20.0f;
	out->z = pos->z;
}

// 0x80165984
// How far pos is from the middle across the floor
extern "C" f32 lastExDistFromCenter(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 z = pos->z;
	return __fres((f32)__frsqrte(x * x + z * z));
}

// 0x801659A4
// The angle of pos about the middle
extern "C" int lastExAngleFromCenter(const Vec3f* pos)
{
	return (int)(10430.38043493439 * (f32)atan2(pos->x, pos->z));
}

// 0x801659E4
// The way out from the middle to pos flat
extern "C" void lastExOutward(const Vec3f* pos, Vec3f* out)
{
	out->x = pos->x;
	out->y = 0.0f;
	out->z = pos->z;
	if (njUnitVector(out) < gUnitVectorMin) {
		*out = gVecZero;
	}
}

// 0x80165A54
// A random point on the floor within radius of the middle
extern "C" void lastExRandomPoint(f32 radius, Vec3f* out)
{
	f32 dist = radius * (0.000030517578f * (f32)rand());

	vecFromAngleXZ(dist, out, (int)(65536.0f * (0.000030517578f * (f32)rand())));
	out->y = 20.0f;
}

// 0x80165B0C
extern "C" void initTEneLastExBoss()
{
	resBankLoad(&sLastExBossBank, "ar_ene_last_ex_boss.arc");
	resBankAddTextures(&sLastExBossBank, 0, "ene_last_ex_boss.gvm");
	sLastExBossModels = resBankGetModels(&sLastExBossBank);
	sLastExBossTextures = resBankGetTextures(&sLastExBossBank);
	sLastExBossMotion = resBankGetMotion(&sLastExBossBank, 0);
	sLastExParams = (int*)enemyParamGet(8);
	fxBankLoadDefault(&sLastExBossFx, "pa_lastboss2.pad");
	gLastExBoss = 0;
	lastExBossFeatherInit();
}

// 0x80165BB4
extern "C" void shutdownTEneLastExBoss()
{
	lastExBossFeatherExit();
	gLastExBoss = 0;
	sLastExParams = 0;
	fxBankRelease(&sLastExBossFx);
	resBankRemoveTextures(&sLastExBossBank, 0);
	resBankRelease(&sLastExBossBank);
	sLastExBossModels = 0;
	sLastExBossTextures = 0;
	sLastExBossMotion = 0;
}

// 0x80165C14
extern "C" TObject* createTEneLastExBoss(TObject* parent, SetDesc* desc)
{
	return new TEneLastExBoss(parent, desc);
}

// 0x80165C68
// Model-table word for an index below 8
extern "C" NJS_OBJECT* modelTEneLastExBoss(int index)
{
	if (index < 0 || index >= 8) {
		return 0;
	}
	return sLastExBossModels[index];
}

// 0x80165C90
extern "C" NJS_TEXLIST* textureTEneLastExBoss()
{
	return *sLastExBossTextures;
}

// 0x80165C9C
extern "C" NJS_MOTION** motionTEneLastExBoss(int index)
{
	return resBankGetMotion(&sLastExBossBank, index);
}

// 0x80165CC8
extern "C" void showLastExBoss()
{
	if (gLastExBoss != 0) {
		gLastExBoss->setShown(1);
	}
}

// 0x80165CF8
extern "C" void hideLastExBoss()
{
	if (gLastExBoss != 0) {
		gLastExBoss->setShown(0);
	}
}

// 0x80165D28
void TEneLastExBoss::setShown(int shown)
{
	if (shown) {
		gLastExBoss->mBossFlags &= ~EX_HIDDEN;
		shadowEnable(&mShadow);
	} else {
		gLastExBoss->mBossFlags |= EX_HIDDEN;
		shadowDisable(&mShadow);
	}
}

// 0x80165D80
extern "C" int lastExBossIsIdle()
{
	if (gLastExBoss != 0) {
		return gLastExBoss->isIdle();
	}
	return 1;
}

// 0x80165DB4
int TEneLastExBoss::isIdle()
{
	return (u8)(mAction == 1);
}

// 0x80165DC8
extern "C" void resetLastExBoss()
{
	if (gLastExBoss != 0) {
		gLastExBoss->requestReset();
	}
}

// 0x80165DF4
void TEneLastExBoss::requestReset()
{
	if (mAi != 0) {
		mAi->requestReset();
	}
}

// 0x80165E20
TEneLastExBoss::TEneLastExBoss(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	readEnemyDesc(desc);
	setupParts();
	initBossBar();
	makeAi();
	camSetChaseBase(0, 33.0f, 150.0f);
}

// 0x80165EE8
void TEneLastExBoss::setupParts()
{
	int last;
	int id;
	ColliShape* shapes;

	_00   = (u32)gTEneLastExBossName;
	mSize = 0x41C;
	motionCtrlSetPlayer(&mAnim, &mMotionData);
	motionCtrlInit(&mAnim, sLastExBossMotions, sLastExBossMotion);
	motionCtrlPlay(&mAnim, 3);
	id           = 0;
	mBody.mRot.x = 0;
	mCollider    = colliCreate(2, 1);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0xFF, 0, 0x10);
	shapes = colliGetShapes(mCollider);
	colliShapeMakeSphere(shapes, &mBody.mPos, 0, 8.0f);
	colliShapeMakeSphere(&shapes[1], &mBody.mPos, 0, 8.0f);
	for (int i = 0; i < 2; i++, id++) {
		colliShapeSetTag(mCollider, i, id);
		colliShapeSetBit8(mCollider, i);
	}
	mHurt = colliCreate(4, 1);
	colliSetOwner(mHurt, (u16)mObjId);
	colliSetMasks(mHurt, 2, 0xFF, 0xFF, 0x10);
	shapes = colliGetShapes(mHurt);
	colliShapeMakeSphere(&shapes[2], &mBody.mPos, 0, 18.0f);
	colliShapeMakeSphere(&shapes[3], &mBody.mPos, 0, 10.0f);
	for (int i = 0; i < 4; i++, id++) {
		colliShapeSetTag(mHurt, i, id);
		colliShapeSetBit8(mHurt, i);
		colliShapeSetBit10(mHurt, i);
	}
	mAi        = 0;
	mPose      = (GjActor*)allocPose(sLastExBossModels[6]);
	mPose2     = allocPose(sLastExBossModels[5]);
	mVertCount = *(u32*)((u8*)mPose2 + 8) / 12;
	mVertOrder = new s16[mVertCount];
	mVertPos   = new HitVec[mVertCount];
	mVertVel   = new HitVec[mVertCount];
	mVertAge   = new u8[mVertCount];
	for (int i = 0; i < mVertCount; i++) {
		mVertPos[i] = mBody.mPos;
	}
	for (int i = 0; i < mVertCount; i++) {
		mVertVel[i].set(0.0f, 0.0f, 0.0f);
	}
	for (int i = 0; i < mVertCount; i++) {
		mVertAge[i] = 0;
	}
	for (int i = 0; i < mVertCount; i++) {
		mVertOrder[i] = i;
	}
	for (id = 0; id < mVertCount; id++) {
		last = mVertCount - 1;
		int pick = (int)((f32)last * (3.0517578E-5f * (f32)rand()));
		s16 temp;

		if (pick >= last) {
			pick = last - 1;
		}
		temp             = mVertOrder[id];
		mVertOrder[id]   = mVertOrder[pick];
		mVertOrder[pick] = temp;
	}
	mFxCount  = mVertCount / 10;
	mFxCursor = 0;
	mFx       = new TParticleGenerator*[mFxCount];
	mFxSet    = 0;
	startFx(mFxSet);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mBossFlags = 0;
	mColliMode = 0;
	mHitDir.x  = 0.0f;
	mHitDir.y  = 0.0f;
	mHitDir.z  = 0.0f;
	mFrameKey  = 0;
	for (int i = 0; i < 18; i++) {
		mPoint[i].x = 0.0f;
		mPoint[i].y = 0.0f;
		mPoint[i].z = 0.0f;
	}
	mMoveTo.x  = 0.0f;
	mMoveTo.y  = 0.0f;
	mMoveTo.z  = 0.0f;
	mAim.x     = 0.0f;
	mAim.y     = 0.0f;
	mAim.z     = 0.0f;
	mAimRot    = 0;
	mAnchor    = mBody.mPos;
	mMaxHealth = 500;
	mHealth    = 500;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	shadowEnable(&mShadow);
	bigShadow();
	mLight.reset();
	mTint.reset();
	mFeatherCount = 0;
	mBlastAt.x    = 0.0f;
	mBlastAt.y    = 0.0f;
	mBlastAt.z    = 0.0f;
	mAlpha        = 0xFF;
	mExtra        = 0;
	fxHolderStop(&mGlow);
	mCore = new TEneLastExBossCore(this);
	mCorePos.x = 0.0f;
	mCorePos.y = 20.0f;
	mCorePos.z = 0.0f;
	mDemo      = 0;
	mFogState  = 0;
	mFogTimer  = 0;
	mStep      = 0;
	mCount     = 0;
	mTurnTo    = 0;
	_414       = 0;
	mTime      = 0.0f;
	if (gLastExBoss == 0) {
		gLastExBoss = this;
	}
}

// 0x8016646C
TEneLastExBoss::~TEneLastExBoss()
{
	if (gLastExBoss == this) {
		gLastExBoss = 0;
	}
	fxHolderStop(&mGlow);
	for (int i = 0; i < 1; i++) {
		if ((&mExtra)[i] != 0) {
			(&mExtra)[i]->mFlags |= 1;
			(&mExtra)[i] = 0;
		}
	}
	if (mDemo != 0) {
		mDemo->mFlags |= 1;
		mDemo = 0;
	}
	unlock();
	if (mAi != 0) {
		delete mAi;
		mAi = 0;
	}
	removeDrawTask(mDrawTask);
	stopFx();
	if (mFx != 0) {
		delete[] mFx;
		mFx = 0;
	}
	if (mVertAge != 0) {
		delete[] mVertAge;
		mVertAge = 0;
	}
	if (mVertVel != 0) {
		delete[] mVertVel;
		mVertVel = 0;
	}
	if (mVertPos != 0) {
		delete[] mVertPos;
		mVertPos = 0;
	}
	if (mVertOrder != 0) {
		delete[] mVertOrder;
		mVertOrder = 0;
	}
	freePose((void**)&mPose2);
	freePose((void**)&mPose);
	colliRetire(mHurt);
	mHurt = 0;
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x80166668
void TEneLastExBoss::setPos(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	mBody.mPos.x = x;
	f32 z = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x801666AC
void TEneLastExBoss::placeAt(const Vec3f* pos)
{
	Vec3f at;
	at.x = pos->x;
	at.y = pos->y;
	at.z = pos->z;
	at.y = 110.0f;
	setPos(&at);
	mBody.mRot.y = (int)(10430.38043493439 * (f32)atan2(at.x, at.z)) + 0x10000 - 0x8000;
}

// 0x80166738
void TEneLastExBoss::setHeight()
{
	mBody.mPos.y = 70.0f;
}

// 0x80166744
f32 TEneLastExBoss::vfn_40()
{
	if (mBossFlags & EX_CORE) {
		return 20.0f;
	}
	return 80.0f;
}

// 0x80166760
void TEneLastExBoss::placeShape(f32 radius, void* shape, int point)
{
	((ColliShape*)shape)->mPos = mPoint[point];
	colliShapeSetRadius((ColliShape*)shape, radius);
}

// 0x801667A4
static inline Vec3f pointSum(const Vec3f* a, const Vec3f* b)
{
	Vec3f r;
	gjAddVector(&r, a, b);
	return r;
}

void TEneLastExBoss::syncCollider()
{
	ColliShape* shapes = colliGetShapes(mCollider);
	Vec3f centre; // of points 10 to 12 for the last shape

	switch (mColliMode) {
	case COLLI_BODY:
		placeShape(8.0f, shapes, 14);
		placeShape(8.0f, &shapes[1], 16);
		colliRefreshBounds(mCollider);
		colliBoundsOn(mCollider);
		colliAddMasks(mCollider, 0xFF, 0, 0);
		colliEnable(mHurt);
		break;
	case COLLI_CORE:
		shapes[0].mPos = mCorePos;
		colliShapeSetRadius(shapes, 10.0f);
		shapes[1].mPos = mCorePos;
		colliShapeSetRadius(&shapes[1], 1.0f);
		colliRefreshBounds(mCollider);
		colliBoundsOff(mCollider);
		colliClearMasks(mCollider, 9, 0, 0);
		colliDisable(mHurt);
		break;
	case COLLI_RUSH: {
		Vec3f mid;
		Vec3f back;
		Vec3f at;

		mid = pointSum(&mPoint[15], &mPoint[14]);
		gjScaleVector(0.5f, &mid, &mid);
		back = pointSum(&mPoint[17], &mPoint[16]);
		gjScaleVector(0.5f, &back, &back);
		vecLerp(0.9f, &mid, &back, &at);
		shapes[0].mPos = at;
		colliShapeSetRadius(shapes, 16.0f);
		vecLerp(0.9f, &back, &mid, &at);
		shapes[1].mPos = at;
		colliShapeSetRadius(&shapes[1], 16.0f);
		colliRefreshBounds(mCollider);
		colliBoundsOn(mCollider);
		colliAddMasks(mCollider, 0xFF, 0, 0);
		colliEnable(mHurt);
		break;
	}
	}
	shapes = colliGetShapes(mHurt);
	placeShape(18.0f, &shapes[2], 9);
	centre.x = mPoint[10].x;
	centre.y = mPoint[10].y;
	centre.z = mPoint[10].z;
	njAddVector(&centre, &mPoint[11]);
	njAddVector(&centre, &mPoint[12]);
	gjScaleVector(0.33333334f, &centre, &centre);
	shapes[3].mPos = centre;
	colliShapeSetRadius(&shapes[3], 10.0f);
}

// 0x80166A8C
void TEneLastExBoss::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
}

// 0x80166AAC
int TEneLastExBoss::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit       = 0;

	if (!(mBossFlags & EX_NOTOUCH)) {
		if (isPlayerOrKind4(other)) {
			info->mDamage = 10;
			hit           = 1;
			info->mFlags.clear();
		} else if (isEggObj(other) && !(mBossFlags & EX_CORE)) {
			info->mDamage = 1;
			hit           = 1;
			info->mFlags.clear();
			info->mFlags.setBit(0xD);
		}
	}
	return hit;
}

// 0x80166B54
void TEneLastExBoss::setMoveTo(const Vec3f* pos)
{
	mMoveTo = *pos;
}

// 0x80166B70
void TEneLastExBoss::setMoveToHigh(const Vec3f* pos)
{
	mMoveTo   = *pos;
	mMoveTo.y = 110.0f;
}

// 0x80166B94
void TEneLastExBoss::setAim(const Vec3f* pos, int rotY)
{
	mAim    = *pos;
	mAimRot = rotY;
}

// 0x80166BB4
void TEneLastExBoss::setMaxHealth(int health)
{
	mMaxHealth = health;
	mHealth    = health;
}

// 0x80166BC0
void TEneLastExBoss::clearOpen()
{
	mBossFlags &= ~EX_OPEN;
}

// 0x80166BD0
void TEneLastExBoss::setOpen()
{
	mBossFlags |= EX_OPEN;
}

// 0x80166BE0
void TEneLastExBoss::setTouchable()
{
	int i;

	for (i = 0; i < 2; i++) {
		colliShapeSetBit8(mCollider, i);
	}
	mBossFlags &= ~EX_NOTOUCH;
}

// 0x80166C30
void TEneLastExBoss::setUntouchable()
{
	int i;

	for (i = 0; i < 2; i++) {
		colliShapeClearBit8(mCollider, i);
	}
	mBossFlags |= EX_NOTOUCH;
}

// 0x80166C80
void TEneLastExBoss::onHardHit()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
	hideBossBar();
	mCore->flash();
	seStartAtFar(0x1A1, &mCorePos);
}

// 0x80166CD8
void TEneLastExBoss::onLightHit()
{
	enemyFxSparks(0.0f, &mContact.mSurface, &mContact.mCenter);
	seStartAtFar(0x19D, &mBody.mPos);
}

// 0x80166D1C
int TEneLastExBoss::isDone0()
{
	return isActionDone(0);
}

// 0x80166D40
int TEneLastExBoss::isDone1()
{
	return isActionDone(1);
}

// 0x80166D64
int TEneLastExBoss::isDone2()
{
	return isActionDone(2);
}

// 0x80166D88
int TEneLastExBoss::isDone3()
{
	return isActionDone(3);
}

// 0x80166DAC
int TEneLastExBoss::isDone4()
{
	return isActionDone(4);
}

// 0x80166DD0
int TEneLastExBoss::isDone5()
{
	return isActionDone(5);
}

// 0x80166DF4
int TEneLastExBoss::isDone7()
{
	return isActionDone(7);
}

// 0x80166E18
int TEneLastExBoss::isDone8()
{
	return isActionDone(8);
}

// 0x80166E3C
void TEneLastExBoss::setFeatherCount(int count)
{
	mFeatherCount = count;
}

// 0x80166E44
int TEneLastExBoss::isDone9()
{
	return isActionDone(9);
}

// 0x80166E68
int TEneLastExBoss::isDone10()
{
	return isActionDone(10);
}

// 0x80166E8C
int TEneLastExBoss::isDone11()
{
	return isActionDone(11);
}

// 0x80166EB0
int TEneLastExBoss::isDone12()
{
	return isActionDone(12);
}

// 0x80166ED4
int TEneLastExBoss::isDone13()
{
	return isActionDone(13);
}

// 0x80166EF8
void TEneLastExBoss::updateBody()
{
	if (!isBlocked() && takeHits(&mHit, &mContact)) {
		enemyHitPushAway((Vec3f*)&mHit, &mBody.mPos, (const Vec3f*)&mContact, &mHitDir);
		applyDamage(&mHit);
	}
	mAi->tick(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	mTint.update();
	syncCollider();
	setBossBar(mMaxHealth, mHealth);
	if (mFogState == 0) {
		mBossFlags &= ~EX_FOG;
	} else if (mFogState == 3) {
		mBossFlags |= EX_FOG;
	} else {
		mFogTimer++;
		if (mFogTimer >= 10) {
			mFogTimer = 0;
			if (mFogState == 2) {
				mFogState = 0;
			} else {
				mFogState = 3;
			}
		}
	}
	if (mBossFlags & EX_FOG) {
		fogRequest(0, 0x12, 2, 0x64, 0.1f);
	}
}

// 0x80167068
void TEneLastExBoss::vfn_6C()
{
	if (returnZero()) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		gjPushUnitMatrix();
		beginNodes();
		poseFirstFrame(sLastExBossMotion, 0x13, mPose);
		endNodes();
		poseFirstFrame(sLastExBossMotion, 0x13, mPose2);
		njPopMatrixEx();
		calcPoints();
		syncCollider();
	}
}

// 0x801670FC
void TEneLastExBoss::makeAi()
{
	mAi = createLastExBossAi(this);
}

// 0x8016712C
void TEneLastExBoss::beginNodes()
{
	nodeTrackBeginOrigins(sLastExBossNodes, mPoint);
}

// 0x8016715C
void TEneLastExBoss::endNodes()
{
	nodeTrackEnd();
}

// 0x8016717C
void TEneLastExBoss::updatePose()
{
	beginNodes();
	gjPushUnitMatrix();
	applyPose(mPose, &mAnim);
	applyPose(mPose2, &mAnim);
	njPopMatrixEx();
	if (hatchActive(&mAppearTimer)) {
		hatchPose(&mAppearTimer, (GjActor*)mPose, 60.0f);
	}
	endNodes();
}

// 0x801671F8
void TEneLastExBoss::updatePoseRooted()
{
	MotionNodePose root;
	NJS_MOTION* node = motionCtrlGetMotion(&mAnim);

	motionGetNodePose(&root, *(NJS_OBJECT**)mPose, node, motionCtrlFrame(&mAnim));
	motionPoseGetPos(&root, &mBody.mPos);
	mBody.mRot.y = root.mAng[1];
	beginNodes();
	gjPushUnitMatrix();
	gjRotateY(-mBody.mRot.y);
	gjTranslate(-mBody.mPos.x, -mBody.mPos.y, -mBody.mPos.z);
	applyPose(mPose, &mAnim);
	applyPose(mPose2, &mAnim);
	njPopMatrixEx();
	endNodes();
}

// 0x801672C0
// Moves the attached vertices to the body and hangs the particles on some of them
// The file is built without lifetime splitting configure.py so set the first loop's counter reused for the particle set
// stays in r27
void TEneLastExBoss::updateVerts()
{
	const s16* mesh;
	Vec3f* pos;
	u8* age;
	int set;
	int cursor;
	int i;

	asm {
		li r3, 0x0807
		oris r3, r3, 0x0807
		mtspr 918, r3
	}
	pos  = mVertPos;
	set  = 0;
	mesh = *(const s16**)((u8*)mPose2 + 4);
	age  = mVertAge;
	for (; set < mVertCount; set++) {
		if (*age == 0) {
			register const s16* from = mesh;
			register f32 x;
			register f32 y;
			register f32 z;
			Vec3f at;

			asm {
				psq_l x, 0(from), 1, 6
				psq_l y, 2(from), 1, 6
				psq_l z, 4(from), 1, 6
			}
			at.x = x;
			at.y = y;
			at.z = z;
			njCalcPointEx(&at, pos);
		}
		mesh += 6;
		pos++;
		age++;
	}
	set = 0;
	if (mBossFlags & EX_FOG) {
		set = 3;
	} else if (mBossFlags & EX_ATTACK) {
		set = 1;
	} else if (mHealth < sLastExParams[2]) {
		set = 2;
	}
	if (set != mFxSet) {
		mFxSet = set;
		stopFx();
		startFx(set);
	}
	cursor = mFxCursor;
	for (i = 0; i < mFxCount; i++) {
		mFx[i]->mPos = mVertPos[mVertOrder[cursor]];
		cursor++;
		if (cursor >= mVertCount) {
			cursor = 0;
		}
	}
	mFxCursor = cursor;
}

// 0x80167430
void TEneLastExBoss::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 18; i++) {
		njCalcPointEx(&mPoint[i], &mPoint[i]);
	}
	updateVerts();
	njPopMatrixEx();
}

// 0x80167498
int TEneLastExBoss::getHitKind(const void* touch, HitInfo* info)
{
	if (mBossFlags & EX_OPEN) {
		return 3;
	}
	return info->mFlags.testBit(0x10) ? 3 : 1;
}

// 0x801674E4
int TEneLastExBoss::getDamage(EnemyHit* hit)
{
	int kind   = hit->mKind;
	int damage = hit->mInfo.mDamage;

	if (kind != 3) {
		damage = 0;
	} else if (!(mBossFlags & EX_OPEN)) {
		damage = 0;
	}
	return damage;
}

// 0x80167514
void TEneLastExBoss::bodyMatrix()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
	gjRotateX(mBody.mRot.x);
	gjRotateZ(mBody.mRot.z);
}

// 0x8016755C
int TEneLastExBoss::angleToMoveTo()
{
	Vec3f to;
	to.x = mMoveTo.x;
	to.y = mMoveTo.y;
	to.z = mMoveTo.z;
	njSubVector(&to, &mBody.mPos);
	return (int)(10430.38043493439 * (f32)atan2(to.x, to.z));
}

// 0x801675C4
void TEneLastExBoss::updateShadow()
{
	Vec3f at;
	at.x = mBody.mPos.x;
	at.y = mBody.mPos.y;
	at.z = mBody.mPos.z;

	at.y = 20.0f;
	shadowSetGround(&mShadow, &at, 0);
}

// 0x80167614
void TEneLastExBoss::damp(f32 rate)
{
	mBody.mVel.x *= rate;
	mBody.mVel.z *= rate;
}

// 0x80167630
void TEneLastExBoss::dampAndMove(f32 rate)
{
	damp(rate);
	njAddVector(&mBody.mPos, &mBody.mVel);
}

// 0x80167668
// Moves the core and bounces it off what it runs into
void TEneLastExBoss::moveCore()
{
	Vec3f move;
	Vec3f to;
	Vec3f hit;
	Vec3f normal;

	getFlatPush(&move);
	njAddVector(&move, &mBody.mVel);
	to = mCorePos;
	njAddVector(&to, &move);
	if (probeGroundLine(&mCorePos, &to, &hit, &normal)) {
		f32 speed = njUnitVector(&move);

		if (speed < 1e-06f) {
			move = gVecZero;
		}
		gjMulAddVector(-10.0f, &mCorePos, &hit, &move);
		vecReflect(&move, &normal, &mBody.mVel);
		gjScaleVector(0.8f * speed, &mBody.mVel, &mBody.mVel);
	} else {
		njAddVector(&mCorePos, &move);
	}
	mBody.mPos = mCorePos;
	mPoint[8]  = mCorePos;
}

// 0x801677A0
void TEneLastExBoss::aimVel()
{
	njSinCos(mBody.mRot.y, &mBody.mForce.x, &mBody.mForce.z);
	mBody.mForce.y = 0.0f;
}

// 0x801677E0
void TEneLastExBoss::accel()
{
	Vec3f flat;
	f32 speed;
	f32 sin;
	f32 cos;

	turnTo(mAimRot, 0x80);
	flat   = mBody.mVel;
	flat.y = 0.0f;
	speed  = vecLenXZ(&flat);
	speed += 0.2f;
	if (speed > 7.0f) {
		speed = 7.0f;
	}
	njSinCos(mBody.mRot.y, &sin, &cos);
	mBody.mVel.x = sin * speed;
	mBody.mVel.z = cos * speed;
}

// 0x8016788C
// Runs along mAccel and slides sideways toward mAim with a roll
void TEneLastExBoss::steer()
{
	Vec3f dir;
	dir.x = mBody.mForce.x;
	dir.y = mBody.mForce.y;
	dir.z = mBody.mForce.z;
	f32 along = vecDotXZ(&mBody.mVel, &dir);
	Vec3f side;
	side.x = mBody.mVel.x;
	side.y = mBody.mVel.y;
	side.z = mBody.mVel.z;
	f32 speed;
	f32 slide;
	Vec3f toAim;
	Vec3f heading;

	side.y = 0.0f;
	gjMulAddVector(-along, &side, &side, &dir);
	speed = along;
	speed += 0.2f;
	if (speed > 7.0f) {
		speed = 7.0f;
	}
	toAim = mAim;
	njSubVector(&toAim, &mBody.mPos);
	toAim.y = 0.0f;
	if (njUnitVector(&toAim) < 1e-06f) {
		toAim = gVecZero;
	}
	heading   = mBody.mVel;
	heading.y = 0.0f;
	if (njUnitVector(&heading) < 1e-06f) {
		heading = gVecZero;
	}
	slide = njCos(0x222);
	if (vecDotXZ(&toAim, &heading) > slide) {
		slide = vecLenXZ(&side);
	} else {
		Vec3f normal;

		normal.x = dir.z;
		normal.y = 0.0f;
		normal.z = -dir.x;
		if (!(vecCrossXZ(&heading, &toAim) >= 0.0f)) {
			gjScaleVector(-1.0f, &normal, &normal);
		}
		slide = vecDotXZ(&normal, &side);
		if (slide < 0.0f) {
			slide += 0.12f;
		} else {
			slide += 0.18f;
		}
		if (slide > 5.0f) {
			slide = 5.0f;
		}
		side = normal;
		gjScaleVector(slide, &side, &side);
	}
	if (slide > 0.0f) {
		if (vecCrossXZ(&dir, &side) >= 0.0f) {
			slide *= -1.0f;
		}
		mBody.mRot.z = (int)(1250.0f * slide);
	}
	gjMulAddVector(speed, &side, &side, &dir);
	mBody.mVel.x = side.x;
	mBody.mVel.z = side.z;
}

// 0x80167B3C
// Returns what is left to go
f32 TEneLastExBoss::moveToward()
{
	Vec3f to;
	to.x = mMoveTo.x;
	to.y = mMoveTo.y;
	to.z = mMoveTo.z;
	f32 y;
	f32 left;

	njSubVector(&to, &mBody.mPos);
	to.y = 0.0f;
	y    = mBody.mVel.y;
	left = springVelocity(0.05f, 0.95f, &mBody.mVel, 0.1f, &to, 0.4f, 5.0f, 0.1f);
	mBody.mVel.y = y;
	return left;
}

// 0x80167BCC
void TEneLastExBoss::riseTo(f32 height)
{
	mBody.mVel.y = approachSpeed(mBody.mVel.y, height, 0.7f, 0.1f, 0.2f, 4.0f, 0.01f);
}

// 0x80167C18
void TEneLastExBoss::riseToward(f32 height)
{
	riseTo(height - mBody.mPos.y);
}

// 0x80167C40
f32 TEneLastExBoss::coreToward(f32 height)
{
	Vec3f to;
	to.x = mMoveTo.x;
	to.y = mMoveTo.y;
	to.z = mMoveTo.z;

	to.y += 10.0f + height;
	njSubVector(&to, &mCorePos);
	return springVelocity(0.05f, 0.95f, &mBody.mVel, 0.1f, &to, 0.4f, 5.0f, 0.1f);
}

// 0x80167CC0
void TEneLastExBoss::unroll()
{
	int roll = mBody.mRot.z;

	if (roll > 0) {
		roll -= 0x200;
		if (roll < 0) {
			roll = 0;
		}
	} else if (roll < 0) {
		roll += 0x200;
		if (roll > 0) {
			roll = 0;
		}
	}
	mBody.mRot.z = roll;
}

// 0x80167CF4
// Sets loose the vertices within radius of center
void TEneLastExBoss::blowVerts(f32 radius, f32 speed, const Vec3f* center)
{
	f32 x;
	f32 y;
	f32 z;
	f32 cx = center->x;
	f32 cy = center->y;
	f32 cz = center->z;
	u8* age    = mVertAge;
	Vec3f* pos = mVertPos;
	Vec3f* vel = mVertVel;
	int i;

	i = 0;
	while (i < mVertCount) {
		if (*age == 0) {
			x = pos->x;
			y = pos->y;
			z = pos->z;
			x -= cx;
			y -= cy;
			z -= cz;
			f32 dist = __fres((f32)__frsqrte(z * z + (y * y + x * x)));

			if (dist <= radius) {
				f32 scale = speed * __fres(dist);

				*age   = 1;
				x *= scale;
				y *= scale;
				z *= scale;
				vel->x = x;
				vel->y = y;
				vel->z = z;
			}
		}
		age++;
		pos++;
		vel++;
		i++;
	}
}

// 0x80167DA4
void TEneLastExBoss::moveVerts()
{
	u8* age    = mVertAge;
	Vec3f* pos = mVertPos;
	Vec3f* vel = mVertVel;
	int i;

	for (i = 0; i < mVertCount; i++) {
		if (*age >= 20) {
			pos->x = 0.0f;
			pos->y = -180.0f;
			pos->z = 0.0f;
		} else if (*age >= 1) {
			(*age)++;
			njAddVector(pos, vel);
			gjScaleVector(0.8f, vel, vel);
		}
		age++;
		pos++;
		vel++;
	}
}

// 0x80167E4C
void TEneLastExBoss::dropVerts()
{
	Vec3f* pos = mVertPos;
	int i;

	for (i = 0; i < mVertCount; i++) {
		pos->x = 0.0f;
		pos->y = -180.0f;
		pos->z = 0.0f;
		pos++;
	}
	for (int j = 0; j < mVertCount; j++) {
		mVertAge[j] = 20;
	}
}

// 0x80167EA8
void TEneLastExBoss::gatherVerts()
{
	int i;

	for (i = 0; i < mVertCount; i++) {
		mVertAge[i] = 0;
	}
}

// 0x80167ED0
void TEneLastExBoss::stopFx()
{
	if (mFx != 0) {
		int i;

		for (i = mFxCount - 1; i >= 0; i--) {
			if (mFx[i] != 0) {
				mFx[i]->mFlags |= 1;
				mFx[i] = 0;
			}
		}
	}
}

// 0x80167F28
void TEneLastExBoss::startFx(int set)
{
	if (mFx != 0) {
		const int* ids = sLastExFeatherSets[set];
		int i;

		for (i = 0; i < mFxCount; i++) {
			mFx[i] = particleGenById(this, &mBody.mPos, 1, ids[i % 4]);
		}
	}
}

// 0x80167FC0
void TEneLastExBoss::bigShadow()
{
	mShadow._30 = 36.0f;
	mShadow._34 = 36.0f;
}

// 0x80167FD0
void TEneLastExBoss::smallShadow()
{
	mShadow._30 = 18.0f;
	mShadow._34 = 18.0f;
}

// 0x80167FE0
void TEneLastExBoss::playDemo(int model, int motion)
{
	void* actor  = resBankGetObjects(&sLastExBossBank)[model];
	NJS_MOTION** table = resBankGetMotion(&sLastExBossBank, 1);

	mDemo = createCamPath(0, 3, actor, table[motion], 0, 1.0f, 1.0f);
	if (mDemo != 0) {
		((TCamera*)mDemo)->setCut();
	}
}

// 0x80168070
void TEneLastExBoss::startFog()
{
	if (mFogState == 0 || mFogState == 2) {
		mFogState = 1;
		mFogTimer = 0;
	}
}

// 0x80168098
void TEneLastExBoss::stopFog()
{
	if (mFogState == 3 || mFogState == 1) {
		mFogState = 2;
		mFogTimer = 0;
	}
}

// 0x801680C0
void TEneLastExBoss::stepSound(f32 frame)
{
	if (motionCtrlPassed(frame, &mAnim)) {
		seStartAtRange(0x1A8, &mBody.mPos, 0x3C, 700.0f);
	}
}

// 0x8016810C
void TEneLastExBoss::onActionPhase(int action, int phase)
{
	switch (action) {
	case 0: actAppear(phase); break;
	case 1: actTurn(phase); break;
	case 2: actRush(phase); break;
	case 3: actApproach(phase); break;
	case 4: actBurstUp(phase); break;
	case 5: actBurstMid(phase); break;
	case 6: actBurstEnd(phase); break;
	case 7: actRise(phase); break;
	case 8: actCross(phase); break;
	case 9: actFeathers(phase); break;
	case 10: actBlowUp(phase); break;
	case 11: actCoreRun(phase); break;
	case 12: actCoreHit(phase); break;
	case 13: actRebuild(phase); break;
	case 14: actDie(phase); break;
	}
}

// 0x801681F8
void TEneLastExBoss::actAppear(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x12);
		mBody.mPos.x = 0.0f;
		mBody.mPos.y = 20.0f;
		mBody.mPos.z = 0.0f;
		mBody.mRot.y = 0;
		colliDisable(mCollider);
		setUntouchable();
		mStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePoseRooted();
		calcPoints();
		waitMotion();
		lockOn();
		if (mStep == 0) {
			mStep = 1;
			playDemo(1, 1);
			seStartArg(0x2B6, 0x186);
		}
		break;
	case 2:
		mBody.mRot.x = 0;
		colliEnable(mCollider);
		setTouchable();
		motionCtrlPlay(&mAnim, 3);
		motionCtrlUpdate(&mAnim);
		if (mDemo != 0) {
			mDemo->mFlags |= 1;
			mDemo = 0;
		}
		unlock();
		break;
	}
}

// 0x80168338
void TEneLastExBoss::actTurn(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 3);
		clearFrameKey();
		break;
	case 1: {
		int left;
		int rot;

		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		rot = angleApproachDiff(mBody.mRot.y, mAimRot, 0x200, &left);
		if (left < 0x400) {
			setDone();
		}
		mBody.mRot.y = rot;
		stepSound(11.0f);
		riseToward(110.0f);
		dampAndMove(0.9f);
		updateShadow();
		calcPoints();
		break;
	}
	case 2:
		break;
	}
}

// 0x80168414
// Drops low and runs across the arena
void TEneLastExBoss::actRush(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 6);
		clearFrameKey();
		mBossFlags |= EX_ATTACK;
		mBossFlags &= ~EX_STOMPED;
		mColliMode = COLLI_RUSH;
		mAnchor    = mBody.mPos;
		mAnchor.y -= 55.0f;
		mBody.mVel.y = 3.0f;
		mStep        = 0;
		mTurnTo      = 0;
		mTime        = 0.0f;
		break;
	case 1: {
		f32 sin;
		f32 cos;

		motionCtrlUpdate(&mAnim);
		updatePose();
		switch (mStep) {
		case 0:
			accel();
			riseTo(20.0f - mAnchor.y);
			if (motionCtrlFrame(&mAnim) >= 10.0f) {
				mStep = 1;
				seStartAtRange(0x1A5, &mBody.mPos, 0x3C, 700.0f);
				aimVel();
			}
			stepSound(1.0f);
			break;
		case 1: {
			Vec3f flat;

			steer();
			riseTo(12.0f - mAnchor.y);
			if (motionCtrlIsDone(&mAnim)) {
				motionCtrlPlayBlend(0.2f, &mAnim, 7);
			}
			flat.x = mBody.mPos.x;
			flat.z = mBody.mPos.z;
			if (vecDotXZ(&flat, &mBody.mVel) > 0.0f && lastExDistFromCenter(&mBody.mPos) > 230.0f) {
				motionCtrlPlayBlend(0.2f, &mAnim, 8);
				mStep = 2;
			}
			break;
		}
		case 2:
			stepSound(6.0f);
			if (motionCtrlIsDone(&mAnim)) {
				motionCtrlPlayBlend(0.2f, &mAnim, 2);
			}
		case 3:
			unroll();
			riseTo(55.0f - mAnchor.y);
			if (lastExDistFromCenter(&mBody.mPos) >= 350.0f) {
				setDone();
			}
			break;
		}
		njAddVector(&mAnchor, &mBody.mVel);
		if (!(mBossFlags & EX_STOMPED) && (f32)__fabs(mAnchor.y - 12.0f) < 2.0f) {
			mBossFlags |= EX_STOMPED;
			seStartAtFar(0x1A6, &mBody.mPos);
		}
		njSinCos(mBody.mRot.z, &sin, &cos);
		sin = (f32)__fabs(sin);
		f32 lift = 55.0f * cos + 15.0f * sin;
		mBody.mPos = mAnchor;
		mBody.mPos.y += lift;
		updateShadow();
		if (mStep == 1) {
			mCount++;
			if ((mCount & 7) == 0) {
				int rot = mBody.mRot.y;
				Vec3f at;

				at.y = 0.0f;
				njSinCos(rot, &at.x, &at.z);
				gjScaleVector(60.0f, &at, &at);
				njAddVector(&at, &mAnchor);
				fxSpawnTurned(5, (void*)0x2D, (const Vec3f*)&at, 0, rot);
			}
		}
		calcPoints();
		break;
	}
	case 2:
		mColliMode = COLLI_BODY;
		mBossFlags &= ~EX_ATTACK;
		mBossFlags &= ~EX_STOMPED;
		mBody.mRot.x = 0;
		mBody.mRot.z = 0;
		break;
	}
}

// 0x801687D4
void TEneLastExBoss::actApproach(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0);
		clearFrameKey();
		mStep   = 0;
		mTurnTo = angleToMoveTo();
		break;
	case 1: {
		f32 left;

		motionCtrlUpdate(&mAnim);
		updatePose();
		left = moveToward();
		turnTo(mTurnTo, 0x200);
		switch (mStep) {
		case 0:
			riseToward(110.0f);
			njAddVector(&mBody.mPos, &mBody.mVel);
			stepSound(5.0f);
			if (left <= 70.0f) {
				mStep = 1;
				motionCtrlPlayBlend(0.1f, &mAnim, 9);
			}
			break;
		case 1:
			riseToward(110.0f);
			njAddVector(&mBody.mPos, &mBody.mVel);
			if (!motionCtrlIsBlending(&mAnim) && motionCtrlFrame(&mAnim) >= 35.0f) {
				mStep        = 2;
				mBody.mVel.y = -((mBody.mPos.y - 70.0f) / 20.0f);
			}
			break;
		case 2:
			njAddVector(&mBody.mPos, &mBody.mVel);
			if (mBody.mPos.y - 20.0f <= 50.0f) {
				mBody.mPos.y = 70.0f;
				mBody.mVel.y = 0.0f;
				mStep        = 3;
			}
			break;
		case 3:
			waitMotion();
			break;
		}
		if (mStep == 1 || mStep == 2) {
			stepSound(8.0f);
			stepSound(24.0f);
			stepSound(38.0f);
		}
		updateShadow();
		calcPoints();
		break;
	}
	case 2:
		break;
	}
}

// 0x801689FC
void TEneLastExBoss::actBurstUp(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0xA);
		clearFrameKey();
		setHeight();
		mStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		switch (mStep) {
		case 0:
			if (motionCtrlIsDone(&mAnim)) {
				mStep = 1;
				motionCtrlPlayBlend(0.2f, &mAnim, 0xB);
				mBossFlags |= EX_ATTACK;
				seStartAtFar(0x1A2, &mBody.mPos);
				seStartAtFar(0x1AC, &mBody.mPos);
			}
			break;
		case 1:
			setDone();
			break;
		}
		dampAndMove(0.9f);
		updateShadow();
		calcPoints();
		break;
	case 2:
		mBossFlags &= ~EX_ATTACK;
		break;
	}
}

// 0x80168B24
void TEneLastExBoss::actBurstMid(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0xC);
		mBossFlags |= EX_ATTACK;
		clearFrameKey();
		setHeight();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		dampAndMove(0.9f);
		updateShadow();
		calcPoints();
		if (motionCtrlPassed(16.0f, &mAnim)) {
			dropLastBossShot(&mPoint[13]);
			seStartAtFar(0x1A3, &mBody.mPos);
			seStartAtFar(0x1A4, &mBody.mPos);
		}
		break;
	case 2:
		mBossFlags &= ~EX_ATTACK;
		break;
	}
}

// 0x80168C18
void TEneLastExBoss::actBurstEnd(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0xD);
		mBossFlags |= EX_ATTACK;
		clearFrameKey();
		setHeight();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		dampAndMove(0.9f);
		updateShadow();
		calcPoints();
		break;
	case 2:
		mBossFlags &= ~EX_ATTACK;
		break;
	}
}

// 0x80168CD0
void TEneLastExBoss::actRise(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0xE);
		clearFrameKey();
		setHeight();
		mStep   = 0;
		mTurnTo = angleToMoveTo();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		switch (mStep) {
		case 0:
			dampAndMove(0.9f);
			if (motionCtrlFrame(&mAnim) >= 30.0f) {
				mStep        = 1;
				mBody.mVel.y = (110.0f - mBody.mPos.y) / 50.0f;
				mBody.mVel.y = 0.0f;
			}
			break;
		case 1: {
			int left;

			mBody.mRot.y = angleApproachDiff(mBody.mRot.y, mTurnTo, 0x80, &left);
			if (motionCtrlIsDone(&mAnim) && left == 0) {
				setDone();
			}
			damp(0.9f);
			mBody.mVel.y = approachSpeed(mBody.mVel.y, 110.0f - mBody.mPos.y, 0.05f, 0.1f, 0.2f, 0.8f, 0.01f);
			njAddVector(&mBody.mPos, &mBody.mVel);
			if (mBody.mPos.y >= 110.0f) {
				mBody.mPos.y = 110.0f;
			}
			break;
		}
		}
		stepSound(1.0f);
		stepSound(28.0f);
		stepSound(47.0f);
		updateShadow();
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x80168E94
void TEneLastExBoss::actCross(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0);
		clearFrameKey();
		mStep   = 0;
		mTurnTo = angleToMoveTo();
		break;
	case 1: {
		f32 left;

		motionCtrlUpdate(&mAnim);
		updatePose();
		left = moveToward();
		riseToward(110.0f);
		turnTo(mTurnTo, 0x200);
		switch (mStep) {
		case 0:
			if (left <= 70.0f) {
				mStep   = 1;
				mTurnTo = lastExAngleFromCenter(&mMoveTo);
				mTurnTo = mTurnTo + 0x10000 - 0x8000;
			}
			break;
		case 1:
			if (left <= 1.0f) {
				setDone();
			}
			break;
		}
		njAddVector(&mBody.mPos, &mBody.mVel);
		stepSound(5.0f);
		updateShadow();
		calcPoints();
		break;
	}
	case 2:
		break;
	}
}

// 0x80168FE4
void TEneLastExBoss::actFeathers(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0xF);
		clearFrameKey();
		mBossFlags |= EX_ATTACK;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		damp(0.9f);
		riseToward(110.0f);
		turnTo(mAimRot, 0x200);
		njAddVector(&mBody.mPos, &mBody.mVel);
		updateShadow();
		calcPoints();
		if (motionCtrlPassed(35.0f, &mAnim)) {
			int which = (int)(8.0f * (3.0517578E-5f * (f32)rand()));
			Vec3f at;
			int i;

			if (which >= 8) {
				which = 7;
			}
			at.x = mAim.x;
			at.y = 20.0f;
			at.z = mAim.z;
			new TEneLastExBossFeather(this, &mPoint[sLastExFeatherPoints[which]], &at);
			for (i = 1; i < mFeatherCount; i++) {
				lastExRandomPoint(220.0f, &at);
				which++;
				if (which >= 4) {
					which = 0;
				}
				new TEneLastExBossFeather(this, &mPoint[sLastExFeatherPoints[which]], &at);
			}
			seStartAtFar(0x1A0, &mBody.mPos);
		}
		break;
	case 2:
		mBossFlags &= ~EX_ATTACK;
		break;
	}
}

// 0x801691F0
// The body comes apart from where it was hit
void TEneLastExBoss::actBlowUp(int phase)
{
	switch (phase) {
	case 0:
		clearFrameKey();
		smallShadow();
		mBossFlags |= EX_FADING;
		mAlpha = 0xFF;
		mCore->appear();
		mCorePos = mPoint[8];
		mCore->setPos(&mCorePos);
		gatherVerts();
		mTime    = 10.0f;
		mBlastAt = mContact.mSurface;
		seStartAtFar(0x1A1, &mBody.mPos);
		seStartAtFar(0x1AB, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		moveVerts();
		mTime += 2.0f;
		blowVerts(mTime, 9.0f, &mBlastAt);
		mAlpha -= 8;
		if (mAlpha < 0) {
			mAlpha = 0;
			if (mTime >= 250.0f) {
				setDone();
			}
		}
		dampAndMove(0.9f);
		updateShadow();
		calcPoints();
		mCorePos = mPoint[8];
		mCore->setPos(&mCorePos);
		break;
	case 2:
		gatherVerts();
		bigShadow();
		mAlpha = 0xFF;
		mBossFlags &= ~EX_FADING;
		mCore->hide();
		break;
	}
}

// 0x801693B0
void TEneLastExBoss::actCoreRun(int phase)
{
	switch (phase) {
	case 0:
		clearFrameKey();
		smallShadow();
		dropVerts();
		mBossFlags |= EX_CORE;
		mCore->show();
		mCorePos = mPoint[8];
		mCore->setPos(&mCorePos);
		mColliMode = COLLI_CORE;
		mStep      = 0xA;
		break;
	case 1: {
		f32 left = coreToward(5.0f);

		if (left <= 1.0f) {
			mStep = 0xA;
			setDone();
		} else if (left >= 30.0f) {
			mStep--;
			if (mStep == 0) {
				seStartAtFar(0x1AA, &mBody.mPos);
			}
		}
		moveCore();
		mCore->setPos(&mCorePos);
		updateShadow();
		break;
	}
	case 2:
		gatherVerts();
		bigShadow();
		mBossFlags &= ~EX_CORE;
		mCore->hide();
		mColliMode = COLLI_BODY;
		break;
	}
}

// 0x80169504
void TEneLastExBoss::actCoreHit(int phase)
{
	switch (phase) {
	case 0:
		smallShadow();
		dropVerts();
		mBossFlags |= EX_CORE;
		mCore->show();
		mCorePos = mPoint[8];
		mCore->setPos(&mCorePos);
		mColliMode = COLLI_CORE;
		mEnemyFlags |= ENEMY_FLIPPING;
		mBody.mVel.x = mHitDir.x;
		mBody.mVel.z = mHitDir.z;
		mStep        = 0x1E;
		break;
	case 1:
		gjScaleVector(0.7f, &mBody.mVel, &mBody.mVel);
		moveCore();
		mCore->setPos(&mCorePos);
		mStep--;
		if (mStep <= 0) {
			setDone();
		}
		updateShadow();
		break;
	case 2:
		mEnemyFlags &= ~ENEMY_FLIPPING;
		gatherVerts();
		bigShadow();
		mBossFlags &= ~EX_CORE;
		mCore->hide();
		mColliMode = COLLI_BODY;
		break;
	}
}

// 0x80169648
void TEneLastExBoss::actRebuild(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x11);
		clearFrameKey();
		smallShadow();
		mCore->vanish();
		mCorePos = mPoint[8];
		mCore->setPos(&mCorePos);
		mBossFlags |= EX_FADING;
		mAlpha = 0;
		dropVerts();
		seStartAtFar(0x19E, &mBody.mPos);
		seStartAtFar(0x1AB, &mBody.mPos);
		mStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		mAlpha += 8;
		if (mAlpha >= 0xFF) {
			mAlpha = 0xFF;
			mBossFlags &= ~EX_FADING;
		} else {
			mBossFlags |= EX_FADING;
		}
		switch (mStep) {
		case 0:
			mBody.mPos.y = mCorePos.y - mPoint[8].y;
			gatherVerts();
			mStep = 1;
			break;
		case 1:
			break;
		}
		calcPoints();
		break;
	case 2:
		gatherVerts();
		bigShadow();
		mAlpha = 0xFF;
		mBossFlags &= ~EX_FADING;
		mCore->hide();
		break;
	}
}

// 0x801697E0
void TEneLastExBoss::actDie(int phase)
{
	switch (phase) {
	case 0:
		mStep  = 0;
		mCount = 0x12;
		mTint.start();
		mEnemyFlags |= ENEMY_BLOCKED;
		colliDisable(mCollider);
		smallShadow();
		dropVerts();
		mBossFlags |= EX_CORE;
		break;
	case 1:
		if (mStep == 0) {
			mCount--;
			if (mCount >= 0) {
				break;
			}
			mBody.mPos.x = 0.0f;
			mBody.mPos.y = 0.0f;
			mBody.mPos.z = 0.0f;
			mBody.mRot.x = 0;
			mBody.mRot.y = 0;
			mBody.mRot.z = 0;
			mEnemyFlags |= ENEMY_INTANGIBLE | ENEMY_BLOCKED;
			colliDisable(mCollider);
			updateVerts();
			mCore->burst();
			armLastExBg();
			playDemo(0, 0);
			seStartArg(0x2B7, 0x2EE);
			seCueSet(&mSe, 0x69, 0xA5);
			mStep = 1;
			mTime = 0.0f;
		}
		mTime += 1.0f;
		lockOn();
		if (mTime >= 600.0f) {
			mBossFlags |= EX_GLOW;
			mGlow.start(5, 0x3D);
		}
		seCueUpdate(&mSe, (int)mTime);
		if (mTime >= 730.0f) {
			mBossFlags &= ~EX_GLOW;
			fxHolderStop(&mGlow);
			slain();
			sendBossDefeated(this);
			mFlags |= 1;
		}
		break;
	case 2:
		mEnemyFlags &= ~(ENEMY_INTANGIBLE | ENEMY_BLOCKED);
		clearFrameKey();
		break;
	}
}

// 0x801699DC
void TEneLastExBoss::vfn_3C()
{
	njPrint((kEnemyInfoX << 16) | kEnemyInfoLine, ":HP %4d", mHealth);
}

// 0x80169A18
void TEneLastExBoss::vfn_10()
{
	if (mBossFlags & EX_GLOW) {
		Vec3f* camera = (Vec3f*)camGetDrawView();

		if (camera != 0) {
			Vec3f at;
			at.x = camera[0].x;
			at.y = camera[0].y;
			at.z = camera[0].z;
			f32 angle;
			void* mtx;

			gjMulAddVector(512.0f, &at, &at, &camera[2]);
			gjMulAddVector(158.0f, &at, &at, &camera[4]);
			angle = atan2(camera[2].x, camera[2].z);
			mtx   = mGlow.mMtx.m;
			mtxSetTransV((f32*)mtx, &at);
			mtxRotateY((f32*)mtx, (const f32*)mtx, (int)(10430.38043493439 * angle));
		}
	}
	if (isAppearing() || (mBossFlags & EX_HIDDEN)) {
		mFlags |= 0x10;
	} else if (mBossFlags & EX_CORE) {
		mFlags |= 0x10;
	} else if (!mLight.isVisible(mDrawTask, 3000.0f, &mBody.mPos, 30.0f, 0)) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
		if (mBossFlags & (EX_SUIT | EX_FADING)) {
			moveDrawTask(mDrawTask, 0x22);
		}
	}
}

// 0x80169B90
void TEneLastExBoss::render()
{
	if (mBossFlags & EX_SUIT) {
		texFxSuitLoadTex(2);
	}
	njSetTexture(*sLastExBossTextures);
	if (mBossFlags & EX_SUIT) {
		gjSetBlendOverride(1, 4, 5, 3);
		_nj_control_3d_flag_ |= 0x8000;
		texFxSuitBegin(0, *(u32*)&mSuit);
	}
	if (!(mBossFlags & (EX_FADING | EX_CORE))) {
		fogOff();
		njPushMatrixEx();
		njTranslateEx(&mPoint[11]);
		gjRotateY(mBody.mRot.y);
		drawObjectDefault(sLastExBossModels[2]);
		njPopMatrixEx();
		njPushMatrixEx();
		njTranslateEx(&mPoint[12]);
		gjRotateY(mBody.mRot.y);
		drawObjectDefault(sLastExBossModels[3]);
		njPopMatrixEx();
		fogApply();
	}
	njPushMatrixEx();
	bodyMatrix();
	if (mBossFlags & EX_SUIT) {
		shadowAlphaBegin();
		gjDrawActor(mPose);
		shadowAlphaEnd();
	} else if (mBossFlags & EX_FADING) {
		if (mAlpha > 0) {
			ObjDrawState state;
			DrawStateSlot* slot = getDrawStateSlot();

			state = *(const DrawState*)getDefaultDrawState();
			state.mFlags |= 0x200;
			state.mAmbient = mAlpha & 0xFF;
			mTint.addToLight((u32*)&state);
			beginDrawState(slot, (const DrawState*)&state);
			shadowAlphaBegin();
			gjDrawActor(mPose);
			shadowAlphaEnd();
			endDrawState((DrawStateSlot*)slot);
		}
	} else {
		drawLit(&mLight, &mTint, mPose);
	}
	njPopMatrixEx();
}

// 0x80169D68
void LastExBossAiBase::tick(TEneLastExBoss* owner)
{
	mOwner = owner;
	mNext  = -1;
	if (!checkInterrupt()) {
		onPhase(mState, 2);
	}
	onPhase(mState, 1);
	if (mNext != -1) {
		mOwner->setAction(mNext);
	}
}

// 0x80169E00
LastExBossAiState::LastExBossAiState()
{
	mOwner = 0;
}

// 0x80169E34
LastExBossAiState::~LastExBossAiState()
{
}

// 0x80169E90
extern "C" LastExBossAi* createLastExBossAi(TEneLastExBoss* owner)
{
	return new LastExBossAi(owner);
}

// 0x80169ED8
LastExBossAi::LastExBossAi(TEneLastExBoss* owner)
{
	mOwner      = owner;
	mHealthMark = sLastExParams[0];
	mNextAttack = 5;
	mReset      = 0;
	(&mSight)->reset();
	mSight.mHeightLimit = 1000.0f;
	mStep               = 0;
	mCount              = 0;
	_54                 = 0;
	_58                 = 0;
	if (mOwner != 0) {
		mNext  = -1;
		mState = 0;
		onPhase(mState, 0);
		if (mNext != -1) {
			mOwner->setAction(mNext);
		}
	}
}

// 0x80169FB8
LastExBossAi::~LastExBossAi()
{
}

// 0x8016A020
int LastExBossAi::checkInterrupt()
{
	if (mReset != 0) {
		mReset = 0;
		placeAtStart();
		changeState(2);
	} else {
		if (mOwner->isDead() && mState != 7) {
			changeState(7);
			return 1;
		}
		if (!mOwner->wasHitHard() && mOwner->wasHitKind1()) {
			mOwner->onLightHit();
		}
	}
	return 0;
}

// 0x8016A130
void LastExBossAi::track()
{
	(&mSight)->findInRange(100000000.0f, mOwner->getPos());
}

// 0x8016A188
void LastExBossAi::aimAtTarget()
{
	mOwner->setAim(&mSight.mTarget, mSight.mAngle);
}

// 0x8016A1B8
void LastExBossAi::rememberHealth()
{
	mHealthMark = mOwner->mHealth;
}

// 0x8016A1C8
int LastExBossAi::lostEnough()
{
	int health = mOwner->mHealth;
	int phase  = thresholdIndex(health, sLastExParams + 1);

	return mHealthMark - health > sLastExParams[phase + 7];
}

// 0x8016A234
int LastExBossAi::getPhase()
{
	return thresholdIndex(mOwner->mHealth, sLastExParams + 1);
}

// 0x8016A264
int LastExBossAi::checkHardHit()
{
	if (mOwner->wasHitHard()) {
		changeState(6);
		return 1;
	}
	return 0;
}

// 0x8016A2E8
void LastExBossAi::pickNextAttack()
{
	if (getPhase() > 0) {
		if (mNextAttack == 2) {
			mNextAttack = 5;
			changeState(2);
		} else {
			mNextAttack = 2;
			changeState(5);
		}
	} else {
		changeState(2);
	}
}

// 0x8016A400
void LastExBossAi::startFog()
{
	if (getPhase() >= 2) {
		mOwner->startFog();
	}
}

// 0x8016A43C
void LastExBossAi::stopFog()
{
	mOwner->stopFog();
}

// 0x8016A460
void LastExBossAi::setFeathers()
{
	getPhase();
	mOwner->setFeatherCount(0xF);
}

// 0x8016A498
void LastExBossAi::moveToFlank()
{
	Vec3f at;

	lastExOutward(mOwner->getPos(), &at);
	gjScaleVector(-350.0f, &at, &at);
	vecRotateY(&at, &at, 3.0517578E-5f * (f32)rand() >= 0.5f ? 0x3000 : -0x3000);
	lastExOnFloor(&at, &at);
	mOwner->setMoveTo(&at);
}

// 0x8016A554
void LastExBossAi::placeAtStart()
{
	Vec3f at;

	at.x = 0.0f;
	at.y = 20.0f;
	at.z = 350.0f;
	mOwner->placeAt(&at);
}

// 0x8016A594
void LastExBossAi::moveToRandom()
{
	Vec3f at;

	lastExRandomPoint(160.0f, &at);
	mOwner->setMoveTo(&at);
}

// 0x8016A5D4
int LastExBossAi::getStunFrames()
{
	int phase = getPhase();
	return sLastExParams[phase + 4];
}

// 0x8016A604
void LastExBossAi::requestReset()
{
	mReset = 1;
}

// 0x8016A610
void LastExBossAi::onPhase(int state, int phase)
{
	switch (state) {
	case 0: stateIntro(phase); break;
	case 1: stateIdle(phase); break;
	case 2: stateRush(phase); break;
	case 3: stateBurst(phase); break;
	case 4: stateCross(phase); break;
	case 5: stateFeathers(phase); break;
	case 6: stateCore(phase); break;
	case 7: stateDie(phase); break;
	}
}

// 0x8016A6A8
void LastExBossAi::stateIntro(int phase)
{
	switch (phase) {
	case 0:
		mNext = 0;
		mOwner->setMaxHealth(sLastExParams[0]);
		break;
	case 2:
		if (mOwner->isDone0()) {
			changeState(2);
		}
		break;
	case 3:
		placeAtStart();
		break;
	}
}

// 0x8016A768
void LastExBossAi::stateIdle(int phase)
{
	switch (phase) {
	case 0:
		mNext = 1;
		break;
	case 2:
		if (!checkHardHit() && mOwner->isDone1()) {
			changeState(2);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		break;
	case 3:
		break;
	}
}

// 0x8016A828
void LastExBossAi::stateRush(int phase)
{
	switch (phase) {
	case 0:
		mNext  = 1;
		mStep  = 0;
		mCount = 2;
		break;
	case 2:
		if (!checkHardHit() && mCount <= 0) {
			changeState(3);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		switch (mStep) {
		case 0:
			if (mOwner->isDone1()) {
				mStep = 1;
				mNext = 2;
				startFog();
			}
			break;
		case 1:
			if (mOwner->isDone2()) {
				mCount--;
				if (mCount > 0) {
					mStep = 0;
					mNext = 1;
				} else {
					mStep = 2;
				}
			}
			break;
		case 2:
			break;
		}
		break;
	case 3:
		stopFog();
		break;
	}
}

// 0x8016A994
void LastExBossAi::stateBurst(int phase)
{
	switch (phase) {
	case 0:
		mNext = 1;
		mStep = 0;
		break;
	case 2:
		if (!checkHardHit() && mStep == 5) {
			changeState(4);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		switch (mStep) {
		case 0:
			if (mOwner->isDone1()) {
				Vec3f at;

				lastExOutward(mOwner->getPos(), &at);
				at.x *= 160.0f;
				at.z *= 160.0f;
				lastExOnFloor(&at, &at);
				mOwner->setMoveTo(&at);
				mNext = 3;
				mStep = 1;
			}
			break;
		case 1:
			if (mOwner->isDone3()) {
				mNext  = 4;
				mStep  = 2;
				mCount = 0x3C;
			}
			break;
		case 2:
			if (mOwner->isDone4()) {
				mCount--;
				if (mCount <= 0) {
					mStep  = 3;
					mCount = 0;
					mNext  = 5;
				}
			}
			break;
		case 3:
			if (mOwner->isDone5()) {
				mStep  = 4;
				mCount = 0x3C;
				mNext  = 6;
			}
			break;
		case 4:
			mCount--;
			if (mCount <= 0) {
				mStep = 5;
				mNext = 7;
			}
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8016ABC0
void LastExBossAi::stateCross(int phase)
{
	switch (phase) {
	case 0:
		mNext = 7;
		mStep = 0;
		moveToFlank();
		break;
	case 2:
		if (!checkHardHit() && mOwner->isDone8()) {
			pickNextAttack();
		}
		break;
	case 1:
		track();
		aimAtTarget();
		switch (mStep) {
		case 0:
			if (mOwner->isDone7()) {
				mNext = 8;
			}
			break;
		case 1:
			break;
		}
		break;
	case 3:
		break;
	}
}

// 0x8016AC88
void LastExBossAi::stateFeathers(int phase)
{
	switch (phase) {
	case 0:
		mNext = 1;
		mStep = 0;
		break;
	case 2:
		if (!checkHardHit() && mOwner->isDone9()) {
			changeState(3);
		}
		break;
	case 1:
		track();
		aimAtTarget();
		if (mStep == 0 && mOwner->isDone1()) {
			setFeathers();
			mNext = 9;
			mStep = 1;
			startFog();
		}
		break;
	case 3:
		stopFog();
		break;
	}
}

// 0x8016AD9C
void LastExBossAi::stateCore(int phase)
{
	switch (phase) {
	case 0:
		mNext  = 0xA;
		mStep  = 0;
		mCount = getStunFrames();
		break;
	case 2:
		if (mOwner->isDone8()) {
			pickNextAttack();
		}
		break;
	case 1:
		track();
		aimAtTarget();
		switch (mStep) {
		case 0:
			if (mOwner->isDone10()) {
				mStep = 1;
				rememberHealth();
				mOwner->setOpen();
				moveToRandom();
				mNext = 0xB;
			}
			break;
		case 1:
			if (mOwner->wasHitHard()) {
				mOwner->onHardHit();
				mStep = 2;
				mNext = 0xC;
			}
			if (lostEnough()) {
				mOwner->clearOpen();
				mCount = 0;
			}
			mCount--;
			if (mCount <= 0) {
				Vec3f at;

				mStep = 3;
				mOwner->clearOpen();
				at   = *mOwner->getPos();
				at.y = 20.0f;
				mOwner->setMoveToHigh(&at);
				mNext = 0xB;
			}
			break;
		case 2:
			if (mOwner->wasHitHard()) {
				mOwner->onHardHit();
				if (mCount > 0) {
					mNext = 0xC;
				}
			}
			if (lostEnough()) {
				mOwner->clearOpen();
				mCount = 0;
			}
			mCount--;
			if (mOwner->isDone12()) {
				mStep = 1;
				moveToRandom();
				mNext = 0xB;
			}
			break;
		case 3:
			if (mOwner->isDone11()) {
				mStep = 4;
				mNext = 0xD;
			}
			break;
		case 4:
			if (mOwner->isDone13()) {
				moveToFlank();
				mStep = 5;
				mNext = 8;
			}
			break;
		case 5:
			break;
		}
		break;
	case 3:
		mOwner->clearOpen();
		break;
	}
}

// 0x8016B03C
void LastExBossAi::stateDie(int phase)
{
	switch (phase) {
	case 0:
		mNext = 0xE;
		break;
	case 1:
		break;
	case 2:
		break;
	}
}
