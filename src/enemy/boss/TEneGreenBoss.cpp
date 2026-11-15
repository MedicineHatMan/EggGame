#include "io/ResBank.h"
#include "enemy/boss/TEneGreenBoss.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "enemy/boss/BossParts.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "engine/MotionPose.h"
#include "stage/SetManager.h"
#include "engine/CharaColli.h"
#include "enemy/EnemyHit.h"
#include "enemy/ObjEnemy.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "object/ObjAction.h"
#include "enemy/EnemyFx.h"
#include "object/ObjShadow.h"
#include "camera/TCameraCtrl.h"
#include "engine/TParticle.h"
#include "ui/AeUtil.h"
#include "enemy/EnemyTexFx.h"
#include "enemy/EnemyCommon.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "enemy/TEneAm01.h"
struct TObject;
struct Vec3f;
struct NJS_TEXLIST;

struct GjActor;

ResBank sGreenBossBank;   // 0x802861E8
NJS_OBJECT** sGreenBossModels;  // 0x802C57F0
NJS_TEXLIST** sGreenBossTextures;
NJS_MOTION** sGreenBossMotion;
extern "C" {
GreenBossParams* gGreenBossParams;       // 0x802C57FC
}
FxBankFile sGreenBossFx;  // 0x802C5800

// 0x8024A2B8
// The nodes the body points follow -1 at the end
extern const int sGreenBossNodes[] = { 6, 11, 14, 22, 23, 25, 27, 104, 111, 118, 125, 131, 137, -1 };
const char* sGreenBossName = "TEneGreenBoss";
extern const EnemyFrameKey sGreenBossFrameKey0[] = { { 18.0f, 0 }, { 28.0f, 5 }, { 41.0f, 0 }, { 47.0f, 3 }, { -1.0f, 0 } };
// 0x802614B8
// The twenty-three motions
MotionEntry sGreenBossMotionKeys[23] = {
	{ MOTION_LOOP, 20, -1, 0, 0.0f, 40.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 21, -1, 0, 0.0f, 40.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 22, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 16, -1, 0, 0.0f, 20.0f, 0.6f, 0.0f, 0 },
	{ MOTION_ONCE, 10, -1, 0, 0.0f, 29.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 5, -1, 0, 0.0f, 9.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 6, -1, 0, 0.0f, 10.0f, 0.2f, 0.0f, 0 },
	{ MOTION_ONCE, 7, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 23, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 24, -1, 0, 0.0f, 10.0f, 0.55f, 0.0f, 0 },
	{ MOTION_ONCE, 25, -1, 0, 0.0f, 19.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 20.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 12, -1, 0, 0.0f, 19.0f, 0.25f, 0.0f, 0 },
	{ MOTION_ONCE, 17, -1, 0, 0.0f, 29.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 18, -1, 0, 0.0f, 9.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 19.0f, 0.35f, 0.0f, 0 },
	{ MOTION_ONCE, 9, -1, 0, 0.0f, 24.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 9.0f, 0.35f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 9.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 19, -1, 0, 0.0f, 539.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 419.0f, 1.0f, 0.0f, 0 },
};

// Never called so the linker drops them
f32 greenBossWaterLine() { return 25.31f; }
f32 greenBossRand01() { return 3.0517578e-5f * rand(); }

// 0x8015F608
extern "C" void initTEneGreenBoss()
{
	resBankLoad(&sGreenBossBank, "ar_ene_green_boss.arc");
	resBankAddTextures(&sGreenBossBank, 0, "ene_green_boss.gvm");
	sGreenBossModels = resBankGetModels(&sGreenBossBank);
	sGreenBossTextures = resBankGetTextures(&sGreenBossBank);
	sGreenBossMotion = resBankGetMotion(&sGreenBossBank, 0);
	gGreenBossParams = (GreenBossParams*)enemyParamGet(1);
	fxBankLoadDefault(&sGreenBossFx, "pa_greenboss.pad");
}

// 0x8015F6A4
extern "C" void shutdownTEneGreenBoss()
{
	fxBankRelease(&sGreenBossFx);
	gGreenBossParams = 0;
	resBankRemoveTextures(&sGreenBossBank, 0);
	resBankRelease(&sGreenBossBank);
	sGreenBossModels = 0;
	sGreenBossTextures = 0;
	sGreenBossMotion = 0;
}

// 0x8015F6FC
extern "C" TObject* createTEneGreenBoss(TObject* parent, SetDesc* desc)
{
	return new TEneGreenBoss(parent, desc);
}

// 0x8015F750
extern "C" void* paramsTEneGreenBoss()
{
	return (int*)sGreenBossNodes;
}

// 0x8015F75C
TEneGreenBoss::TEneGreenBoss(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	readEnemyDesc(desc);
	init();
	initBossBar();
	makeBrain();
	setBossView(10.0f, 33.0f, 150.0f);
}

