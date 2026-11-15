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
struct Vec3f;
struct CharaColli;
struct GjActor;

extern "C" {
MotionEntry gYellowBossOrangeMotionTable[] = {
	{ MOTION_LOOP, 0, -1, 0, 0.0f, 20.0f, 0.5f, 0.0f, 0 },
};
}

typedef char TEneYellowBossOrange_size_check[(sizeof(TEneYellowBossOrange) == 0x2B4) ? 1 : -1];

// 0x8014FC18
TEneYellowBossOrange::TEneYellowBossOrange(TObject* parent, const Vec3f* pos)
    : TEneYellowBossCrystal(parent)
{
	int i;

	_00         = gTEneYellowBossOrangeName;
	mSize       = 0x2B4;
	mShape      = 0;
	mModel      = modelTEneYellowBoss(4);
	mMotions    = motionTEneYellowBoss(4);
	mXform.mPos = *pos;
	motionCtrlInit(&mClip, gYellowBossOrangeMotionTable, mMotions);
	motionCtrlPlay(&mClip, 0);
	mAnim  = (GjActor*)actorCreate(mModel);
	mShape = (CharaColli*)orangeBossMakeCollider(mObjId);
	orangeBossColliderBit8((CharaColli*)mShape);
	shadowSetGround(&mShadow, &mXform.mPos, 0);
	mShadow._30 = 30.0f;
	mShadow._34 = 30.0f;
	shadowEnable(&mShadow);
	for (i = 0; i < 16; i++) {
		mPoint[i].x = 0.0f;
		mPoint[i].y = 0.0f;
		mPoint[i].z = 0.0f;
	}
	mDraw        = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mOrangeFlags = 0;
	mTama        = 0;
	mSpeed       = 1.0f;
	mThrows      = 0;
	mStep        = 0;
	mHandStep    = 0;
	mTime        = 0.0f;
	startAction(0);
}

// 0x8014FDA0
TEneYellowBossOrange::~TEneYellowBossOrange()
{
	if (mTama != 0) {
		mTama->mFlags |= 1;
		mTama = 0;
	}
	removeDrawTask(mDraw);
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	actorFree((void**)&mAnim);
}

// 0x8014FE40
u8 TEneYellowBossOrange::isBroken()
{
	return mOrangeFlags & 1;
}

// 0x8014FE4C
int TEneYellowBossOrange::onHit(const TObjAction* other, int arg, int* result)
{
	HitInfo* hit = (HitInfo*)result;
	int ret      = 0;

	if (isPlayerOrKind4(other)) {
		hit->mDamage = 0x14;
		ret          = 1;
		hit->mFlags.clear();
	} else if (isEggObj(other)) {
		hit->mDamage = 1;
		ret          = 1;
		hit->mFlags.clear();
		hit->mFlags.setBit(0xD);
	}
	return ret;
}

// 0x8014FED8
f32 TEneYellowBossOrange::vfn_40()
{
	return 80.0f;
}

// 0x8014FEE0
void TEneYellowBossOrange::pose()
{
	nodeTrackBeginOrigins((const int*)paramsTEneOrangeBoss(), (Vec3f*)mPoint);
	gjPushUnitMatrix();
	TEneOrangeBoss::localMatrix();
	actorApplyMotion(mAnim, &mClip);
	njPopMatrixEx();
	nodeTrackEnd();
}

// 0x8014FF30
void TEneYellowBossOrange::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 16; i++) {
		njCalcPointEx(&mPoint[i], &mPoint[i]);
	}
	njPopMatrixEx();
}

// 0x8014FF90
void TEneYellowBossOrange::syncShape()
{
	orangeBossFitCollider(mShape, mPoint);
}

// 0x8014FFBC
void TEneYellowBossOrange::bodyMatrix()
{
	njTranslateEx(&mXform.mPos);
	gjRotateY(mXform.mRot.y);
	gjRotateX(mXform.mRot.x);
}

// 0x8014FFFC
void TEneYellowBossOrange::walkFoot(int foot, f32 scale, Vec3f* vel)
{
	Vec3f before;
	Vec3f after;

	before.x = mPoint[foot].x;
	before.y = mPoint[foot].y;
	before.z = mPoint[foot].z;
	pose();
	gjPushUnitMatrix();
	bodyMatrix();
	njCalcPointEx(&mPoint[foot], &after);
	njPopMatrixEx();
	flatSteer(scale, &before, &after, vel);
	vel->y += -0.12f;
}

