#include "io/ResBank.h"
struct DrawTask;
#include "enemy/boss/EneBoss.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyAppear.h"
#include "enemy/EnemySight.h"
#include "enemy/EnemyLight.h"
#include "engine/MotionPose.h"
#include "enemy/boss/BossParts.h"
#include "egg/ObjEgg.h"
#include "object/ObjCommon.h"
#include "enemy/TEneAm01Parts.h"
#include "engine/FxHolder.h"
#include "sound/SeHolder.h"
#include "enemy/boss/TEneLastBossDarkCtrl.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "enemy/EnemyHit.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "camera/TCamPath.h"
#include "engine/FxSystem.h"
#include "enemy/ObjEnemy.h"
#include "object/ObjShadow.h"
#include "enemy/EnemyFx.h"
#include "camera/TCameraCtrl.h"
#include "enemy/EnemyTexFx.h"
#include "core/dolphin.h"
#include "enemy/boss/LastBossRes.h"
#include "enemy/boss/TEneLastBoss.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "enemy/boss/TEneLastBossFeather_Home.h"
#include "object/last/TObjLastBossInd.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "engine/TParticle.h"
#include "enemy/boss/TEneLastBossDemo_Darkness.h"
#include "core/TMainTask.h"
#include "enemy/TEneAm01.h"
struct GjActor;
struct Vec3f;
struct ColliContact;
struct CharaColli;
struct ColliShape;
struct Fx;

extern "C" {
MotionEntry sLastBossMotions[] = {
	{ MOTION_LOOP, 18, -1, 0, 0.0f, 30.0f, 0.2f, 0.0f, 0 },  { MOTION_ONCE, 4, -1, 0, 0.0f, 24.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 30.0f, 0.25f, 0.0f, 0 },  { MOTION_LOOP, 5, -1, 0, 0.0f, 25.0f, 0.25f, 0.0f, 0 },
	{ MOTION_ONCE, 10, -1, 0, 0.0f, 34.0f, 0.25f, 0.0f, 0 }, { MOTION_ONCE, 9, -1, 0, 0.0f, 44.0f, 0.25f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 27.0f, 0.25f, 0.0f, 0 },  { MOTION_LOOP, 7, -1, 0, 0.0f, 30.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 8, -1, 0, 0.0f, 16.0f, 0.35f, 0.0f, 0 },  { MOTION_ONCE, 12, -1, 0, 0.0f, 11.0f, 0.15f, 0.0f, 0 },
	{ MOTION_LOOP, 13, -1, 0, 0.0f, 10.0f, 0.4f, 0.0f, 0 },  { MOTION_ONCE, 14, -1, 0, 0.0f, 52.0f, 0.25f, 0.0f, 0 },
	{ MOTION_ONCE, 15, -1, 0, 0.0f, 34.0f, 0.25f, 0.0f, 0 }, { MOTION_ONCE, 16, -1, 0, 0.0f, 34.0f, 0.25f, 0.0f, 0 },
	{ MOTION_LOOP, 17, -1, 0, 0.0f, 20.0f, 0.3f, 0.0f, 0 },  { MOTION_ONCE, 0, -1, 0, 0.0f, 19.0f, 0.2f, 0.0f, 0 },
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 49.0f, 0.17f, 0.0f, 0 }, { MOTION_ONCE, 2, -1, 0, 0.0f, 959.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 1199.0f, 1.0f, 0.0f, 0 },
};
extern const int sLastBossNodes[]               = { 1, 11, 17, 18, 19, 20, 21, 22, 23, 33, 69, 70, 73, 111, 176, 191, -1 };
extern const int sLastBossMatrixNodes[]         = { 17, 18, 19, 20, 21, 22, 23, -1 };
extern const int sLastBossFeatherNodes[]        = { 2, 1, 0, 6, 3, 4, 5 };
const char* gTEneLastBossName                   = "TEneLastBoss";
extern const EnemyFrameKey sLastBossFrameKey0[] = { { 18.0f, 0 }, { 28.0f, 5 }, { 41.0f, 0 }, { 47.0f, 3 }, { -1.0f, 0 } };
}

class TEneLastBoss;

typedef char TEneLastBossDemo_Darkness_size_check[(sizeof(TEneLastBossDemo_Darkness) == 0x68) ? 1 : -1];

typedef char LastBossAi_size_check[(sizeof(LastBossAi) == 0x60) ? 1 : -1];

typedef char TEneLastBoss_size_check[(sizeof(TEneLastBoss) == 0x5D0) ? 1 : -1];

// 0x80156484
extern "C" void initTEneLastBoss()
{
	resBankLoad(&sLastBossResBank, "ar_ene_last_boss.arc");
	resBankAddTextures(&sLastBossResBank, 0, "ene_last_boss.gvm");
	sLastBossModelList = resBankGetModels(&sLastBossResBank);
	sLastBossTextures  = resBankGetTextures(&sLastBossResBank);
	sLastBossMotion    = resBankGetMotion(&sLastBossResBank, 0);
	sLastBossParams    = (int*)enemyParamGet(7);
	fxBankLoadDefault(&sLastBossFx, "pa_lastboss.pad");
	lastBossFeatherInit();
	initLastBossDark();
}

// 0x80156528
extern "C" void shutdownTEneLastBoss()
{
	shutdownLastBossDark();
	lastBossFeatherTerm();
	fxBankRelease(&sLastBossFx);
	sLastBossParams = 0;
	resBankRemoveTextures(&sLastBossResBank, 0);
	resBankRelease(&sLastBossResBank);
	sLastBossModelList = 0;
	sLastBossTextures  = 0;
	sLastBossMotion    = 0;
}