// 0x8015F844
void TEneGreenBoss::init()
{
	int j;
	int i;
	ColliShape* shapes;

	_00   = (u32)sGreenBossName;
	mSize = 0x444;
	motionCtrlSetPlayer(&mAnim, &mMotion);
	motionCtrlInit(&mAnim, sGreenBossMotionKeys, sGreenBossMotion);
	motionCtrlPlay(&mAnim, 0);
	mBody.mPos.x = 0.0f;
	mBody.mPos.y = 0.0f;
	mBody.mPos.z = 0.0f;
	mBody.mPos.y = 25.0f;
	mBody.mRot.x = 0;
	mCollider = colliCreate(5, 1);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0xFF, 0, 0x14);
	shapes = colliGetShapes(mCollider);
	colliShapeMakeSphere(&shapes[0], &mBody.mPos, 0, 12.0f);
	colliShapeMakeSphere(&shapes[1], &mBody.mPos, 0, 8.0f);
	colliShapeMakeSphere(&shapes[2], &mBody.mPos, 0, 8.0f);
	colliShapeMakeSphere(&shapes[3], &mBody.mPos, 0, 7.0f);
	colliShapeMakeSphere(&shapes[4], &mBody.mPos, 0, 7.0f);
	mSwingColli = colliCreate(1, 1);
	colliSetOwner(mSwingColli, (u16)mObjId);
	colliSetMasks(mSwingColli, 2, 0xFF, 0xFF, 0xF6);
	shapes = colliGetShapes(mSwingColli);
	colliShapeMakeSphere(shapes, &mBody.mPos, 0, gGreenBossParams->mSwingRadius);
	shapes->mFlags |= 8;
	shapes->mFlags |= 0x10;
	colliDisable(mSwingColli);
	mBoxColli = colliCreate(1, 1);
	colliSetOwner(mBoxColli, (u16)mObjId);
	colliSetMasks(mBoxColli, 2, 0xFF, 0xFF, 0xF6);
	shapes = colliGetShapes(mBoxColli);
	colliShapeMakeBox(shapes, &mBody.mPos, mBody.mRot.y, gGreenBossParams->mBoxSize[0], gGreenBossParams->mBoxSize[1], gGreenBossParams->mBoxSize[2], 1);
	shapes->mFlags |= 8;
	shapes->mFlags |= 0x10;
	colliDisable(mBoxColli);
	j = 0;
	for (i = 0; i < 5; i++) {
		colliShapeSetTag(mCollider, i, j);
		j++;
	}
	colliShapeSetTag(mSwingColli, 0, j);
	j++;
	colliShapeSetTag(mBoxColli, 0, j++);
	mBrain   = 0;
	mModelFx = 0;
	mPose    = (GjActor*)allocPose(sGreenBossModels[1]);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mBossFlags = 0;
	mHitDir.x  = 0.0f;
	mHitDir.y  = 0.0f;
	mHitDir.z  = 0.0f;
	mFrameKey  = 0;
	for (i = 0; i < 13; i++) {
		mNodes[i].x = 0.0f;
		mNodes[i].y = 0.0f;
		mNodes[i].z = 0.0f;
	}
	mMoveTarget.x = 0.0f;
	mMoveTarget.y = 0.0f;
	mMoveTarget.z = 0.0f;
	mLookTarget.x = 0.0f;
	mLookTarget.y = 0.0f;
	mLookTarget.z = 0.0f;
	mLookAngle    = 0;
	mMaxHealth    = 500;
	mHealth       = 500;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 20.0f;
	mShadow._34 = 20.0f;
	shadowEnable(&mShadow);
	mLight.reset();
	mTint.reset();
	for (i = 0; i < 10; i++) {
		mFx[i] = 0;
	}
	mStepFx.stop();
	mSlamFx.stop();
	mSwingFx.stop();
	seCueClear(&mSe);
	mActStep   = 0;
	mActTimer  = 0;
	mBaseAngle = 0;
	mActTime   = 0.0f;
	mScale     = 1.0f;
	mSpeed     = 1.0f;
}

// 0x8015FBE8
TEneGreenBoss::~TEneGreenBoss()
{
	int i;

	exitAction(mAction);
	for (i = 9; i >= 0; i--) {
		if (mFx[i] != 0) {
			mFx[i]->mFlags |= 1;
			mFx[i] = 0;
		}
	}
	mStepFx.stop();
	mSlamFx.stop();
	mSwingFx.stop();
	seLoopStop(&mLoop);
	if (mModelFx != 0) {
		delete mModelFx;
		mModelFx = 0;
	}
	if (mBrain != 0) {
		delete mBrain;
		mBrain = 0;
	}
	removeDrawTask(mDrawTask);
	freePose((void**)&mPose);
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x8015FD84
void TEneGreenBoss::setPos(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	mBody.mPos.x = x;
	f32 z = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x8015FDC8
f32 TEneGreenBoss::vfn_40()
{
	return 80.0f;
}

// 0x8015FDD0
// The five spheres on the nodes
void TEneGreenBoss::syncCollider()
{
	Vec3f mid;
	ColliShape* shapes = colliGetShapes(mCollider);

	shapes->mPos = mNodes[0];
	mid          = mNodes[1];
	njAddVector(&mid, &mNodes[2]);
	gjScaleVector(0.5f, &mid, &mid);
	colliShapeSetPos(&shapes[0], &mNodes[0]);
	colliShapeSetPos(&shapes[1], &mid);
	colliShapeSetPos(&shapes[2], &mNodes[4]);
	colliShapeSetPos(&shapes[3], &mNodes[7]);
	colliShapeSetPos(&shapes[4], &mNodes[9]);
	colliRefreshBounds(mCollider);
	colliBoundsOn(mCollider);
}

// 0x8015FE9C
void TEneGreenBoss::syncSwing()
{
	ColliShape* shapes = colliGetShapes(mSwingColli);

	shapes->mPos = mBody.mPos;
	colliShapeSetPos(shapes, &mBody.mPos);
	colliRefreshBounds(mSwingColli);
	colliBoundsOn(mSwingColli);
}

// 0x8015FEFC
// The box sits in front of it
void TEneGreenBoss::syncBox()
{
	ColliShape* shapes = colliGetShapes(mBoxColli);
	int rotY = mBody.mRot.y;
	Vec3f at;
	at.x = mBody.mPos.x;
	at.y = mBody.mPos.y;
	at.z = mBody.mPos.z;

	at.x = gGreenBossParams->mBoxSize[2] * njSin(rotY) + at.x;
	at.z = gGreenBossParams->mBoxSize[2] * njCos(rotY) + at.z;
	shapes->mPos = at;
	colliShapeSetBoxRot(shapes, rotY);
	colliShapeSetPos(shapes, &at);
	colliRefreshBounds(mBoxColli);
	colliBoundsOn(mBoxColli);
}

// 0x8015FFC4
void TEneGreenBoss::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
}

// 0x8015FFE4
// An egg hits only from the front arc while it spins
int TEneGreenBoss::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit = 0;

	switch (arg) {
	case 5: {
		int diff = (u16)angleToXZ(&mBody.mPos, ((TObjAction*)other)->getPos()) - (u16)mBaseAngle;
		if (diff > gGreenBossParams->mHitAngle || diff < -gGreenBossParams->mHitAngle) {
			return 0;
		}
		break;
	}
	}
	if (!(mBossFlags & BOSS_GUARD)) {
		if (isPlayerOrKind4(other)) {
			info->mDamage = 10;
			hit = 1;
			info->mFlags.clear();
		} else if (isEggObj(other)) {
			info->mDamage = 1;
			hit = 1;
			info->mFlags.clear();
			info->mFlags.setBit(0xD);
		}
	}
	return hit;
}

// 0x801600E0
void TEneGreenBoss::setLookTarget(const Vec3f* pos, int angle)
{
	mLookTarget = *pos;
	mLookAngle  = angle;
}

// 0x80160100
void TEneGreenBoss::setHealth(int health)
{
	mMaxHealth = health;
	mHealth    = health;
}

