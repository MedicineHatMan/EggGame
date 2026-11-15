#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "camera/TCamZelda5.h"
#include "egg/EggCreature.h"
#include "camera/CamCurve.h"
#include "camera/TCameraCtrl.h"
#include "enemy/boss/EneBoss.h"
#include "stl/math.h"
#include "Ninja/ninja.h"

extern TObject* gTL_01;

static inline f32 zeldaSqrt(f32 x)
{
	static const f64 _half  = .5;
	static const f64 _three = 3.0;
	volatile f32 y;
	if (x > 0.0f) {
		f64 guess = __frsqrte((f64)x);
		guess     = _half * guess * (_three - guess * guess * x);
		guess     = _half * guess * (_three - guess * guess * x);
		y         = (f32)(x * (_half * guess * (_three - guess * guess * x)));
		return y;
	}
	return x;
}

/** The angle round y that d points along */
static inline int zeldaYaw(const Vec3f* d)
{
	return 10430.38043493439 * (f32)atan2(d->x, d->z);
}

typedef char TCamZelda5_size_check[(sizeof(TCamZelda5) == 0x424) ? 1 : -1];

const char* gTCamZelda5Name = "TCamZelda5";

// 0x800632D0
TCamZelda5::TCamZelda5(TObject* parent, int player, int look)
    : TCamera(parent, player, look)
{
	_00   = (u32)gTCamZelda5Name;
	mSize = 0x424;
	start();
}

// 0x80063378
TCamZelda5::~TCamZelda5()
{
}

// 0x800633D4
void TCamZelda5::start()
{
	Vec3f at;

	camHeadStart(&mHead, mKind);
	camHeadGetPos(&mHead, &at);
	camCurveStart(&mCurve, &at);
	camCurveGetPos(&mCurve, &mCurvePos);
	camChaseInit(&mChase, &mHead, &mCurve);
	mShaking    = FALSE;
	mShakeVel   = 0.0f;
	mShake      = 0.0f;
	mCamFlags   = 0;
	mLookFrom.x = 0.0f;
	mLookFrom.y = 0.0f;
	mLookFrom.z = 0.0f;
}

// 0x80063460
void TCamZelda5::snap()
{
	Vec3f at;
	Vec3f eye;
	int rot;
	CamView* view;

	camHeadUpdate(&mHead, mKind);
	camHeadGetPos(&mHead, &at);
	rot = camHeadGetRot(&mHead);
	camCurveReset(&mCurve, &at);
	camCurveGetPos(&mCurve, &at);
	camChaseSnap(&mChase, rot);
	camChaseUpdate(&mChase, (CamViewInfo*)mView, camGetPath(mPlayer));
	camChaseGetPos(&mChase, &eye);
	view          = (CamView*)mView;
	view->mTarget = at;
	view          = (CamView*)mView;
	view->mPos    = eye;
}

inline void TCamZelda5::restart()
{
	Vec3f at;

	camHeadStart(&mHead, mKind);
	camHeadGetPos(&mHead, &at);
	camCurveReset(&mCurve, &at);
	camCurveGetPos(&mCurve, &mCurvePos);
	camChaseReset(&mChase, camHeadGetRot(&mHead));
	mShaking  = FALSE;
	mShakeVel = 0.0f;
	mShake    = 0.0f;
	mCamFlags = 0;
}

// 0x80063530
void TCamZelda5::setTarget(u16 id)
{
	u16 old = mKind;

	TCamera::setTarget(id);
	if (id != 0xFFFF && old == 0xFFFF) {
		restart();
		mLookFrom.x = 0.0f;
		mLookFrom.y = 0.0f;
		mLookFrom.z = 0.0f;
		snap();
	}
}

// 0x800635E8
void TCamZelda5::vfn_30()
{
	setTarget(mKind);
	restart();
	mBlendStep  = 1.0f;
	mLookFrom.x = 0.0f;
	mLookFrom.y = 0.0f;
	mLookFrom.z = 0.0f;
}

// 0x80063694
// Swings the view the way it is already going
void TCamZelda5::shake(f32 power)
{
	mShaking = TRUE;
	if (mShakeVel >= 0.0f) {
		mShakeVel = mShakeVel + power;
		return;
	}
	mShakeVel = mShakeVel - power;
}

// 0x800636C8
void TCamZelda5::setMode(int mode)
{
}

// 0x800636CC
int TCamZelda5::vfn_28()
{
	u8 changed = FALSE;

	if (TCamera::vfn_28() || (mCamFlags & ZELDA_CHANGED)) {
		changed = TRUE;
	}
	return changed;
}

// 0x80063718
void TCamZelda5::setCurveHeight(f32 height)
{
	camCurveSetHeight(height, &mCurve);
	resetCurveLift();
}

// 0x80063750
// The distance is given forward
void TCamZelda5::setChaseBase(f32 height, f32 dist)
{
	camChaseSetBase(&mChase, height, -dist);
	resetChaseOffset();
}

