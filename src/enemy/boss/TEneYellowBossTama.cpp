#include "camera/TCameraCtrl.h"
struct DrawTask;
#include "enemy/boss/TEneBoss03_VolRock.h"
#include "animal/RacerMover.h"
#include "enemy/boss/TEneYellowBossCrystal.h"
#include "enemy/boss/TEneOrangeBoss.h"
#include "enemy/boss/TEneYellowBossTama_Orange.h"
#include "enemy/EnemyGround.h"
#include "egg/ObjEgg.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "enemy/EnemyFx.h"
#include "object/ObjShadow.h"
#include "enemy/ObjEnemy.h"
#include "ui/AeUtil.h"
#include "stl/math.h"
#include "enemy/EnemyAnim.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyTexFx.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "engine/MotionPose.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "enemy/boss/TEneYellowBossOrange.h"
#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "stage/SetManager.h"
#include "object/ObjCommon.h"
#include "enemy/EnemySight.h"
#include "engine/CharaColli.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "enemy/boss/TEneYellowBossTama_ShockWave.h"
#include "enemy/boss/TEneYellowBossPurple.h"
#include "engine/DrawState.h"

struct Vec3f;
struct CharaColli;
struct GjActor;

// 0x80150818
// Finds the arena floor under pos the middle is flat the ring past 230 has none
extern "C" int crystalGroundAt(const Vec3f* pos, f32* groundY, ObjShadow* shadow)
{
	Vec3f hit;
	f32 len;
	f32 y;
	int ground;

	hit.x = pos->x;
	hit.y = pos->y;
	hit.z = pos->z;
	len   = vecLenXZ(&hit);
	y     = 50.02f;
	if (len < 205.0f) {
		ground = 1;
	} else if (len <= 230.0f) {
		Vec3f from = *pos;
		from.y += 10.0f;
		ground = probeGround(&from, &hit, 0);
		if (ground) {
			y = hit.y;
		}
	} else {
		ground = 0;
	}
	if (groundY != 0) {
		*groundY = y;
	}
	if (shadow != 0) {
		if (ground) {
			Vec3f at;
			Vec3f up;
			at.y = pos->y;
			at.x = pos->x;
			at.z = pos->z;
			at.y = y;
			up.x = 0.0f;
			up.y = 1.0f;
			up.z = 0.0f;
			shadowSetGround(shadow, &at, &up);
			shadowEnable(shadow);
		} else {
			shadowDisable(shadow);
		}
	}
	return ground;
}

#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "stage/SetManager.h"
#include "object/ObjCommon.h"
#include "enemy/EnemySight.h"
#include "engine/CharaColli.h"
#include "enemy/boss/TEneYellowBossTama_Orange.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "enemy/boss/TEneYellowBossOrange.h"
#include "enemy/boss/TEneYellowBossPurple.h"
#include "sound/SeHolder.h"
#include "enemy/ObjEnemy.h"
#include "enemy/boss/TEneBoss03_VolRock.h"
#include "object/ObjShadow.h"
#include "engine/DrawState.h"
#include "camera/TCameraCtrl.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "enemy/EnemyFx.h"
#include "animal/RacerMover.h"

const char* sOrangeTamaName = "TEneYellowBossTama_Orange";
const char* sBlueTamaName   = "TEneYellowBossTama_Blue";
const char* sShockWaveName  = "TEneYellowBossTama_ShockWave";
struct CharaColli;

typedef char TEneYellowBossTama_Orange_size_check[(sizeof(TEneYellowBossTama_Orange) == 0x110) ? 1 : -1];

