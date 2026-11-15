#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyAnim.h"
#include "object/ObjAction.h"
#include "enemy/EnemyFx.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "stl/math.h"
#include "engine/MotionPose.h"
#include "ui/AeUtil.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "object/ObjShadow.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "object/last/TObjLastEggLight.h"
#include "enemy/boss/TEneLastBossDarkCtrl.h"
#include "animal/RacerMover.h"
struct Vec3f;
struct CharaColli;

extern "C" {
MotionEntry sLastBossFeatherMotions[] = {
	{ MOTION_LOOP, 0, -1, 0, 0.0f, 20.0f, 1.2f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 20.0f, 1.2f, 0.0f, 0 },
};
}

typedef char TEneLastBossFeather_size_check[(sizeof(TEneLastBossFeather) == 0x1F4) ? 1 : -1];

const int gLastBossFeatherNodes[] = { 2, 3, 4, 5, 6, 7, 8, 9, -1 };

const char* gTEneLastBossFeatherName = "TEneLastBossFeather";

// 0x8015D314
TEneLastBossFeather::TEneLastBossFeather(TObject* parent)
    : TObjAction(parent)
{
	int i;

	_00   = (u32)gTEneLastBossFeatherName;
	mSize = 0x1F4;
	setObjId(this, 0xFFFF);
	motionCtrlSetPlayer(&mClip, &mMotion);
	motionCtrlInit(&mClip, sLastBossFeatherMotions, gLastBossFeatherMotion);
	motionCtrlPlay(&mClip, 0);
	mDraw  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mAnim  = actorCreate(gLastBossFeatherObject);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 2, 255, 255, 0x14);
	colliShapeMakeSphere(colliGetShapes((CharaColli*)mShape), &mXform.mPos, 0, 5.0f);
	colliEnable((CharaColli*)mShape);
	colliShapeSetBit8((CharaColli*)mShape, 0);
	colliShapeSetBit10((CharaColli*)mShape, 0);
	colliShapeSetTag(mShape, 0, 1);
	mShadow._30 = 9.0f;
	mShadow._34 = 9.0f;
	shadowEnable(&mShadow);
	mAim.x      = 0.0f;
	mAim.y      = 0.0f;
	mAim.z      = 0.0f;
	mAimRotY    = 0;
	mTrailKind  = 0x45;
	mTrailPhase = 0;
	mGlowKind   = 0xF;
	for (i = 0; i < 8; i++) {
		mPoint[i].x = 0.0f;
		mPoint[i].y = 0.0f;
		mPoint[i].z = 0.0f;
	}
	for (i = 0; i < 4; i++) {
		mPart[i] = 0;
	}
	fxHolderStop(&mGlow);
	seHolderStop(&mRing);
}

// 0x8015D524
TEneLastBossFeather::~TEneLastBossFeather()
{
	seHolderStop(&mRing);
	stopTrail();
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	actorFree(&mAnim);
	removeDrawTask(mDraw);
	releaseObjId(this);
}

static inline Vec3f featherSub(const Vec3f* a, const Vec3f* b)
{
	Vec3f v;
	gjSubVector(&v, a, b);
	return v;
}

// 0x8015D608
void TEneLastBossFeather::orient(int rotY, const f32* mtx)
{
	f32 m[12];
	Vec3f side;
	Vec3f fwd;
	int pitch;
	int yaw;

	mtxSetRotY(m, rotY);
	mtxConcat(m, m, mtx);
	mXform.mVel.x = 0.0f;
	mXform.mVel.y = 1.0f;
	mXform.mVel.z = 0.0f;
	mtxMultVecSR(m, &mXform.mVel, &mXform.mVel);
	fwd.x  = 0.0f;
	fwd.y  = 0.0f;
	fwd.z  = 1.0f;
	side.x = 1.0f;
	side.y = 0.0f;
	side.z = 0.0f;
	mtxMultVecSR(m, &fwd, &fwd);
	f32 a = (f32)atan2(fwd.x, fwd.z);
	f32 x = fwd.x;
	f32 z = fwd.z;
	x *= x;
	z *= z;
	x += z;
	f32 r = (f32)__frsqrte(x);
	yaw   = (int)(10430.38043493439 * a);
	pitch = (int)(10430.38043493439 * (f32)atan2(-fwd.y, __fres(r)));
	mtxMultVecSR(m, &side, &side);
	mtxSetRotX(m, -pitch);
	mtxRotateY(m, m, -yaw);
	mtxMultVecSR(m, &side, &side);
	f32 roll      = (f32)atan2(side.y, side.x);
	mXform.mRot.x = pitch;
	mXform.mRot.y = yaw;
	mXform.mRot.z = (int)(10430.38043493439 * roll);
}

