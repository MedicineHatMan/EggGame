#include "enemy/boss/TEneYellowBossCrystal.h"
#include "enemy/boss/TEnePurpleBoss.h"
#include "engine/CharaColli.h"
#include "sound/SeHolder.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "enemy/EnemyFx.h"
#include "object/ObjShadow.h"
#include "enemy/ObjEnemy.h"
#include "ui/AeUtil.h"
#include "core/MathUtil.h"
#include "enemy/EnemyAnim.h"
#include "enemy/EnemyTexFx.h"
#include "enemy/EnemyGround.h"
#include "engine/MotionPose.h"
#include "object/ObjAction.h"
#include "engine/Colli.h"
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "stl/math.h"
#include "Ninja/ninja.h"
#include "enemy/EnemyHit.h"
#include "enemy/boss/TEneYellowBossPurple.h"
struct Vec3f;
struct GjActor;

extern "C" {
MotionEntry sYellowBossPurpleMotions[] = {
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 35.0f, 0.4f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 59.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 30.0f, 1.4f, 0.0f, 0 },
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 59.0f, 0.4f, 0.0f, 0 },
};
}

// A message an object sends to what it touched

typedef char TEneYellowBossPurple_size_check[(sizeof(TEneYellowBossPurple) == 0x264) ? 1 : -1];

// 0x8014E4B8
TEneYellowBossPurple::TEneYellowBossPurple(TObject* parent, const Vec3f* pos)
    : TEneYellowBossCrystal(parent)
{
	int i;

	_00            = gTEneYellowBossPurpleName;
	mSize          = 0x264;
	mShape         = 0;
	mModel         = modelTEneYellowBoss(6);
	mMotions       = motionTEneYellowBoss(5);
	mXform.mPos    = *pos;
	motionCtrlInit(&mClip, sYellowBossPurpleMotions, mMotions);
	motionCtrlPlay(&mClip, 0);
	mAnim  = (GjActor*)actorCreate(mModel);
	mShape = (CharaColli*)colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 2, 255, 0, 0x10);
	colliShapeMakeSphere(colliGetShape(mShape, 0), &mXform.mPos, 0, 25.0f);
	colliShapeSetTag(mShape, 0, 0);
	colliShapeSetBit8(mShape, 0);
	shadowSetGround(&mShadow, &mXform.mPos, 0);
	mShadow._30 = 50.0f;
	mShadow._34 = 50.0f;
	shadowEnable(&mShadow);
	mHits = 0;
	for (i = 0; i < 6; i++) {
		mPoint[i].x = 0.0f;
		mPoint[i].y = 0.0f;
		mPoint[i].z = 0.0f;
	}
	mDraw        = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mPurpleFlags = 0;
	seHolderStop(&mSe);
	mStep = 0;
	mTime = 0.0f;
	startAction(0);
}

// 0x8014E69C
TEneYellowBossPurple::~TEneYellowBossPurple()
{
	seHolderStop(&mSe);
	removeDrawTask(mDraw);
	colliRetire(mShape);
	mShape = 0;
	actorFree((void**)&mAnim);
}

// 0x8014E730
u8 TEneYellowBossPurple::isBroken() { return mHits >= 3; }

// 0x8014E750
int TEneYellowBossPurple::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int ret = 0;

	if (isPlayerOrKind4(other)) {
		hit->mDamage = 0x14;
		ret = 1;
		hit->mFlags.clear();
	} else if (isEggObj(other) && (mPurpleFlags & 1)) {
		hit->mDamage = 1;
		ret = 1;
		hit->mFlags.clear();
		hit->mFlags.setBit(0xD);
	}
	return ret;
}

// 0x8014E7EC
f32 TEneYellowBossPurple::vfn_40() { return 80.0f; }

// 0x8014E7F4
void TEneYellowBossPurple::pose()
{
	nodeTrackBeginOrigins((const int*)paramsTEnePurpleBoss(), (Vec3f*)mPoint);
	gjPushUnitMatrix();
	actorApplyMotion(mAnim, &mClip);
	njPopMatrixEx();
	nodeTrackEnd();
}

// 0x8014E840
void TEneYellowBossPurple::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 6; i++) {
		njCalcPointEx(&mPoint[i], &mPoint[i]);
	}
	njPopMatrixEx();
}

// 0x8014E8A0
void TEneYellowBossPurple::syncShape()
{
	ColliShape* shape = colliGetShapes(mShape);

	shape->mPos.x = mPoint[0].x;
	shape->mPos.y = mPoint[0].y;
	shape->mPos.z = mPoint[0].z;
}

// 0x8014E8E8
void TEneYellowBossPurple::bodyMatrix()
{
	njTranslateEx(&mXform.mPos);
	gjRotateY(mXform.mRot.y);
	gjRotateX(mXform.mRot.x);
}

// 0x8014E928
void TEneYellowBossPurple::gravity(Vec3f* vel) { TEnePurpleBoss::gravity(vel); }

// 0x8014E94C
// Returns whether it hit a wall
int TEneYellowBossPurple::move(Vec3f* vel)
{
	Vec3f step;
	u32 hit;

	colliFlatPush(mShape, &step);
	step.y = 0.0f;
	njAddVector(&step, vel);
	hit = moveBodyStep(25.0f, 10.0f, &mXform.mPos, &step, &mShadow);
	if (!(hit & 2)) {
		vel->y = 0.0f;
	}
	return hit & 4;
}