// 0x80150960
TEneYellowBossTama_Orange::TEneYellowBossTama_Orange(TObject* parent, const Vec3f* pos)
    : TObjAction(parent)
{
	EnemySight* sight;

	_00   = (u32)sOrangeTamaName;
	mSize = 0x110;
	setObjId(this, 0xFFFF);
	mModel      = modelTEneYellowBoss(3);
	mXform.mPos = *pos;
	shadowSetGround(&mShadow, &mXform.mPos, 0);
	mShadow._30 = 9.0f;
	mShadow._34 = 9.0f;
	shadowEnable(&mShadow);
	sight = &mSight;
	sight->reset();
	mSight.mHeightLimit = 1000.0f;
	mShape              = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 2, 255, 0, 0x16);
	colliShapeMakeSphere(colliGetShapes((CharaColli*)mShape), &mXform.mPos, 0, 5.0f);
	colliDisable((CharaColli*)mShape);
	colliShapeSetBit8((CharaColli*)mShape, 0);
	mDraw    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mAimed   = 1;
	mDrift.x = 0.0f;
	mDrift.y = 0.0f;
	mDrift.z = 0.0f;
	mTarget  = mXform.mPos;
	mState   = ORANGE_TAMA_HELD;
	mTimer   = 0;
	mRadius  = 5.0f;
}

// 0x80150AFC
TEneYellowBossTama_Orange::~TEneYellowBossTama_Orange()
{
	removeDrawTask(mDraw);
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	releaseObjId(this);
}

// 0x80150BBC
int TEneYellowBossTama_Orange::onHit(const TObjAction* other, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int hurt     = 0;
	if (mState == ORANGE_TAMA_FLY || mState == ORANGE_TAMA_SWELL) {
		hit->mFlags.clear();
		hit->mPos.x = 0.0f;
		hit->mPos.y = 0.0f;
		hit->mPos.z = 0.0f;
		if (isPlayerObj(other)) {
			hurt         = 1;
			hit->mDamage = 10;
		} else if (isEggObj(other)) {
			hit->mDamage = 1;
			hurt         = 1;
			hit->mFlags.setBit(0xD);
		}
	}
	return hurt;
}

// 0x80150C64
void TEneYellowBossTama_Orange::placeColli()
{
	ColliShape* shape = colliGetShapes(mShape);
	if (shape != NULL) {
		colliShapeSetPos(shape, &mXform.mPos);
		colliShapeSetRadius(shape, mRadius);
	}
}

// 0x80150CB0
void TEneYellowBossTama_Orange::placeShadow()
{
	crystalGroundAt(&mXform.mPos, NULL, &mShadow);
}

// 0x80150CE0
void TEneYellowBossTama_Orange::setPos(const Vec3f* pos)
{
	mXform.mPos.set(pos->x, pos->y, pos->z);
}

