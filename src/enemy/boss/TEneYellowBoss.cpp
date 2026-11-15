#include "io/ResBank.h"
struct DrawTask;
#include "enemy/ObjEnemy.h"
#include "engine/DrawTask.h"
#include "camera/TCamera.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyAppear.h"
#include "enemy/EnemyCommon.h"
#include "enemy/EnemySight.h"
#include "enemy/EnemyLight.h"
#include "engine/MotionPose.h"
#include "enemy/boss/BossParts.h"
#include "object/ObjCommon.h"
#include "enemy/TEneAm01Parts.h"
#include "engine/FxHolder.h"
#include "egg/ObjEgg.h"
#include "sound/SeHolder.h"
#include "enemy/boss/TEneYellowBossCrystal.h"
#include "object/ObjAction.h"
#include "enemy/EnemyHit.h"
#include "object/last/TObjLastBossEgg_Frag.h"
#include "enemy/boss/TEneYellowBossGreen.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "core/MathUtil.h"
#include "object/ObjShadow.h"
#include "enemy/EnemyFx.h"
#include "camera/TCameraCtrl.h"
#include "enemy/boss/TEneYellowBossDemo_Gate.h"
#include "object/TObject.h"
#include "enemy/EnemyTexFx.h"
#include "object/TObjLauncher.h"
#include "object/ObjModel.h"
#include "engine/DrawState.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "ui/AeArc.h"
#include "camera/TCamPath.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "engine/TParticle.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "enemy/boss/TEneYellowBossBlue.h"
#include "enemy/boss/TEneYellowBossOrange.h"
#include "enemy/boss/TEneYellowBossPurple.h"
#include "enemy/boss/TEneYellowBossRed.h"
#include "enemy/TEneAm01.h"
struct GjActor;
struct Vec3f;
struct DrawStateSlot;
struct ColliContact;
struct ColliShape;
struct CharaColli;

extern "C" {
MotionEntry sYellowBossMotions[] = {
	{ MOTION_LOOP, 24, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 4, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 14, -1, 0, 0.0f, 49.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 15, -1, 0, 0.0f, 9.0f, 0.45f, 0.0f, 0 },
	{ MOTION_LOOP, 16, -1, 0, 0.0f, 10.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 17, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 21, -1, 0, 0.0f, 9.0f, 0.2f, 0.0f, 0 },
	{ MOTION_LOOP, 22, -1, 0, 0.0f, 20.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 23, -1, 0, 0.0f, 20.0f, 0.35f, 0.0f, 0 },
	{ MOTION_ONCE, 18, -1, 0, 0.0f, 24.0f, 0.5f, 0.0f, 0 },
	{ MOTION_LOOP, 19, -1, 0, 0.0f, 10.0f, 0.6f, 0.0f, 0 },
	{ MOTION_ONCE, 20, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 11, -1, 0, 0.0f, 9.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 12, -1, 0, 0.0f, 10.0f, 0.35f, 0.0f, 0 },
	{ MOTION_ONCE, 6, -1, 0, 0.0f, 9.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 7, -1, 0, 0.0f, 10.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 29.0f, 0.45f, 0.0f, 0 },
	{ MOTION_BLEND, 8, -1, 18, 0.0f, 9.0f, 0.4f, 0.2f, 0 },
	{ MOTION_LOOP, 9, -1, 0, 0.0f, 20.0f, 0.25f, 0.0f, 0 },
	{ MOTION_ONCE, 10, -1, 0, 0.0f, 9.0f, 0.4f, 0.0f, 0 },
	{ MOTION_LOOP, 5, -1, 0, 0.0f, 20.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 1, -1, 0, 0.0f, 9.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 19.0f, 0.3f, 0.0f, 0 },
	{ MOTION_ONCE, 13, -1, 0, 0.0f, 599.0f, 1.0f, 0.0f, 0 },
	{ MOTION_ONCE, 3, -1, 0, 0.0f, 519.0f, 1.0f, 0.0f, 0 },
};
extern const int sYellowBossNodes[] = { 4, 39, 46, 61, 67, -1 };
const char* gTEneYellowBossName = "TEneYellowBoss";
extern const EnemyFrameKey sYellowBossFrameKey0[] = { { 6.0f, 0 }, { 10.0f, 2 }, { -1.0f, 0 } };
extern const EnemyFrameKey sYellowBossFrameKey1[] = { { 5.0f, 2 }, { 18.0f, 0 }, { 22.0f, 4 }, { -1.0f, 0 } };
extern const EnemyFrameKey sYellowBossFrameKey2[] = { { 11.0f, 0 }, { 16.0f, 4 }, { -1.0f, 0 } };
const EnemyFrameKey* sYellowBossFrameKeys[3] = { sYellowBossFrameKey0, sYellowBossFrameKey1, sYellowBossFrameKey2 };
}

class TEneYellowBoss;

// The five parts share these two slots
class YellowBossPart : public TObjAction {
public:
	virtual void vfn_6C();
	virtual int isFinished(); // _70
	virtual void dismiss();   // _74
};

class YellowBossAi;

typedef char TEneYellowBoss_size_check[(sizeof(TEneYellowBoss) == 0x41C) ? 1 : -1];

typedef char YellowBossAi_size_check[(sizeof(YellowBossAi) == 0x98) ? 1 : -1];

// The boss's body sits 50.02 above the point it turns about

// 0x80148E70
extern "C" void yellowBossLower(Vec3f* out, const Vec3f* pos)
{
	*out = *pos;
	out->y -= 50.02f;
}

// 0x80148E9C
extern "C" void yellowBossRaise(Vec3f* out, const Vec3f* pos)
{
	*out = *pos;
	out->y += 50.02f;
}

// 0x80148EC8
// The angle that faces the middle of the arena from pos
extern "C" int yellowBossFaceCenter(const Vec3f* pos)
{
	Vec3f low;
	yellowBossLower(&low, pos);
	return (int)(10430.38043493439 * (f32)atan2(low.x, low.z)) + 0x8000;
}