// 0x80156588
extern "C" void setupLastBossDark()
{
	Vec3f pos;

	createLastBossDark();
	pos.x = 0.0f;
	pos.y = 20.0f;
	pos.z = 0.0f;
	darkCtrlSetPos(&pos);
	darkCtrlSetFx(5, 0x2F, 0x2A);
	darkCtrlSetSe(0x194);
}

// 0x801565DC
extern "C" TObject* createTEneLastBoss(TObject* parent, SetDesc* desc)
{
	return new TEneLastBoss(parent, desc);
}

// 0x80156630
// Model-table word for an index below 3
extern "C" NJS_OBJECT* modelTEneLastBoss(int index)
{
	if (index < 0 || index >= 3) {
		return 0;
	}
	return sLastBossModelList[index];
}

// 0x80156658
extern "C" NJS_TEXLIST* textureTEneLastBoss()
{
	return *sLastBossTextures;
}

// 0x80156664
extern "C" NJS_MOTION** motionTEneLastBoss(int index)
{
	return resBankGetMotion(&sLastBossResBank, index);
}

// 0x80156690
extern "C" int featherDamageTEneLastBoss()
{
	return sLastBossParams[22];
}

// 0x8015669C
extern "C" void angleOfTEneLastBoss(const Vec3f* pos, int* angle)
{
	*angle = (int)(10430.38043493439 * (f32)atan2(pos->x, pos->z));
}

// 0x801566EC
// The flat distance from the centre and the angle around it
extern "C" void polarOfTEneLastBoss(const Vec3f* pos, f32* radius, int* angle)
{
	f32 x = pos->x;
	f32 z = pos->z;
	f32 a = atan2(x, z);
	f32 r;

	x *= x;
	z *= z;
	z += x;
	r       = (f32)__frsqrte(z);
	*angle  = (int)(10430.38043493439 * a);
	*radius = __fres(r);
}

// 0x80156784
extern "C" void fromPolarTEneLastBoss(f32 radius, Vec3f* out, int angle)
{
	vecFromAngleXZ(radius, out, angle);
	out->y = 20.0f;
}

// 0x801567B8
extern "C" void randomOnRingTEneLastBoss(Vec3f* out, f32 radius)
{
	fromPolarTEneLastBoss(radius, out, (int)(65536.0f * (3.0517578E-5f * (f32)rand())));
}

// 0x8015683C
TEneLastBoss::TEneLastBoss(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	setupLastBossDark();
	readEnemyDesc(desc);
	setupParts();
	initBossBar();
	makeAi();
	setBossView(10.0f, 33.0f, 150.0f);
}

// 0x80156918
void TEneLastBoss::setupParts()
{
	int i;

	_00   = (u32)gTEneLastBossName;
	mSize = 0x5D0;
	motionCtrlSetPlayer(&mAnim, &mMotionData);
	motionCtrlInit(&mAnim, sLastBossMotions, sLastBossMotion);
	motionCtrlPlay(&mAnim, 0);
	mBody.mRot.x = 0;
	mCollider    = (CharaColli*)colliCreate(1, 1);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0xFF, 0, 0x10);
	colliShapeMakeCylinder(colliGetShapes(mCollider), &mBody.mPos, 1, 10.0f, 30.0f);
	colliShapeSetTag(mCollider, 0, 0);
	mAi          = 0;
	mPose        = (GjActor*)allocPose(sLastBossModelList[0]);
	mFeatherPose = allocPose(sLastBossModelList[1]);
	mDrawTask    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mBossFlags   = 0;
	mHitDir.x    = 0.0f;
	mHitDir.y    = 0.0f;
	mHitDir.z    = 0.0f;
	mFrameKey    = 0;
	for (i = 0; i < 16; i++) {
		mPoint[i].x = 0.0f;
		mPoint[i].y = 0.0f;
		mPoint[i].z = 0.0f;
	}
	for (i = 0; i < 7; i++) {
		mtxIdentity(mFeatherMtx[i]);
	}
	mMoveTo.x  = 0.0f;
	mMoveTo.y  = 0.0f;
	mMoveTo.z  = 0.0f;
	mAim.x     = 0.0f;
	mAim.y     = 0.0f;
	mAim.z     = 0.0f;
	mAimRot    = 0;
	mFrom      = mBody.mPos;
	mMaxHealth = 500;
	mHealth    = 500;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 20.0f;
	mShadow._34 = 20.0f;
	shadowEnable(&mShadow);
	mLight.reset();
	mTint.reset();
	mRadiusVel = 0.0f;
	mAngleVel  = 0;
	showFeathers();
	mDarkCount = 0;
	for (i = 0; i < 10; i++) {
		mDark[i] = 0;
	}
	mHomeCount = 0;
	for (i = 0; i < 4; i++) {
		mHome[i] = 0;
	}
	mFx = 0;
	fxHolderStop(&mGlow);
	fxHolderStop(&mGlow2);
	mTail.stop();
	mDemo = 0;
	seCueClear(&mSe);
	mColorMode = 0;
	mDarkness  = 0;
	mStep      = 0;
	mWait      = 0;
	_5C4       = 0;
	_5C8       = 0;
	_5CC       = 0.0f;
}