// 0x8016010C
void TEneGreenBoss::clearEngaged()
{
	mBossFlags &= ~BOSS_ENGAGED;
}

// 0x8016011C
void TEneGreenBoss::setEngaged()
{
	mBossFlags |= BOSS_ENGAGED;
}

// 0x8016012C
void TEneGreenBoss::clearGuard()
{
	mBossFlags &= ~BOSS_GUARD;
}

// 0x8016013C
void TEneGreenBoss::setGuard()
{
	mBossFlags |= BOSS_GUARD;
}

// 0x8016014C
void TEneGreenBoss::clearRecovered()
{
	mBossFlags &= ~BOSS_RECOVERED;
}

// 0x8016015C
void TEneGreenBoss::setRecovered()
{
	mBossFlags |= BOSS_RECOVERED;
}

// 0x8016016C
u32 TEneGreenBoss::isRecovered()
{
	return mBossFlags & BOSS_RECOVERED;
}

// 0x80160178
void TEneGreenBoss::clearFell()
{
	mBossFlags &= ~BOSS_FELL;
}

// 0x80160188
void TEneGreenBoss::setFell()
{
	mBossFlags |= BOSS_FELL;
}

// 0x80160198
void TEneGreenBoss::beginDeath()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
	hideBossBar();
	seStartAtFar(0x180, &mBody.mPos);
}

// 0x801601E8
void TEneGreenBoss::hurtReact()
{
	enemyFxSparks(0.0f, &mContact.mSurface, &mContact.mCenter);
	seStartAtFar(0x174, &mBody.mPos);
}

// 0x8016022C
int TEneGreenBoss::isAction0Done()
{
	return isActionDone(0);
}

// 0x80160250
int TEneGreenBoss::isAction1Done()
{
	return isActionDone(1);
}

// 0x80160274
int TEneGreenBoss::isAction2Done()
{
	return isActionDone(2);
}

// 0x80160298
int TEneGreenBoss::isAction3Done()
{
	return isActionDone(3);
}

// 0x801602BC
int TEneGreenBoss::isAction4Done()
{
	return isActionDone(4);
}

// 0x801602E0
int TEneGreenBoss::isAction5Done()
{
	return isActionDone(5);
}

// 0x80160304
int TEneGreenBoss::isAction6Done()
{
	return isActionDone(6);
}

// 0x80160328
int TEneGreenBoss::isAction7Done()
{
	return isActionDone(7);
}

// 0x8016034C
int TEneGreenBoss::isAction8Done()
{
	return isActionDone(8);
}

// 0x80160370
int TEneGreenBoss::isAction9Done()
{
	return isActionDone(9);
}

// 0x80160394
int TEneGreenBoss::isAction11Done()
{
	return isActionDone(11);
}

// 0x801603B8
int TEneGreenBoss::isAction12Done()
{
	return isActionDone(12);
}

// 0x801603DC
int TEneGreenBoss::isAction13Done()
{
	return isActionDone(13);
}

// 0x80160400
int TEneGreenBoss::isAction14Done()
{
	return isActionDone(14);
}

// 0x80160424
int TEneGreenBoss::isAction15Done()
{
	return isActionDone(15);
}

// 0x80160448
int TEneGreenBoss::isAction16Done()
{
	return isActionDone(16);
}

// 0x8016046C
int TEneGreenBoss::isAction20Done()
{
	return isActionDone(20);
}

// 0x80160490
void TEneGreenBoss::updateBody()
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
	setBossBar(mMaxHealth, mHealth);
}

// 0x8016056C
void TEneGreenBoss::vfn_6C()
{
	if (returnZero()) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginNodes();
		gjPushUnitMatrix();
		poseFirstFrame(sGreenBossMotion, 0x1A, mPose);
		njPopMatrixEx();
		endNodes();
		calcPoints();
		syncCollider();
	}
}

// 0x801605F0
void TEneGreenBoss::makeBrain()
{
	mBrain = createGreenBrain(this);
}

// 0x80160620
void TEneGreenBoss::beginNodes()
{
	nodeTrackBeginOrigins(sGreenBossNodes, mNodes);
}

// 0x80160650
void TEneGreenBoss::endNodes()
{
	nodeTrackEnd();
}

// 0x80160670
void TEneGreenBoss::updatePose()
{
	beginNodes();
	gjPushUnitMatrix();
	applyPose(mPose, &mAnim);
	njPopMatrixEx();
	if (hatchActive(&mAppearTimer)) {
		hatchPose(&mAppearTimer, (GjActor*)mPose, 60.0f);
	}
	endNodes();
}

// 0x801606DC
// The motion moves the body
void TEneGreenBoss::updatePoseRooted()
{
	MotionNodePose root;
	NJS_MOTION* node = motionCtrlGetMotion(&mAnim);

	motionGetNodePose(&root, *(NJS_OBJECT**)mPose, node, motionCtrlFrame(&mAnim));
	motionPoseGetPos(&root, &mBody.mPos);
	beginNodes();
	gjPushUnitMatrix();
	gjTranslate(-mBody.mPos.x, -mBody.mPos.y, -mBody.mPos.z);
	applyPose(mPose, &mAnim);
	njPopMatrixEx();
	endNodes();
}

// 0x80160780
void TEneGreenBoss::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 13; i++) {
		njCalcPointEx(&mNodes[i], &mNodes[i]);
	}
	njPopMatrixEx();
}

// 0x801607E0
int TEneGreenBoss::getHitKind(const void* touch, HitInfo* info)
{
	return (mBossFlags & BOSS_ENGAGED) ? 3 : 1;
}

// 0x801607F8
int TEneGreenBoss::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x80160810
void TEneGreenBoss::bodyMatrix()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
	gjRotateX(mBody.mRot.x);
	njScale(0, mScale, mScale, mScale);
}

// 0x80160864
void TEneGreenBoss::updateShadow()
{
	Vec3f at;
	Vec3f ground;
	Vec3f normal;

	at = mBody.mPos;
	at.y += 8.0f;
	if (probeGround(&at, &ground, &normal)) {
		shadowSetGround(&mShadow, &ground, &normal);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
}

// 0x801608EC
void TEneGreenBoss::spawnFx(int model, int motion)
{
	void* obj = resBankGetObjects(&sGreenBossBank)[model];
	NJS_MOTION** motions = resBankGetMotion(&sGreenBossBank, 1);

	mModelFx = createCamPath(0, 3, obj, motions[motion], 0, 1.0f, 1.0f);
	if (mModelFx != 0) {
		((TCamera*)mModelFx)->setCut();
	}
}

// 0x8016097C
// How far the node moved since the last pose as a velocity
void TEneGreenBoss::moveByNode(int node, Vec3f* out)
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
	moveAlong(1.0f, &from, &to, out);
	out->y += -0.12f;
}