// 0x8014E9C4
int TEneYellowBossPurple::fall(Vec3f* vel)
{
	gravity(vel);
	return move(vel);
}

// 0x8014EA00
void TEneYellowBossPurple::slide(f32 drag)
{
	gravity(&mXform.mVel);
	mXform.mVel.x *= drag;
	mXform.mVel.z *= drag;
	if (move(&mXform.mVel)) {
		mXform.mVel.x = 0.0f;
		mXform.mVel.z = 0.0f;
	}
}

// 0x8014EA7C
void TEneYellowBossPurple::draw()
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
}

// 0x8014EAFC
void TEneYellowBossPurple::runAction(int action, int phase)
{
	switch (action) {
	case 0:
		actBreak(phase);
		break;
	case 1:
		actTurn(phase);
		break;
	case 2:
		actSkate(phase);
		break;
	case 3:
		actBrake(phase);
		break;
	case 4:
		actDie(phase);
		break;
	}
}

// 0x8014EB7C
// Comes out of the boss's crystal and faces the middle
void TEneYellowBossPurple::actBreak(int phase)
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
		*step = crystalBreakFx(25.0f, &mXform.mPos);
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

// 0x8014ECB8
void TEneYellowBossPurple::actTurn(int phase)
{
	f32* rate = &mTime;
	int angle;

	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.1f, &mClip, 0);
		mStep = 0;
		*rate = 0.0f;
		break;
	case 1:
		if (mStep == 1) {
			setNext(2);
		}
		break;
	case 2:
		look();
		motionCtrlUpdate(&mClip);
		pose();
		angle = mXform.mRot.y;
		if (TEnePurpleBoss::turnStep(motionCtrlFrame(&mClip), &angle, rate, mLookAngle)) {
			mStep = 1;
		}
		mXform.mRot.y = angle;
		calcPoints();
		break;
	case 3:
		break;
	}
}

// 0x8014ED94
void TEneYellowBossPurple::actSkate(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mClip, 1);
		mPurpleFlags &= ~1;
		seHolderStop(&mSe);
		mStep = 0;
		break;
	case 1:
		if (mStep == 3 || mStep == 2) {
			Vec3f at;
			at.x = mXform.mPos.x;
			at.y = mXform.mPos.y;
			at.z = mXform.mPos.z;
			if (vecDotXZ(&at, &mXform.mVel) >= 1.0f && vecLenXZ(&at) >= 30.0f) {
				setNext(3);
			}
		}
		break;
	case 2:
		look();
		motionCtrlUpdate(&mClip);
		pose();
		switch (mStep) {
		case 0:
			if (TEnePurpleBoss::pushOff(&mXform, &mClip, mLookAngle)) {
				mStep = 1;
				fall(&mXform.mVel);
				mPurpleFlags |= 1;
			} else {
				slide(0.8f);
			}
		case 1:
			if (TEnePurpleBoss::pastLaunch(&mClip)) {
				mStep = 2;
				seStartAtFar(0x16A, &mXform.mPos);
			}
			break;
		case 2:
			if (motionCtrlIsDone(&mClip)) {
				motionCtrlPlay(&mClip, 2);
				mStep = 3;
			}
		case 3:
			fall(&mXform.mVel);
			seHolderPlay(&mSe, 0x16C, &mXform.mPos);
			break;
		}
		calcPoints();
		break;
	case 3:
		mPurpleFlags &= ~1;
		seHolderStop(&mSe);
		break;
	}
}

// 0x8014EF94
void TEneYellowBossPurple::actBrake(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mClip, 3);
		mPurpleFlags |= 1;
		seStartAtFar(0x16D, &mXform.mPos);
		break;
	case 1:
		if (motionCtrlIsDone(&mClip)) {
			mHits++;
			setNext(1);
		}
		break;
	case 2:
		look();
		motionCtrlUpdate(&mClip);
		pose();
		slide(TEnePurpleBoss::brakeRate(motionCtrlFrame(&mClip)));
		calcPoints();
		break;
	case 3:
		mPurpleFlags &= ~1;
		break;
	}
}

// 0x8014F07C
void TEneYellowBossPurple::actDie(int phase)
{
	switch (phase) {
	case 0:
		startSink(80.0f, 0.0f, 10.0f);
		colliDisable(mShape);
		mPurpleFlags |= 8;
		mTime = -1.0f;
		break;
	case 2:
		mTime += 1.0f;
		bossSuitUpdateNoFx(mTime, &mSuit);
		if (mTime >= 60.0f) {
			mFlags |= 1;
		}
		break;
	case 3:
		mPurpleFlags &= ~1;
		break;
	}
}

// 0x8014F144
void TEneYellowBossPurple::vfn_10() { cull(mDraw, 3000.0f, 40.0f, &mXform.mPos, 0); }

// 0x8014F178
void TEneYellowBossPurple::render()
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

// 0x8014F214
int TEneYellowBossCrystal::getKind() { return 2; }

// 0x8014F21C
Vec3f* TEneYellowBossCrystal::getPos() { return &mXform.mPos; }

// 0x8014F224
int TEneYellowBossCrystal::canAct() { return !(mCrystalFlags & 8); }
