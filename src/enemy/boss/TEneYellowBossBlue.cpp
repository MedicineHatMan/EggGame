#include "enemy/boss/TEneYellowBossCrystal.h"
#include "enemy/boss/TEneBoss02.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "ui/TAeObjGo.h"
#include "io/Resource.h"
#include "egg/ObjEgg.h"
#include "enemy/boss/TEneYellowBoss.h"
#include "enemy/EnemyFx.h"
#include "object/ObjShadow.h"
#include "enemy/ObjEnemy.h"
#include "ui/AeUtil.h"
#include "enemy/EnemyAnim.h"
#include "sound/SeHolder.h"
#include "enemy/EnemyTexFx.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "enemy/boss/EneBoss.h"
#include "engine/MotionPose.h"
#include "animal/AniMount.h"
#include "stl/math.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "ui/AeObjControl.h"
#include "enemy/boss/TEneYellowBossBlue.h"
#include "core/TMainTask.h"
#include "ui/TAeObject.h"
struct NJS_TEXLIST;
struct Vec3f;
struct CharaColli;
struct GjActor;

extern "C" {
MotionEntry gYellowBossBlueMotionTable[] = {
	{ MOTION_ONCE, 0, -1, 0, 0.0f, 14.0f, 0.3f, 0.0f, 0 },
	{ MOTION_LOOP, 1, -1, 0, 0.0f, 19.0f, 0.5f, 0.0f, 0 },
};
}

typedef char TEneYellowBossBlue_size_check[(sizeof(TEneYellowBossBlue) == 0x254) ? 1 : -1];

// 0x801520F4
TEneYellowBossBlue::TEneYellowBossBlue(TObject* parent, const Vec3f* pos)
    : TEneYellowBossCrystal(parent)
{
	int i;

	_00         = gTEneYellowBossBlueName;
	mSize       = 0x254;
	mShape      = 0;
	mModel      = modelTEneYellowBoss(1);
	mMotions    = motionTEneYellowBoss(0);
	mXform.mPos = *pos;
	motionCtrlInit(&mClip, gYellowBossBlueMotionTable, mMotions);
	motionCtrlPlay(&mClip, 1);
	mAnim  = (GjActor*)actorCreate(mModel);
	mShape = (CharaColli*)createBoss02Body(mObjId);
	shadowSetGround(&mShadow, &mXform.mPos, 0);
	mShadow._30 = 80.0f;
	mShadow._34 = 80.0f;
	shadowEnable(&mShadow);
	for (i = 0; i < 6; i++) {
		mPoint[i].x = 0.0f;
		mPoint[i].y = 0.0f;
		mPoint[i].z = 0.0f;
	}
	mDraw        = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mBlueFlags   = 0;
	mHome        = (int)(10430.38043493439 * (f32)atan2(mXform.mPos.x, mXform.mPos.z));
	mTargetAngle = 0;
	mTarget.x    = 0.0f;
	mTarget.y    = 0.0f;
	mTarget.z    = 0.0f;
	mAimAngle    = 0;
	mRounds      = 0;
	mStep        = 0;
	mShots       = 0;
	mTurnStep    = 0;
	mAimStep     = 0;
	mTime        = 0.0f;
	startAction(0);
}

// 0x801522B0
TEneYellowBossBlue::~TEneYellowBossBlue()
{
	removeDrawTask(mDraw);
	colliRetire((CharaColli*)mShape);
	mShape = 0;
	actorFree((void**)&mAnim);
}

// 0x80152330
u8 TEneYellowBossBlue::isBroken() { return mBlueFlags & 1; }

// 0x8015233C
int TEneYellowBossBlue::onHit(const TObjAction* other, int arg, int* result)
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

// 0x801523C8
f32 TEneYellowBossBlue::vfn_40() { return 90.0f; }

// 0x801523D0
void TEneYellowBossBlue::pose()
{
	nodeTrackBeginOrigins((const int*)paramsTEneBoss02(), (Vec3f*)mPoint);
	gjPushUnitMatrix();
	tiltMatrix(mXform.mRot.x);
	actorApplyMotion(mAnim, &mClip);
	njPopMatrixEx();
	nodeTrackEnd();
}

// 0x80152424
void TEneYellowBossBlue::calcPoints()
{
	int i;

	gjPushUnitMatrix();
	bodyMatrix();
	for (i = 0; i < 6; i++) {
		njCalcPointEx(&mPoint[i], &mPoint[i]);
	}
	njPopMatrixEx();
}

// 0x80152484
void TEneYellowBossBlue::syncShape()
{
	Vec3f base;

	bodyBase(&mXform.mPos, mPoint, &base);
	placeBody(mShape, &base);
}

