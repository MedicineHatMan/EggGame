#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "enemy/EnemyAnim.h"
#include "engine/MotionPlayer.h"
#include "enemy/ObjEnemy.h"
#include "object/ObjCommon.h"
#include "engine/FxHolder.h"
#include "egg/ObjEgg.h"
#include "stage/SetManager.h"
#include "item/ItemParts.h"
#include "animal/AniRider.h"
#include "engine/MeshColli.h"
#include "sound/SeHolder.h"
#include "object/ObjAction.h"
#include "enemy/EnemyFx.h"
#include "egg/EggCreature.h"
#include "ui/AeUtil.h"
#include "enemy/boss/TEneLastBossDarkCtrl.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "engine/MotionPose.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "object/ObjShadow.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "engine/WorldMesh.h"
#include "engine/GroundQuery.h"
#include "enemy/boss/TEneLastExBossFeather.h"
#include "animal/AniBody.h"
struct Vec3f;
struct CharaColli;

extern "C" {
// The feather's model motions and textures loaded with the boss
NJS_OBJECT* gLastExBossFeatherObject;  // 0x802C5978
NJS_MOTION** gLastExBossFeatherMotion;  // 0x802C597C
NJS_TEXLIST* gLastExBossFeatherTexList; // 0x802C5980
MotionEntry gLastExBossFeatherMotionTable[] = {
	{ MOTION_LOOP, 0, -1, 0, 0.0f, 20.0f, 1.2f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 20.0f, 1.2f, 0.0f, 0 },
};
}

typedef char TEneLastExBossFeather_size_check[(sizeof(TEneLastExBossFeather) == 0x1FC) ? 1 : -1];

const char* gTEneLastExBossFeatherName = "TEneLastExBossFeather";

// 0x8018A120
TEneLastExBossFeather::TEneLastExBossFeather(TObject* parent, const Vec3f* pos, const Vec3f* aim)
    : TObjAction(parent)
{
	int i;

	_00    = (u32)gTEneLastExBossFeatherName;
	mSize  = 0x1FC;
	setObjId(this, 0xFFFF);
	mAim          = *aim;
	mXform.mPos   = *pos;
	mXform.mVel   = mAim;
	njSubVector(&mXform.mVel, &mXform.mPos);
	if (njUnitVector(&mXform.mVel) < gUnitVectorMin) {
		mXform.mVel = gVecZero;
	}
	f32 scale = 2.0f * (3.0517578e-05f * (f32)rand()) + 8.0f;
	gjScaleVector(scale, &mXform.mVel, &mXform.mVel);
	TEneLastBossFeather::dirToRot(&mXform.mVel, &mXform.mRot);
	motionCtrlSetPlayer(&mClip, &mMotion);
	motionCtrlInit(&mClip, gLastExBossFeatherMotionTable, gLastExBossFeatherMotion);
	motionCtrlPlay(&mClip, 1);
	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mAnim  = actorCreate(gLastExBossFeatherObject);
	mShape = (CharaColli*)colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 2, 255, 255, 0x14);
	colliShapeMakeSphere(colliGetShapes(mShape), &mXform.mPos, 0, 5.0f);
	colliEnable((CharaColli*)mShape);
	colliShapeSetBit8(mShape, 0);
	colliShapeSetBit10((CharaColli*)mShape, 0);
	colliShapeSetTag(mShape, 0, 1);
	mShadow._30 = 9.0f;
	mShadow._34 = 9.0f;
	shadowEnable(&mShadow);
	mTrailKind = 0x46;
	mTrailPhase = 0;
	mGlowKind   = 0x31;
	for (i = 0; i < 8; i++) {
		mPoint[i].x = 0.0f;
		mPoint[i].y = 0.0f;
		mPoint[i].z = 0.0f;
	}
	for (i = 0; i < 4; i++) {
		mPart[i] = 0;
	}
	fxHolderStop(&mGlow);
	mBurst.stop();
	mState  = FEATHER_FLY;
	mTimer  = 0;
	mRadius = 0.2f;
}

// 0x8018A40C
TEneLastExBossFeather::~TEneLastExBossFeather()
{
	mBurst.stop();
	stopTrail();
	colliRetire(mShape);
	mShape = 0;
	actorFree(&mAnim);
	removeDrawTask(mDraw);
	releaseObjId(this);
}

// 0x8018A4F0
// Only once it has burst 10 to a player and an egg is knocked away
int TEneLastExBossFeather::onHit(const TObjAction* other, int, int* result)
{
	int hit = 0;

	if (mState != FEATHER_FLY) {
		if (isPlayerOrKind4(other)) {
			HitInfo* info = (HitInfo*)result;
			info->mDamage = 10;
			hit           = 1;
			info->mFlags.clear();
		} else if (isEggObj(other)) {
			HitInfo* info = (HitInfo*)result;
			info->mDamage = 1;
			hit           = 1;
			info->mFlags.clear();
			info->mFlags.setBit(13);
		}
	}
	return hit;
}