// 0x80160A14
void TEneGreenBoss::damp(f32 rate)
{
	mBody.mVel.x *= rate;
	mBody.mVel.z *= rate;
}

// 0x80160A30
// It never sinks below the water
int TEneGreenBoss::moveBody(Vec3f* vel)
{
	Vec3f step;

	step.y = 0.0f;
	step.x = 0.0f;
	step.z = 0.0f;
	step.y = 0.0f;
	njAddVector(&step, vel);
	njAddVector(&mBody.mPos, &step);
	if (mBody.mPos.y <= 25.31f) {
		mBody.mPos.y = 25.31f;
	}
	mEnemyFlags |= ENEMY_AIRBORNE;
	updateShadow();
	return 0;
}

// 0x80160AB0
void TEneGreenBoss::slideMove(f32 rate)
{
	damp(rate);
	if (moveBody(&mBody.mVel)) {
		mBody.mVel.x = 0.0f;
		mBody.mVel.z = 0.0f;
	}
}

// 0x80160AFC
f32 TEneGreenBoss::blendTime(s16 key)
{
	return mSpeed * sGreenBossMotionKeys[key].mRate;
}

// 0x80160B20
void TEneGreenBoss::setScale(f32 scale)
{
	ColliShape* shapes = colliGetShapes(mCollider);

	colliShapeSetRadius(&shapes[0], 12.0f * scale);
	colliShapeSetRadius(&shapes[1], 8.0f * scale);
	colliShapeSetRadius(&shapes[2], 8.0f * scale);
	colliShapeSetRadius(&shapes[3], 7.0f * scale);
	colliShapeSetRadius(&shapes[4], 7.0f * scale);
}

// 0x80160BC0
int TEneGreenBoss::shrink(f32 min, f32 rate)
{
	int done;

	mScale -= rate;
	done = 0;
	if (mScale <= min) {
		mScale = min;
		done  = 1;
	}
	setScale(mScale);
	return done;
}

// 0x80160C18
int TEneGreenBoss::grow(f32 max, f32 rate)
{
	int done;

	mScale += rate;
	done = 0;
	if (mScale >= max) {
		mScale = max;
		done  = 1;
	}
	setScale(mScale);
	return done;
}

// 0x80160C70
void TEneGreenBoss::startDust()
{
	Vec3f at = mBody.mPos;

	if (mFx[0] == 0) {
		mFx[0] = (TObject*)particleGenByIdDefault(&at, 1, 0x45);
	}
	if (mFx[1] == 0) {
		mFx[1] = (TObject*)particleGenByIdDefault(&at, 1, 0x46);
	}
	if (mFx[2] == 0) {
		mFx[2] = (TObject*)particleGenByIdDefault(&at, 1, 0x47);
	}
}

// A kept particle's position
static inline void fxPlace(TObject* fx, f32 x, f32 y, f32 z)
{
	Vec3f* pos = (Vec3f*)((u8*)fx + 0x24);
	pos->x = x;
	pos->y = y;
	pos->z = z;
}

static inline void fxKill(TObject** fx)
{
	if (*fx != 0) {
		(*fx)->mFlags |= 1;
		*fx = 0;
	}
}

// 0x80160D10
static inline void fxPlaceAt(TObject* fx, const Vec3f* at)
{
	Vec3f* pos = (Vec3f*)((u8*)fx + 0x24);
	pos->x = at->x;
	pos->y = at->y;
	pos->z = at->z;
}

void TEneGreenBoss::moveDust()
{
	TObject* fx = mFx[0];
	Vec3f at = mBody.mPos;

	if (fx != 0) {
		fxPlaceAt(fx, &at);
	}
	fx = mFx[1];
	if (fx != 0) {
		fxPlaceAt(fx, &at);
	}
	fx = mFx[2];
	if (fx != 0) {
		fxPlaceAt(fx, &at);
	}
}

// 0x80160DA0
void TEneGreenBoss::stopDust()
{
	fxKill(&mFx[0]);
	fxKill(&mFx[1]);
	fxKill(&mFx[2]);
}

// 0x80160E04
void TEneGreenBoss::startSplash()
{
	Vec3f at = mBody.mPos;

	if (mFx[5] == 0) {
		mFx[5] = (TObject*)particleGenByIdDefault(&at, 1, 0x4C);
	}
	if (mFx[4] == 0) {
		mFx[4] = (TObject*)particleGenByIdDefault(&at, 1, 0x4B);
	}
}

// 0x80160E84
void TEneGreenBoss::moveSplash()
{
	TObject* fx = mFx[5];
	Vec3f at = mBody.mPos;

	if (fx != 0) {
		fxPlaceAt(fx, &at);
	}
	fx = mFx[4];
	if (fx != 0) {
		fxPlaceAt(fx, &at);
	}
}

// 0x80160EF0
void TEneGreenBoss::stopSplash()
{
	fxKill(&mFx[5]);
	fxKill(&mFx[4]);
}

// 0x80160F34
void TEneGreenBoss::startTrail()
{
	if (mFx[6] == 0) {
		mFx[6] = (TObject*)particleGenByIdDefault(&mNodes[5], 1, 0x4F);
	}
}

// 0x80160F7C
void TEneGreenBoss::moveTrail()
{
	if (mFx[6] != 0) {
		fxPlaceAt(mFx[6], &mNodes[5]);
		((TParticleGenerator*)mFx[6])->setRot(0, mBody.mRot.y);
	}
}

// 0x80160FD0
void TEneGreenBoss::stopTrail()
{
	fxKill(&mFx[6]);
}

// 0x80160FF4
void TEneGreenBoss::startSpinFx()
{
	Vec3f at = mBody.mPos;

	if (mFx[7] == 0) {
		mFx[7] = (TObject*)particleGenByIdDefault(&at, 1, 0x48);
	}
	if (mFx[8] == 0) {
		mFx[8] = (TObject*)particleGenByIdDefault(&at, 1, 0x49);
	}
	if (mFx[9] == 0) {
		mFx[9] = (TObject*)particleGenByIdDefault(&at, 1, 0x4A);
	}
}