// 0x8015D774
void TEneLastBossFeather::updateColli()
{
	colliShapeSetPos(colliGetShapes(mShape), &mXform.mPos);
}

// 0x8015D7AC
// The shadow falls on the arena floor
void TEneLastBossFeather::updateShadow()
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

// 0x8015D810
void TEneLastBossFeather::trackNodes()
{
	nodeTrackBeginOrigins(gLastBossFeatherNodes, mPoint);
	gjPushUnitMatrix();
	actorApplyMotion(mAnim, &mClip);
	njPopMatrixEx();
	nodeTrackEnd();
}

// 0x8015D860
// Takes the points into the world
void TEneLastBossFeather::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	applyMatrix();
	for (i = 0; i < 8; i++) {
		njCalcPointEx(&mPoint[i], &mPoint[i]);
	}
	njPopMatrixEx();
}

// 0x8015D8C0
// Every other point drops a trail particle the halves taking turns
void TEneLastBossFeather::emitTrail()
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

// 0x8015D96C
void TEneLastBossFeather::stopTrail()
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

// 0x8015D9D4
void TEneLastBossFeather::applyMatrix()
{
	njTranslateEx(&mXform.mPos);
	gjRotateY(mXform.mRot.y);
	gjRotateX(mXform.mRot.x);
	gjRotateZ(mXform.mRot.z);
}

// 0x8015DA1C
void TEneLastBossFeather::setAim(const Vec3f* pos, int rotY)
{
	mAim     = *pos;
	mAimRotY = rotY;
}

// 0x8015DA3C
// Springs toward the aim
void TEneLastBossFeather::approachAim()
{
	Vec3f to;

	to.x = mAim.x;
	to.y = mAim.y;
	to.z = mAim.z;
	njSubVector(&to, &mXform.mPos);
	springVelocity(0.1f, 0.8f, &mXform.mVel, 0.1f, &to, 0.2f, 5.0f, 0.1f);
	njAddVector(&mXform.mPos, &mXform.mVel);
}

// 0x8015DAB8
// Stands up and turns to the aim's heading
void TEneLastBossFeather::turnToAim()
{
	mXform.mRot.x = angleApproach(mXform.mRot.x, -0x4000, 0x180);
	mXform.mRot.z = angleApproach(mXform.mRot.z, 0, 0x180);
	mXform.mRot.y = angleApproach(mXform.mRot.y, mAimRotY, 0x180);
}

// 0x8015DB1C
// Bends the velocity toward the target the part along it grows by ahead or behind when moving away and the part across
// it is scaled by side
f32 TEneLastBossFeather::steer(f32 ahead, f32 behind, f32 side, f32 maxSpeed, const Vec3f* target)
{
	Vec3f to;
	Vec3f along;
	Vec3f across;
	f32 dist;
	f32 dot;
	f32 speed;

	f32 ty = target->y;
	f32 tz = target->z;
	to.set(target->x, ty, tz);
	njSubVector(&to, &mXform.mPos);
	dist = njUnitVector(&to);
	if (dist < gUnitVectorMin) {
		to = gVecZero;
	}
	dot     = njInnerProduct(&mXform.mVel, &to);
	along.x = to.x;
	along.y = to.y;
	along.z = to.z;
	gjScaleVector(dot, &along, &along);
	Vec3f perp = featherSub(&mXform.mVel, &along);
	across.x   = perp.x;
	across.y   = perp.y;
	across.z   = perp.z;
	along.x    = to.x;
	along.y    = to.y;
	along.z    = to.z;
	if (dot >= 0.0f) {
		gjScaleVector(dot + ahead, &along, &along);
	} else {
		gjScaleVector(dot + behind, &along, &along);
	}
	gjScaleVector(side, &across, &across);
	njAddVector(&along, &across);
	speed = njUnitVector(&along);
	if (speed < gUnitVectorMin) {
		along = gVecZero;
	}
	if (speed > maxSpeed) {
		speed = maxSpeed;
	}
	gjScaleVector(speed, &along, &along);
	mXform.mVel.x = along.x;
	mXform.mVel.y = along.y;
	mXform.mVel.z = along.z;
	return dist;
}