// 0x80156BC0
TEneLastBoss::~TEneLastBoss()
{
	if (mDarkness != 0) {
		mDarkness->mFlags |= 1;
		mDarkness = 0;
	}
	for (int i = 0; i < 1; i++) {
		if ((&mFx)[i] != 0) {
			(&mFx)[i]->mFlags |= 1;
			(&mFx)[i] = 0;
		}
	}
	if (mDemo != 0) {
		mDemo->mFlags |= 1;
		mDemo = 0;
	}
	removeDarks();
	removeHomes();
	fxRemoveOwner(this);
	mTail.stop();
	fxHolderStop(&mGlow2);
	fxHolderStop(&mGlow);
	unlock();
	if (mAi != 0) {
		delete mAi;
		mAi = 0;
	}
	removeDrawTask(mDrawTask);
	freePose((void**)&mFeatherPose);
	freePose((void**)&mPose);
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x80156D68
void TEneLastBoss::removeDarks()
{
	int i;

	for (i = 9; i >= 0; i--) {
		if (mDark[i] != 0) {
			mDark[i]->mFlags |= 1;
			mDark[i] = 0;
		}
	}
}

// 0x80156DA4
void TEneLastBoss::removeHomes()
{
	int i;

	for (i = 3; i >= 0; i--) {
		if (mHome[i] != 0) {
			mHome[i]->mFlags |= 1;
			mHome[i] = 0;
		}
	}
}

// 0x80156DE0
void TEneLastBoss::setPos(const Vec3f* pos)
{
	f32 x        = pos->x;
	f32 y        = pos->y;
	mBody.mPos.x = x;
	f32 z        = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x80156E24
f32 TEneLastBoss::vfn_40()
{
	return 95.0f;
}

// 0x80156E2C
void TEneLastBoss::syncCollider()
{
	f32 mid;
	f32 half;
	Vec3f* shape;

	mid = mPoint[14].y;
	mid += mPoint[15].y;
	mid *= 0.5f;
	half = mPoint[12].y - mid;
	half *= 0.5f;
	shape    = (Vec3f*)colliGetShapes(mCollider);
	*shape   = mPoint[0];
	shape->y = mid + half;
	colliShapeSetCylHeight((ColliShape*)shape, half);
	colliRefreshBounds(mCollider);
	colliBoundsOn(mCollider);
}

// 0x80156ED4
void TEneLastBoss::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
}

// 0x80156EF4
int TEneLastBoss::onHit(const TObjAction* other, int arg, int* result)
{
	return 0;
}

// 0x80156EFC
void TEneLastBoss::setMoveTo(const Vec3f* pos)
{
	mMoveTo = *pos;
}

// 0x80156F18
void TEneLastBoss::setAim(const Vec3f* pos, int rotY)
{
	mAim    = *pos;
	mAimRot = rotY;
}

// 0x80156F38
void TEneLastBoss::setMaxHealth(int health)
{
	mMaxHealth = health;
	mHealth    = health;
}

// 0x80156F44
void TEneLastBoss::clearOpen()
{
	mBossFlags &= ~BOSS_OPEN;
}

// 0x80156F54
void TEneLastBoss::setOpen()
{
	mBossFlags |= BOSS_OPEN;
}

// 0x80156F64
void TEneLastBoss::setGuard()
{
	mBossFlags |= BOSS_GUARD;
}

// 0x80156F74
void TEneLastBoss::clearGuard()
{
	mBossFlags &= ~BOSS_GUARD;
}

// 0x80156F84
void TEneLastBoss::setTouchable()
{
	colliShapeSetBit8(mCollider, 0);
	mBossFlags &= ~BOSS_NOTOUCH;
}

// 0x80156FC4
void TEneLastBoss::setUntouchable()
{
	colliShapeClearBit8(mCollider, 0);
	mBossFlags |= BOSS_NOTOUCH;
}

// 0x80157004
void TEneLastBoss::onHardHit()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
	hideBossBar();
	seStartAtFar(0x197, &mBody.mPos);
}

// 0x80157054
void TEneLastBoss::onLightHit()
{
	Vec3f at;
	f32 y;
	f32 angle;

	at = mContact.mSurface;
	y  = at.y;
	at.x -= mContact.mCenter.x;
	at.y = 0.0f;
	at.z -= mContact.mCenter.z;
	if (njUnitVector(&at) < gUnitVectorMin) {
		at = gVecZero;
	}
	angle = atan2(at.x, at.z);
	gjScaleVector(12.0f, &at, &at);
	at.x += mContact.mCenter.x;
	at.y = y;
	at.z += mContact.mCenter.z;
	particleOnceByIdRotDefault(&at, 1, 0x42, 0, (int)(10430.38043493439 * angle), 0.0f);
	seStartAtFar(0x189, &mBody.mPos);
	mBossFlags |= BOSS_SPARK;
}

// 0x8015718C
int TEneLastBoss::hasArrived()
{
	return mBossFlags & BOSS_ARRIVED;
}

// 0x80157198
int TEneLastBoss::isDone0()
{
	return isActionDone(0);
}

// 0x801571BC
void TEneLastBoss::setDarkCount(int count)
{
	mDarkCount = count < 0 ? 0 : (count > 10 ? 10 : count);
}

// 0x801571E4
int TEneLastBoss::isDone3()
{
	return isActionDone(3);
}

// 0x80157208
int TEneLastBoss::isDone4()
{
	return isActionDone(4);
}

// 0x8015722C
int TEneLastBoss::hasLanded()
{
	return mBossFlags & BOSS_LANDED;
}

// 0x80157238
int TEneLastBoss::isDone5()
{
	return isActionDone(5);
}

