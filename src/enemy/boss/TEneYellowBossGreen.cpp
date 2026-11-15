#include "enemy/boss/TEneYellowBossCrystal.h"
#include "engine/CharaColli.h"
#include "sound/SeHolder.h"
#include "engine/FxHolder.h"
#include "egg/ObjEgg.h"
#include "io/ResBank.h"
#include "io/Resource.h"
#include "enemy/EnemyFx.h"
#include "object/ObjShadow.h"
#include "enemy/ObjEnemy.h"
#include "object/ObjAction.h"
#include "core/MathUtil.h"
#include "enemy/EnemyGround.h"
#include "enemy/boss/TEneGreenBoss.h"
#include "ui/AeUtil.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyTexFx.h"
#include "Ninja/ninja.h"
#include "object/TObjKey00.h"
#include "stage/TGameState.h"
#include "engine/MotionPose.h"
#include "engine/Colli.h"
#include "engine/DrawTask.h"
#include "stl/math.h"
#include "engine/GroundQuery.h"
#include "core/Mem.h"
#include "enemy/boss/TEneYellowBossGreen.h"
struct Vec3f;
struct GjActor;

extern "C" {
NJS_OBJECT** gYellowGreenModels;       // 0x802C5A58
NJS_MOTION** gYellowGreenMotion;        // 0x802C5A5C
}

const char* gTEneYellowBossGreenName = "TEneYellowBossGreen";

// 0x801A91BC
// Loaded with the yellow boss
extern "C" void loadYellowBossGreen()
{
	resBankLoad(&gYellowGreenBank, "ar_ene_yellow_boss_green.arc");
	gYellowGreenModels = resBankGetModels(&gYellowGreenBank);
	gYellowGreenMotion = resBankGetMotion(&gYellowGreenBank, 0);
}

// 0x801A9210
extern "C" void releaseYellowBossGreen()
{
	resBankRelease(&gYellowGreenBank);
	gYellowGreenModels = 0;
	gYellowGreenMotion = 0;
}

typedef char TEneYellowBossGreen_size_check[(sizeof(TEneYellowBossGreen) == 0x2FC) ? 1 : -1];

// 0x80266D90
GreenMotion sYellowBossGreenMotions[] = {
	{ 0, 2, -1, 0, 0.0f, 40.0f, 0.4f, 0.0f, 0 },
	{ 0, 1, -1, 0, 0.0f, 20.0f, 0.6f, 0.0f, 0 },
	{ 1, 0, -1, 0, 0.0f, 29.0f, 0.3f, 0.0f, 0 },
};

// 0x801A9244
TEneYellowBossGreen::TEneYellowBossGreen(TObject* parent, const Vec3f* pos)
    : TEneYellowBossCrystal(parent)
{
	ColliShape* shape;
	int i;

	_00         = (u32)gTEneYellowBossGreenName;
	mSize       = 0x2FC;
	mShape      = 0;
	mXform.mPos = *pos;
	motionCtrlInit(&mClip, sYellowBossGreenMotions, gYellowGreenMotion);
	motionCtrlPlay(&mClip, 0);
	mAnim = (GjActor*)actorCreate(*gYellowGreenModels);
	mTrail.stop();
	mShape = (CharaColli*)colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 2, 255, 0, 0x10);
	colliShapeMakeSphere(colliGetShape(mShape, 0), &mXform.mPos, 0, 15.0f);
	mSwing = (CharaColli*)colliCreate(1, 1);
	colliSetOwner(mSwing, (u16)mObjId);
	colliSetMasks(mSwing, 2, 255, 255, 0xF6);
	shape = colliGetShapes(mSwing);
	colliShapeMakeSphere(shape, &mXform.mPos, 0, 70.0f);
	shape->mFlags |= 8;
	shape->mFlags |= 0x10;
	colliDisable(mSwing);
	colliShapeSetTag(mShape, 0, 0);
	colliShapeSetBit8(mShape, 0);
	colliShapeSetTag(mSwing, 0, 1);
	shadowSetGround(&mShadow, &mXform.mPos, 0);
	mShadow._30 = 30.0f;
	mShadow._34 = 30.0f;
	shadowEnable(&mShadow);
	mHits   = 0;
	mFrames = 0;
	mSpeed  = 1.5f;
	for (i = 0; i < 13; i++) {
		mPoint[i].x = 0.0f;
		mPoint[i].y = 0.0f;
		mPoint[i].z = 0.0f;
	}
	mDraw       = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mGreenFlags = 0;
	seHolderStop(&mSe);
	mStep = 0;
	mTime = 0.0f;
	startAction(0);
}

