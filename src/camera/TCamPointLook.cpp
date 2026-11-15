#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "camera/CamCurve.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "camera/TCamPointLook.h"
#include "animal/RacerMover.h"
#include "core/TMainTask.h"

extern "C" {

}

typedef char TCamPointLook_size_check[(sizeof(TCamPointLook) == 0x26C) ? 1 : -1];

// 0x800A5B64
TCamPointLook* createTCamPointLook(int player, int look, const Vec3f* target, f32 distance)
{
	return new TCamPointLook(gTL_01, player, look, target, distance);
}

const char* gTCamPointLookName = "TCamPointLook";

// 0x800A5BD4
TCamPointLook::TCamPointLook(TObject* parent, int player, int look, const Vec3f* target, f32 distance)
    : TCamera(parent, player, look)
{
	_00   = (u32)gTCamPointLookName;
	mSize = 0x26C;
	Vec3f zero;
	zero.x = 0.0f;
	zero.y = 0.0f;
	zero.z = 0.0f;
	camHeadStart(&mHead, 0xFFFF);
	camCurveStart(&mCurve, &zero);
	camCurveHold(&mCurve);
	setTarget(target);
	setDistance(distance);
}

// 0x800A5CBC
TCamPointLook::~TCamPointLook()
{
}

// 0x800A5D18
void TCamPointLook::setTarget(const Vec3f* target)
{
	mTarget = *target;
}

// 0x800A5D34
void TCamPointLook::setDistance(f32 distance)
{
	mDistance = distance;
	if (mDistance < 5.0f) {
		mDistance = 5.0f;
	}
}

// 0x800A5D50
void TCamPointLook::setTarget(u16 id)
{
	Vec3f at;
	TCamera::setTarget(id);
	camHeadStart(&mHead, id);
	camHeadGetPos(&mHead, &at);
	camCurveStart(&mCurve, &at);
	camCurveHold(&mCurve);
}

// 0x800A5DAC
void TCamPointLook::vfn_20()
{
	Vec3f at;
	Vec3f dir;
	camHeadUpdate(&mHead, mKind);
	camCurveUpdate(&mCurve, &mHead);
	camCurveGetPos(&mCurve, &at);
	((Vec3f*)mView)[1] = at;
	dir = at;
	njSubVector(&dir, &mTarget);
	if (njUnitVector(&dir) < gUnitVectorMin) {
		dir = gVecZero;
	}
	Vec3f eye;
	gjMulAddVector(mDistance, &eye, &at, &dir);
	((Vec3f*)mView)[0] = eye;
}

// 0x800A5E98
void TCamPointLook::vfn_10()
{
}