// 0x8015725C
void TEneLastBoss::setHomeCount(int count)
{
	mHomeCount = count < 0 ? 0 : (count > 4 ? 4 : count);
}

// 0x80157284
int TEneLastBoss::isDone6()
{
	return isActionDone(6);
}

// 0x801572A8
int TEneLastBoss::isDone7()
{
	return isActionDone(7);
}

// 0x801572CC
int TEneLastBoss::isDone8()
{
	return isActionDone(8);
}

// 0x801572F0
int TEneLastBoss::isDone9()
{
	return isActionDone(9);
}

// 0x80157314
int TEneLastBoss::isDone11()
{
	return isActionDone(11);
}

// 0x80157338
int TEneLastBoss::isDone12()
{
	return isActionDone(12);
}

// 0x8015735C
void TEneLastBoss::updateBody()
{
	int i;

	if (!isBlocked() && takeHits(&mHit, &mContact)) {
		enemyHitPushAway((Vec3f*)&mHit, &mBody.mPos, (const Vec3f*)&mContact, &mHitDir);
		applyDamage(&mHit);
	}
	mAi->tick(this);
	clearHitFlags();
	onActionPhase((s16)mAction, 1);
	mTint.update();
	syncCollider();
	if (mBossFlags & BOSS_OPEN) {
		fxHolderStop(&mGlow2);
	} else {
		mGlow2.start(5, 0xC, &mPoint[1], -0x4000, 0);
		if (mBossFlags & BOSS_SPARK) {
			Fx* fx = (Fx*)fxSpawnFollow(1, 5, 0xD, (f32*)&mGlow2);
			if (fx != 0) {
				fxSetOwner(fx, (void*)this);
			}
		}
	}
	mBossFlags &= ~BOSS_SPARK;
	for (i = 0; i < 7; i++) {
		f32 size = mFeatherSize[i];

		size += 0.03f;
		if (size > 1.0f) {
			size = 1.0f;
		}
		mFeatherSize[i] = size;
	}
	setBossBar(mMaxHealth, mHealth);
}

// 0x801574E0
void TEneLastBoss::vfn_6C()
{
	if (returnZero()) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginNodes();
		gjPushUnitMatrix();
		poseFirstFrame(sLastBossMotion, 0x14, mPose);
		njPopMatrixEx();
		endNodes();
		calcPoints();
		syncCollider();
	}
}

// 0x80157564
void TEneLastBoss::makeAi()
{
	mAi = createLastBossAi(this);
}

// 0x80157594
// Called at each node of the pose
extern "C" void saveNodeMatrixTEneLastBoss()
{
	int node = nodeTrackIndex();

	nodeTrackOrigins();
	while (node == *sLastBossNodeList) {
		njGetMatrix(sLastBossMtxOut);
		sLastBossMtxOut += 12;
		sLastBossNodeList++;
	}
	nodeTrackNext();
}

// 0x801575FC
void TEneLastBoss::beginNodes()
{
	nodeTrackSet(sLastBossNodes, mPoint);
	sLastBossNodeList = sLastBossMatrixNodes;
	sLastBossMtxOut   = mFeatherMtx[0];
	nodeTrackSetCallback(saveNodeMatrixTEneLastBoss);
}

// 0x80157654
void TEneLastBoss::endNodes()
{
	nodeTrackEnd();
}

// 0x80157674
void TEneLastBoss::updatePose()
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

// 0x801576E0
void TEneLastBoss::updatePoseRooted()
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

// 0x80157798
void TEneLastBoss::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 16; i++) {
		njCalcPointEx(&mPoint[i], &mPoint[i]);
	}
	njPopMatrixEx();
	for (i = 0; i < 7; i++) {
		mFeatherMtx[i][3]  = 0.0f;
		mFeatherMtx[i][7]  = 0.0f;
		mFeatherMtx[i][11] = 0.0f;
	}
}

// 0x80157820
int TEneLastBoss::getHitKind(const void* touch, HitInfo* info)
{
	if (colliContactInGroup2((ColliContact*)touch)) {
		if (mBossFlags & BOSS_GUARD) {
			return 2;
		}
	} else if (mBossFlags & BOSS_OPEN) {
		return 3;
	}
	return 1;
}

// 0x80157884
int TEneLastBoss::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;

	if (!(mBossFlags & BOSS_OPEN)) {
		damage = 0;
	} else if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x801578B0
void TEneLastBoss::bodyMatrix()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
	gjRotateX(mBody.mRot.x);
}