// 0x80150CFC
// Lands on the floor y 50.02 at the target 120 frames on leaving at 4 up
void TEneYellowBossTama_Orange::throwAt(const Vec3f* target, int aimed)
{
	Vec3f dir;

	mAimed    = aimed;
	mTarget.x = target->x;
	mTarget.y = target->y;
	mTarget.z = target->z;
	mTarget.y = 50.02f;
	dir.x     = mTarget.x;
	dir.y     = mTarget.y;
	dir.z     = mTarget.z;
	njSubVector(&dir, &mXform.mPos);
	f32 height = dir.y;
	dir.y      = 0.0f;
	f32 dist   = njUnitVector(&dir);
	if (dist < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 speed       = dist / 120.0f;
	mXform.mVel.x   = dir.x * speed;
	mXform.mVel.z   = dir.z * speed;
	mXform.mVel.y   = 4.0f;
	mXform.mForce.y = gravityInSteps(120, mXform.mVel.y, height);
	mState          = ORANGE_TAMA_FLY;
	colliEnable(mShape);
	placeColli();
	placeShadow();
}

// 0x80150E14
void TEneYellowBossTama_Orange::draw()
{
	switch (mState) {
	case ORANGE_TAMA_HELD:
		placeShadow();
		break;
	case ORANGE_TAMA_FLY: {
		f32 groundY;
		mXform.mVel.y += mXform.mForce.y;
		njAddVector(&mXform.mPos, &mXform.mVel);
		if (mAimed) {
			Vec3f moved;
			mSight.findInRange(100000000.0f, &mXform.mPos);
			moved.x = mSight.mTarget.x;
			moved.y = mSight.mTarget.y;
			moved.z = mSight.mTarget.z;
			njSubVector(&moved, &mTarget);
			moved.y = 0.0f;
			if (njUnitVector(&moved) < gUnitVectorMin) {
				moved = gVecZero;
			}
			gjMulAddVector(0.15f, &mDrift, &mDrift, &moved);
			f32 len = njUnitVector(&mDrift);
			if (len < gUnitVectorMin) {
				mDrift.x = gVecZero.x;
				mDrift.y = gVecZero.y;
				mDrift.z = gVecZero.z;
			}
			if (len > 1.5f) {
				len = 1.5f;
			}
			gjScaleVector(len, &mDrift, &mDrift);
			njAddVector(&mXform.mPos, &mDrift);
			mTarget.x = mSight.mTarget.x;
			mTarget.y = mSight.mTarget.y;
			mTarget.z = mSight.mTarget.z;
		}
		if (crystalGroundAt(&mXform.mPos, &groundY, &mShadow)) {
			if (mXform.mPos.y <= groundY) {
				mXform.mPos.y = groundY;
				mState        = ORANGE_TAMA_SWELL;
				mTimer        = 3;
				colliShapeSetBit10(mShape, 0);
				shadowDisable(&mShadow);
				seStartAt(0x171, &mXform.mPos);
				fxSpawnScaled(2.3529413f, 2.3529413f, 2.3529413f, 5, (void*)0xE, (const Vec3f*)&mXform.mPos, -0x4000, 0);
			}
		} else if (50.02f - mXform.mPos.y >= 100.0f) {
			mFlags |= 1;
			colliDisable(mShape);
		}
		placeColli();
		break;
	}
	case ORANGE_TAMA_SWELL:
		mRadius += 1.6666666f;
		if (mRadius >= 40.0f) {
			mRadius = 40.0f;
			mTimer--;
			if (mTimer <= 0) {
				mFlags |= 1;
			}
		}
		placeColli();
		break;
	}
}

// 0x801510A8
void TEneYellowBossTama_Orange::vfn_10()
{
	if (mState == ORANGE_TAMA_SWELL) {
		mFlags |= 0x10;
	} else if (cameraIsVisible(3000.0f, mRadius, &mXform.mPos, NULL)) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x80151124
// The model is 5 across
void TEneYellowBossTama_Orange::render()
{
	crystalBeginDraw();
	njPushMatrixEx();
	njTranslateEx(&mXform.mPos);
	f32 scale = mRadius / 5.0f;
	njScale(NULL, scale, scale, scale);
	crystalSetTex(mXform.mRot.y);
	shadowAlphaBegin();
	drawObjectDefault(mModel);
	shadowAlphaEnd();
	njPopMatrixEx();
	crystalEndDraw();
}

struct DrawTask;
struct CharaColli;

typedef char TEneYellowBossTama_Blue_size_check[(sizeof(TEneYellowBossTama_Blue) == 0x100) ? 1 : -1];

// 0x80151198
TEneYellowBossTama_Blue::TEneYellowBossTama_Blue(TObject* parent, const Vec3f* pos, const Vec3f* dir)
    : TObjAction(parent)
{
	EnemySight* sight;

	_00   = (u32)sBlueTamaName;
	mSize = 0x100;
	setObjId(this, 0xFFFF);
	mModel        = modelTEneYellowBoss(3);
	mXform.mPos.x = pos->x;
	mXform.mPos.y = pos->y;
	mXform.mPos.z = pos->z;
	mXform.mPos.y -= 7.5f;
	mDir.x = dir->x;
	mDir.y = dir->y;
	mDir.z = dir->z;
	crystalGroundAt(&mXform.mPos, 0, &mShadow);
	mShadow._30 = 13.5f;
	mShadow._34 = 13.5f;
	shadowEnable(&mShadow);
	sight = &mSight;
	sight->reset();
	mSight.mHeightLimit = 1000.0f;
	mShape              = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 2, 255, 0, 0x16);
	colliShapeMakeSphere(colliGetShapes((CharaColli*)mShape), &mXform.mPos, 0, 7.5f);
	colliEnable((CharaColli*)mShape);
	colliShapeSetBit8((CharaColli*)mShape, 0);
	mDraw    = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mState   = BLUE_TAMA_FLY;
	mBounces = 0;
	mSquash  = 1.0f;
	launchAt(&mXform.mPos, &mDir, &mXform.mVel);
}

// 0x80151344
TEneYellowBossTama_Blue::~TEneYellowBossTama_Blue()
{
	removeDrawTask(mDraw);
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	releaseObjId(this);
}

// 0x80151404
int TEneYellowBossTama_Blue::onHit(const TObjAction* other, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int hurt     = 0;
	if (isPlayerObj(other)) {
		hurt = 1;
		hit->mFlags.clear();
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 10;
	} else if (isEggObj(other)) {
		hurt = 1;
		hit->mFlags.clear();
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 1;
		hit->mFlags.setBit(0xD);
	}
	return hurt;
}

// 0x801514B0
// The sphere sits on top of the shot
void TEneYellowBossTama_Blue::placeColli()
{
	ColliShape* shape = colliGetShapes(mShape);
	if (shape != NULL) {
		Vec3f center;
		center.x = mXform.mPos.x;
		center.y = mXform.mPos.y;
		center.z = mXform.mPos.z;
		center.y += 7.5f;
		colliShapeSetPos(shape, &center);
	}
}

// 0x80151514
// A hop from from to to two units of ground a frame under a gravity of 0.06
void TEneYellowBossTama_Blue::launchAt(const Vec3f* from, const Vec3f* to, Vec3f* vel)
{
	Vec3f dir;
	dir.x = to->x;
	dir.y = to->y;
	dir.z = to->z;
	njSubVector(&dir, from);
	f32 height = dir.y;
	dir.y      = 0.0f;
	f32 dist   = njUnitVector(&dir);
	if (dist < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 steps = ceil(dist / 2.0f);
	f32 speed = dist / steps;
	f32 up    = launchSpeedInSteps(steps, height, -0.06f);
	vel->x    = dir.x;
	vel->y    = dir.y;
	vel->z    = dir.z;
	vel->x *= speed;
	vel->y = up;
	vel->z *= speed;
}

// 0x8015161C
void TEneYellowBossTama_Blue::draw()
{
	switch (mState) {
	case BLUE_TAMA_FLY: {
		f32 groundY;
		mXform.mVel.y += -0.06f;
		njAddVector(&mXform.mPos, &mXform.mVel);
		if (crystalGroundAt(&mXform.mPos, &groundY, &mShadow)) {
			if (mXform.mVel.y <= 0.0f && mXform.mPos.y <= groundY) {
				mBounces++;
				mXform.mPos.y = groundY;
				if (mBounces > 3) {
					mState = BLUE_TAMA_FADE;
					colliDisable(mShape);
				} else {
					mState = BLUE_TAMA_SQUASH;
				}
				new TEneYellowBossTama_ShockWave(mParent, &mXform.mPos);
				seStartAt(0x168, &mXform.mPos);
			}
		} else if (50.02f - mXform.mPos.y >= 50.0f) {
			mFlags |= 1;
		}
		placeColli();
		break;
	}
	case BLUE_TAMA_SQUASH:
		mSquash -= 0.1f;
		if (mSquash <= 0.4f) {
			mSquash = 0.4f;
			mState  = BLUE_TAMA_UNSQUASH;
		}
		break;
	case BLUE_TAMA_UNSQUASH:
		mSquash += 0.1f;
		if (mSquash >= 1.0f) {
			mSquash = 1.0f;
			if (mSight.findInRange(100000000.0f, &mXform.mPos)) {
				mDir.x = mSight.mTarget.x;
				mDir.y = mSight.mTarget.y;
				mDir.z = mSight.mTarget.z;
				njSubVector(&mDir, &mXform.mPos);
				mDir.y   = 0.0f;
				f32 dist = njUnitVector(&mDir);
				if (dist < gUnitVectorMin) {
					mDir.x = gVecZero.x;
					mDir.y = gVecZero.y;
					mDir.z = gVecZero.z;
				}
				dist *= 1.3f;
				gjScaleVector(dist, &mDir, &mDir);
				njAddVector(&mDir, &mXform.mPos);
				mDir.y = 50.02f;
			}
			mState = BLUE_TAMA_FLY;
			launchAt(&mXform.mPos, &mDir, &mXform.mVel);
		}
		break;
	case BLUE_TAMA_FADE:
		mSquash -= 0.1f;
		if (mSquash <= 0.0f) {
			mSquash = 0.01f;
			mFlags |= 1;
		}
		break;
	}
}

// 0x801518C8
void TEneYellowBossTama_Blue::vfn_10()
{
	if (cameraIsVisible(3000.0f, 15.0f, &mXform.mPos, NULL)) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x80151928
// Squashed down it spreads out
void TEneYellowBossTama_Blue::render()
{
	crystalBeginDraw();
	njPushMatrixEx();
	njTranslateEx(&mXform.mPos);
	f32 width  = 2.0f - mSquash;
	f32 height = mSquash;
	height *= 1.5f;
	width *= 1.5f;
	njScale(NULL, width, height, width);
	gjTranslate(0.0f, 5.0f, 0.0f);
	crystalSetTex(0);
	shadowAlphaBegin();
	drawObjectDefault(mModel);
	shadowAlphaEnd();
	njPopMatrixEx();
	crystalEndDraw();
}

typedef char TEneYellowBossTama_ShockWave_size_check[(sizeof(TEneYellowBossTama_ShockWave) == 0x48) ? 1 : -1];

// 0x801519B4
TEneYellowBossTama_ShockWave::TEneYellowBossTama_ShockWave(TObject* parent, const Vec3f* pos)
    : TObjAction(parent)
{
	_00   = (u32)sShockWaveName;
	mSize = 0x48;
	setObjId(this, 0xFFFF);
	mPos    = *pos;
	mRadius = -33.333332f;
	mLife   = 10;
	mShape  = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 2, 255, 0, 0x16);
	colliShapeMakeCylinder(colliGetShapes(mShape), &mPos, 1, 5.0f, 3.0f);
	colliEnable(mShape);
	colliShapeSetBit8(mShape, 0);
	fxSpawnScaled(2.2727273f, 2.2727273f, 1.0f, 5, (void*)0x14, (const Vec3f*)&mPos, (int)0xFFFFC000, 0);
}

// 0x80151AC4
TEneYellowBossTama_ShockWave::~TEneYellowBossTama_ShockWave()
{
	colliRetire(mShape);
	mShape = 0;
	releaseObjId(this);
}

// 0x80151B3C
int TEneYellowBossTama_ShockWave::onHit(const TObjAction* other, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int hurt     = 0;
	if (isPlayerObj(other)) {
		hurt = 1;
		hit->mFlags.clear();
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 10;
	} else if (isEggObj(other)) {
		hurt = 1;
		hit->mFlags.clear();
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 1;
		hit->mFlags.setBit(0xD);
	}
	return hurt;
}

// 0x80151BE8
void TEneYellowBossTama_ShockWave::draw()
{
	mRadius += 3.3333333f;
	if (mRadius >= 50.0f) {
		mRadius = 50.0f;
		mLife--;
		if (mLife <= 0) {
			mFlags |= 1;
		}
	} else {
		f32 radius;
		if (mRadius < 5.0f) {
			radius = 5.0f;
		} else {
			radius = mRadius;
		}
		colliShapeSetCylRadius(colliGetShapes(mShape), radius);
	}
}

// 0x80151C8C
int TEneYellowBossTama_ShockWave::getKind()
{
	return 2;
}

// 0x80151C94
Vec3f* TEneYellowBossTama_ShockWave::getPos()
{
	return &mPos;
}

// 0x80151C9C
int TEneYellowBossTama_Blue::getKind()
{
	return 2;
}

// 0x80151CA4
Vec3f* TEneYellowBossTama_Blue::getPos()
{
	return &mXform.mPos;
}

// 0x80151CAC
int TEneYellowBossTama_Orange::getKind()
{
	return 2;
}

// 0x80151CB4
Vec3f* TEneYellowBossTama_Orange::getPos()
{
	return &mXform.mPos;
}