// 0x80148F1C
extern "C" void initTEneYellowBoss()
{
	resBankLoad(&sYellowBossBank, "ar_ene_yellow_boss.arc");
	resBankAddTextures(&sYellowBossBank, 0, "ene_yellow_boss.gvm");
	resBankAddTextures(&sYellowBossBank, 1, "ene_yellow_boss_onpara.gvm");
	sYellowBossModels = resBankGetModels(&sYellowBossBank);
	sYellowBossTextures = resBankGetTextures(&sYellowBossBank);
	sYellowBossMotion = resBankGetMotion(&sYellowBossBank, 1);
	sYellowBossParams = (int*)enemyParamGet(6);
	fxBankLoadDefault(&sYellowBossFx, "pa_yellowboss.pad");
	yellowBossRingInit();
	loadYellowBossGreen();
}

// 0x80148FD4
extern "C" void shutdownTEneYellowBoss()
{
	releaseYellowBossGreen();
	yellowBossRingExit();
	fxBankRelease(&sYellowBossFx);
	sYellowBossParams = 0;
	resBankRemoveTextures(&sYellowBossBank, 1);
	resBankRemoveTextures(&sYellowBossBank, 0);
	resBankRelease(&sYellowBossBank);
	sYellowBossModels = 0;
	sYellowBossTextures = 0;
	sYellowBossMotion = 0;
}

// 0x80149044
extern "C" TObject* createTEneYellowBoss(TObject* parent, SetDesc* desc)
{
	return new TEneYellowBoss(parent, desc);
}

// 0x80149098
// Model-table word for an index below 8
extern "C" NJS_OBJECT* modelTEneYellowBoss(int index)
{
	if (index < 0 || index >= 8) {
		return 0;
	}
	return sYellowBossModels[index];
}

// 0x801490C0
extern "C" NJS_TEXLIST* textureTEneYellowBoss()
{
	return sYellowBossTextures[0];
}

// 0x801490CC
extern "C" NJS_TEXLIST* texture2TEneYellowBoss()
{
	return sYellowBossTextures[1];
}

// 0x801490D8
extern "C" NJS_MOTION** motionTEneYellowBoss(int index)
{
	return resBankGetMotion(&sYellowBossBank, index);
}

// 0x80149104
// A random point within the radius at the floor's height
extern "C" void randomPointTEneYellowBoss(Vec3f* out, f32 radius)
{
	int angle = (int)(65536.0f * (3.0517578E-5f * (f32)rand()));
	vecFromAngleXZ(radius * (3.0517578E-5f * (f32)rand()), out, angle);
	out->y = 50.02f;
}

// 0x801491BC
extern "C" const int* paramsTEneYellowBoss()
{
	return sYellowBossNodes;
}

// 0x801491C8
TEneYellowBoss::TEneYellowBoss(TObject* parent, SetDesc* desc)
    : TObjEnemy(parent, desc)
{
	readEnemyDesc(desc);
	setupParts();
	initBossBar();
	makeAi();
	setBossView(10.0f, 33.0f, 150.0f);
}

// 0x80149290
TEneYellowBoss::TEneYellowBoss(TObject* parent, YellowBossAi* ai)
    : TObjEnemy(parent)
{
	mBody.mPos.x = 0.0f;
	mBody.mPos.y = 50.02f;
	mBody.mPos.z = 0.0f;
	setupParts();
	mAi = ai;
}

// 0x8014933C
void TEneYellowBoss::setupParts()
{
	ColliShape* shapes;
	int i;

	_00   = (u32)gTEneYellowBossName;
	mSize = 0x41C;
	motionCtrlSetPlayer(&mAnim, &mMotionData);
	motionCtrlInit(&mAnim, sYellowBossMotions, sYellowBossMotion);
	motionCtrlPlay(&mAnim, 0);
	mBody.mRot.x = 0;
	mCollider    = colliCreate(2, 1);
	colliSetOwner(mCollider, (u16)mObjId);
	colliSetMasks(mCollider, 2, 0xFF, 0, 0x10);
	shapes = colliGetShapes(mCollider);
	colliShapeMakeSphere(shapes, &mBody.mPos, 0, 9.0f);
	colliShapeMakeSphere(&shapes[1], &mBody.mPos, 0, 5.0f);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 2, 0xFF, 0xFF, 0xF6);
	shapes = colliGetShapes(mShape);
	colliShapeMakeSphere(shapes, &mBody.mPos, 0, 15.0f);
	shapes[0].mFlags |= 8;
	shapes[0].mFlags |= 0x10;
	colliDisable(mShape);
	for (i = 0; i < 2; i++) {
		colliShapeSetTag(mCollider, i, 0);
	}
	colliShapeSetTag(mShape, 0, 1);
	mAi        = 0;
	mPose      = (GjActor*)allocPose(sYellowBossModels[7]);
	mDrawTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mBossFlags = 0;
	mHitDir.x  = 0.0f;
	mHitDir.y  = 0.0f;
	mHitDir.z  = 0.0f;
	mFrameKey  = 0;
	for (i = 0; i < 5; i++) {
		mPoint[i].x = 0.0f;
		mPoint[i].y = 0.0f;
		mPoint[i].z = 0.0f;
	}
	mMoveTo.x  = 0.0f;
	mMoveTo.y  = 0.0f;
	mMoveTo.z  = 0.0f;
	mMoveToRot = 0;
	mAim.x     = 0.0f;
	mAim.y     = 0.0f;
	mAim.z     = 0.0f;
	mAimRot    = 0;
	_20C       = 0xFFFF;
	mFrom.x    = 0.0f;
	mFrom.y    = 0.0f;
	mFrom.z    = 0.0f;
	mMaxHealth = 500;
	mHealth    = 500;
	shadowSetGround(&mShadow, &mBody.mPos, 0);
	mShadow._30 = 18.0f;
	mShadow._34 = 18.0f;
	shadowEnable(&mShadow);
	mLight.reset();
	mTint.reset();
	mTravelTime = 20.0f;
	mSide       = 1;
	mFx         = 0;
	seCueClear(&mSe);
	mFx2 = 0;
	fxHolderStop(&mGlow);
	for (i = 0; i < 5; i++) {
		mPart[i] = 0;
	}
	mStep  = 0;
	mBurst = 0;
	mWait  = 0;
	_410   = 0;
	_414   = 0;
	mTime  = 0.0f;
}