// 0x801500A4
// Turns the walk into a step round the ring easing the radius back to 160
void TEneYellowBossOrange::keepOnRing()
{
	Vec3f vel;
	Vec3f pos;
	int next;
	int angle;
	f32 len;
	f32 off;
	f32 radius;

	pos.x = mXform.mPos.x;
	pos.y = mXform.mPos.y;
	pos.z = mXform.mPos.z;
	angle = (int)(10430.38043493439 * (f32)atan2(pos.x, pos.z));
	len   = vecLenXZ(&pos);
	off   = len - 160.0f;
	if ((f32)__fabs(off) <= 0.2f) {
		radius = 160.0f;
	} else if (off > 0.0f) {
		radius = len - 0.2f;
	} else {
		radius = len;
		radius += 0.2f;
	}
	vel.x = mXform.mVel.x;
	vel.y = mXform.mVel.y;
	vel.z = mXform.mVel.z;
	next  = (int)(10430.38043493439 * (vecLenXZ(&vel) / radius));
	next += angle;
	vecFromAngleXZ(radius, &mXform.mPos, next);
	mXform.mPos.y = 50.02f;
	mXform.mRot.y = next + 0x4000;
	dropShadow();
}

// 0x801501BC
void TEneYellowBossOrange::dropShadow()
{
	Vec3f up;

	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	shadowFollowGround(1, &mShadow, &mXform.mPos, &up);
}

// 0x80150200
void TEneYellowBossOrange::draw()
{
	clearNext();
	crystalTouch((CharaColli*)mShape, &mTouch, 3.0f);
	if (mCrystalFlags & 4) {
		mCrystalFlags &= ~4;
		setNext(2);
	} else {
		checkAction();
	}
	switchAction();
	stepAction();
	syncShape();
}

// 0x80150280
void TEneYellowBossOrange::runAction(int action, int phase)
{
	switch (action) {
	case 0:
		actBreak(phase);
		break;
	case 1:
		actThrow(phase);
		break;
	case 2:
		actDie(phase);
		break;
	}
}

// 0x801502E0
// Comes out of the boss's crystal side on to the middle
void TEneYellowBossOrange::actBreak(int phase)
{
	int* step = &mStep;
	f32* time = &mTime;

	switch (phase) {
	case 0:
		mXform.mRot.y = (int)(10430.38043493439 * (f32)atan2(mXform.mPos.x, mXform.mPos.z)) + 0x4000;
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

// 0x80150408
// Walks the ring for twelve seconds quickening and throws a shot each throw of the walk
void TEneYellowBossOrange::actThrow(int phase)
{
	int* handStep = &mHandStep;
	Vec3f hand;
	Vec3f at;
	int foot;
	int prev;
	int thrown;

	switch (phase) {
	case 0:
		motionCtrlPlay(&mClip, 0);
		mOrangeFlags &= ~1;
		mStep     = 720;
		*handStep = 0;
		mTama     = 0;
		mSpeed    = 1.0f;
		break;
	case 1:
		mStep--;
		if (mStep < 0) {
			mOrangeFlags |= 1;
		}
		break;
	case 2:
		mSpeed += 0.0020833334f;
		if (mSpeed > 2.0f) {
			mSpeed = 2.0f;
		}
		motionCtrlSetSpeed(&mClip, mSpeed);
		motionCtrlUpdate(&mClip);
		foot = throwFoot(&mClip);
		if (foot >= 0) {
			walkFoot(foot, 1.0f, &mXform.mVel);
		} else {
			pose();
		}
		if (throwStepFrame(&mClip)) {
			seStartAtFar(0x16F, &mXform.mPos);
		}
		keepOnRing();
		calcPoints();
		prev   = *handStep;
		thrown = throwHand(handStep, &mClip, mPoint, &hand);
		switch (prev) {
		case 0:
			break;
		case 1:
			if (thrown) {
				mTama = new TEneYellowBossTama_Orange(mParent, &hand);
			}
			break;
		case 2:
		case 3:
			mTama->setPos(&hand);
			break;
		case 4:
			mTama->setPos(&hand);
			if (thrown) {
				mThrows++;
				if (mThrows % 3 == 0) {
					randomPointTEneYellowBoss(&at, 180.0f);
					mTama->throwAt(&at, 0);
				} else {
					look();
					mTama->throwAt(&mSight.mTarget, 1);
				}
				mTama = 0;
				seStartAtFar(0x170, &mXform.mPos);
			}
			break;
		}
		break;
	case 3:
		mOrangeFlags &= ~1;
		motionCtrlSetSpeed(&mClip, 1.0f);
		if (mTama != 0) {
			look();
			mTama->throwAt(&mSight.mTarget, 1);
			mTama = 0;
		}
		break;
	}
}

// 0x80150698
void TEneYellowBossOrange::actDie(int phase)
{
	switch (phase) {
	case 0:
		startSink(80.0f, 0.0f, 10.0f);
		colliDisable(mShape);
		mOrangeFlags |= 8;
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
		break;
	}
}

// 0x80150748
void TEneYellowBossOrange::vfn_10()
{
	cull(mDraw, 3000.0f, 40.0f, &mXform.mPos, 0);
}

// 0x8015077C
void TEneYellowBossOrange::render()
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
