#include "stage/SetActor.h"
#include "engine/CharaColli.h"
#include "object/ObjAction.h"
#include "engine/MotionPlayer.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "stage/StageObjUtil.h"
#include "camera/TCamera.h"
#include "engine/DrawState.h"
#include "core/MathUtil.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "core/Mem.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "engine/TParticle.h"
#include "sound/Sound3DPlay.h"
#include "engine/Colli.h"
#include "stl/math.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "core/VecMath.h"
#include "object/TObjPlayer.h"
#include "object/TObjShark01.h"

extern const f32 gUnitVectorMin;
struct DrawTask;
struct CharaColli;
struct GjActor;
struct ColliContact;

extern "C" {
MotionEntry sShark01Motions[] = {
	{ MOTION_LOOP, 3, -1, 0, 0.0f, 10.0f, 0.5f, 0.0f, 0 },
	{ MOTION_ONCE, 2, -1, 0, 0.0f, 25.0f, 0.25f, 0.0f, 0 },
};
const char* gTObjShark01Name = "TObjShark01";

extern const ColliShape sShark01Shape[] = {
	{ { 0.0f, 0.0f, 10.0f }, 0x0, 0, 0x0, 10.0f, 0.0f, 0.0f, 0x0, COLLISHAPE_SPHERE, 0x0 },
};

}

typedef char TObjShark01_size_check[(sizeof(TObjShark01) == 0xFC) ? 1 : -1];

// 0x8002CFEC
extern "C" TObject* createTObjShark01(TObject* parent, SetDesc* desc)
{
	return new TObjShark01(parent, desc);
}

// 0x8002D040
void TObjShark01::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mRadius = 100.0f + desc->getFloat(1);
	f32 height = 30.0f + desc->getFloat(2);
	Vec3f offset = gVecZero;
	colliSetCylinder((CharaColli*)mTerritory, &mPos, 0, &offset, mRadius, height);
	colliRefreshBounds(mTerritory);
}

// 0x8002D0F4
void TObjShark01::debugPrint(void*)
{
	njPrintC(0xA0018, "+100.0:COL_RADIUS");
	njPrintC(0xA0019, "+20.0:COL_HEIGHT");
	colliDrawShapes(mTerritory);
}

// 0x8002D14C
TObjShark01::TObjShark01(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)gTObjShark01Name;
	mSize = 0xFC;
	mTerritory = colliCreate(1, 1);
	colliSetOwner(mTerritory, (u16)mObjId);
	colliSetMasks(mTerritory, 4, 0xFF, 0xFF, 0xF6);
	initFromDesc(desc);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	vfn_74();
}

// 0x8002D228
TObjShark01::~TObjShark01()
{
	colliRetire(mShape);
	mShape = 0;
	colliRetire(mTerritory);
	mTerritory = 0;
	memFree(mActor);
	removeDrawTask(mDrawTask);
}

// 0x8002D2C4
void TObjShark01::vfn_74()
{
	mHome      = *getPos();
	mTarget    = mHome;
	mVel.x     = 0.0f;
	mVel.y     = 0.0f;
	mVel.z     = 0.0f;
	mAct       = 0;
	mMotionId  = 0;
	mSwimTimer = 300;
	mSplashed  = 0;
	mSpeed     = 1.2f;
	mTurnRate  = 0x180;

	mTexList = getGeObjTextures()[0];
	mMotion.init((const MotionEntry*)sShark01Motions, gGeObjCommon->mSet.mMotions);
	mMotion.play((s16)mMotionId);
	mMotion.mSpeed = 0.5f;

	void* model = getGeObjModels()[0];
	mActor = (GjActor*)(memAlloc(gjActorSize((NJS_OBJECT*)model)));
	gjCreateActor((NJS_OBJECT*)model, (u8*)mActor);
	updateActor();

	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0, 0, 0xF6);
	colliCopyShapes(mShape, sShark01Shape);
	colliPlaceShapesRotY(mShape, &mPos, getRotY(), sShark01Shape);
	colliShapeSetBit8(mShape, 0);
}

// 0x8002D448
void TObjShark01::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (!isInRange(this, &mPos, 1100.0f)) {
		mFlags |= 1;
		return;
	}
	switch (mAct) {
	case 0:
		stateSwim();
		break;
	case 1:
		stateChase();
		break;
	case 2:
		stateLeap();
		break;
	}
	updateActor();
	colliPlaceShapesRotY(mShape, &mPos, getRotY(), sShark01Shape);
}