// 0x80149610
TEneYellowBoss::~TEneYellowBoss()
{
	int i;

	for (i = 0; i < 5; i++) {
		if (mPart[i] != 0) {
			mPart[i]->mFlags |= 1;
			mPart[i] = 0;
		}
	}
	if (mFx != 0) {
		mFx->mFlags |= 1;
		mFx = 0;
	}
	fxHolderStop(&mGlow);
	for (int i = 0; i < 1; i++) {
		if ((&mFx2)[i] != 0) {
			(&mFx2)[i]->mFlags |= 1;
			(&mFx2)[i] = 0;
		}
	}
	unlock();
	if (mAi != 0) {
		delete mAi;
		mAi = 0;
	}
	removeDrawTask(mDrawTask);
	freePose((void**)&mPose);
	colliRetire(mShape);
	mShape = 0;
	colliRetire(mCollider);
	mCollider = 0;
}

// 0x80149794
void TEneYellowBoss::setPos(const Vec3f* pos)
{
	f32 x = pos->x;
	f32 y = pos->y;
	mBody.mPos.x = x;
	f32 z = pos->z;
	mBody.mPos.y = y;
	mBody.mPos.z = z;
	syncCollider();
}

// 0x801497D8
f32 TEneYellowBoss::vfn_40()
{
	return 48.0f;
}

// 0x801497E0
void TEneYellowBoss::syncCollider()
{
	Vec3f spot;
	ColliShape* shapes = colliGetShapes(mCollider);

	shapes[0].mPos          = mPoint[0];
	shapes[1].mPos = mPoint[2];
	colliRefreshBounds(mCollider);
	colliBoundsOn(mCollider);
	if (!colliIsDisabled(mShape)) {
		vecFromAngleXZ(20.0f, &spot, mBody.mRot.y);
		njAddVector(&spot, &mPoint[0]);
		*(Vec3f*)colliGetShapes(mShape) = spot;
	}
}

// 0x8014989C
void TEneYellowBoss::readEnemyDesc(SetDesc* desc)
{
	TObjEnemy::readEnemyDesc(desc);
}

// 0x801498BC
int TEneYellowBoss::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* info = (HitInfo*)result;
	int hit       = 0;

	if ((mBossFlags & BOSS_SOLID) || arg == 1) {
		if (isPlayerOrKind4(other)) {
			info->mDamage = 10;
			hit           = 1;
			info->mFlags.clear();
		} else if (isEggObj(other)) {
			info->mDamage = 1;
			hit           = 1;
			info->mFlags.clear();
			info->mFlags.setBit(13);
		}
	}
	return hit;
}

// 0x8014995C
void TEneYellowBoss::setMoveTo(const Vec3f* pos)
{
	mMoveTo = *pos;
}

// 0x80149978
void TEneYellowBoss::setMoveTo(const Vec3f* pos, int rotY)
{
	mMoveTo    = *pos;
	mMoveToRot = rotY;
}

// 0x80149998
void TEneYellowBoss::setAim(const Vec3f* pos, int rotY)
{
	mAim    = *pos;
	mAimRot = rotY;
}

// 0x801499B8
void TEneYellowBoss::setMaxHealth(int health)
{
	mMaxHealth = health;
	mHealth    = health;
}

// 0x801499C4
void TEneYellowBoss::clearOpen()
{
	mBossFlags &= ~BOSS_OPEN;
}

// 0x801499D4
void TEneYellowBoss::setOpen()
{
	mBossFlags |= BOSS_OPEN;
}

// 0x801499E4
void TEneYellowBoss::setCopy()
{
	mBossFlags |= BOSS_COPY;
}

// 0x801499F4
void TEneYellowBoss::setSolid()
{
	int i;

	for (i = 0; i < 2; i++) {
		colliShapeSetBit8(mCollider, i);
	}
	mBossFlags |= BOSS_SOLID;
}

// 0x80149A44
void TEneYellowBoss::clearSolid()
{
	int i;

	for (i = 0; i < 2; i++) {
		colliShapeClearBit8(mCollider, i);
	}
	mBossFlags &= ~BOSS_SOLID;
}

// 0x80149A94
void TEneYellowBoss::onHardHit()
{
	enemyFxBurst4E(&mContact.mSurface);
	mTint.start();
	creditKiller();
	if (!(mBossFlags & BOSS_COPY)) {
		hideBossBar();
	}
	puff();
	seStartAtFar(0x163, &mBody.mPos);
}

// 0x80149AF8
void TEneYellowBoss::onLightHit()
{
	enemyFxSparks(0.0f, &mContact.mSurface, &mContact.mCenter);
	seStartAtFar(0x155, &mBody.mPos);
}

// 0x80149B3C
int TEneYellowBoss::findFreePart()
{
	int i;

	for (i = 0; i < 5; i++) {
		if (mPart[i] == 0) {
			return i;
		}
	}
	return -1;
}

// 0x80149B78
void TEneYellowBoss::summonPart(int kind)
{
	Vec3f pos;
	int angle;
	int slot;

	yellowBossLower(&pos, &mBody.mPos);
	angle = (int)(10430.38043493439 * (f32)atan2(pos.x, pos.z));
	slot  = findFreePart();
	if (slot >= 0) {
		switch (kind) {
		case 0:
			vecFromAngleXZ(160.0f, &pos, angle - 0x1000);
			yellowBossRaise(&pos, &pos);
			mPart[slot] = new TEneYellowBossGreen(this, &pos);
			break;
		case 1:
			vecFromAngleXZ(160.0f, &pos, angle - 0x1000);
			yellowBossRaise(&pos, &pos);
			mPart[slot] = new TEneYellowBossBlue(this, &pos);
			break;
		case 2:
			vecFromAngleXZ(145.0f, &pos, angle - 0x1000);
			yellowBossRaise(&pos, &pos);
			mPart[slot] = new TEneYellowBossRed(this, &pos);
			break;
		case 3:
			vecFromAngleXZ(160.0f, &pos, angle - 0x1000);
			yellowBossRaise(&pos, &pos);
			mPart[slot] = new TEneYellowBossPurple(this, &pos);
			break;
		case 4:
			vecFromAngleXZ(190.0f, &pos, angle + 0x1200);
			yellowBossRaise(&pos, &pos);
			mPart[slot] = new TEneYellowBossOrange(this, &pos);
			break;
		}
	}
}