// 0x80161094
// Out in front of it at the angle
#pragma opt_common_subs off
void TEneGreenBoss::moveSpinFx(const Vec3f* pos, int angle)
{
	f32 x = pos->x;
	f32 y = pos->y;
	f32 z = pos->z;
	TObject* fx;

	x = 130.0f * njSin(angle) + x;
	z = 130.0f * njCos(angle) + z;
	fx = mFx[7];
	if (fx != 0) {
		fxPlace(fx, x, y, z);
	}
	fx = mFx[8];
	if (fx != 0) {
		fxPlace(fx, x, y, z);
	}
	fx = mFx[9];
	if (fx != 0) {
		fxPlace(fx, x, y, z);
	}
}
#pragma opt_common_subs reset

// 0x80161164
void TEneGreenBoss::stopSpinFx()
{
	fxKill(&mFx[7]);
	fxKill(&mFx[8]);
	fxKill(&mFx[9]);
}

// 0x801611C8
// Slot _7C
void TEneGreenBoss::onActionPhase(int action, int phase)
{
	switch (action) {
	case 0:
		actAppear(phase);
		break;
	case 1:
		actIdle(phase);
		break;
	case 2:
		actShrink(phase);
		break;
	case 3:
		actTurnLeft(phase);
		break;
	case 4:
		actTurnRight(phase);
		break;
	case 5:
		actWalk(phase);
		break;
	case 6:
		actRun(phase);
		break;
	case 7:
		actDaze(phase);
		break;
	case 8:
		actGrow(phase);
		break;
	case 9:
		actDown(phase);
		break;
	case 10:
		actSink(phase);
		break;
	case 11:
		actRoar(phase);
		break;
	case 12:
		actSwing(phase);
		break;
	case 13:
		actSlam(phase);
		break;
	case 14:
		actRise(phase);
		break;
	case 15:
		actSpin(phase);
		break;
	case 16:
		actSpinEnd(phase);
		break;
	case 17:
		actStand(phase);
		break;
	case 18:
		actTurn(phase);
		break;
	case 19:
		actHurt(phase);
		break;
	case 20:
		actHurtSink(phase);
		break;
	case 21:
		actDie(phase);
		break;
	}
}

// 0x80161308
// Action 0 it rises out of the pond
void TEneGreenBoss::actAppear(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x15);
		mBody.mPos.x = 0.0f;
		mBody.mPos.y = 25.31f;
		mBody.mPos.z = 0.0f;
		mBody.mRot.y = 0;
		clearFrameKey();
		colliDisable(mCollider);
		mModelFx = 0;
		mActStep = 0;
		mStepFx.stop();
		seStartArg(0x2AE, 0x21C);
		break;
	case 1:
		if (mActStep == 0) {
			mActStep = 1;
			spawnFx(1, 1);
		}
		lockOn();
		if (motionCtrlPassed(216.0f, &mAnim) || motionCtrlPassed(312.0f, &mAnim)) {
			MotionNodePose root;
			NJS_MOTION* node = motionCtrlGetMotion(&mAnim);

			motionGetNodePose(&root, *(NJS_OBJECT**)mPose, node, motionCtrlFrame(&mAnim));
			motionPoseGetPos(&root, &mBody.mPos);
			particleOnceByIdDefault(&mBody.mPos, 1, 0x41);
			mStepFx.start(5, 6);
			mStepFx.mMtx.setPosRot(&mBody.mPos, 0, mBody.mRot.y);
		} else if (motionCtrlPassed(311.0f, &mAnim)) {
			mStepFx.stop();
		}
		if (motionCtrlPassed(361.0f, &mAnim)) {
			mBrain->pathOn();
		}
		motionCtrlUpdate(&mAnim);
		updatePoseRooted();
		waitMotion();
		calcPoints();
		break;
	case 2:
		mBrain->pathOff();
		if (mModelFx != 0) {
			mModelFx->mFlags |= 1;
			mModelFx = 0;
		}
		camSetLookFrom(0, &mBody.mPos);
		unlock();
		colliEnable(mCollider);
		mStepFx.stop();
		break;
	}
}

// 0x80161518
// Action 17
void TEneGreenBoss::actStand(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x80161598
// Action 18
void TEneGreenBoss::actTurn(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 1);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x80161614
// Action 1
void TEneGreenBoss::actIdle(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xE);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x80161690
// Action 2 it shrinks down to 0.4
void TEneGreenBoss::actShrink(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xF);
		clearFrameKey();
		mActStep = 0;
		startSplash();
		seStartAtFar(0x186, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		calcPoints();
		switch (mActStep) {
		case 0:
			if (shrink(0.4f, 0.1f)) {
				mActStep = 1;
			}
			break;
		case 1:
			waitMotion();
			break;
		}
		moveSplash();
		break;
	case 2:
		stopSplash();
		break;
	}
}

// 0x80161780
// Turns the angle toward the target by at most rate times the speed
int TEneGreenBoss::turnStep(int* angle, f32* step, int rate, int target)
{
	int diff = angleDiff(*angle, target);
	f32 left = diff;
	int done;

	if (diff >= 0) {
		*step = rate;
	} else {
		*step = -(f32)rate;
	}
	*step *= mSpeed;
	if (left > 0.0f) {
		done = *step > left;
	} else if (left < 0.0f) {
		done = *step < left;
	} else {
		done = 1;
	}
	if (done) {
		*step = left;
	}
	*angle += (int)*step;
	return done;
}

// 0x80161898
// Action 3
void TEneGreenBoss::actTurnLeft(int phase)
{
	f32* step = &mActTime;

	switch (phase) {
	case 0:
		motionCtrlPlayBlend(blendTime(2), &mAnim, 2);
		clearFrameKey();
		*step = 0.0f;
		break;
	case 1: {
		int angle;

		motionCtrlUpdate(&mAnim);
		updatePose();
		angle = mBody.mRot.y;
		motionCtrlFrame(&mAnim);
		if (turnStep(&angle, step, gGreenBossParams->mTurnRate, mLookAngle)) {
			mHitFlags |= 0x20;
		}
		mBody.mRot.y = angle;
		slideMove(0.9f);
		calcPoints();
		break;
	}
	case 2:
		break;
	}
}

