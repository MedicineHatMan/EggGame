#include "camera/TCamera.h"
#include "egg/ObjEgg.h"
#include "camera/CamCurve.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "camera/TCamVerticalLine.h"
#include "core/TMainTask.h"
struct Vec3f;

extern "C" {

}

typedef char TCamVerticalLine_size_check[(sizeof(TCamVerticalLine) == 0x270) ? 1 : -1];

extern "C" TCamera* createTCamVerticalLine(int player, int look, const Vec3f* target, const void* line)
{
	return new TCamVerticalLine(gTL_01, player, look, target, line);
}

const char* gTCamVerticalLineName = "TCamVerticalLine";

// 0x8008A8B4
TCamVerticalLine::TCamVerticalLine(TObject* parent, int player, int look, const Vec3f* target, const void* line)
    : TCamera(parent, player, look)
{
	_00    = (u32)gTCamVerticalLineName;
	mSize  = 0x270;
	camHeadStart(&mHead, 0xFFFF);
	camCurveStart(&mCurve, target);
	camCurveHold(&mCurve);
	mTarget = *target;
	mRise  = *(f32*)((u8*)line + 4);
	mReach = vecLenXZ((const Vec3f*)line);
	if (mReach < 5.0f) {
		mReach = 5.0f;
	}
}

// 0x8008A9A4
TCamVerticalLine::~TCamVerticalLine()
{
}

// 0x8008AA00
void TCamVerticalLine::setTarget(u16 id)
{
	Vec3f at;
	TCamera::setTarget(id);
	camHeadStart(&mHead, id);
	camHeadGetPos(&mHead, &at);
	camCurveStart(&mCurve, &at);
	camCurveHold(&mCurve);
}

// 0x8008AA5C
void TCamVerticalLine::vfn_20()
{
	Vec3f at;
	camHeadUpdate(&mHead, mKind);
	camCurveUpdate(&mCurve, &mHead);
	camCurveGetPos(&mCurve, &at);
	((Vec3f*)mView)[1] = at;
	Vec3f dir;
	dir = at;
	njSubVector(&dir, &mTarget);
	vecNormalizeXZ(&dir, &dir);
	f32 x = dir.x * mReach + at.x;
	f32 y = at.y + mRise;
	f32 z = dir.z * mReach + at.z;
	((Vec3f*)mView)[0].set(x, y, z);
}

// 0x8008AB2C
void TCamVerticalLine::vfn_10()
{
}