// 0x80149D9C
int TEneYellowBoss::partsDone()
{
	int i;

	for (i = 0; i < 5; i++) {
		if (mPart[i] != 0 && !((YellowBossPart*)mPart[i])->isFinished()) {
			return 0;
		}
	}
	return 1;
}

// 0x80149E10
void TEneYellowBoss::removeParts()
{
	int i;

	for (i = 0; i < 5; i++) {
		if (mPart[i] != 0) {
			((YellowBossPart*)mPart[i])->dismiss();
			mPart[i] = 0;
		}
	}
}

// 0x80149E78
int TEneYellowBoss::isDone0()
{
	return isActionDone(0);
}

// 0x80149E9C
int TEneYellowBoss::isDone1()
{
	return isActionDone(1);
}

// 0x80149EC0
int TEneYellowBoss::isDone3()
{
	return isActionDone(3);
}

// 0x80149EE4
int TEneYellowBoss::isPartsDone()
{
	return partsDone();
}

// 0x80149F04
int TEneYellowBoss::isDone4()
{
	return isActionDone(4);
}

// 0x80149F28
int TEneYellowBoss::isDone6()
{
	return isActionDone(6);
}

// 0x80149F4C
int TEneYellowBoss::isDone7()
{
	return isActionDone(7);
}

// 0x80149F70
void TEneYellowBoss::setTravelTime(f32 frames)
{
	mTravelTime = frames;
}

// 0x80149F78
int TEneYellowBoss::isDone9()
{
	return isActionDone(9);
}

// 0x80149F9C
int TEneYellowBoss::isDone11()
{
	return isActionDone(11);
}

// 0x80149FC0
int TEneYellowBoss::isDone12()
{
	return isActionDone(12);
}

// 0x80149FE4
int TEneYellowBoss::isDone14()
{
	return isActionDone(14);
}

// 0x8014A008
int TEneYellowBoss::isDone16()
{
	return isActionDone(16);
}

// 0x8014A02C
int TEneYellowBoss::isDone18()
{
	return isActionDone(18);
}

// 0x8014A050
int TEneYellowBoss::isDone19()
{
	return isActionDone(19);
}

// 0x8014A074
void TEneYellowBoss::updateBody()
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
	if (!(mBossFlags & BOSS_COPY)) {
		setBossBar(mMaxHealth, mHealth);
	}
}

// 0x8014A15C
void TEneYellowBoss::vfn_6C()
{
	if (returnZero()) {
		mEnemyFlags &= ~ENEMY_APPEARING;
		beginNodes();
		gjPushUnitMatrix();
		localMatrix();
		poseFirstFrame(sYellowBossMotion, 0x19, mPose);
		njPopMatrixEx();
		endNodes();
		calcPoints();
		syncCollider();
	}
}

// 0x8014A1E8
void TEneYellowBoss::makeAi()
{
	mAi = createYellowBossAi(this);
}

// 0x8014A218
void TEneYellowBoss::beginNodes()
{
	nodeTrackBeginOrigins(sYellowBossNodes, mPoint);
}

// 0x8014A248
void TEneYellowBoss::endNodes()
{
	nodeTrackEnd();
}

// 0x8014A268
void TEneYellowBoss::updatePose()
{
	beginNodes();
	gjPushUnitMatrix();
	localMatrix();
	applyPose(mPose, &mAnim);
	njPopMatrixEx();
	if (hatchActive(&mAppearTimer)) {
		hatchPose(&mAppearTimer, (GjActor*)mPose, 28.0f);
	}
	endNodes();
}

// 0x8014A2DC
void TEneYellowBoss::updatePoseRooted()
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
	localMatrix();
	applyPose(mPose, &mAnim);
	njPopMatrixEx();
	endNodes();
}

// 0x8014A39C
void TEneYellowBoss::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 5; i++) {
		njCalcPointEx(&mPoint[i], &mPoint[i]);
	}
	njPopMatrixEx();
}

// 0x8014A3FC
int TEneYellowBoss::getHitKind(const void* touch, HitInfo* info)
{
	return (mBossFlags & BOSS_OPEN) ? 3 : 1;
}

// 0x8014A414
int TEneYellowBoss::getDamage(EnemyHit* hit)
{
	int damage = hit->mInfo.mDamage;
	if (hit->mKind != 3) {
		damage = 0;
	}
	return damage;
}

// 0x8014A42C
void TEneYellowBoss::localMatrix()
{
}

// 0x8014A430
void TEneYellowBoss::bodyMatrix()
{
	njTranslateEx(&mBody.mPos);
	gjRotateY(mBody.mRot.y);
	gjRotateX(mBody.mRot.x);
}

// 0x8014A470
void TEneYellowBoss::makeSolidMasks()
{
	colliAddMasks(mCollider, 0xFF, 0, 0);
}

// 0x8014A4A0
void TEneYellowBoss::clearSolidMasks()
{
	colliClearMasks(mCollider, 0xB, 0, 0);
}

// Not in the DOL the linker stripped it
extern "C" void yellowBossStepAt8(TEneYellowBoss* boss)
{
	boss->stepSound(8.0f);
}

// 0x8014A4D0
void TEneYellowBoss::playDemo(int model, int motion)
{
	void* actor = resBankGetObjects(&sYellowBossBank)[model];
	NJS_MOTION** table = resBankGetMotion(&sYellowBossBank, 2);

	mFx = createCamPath(0, 3, actor, table[motion], 0, 1.0f, 1.0f);
	if (mFx != 0) {
		((TCamera*)mFx)->setCut();
	}
}

// 0x8014A560
f32 TEneYellowBoss::riseTo(f32 height)
{
	f32 gap = (50.02f + height) - mBody.mPos.y;

	mBody.mVel.y = approachSpeed(mBody.mVel.y, gap, 0.08f, 0.02f, 0.2f, 10.0f, 0.01f);
	return gap;
}

// 0x8014A5D0
void TEneYellowBoss::damp(f32 rate)
{
	mBody.mVel.x *= rate;
	mBody.mVel.z *= rate;
}