// 0x80161974
// Action 4
void TEneGreenBoss::actTurnRight(int phase)
{
	f32* step = &mActTime;

	switch (phase) {
	case 0:
		motionCtrlPlayBlend(blendTime(3), &mAnim, 3);
		clearFrameKey();
		*step = 0.0f;
		break;
	case 1: {
		int angle;

		motionCtrlUpdate(&mAnim);
		updatePose();
		angle = mBody.mRot.y;
		motionCtrlFrame(&mAnim);
		if (turnStep(&angle, step, gGreenBossParams->mTurnRate2, mLookAngle)) {
			mHitFlags |= 0x20;
		}
		mBody.mRot.y = angle;
		slideMove(0.9f);
		calcPoints();
		break;
	}
	case 2:
		break;
	}
}

// 0x80161A50
// Action 5 it walks to the look target kicking up water at each step
void TEneGreenBoss::actWalk(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(blendTime(2), &mAnim, 2);
		clearFrameKey();
		motionCtrlUpdate(&mAnim);
		updatePose();
		calcPoints();
		mActStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		faceToward(&mLookTarget, &mBody.mPos, (void*)gGreenBossParams->mTurnRate);
		if (motionCtrlFrame(&mAnim) < 10.0f && !motionCtrlIsBlending(&mAnim)) {
			moveByNode(8, &mBody.mVel);
			if (motionCtrlPassed(4.0f, &mAnim)) {
				particleOnceByIdDefault(&mNodes[8], 1, 0x40);
				seStartAtFar(0x176, &mBody.mPos);
			}
		} else {
			moveByNode(10, &mBody.mVel);
			if (motionCtrlPassed(12.0f, &mAnim)) {
				particleOnceByIdDefault(&mNodes[10], 1, 0x40);
				seStartAtFar(0x176, &mBody.mPos);
			}
		}
		moveBody(&mBody.mVel);
		if (probeLine(&mLookTarget, &mBody.mPos, (void*)gGreenBossParams->mTurnRate, &mBody.mVel) ||
		    mActStep >= 0x4B0) {
			mHitFlags |= 0x20;
		}
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x80161BEC
// Action 6 the same at a run with the dust
void TEneGreenBoss::actRun(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(blendTime(3), &mAnim, 3);
		if (mScale >= 1.0f) {
			startDust();
		}
		clearFrameKey();
		motionCtrlUpdate(&mAnim);
		updatePose();
		calcPoints();
		mActStep = 0;
		break;
	case 1: {
		int node;

		motionCtrlUpdate(&mAnim);
		faceToward(&mLookTarget, &mBody.mPos, (void*)gGreenBossParams->mTurnRate);
		if (motionCtrlFrame(&mAnim) < 10.0f && !motionCtrlIsBlending(&mAnim)) {
			node = 8;
			if (motionCtrlPassed(4.0f, &mAnim)) {
				seStartAtFar(0x176, &mBody.mPos);
			}
		} else {
			node = 10;
			if (motionCtrlPassed(12.0f, &mAnim)) {
				seStartAtFar(0x176, &mBody.mPos);
			}
		}
		moveByNode(node, &mBody.mVel);
		moveBody(&mBody.mVel);
		if (probeLine(&mLookTarget, &mBody.mPos, (void*)gGreenBossParams->mTurnRate, &mBody.mVel) ||
		    mActStep >= 0x4B0) {
			mHitFlags |= 0x20;
		}
		moveDust();
		calcPoints();
		break;
	}
	case 2:
		stopDust();
		break;
	}
}

// 0x80161D94
// Whether the daze has gone on longer than this stage's time
int TEneGreenBoss::isDazeOver(int timer)
{
	int stage = thresholdIndex(mHealth, gGreenBossParams->mHealthSteps);
	bool over = timer >= gGreenBossParams->mDazeTime[stage];
	return over;
}

// 0x80161DF0
// Action 7 knocked dizzy
void TEneGreenBoss::actDaze(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 6);
		seStartAtFar(0x17A, &mBody.mPos);
		clearFrameKey();
		mActTimer = 0;
		mActStep  = 0;
		clearRecovered();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		switch (mActStep) {
		case 0:
			if (motionCtrlIsDone(&mAnim)) {
				motionCtrlPlay(&mAnim, 7);
				mActStep = 1;
			}
			break;
		case 1:
			mActTimer++;
			if (isDazeOver(mActTimer)) {
				motionCtrlPlay(&mAnim, 8);
				seStartAtFar(0x17A, &mBody.mPos);
				mActStep = 2;
			}
			break;
		case 2:
			waitMotion();
			break;
		}
		calcPoints();
		break;
	case 2:
		setRecovered();
		break;
	}
}

// 0x80161F30
// Action 8 back up to full size
void TEneGreenBoss::actGrow(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x10);
		clearFrameKey();
		mActStep = 0;
		mStepFx.stop();
		seStartAtFar(0x187, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		calcPoints();
		switch (mActStep) {
		case 0:
			if (grow(1.0f, 0.1f)) {
				mActStep = 1;
			}
			break;
		case 1:
			waitMotion();
			break;
		}
		if (motionCtrlPassed(1.0f, &mAnim)) {
			particleOnceByIdDefault(&mBody.mPos, 1, 0x4D);
			particleOnceByIdDefault(&mBody.mPos, 1, 0x4E);
		} else if (motionCtrlPassed(14.0f, &mAnim)) {
			particleOnceByIdDefault(&mBody.mPos, 1, 0x42);
			mStepFx.start(5, 6);
			mStepFx.mMtx.setPosRot(&mBody.mPos, 0, mBody.mRot.y);
		}
		break;
	case 2:
		mStepFx.stop();
		break;
	}
}

// 0x8016209C
// Action 9 it falls over the first time or again
void TEneGreenBoss::actDown(int phase)
{
	switch (phase) {
	case 0:
		if (mBossFlags & BOSS_FELL) {
			motionCtrlPlay(&mAnim, 0x12);
		} else {
			motionCtrlPlay(&mAnim, 0x11);
		}
		clearFrameKey();
		setGuard();
		mActStep = 0;
		mStepFx.stop();
		beginDeath();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		calcPoints();
		switch (mActStep) {
		case 0:
			if (grow(1.0f, 0.1f)) {
				mActStep = 1;
			}
			break;
		case 1:
			waitMotion();
			break;
		}
		if (mBossFlags & BOSS_FELL) {
			if (motionCtrlPassed(9.0f, &mAnim)) {
				particleOnceByIdDefault(&mBody.mPos, 1, 0x42);
				mStepFx.start(5, 6);
				mStepFx.mMtx.setPosRot(&mBody.mPos, 0, mBody.mRot.y);
				mBrain->splash(40.0f, &mBody.mPos);
			}
		} else {
			if (motionCtrlPassed(11.0f, &mAnim)) {
				particleOnceByIdDefault(&mBody.mPos, 1, 0x42);
				mStepFx.start(5, 6);
				mStepFx.mMtx.setPosRot(&mBody.mPos, 0, mBody.mRot.y);
				mBrain->splash(40.0f, &mBody.mPos);
			}
		}
		break;
	case 2:
		clearGuard();
		setFell();
		mStepFx.stop();
		break;
	}
}