// 0x801578F0
void TEneLastBoss::updateShadow()
{
	Vec3f from;
	Vec3f ground;
	Vec3f normal;

	from.x = mBody.mPos.x;
	from.y = mBody.mPos.y;
	from.z = mBody.mPos.z;
	from.y += 8.0f;
	if (probeGround(&from, &ground, &normal)) {
		shadowSetGround(&mShadow, &ground, &normal);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
}

// 0x80157978
// Moves around the centre toward mMoveTo
int TEneLastBoss::moveOrbit()
{
	int angle;
	int toAngle;
	f32 radius;
	f32 toRadius;
	int diff;
	Vec3f at;

	polarOfTEneLastBoss(&mBody.mPos, &radius, &angle);
	polarOfTEneLastBoss(&mMoveTo, &toRadius, &toAngle);
	diff      = angleDiff(angle, toAngle);
	mAngleVel = (int)approachSpeed((f32)mAngleVel, (f32)diff, 0.1f, 20.0f, 50.0f, 500.0f, 50.0f);
	angle += mAngleVel;
	toRadius -= radius;
	mRadiusVel = approachSpeed(mRadiusVel, toRadius, 0.1f, 0.1f, 0.2f, 10.0f, 0.1f);
	radius += mRadiusVel;
	if ((f32)__fabs((f32)diff) < 100.0f && (f32)__fabs(toRadius) < 1.0f) {
		mBossFlags |= BOSS_ARRIVED;
	} else {
		mBossFlags &= ~BOSS_ARRIVED;
	}
	fromPolarTEneLastBoss(radius, &at, angle);
	njSubVector(&at, &mBody.mPos);
	mBody.mVel.x = at.x;
	mBody.mVel.z = at.z;
	return angle;
}

// 0x80157B10
int TEneLastBoss::slowOrbit()
{
	int angle;
	f32 radius;
	f32 vel;
	Vec3f at;

	polarOfTEneLastBoss(&mBody.mPos, &radius, &angle);
	vel = mAngleVel;
	vel *= 0.8f;
	mAngleVel = (int)vel;
	mRadiusVel *= 0.8f;
	angle += mAngleVel;
	radius += mRadiusVel;
	fromPolarTEneLastBoss(radius, &at, angle);
	njSubVector(&at, &mBody.mPos);
	mBody.mVel.x = at.x;
	mBody.mVel.z = at.z;
	return angle;
}

// 0x80157BD4
int TEneLastBoss::moveBy(const Vec3f* step)
{
	Vec3f move;

	move.x = 0.0f;
	move.y = 0.0f;
	move.z = 0.0f;
	getPush(&move);
	move.y = 0.0f;
	njAddVector(&move, step);
	njAddVector(&mBody.mPos, &move);
	if (mBody.mPos.y <= 20.0f) {
		mBody.mPos.y = 20.0f;
	}
	mEnemyFlags |= ENEMY_AIRBORNE;
	updateShadow();
	return 0;
}

// 0x80157C68
extern "C" int featherIndexTEneLastBoss(TEneLastBoss* boss, f32 t)
{
	int index;

	t *= 6.0f;
	t += 0.5f;
	index = (int)(f32)floor(t);
	if (index < 0) {
		return 0;
	}
	return index > 6 ? 6 : index;
}

// 0x80157CC8
extern "C" int featherNodeTEneLastBoss(TEneLastBoss* boss, f32 t)
{
	return sLastBossFeatherNodes[featherIndexTEneLastBoss(boss, t)];
}

// 0x80157CF8
// A point on the arc over the wings from t 0 to 1
void TEneLastBoss::placeOnWing(f32 t, Vec3f* out, int* rotY)
{
	Vec3f v;

	v     = mPoint[featherNodeTEneLastBoss(this, 0.0f) + 2];
	int n = featherNodeTEneLastBoss(this, 1.0f) + 2;
	njSubVector(&v, &mPoint[n]);
	*rotY = (int)(10430.38043493439 * (f32)atan2(v.x, v.z));
	*rotY -= 0x4000;
	v.x = 0.0f;
	v.y = 1.0f;
	v.z = 0.0f;
	gjPushUnitMatrix();
	gjRotateY(*rotY);
	gjRotateZ((int)(24576.0f * (t - 0.5f)));
	njCalcVectorEx(&v, out);
	njPopMatrixEx();
	gjScaleVector(30.0f, out, out);
	njAddVector(out, &mPoint[8]);
}

// 0x80157E30
f32 TEneLastBoss::getFeatherStep(int count)
{
	return __fres((f32)(count - 1));
}

// 0x80157E60
void TEneLastBoss::hideFeather(int index)
{
	mFeatherSize[index] = -3.0f;
}

// 0x80157E74
void TEneLastBoss::hideFeathers()
{
	int i;

	for (i = 0; i < 7; i++) {
		hideFeather(i);
	}
}

// 0x80157EB8
void TEneLastBoss::showFeathers()
{
	int i;

	for (i = 0; i < 7; i++) {
		mFeatherSize[i] = 1.0f;
	}
}

// 0x80157EDC
void TEneLastBoss::playDemo(int model, int motion)
{
	void* actor        = resBankGetObjects(&sLastBossResBank)[model];
	NJS_MOTION** table = resBankGetMotion(&sLastBossResBank, 1);

	mDemo = createCamPath(0, 3, actor, table[motion], 0, 1.0f, 1.0f);
	if (mDemo != 0) {
		((TCamera*)mDemo)->setCut();
	}
}

// 0x80157F6C
void TEneLastBoss::onActionPhase(int action, int phase)
{
	switch (action) {
	case 0:
		actAppear(phase);
		break;
	case 1:
		actIdle(phase);
		break;
	case 2:
		actMove(phase);
		break;
	case 3:
		actDarks(phase);
		break;
	case 4:
		actBurst(phase);
		break;
	case 5:
		actSpin(phase);
		break;
	case 6:
		actHome(phase);
		break;
	case 7:
		actHomeEnd(phase);
		break;
	case 8:
		actStunA(phase);
		break;
	case 9:
		actStunB(phase);
		break;
	case 10:
		actDown(phase);
		break;
	case 11:
		actUp(phase);
		break;
	case 12:
		actFlinch(phase);
		break;
	case 13:
		actDie(phase);
		break;
	}
}

// 0x8015804C
void TEneLastBoss::actAppear(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x11);
		mBody.mPos.x = 0.0f;
		mBody.mPos.y = 20.0f;
		mBody.mPos.z = 0.0f;
		mBody.mRot.y = 0;
		mDemo        = 0;
		colliDisable(mCollider);
		setUntouchable();
		setOpen();
		setCantAct();
		fxHolderStop(&mGlow);
		mTail.stop();
		mStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePoseRooted();
		waitMotion();
		lockOn();
		if (mStep == 0) {
			mStep = 1;
			playDemo(1, 1);
			seStartArg(0x2B0, 0x3C0);
		} else {
			Vec3f camera;
			Vec3f dir;
			int a;
			int b;

			if (motionCtrlPassed(476.0f, &mAnim)) {
				mTail.start(5, 0x18);
			}
			if (motionCtrlPassed(880.0f, &mAnim)) {
				clearOpen();
				mBossFlags |= BOSS_SPARK;
			}
			camGetPos(0, &camera);
			camGetFlat(0, &dir);
			gjScaleVector(-1.0f, &dir, &dir);
			vectorToAngles(&dir, &a, &b);
			gjMulAddVector(-40.0f, &camera, &camera, &dir);
			mTail.mMtx.setPosRotScale(&camera, a, b, 0.09f, 0.09f, 1.0f);
		}
		calcPoints();
		updateShadow();
		break;
	case 2:
		mBody.mRot.x = 0;
		colliEnable(mCollider);
		setTouchable();
		clearOpen();
		clearCantAct();
		fxHolderStop(&mGlow);
		mTail.stop();
		if (mDemo != 0) {
			mDemo->mFlags |= 1;
			mDemo = 0;
		}
		camSetLookFrom(0, &mBody.mPos);
		unlock();
		break;
	}
}