// 0x8014A5EC
void TEneYellowBoss::addContacts(void* collider, Vec3f* out, int shape)
{
	ColliContact* touch = ((CharaColli*)collider)->mContacts;

	while (touch != 0) {
		if (!(colliContactShape(touch)->mFlags & 0x10) && touch->mShape == shape) {
			njAddVector(out, &touch->mPush);
		}
		touch = touch->mNext;
	}
}

// 0x8014A658
int TEneYellowBoss::moveBy(const Vec3f* step)
{
	Vec3f move;
	f32 dy;
	int result;

	move.x = 0.0f;
	move.y = 0.0f;
	move.z = 0.0f;
	addContacts(mCollider, &move, 0);
	move.y = 0.0f;
	njAddVector(&move, step);
	dy     = move.y;
	move.y = 0.0f;
	result = collide(9.0f, &mBody.mPos, &move, &mShadow);
	mBody.mPos.y += dy;
	if (mBody.mPos.y <= 50.02f) {
		mBody.mPos.y = 50.02f;
	}
	mEnemyFlags |= ENEMY_AIRBORNE;
	if (mBossFlags & BOSS_COPY) {
		shadowDisable(&mShadow);
	}
	return result;
}

// 0x8014A738
void TEneYellowBoss::moveDamped(f32 rate)
{
	damp(rate);
	if (moveBy(&mBody.mVel)) {
		mBody.mVel.x = 0.0f;
		mBody.mVel.z = 0.0f;
	}
}

// 0x8014A784
int TEneYellowBoss::moveAlongPath(f32 time)
{
	Vec3f at;
	Vec3f step;

	gjMulAddVector(time, &at, &mFrom, &mBody.mVel);
	step = at;
	njSubVector(&step, &mBody.mPos);
	return moveBy(&step);
}

// 0x8014A7EC
void TEneYellowBoss::planDash()
{
	Vec3f dir;
	Vec3f side;
	f32 len;

	dir = mAim;
	njSubVector(&dir, &mFrom);
	dir.y = 0.0f;
	len   = njUnitVector(&dir);
	if (len < gUnitVectorMin) {
		dir = gVecZero;
	}
	side.x = dir.z;
	side.y = 0.0f;
	side.z = -dir.x;
	if (mSide < 0) {
		gjScaleVector(-1.0f, &side, &side);
		mSide = 1;
	} else {
		mSide = -1;
	}
	gjMulAddVector(0.5f * len, &mMoveTo, &mFrom, &dir);
	gjMulAddVector(0.4f * len, &mMoveTo, &mMoveTo, &side);
	mMoveTo.y = 50.02f;
	mBody.mVel = mMoveTo;
	njSubVector(&mBody.mVel, &mFrom);
	gjScaleVector(0.2f, &mBody.mVel, &mBody.mVel);
	mBody.mVel.y *= 0.5f;
}

// 0x8014A94C
void TEneYellowBoss::puff()
{
	particleOnceByIdDefault(&mBody.mPos, 1, 0x48);
}

// 0x8014A978
void TEneYellowBoss::burst()
{
	Vec3f at;
	at.x = mPoint[3].x;
	at.y = mPoint[3].y;
	at.z = mPoint[3].z;

	njAddVector(&at, &mPoint[4]);
	gjScaleVector(0.5f, &at, &at);
	particleOnceByIdDefault(&at, 1, 0x46);
}

// 0x8014A9DC
void TEneYellowBoss::stepSound(f32 frame)
{
	if (motionCtrlPassed(frame, &mAnim)) {
		seStartAtRange(0x157, &mBody.mPos, 0x1E, 700.0f);
	}
}

// 0x8014AA28
void TEneYellowBoss::onActionPhase(int action, int phase)
{
	switch (action) {
	case 0: actAppear(phase); break;
	case 1: actHover(phase); break;
	case 2: actBreath(phase); break;
	case 3: actSummon(phase); break;
	case 4: actStun(phase); break;
	case 5: actStagger(phase); break;
	case 6: actRise(phase); break;
	case 7: actLand(phase); break;
	case 8: actTurn(phase); break;
	case 9: actTeleport(phase); break;
	case 10: actFade(phase); break;
	case 11: actTurnStep(phase); break;
	case 12: actDash(phase); break;
	case 13: actDashTurn(phase); break;
	case 14: actSwipe(phase); break;
	case 15: actRecoil(phase); break;
	case 16: actTaunt(phase); break;
	case 17: actIdle(phase); break;
	case 18: actDieA(phase); break;
	case 19: actDieB(phase); break;
	case 20: actDie(phase); break;
	}
}

// 0x8014AB5C
void TEneYellowBoss::actAppear(int phase)
{
	switch (phase) {
	case 0:
		mBody.mPos.x = 0.0f;
		mBody.mPos.y = 0.0f;
		mBody.mPos.z = 0.0f;
		mBody.mRot.x = 0;
		mBody.mRot.y = 0;
		mBody.mRot.z = 0;
		motionCtrlPlay(&mAnim, 0x17);
		clearFrameKey();
		setCantAct();
		colliDisable(mCollider);
		shadowDisable(&mShadow);
		mFx    = 0;
		mStep  = 0;
		mBurst = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePoseRooted();
		waitMotion();
		lockOn();
		switch (mStep) {
		case 0:
			playDemo(1, 1);
			seStartArg(0x2AC, 0x258);
			spawnDemoGate0();
			mStep = 1;
			break;
		case 1:
			break;
		}
		calcPoints();
		if (motionCtrlFrame(&mAnim) >= sYellowAppearBursts[mBurst]) {
			mBurst++;
			burst();
		}
		break;
	case 2:
		clearFrameKey();
		mBody.mRot.x = 0;
		colliEnable(mCollider);
		shadowEnable(&mShadow);
		clearCantAct();
		mBody.mPos   = mMoveTo;
		mBody.mRot.y = mMoveToRot;
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		camSetLookFrom(0, &mBody.mPos);
		motionCtrlPlay(&mAnim, 2);
		motionCtrlUpdate(&mAnim);
		unlock();
		break;
	}
}

