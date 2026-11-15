#include "enemy/boss/TEneBoss02_GateShot.h"
#include "engine/MotionPose.h"
#include "sound/SeHolder.h"
#include "Ninja/ninja.h"
#include "enemy/boss/TEneBoss02_Pool.h"
#include "enemy/boss/TEneBoss02_PoolPow.h"
#include "engine/TParticle.h"
struct DrawTask;

extern "C" {
}

typedef char TEneBoss02_GateShot_size_check[(sizeof(TEneBoss02_GateShot) == 0x44) ? 1 : -1];

// 0x800E73F8
// The first draw steps onto frame
TEneBoss02_GateShot::TEneBoss02_GateShot(TObject* parent, void* motion, f32 frame, f32 rate)
    : TObject(parent)
{
	mMotion = motion;
	mFrame  = frame - rate;
	mRate   = rate;
	mVisual = 0;
	mPos.x  = 0.0f;
	mPos.y  = 0.0f;
	mPos.z  = 0.0f;
	mState  = 0;
	_3C     = 0;
}

// 0x800E7490
TEneBoss02_GateShot::~TEneBoss02_GateShot()
{
	if (mVisual != 0) {
		mVisual->mFlags |= 1;
		mVisual = 0;
	}
}

// 0x800E7510
void TEneBoss02_GateShot::getPos(Vec3f* out)
{
	MotionNodePose pose;
	NJS_OBJECT obj;
	motionObjInit(&obj);
	motionGetNodePose(&pose, &obj, mMotion, mFrame);
	motionPoseGetPos(&pose, out);
}

// 0x800E7564
void TEneBoss02_GateShot::draw()
{
	mFrame += mRate;
	switch (mState) {
	case 0:
		getPos(&mPos);
		if (mVisual == 0) {
			mVisual = new TEneBoss02_WaterShot_Visual(this, &mPos);
		} else {
			mVisual->setPos(&mPos);
		}
		if (mFrame >= 30.0f) {
			mState = 1;
			makeBossGate(mParent, &mPos);
			particleOnceByIdDefault(&mPos, 1, 0x4B);
			seStartWith(0x113, 0x5F);
			if (mVisual != 0) {
				mVisual->mFlags |= 1;
				mVisual = 0;
			}
		}
		break;
	case 1:
		if (mFrame >= 170.0f) {
			mState = 2;
			particleGenTimedByIdDefault(&mPos, 1, 0x50, 0, 0x96);
			particleGenTimedByIdDefault(&mPos, 1, 0x51, 0, 0x96);
			createPoolPowGen(mParent, &mPos, 0x96);
		}
		break;
	case 2:
		if (mFrame >= 320.0f) {
			mState = 4;
			closeBossGates();
		}
		break;
	case 4:
		mFlags |= 1;
		break;
	}
}