// 0x8015828C
void TEneLastBoss::actIdle(int phase)
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

// 0x8015830C
void TEneLastBoss::actMove(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 2);
		clearFrameKey();
		seStartAtFar(0x18B, &mBody.mPos);
		break;
	case 1: {
		int angle;

		motionCtrlUpdate(&mAnim);
		updatePose();
		angle = moveOrbit();
		moveBy(&mBody.mVel);
		turnTo(angle + 0x8000, 0x200);
		calcPoints();
		break;
	}
	case 2:
		break;
	}
}

// 0x801583BC
void TEneLastBoss::launchDark(int index, const Vec3f* pos)
{
	if (mDark[index] != 0) {
		int sector = 0x10000 / mDarkCount;
		int angle  = sector * (index - 1);
		Vec3f at;
		Vec3f dir;

		angle += (int)((f32)sector * (3.0517578E-5f * (f32)rand() - 0.5f));
		fromPolarTEneLastBoss(20.0f * (3.0517578E-5f * (f32)rand()) + 100.0f, &at, angle);
		dir = *pos;
		njSubVector(&dir, &mPoint[8]);
		if (njUnitVector(&dir) < gUnitVectorMin) {
			dir = gVecZero;
		}
		mDark[index]->launch(&dir, &at, 100.0f);
		mDark[index] = 0;
	}
}

// 0x80158514
void TEneLastBoss::actDarks(int phase)
{
	switch (phase) {
	case 0: {
		f32 step;
		int i;

		motionCtrlPlayBlend(0.1f, &mAnim, 5);
		mColorMode = 1;
		clearFrameKey();
		mStep = 0;
		removeDarks();
		step = getFeatherStep(mDarkCount);
		for (i = 0; i < mDarkCount; i++) {
			f32 t    = (f32)i * step;
			int node = featherNodeTEneLastBoss(this, t);
			mDark[i] = new TEneLastBossFeather_Dark(this, &mPoint[node + 2], mBody.mRot.y, mFeatherMtx[node]);
			hideFeather(featherIndexTEneLastBoss(this, t));
		}
		seStartAtFar(0x18C, &mBody.mPos);
		break;
	}
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slowOrbit();
		moveBy(&mBody.mVel);
		turnTo(mAimRot, 0x200);
		calcPoints();
		if (mStep == 0) {
			int launch;
			f32 step;
			int i;

			if (motionCtrlPassed(15.0f, &mAnim)) {
				mStep  = 1;
				launch = 1;
				particleOnceByIdDefault(&mPoint[1], 1, 0x47);
				seStartAtFar(0x18D, &mBody.mPos);
			} else {
				launch = 0;
			}
			step = getFeatherStep(mDarkCount);
			for (i = 0; i < mDarkCount; i++) {
				if (mDark[i] != 0) {
					Vec3f pos;
					int rotY;

					placeOnWing((f32)i * step, &pos, &rotY);
					if (launch) {
						launchDark(i, &pos);
					} else {
						mDark[i]->setAim(&pos, rotY);
					}
				}
			}
		}
		break;
	case 2:
		removeDarks();
		mColorMode = 0;
		break;
	}
}

// 0x801587DC
void TEneLastBoss::actBurst(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 9);
		mColorMode = 2;
		clearFrameKey();
		mStep = 0;
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		fxHolderStop(&mGlow);
		break;
	case 1: {
		int angle;

		motionCtrlUpdate(&mAnim);
		updatePose();
		slowOrbit();
		moveBy(&mBody.mVel);
		calcPoints();
		angleOfTEneLastBoss(&mBody.mPos, &angle);
		angle += 0x8000;
		turnTo(angle, 0x200);
		switch (mStep) {
		case 0:
			if (motionCtrlIsDone(&mAnim)) {
				motionCtrlPlayBlend(0.2f, &mAnim, 0xA);
				mStep = 1;
			}
			break;
		case 1:
			mHitFlags |= HIT_DONE;
			particleKeepAt((void**)&mFx, (const Vec3f*)&mPoint[13], 1, 0x49);
			mGlow.start(5, 8, &mBody.mPos, -0x4000, 0);
			break;
		}
		break;
	}
	case 2:
		mColorMode = 0;
		fxHolderStop(&mGlow);
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		break;
	}
}