// 0x8014AD3C
void TEneYellowBoss::actHover(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 2);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		int angle = yellowBossFaceCenter(&mBody.mPos);
		turnTo(angle, 0x400);
		riseTo(50.0f);
		moveDamped(0.9f);
		calcPoints();
		if (motionCtrlPassed(36.0f, &mAnim)) {
			seStartAtFar(0x159, &mBody.mPos);
		}
		break;
	case 2:
		break;
	}
}

// 0x8014AE10
void TEneYellowBoss::actBreath(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 4);
		clearFrameKey();
		setCantAct();
		fxHolderStop(&mGlow);
		seStartAtFar(0x15A, &mBody.mPos);
		break;
	case 1: {
		Vec3f a;
		Vec3f b;

		motionCtrlUpdate(&mAnim);
		updatePose();
		int angle = yellowBossFaceCenter(&mBody.mPos);
		turnTo(angle, 0x400);
		riseTo(50.0f);
		moveDamped(0.9f);
		calcPoints();
		a = mBody.mPos;
		a.y -= 10.0f;
		particleKeepAt((void**)&mFx2, &a, 1, 0x45);
		b = mBody.mPos;
		b.y -= 2.0f;
		mGlow.start(5, 0x11, &b, -0x4000, 0);
		break;
	}
	case 2:
		fxHolderStop(&mGlow);
		if (mFx2 != 0) {
			mFx2->mFlags |= 1;
			mFx2 = 0;
		}
		clearCantAct();
		break;
	}
}

// 0x8014AF8C
void TEneYellowBoss::actSummon(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 5);
		removeParts();
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		turnTo(mAimRot, 0x200);
		riseTo(50.0f);
		moveDamped(0.9f);
		stepSound(13.0f);
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x8014B048
void TEneYellowBoss::actStun(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 6);
		clearFrameKey();
		clearSolidMasks();
		mStep = 0;
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		switch (mStep) {
		case 0:
			riseTo(50.0f);
			moveDamped(0.9f);
			setFrameKey(sYellowBossFrameKeys[0]);
			stepSound(1.0f);
			if (motionCtrlIsDone(&mAnim)) {
				motionCtrlPlayBlend(0.2f, &mAnim, 7);
				mFrameKey = 2;
				mStep     = 1;
			}
			break;
		case 1:
			if (riseTo(0.0f) > -2.0f) {
				mHitFlags |= HIT_DONE;
			}
			stepSound(5.0f);
			moveDamped(0.9f);
			break;
		}
		calcPoints();
		break;
	case 2:
		makeSolidMasks();
		clearFrameKey();
		break;
	}
}

// 0x8014B1AC
void TEneYellowBoss::actStagger(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 8);
		mFrameKey = 2;
		clearSolidMasks();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		riseTo(0.0f);
		moveDamped(0.9f);
		calcPoints();
		break;
	case 2:
		makeSolidMasks();
		clearFrameKey();
		break;
	}
}

// 0x8014B25C
void TEneYellowBoss::actRise(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 9);
		clearFrameKey();
		mStep = 0;
		break;
	case 1: {
		int effect;

		motionCtrlUpdate(&mAnim);
		updatePose();
		effect = 0;
		switch (mStep) {
		case 0:
			riseTo(0.0f);
			setFrameKey(sYellowBossFrameKeys[1]);
			stepSound(20.0f);
			if (motionCtrlPassed(21.0f, &mAnim)) {
				mStep        = 1;
				mBody.mVel.y = 5.0f;
				clearFrameKey();
				effect = 1;
			}
			break;
		case 1:
			if (motionCtrlIsDone(&mAnim)) {
				motionCtrlPlayBlend(0.5f, &mAnim, 0xA);
				mStep = 1;
			}
		case 2: {
			f32 gap      = (50.02f + 30.0f) - mBody.mPos.y;
			mBody.mVel.y = approachSpeed(mBody.mVel.y, gap, 0.1f, 0.02f, 0.2f, 10.0f, 0.01f);
			if (gap < 12.0f) {
				motionCtrlPlayBlend(0.2f, &mAnim, 0xB);
				mStep = 3;
			}
			break;
		}
		case 3:
			riseTo(30.0f);
			if (motionCtrlPassed(5.0f, &mAnim)) {
				effect = 2;
			}
			waitMotion();
			break;
		}
		moveDamped(0.9f);
		calcPoints();
		if (effect == 1) {
			puff();
			particleOnceByIdDefault(&mBody.mPos, 1, 0x42);
		} else if (effect == 2) {
			puff();
		}
		break;
	}
	case 2:
		clearFrameKey();
		break;
	}
}

// 0x8014B478
void TEneYellowBoss::actLand(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0xC);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		riseTo(30.0f);
		moveDamped(0.9f);
		calcPoints();
		if (motionCtrlPassed(8.0f, &mAnim)) {
			seStartAtFar(0x15E, &mBody.mPos);
		}
		break;
	case 2:
		break;
	}
}

// 0x8014B530
void TEneYellowBoss::actTurn(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0xD);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		turnTo(mAimRot, 0x200);
		moveDamped(0.9f);
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x8014B5C0
void TEneYellowBoss::actTeleport(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0xD);
		clearFrameKey();
		colliDisable(mCollider);
		mTime      = -1.0f;
		mBody.mVel = mMoveTo;
		njSubVector(&mBody.mVel, &mBody.mPos);
		gjScaleVector(1.0f / mTravelTime, &mBody.mVel, &mBody.mVel);
		mFrom        = mBody.mPos;
		mBody.mRot.y = mAimRot;
		seStartAtFar(0x15F, &mBody.mPos);
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		mTime += 1.0f;
		if (mTime <= mTravelTime) {
			moveAlongPath(mTime);
		} else {
			mBody.mVel.x = 0.0f;
			mBody.mVel.z = 0.0f;
			mHitFlags |= HIT_DONE;
		}
		mBody.mRot.y = mAimRot;
		calcPoints();
		break;
	case 2:
		colliEnable(mCollider);
		break;
	}
}