// 0x8015DD4C
void TEneLastBossFeather::dirToRot(const Vec3f* dir, SetRot* rot)
{
	Vec3f d = *dir;
	f32 yaw;
	f32 pitch;

	yaw   = atan2(d.x, d.z);
	f32 x = d.x;
	f32 z = d.z;
	x *= x;
	z *= z;
	z += x;
	f32 r  = (f32)__frsqrte(z);
	pitch  = atan2(__fres(r), -d.y);
	rot->x = -(int)(10430.38043493439 * pitch);
	rot->y = (int)(10430.38043493439 * yaw);
}

#include "enemy/boss/EneBoss.h"
#include "enemy/EnemyAnim.h"
#include "enemy/boss/TEneLastBossDarkCtrl.h"
#include "egg/EggCreature.h"
#include "sound/SeHolder.h"
#include "enemy/ObjEnemy.h"
#include "Ninja/ninja.h"
#include "enemy/boss/TEneLastBossFeather_Home.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "object/last/TObjLastEggLight.h"

extern "C" {
}

typedef char TEneLastBossFeather_Dark_size_check[(sizeof(TEneLastBossFeather_Dark) == 0x20C) ? 1 : -1];

const char* gTEneLastBossFeather_DarkName = "TEneLastBossFeather_Dark";

// 0x8015DE0C
TEneLastBossFeather_Dark::TEneLastBossFeather_Dark(TObject* parent, const Vec3f* pos, int rotY, const f32* mtx)
    : TEneLastBossFeather(parent)
{
	_00         = (u32)gTEneLastBossFeather_DarkName;
	mSize       = 0x20C;
	mXform.mPos = *pos;
	orient(rotY, mtx);
	mState     = 0;
	_1F8       = 0;
	mAim       = mXform.mPos;
	mAimRotY   = mXform.mRot.y;
	mTrailKind = 0x45;
	mGlowKind  = 0xF;
	mTarget.x  = 0.0f;
	mTarget.y  = 20.0f;
	mTarget.z  = 0.0f;
	mMapRadius = 50.0f;
	updateColli();
	updateShadow();
}

// 0x8015DEF4
TEneLastBossFeather_Dark::~TEneLastBossFeather_Dark()
{
}

// 0x8015DF50
int TEneLastBossFeather_Dark::onHit(const TObjAction*, int, int*)
{
	return 0;
}

// 0x8015DF58
// Throws the feather along dir
void TEneLastBossFeather_Dark::launch(const Vec3f* dir, const Vec3f* target, f32 radius)
{
	mState = 1;
	motionCtrlPlay(&mClip, 1);
	mXform.mRot.x = -0x4000;
	mXform.mVel.x = dir->x;
	mXform.mVel.y = dir->y;
	mXform.mVel.z = dir->z;
	gjScaleVector(5.0f, &mXform.mVel, &mXform.mVel);
	mXform.mVel.y += 2.0f;
	mTarget.x  = target->x;
	mTarget.y  = target->y;
	mTarget.z  = target->z;
	mMapRadius = radius;
}

// 0x8015E00C
void TEneLastBossFeather_Dark::draw()
{
	f32 dist;

	switch (mState) {
	case 0:
		motionCtrlUpdate(&mClip);
		trackNodes();
		approachAim();
		turnToAim();
		calcPoints();
		updateColli();
		updateShadow();
		break;
	case 1:
		motionCtrlUpdate(&mClip);
		trackNodes();
		dist = steer(0.18f, 0.36f, 0.9f, 7.0f, &mTarget);
		dirToRot(&mXform.mVel, &mXform.mRot);
		njAddVector(&mXform.mPos, &mXform.mVel);
		if (mXform.mPos.y < 22.0f) {
			mXform.mPos.y = 22.0f;
			mXform.mVel.y *= -0.2f;
		}
		if (dist <= 5.0f) {
			darkCtrlAddMap(mMapRadius, &mTarget, 0x78);
			createLastBossMagic(mParent, &mTarget, mMapRadius);
			seStartAtFar(0x192, &mXform.mPos);
			mFlags |= 1;
		}
		calcPoints();
		emitTrail();
		updateColli();
		updateShadow();
		seHolderPlay(&mRing, 0x191, &mXform.mPos);
		break;
	}
}

