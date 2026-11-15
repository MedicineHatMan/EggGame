#include "camera/TCamera.h"
#include "enemy/EnemyLight.h"
#include "engine/ScrollModel.h"
#include "engine/DrawTask.h"
#include "object/ObjModel.h"
#include "io/ResBank.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/GenTex.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "core/MathUtil.h"
#include "camera/TCameraCtrl.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "object/red/TObjRedMagmaScene.h"
#include "camera/TCamPath.h"
#include "enemy/boss/TEneBoss02_Pool.h"
struct GjActor;
struct NJS_OBJECT;
struct DrawStateSlot;

// What njGetCameraMotion fills in
struct CamMotionOut {
	Vec3f* mPos;
	Vec3f* mDir;
	s16* mRoll;
	s16* mFov;
};

typedef char TCamPath_size_check[(sizeof(TCamPath) == 0xB4) ? 1 : -1];

extern const char* gTCamPathName;

// Not in the DOL the linker stripped it
f32 camPathFirstFrame()
{
	return 0.0f;
}

// 0x80063DB8
TCamPath::TCamPath(TObject* parent, int player, int look, void* object, void* motion, f32 start, f32 speed)
    : TCamera(parent, player, look)
{
	_00        = (u32)gTCamPathName;
	mSize      = 0xB4;
	mMotion    = motion;
	mSpeed     = speed;
	mFrame     = start - mSpeed;
	mLastFrame = (u32)(*(int*)((u8*)motion + 4) - 1);
	mPathFlags = 0;
	if (object != 0) {
		setStart((CamPathStart*)object);
	} else {
		startFromView();
	}
	mtxIdentity(mMtx);
	setBlendStep(1.0f);
}

// 0x80063EA8
TCamPath::~TCamPath()
{
}

// 0x80063F04
void TCamPath::vfn_30()
{
	mFrame = 0.0f;
}

// 0x80063F10
void TCamPath::setAutoDelete()
{
	mPathFlags |= 2;
}

// 0x80063F20
void TCamPath::clearAutoDelete()
{
	mPathFlags &= ~2;
}

// 0x80063F30
void TCamPath::setStart(const CamPathStart* start)
{
	mStart.mPos.x = start->mPos.x;
	mStart.mPos.y = start->mPos.y;
	mStart.mPos.z = start->mPos.z;
	mStart.mTarget.set(start->mTarget.x, start->mTarget.y, start->mTarget.z);
	mStart.mRoll = start->mRoll;
	mStart.mFov  = start->mFov;
	mStart._1C   = start->_1C;
}

// 0x80063F7C
void TCamPath::startFromView()
{
	mStart.mPos.x    = ((CamView*)mView)->mPos.x;
	mStart.mPos.y    = ((CamView*)mView)->mPos.y;
	mStart.mPos.z    = ((CamView*)mView)->mPos.z;
	mStart.mTarget.x = ((CamView*)mView)->mTarget.x;
	mStart.mTarget.y = ((CamView*)mView)->mTarget.y;
	mStart.mTarget.z = ((CamView*)mView)->mTarget.z;
	mStart.mRoll     = ((CamView*)mView)->mRoll;
	mStart.mFov      = ((CamView*)mView)->mFov;
}

// 0x80063FE0
void TCamPath::attachToMtx()
{
	mPathFlags |= 4;
}

// 0x80063FF0
f32* TCamPath::getMtx()
{
	return mMtx;
}

// 0x80063FF8
f32 TCamPath::getFrame()
{
	return mFrame;
}

// 0x80064000
void TCamPath::setFrame(f32 frame)
{
	mFrame = frame;
}

// 0x80064008
int TCamPath::isDone()
{
	return mFrame >= mLastFrame;
}

// 0x80064024
void TCamPath::advance()
{
	mFrame += mSpeed;
	if (mFrame > mLastFrame) {
		mFrame = mLastFrame;
	}
}

// 0x8006404C
void TCamPath::vfn_20()
{
	Vec3f pos;
	Vec3f dir;
	CamMotionOut out;
	Vec3f tmp;
	s16 roll;
	s16 fov;
	CamView* view;

	advance();
	out.mPos  = &pos;
	out.mDir  = &dir;
	out.mRoll = &roll;
	out.mFov  = &fov;
	roll      = mStart.mRoll;
	fov       = mStart.mFov;
	njGetCameraMotion((const GjCameraBase*)&mStart, mMotion, (const GjCameraOut*)&out, mFrame);
	if (mPathFlags & 4) {
		tmp.x = pos.x;
		tmp.y = pos.y;
		tmp.z = pos.z;
		mtxMultVec(mMtx, &tmp, &tmp);
		pos.x = tmp.x;
		pos.y = tmp.y;
		pos.z = tmp.z;
		tmp.x = dir.x;
		tmp.y = dir.y;
		tmp.z = dir.z;
		mtxMultVecSR(mMtx, &tmp, &tmp);
		dir.x = tmp.x;
		dir.y = tmp.y;
		dir.z = tmp.z;
	}
	((CamView*)mView)->mPos.set(pos.x, pos.y, pos.z);
	dir.x = dir.x + pos.x;
	dir.y = dir.y + pos.y;
	dir.z = dir.z + pos.z;
	((CamView*)mView)->mTarget.set(dir.x, dir.y, dir.z);
	((CamView*)mView)->mRoll = roll;
	((CamView*)mView)->mFov  = fov;
	if (isDone() && (mPathFlags & 2)) {
		mFlags |= 1;
	}
}

// 0x800641E4
void TCamPath::vfn_24()
{
	advance();
}

// 0x80064204
void TCamPath::vfn_10()
{
}

const char* gTCamPathName = "TCamPath";