// 0x8006378C
void TCamZelda5::setChaseBaseSpeed(f32 speed)
{
	camChaseSetBaseSpeed(&mChase, speed);
	resetChaseSpeed();
}

// 0x800637C4
void TCamZelda5::setCurveLift(f32 lift)
{
	camCurveSetLift(lift, &mCurve);
	mCamFlags |= ZELDA_CHANGED;
}

// 0x80063800
void TCamZelda5::resetCurveLift()
{
	camCurveResetLift(&mCurve);
	mCamFlags |= ZELDA_CHANGED;
}

// 0x8006383C
void TCamZelda5::setChaseOffset(f32 height, f32 dist)
{
	camChaseSetOffset(&mChase, height, -dist);
	mCamFlags |= ZELDA_CHANGED;
}

// 0x8006387C
void TCamZelda5::resetChaseOffset()
{
	camChaseResetOffset(&mChase);
	mCamFlags |= ZELDA_CHANGED;
}

// 0x800638B8
void TCamZelda5::setChaseSpeed(f32 speed)
{
	camChaseSetSpeed(&mChase, speed);
}

// 0x800638DC
void TCamZelda5::resetChaseSpeed()
{
	camChaseResetSpeed(&mChase);
}

// 0x80063900
void TCamZelda5::setChasePitch(int pitch)
{
	camChaseSetPitch(&mChase, pitch);
}

// 0x80063924
void TCamZelda5::resetChasePitch()
{
	camChaseResetPitch(&mChase);
}

// 0x80063948
void TCamZelda5::resetChase(int rot)
{
	mCamFlags |= ZELDA_CHANGED;
	camChaseReset(&mChase, rot);
}

// 0x8006397C
void TCamZelda5::setLookFrom(const Vec3f* from)
{
	mCamFlags |= ZELDA_LOOK_FROM;
	mLookFrom = *from;
}

// 0x800639A4
// On a cut the curve and the chase start again from the head turned to face along the cut
// The file is built with lifetimes off configure.py z stays a named local so the sqrt result is not coalesced into it
// fmr f1,f4
void TCamZelda5::vfn_20()
{
	Vec3f at;
	Vec3f eye;
	Vec3f d;
	Vec3f swing;
	CamView* view;

	mCamFlags &= ~ZELDA_CHANGED;
	if (mKind == 0xFFFF) {
		return;
	}
	camHeadUpdate(&mHead, mKind);
	if (camLinkIsEntered(&mLook)) {
		camHeadGetPos(&mHead, &at);
		camCurveReset(&mCurve, &at);
		camCurveGetPos(&mCurve, &at);
		if (mCamFlags & ZELDA_LOOK_FROM) {
			mCamFlags &= ~ZELDA_LOOK_FROM;
			d = mLookFrom;
			njSubVector(&d, &at);
			camChaseSnap(&mChase, zeldaYaw(&d));
		} else if (camLinkIsCut(&mLook)) {
			snap();
		} else if (mView != 0) {
			d = at;
			njSubVector(&d, (Vec3f*)mView);
			camChaseSnap(&mChase, zeldaYaw(&d));
		}
	} else {
		camCurveUpdate(&mCurve, &mHead);
		camCurveGetPos(&mCurve, &at);
	}
	camChaseUpdate(&mChase, (CamViewInfo*)mView, camGetPath(mPlayer));
	camChaseGetPos(&mChase, &eye);

	if (mShaking) {
		s32 pitch;
		s32 yaw;

		mShakeVel = mShakeVel - 0.3f * mShake;
		mShakeVel *= 0.92f;
		mShake    = mShake + mShakeVel;
		if ((f32)__fabs(mShake) < 0.1f && (f32)__fabs(mShakeVel) < 0.1f) {
			mShaking = FALSE;
		}
		pitch = 100.0f * mShake;
		swing = at;
		njSubVector(&swing, &eye);
		yaw     = zeldaYaw(&swing);
		f32 z = swing.z;
		z = z * z + swing.x * swing.x;
		swing.z = zeldaSqrt(z);
		swing.x = 0.0f;
		gjPushUnitMatrix();
		gjRotateY(yaw);
		gjRotateX(pitch);
		njCalcVectorEx(&swing, &swing);
		njPopMatrixEx();
		at = eye;
		njAddVector(&at, &swing);
	}
	view          = (CamView*)mView;
	view->mTarget = at;
	view          = (CamView*)mView;
	view->mPos    = eye;
}

// 0x80063D0C
void TCamZelda5::vfn_24()
{
}

// 0x80063D10
void TCamZelda5::vfn_10()
{
}

// 0x80063D14
TCamPath* createCamPath(int player, int kind, void* object, void* motion, int autoDelete, f32 start, f32 speed)
{
	TCamPath* path = new TCamPath(gTL_01, player, kind, object, motion, start, speed);

	if (path != 0 && autoDelete) {
		path->setAutoDelete();
	}
	return path;
}