// 0x80158984
void TEneLastBoss::actSpin(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0xB);
		mBossFlags &= ~BOSS_LANDED;
		mColorMode = 2;
		clearFrameKey();
		mStep = 0;
		mFrom = mBody.mPos;
		seStartAtFar(0x18E, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		if (mStep == 0) {
			if (motionCtrlIsBlending(&mAnim)) {
				updatePose();
			} else {
				updatePoseRooted();
				njAddVector(&mBody.mPos, &mFrom);
			}
			if (motionCtrlPassed(13.0f, &mAnim)) {
				mBossFlags |= BOSS_LANDED;
				seStartAtFar(0x18F, &mBody.mPos);
			}
			if (motionCtrlIsDone(&mAnim)) {
				motionCtrlPlayBlend(0.2f, &mAnim, 2);
				mStep = 1;
			}
		} else {
			updatePose();
			mBody.mPos = mFrom;
			mHitFlags |= HIT_DONE;
		}
		slowOrbit();
		moveBy(&mBody.mVel);
		calcPoints();
		if (mStep == 0 && motionCtrlPassed(13.0f, &mAnim)) {
			fxSpawnFlat(5, (void*)0x11, &mBody.mPos);
			enemyFxSpawnAtPos(5, (void*)0x15, (const Vec3f*)&mPoint[13]);
			particleOnceByIdDefault(&mBody.mPos, 1, 0x40);
		}
		break;
	case 2:
		mHitFlags &= ~HIT_DONE;
		mColorMode = 0;
		mBody.mPos = mFrom;
		break;
	}
}

// 0x80158B90
void TEneLastBoss::actHome(int phase)
{
	switch (phase) {
	case 0: {
		f32 step;
		int i;

		motionCtrlPlayBlend(0.2f, &mAnim, 6);
		mColorMode = 3;
		clearFrameKey();
		mStep = 0;
		removeHomes();
		fxHolderStop(&mGlow);
		step = getFeatherStep(mHomeCount);
		for (i = 0; i < mHomeCount; i++) {
			f32 t    = (f32)i * step;
			int node = featherNodeTEneLastBoss(this, t);
			mHome[i] = new TEneLastBossFeather_Home(this, &mPoint[node + 2], mBody.mRot.y, mFeatherMtx[node]);
			hideFeather(featherIndexTEneLastBoss(this, t));
		}
		seStartAtFar(0x18C, &mBody.mPos);
		break;
	}
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		slowOrbit();
		moveBy(&mBody.mVel);
		turnTo(mAimRot, 0x200);
		calcPoints();
		switch (mStep) {
		case 0: {
			int launch;
			f32 step;
			TEneLastBossFeather_Home* home;
			int i;

			if (motionCtrlPassed(14.0f, &mAnim)) {
				mStep  = 1;
				launch = 1;
				particleOnceByIdDefault(&mPoint[1], 1, 0x48);
				seStartAtFar(0x18D, &mBody.mPos);
			} else {
				launch = 0;
			}
			step = getFeatherStep(mHomeCount);
			for (i = 0; i < 4; i++) {
				home = mHome[i];

				if (home != 0) {
					Vec3f pos;
					int rotY;

					placeOnWing((f32)i * step, &pos, &rotY);
					home->setAim(&pos, rotY);
					if (launch) {
						njSubVector(&pos, &mPoint[8]);
						if (njUnitVector(&pos) < gUnitVectorMin) {
							pos = gVecZero;
						}
						home->launch(&pos);
						mHome[i] = 0;
					}
				}
			}
			break;
		}
		case 1:
			mGlow.start(5, 0x1C, &mPoint[9]);
			if (motionCtrlIsDone(&mAnim)) {
				motionCtrlPlayBlend(0.2f, &mAnim, 7);
				mStep = 2;
			}
			break;
		case 2:
			mGlow.start(5, 0x1C, &mPoint[9]);
			mHitFlags |= HIT_DONE;
			break;
		}
		break;
	case 2: {
		int i;

		for (i = 0; i < 4; i++) {
			if (mHome[i] != 0) {
				Vec3f zero;

				zero.x = 0.0f;
				zero.y = 0.0f;
				zero.z = 0.0f;
				mHome[i]->launch(&zero);
				mHome[i] = 0;
			}
		}
		fxHolderStop(&mGlow);
		mColorMode = 0;
		break;
	}
	}
}

// 0x80158F60
void TEneLastBoss::actHomeEnd(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 8);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slowOrbit();
		moveBy(&mBody.mVel);
		turnTo(mAimRot, 0x200);
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x80159004
void TEneLastBoss::actStunA(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xC);
		clearFrameKey();
		fxSpawnFlat(5, (void*)0x16, (const Vec3f*)&mPoint[1]);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slowOrbit();
		moveBy(&mBody.mVel);
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x801590A4
void TEneLastBoss::actStunB(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xD);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slowOrbit();
		moveBy(&mBody.mVel);
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x80159134
void TEneLastBoss::actDown(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0xE);
		clearFrameKey();
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		slowOrbit();
		moveBy(&mBody.mVel);
		calcPoints();
		particleKeepAt((void**)&mFx, &mBody.mPos, 1, 0x41);
		break;
	case 2:
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		break;
	}
}