// 0x8014B714
void TEneYellowBoss::actFade(int phase)
{
	switch (phase) {
	case 0:
		mEnemyFlags |= ENEMY_INTANGIBLE | ENEMY_BLOCKED;
		mBossFlags |= BOSS_FADING;
		colliDisable(mCollider);
		mFrameKey = 3;
		mFade     = 1.0f;
		seStartAtFar(0x160, &mBody.mPos);
		break;
	case 1:
		mFade -= 0.03f;
		if (mFade <= 0.0f) {
			mFade = 0.0f;
			mHitFlags |= HIT_DONE;
			mFlags |= 1;
		}
		break;
	case 2:
		mEnemyFlags &= ~(ENEMY_INTANGIBLE | ENEMY_BLOCKED);
		mBossFlags &= ~BOSS_FADING;
		clearFrameKey();
		break;
	}
}

// 0x8014B804
void TEneYellowBoss::actTurnStep(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0xE);
		clearFrameKey();
		clearSolidMasks();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		stepSound(1.0f);
		stepSound(8.0f);
		turnTo(mAimRot, 0x200);
		riseTo(0.0f);
		moveDamped(0.9f);
		calcPoints();
		break;
	case 2:
		makeSolidMasks();
		break;
	}
}

// 0x8014B8DC
void TEneYellowBoss::actDash(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0xF);
		clearFrameKey();
		mTime = -1.0f;
		mFrom = mBody.mPos;
		planDash();
		puff();
		if (mFx2 != 0) {
			mFx2->mFlags |= 1;
			mFx2 = 0;
		}
		seStartAtFar(0x161, &mBody.mPos);
		break;
	case 1: {
		Vec3f at;

		motionCtrlUpdate(&mAnim);
		updatePose();
		mBody.mRot.y = mAimRot;
		mTime += 1.0f;
		if (mTime <= 5.0f) {
			if (moveAlongPath(mTime)) {
				setDone();
			}
		} else {
			setDone();
		}
		calcPoints();
		at = mBody.mPos;
		at.y += 2.0f;
		particleKeepAt((void**)&mFx2, &at, 1, 0x41);
		break;
	}
	case 2:
		mBody.mVel.x *= 0.4f;
		mBody.mVel.y = 0.0f;
		mBody.mVel.z *= 0.4f;
		if (mFx2 != 0) {
			mFx2->mFlags |= 1;
			mFx2 = 0;
		}
		break;
	}
}

// 0x8014BA88
void TEneYellowBoss::actDashTurn(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mAnim, 0xF);
		clearFrameKey();
		clearSolidMasks();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		turnTo(mAimRot, 0x200);
		riseTo(0.0f);
		moveDamped(0.9f);
		calcPoints();
		break;
	case 2:
		makeSolidMasks();
		break;
	}
}

// 0x8014BB40
void TEneYellowBoss::actSwipe(int phase)
{
	switch (phase) {
	case 0: {
		Vec3f dir;

		motionCtrlPlayBlend(0.2f, &mAnim, 0x10);
		clearFrameKey();
		colliDisable(mShape);
		mStep = 0;
		mTime = 0.0f;
		dir   = mAim;
		njSubVector(&dir, &mBody.mPos);
		dir.y = 0.0f;
		if (njUnitVector(&dir) < gUnitVectorMin) {
			dir = gVecZero;
		}
		gjMulAddVector(0.5f, &mBody.mVel, &mBody.mVel, &dir);
		fxHolderStop(&mGlow);
		break;
	}
	case 1: {
		f32 frame;

		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		setFrameKey(sYellowBossFrameKeys[2]);
		switch (mStep) {
		case 0: {
			Vec3f dir;
			Vec3f rest;
			f32 keepY;
			f32 along;
			f32 speed;

			turnTo(mAimRot, 0x200);
			riseTo(0.0f);
			keepY = mBody.mVel.y;
			dir   = mAim;
			njSubVector(&dir, &mBody.mPos);
			dir.y = 0.0f;
			if (njUnitVector(&dir) < gUnitVectorMin) {
				dir = gVecZero;
			}
			along  = vecDotXZ(&dir, &mBody.mVel);
			rest   = mBody.mVel;
			rest.y = 0.0f;
			gjMulAddVector(-along, &rest, &rest, &dir);
			gjScaleVector(0.9f, &rest, &rest);
			gjScaleVector(0.1f + along, &dir, &dir);
			njAddVector(&dir, &rest);
			speed = njUnitVector(&dir);
			if (speed < gUnitVectorMin) {
				dir = gVecZero;
			}
			if (speed > 1.2f) {
				speed = 1.2f;
			}
			gjScaleVector(speed, &dir, &dir);
			mBody.mVel   = dir;
			mBody.mVel.y = keepY;
			moveBy(&mBody.mVel);
			if (motionCtrlPassed(13.0f, &mAnim)) {
				mStep = 1;
				mTime = 4096.0f;
			}
			break;
		}
		case 1:
			mTime *= 0.9f;
			mBody.mRot.y += (int)mTime;
			riseTo(0.0f);
			moveDamped(0.9f);
			break;
		}
		calcPoints();
		if (!motionCtrlIsBlending(&mAnim)) {
			frame = motionCtrlFrame(&mAnim);
			if (frame >= 11.5f && frame <= 13.5f) {
				colliEnable(mShape);
			} else {
				colliDisable(mShape);
			}
			if (frame >= 10.0f && frame <= 15.0f) {
				mGlow.start(5, 0x19, &mPoint[1]);
			} else {
				fxHolderStop(&mGlow);
			}
		}
		if (motionCtrlPassed(11.5f, &mAnim)) {
			seStartAtFar(0x162, &mBody.mPos);
		}
		break;
	}
	case 2:
		clearFrameKey();
		colliDisable(mShape);
		fxHolderStop(&mGlow);
		break;
	}
}

// 0x8014BF40
void TEneYellowBoss::actRecoil(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0x11);
		clearFrameKey();
		clearSolidMasks();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		if (!(mBossFlags & BOSS_COPY)) {
			if ((s16)motionCtrlCurrent(&mAnim) == 0x11) {
				stepSound(5.0f);
			} else {
				stepSound(4.0f);
				stepSound(9.0f);
				stepSound(14.0f);
				stepSound(19.0f);
			}
		}
		turnTo(mAimRot + 0x8000, 0x400);
		riseTo(0.0f);
		moveDamped(0.9f);
		calcPoints();
		break;
	case 2:
		makeSolidMasks();
		break;
	}
}