// 0x80162274
// Action 10 it sinks with the swirl
void TEneGreenBoss::actSink(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xC);
		clearFrameKey();
		setGuard();
		if (mFx[3] == 0) {
			mFx[3] = (TObject*)particleGenByIdDefault(&mBody.mPos, 1, 0x43);
		}
		seLoopStart(&mLoop, 0x17D, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		calcPoints();
		seLoopMove(&mLoop, &mBody.mPos);
		// The game's own test is inverted it moves the splash only when there is none writing through null
		if (mFx[3] == 0) {
			fxPlaceAt(mFx[3], &mBody.mPos);
		}
		break;
	case 2:
		seLoopStop(&mLoop);
		clearGuard();
		fxKill(&mFx[3]);
		break;
	}
}

// 0x8016238C
// Action 11
void TEneGreenBoss::actRoar(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xD);
		clearFrameKey();
		setGuard();
		seStartAtFar(0x17E, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		if (motionCtrlPassed(7.0f, &mAnim)) {
			particleOnceByIdDefault(&mBody.mPos, 1, 0x41);
		} else if (motionCtrlPassed(11.0f, &mAnim)) {
			particleOnceByIdDefault(&mBody.mPos, 1, 0x42);
		}
		calcPoints();
		break;
	case 2:
		clearGuard();
		break;
	}
}

// 0x80162478
// Action 12 it spins round once with the sphere out
void TEneGreenBoss::actSwing(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(blendTime(5), &mAnim, 5);
		clearFrameKey();
		mSwingFx.stop();
		mActStep   = 0;
		mBaseAngle = mBody.mRot.y;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slideMove(0.0f);
		syncSwing();
		calcPoints();
		switch (mActStep) {
		case 0:
			if (motionCtrlPassed(10.0f, &mAnim)) {
				mActStep = 1;
			}
			break;
		case 1:
			mBody.mRot.y += 0x666;
			if (motionCtrlPassed(16.0f, &mAnim)) {
				mActStep = 2;
			}
			break;
		case 2:
			break;
		}
		if (!motionCtrlIsBlending(&mAnim)) {
			f32 frame;

			if (motionCtrlPassed(14.0f, &mAnim)) {
				mSwingFx.start(5, 7);
				mSwingFx.mMtx.setPosRot(&mBody.mPos, 0, mBaseAngle);
				seStartAtFar(0x178, &mBody.mPos);
			}
			frame = motionCtrlFrame(&mAnim);
			if (frame >= 14.0f && frame <= 16.0f) {
				colliEnable(mSwingColli);
			} else {
				colliDisable(mSwingColli);
			}
		}
		break;
	case 2:
		colliDisable(mSwingColli);
		mSwingFx.stop();
		break;
	}
}

// 0x8016263C
// Action 13 it slams the ground in front of it
void TEneGreenBoss::actSlam(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(blendTime(4), &mAnim, 4);
		clearFrameKey();
		mSlamFx.stop();
		seStartAtFar(0x177, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slideMove(0.0f);
		syncBox();
		calcPoints();
		if (!motionCtrlIsBlending(&mAnim)) {
			f32 frame;

			if (motionCtrlPassed(13.0f, &mAnim)) {
				mSlamFx.start(5, 3);
				mSlamFx.mMtx.setPosRotScale(&mNodes[6], 0x300, mBody.mRot.y, 3.5f, 3.5f, 5.0f);
				seStartAtFar(0x188, &mNodes[6]);
			}
			frame = motionCtrlFrame(&mAnim);
			if (frame >= 13.0f && frame <= 26.0f) {
				colliEnable(mBoxColli);
			} else {
				colliDisable(mBoxColli);
			}
		}
		break;
	case 2:
		colliDisable(mBoxColli);
		mSlamFx.stop();
		seStartAtFar(0x175, &mBody.mPos);
		break;
	}
}

// 0x801627AC
// Action 14
void TEneGreenBoss::actRise(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 9);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slideMove(0.0f);
		calcPoints();
		break;
	case 2:
		seStartAtFar(0x175, &mBody.mPos);
		break;
	}
}

// 0x8016284C
void TEneGreenBoss::addRotY(int delta)
{
	mBody.mRot.y += delta;
}

// 0x8016285C
// Action 15 it spins with its arm out one way or the other for a while
void TEneGreenBoss::actSpin(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 10);
		clearFrameKey();
		if (2.0f * (3.0517578e-5f * rand() - 0.5f) < 0.0f) {
			mSpinRate = gGreenBossParams->mSpinRate;
		} else {
			mSpinRate = -gGreenBossParams->mSpinRate;
		}
		mActStep   = 0;
		mActLimit  = 0xFFFF / gGreenBossParams->mSpinRate;
		mActTimer  = mActLimit + 30;
		mBaseAngle = mBody.mRot.y;
		startTrail();
		seLoopStart(&mLoop, 0x17F, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		mActTimer--;
		seLoopMove(&mLoop, &mBody.mPos);
		switch (mActStep) {
		case 0:
			addRotY(mSpinRate);
			if (mActTimer <= mActLimit) {
				mActStep = 1;
				startSpinFx();
				mActLimit = 30;
			}
			break;
		case 1:
			addRotY(mSpinRate);
			mBaseAngle += mSpinRate;
			if (mActTimer <= mActLimit) {
				mActStep = 2;
				stopTrail();
			}
			break;
		case 2:
			mBaseAngle += mSpinRate;
			if (mActTimer <= 0) {
				mActStep = 3;
				mHitFlags |= 0x20;
			}
			break;
		case 3:
			break;
		}
		slideMove(0.9f);
		calcPoints();
		moveTrail();
		moveSpinFx(&mBody.mPos, mBaseAngle);
		break;
	case 2:
		stopTrail();
		stopSpinFx();
		seLoopStop(&mLoop);
		break;
	}
}

