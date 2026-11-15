#include "enemy/boss/TEneYellowBossCrystal.h"
#include "object/blue/TObjBlueCaptain.h"
#include "engine/CharaColli.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "enemy/EnemyFx.h"
#include "object/ObjShadow.h"
#include "enemy/ObjEnemy.h"
#include "engine/MotionPose.h"
#include "ui/AeUtil.h"
#include "enemy/EnemyAnim.h"
#include "enemy/boss/TEneBoss03_Beam_Pera.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyTexFx.h"
#include "engine/Colli.h"
#include "engine/DrawTask.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "stl/math.h"
#include "enemy/boss/TEneYellowBossRed.h"
struct GjActor;

extern "C" {
const char* gTEneYellowBossRedName      = "TEneYellowBossRed";
const int gYellowBossRedNodes[2]        = { 2, -1 };
MotionEntry gYellowBossRedMotionTable[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 44.0f, 0.6f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 20.0f, 0.3f, 0.0f, 0 },
};
}

typedef char TEneYellowBossRed_size_check[(sizeof(TEneYellowBossRed) == 0x208) ? 1 : -1];

// 0x8016D914
// From the yellow boss's load
extern "C" void yellowBossRingInit()
{
	CaptainRing desc;
	desc.mA     = 0.0f;
	desc.mB     = 0.0f;
	desc.mX     = 50.02f;
	desc.mY     = 20.02f;
	desc.mZ     = 230.0f;
	desc.mFlag0 = 0;
	desc.mFlag1 = 1;
	desc.mKind  = 0x46;
	desc.mId    = 0x46;
	buildCaptainRing(&desc);
}

// 0x8016D978
extern "C" void yellowBossRingExit()
{
	freeCaptainRing();
}

// 0x8016D998
TEneYellowBossRed::TEneYellowBossRed(TObject* parent, const Vec3f* pos)
    : TEneYellowBossCrystal(parent)
{
	_00         = (u32)gTEneYellowBossRedName;
	mSize       = 0x208;
	mModel      = modelTEneYellowBoss(2);
	mMotions    = motionTEneYellowBoss(6);
	mXform.mPos = *pos;
	motionCtrlInit(&mClip, gYellowBossRedMotionTable, mMotions);
	motionCtrlPlay(&mClip, 1);
	mAnim  = (GjActor*)actorCreate(mModel);
	mShape = (CharaColli*)colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 2, 255, 0, 6);
	colliShapeMakeSphere(colliGetShapes(mShape), &mXform.mPos, 1, 60.0f);
	colliShapeSetTag(mShape, 0, 0);
	colliShapeSetBit8(mShape, 0);
	colliEnable(mShape);
	shadowSetGround(&mShadow, &mXform.mPos, 0);
	mShadow._30 = 108.0f;
	mShadow._34 = 108.0f;
	shadowEnable(&mShadow);
	mPoint[0].x = 0.0f;
	mPoint[0].y = 0.0f;
	mPoint[0].z = 0.0f;
	mDraw       = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mRedFlags   = 0;
	mShots      = 0;
	mHeading    = 0;
	mGoal.x     = 0.0f;
	mGoal.y     = 0.0f;
	mGoal.z     = 0.0f;
	mStep       = 0;
	mTimer      = 0;
	mTime       = 0.0f;
	startAction(0);
}

// 0x8016DB58
TEneYellowBossRed::~TEneYellowBossRed()
{
	removeDrawTask(mDraw);
	colliRetire(mShape);
	mShape = 0;
	actorFree((void**)&mAnim);
}

// 0x8016DBD8
u8 TEneYellowBossRed::isBroken()
{
	return mRedFlags & 1;
}

// 0x8016DBE4
int TEneYellowBossRed::onHit(const TObjAction* other, int arg, int* result)
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

// 0x8016DC70
f32 TEneYellowBossRed::vfn_40()
{
	return 150.0f;
}

// 0x8016DC78
void TEneYellowBossRed::pose()
{
	nodeTrackBeginOrigins(gYellowBossRedNodes, mPoint);
	gjPushUnitMatrix();
	actorApplyMotion(mAnim, &mClip);
	njPopMatrixEx();
	nodeTrackEnd();
}

// 0x8016DCC4
void TEneYellowBossRed::calcPoint()
{
	gjPushUnitMatrix();
	bodyMatrix();
	njCalcPointEx(mPoint, mPoint);
	njPopMatrixEx();
}

// 0x8016DD08
// The sphere sits above the body's origin
void TEneYellowBossRed::syncShape()
{
	f32 x = mXform.mPos.x;
	f32 y = mXform.mPos.y;
	f32 z = mXform.mPos.z;
	y += 65.0f;
	ColliShape* shape = colliGetShapes(mShape);

	shape->mPos.x = x;
	shape->mPos.y = y;
	shape->mPos.z = z;
}

// 0x8016DD7C
void TEneYellowBossRed::bodyMatrix()
{
	njTranslateEx(&mXform.mPos);
	gjRotateY(mXform.mRot.y);
	gjRotateX(mXform.mRot.x);
}