// 0x801524C8
void TEneYellowBossBlue::bodyMatrix()
{
	njTranslateEx(&mXform.mPos);
	gjRotateY(mXform.mRot.y);
}

// 0x80152500
void TEneYellowBossBlue::dropShadow() { fitShadow(50.02f, &mXform.mPos, mPoint, &mShadow); }

// 0x80152534
void TEneYellowBossBlue::draw()
{
	clearNext();
	crystalTouch((CharaColli*)mShape, &mTouch, 5.0f);
	if (mCrystalFlags & 4) {
		mCrystalFlags &= ~4;
		setNext(2);
	} else {
		checkAction();
	}
	switchAction();
	stepAction();
	dropShadow();
	syncShape();
}

// 0x801525BC
void TEneYellowBossBlue::runAction(int action, int phase)
{
	switch (action) {
	case 0:
		actBreak(phase);
		break;
	case 1:
		actShoot(phase);
		break;
	case 2:
		actDie(phase);
		break;
	}
}

// 0x8015261C
// Comes out of the boss's crystal facing the middle
void TEneYellowBossBlue::actBreak(int phase)
{
	int* step = &mStep;
	f32* time = &mTime;

	switch (phase) {
	case 0:
		motionCtrlPlay(&mClip, 1);
		motionCtrlUpdate(&mClip);
		pose();
		mXform.mRot.y = mHome + 0x8000;
		calcPoints();
		*step = crystalBreakFx(40.0f, &mXform.mPos);
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

// 0x80152728
// Puts target on the ring at angle and gives the heading to it from from
void TEneYellowBossBlue::aimAt(Vec3f from, int angle, Vec3f* target, int* heading)
{
	Vec3f dir;

	vecFromAngleXZ(180.0f, target, angle);
	target->y = 50.02f;
	dir.x     = target->x;
	dir.y     = target->y;
	dir.z     = target->z;
	njSubVector(&dir, &from);
	*heading = (u16)(int)(10430.38043493439 * (f32)atan2(dir.x, dir.z));
}

// 0x801527C0
// Turns to each point of the fan and fires
void TEneYellowBossBlue::actShoot(int phase)
{
	int* turn = &mTurnStep;
	int angle;
	f32 frame;

	switch (phase) {
	case 0:
		motionCtrlPlay(&mClip, 1);
		motionCtrlSetSpeed(&mClip, 2.0f);
		mStep     = 0;
		*turn     = 0;
		mShots    = 0;
		f32 fanStep = 5851.4287f;
		mAimStep  = (int)fanStep;
		mAimAngle = mHome + 0x3000;
		aimAt(mXform.mPos, mAimAngle, &mTarget, &mTargetAngle);
		break;
	case 1:
		if (mStep == 2) {
			setNext(1);
		}
		break;
	case 2:
		motionCtrlUpdate(&mClip);
		pose();
		calcPoints();
		switch (mStep) {
		case 0:
			frame = motionCtrlFrame(&mClip);
			angle = mXform.mRot.y;
			if (turnToward(frame, 2.0f, mTargetAngle, &angle, turn)) {
				mStep = 1;
				motionCtrlPlayBlend(0.3f, &mClip, 0);
			}
			mXform.mRot.y = angle;
			break;
		case 1:
			if (atShotFrame(&mClip)) {
				new TEneYellowBossTama_Blue(mParent, &mPoint[3], &mTarget);
				seStartAtFar(0x167, &mPoint[3]);
			}
			if (motionCtrlIsDone(&mClip)) {
				mShots++;
				if ((f32)mShots >= 8.0f) {
					mRounds++;
					if (mRounds >= 1) {
						mBlueFlags |= 1;
						motionCtrlPlay(&mClip, 1);
					} else {
						mStep = 2;
					}
				} else {
					mAimAngle += mAimStep;
					aimAt(mXform.mPos, mAimAngle, &mTarget, &mTargetAngle);
					mStep = 0;
					motionCtrlPlayBlend(0.2f, &mClip, 1);
				}
			}
			break;
		}
		break;
	case 3:
		motionCtrlSetSpeed(&mClip, 1.0f);
		break;
	}
}

// 0x80152A60
void TEneYellowBossBlue::actDie(int phase)
{
	switch (phase) {
	case 0:
		startSink(80.0f, 0.0f, 10.0f);
		colliDisable(mShape);
		mBlueFlags |= 8;
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

// 0x80152B10
void TEneYellowBossBlue::vfn_10() { cull(mDraw, 3000.0f, 70.0f, &mXform.mPos, 0); }

// 0x80152B44
void TEneYellowBossBlue::render()
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

// --- The "Go!" sign ---