// 0x80162AA4
// Action 16
void TEneGreenBoss::actSpinEnd(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xB);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slideMove(0.9f);
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x80162B2C
void TEneGreenBoss::beginHurt()
{
	mEnemyFlags |= ENEMY_FLIPPING;
	setGuard();
	beginDeath();
}

// 0x80162B6C
void TEneGreenBoss::endHurt()
{
	mEnemyFlags &= ~ENEMY_FLIPPING;
	clearFrameKey();
	clearGuard();
}

// 0x80162BAC
// Action 19
void TEneGreenBoss::actHurt(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x13);
		beginHurt();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slideMove(0.7f);
		calcPoints();
		break;
	case 2:
		endHurt();
		break;
	}
}

// 0x80162C44
// Action 20 hurt while sunk
void TEneGreenBoss::actHurtSink(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x14);
		beginHurt();
		if (mFx[3] == 0) {
			mFx[3] = (TObject*)particleGenByIdDefault(&mBody.mPos, 1, 0x43);
		}
		seLoopStart(&mLoop, 0x17D, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slideMove(0.7f);
		calcPoints();
		seLoopMove(&mLoop, &mBody.mPos);
		// The game's own test is inverted it moves the splash only when there is none writing through null
		if (mFx[3] == 0) {
			fxPlaceAt(mFx[3], &mBody.mPos);
		}
		break;
	case 2:
		seLoopStop(&mLoop);
		endHurt();
		fxKill(&mFx[3]);
		break;
	}
}

// 0x80162D68
// Action 21 it waits then the death motion plays and the suit comes apart
void TEneGreenBoss::actDie(int phase)
{
	switch (phase) {
	case 0:
		mActStep  = 0;
		mActTimer = 18;
		mTint.start();
		mEnemyFlags |= ENEMY_BLOCKED;
		colliDisable(mCollider);
		colliDisable(mSwingColli);
		colliDisable(mBoxColli);
		break;
	case 1: {
		int frame;

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
			mScale = 1.0f;
			motionCtrlPlay(&mAnim, 0x16);
			mEnemyFlags |= ENEMY_INTANGIBLE | ENEMY_BLOCKED;
			colliDisable(mCollider);
			colliDisable(mSwingColli);
			colliDisable(mBoxColli);
			mFrameKey = 3;
			shadowDisable(&mShadow);
			mActStep = 1;
			mActTime = 0.0f;
			spawnFx(0, 0);
			bossSuitSetFade(240.0f, 320.0f, &mSuit);
			bossSuitSetSink(270.0f, 380.0f, 25.0f, -15.0f, 5.0f, &mSuit);
			bossSuitSetFx(&mSuit, 1, 0x51, 0x50);
			seStartArg(0x2AF, 0x1A9);
			seCueSet(&mSe, 0x69, 0xA5);
		}
		motionCtrlUpdate(&mAnim);
		updatePoseRooted();
		lockOn();
		calcPoints();
		switch (mActStep) {
		case 1:
			if (motionCtrlPassed(45.0f, &mAnim)) {
				particleOnceByIdDefault(&mNodes[8], 1, 0x40);
			} else if (motionCtrlPassed(85.0f, &mAnim)) {
				particleOnceByIdDefault(&mNodes[10], 1, 0x40);
			} else if (motionCtrlPassed(150.0f, &mAnim)) {
				particleOnceByIdDefault(&mBody.mPos, 1, 0x42);
				mBrain->splash(120.0f, &mBody.mPos);
			}
			if (motionCtrlIsDone(&mAnim)) {
				mActStep = 2;
			}
			mActTime = motionCtrlFrame(&mAnim);
			break;
		case 2:
			mActTime += 1.0f;
			break;
		}
		Vec3f at = mBody.mPos;
		at.y = 0.0f;
		at.z += 20.0f;
		if (bossSuitUpdate(mActTime, &mSuit, &at, sGreenBossModels[0])) {
			mBossFlags |= BOSS_ALPHA;
		} else {
			mBossFlags &= ~BOSS_ALPHA;
		}
		if (mActTime >= 420.0f) {
			slain();
			sendBossDefeated(this);
			mFlags |= 1;
		}
		frame = (int)mActTime;
		seCueUpdate(&mSe, frame);
		break;
	}
	case 2:
		mEnemyFlags &= ~(ENEMY_INTANGIBLE | ENEMY_BLOCKED);
		if (mModelFx != 0) {
			mModelFx->mFlags |= 1;
			mModelFx = 0;
		}
		unlock();
		break;
	}
}

// 0x801630E4
// Slot _3C
void TEneGreenBoss::vfn_3C()
{
	njPrint((kEnemyInfoX << 16) | kEnemyInfoLine, ":HP %4d", mHealth);
}

// 0x80163120
// Slot _10
void TEneGreenBoss::vfn_10()
{
	if (isAppearing()) {
		mFlags |= 0x10;
	} else {
		if (!mLight.isVisible(mDrawTask, 3000.0f, &mBody.mPos, 60.0f, 0)) {
			mFlags |= 0x10;
		} else {
			mFlags &= ~0x10;
			if (mBossFlags & BOSS_ALPHA) {
				moveDrawTask(mDrawTask, 0x22);
			}
		}
	}
}

// 0x801631C8
void TEneGreenBoss::render()
{
	if (mBossFlags & BOSS_ALPHA) {
		texFxSuitLoadTex(2);
	}
	njSetTexture(*sGreenBossTextures);
	enemyEyeSet((NJS_TEXLIST*)*sGreenBossTextures, 0, mFrameKey);
	if (mBossFlags & BOSS_ALPHA) {
		gjSetBlendOverride(1, 4, 5, 3);
		_nj_control_3d_flag_ |= 0x8000;
		texFxSuitBegin(0, *(u32*)&mSuit);
	}
	njPushMatrixEx();
	bodyMatrix();
	if (mBossFlags & BOSS_ALPHA) {
		texFxSuitScroll(5.0f + mSuit.mHeight, mSuit.mHeight, 1);
		shadowAlphaBegin();
		gjDrawActor(mPose);
		shadowAlphaEnd();
	} else {
		drawLit(&mLight, &mTint, mPose);
	}
	njPopMatrixEx();
	enemyEyeRestore();
	shadowSetAngle(&mShadow, mBody.mRot.y);
}