// 0x801A94B0
TEneYellowBossGreen::~TEneYellowBossGreen()
{
	seHolderStop(&mSe);
	removeDrawTask(mDraw);
	colliRetire(mSwing);
	mTrail.stop();
	mSwing = 0;
	colliRetire(mShape);
	mShape = 0;
	actorFree((void**)&mAnim);
}

// 0x801A9568
u8 TEneYellowBossGreen::isBroken()
{
	int broken = 0;

	if (mHits >= 3 || (mFrames >= 600 && mAction != 3)) {
		broken = 1;
	}
	return broken;
}

// 0x801A959C
// The swing tag 1 hits only in front
#pragma optimization_level 1
int TEneYellowBossGreen::onHit(const TObjAction* other, int arg, int* result)
{
	int ret = 0;
	HitInfo* hit = (HitInfo*)result;

	if (arg == 1) {
		int diff = (u16)angleToXZ(&mXform.mPos, ((TObjAction*)other)->getPos());
		diff -= (u16)mSwingAngle;
		if (diff > 0x3FFF || diff < -0x3FFF) {
			return 0;
		}
		if (isPlayerOrKind4(other)) {
			hit->mDamage = 0x14;
			ret = 1;
			hit->mFlags.clear();
		} else if (isEggObj(other)) {
			hit->mDamage = 1;
			ret = 1;
			hit->mFlags.clear();
			hit->mFlags.setBit(0xD);
		}
	} else if (isPlayerOrKind4(other)) {
		hit->mDamage = 0x14;
		ret = 1;
		hit->mFlags.clear();
	}
	return ret;
}
#pragma optimization_level reset

// 0x801A96A4
f32 TEneYellowBossGreen::vfn_40() { return 80.0f; }

// 0x801A96AC
void TEneYellowBossGreen::pose()
{
	nodeTrackBeginOrigins((const int*)paramsTEneGreenBoss(), (Vec3f*)mPoint);
	gjPushUnitMatrix();
	actorApplyMotion(mAnim, &mClip);
	njPopMatrixEx();
	nodeTrackEnd();
}

// 0x801A96F8
void TEneYellowBossGreen::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 13; i++) {
		njCalcPointEx(&mPoint[i], &mPoint[i]);
	}
	njPopMatrixEx();
}

// 0x801A9758
void TEneYellowBossGreen::syncShape()
{
	ColliShape* shape = colliGetShapes(mShape);

	shape->mPos.x = mPoint[0].x;
	shape->mPos.y = mPoint[0].y;
	shape->mPos.z = mPoint[0].z;
	colliRefreshBounds(mShape);
	colliBoundsOn(mShape);
}

// 0x801A97B0
void TEneYellowBossGreen::syncSwing()
{
	ColliShape* shape = colliGetShapes(mSwing);

	shape->mPos.x = mXform.mPos.x;
	shape->mPos.y = mXform.mPos.y;
	shape->mPos.z = mXform.mPos.z;
	colliShapeSetPos(shape, &mXform.mPos);
	colliRefreshBounds(mSwing);
	colliBoundsOn(mSwing);
}

// 0x801A9810
void TEneYellowBossGreen::bodyMatrix()
{
	njTranslateEx(&mXform.mPos);
	gjRotateY(mXform.mRot.y);
	gjRotateX(mXform.mRot.x);
}