// 0x8014C060
void TEneYellowBoss::actTaunt(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0x13);
		clearFrameKey();
		mStep = 0;
		clearSolidMasks();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		switch (mStep) {
		case 0:
			if (motionCtrlIsDone(&mAnim)) {
				mStep = 1;
				motionCtrlPlay(&mAnim, 0xC);
			}
			break;
		case 1:
			waitMotion();
			break;
		}
		riseTo(0.0f);
		moveDamped(0.9f);
		calcPoints();
		break;
	case 2:
		makeSolidMasks();
		break;
	}
}

// 0x8014C15C
void TEneYellowBoss::actIdle(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mAnim, 0);
		clearFrameKey();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		stepSound(3.0f);
		stepSound(12.0f);
		riseTo(0.0f);
		moveDamped(0.9f);
		calcPoints();
		break;
	case 2:
		break;
	}
}

// 0x8014C204
void TEneYellowBoss::beginDying()
{
	mEnemyFlags |= ENEMY_FLIPPING;
	onHardHit();
	clearSolidMasks();
	mBody.mVel.x = mHitDir.x;
	mBody.mVel.z = mHitDir.z;
	mFrameKey    = 3;
	mBody.mRot.y = (int)(10430.38043493439 * (f32)atan2(mBody.mVel.x, mBody.mVel.z)) + 0x8000;
}

// 0x8014C28C
void TEneYellowBoss::endDying()
{
	mEnemyFlags &= ~ENEMY_FLIPPING;
	makeSolidMasks();
	clearFrameKey();
}

// 0x8014C2CC
void TEneYellowBoss::actDieA(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x16);
		beginDying();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		riseTo(0.0f);
		moveDamped(0.7f);
		calcPoints();
		break;
	case 2:
		endDying();
		break;
	}
}

// 0x8014C370
void TEneYellowBoss::actDieB(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlay(&mAnim, 0x15);
		beginDying();
		break;
	case 1:
		motionCtrlUpdate(&mAnim);
		updatePose();
		waitMotion();
		riseTo(0.0f);
		moveDamped(0.7f);
		calcPoints();
		break;
	case 2:
		endDying();
		break;
	}
}

// 0x8014C414
void TEneYellowBoss::actDie(int phase)
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
			mBody.mPos.x = 0.0f;
			mBody.mPos.y = 0.0f;
			mBody.mPos.z = 0.0f;
			mBody.mRot.x = 0;
			mBody.mRot.y = 0;
			mBody.mRot.z = 0;
			motionCtrlPlay(&mAnim, 0x18);
			mEnemyFlags |= ENEMY_INTANGIBLE | ENEMY_BLOCKED;
			colliDisable(mCollider);
			mFrameKey = 3;
			shadowDisable(&mShadow);
			removeParts();
			objMarkChildrenDeleted(this);
			mStep  = 1;
			mBurst = 0;
			playDemo(0, 0);
			spawnDemoGate1();
			seStartArg(0x2AD, 0x208);
			seCueSet(&mSe, 0x69, 0xA5);
		}
		motionCtrlUpdate(&mAnim);
		updatePoseRooted();
		calcPoints();
		lockOn();
		frame = motionCtrlFrame(&mAnim);
		if (frame >= sYellowDeathBursts[mBurst]) {
			mBurst++;
			burst();
		}
		if (motionCtrlPassed(243.0f, &mAnim)) {
			particleOnceByIdDefault(&mBody.mPos, 1, 0x42);
		}
		seCueUpdate(&mSe, (int)frame);
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
		if (mFx != 0) {
			mFx->mFlags |= 1;
			mFx = 0;
		}
		unlock();
		break;
	}
}

// 0x8014C664
void TEneYellowBoss::vfn_3C()
{
	njPrint((kEnemyInfoX << 16) | kEnemyInfoLine, ":HP %4d", mHealth);
}

// 0x8014C6A0
void TEneYellowBoss::vfn_10()
{
	if (isAppearing()) {
		mFlags |= 0x10;
	} else if (!mLight.isVisible(mDrawTask, 3000.0f, &mBody.mPos, 28.0f, 0)) {
		mFlags |= 0x10;
	} else {
		mFlags &= ~0x10;
		if (mBossFlags & (BOSS_SUIT | BOSS_FADING)) {
			moveDrawTask(mDrawTask, 0x22);
		}
	}
}

// 0x8014C748
void TEneYellowBoss::render()
{
	if (mBossFlags & BOSS_SUIT) {
		texFxSuitLoadTex(2);
	}
	njSetTexture(sYellowBossTextures[0]);
	if (mFrameKey != 0) {
		enemyEyeSet(sYellowBossTextures[0], 0, mFrameKey);
	}
	if (mBossFlags & BOSS_SUIT) {
		gjSetBlendOverride(1, 4, 5, 3);
		_nj_control_3d_flag_ |= 0x8000;
		texFxSuitBegin(0, *(u32*)&mSuit);
	}
	njPushMatrixEx();
	bodyMatrix();
	shadowAlphaBegin();
	if (mBossFlags & BOSS_SUIT) {
		gjDrawActor(mPose);
	} else {
		DrawState state;
		DrawStateSlot* slot;

		state.mFlags = 0;
		state._20 = 0;
		slot       = getDrawStateSlot();
		state      = *(DrawState*)getDefaultDrawState();
		setDrawAmbient(&mLight, &state);
		if (mBossFlags & BOSS_FADING) {
			state.mFlags |= 0x200;
			state.mAmbient = (u32)(255.0f * mFade) & 0xFF;
		}
		mTint.addToLight((u32*)&state);
		beginDrawState(slot, &state);
		gjDrawActor(mPose);
		endDrawState((DrawStateSlot*)slot);
	}
	shadowAlphaEnd();
	njPopMatrixEx();
	if (mFrameKey != 0) {
		enemyEyeRestore();
	}
	shadowSetAngle(&mShadow, mBody.mRot.y);
}

// 0x8014C8F4
// A crystal's virtual _74 its next action is the death
void TEneYellowBossCrystal::kill() { mCrystalFlags |= 4; }
