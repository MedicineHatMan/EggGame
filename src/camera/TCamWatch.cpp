#include "core/TMainTask.h"
#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "camera/CamCurve.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCamWatch.h"

const char* gTCamWatchName = "TCamWatch";

typedef char TCamWatch_size_check[(sizeof(TCamWatch) == 0x268) ? 1 : -1];

extern "C" TCamera* createTCamWatch(int player, int look, const Vec3f* target)
{
	return new TCamWatch(gTL_01, player, look, target);
}

// 0x8008A638
TCamWatch::TCamWatch(TObject* parent, int player, int look, const Vec3f* target)
    : TCamera(parent, player, look)
{
	_00   = (u32)gTCamWatchName;
	mSize = 0x268;
	Vec3f zero;
	zero.x = 0.0f;
	zero.y = 0.0f;
	zero.z = 0.0f;
	camHeadStart(&mHead, 0xFFFF);
	camCurveStart(&mCurve, &zero);
	camCurveHold(&mCurve);
	mTarget = *target;
}

// 0x8008A70C
TCamWatch::~TCamWatch()
{
}

// 0x8008A768
void TCamWatch::setTarget(u16 id)
{
	Vec3f at;
	TCamera::setTarget(id);
	camHeadStart(&mHead, id);
	camHeadGetPos(&mHead, &at);
	camCurveStart(&mCurve, &at);
	camCurveHold(&mCurve);
}

// 0x8008A7C4
void TCamWatch::vfn_20()
{
	Vec3f at;
	camHeadUpdate(&mHead, mKind);
	camCurveUpdate(&mCurve, &mHead);
	camCurveGetPos(&mCurve, &at);
	((Vec3f*)mView)[1] = at;
	((Vec3f*)mView)[0] = mTarget;
}
// 0x8008A848
void TCamWatch::vfn_10()
{
}