// 0x8018A588
void TEneLastExBossFeather::updateColli() { colliShapeSetPos(colliGetShapes(mShape), &mXform.mPos); }

// 0x8018A5C0
// The shadow falls on the arena floor
void TEneLastExBossFeather::updateShadow()
{
	Vec3f at;
	Vec3f normal;

	at       = mXform.mPos;
	at.y     = 20.0f;
	normal.x = 0.0f;
	normal.y = 1.0f;
	normal.z = 0.0f;
	shadowSetGround(&mShadow, &at, &normal);
}

// 0x8018A624
void TEneLastExBossFeather::trackNodes()
{
	nodeTrackBeginOrigins((const int*)lastBossFeatherTable(), (Vec3f*)mPoint);
	gjPushUnitMatrix();
	actorApplyMotion(mAnim, &mClip);
	njPopMatrixEx();
	nodeTrackEnd();
}

// 0x8018A670
// Takes the points into the world
void TEneLastExBossFeather::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	applyMatrix();
	for (i = 0; i < 8; i++) {
		njCalcPointEx(&mPoint[i], &mPoint[i]);
	}
	njPopMatrixEx();
}

// 0x8018A6D0
// Every other point drops a trail particle the halves taking turns
void TEneLastExBossFeather::emitTrail()
{
	int i;
	int point;

	for (i = 0, point = mTrailPhase; i < 4 && point < 8; i++, point += 2) {
		particleKeepAt((void**)&mPart[i], (const Vec3f*)&mPoint[point], 1, mTrailKind);
	}
	mTrailPhase++;
	if (mTrailPhase > 1) {
		mTrailPhase = 0;
	}
	mGlow.start(5, mGlowKind, &mPoint[7]);
}

// 0x8018A77C
void TEneLastExBossFeather::stopTrail()
{
	int i;

	fxHolderStop(&mGlow);
	for (i = 3; i >= 0; i--) {
		if (mPart[i] != 0) {
			mPart[i]->mFlags |= 1;
			mPart[i] = 0;
		}
	}
}

// 0x8018A7E4
void TEneLastExBossFeather::applyMatrix()
{
	njTranslateEx(&mXform.mPos);
	gjRotateY(mXform.mRot.y);
	gjRotateX(mXform.mRot.x);
	gjRotateZ(mXform.mRot.z);
}

// 0x8018A82C
// The burst lies flat where the feather landed
void TEneLastExBossFeather::placeBurst()
{
	mBurst.mMtx.setPosRotScale(&mXform.mPos, -0x4000, 0, 0.39473686f, 0.39473686f, 0.39473686f);
}

// 0x8018A86C
void TEneLastExBossFeather::draw()
{
	switch (mState) {
	case FEATHER_FLY:
		motionCtrlUpdate(&mClip);
		trackNodes();
		njAddVector(&mXform.mPos, &mXform.mVel);
		if (mXform.mPos.y <= mAim.y) {
			mXform.mPos.y = mAim.y;
		}
		calcPoints();
		emitTrail();
		updateColli();
		updateShadow();
		if (mXform.mPos.y <= mAim.y) {
			mState  = FEATHER_LAND;
			mTimer  = 20;
			mRadius = 8.0f;
			stopTrail();
			shadowDisable(&mShadow);
			seStartAt(0x1A9, &mXform.mPos);
			mBurst.start(5, 0x17);
			placeBurst();
		}
		break;
	case FEATHER_LAND:
		mTimer--;
		if (mTimer <= 0) {
			mState = FEATHER_GROW;
		}
		placeBurst();
		break;
	case FEATHER_GROW:
		mRadius += 0.2f;
		if (mRadius >= 15.0f) {
			mRadius = 15.0f;
			mState  = FEATHER_FADE;
			mTimer  = 60;
			colliDisable(mShape);
		}
		colliShapeSetRadius(colliGetShapes(mShape), mRadius);
		placeBurst();
		break;
	case FEATHER_FADE:
		mTimer--;
		if (mTimer <= 0) {
			mFlags |= 1;
		}
		placeBurst();
		break;
	}
}

// 0x8018AA2C
void TEneLastExBossFeather::vfn_10()
{
	if (cameraIsVisible(2500.0f, 30.0f, &mXform.mPos, 0)) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x8018AA8C
void TEneLastExBossFeather::render()
{
	if (mState == FEATHER_FLY) {
		njSetTexture(gLastExBossFeatherTexList);
		njPushMatrixEx();
		applyMatrix();
		drawModelLit(mAnim);
		njPopMatrixEx();
	}
}

// 0x8018AAE0
int TEneLastExBossFeather::getKind()
{
	return 2;
}

// 0x8018AAE8
Vec3f* TEneLastExBossFeather::getPos()
{
	return &mXform.mPos;
}