// 0x8016DDBC
// The shadow stays on the floor under it
void TEneYellowBossRed::dropShadow()
{
	Vec3f at;

	at.x = mXform.mPos.x;
	at.y = mXform.mPos.y;
	at.z = mXform.mPos.z;
	at.y = 50.02f;
	shadowSetGround(&mShadow, &at, 0);
}

// 0x8016DE0C
void TEneYellowBossRed::draw()
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

// 0x8016DE8C
void TEneYellowBossRed::runAction(int action, int phase)
{
	switch (action) {
	case 0:
		actBreak(phase);
		break;
	case 1:
		actMove(phase);
		break;
	case 2:
		actShoot(phase);
		break;
	case 3:
		actEnd(phase);
		break;
	case 4:
		actDie(phase);
		break;
	}
}

// 0x8016DF0C
// Comes out of the boss's crystal facing the middle
void TEneYellowBossRed::actBreak(int phase)
{
	int* step = &mStep;
	f32* time = &mTime;
	Vec3f dir;
	int angle;

	switch (phase) {
	case 0:
		dir.x = 0.0f;
		dir.z = 0.0f;
		njSubVector(&dir, &mXform.mPos);
		angle         = (int)(10430.38043493439 * (f32)atan2(dir.x, dir.z));
		mXform.mRot.y = angle;
		mHeading      = angle + 0x8000;
		motionCtrlPlay(&mClip, 1);
		motionCtrlUpdate(&mClip);
		pose();
		calcPoint();
		*step = crystalBreakFx(60.0f, &mXform.mPos);
		*time = startSink(130.0f, -20.0f, 10.0f);
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

// 0x8016E054
// Floats to its point on the ring then hangs for a second
void TEneYellowBossRed::actMove(int phase)
{
	Vec3f to;

	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mClip, 1);
		vecFromAngleXZ(320.0f, &mGoal, mHeading);
		mGoal.y       = 90.02000427246094f;
		mXform.mVel.x = 0.0f;
		mXform.mVel.y = 0.0f;
		mXform.mVel.z = 0.0f;
		mStep         = 0;
		break;
	case 1:
		if (mStep == 2) {
			setNext(2);
		}
		break;
	case 2:
		motionCtrlUpdate(&mClip);
		pose();
		if (mStep == 0) {
			to.x = mGoal.x;
			to.y = mGoal.y;
			to.z = mGoal.z;
			njSubVector(&to, &mXform.mPos);
			if (springVelocity(0.6f, 0.95f, &mXform.mVel, 0.2f, &to, 0.4f, 15.0f, 0.1f) <= 0.05f) {
				mTimer = 60;
				mStep  = 1;
			}
			njAddVector(&mXform.mPos, &mXform.mVel);
			to.x = 0.0f;
			to.z = 0.0f;
			njSubVector(&to, &mXform.mPos);
			mXform.mRot.y = (int)(10430.38043493439 * (f32)atan2(to.x, to.z));
			dropShadow();
		} else {
			mTimer--;
			if (mTimer <= 0) {
				mStep = 2;
			}
		}
		calcPoint();
		break;
	case 3:
		break;
	}
}

// 0x8016E1FC
// Fires a beam after the third it is done else it moves on a third of the ring
void TEneYellowBossRed::actShoot(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mClip, 0);
		break;
	case 1:
		if (motionCtrlIsDone(&mClip)) {
			mShots++;
			if (mShots >= 3) {
				mRedFlags |= 1;
			} else {
				mHeading += 0x5555;
				setNext(1);
			}
		}
		break;
	case 2:
		motionCtrlUpdate(&mClip);
		pose();
		calcPoint();
		if (motionCtrlPassed(15.0f, &mClip)) {
			createTEneBoss03_Beam_Pera(mParent, mPoint);
			seStartAtRange(0x169, mPoint, 0x78, 1200.0f);
		}
		break;
	case 3:
		break;
	}
}

// 0x8016E2F8
void TEneYellowBossRed::actEnd(int phase)
{
	switch (phase) {
	case 0:
		motionCtrlPlayBlend(0.2f, &mClip, 1);
		break;
	case 1:
		if (motionCtrlIsDone(&mClip)) {
			mRedFlags |= 1;
		}
		break;
	case 2:
		motionCtrlUpdate(&mClip);
		pose();
		calcPoint();
		break;
	case 3:
		break;
	}
}

// 0x8016E38C
void TEneYellowBossRed::actDie(int phase)
{
	switch (phase) {
	case 0:
		startSink(130.0f, -20.0f, 10.0f);
		colliDisable(mShape);
		mRedFlags |= 8;
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

// 0x8016E43C
void TEneYellowBossRed::vfn_10()
{
	cull(mDraw, 3000.0f, 100.0f, &mXform.mPos, 0);
}

// 0x8016E470
void TEneYellowBossRed::render()
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