// 0x8002D520
void TObjShark01::vfn_10()
{
	if (cameraIsVisible(1000.0f, 30.0f, getPos(), 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8002D594
void TObjShark01::render()
{
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY(mRot.y);
	gjRotateX(mRot.x);
	njSetTexture(mTexList);
	drawActorDefault(mActor);
	njPopMatrixEx();
}

// 0x8002D5EC
// Looks for a player kind 0 in the territory and remembers it
int TObjShark01::findPlayer()
{
	for (ColliContact* hit = mTerritory->mContacts; hit != 0; hit = hit->mNext) {
		if (colliContactNoBit10(hit) != 0) {
			TObjAction* obj = (TObjAction*)findObjById((u16)colliContactOwner(hit));
			if (obj != 0 && obj->getKind() == 0) {
				mPlayer.setId(colliContactOwnerU16(hit));
				return 1;
			}
		}
	}
	return 0;
}

// 0x8002D68C
void TObjShark01::stateSwim()
{
	if (findPlayer() != 0) {
		mAct = 1;
		mMotion.mSpeed = 1.0f;
		return;
	}
	if (distXZ(&mTarget, getPos()) < mSpeed || mSwimTimer == 0) {
		mSwimTimer = 300;
		pickTarget();
	}
	mSwimTimer--;
	swimToTarget();
}

// 0x8002D730
// Speeds up to 2.4 on the way
void TObjShark01::stateChase()
{
	if (findPlayer() == 0) {
		mTarget   = mHome;
		mSpeed    = 1.2f;
		mTurnRate = 0x180;
		mAct      = 0;
		mMotionId = 0;
		mMotion.play((s16)mMotionId);
		mMotion.mSpeed = 0.5f;
		return;
	}
	chaseTarget();
	if (distXZ(&mTarget, getPos()) < 86.0f && mPos.y == mHome.y) {
		particleOnceById(this, getPos(), 1, 2);
		particleOnceById(this, getPos(), 1, 3);
		startLeap();
		mAct      = 2;
		mMotionId = 1;
		mMotion.play((s16)mMotionId);
		sePlay3D(0x210, getPos());
	} else {
		swimToTarget();
		if (mSpeed < 2.4f) {
			mSpeed += 0.02f;
		} else {
			mSpeed = 2.4f;
		}
	}
}

// 0x8002D8C0
void TObjShark01::stateLeap()
{
	if (mSplashed == 0 && mPos.y < mHome.y) {
		sePlay3D(0x211, getPos());
		particleOnceById(this, getPos(), 1, 2);
		particleOnceById(this, getPos(), 1, 3);
		mSplashed = 1;
	}
	if (mPos.y < mHome.y - 50.0f) {
		mTarget   = mHome;
		mSpeed    = 1.2f;
		mTurnRate = 0x180;
		mAct      = 0;
		mMotionId = 0;
		mMotion.play((s16)mMotionId);
		mMotion.mSpeed = 0.5f;
		return;
	}
	mVel.y += -0.08166667f;
	njAddVector(&mPos, &mVel);
	int yaw = vecAngleXZ(&mVel);
	mRot.y = angleApproach(getRotY(), (u16)yaw, mTurnRate);
	mRot.x = -(u16)(int)(10430.38043493439 * (f32)atan2(mVel.y, vecLenXZ(&mVel)));
}

// 0x8002DA64
void TObjShark01::chaseTarget()
{
	mTarget.x = 0.0f;
	mTarget.y = 0.0f;
	mTarget.z = 0.0f;
	TObjAction* player = (TObjAction*)mPlayer.get();
	if (player != 0) {
		mTarget = *player->getPos();
	}
}

// 0x8002DAD4
// A random point within the radius of home
void TObjShark01::pickTarget()
{
	mTarget.x = 0.0f;
	mTarget.y = 0.0f;
	mTarget.z = 0.0f;
	mTarget.x = 2.0f * ((f32)rand() * (1.0f / 32768.0f) - 0.5f);
	mTarget.z = 2.0f * ((f32)rand() * (1.0f / 32768.0f) - 0.5f);
	if (njUnitVector(&mTarget) < gUnitVectorMin) {
		mTarget = gVecZero;
	}
	f32 radius = mRadius;
	f32 lo     = 0.0f;
	f32 dist   = lo + (radius - lo) * ((f32)rand() * (1.0f / 32768.0f));
	gjScaleVector(dist, &mTarget, &mTarget);
	njAddVector(&mTarget, &mHome);
}

// 0x8002DC0C
void TObjShark01::swimToTarget()
{
	int yaw = angleToXZ(getPos(), &mTarget);
	int rotY = angleApproach(getRotY(), (u16)yaw, mTurnRate);
	mRot.y = rotY;
	mRot.x = angleApproach(mRot.x, 0, mTurnRate);
	vecFromAngleXZ(mSpeed, &mVel, rotY);

	f32 rise = mHome.y - mPos.y;
	f32 most = 0.5f * mSpeed;
	if (rise < most) {
		mVel.y = rise;
	} else {
		mVel.y = most;
	}
	njAddVector(&mPos, &mVel);
}

// 0x8002DCD4
// Straight at the target with a rise of 1.225 that gravity then brings down
void TObjShark01::startLeap()
{
	vecSub(&mVel, &mTarget, &mPos);
	vecNormalizeXZ(&mVel, &mVel);
	vecScale(2.8f, &mVel, &mVel);
	mVel.y    = 1.225f;
	mSplashed = 0;
	mTurnRate = 0x400;
}

// 0x8002DD40
void TObjShark01::updateActor()
{
	mMotion.update();
	gjPushUnitMatrix();
	njPushMatrixEx();
	if (mMotion.isBlending()) {
		gjMotionLinkActor(mActor, (void*)mMotion.getBlend(), mMotion.mBlendTime);
	} else {
		int cur = mMotion.mCurrent;
		f32 frame = mMotion.mFrame;
		gjMotionActor(frame, mActor, mMotion.getMotion(cur));
	}
	njPopMatrixEx();
	njPopMatrixEx();
}

// 0x8002DDDC
int TObjShark01::onHit(const TObjAction*, int, int* info)
{
	HitInfo* hit = (HitInfo*)info;
	resetHitInfo(hit);
	hit->mDamage = 10;
	hit->mFlags.setBit(13);
	return 1;
}