// 0x801A9850
void TEneYellowBossGreen::groundShadow()
{
	Vec3f from;
	Vec3f pos;
	Vec3f normal;

	from.x = mXform.mPos.x;
	from.y = mXform.mPos.y;
	from.z = mXform.mPos.z;
	from.y += 8.0f;
	if (probeGround(&from, &pos, &normal)) {
		shadowSetGround(&mShadow, &pos, &normal);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
}

// 0x801A98D8
int TEneYellowBossGreen::probeGround(Vec3f* from, Vec3f* pos, Vec3f* normal)
{
	EggHits hits;
	int found = meshColliProbe((EggHit*)&hits, (const Vec3f*)from, 1);

	if (found) {
		if (pos != 0) {
			pos->x = hits.mHits[0].mPos.x;
			pos->y = hits.mHits[0].mPos.y;
			pos->z = hits.mHits[0].mPos.z;
		}
		if (normal != 0) {
			normal->x = hits.mHits[0].mNormal.x;
			normal->y = hits.mHits[0].mNormal.y;
			normal->z = hits.mHits[0].mNormal.z;
		}
	}
	return found;
}

// 0x801A9980
// The step a foot node made this frame as a velocity
void TEneYellowBossGreen::stepNode(int node, Vec3f* vel)
{
	Vec3f from;
	Vec3f to;

	from.x = mPoint[node].x;
	from.y = mPoint[node].y;
	from.z = mPoint[node].z;
	pose();
	gjPushUnitMatrix();
	bodyMatrix();
	njCalcPointEx(&mPoint[node], &to);
	njPopMatrixEx();
	flatSteer(1.0f, &from, &to, vel);
	vel->y += -0.12f;
}

// 0x801A9A14
void TEneYellowBossGreen::drag(f32 rate)
{
	mXform.mVel.x *= rate;
	mXform.mVel.z *= rate;
}

// 0x801A9A30
int TEneYellowBossGreen::move(Vec3f* vel)
{
	Vec3f step;

	step.y = 0.0f;
	step.x = 0.0f;
	step.z = 0.0f;
	step.y = 0.0f;
	njAddVector(&step, vel);
	njAddVector(&mXform.mPos, &step);
	if (mXform.mPos.y <= 50.02f) {
		mXform.mPos.y = 50.02f;
	}
	groundShadow();
	return 0;
}

// 0x801A9AA4
void TEneYellowBossGreen::slide(f32 rate)
{
	drag(rate);
	if (move(&mXform.mVel)) {
		mXform.mVel.x = 0.0f;
		mXform.mVel.z = 0.0f;
	}
}

// 0x801A9AF0
f32 TEneYellowBossGreen::motionRate(s16 entry) { return mSpeed * sYellowBossGreenMotions[entry].mRate; }

// 0x801A9B14
void TEneYellowBossGreen::draw()
{
	clearNext();
	crystalTouch(mShape, &mTouch, 3.0f);
	if (mCrystalFlags & 4) {
		mCrystalFlags &= ~4;
		setNext(4);
	} else {
		checkAction();
	}
	switchAction();
	stepAction();
	syncShape();
	mFrames++;
}

// 0x801A9BA0
void TEneYellowBossGreen::runAction(int action, int phase)
{
	switch (action) {
	case 0:
		actBreak(phase);
		break;
	case 1:
		actTurn(phase);
		break;
	case 2:
		actWalk(phase);
		break;
	case 3:
		actSwing(phase);
		break;
	case 4:
		actDie(phase);
		break;
	}
}

// 0x801A9C20
void TEneYellowBossGreen::actBreak(int phase)
{
	int* step = &mStep;
	f32* time = &mTime;
	Vec3f dir;

	switch (phase) {
	case 0:
		dir.x = 0.0f;
		dir.z = 0.0f;
		njSubVector(&dir, &mXform.mPos);
		mXform.mRot.y = (int)(10430.38043493439 * (f32)atan2(dir.x, dir.z));
		motionCtrlPlay(&mClip, 0);
		motionCtrlUpdate(&mClip);
		pose();
		calcPoints();
		*step = crystalBreakFx(15.0f, &mXform.mPos);
		*time = startSink(80.0f, 0.0f, 10.0f);
		hide();
		colliDisable(mShape);
		break;
	case 1:
		if (*time <= 0.0f) {
			setNext(1);
		}
		break;
	case 2:
		waitBreak(step, time);
		break;
	case 3:
		endSink();
		show();
		colliEnable(mShape);
		break;
	}
}

// 0x801A9D5C
// Turns *angle toward target by rate a frame scaled by mSpeed
int TEneYellowBossGreen::turnTo(int* angle, f32* step, int rate, int target)
{
	int diff  = angleDiff(*angle, target);
	f32 left  = diff;
	int there;

	if (diff >= 0) {
		*step = rate;
	} else {
		*step = -(f32)rate;
	}
	*step = *step * mSpeed;
	if (left > 0.0f) {
		there = *step > left;
	} else if (left < 0.0f) {
		there = *step < left;
	} else {
		there = 1;
	}
	if (there) {
		*step = left;
	}
	*angle += (int)*step;
	return there;
}

// 0x801A9E74
void TEneYellowBossGreen::actTurn(int phase)
{
	f32* step = &mTime;
	int angle;

	switch (phase) {
	case 0:
		look();
		motionCtrlPlayBlend(motionRate(1), &mClip, 1);
		*step = 0.0f;
		mStep = 0;
		break;
	case 1:
		if (mStep == 1) {
			setNext(2);
		}
		break;
	case 2:
		motionCtrlUpdate(&mClip);
		pose();
		angle = mXform.mRot.y;
		motionCtrlFrame(&mClip);
		if (turnTo(&angle, step, 0x666, mLookAngle)) {
			mStep = 1;
		}
		mXform.mRot.y = angle;
		slide(0.9f);
		calcPoints();
		break;
	case 3:
		break;
	}
}

// 0x801A9F6C
int TEneYellowBossGreen::isNear(Vec3f* at) { return distXZ(&mXform.mPos, at) <= 50.0f; }

// 0x801A9FA4
int TEneYellowBossGreen::isAhead(int angle)
{
	u8 ahead = 0;

	if (angle <= 0x3FFF || angle >= -0x3FFF) {
		ahead = 1;
	}
	return ahead;
}

// 0x801A9FC4
void TEneYellowBossGreen::actWalk(int phase)
{
	int node;

	switch (phase) {
	case 0:
		motionCtrlPlayBlend(motionRate(1), &mClip, 1);
		motionCtrlUpdate(&mClip);
		pose();
		calcPoints();
		mStep = 0;
		break;
	case 1:
		if (isNear(&mSight.mTarget) && isAhead(mSight.mAngle)) {
			setNext(3);
		}
		break;
	case 2:
		look();
		motionCtrlUpdate(&mClip);
		bodyFace(&mXform, &mLookPos, &mXform.mPos, 0x666);
		if (motionCtrlFrame(&mClip) < 10.0f && !motionCtrlIsBlending(&mClip)) {
			node = 8;
			if (motionCtrlPassed(4.0f, &mClip)) {
				seStartAt(0x166, &mXform.mPos);
			}
		} else {
			node = 10;
			if (motionCtrlPassed(12.0f, &mClip)) {
				seStartAt(0x166, &mXform.mPos);
			}
		}
		stepNode(node, &mXform.mVel);
		move(&mXform.mVel);
		calcPoints();
		break;
	case 3:
		break;
	}
}

// 0x801AA12C
void TEneYellowBossGreen::actSwing(int phase)
{
	f32 frame;

	switch (phase) {
	case 0:
		motionCtrlPlayBlend(motionRate(2), &mClip, 2);
		mStep       = 0;
		mSwingAngle = mXform.mRot.y;
		mTrail.stop();
		break;
	case 1:
		if (motionCtrlIsDone(&mClip)) {
			setNext(1);
		}
		break;
	case 2:
		motionCtrlUpdate(&mClip);
		pose();
		slide(0.0f);
		syncSwing();
		calcPoints();
		switch (mStep) {
		case 0:
			if (motionCtrlPassed(10.0f, &mClip)) {
				mStep = 1;
			}
			break;
		case 1:
			mXform.mRot.y += 0x666;
			if (motionCtrlPassed(16.0f, &mClip)) {
				mStep = 2;
			}
			break;
		case 2:
			break;
		}
		if (!motionCtrlIsBlending(&mClip)) {
			if (motionCtrlPassed(14.0f, &mClip)) {
				mTrail.start(5, 0x1D);
				mTrail.mMtx.setPosRot(&mXform.mPos, 0, mSwingAngle);
				seStartAt(0x165, &mXform.mPos);
			}
			frame = motionCtrlFrame(&mClip);
			if (frame >= 14.0f && frame <= 16.0f) {
				colliEnable(mSwing);
			} else {
				colliDisable(mSwing);
			}
		}
		break;
	case 3:
		mTrail.stop();
		colliDisable(mSwing);
		mHits++;
		break;
	}
}

// 0x801AA310
void TEneYellowBossGreen::actDie(int phase)
{
	switch (phase) {
	case 0:
		startSink(80.0f, 0.0f, 10.0f);
		colliDisable(mShape);
		mGreenFlags |= 8;
		mTime = -1.0f;
		motionCtrlPlay(&mClip, 0);
		break;
	case 2:
		motionCtrlUpdate(&mClip);
		pose();
		mTime += 1.0f;
		bossSuitUpdateNoFx(mTime, &mSuit);
		if (mTime >= 60.0f) {
			mFlags |= 1;
		}
		break;
	case 3:
		mGreenFlags &= ~1;
		break;
	}
}

// 0x801AA3F4
void TEneYellowBossGreen::vfn_10() { cull(mDraw, 3000.0f, 60.0f, &mXform.mPos, 0); }

// 0x801AA428
void TEneYellowBossGreen::render()
{
	crystalBeginDraw();
	njPushMatrixEx();
	bodyMatrix();
	crystalSetTex(mXform.mRot.y);
	if (isSinking()) {
		crystalSuitBegin(10.0f, &mSuit);
		shadowAlphaBegin();
		gjDrawActor(mAnim);
		shadowAlphaEnd();
	} else {
		drawLit(&mLight, 0, mAnim);
	}
	njPopMatrixEx();
	if (isSinking()) {
		texFxSuitEnd();
	}
	crystalEndDraw();
}