// 0x8015E180
void TEneLastBossFeather_Dark::vfn_10()
{
	if (cameraIsVisible(2500.0f, 30.0f, &mXform.mPos, 0)) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x8015E1E0
void TEneLastBossFeather_Dark::render()
{
	njSetTexture(gLastBossFeatherTexList);
	njPushMatrixEx();
	applyMatrix();
	drawModelLit(mAnim);
	njPopMatrixEx();
}

#include "enemy/boss/EneBoss.h"
#include "enemy/EnemyAnim.h"
#include "engine/CharaColli.h"
#include "enemy/boss/TEneLastBossMagic.h"
#include "egg/EggCreature.h"
#include "sound/SeHolder.h"
#include "enemy/ObjEnemy.h"
#include "enemy/boss/TEneLastBoss.h"
#include "animal/AniMount.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "object/ObjShadow.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "enemy/boss/TEneLastBossFeather_Home.h"
#include "object/last/TObjLastEggLight.h"

const char* gTEneLastBossFeather_HomeName = "TEneLastBossFeather_Home";

typedef char TEneLastBossFeather_Home_size_check[(sizeof(TEneLastBossFeather_Home) == 0x264) ? 1 : -1];

// 0x8015E228
TEneLastBossFeather_Home::TEneLastBossFeather_Home(TObject* parent, const Vec3f* pos, int rotY, const f32* mtx)
    : TEneLastBossFeather(parent)
{
	EnemySight* sight;

	_00         = (u32)gTEneLastBossFeather_HomeName;
	mSize       = 0x264;
	mXform.mPos = *pos;
	orient(rotY, mtx);
	mState  = HOME_HOVER;
	mTimer  = 0;
	mRadius = 5.0f;
	sight   = &mSight;
	sight->reset();
	mSight.mHeightLimit = 1000.0f;
	mAim                = mXform.mPos;
	mAimRotY            = mXform.mRot.y;
	mTrailKind          = 0x46;
	mGlowKind           = 0xE;
	updateColli();
	updateShadow();
}

// 0x8015E328
TEneLastBossFeather_Home::~TEneLastBossFeather_Home()
{
	mBlast.stop();
}

// 0x8015E3A8
// Throws the feather along dir
void TEneLastBossFeather_Home::launch(const Vec3f* dir)
{
	mState = HOME_CHASE;
	motionCtrlPlay(&mClip, 1);
	mTimer        = featherDamageTEneLastBoss();
	mXform.mRot.x = -0x4000;
	mXform.mVel.x = dir->x;
	mXform.mVel.y = dir->y;
	mXform.mVel.z = dir->z;
	gjScaleVector(3.5f, &mXform.mVel, &mXform.mVel);
}

// 0x8015E420
int TEneLastBossFeather_Home::onHit(const TObjAction* other, int, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	if (mState == HOME_HOVER) {
		return 0;
	}
	int hurt = 0;
	if (isEggObj(other)) {
		hurt = 1;
		hit->mFlags.clear();
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 1;
		hit->mFlags.setBit(0xD);
	} else if (isBursting() && (isPlayerOrKind4(other) || isEnemyObj(other))) {
		hurt = 1;
		hit->mFlags.clear();
		hit->mPos.x  = 0.0f;
		hit->mPos.y  = 0.0f;
		hit->mPos.z  = 0.0f;
		hit->mDamage = 10;
	}
	return hurt;
}

// 0x8015E504
void TEneLastBossFeather_Home::move()
{
	njAddVector(&mXform.mPos, &mXform.mVel);
	if (mXform.mPos.y < 23.0f) {
		mXform.mPos.y = 23.0f;
		mXform.mVel.y *= -0.8f;
	}
}

// 0x8015E55C
void TEneLastBossFeather_Home::slowAndMove()
{
	gjScaleVector(0.7f, &mXform.mVel, &mXform.mVel);
	move();
}

// 0x8015E59C
void TEneLastBossFeather_Home::placeBlast()
{
	mBlast.mMtx.setPosRot(&mXform.mPos, -0x4000, 0);
}

// 0x8015E5D0
int TEneLastBossFeather_Home::isBursting()
{
	bool bursting = false;
	if (mState != HOME_CHASE && mState != HOME_HOVER) {
		bursting = true;
	}
	return bursting;
}

// 0x8015E5F4
void TEneLastBossFeather_Home::draw()
{
	switch (mState) {
	case HOME_HOVER:
		motionCtrlUpdate(&mClip);
		trackNodes();
		approachAim();
		turnToAim();
		calcPoints();
		updateColli();
		updateShadow();
		break;
	case HOME_CHASE: {
		motionCtrlUpdate(&mClip);
		trackNodes();
		mSight.findInRange(100000000.0f, &mXform.mPos);
		if (steer(0.2f, 0.06f, 0.98f, 4.5f, &mSight.mTarget) < 10.0f) {
			mTimer = 0;
		}
		move();
		mXform.mRot.y = angleApproach(mXform.mRot.y, mSight.mAngle, 0x300);
		for (ColliContact* contact = ((CharaColli*)mShape)->mContacts; contact != NULL; contact = contact->mNext) {
			if (colliContactNoBit10(contact)) {
				mTimer = 0;
				break;
			}
		}
		calcPoints();
		emitTrail();
		updateColli();
		updateShadow();
		seHolderPlay(&mRing, 0x191, &mXform.mPos);
		mTimer--;
		if (mTimer <= 0) {
			mTimer  = 9;
			mState  = HOME_BURST;
			mRadius = 5.0f;
			colliShapeSetRadius(colliGetShapes(mShape), mRadius);
			stopTrail();
			shadowDisable(&mShadow);
			mBlast.start(5, 0x16);
			placeBlast();
			seStartAtFar(0x196, &mXform.mPos);
			seHolderStop(&mRing);
		}
		break;
	}
	case HOME_BURST:
		mTimer--;
		if (mTimer <= 0) {
			mState = HOME_GROW;
		}
		slowAndMove();
		calcPoints();
		updateColli();
		placeBlast();
		break;
	case HOME_GROW:
		mRadius += 2.0f;
		if (mRadius >= 35.0f) {
			mRadius = 35.0f;
			mState  = HOME_FADE;
			mTimer  = 72;
			colliDisable((CharaColli*)mShape);
		}
		colliShapeSetRadius(colliGetShapes(mShape), mRadius);
		slowAndMove();
		calcPoints();
		updateColli();
		placeBlast();
		break;
	case HOME_FADE:
		mTimer--;
		if (mTimer <= 0) {
			mFlags |= 1;
		}
		slowAndMove();
		calcPoints();
		placeBlast();
		break;
	}
}

// 0x8015E8D8
// Hidden while off camera
void TEneLastBossFeather_Home::vfn_10()
{
	f32 radius;
	if (isBursting()) {
		radius = 1.5f * mRadius;
	} else {
		radius = 30.0f;
	}
	if (cameraIsVisible(2500.0f, radius, &mXform.mPos, NULL)) {
		mFlags &= ~0x10;
	} else {
		mFlags |= 0x10;
	}
}

// 0x8015E954
// Once it has burst only the effect is drawn
void TEneLastBossFeather_Home::render()
{
	if (!isBursting()) {
		njSetTexture(gLastBossFeatherTexList);
		njPushMatrixEx();
		applyMatrix();
		drawModelLit(mAnim);
		njPopMatrixEx();
	}
}

// 0x8015E9A8
int TEneLastBossFeather::getKind()
{
	return 2;
}

// 0x8015E9B0
Vec3f* TEneLastBossFeather::getPos()
{
	return &mXform.mPos;
}

// 0x8015E9B8
extern "C" void createLastBossMagic(TObject* parent, const Vec3f* pos, f32 radius)
{
	new TEneLastBossMagic(parent, pos, radius);
}