// 0x80159220
void TEneLastBoss::actUp(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0x10);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slowOrbit();
		moveBy(&mBody.mVel);
		calcPoints();
		if (motionCtrlPassed(11.0f, &mAnim)) {
			seStartAtFar(0x18A, &mBody.mPos);
		}
		if (motionCtrlPassed(37.0f, &mAnim)) {
			seStartAtFar(0x18D, &mBody.mPos);
		}
		break;
	case 2:
		break;
	}
}

// 0x801592F4
void TEneLastBoss::actFlinch(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0xF);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		slowOrbit();
		moveBy(&mBody.mVel);
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x80159388
void TEneLastBoss::actDie(int phase)
{
	switch (phase) {
	case 0:
		mStep = 0;
		mWait = 0x12;
		mTint.start();
		mEnemyFlags |= ENEMY_BLOCKED;
		colliDisable(mCollider);
		break;
	case 1: {
		f32 frame;

		if (mStep == 0) {
			mWait--;
			if (mWait >= 0) {
				break;
			}
			mEnemyFlags |= ENEMY_INTANGIBLE | ENEMY_BLOCKED;
			setOpen();
			showFeathers();
			shadowDisable(&mShadow);
			mBody.mPos.x = 0.0f;
			mBody.mPos.y = 0.0f;
			mBody.mPos.z = 0.0f;
			mBody.mRot.x = 0;
			mBody.mRot.y = 0;
			mBody.mRot.z = 0;
			motionCtrlPlay(&mAnim, 0x12);
			colliDisable(mCollider);
			playDemo(0, 0);
			seStartArg(0x2B1, 0x4B0);
			seCueSet(&mSe, 0x69, 0xA5);
			mAppearTimer = 1.0f;
			mStep        = 1;
		}
		motionCtrlUpdate(&mAnim);
		updatePoseRooted();
		lockOn();
		frame = motionCtrlFrame(&mAnim);
		if (motionCtrlPassed(470.0f, &mAnim)) {
			lastBossEggFlash();
		}
		if (motionCtrlPassed(540.0f, &mAnim)) {
			lastBossEggBreak();
			lastBossIndHide();
		}
		if (motionCtrlPassed(670.0f, &mAnim)) {
			mDarkness = new TEneLastBossDemo_Darkness(gTL_TIMESTOP);
		}
		if (frame >= 670.0f) {
			hideFeathers();
		}
		if (frame >= 980.0f && frame < 1200.0f) {
			f32 t = (frame - 980.0f) / 220.0f;

			mAppearTimer = t;
			hatchPose(&mAppearTimer, (GjActor*)mPose, 60.0f);
			mBody.mPos.y += 30.0f * t;
		}
		seCueUpdate(&mSe, (int)frame);
		calcPoints();
		waitMotion();
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
		if (mDarkness != 0) {
			mDarkness->mFlags |= 1;
			mDarkness = 0;
		}
		break;
	}
}

// 0x80159654
void TEneLastBoss::vfn_3C()
{
	njPrint((kEnemyInfoX << 16) | kEnemyInfoLine, ":HP %4d", mHealth);
}

// 0x80159690
void TEneLastBoss::vfn_10()
{
	if (isAppearing()) {
		mFlags |= 0x10;
	} else if (!mLight.isVisible(mDrawTask, 3000.0f, &mBody.mPos, 30.0f, 0)) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
		if (mBossFlags & BOSS_SUIT) {
			moveDrawTask(mDrawTask, 0x22);
		}
	}
}

/** One entry of the boss's texture list */
struct TexEntry {
	u8 _00[8];  // _00
	void* mTex; // _08
};

// 0x80159738
void TEneLastBoss::render()
{
	NJS_TEXLIST* tex;
	void* saved;
	int color;
	int i;

	if (mBossFlags & BOSS_SUIT) {
		texFxSuitLoadTex(2);
	}
	tex = *sLastBossTextures;
	njSetTexture(tex);
	switch (mColorMode) {
	default:
		color = -1;
		break;
	case 1:
		color = 6;
		break;
	case 2:
		color = 5;
		break;
	case 3:
		color = 7;
		break;
	}
	if (color >= 0) {
		TexEntry* entries = *(TexEntry**)tex;

		saved           = entries[1].mTex;
		entries[1].mTex = entries[color].mTex;
	}
	if (mBossFlags & BOSS_SUIT) {
		gjSetBlendOverride(1, 4, 5, 3);
		_nj_control_3d_flag_ |= 0x8000;
		texFxSuitBegin(0, *(u32*)&mSuit);
	}
	njPushMatrixEx();
	bodyMatrix();
	if (mBossFlags & BOSS_SUIT) {
		shadowAlphaBegin();
		gjDrawActor(mPose);
		shadowAlphaEnd();
	} else {
		drawLit(&mLight, &mTint, mPose);
	}
	njPopMatrixEx();
	if (color >= 0) {
		(*(TexEntry**)tex)[1].mTex = saved;
	}
	for (i = 0; i < 7; i++) {
		f32 size = mFeatherSize[i];

		if (!(size <= 0.0f)) {
			int node = sLastBossFeatherNodes[i];

			njPushMatrixEx();
			njTranslateEx(&mPoint[node + 2]);
			gjRotateY(mBody.mRot.y);
			njMultiMatrix(0, mFeatherMtx[node]);
			if (size < 1.0f) {
				njScale(0, size, size, size);
			}
			drawModelLit(mFeatherPose);
			njPopMatrixEx();
		}
	}
	shadowSetAngle(&mShadow, mBody.mRot.y);
}
