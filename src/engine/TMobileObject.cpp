#include "engine/TMobileObject.h"
#include "player/Player.h"
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "core/Vibration.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "stage/TStageScore.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "engine/TParticle.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "engine/Colli.h"
#include "egg/EggCreature.h"
#include "engine/GroundQuery.h"
#include "engine/Viewport.h"
#include "core/Pad.h"
#include "object/TObjPlayer.h"
#include "core/VecMath.h"
#include "egg/EggBody.h"
#include "engine/WorldMesh.h"
#include "egg/EggLevel.h"
#include "enemy/Enemy.h"
#include "core/TMainTask.h"
struct MobileBodyParams;
struct NJS_QUATERNION;
struct ColliShape;
struct CharaColli;
struct Fx;

typedef char TMobileObject_size_check[(sizeof(TMobileObject) == 0x34C) ? 1 : -1];

const char* gTMobileObjectName = "TMobileObject";

extern "C" {

// The per-frame contact callbacks and the two search callbacks defined further down
}

// A message sent to the owner carrying one thing after its type
struct MobileMsg {
	u16 mType;   // _00
	u16 mTarget; // _02
	void* mPtr;  // _04
	void* mPtr2; // _08
	f32 mValue;  // _0C
};

// A task from particleGenById Its flags have bit 1 to end it and bit 4 to hide it and its position is at _24
// msgAttach's message the object and the part of it to hold on to
struct MobileMsgAttach {
	u16 mType;  // _00
	u8 _02[2];
	u16 mObjId; // _04
	u16 mPart;  // _06
};

struct MobileTask {
	u8 _00[4];
	u16 mFlags; // _04
	u8 _06[0x1E];
	Vec3f mPos; // _24
};

// What the wall and rail searches hand back
struct MobileWallResult {
	void* mModel;  // _00
	u16 mFace;     // _04
	Vec3f mPos;    // _08
	Vec3f mNormal; // _14
	f32 mDist;     // _20
	u32 mFlags;    // _24
};

struct MobileRailResult {
	int mFound;     // _00
	Vec3f mPos;     // _04
};

// What checkRail's search fills in
struct MobileRailInfo {
	TMobileObject* mThis; // _00
	u8 _04[0x18];
	RailPath* mRail;      // _1C
	RailPoint mPoint;     // _20
	u8 _28[4];
};

// One face of a collision model
struct MobileFace {
	u8 _00[8];
	u32 mFlags; // _08
	u8 _0C[0x1C];
};

struct MobileColModel {
	u8 _00[0x14];
	MobileFace* mFaces; // _14
};

inline u32 faceFlags(void* model, u16 face)
{
	return ((u32*)&((MobileColModel*)model)->mFaces[face])[2];
}

inline void TMobileObject::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

inline EggSurface::~EggSurface() { }

/*
 * --INFO--
 * Address:	8008AE94
 * Size:	0000F8
 */
// 0x8008AE94
MobileSpark::MobileSpark(TObject* parent, u16 player, Vec3f* pos, f32 scale)
    : TObject(parent)
{
	mLife    = 30;
	mEmitter = colliCreate(1, 1);
	if (mEmitter != 0) {
		colliSetOwner((CharaColli*)mEmitter, player);
		colliSetMasks((CharaColli*)mEmitter, 1, 0, 0, 0x10);
		ColliShape* shape = (ColliShape*)colliGetShape((CharaColli*)mEmitter, 0);
		shape->mTag = 1;
		colliShapeSetSphere(colliGetShape((CharaColli*)mEmitter, 0));
		colliShapeSetPos(colliGetShape((CharaColli*)mEmitter, 0), pos);
		colliShapeSetRadius(colliGetShape((CharaColli*)mEmitter, 0), scale);
		*(u32*)((u8*)colliGetShape((CharaColli*)mEmitter, 0) + 0x24) |= 0x18;
	}
}

/*
 * --INFO--
 * Address:	8008AF8C
 * Size:	000068
 */
MobileSpark::~MobileSpark()
{
	colliFree((CharaColli*)mEmitter);
}

/*
 * --INFO--
 * Address:	8008AFF4
 * Size:	000020
 */
void MobileSpark::draw()
{
	if (--mLife == 0) {
		mFlags |= 1;
	}
}

/*
 * --INFO--
 * Address:	8008B014
 * Size:	00001C
 */
EggHandleBase::EggHandleBase()
{
	mHolder = 0;
	mOwner  = 0;
}

/*
 * --INFO--
 * Address:	8008B030
 * Size:	000064
 */
EggHandleBase::~EggHandleBase()
{
	if (mHolder != 0) {
		mHolder->setHandle(0);
	}
}

/*
 * --INFO--
 * Address:	8008B094
 * Size:	000044
 */
void EggHandleBase::release()
{
	if (mHolder != 0) {
		mHolder->setHandle(0);
		mHolder = 0;
	}
}

/*
 * --INFO--
 * Address:	8008B0D8
 * Size:	000050
 */
int EggHandleBase::send(ObjMsg* msg)
{
	if (mHolder != 0) {
		return mHolder->onMessage(this, msg);
	}
	return onMessage(msg);
}

/*
 * --INFO--
 * Address:	8008B128
 * Size:	000118
 */
// 0x8008B128
TMobileObject::TMobileObject(TObject* parent)
    : TObject(parent)
    , mHandle(0)
    , mSpecial(0)
    , mDust(this)
    , mWake(this)
{
	_00        = (u32)gTMobileObjectName;
	mSize       = 0x34C;
	mState      = MOBILE_DETACHED;
	mComboTimer = 0;
	mCombo      = 0;
	mParams     = playerGetParams(0);
	mBodyParams = (MobileBodyParams*)playerGetParamsTail();

	mGravity.set(0.0f, mBodyParams->mGravity, 0.0f);
	mDust.mBody = &mBody;
	mWake.mBody = &mBody;
}

// 0x8008B2B8
EggBodyItem::~EggBodyItem()
{
	riderDtor((AniRider*)this, -1);
}

/*
 * --INFO--
 * Address:	8008B304
 * Size:	000104
 */
// 0x8008B304
TMobileObject::~TMobileObject()
{
	setHandle(0);
	seStop3D((SoundHandle*)&mLoop);
}

/*
 * --INFO--
 * Address:	8008B408
 * Size:	0000B8
 *
 * Passing 0 detaches. The owner is told with message 0x27 if something was attached.
 */
void TMobileObject::setHandle(EggHandleBase* handle)
{
	EggHandleBase* old = mHandle;
	if (handle != old) {
		runState(mState, MOBILE_EXIT);
		mState = MOBILE_DETACHED;
		riderDetach(&mGround);
		mRailNext.setPath(0);
		if (old != 0) {
			old->mHolder = 0;
		}
		if (handle == 0) {
			ObjMsg msg;
			mState   = MOBILE_DETACHED;
			msg.mType = 0x27;
			sendMsgByIdU16(mTarget.getId(), (ObjMsg*)&msg);
		} else {
			handle->mHolder = this;
			mTarget.setId(0xFFFF);
		}
		mHandle = handle;
	}
}

/*
 * --INFO--
 * Address:	8008B4C0
 * Size:	0000C4
 */
// 0x8008B4C0
void TMobileObject::reset(f32 radius, const Vec3f* pos, const EggRot* rot)
{
	mBody.mPos       = *pos;
	mBody.mRot       = *rot;
	mBody.mRadius    = radius;
	mBody.makeMatrix(mMatrix);
	mState       = MOBILE_DETACHED;
	mBody.mVel   = gVecZero;
	mBody.mAccel = gVecZero;
	mBody.mAngVel = gVecZero;
}

/*
 * --INFO--
 * Address:	8008B584
 * Size:	00001C
 */
TObjAction* TMobileObject::getHeld()
{
	if (mHandle != 0) {
		return (TObjAction*)mHandle->mOwner;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008B5A0
 * Size:	00002C
 */
void TMobileObject::setFacing(void* mtx)
{
	mtxToQuat((const f32*)mtx, (f32*)mFacing);
}

/*
 * --INFO--
 * Address:	8008B5CC
 * Size:	000158
 *
 * What a hit does while something is attached. An egg calls this through its handle.
 */
// 0x8008B5CC
int TMobileObject::takeHit(HitInfo* hit, int power)
{
	Vec3f vel;
	vel = mBody.mVel;

	switch (mState) {
	case MOBILE_AIR:
	case MOBILE_DASH:
		if (njScalor(&vel) < 10.0f) {
			return 0;
		}
		hit->mDamage = power;
		hit->mFlags.setBit(0);
		hit->mFlags.setBit(0xC);
		hit->_14 = 0;
		vecScale(1.0f / 60.0f, &hit->mPos, &vel);
		return power;
	case MOBILE_SLAM:
	case MOBILE_SLAM_LAND:
		hit->_14 = mPower;
		// fallthrough
	case MOBILE_HOP:
		hit->mDamage = power;
		hit->mFlags.setBit(0);
		hit->mFlags.setBit(0xF);
		return power;
	case MOBILE_ROLL:
		if (njScalor(&vel) < 10.0f) {
			return 0;
		}
		hit->mDamage = power;
		hit->mFlags.setBit(0);
		hit->mFlags.setBit(0xC);
		vecScale(1.0f / 60.0f, &hit->mPos, &vel);
		return power;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008B724
 * Size:	000120
 */
// 0x8008B724
void TMobileObject::draw()
{
	runState(mState, MOBILE_CHECK);
	runState(mState, MOBILE_UPDATE);
	mMoveAmount = 0.0f;

	if (mComboTimer != 0 && --mComboTimer == 0) {
		mCombo = 0;
	}

	Vec3f dir;
	dir = mBody.mVel;
	if (vecNormalize(&dir, &dir) > gUnitVectorMin) {
		Vec3f pos;
		pos = mBody.mPos;
		f32 scale = mBody.mRadius / 5.0f;
		mtxSetTransV(mTrailMatrix, &pos);
		mtxRotateY(mTrailMatrix, mTrailMatrix, vecAngleXZ(&dir));
		mtxScale(scale, scale, scale, mTrailMatrix, mTrailMatrix);
	}
	mSound.update();
}

/*
 * --INFO--
 * Address:	8008B844
 * Size:	00000C
 */
void TMobileObject::setMove(f32 amount, int angle)
{
	mMoveAmount = amount;
	mMoveAngle  = angle;
}

/*
 * --INFO--
 * Address:	8008B850
 * Size:	00003C
 */
int TMobileObject::getMove(f32* amount, int* angle)
{
	if (mMoveAmount > 0.1f) {
		if (amount != 0) {
			*amount = mMoveAmount;
		}
		if (angle != 0) {
			*angle = mMoveAngle;
		}
		return 1;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008B88C
 * Size:	000008
 */
void TMobileObject::setSpecial(int special)
{
	mSpecial = special;
}

/*
 * --INFO--
 * Address:	8008B894
 * Size:	000024
 */
void TMobileObject::startRoll()
{
	setState(MOBILE_ROLL);
}

/*
 * --INFO--
 * Address:	8008B8B8
 * Size:	000024
 */
void TMobileObject::startAir()
{
	setState(MOBILE_AIR);
}

/*
 * --INFO--
 * Address:	8008B8DC
 * Size:	000024
 */
void TMobileObject::startDrive()
{
	setState(MOBILE_DRIVE);
}

/*
 * --INFO--
 * Address:	8008B900
 * Size:	0000CC
 *
 * Slows the body by a fraction of its speed, plus a fixed part, and never reverses it.
 */
void TMobileObject::applyDrag(f32 amount)
{
	Vec3f dir;
	dir = mBody.mVel;
	f32 speed = njUnitVector(&dir);
	if (speed < gUnitVectorMin) {
		dir = gVecZero;
	}
	f32 next = speed - (0.035f * speed) * (1.0f + amount);
	if (next < 0.0f) {
		next = 0.0f;
	}
	vecScale(next, &dir, &dir);
	mBody.mVel = dir;
}

/*
 * --INFO--
 * Address:	8008B9CC
 * Size:	000054
 */
// 0x8008B9CC
int TMobileObject::onSpecialGround()
{
	if (mGround.mMesh != 0 && (faceFlags(mGround.mMesh, mFace) & 0x400) != 0) {
		sendSpecial();
		return 1;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008BA20
 * Size:	000040
 */
void TMobileObject::stopSettle()
{
	mBody.mVel = gVecZero;
	setState(MOBILE_SETTLE);
}

/*
 * --INFO--
 * Address:	8008BA60
 * Size:	000040
 */
void TMobileObject::stopBrake()
{
	mBody.mVel = gVecZero;
	setState(MOBILE_BRAKE);
}

/*
 * --INFO--
 * Address:	8008BAA0
 * Size:	00005C
 */
// 0x8008BAA0
int TMobileObject::sendHang(f32 unused0, int unused1)
{
	switch (mState) {
	case MOBILE_ATTACHED:
		ObjMsg msg;
		msg.mType = 0x34;
		mTarget.get()->sendToHandle(&msg);
		return 1;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008BAFC
 * Size:	00005C
 */
// 0x8008BAFC
void TMobileObject::addLift()
{
	MobileParams* params = mParams;
	u16 frames = mFrames;
	if (frames >= params->mLiftFrom && frames <= params->mLiftTo) {
		Vec3f lift;
		lift.y = params->mLift;
		lift.x = 0.0f;
		lift.z = 0.0f;
		njAddVector(&mBody.mAccel, &lift);
	}
}

/*
 * --INFO--
 * Address:	8008BB58
 * Size:	000020
 */
void TMobileObject::steer(f32 amount, int angle)
{
	setMove(amount, angle);
}

/*
 * --INFO--
 * Address:	8008BB78
 * Size:	000080
 */
void TMobileObject::updateDustMatrix()
{
	Vec3f pos;
	pos = mBody.mPos;
	f32 scale = mBody.mRadius / 5.0f;
	mtxSetTransV(mDustMatrix, &pos);
	mtxScale(scale, scale, scale, mDustMatrix, mDustMatrix);
}

/*
 * --INFO--
 * Address:	8008BBF8
 * Size:	00001C
 */
// 0x8008BBF8
void TMobileObject::saveLoopPos()
{
	f32 y = mBody.mPos.y;
	f32 z = mBody.mPos.z;
	mLoop.mPos.set(mBody.mPos.x, y, z);
}

/*
 * --INFO--
 * Address:	8008BC14
 * Size:	000054
 */
f32 TMobileObject::getMotion(Vec3f* pos, Vec3f* vel)
{
	*pos = mBody.mPos;
	*vel = mBody.mVel;
	return vecLength(vel);
}

/*
 * --INFO--
 * Address:	8008BC68
 * Size:	000040
 */
void TMobileObject::setRollSound(f32 level, Vec3f* pos)
{
	mSound.setPos(pos);
	mSound.mLevel = level;
}

/*
 * --INFO--
 * Address:	8008BCA8
 * Size:	00006C
 */
void TMobileObject::startRail(void* path, void* point, f32 speed)
{
	mRail.setPath((RailPath*)path);
	mRail.setPoint((RailPoint*)point);
	mRailSpeed = speed;
	mRailNext.setPath((RailPath*)path);
	setState(MOBILE_RAIL);
}

/*
 * --INFO--
 * Address:	8008BD14
 * Size:	0000AC
 */
void TMobileObject::moveAlongGround(f32 amount, int angle)
{
	if ((mFlags.mBits & 1) != 0 && mGround.mMesh != 0) {
		Vec3f normal;
		Vec3f vel;
		meshTriNormal(mGround.mMesh, mFace, &normal);
		vecFromAngleXZ(amount, &vel, angle);
		f32 dot = njInnerProduct(&vel, &normal);
		vecScaleAdd(-dot, &vel, &normal, &vel);
		mBody.mVel = vel;
	}
}

/*
 * --INFO--
 * Address:	8008BDC0
 * Size:	000040
 */
// 0x8008BDC0
int TMobileObject::startHop()
{
	mBody.mVel.set(0.0f, mParams->mHopHeight, 0.0f);
	setState(MOBILE_HOP);
	return 1;
}

/*
 * --INFO--
 * Address:	8008BE00
 * Size:	000050
 */
// 0x8008BE00
int TMobileObject::tryDash()
{
	if (probeFloor(0) == 0) {
		return 0;
	}
	setState(MOBILE_DASH);
	return 1;
}

/*
 * --INFO--
 * Address:	8008BE50
 * Size:	00005C
 */
// 0x8008BE50
int TMobileObject::startSlam()
{
	mBody.mVel.set(0.0f, mParams->mHopHeight, 0.0f);
	mPower = scoreNewChain(2);
	setState(MOBILE_SLAM);
	return 1;
}

/*
 * --INFO--
 * Address:	8008BEAC
 * Size:	0000A0
 *
 * Needs the bounce jump ability from what is attached, which eggs answer to message 0x40.
 */
// 0x8008BEAC
int TMobileObject::tryUp()
{
	MobileMsg msg;
	msg.mType = 0x40;
	*(u16*)&msg.mPtr = 0xFFFF;
	TObjAction* held = getHeld();
	if (held->sendToHandle((ObjMsg*)&msg) & 0x20) {
		mBody.mVel.set(0.0f, mParams->mUpHeight, 0.0f);
		riderDetach(&mGround);
		mFlags.mBits &= ~1;
		setState(MOBILE_UP);
		return 1;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008BF4C
 * Size:	000138
 *
 * The same four lines three times over in the DOL, so it was probably an inline.
 */
// 0x8008BF4C
int TMobileObject::launch(f32 amount, int angle)
{
	if (mState == MOBILE_DETACHED) {
		riderDetach(&mGround);
		mFlags.mBits &= ~1;
		setLaunchVel(amount, angle);
		setState(MOBILE_LAUNCH);
		return 1;
	}
	if ((mFlags.mBits & 1) != 0 && mGround.mMesh != 0 && (faceFlags(mGround.mMesh, mFace) & 0x400) == 0) {
		riderDetach(&mGround);
		mFlags.mBits &= ~1;
		setLaunchVel(amount, angle);
		setState(MOBILE_LAUNCH);
		return 1;
	}
	if (probeFloor(0) != 0) {
		riderDetach(&mGround);
		mFlags.mBits &= ~1;
		setLaunchVel(amount, angle);
		setState(MOBILE_LAUNCH);
		return 1;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008C084
 * Size:	000074
 */
int TMobileObject::startThrown(Vec3f* dir)
{
	Vec3f vel;
	vecScale(mParams->mDriveCap, &vel, dir);
	mBody.mVel = vel;
	mFlags.mBits &= ~1;
	riderDetach(&mGround);
	setState(MOBILE_THROWN);
	return 1;
}

/*
 * --INFO--
 * Address:	8008C0F8
 * Size:	0000D0
 *
 * True when there is ground within 3 under the body. Unless any is set, the special surfaces don't count.
 */
// 0x8008C0F8
int TMobileObject::probeFloor(int any) const
{
	EggHits probe;
	Vec3f pos;
	pos.x = mBody.mPos.x;
	pos.y = mBody.mPos.y;
	pos.z = mBody.mPos.z;
	if (meshColliProbe((EggHit*)&probe, (const Vec3f*)&pos, 1) != 0 && (probe.mMask & 1) != 0) {
		if (any == 0 && (*(u32*)(probe.mHits[0]._3C + 4) & 0x400) != 0) {
			return 0;
		}
		if (3.0f + probe.mHits[0].mPos.y > pos.y - mBody.mRadius) {
			return 1;
		}
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008C1C8
 * Size:	0000B4
 */
// 0x8008C1C8
int TMobileObject::tryLand()
{
	if ((mFlags.mBits & 1) != 0 && mGround.mMesh != 0 && (faceFlags(mGround.mMesh, mFace) & 0x400) != 0) {
		mBody.mVel.set(mBody.mVel.x, mParams->mLandHeight, mBody.mVel.z);
		mFlags.mBits &= ~1;
		return 1;
	}
	if (probeFloor(1) != 0) {
		mBody.mVel.set(mBody.mVel.x, mParams->mLandHeight, mBody.mVel.z);
		mFlags.mBits &= ~1;
		return 1;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008C27C
 * Size:	0000F8
 */
// 0x8008C27C
int TMobileObject::longJump()
{
	if ((mFlags.mBits & 1) != 0 || probeFloor(1) != 0) {
		Vec3f vel;
		vel = mBody.mVel;
		if (njScalor(&vel) > 1.0f) {
			vecNormalize(&vel, &vel);
			mAim = vel;
			f32 speed = mParams->mJumpSpeed;
			vel.x *= speed;
			vel.y = mParams->mJumpHeight;
			vel.z *= speed;
			mFlags.mBits &= ~1;
			mBody.mVel = vel;
			setState(MOBILE_LONG_JUMP);
			return 1;
		}
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008C374
 * Size:	0000B0
 *
 * On a special surface, and moving no faster than 0.1 away from it.
 */
// 0x8008C374
int TMobileObject::isSinking()
{
	void* model = mGround.mMesh;
	if (model == 0) {
		return 0;
	}
	if ((faceFlags(model, mFace) & 0x400) == 0) {
		return 0;
	}
	Vec3f normal;
	Vec3f vel;
	vel.x = mBody.mVel.x;
	vel.y = mBody.mVel.y;
	vel.z = mBody.mVel.z;
	meshTriNormal((MeshColli*)model, mFace, &normal);
	f32 dot = njInnerProduct(&vel, &normal);
	vecScaleAdd(-dot, &vel, &normal, &vel);
	return vel.y <= 0.1f;
}

/*
 * --INFO--
 * Address:	8008C424
 * Size:	000100
 *
 * Looks for a rail under the body. Message 0x52 asks the owner to take it, unless it is the one just left.
 */
// 0x8008C424
int TMobileObject::checkRail()
{
	MobileRailInfo info;
	Vec3f pos;
	pos = mBody.mPos;
	info.mThis = this;
	info.mRail = 0;
	if (railSearch(mBody.mRadius, &pos, mobileRailContact, &info) == 0) {
		return 0;
	}
	if (info.mRail == 0) {
		return 0;
	}
	if (info.mRail == mRailNext.mPath) {
		return 0;
	}
	Vec3f vel;
	Vec3f tangent;
	MobileMsg msg;
	msg.mType = 0x52;
	vel = mBody.mVel;
	info.mRail->getDir(&info.mPoint, &tangent);
	msg.mPtr   = info.mRail;
	msg.mPtr2  = &info.mPoint;
	msg.mValue = njInnerProduct(&vel, &tangent);
	return mOwner->sendToHandle((ObjMsg*)&msg);
}

/*
 * --INFO--
 * Address:	8008C524
 * Size:	0000A0
 */
int TMobileObject::checkHazards()
{
	if ((mFlags.mBits & 8) != 0 && sendCrush(1) != 0) {
		return 1;
	}
	if ((mFlags.mBits & 2) != 0 && sendHit(1) != 0) {
		return 1;
	}
	if ((mFlags.mBits & 4) != 0 && sendHit(2) != 0) {
		return 1;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008C5C4
 * Size:	0000E8
 *
 * Going upwards, it loses all but the vertical. The angular speed comes from the speed it lost.
 */
// 0x8008C5C4
int TMobileObject::bounceOffWall()
{
	Vec3f vel;
	vel = mBody.mVel;
	if (vel.y <= 0.0f) {
		return 0;
	}
	vecFromAngleXZ(10.0f, &vel, 0);
	Vec3f up;
	vel.y = 1.0f;
	up.x = 0.0f;
	up.y = -1.0f;
	up.z = 0.0f;
	njOuterProduct(&vel, &up, &up);
	vecScale(1.0f / mBody.mRadius, &up, &up);
	mBody.mAngVel = up;
	mBody.mVel    = vel;
	sendSpecial();
	return 1;
}

/*
 * --INFO--
 * Address:	8008C6AC
 * Size:	000090
 */
void TMobileObject::capSpeed(f32 cap)
{
	Vec3f dir;
	dir = mBody.mVel;
	if (vecNormalize(&dir, &dir) > cap) {
		vecScale(cap, &dir, &dir);
		mBody.mVel = dir;
	}
}

/*
 * --INFO--
 * Address:	8008C73C
 * Size:	000080
 */
void TMobileObject::capSpeedHigh()
{
	Vec3f dir;
	dir = mBody.mVel;
	if (vecNormalize(&dir, &dir) > 300.0f) {
		vecScale(300.0f, &dir, &dir);
		mBody.mVel = dir;
	}
}

/*
 * --INFO--
 * Address:	8008C7BC
 * Size:	000090
 */
void TMobileObject::slideOnGround()
{
	if (mGround.mMesh != 0) {
		Vec3f normal;
		meshTriNormal(mGround.mMesh, mFace, &normal);
		Vec3f vel;
		vel = mBody.mVel;
		f32 dot = njInnerProduct(&vel, &normal);
		vecScaleAdd(-dot, &vel, &normal, &vel);
		mBody.mVel = vel;
	}
}

/*
 * --INFO--
 * Address:	8008C84C
 * Size:	000098
 */
// 0x8008C84C
void TMobileObject::spawnShockWave(f32 power)
{
	Vec3f pos;
	pos.x = mBody.mPos.x;
	pos.y = mBody.mPos.y;
	pos.z = mBody.mPos.z;
	pos.y = mBody.mPos.y - mBody.mRadius;
	power *= mBody.mRadius;
	power *= 0.5f;
	eggSpawnShockWave(power, (u16)((TObjAction*)mHandle->mOwner)->mObjId,
	                  ((TObjAction*)mOwner)->mObjId, &pos, mHandle->mDamage, (void*)mPower);
	pos.y += 5.0f;
	particleOnceById(gTL_PARTICLE, &pos, 0, 9);
}

/*
 * --INFO--
 * Address:	8008C8E4
 * Size:	000004
 */
void TMobileObject::checkBumps() { }

/*
 * --INFO--
 * Address:	8008C8E8
 * Size:	00007C
 */
// 0x8008C8E8
void TMobileObject::moveGround(f32 cap)
{
	mBody.mFriction = mBodyParams->mFriction;
	mBody.mBounce = 0.0f;
	njAddVector(&mBody.mAccel, &mGravity);
	capSpeed(cap);
	stepBody();
	mBody.makeMatrix(mMatrix);
}

/*
 * --INFO--
 * Address:	8008C964
 * Size:	0001BC
 */
// 0x8008C964
// A copy of q turned by rot
static inline Quat multipliedQuat(Quat q, const EggRot* rot)
{
	njMultiQuaternion((const NJS_QUATERNION*)&q, (const NJS_QUATERNION*)rot, (NJS_QUATERNION*)&q);
	return q;
}

// A scaled copy of a quaternion
static inline Quat scaledQuat(Quat q, f32 scale)
{
	quatScale(&q, scale);
	return q;
}

void TMobileObject::moveRail()
{
	Vec3f point;
	Vec3f normal;
	Vec3f dir;
	Vec3f axis;

	mRail.advance((1.0f / 60.0f) * mRailSpeed);
	mRail.getPos(&point);
	mRail.getNormal(&normal);
	mRail.getDir(&dir);
	vecScaleAdd(mBody.mRadius, &point, &normal, &point);
	mBody.mPos = point;

	njOuterProduct(&dir, &normal, &axis);
	vecScale(((1.0f / 60.0f) * -mRailSpeed) / mBody.mRadius, &axis, &axis);

	EggRot rot;
	rot = mBody.mRot;
	Quat spin;
	spin.w = 0.0f;
	spin.v = axis;
	Quat step = scaledQuat(multipliedQuat(spin, &rot), 0.5f);
	quatAdd((Quat*)&rot, &step);
	quatNormalize((Quat*)&rot);
	mBody.mRot = rot;
	mBody.makeMatrix(mMatrix);
}

/*
 * --INFO--
 * Address:	8008CB20
 * Size:	0000E4
 *
 * Keeps vertical speed only, and turns toward mFacing.
 */
// 0x8008CB20
void TMobileObject::moveSettle()
{
	f32 rot[4];

	mBody.mFriction = mBodyParams->mFriction;
	mBody.mBounce = 0.0f;
	mBody.mAngVel   = gVecZero;
	njAddVector(&mBody.mAccel, &mGravity);
	capSpeedHigh();
	stepBody();
	mBody.makeMatrix(mMatrix);

	mBody.mVel.x = 0.0f;
	mBody.mVel.z = 0.0f;

	rot[0]             = mBody.mRot.mW;
	*(EggAxis*)&rot[1] = mBody.mRot.mAxis;
	njInterpolateQuaternion((const NJS_QUATERNION*)rot, (const NJS_QUATERNION*)mFacing, (NJS_QUATERNION*)rot, mFacingBlend);
	mBody.mRot.mW      = rot[0];
	mBody.mRot.mAxis   = *(EggAxis*)&rot[1];
}

/*
 * --INFO--
 * Address:	8008CC04
 * Size:	0000A8
 */
// 0x8008CC04
void TMobileObject::moveAttached()
{
	f32 part[12];
	Vec3f pos;
	f32 rot[4];
	TObjAction* target = mTarget.get();
	target->getNodeMatrix(mTargetPart, part);
	mtxConcatQuat(mMatrix, part, mAttachRot);
	mtxGetPos(mMatrix, &pos);
	mtxToQuat(mMatrix, rot);
	mBody.mPos       = pos;
	mBody.mRot.mW    = rot[0];
	mBody.mRot.mAxis = *(EggAxis*)&rot[1];
}

/*
 * --INFO--
 * Address:	8008CCAC
 * Size:	0000E8
 */
// 0x8008CCAC
void TMobileObject::moveDefaultNoGravity()
{
	f32 rot[4];

	mBody.mFriction = mBodyParams->mFriction;
	mBody.mBounce = 0.0f;
	mBody.mAngVel   = gVecZero;
	capSpeedHigh();
	stepBody();
	mBody.mAccel = gVecZero;
	mBody.makeMatrix(mMatrix);

	rot[0]             = mBody.mRot.mW;
	*(EggAxis*)&rot[1] = mBody.mRot.mAxis;
	njInterpolateQuaternion((const NJS_QUATERNION*)rot, (const NJS_QUATERNION*)mFacing, (NJS_QUATERNION*)rot, mFacingBlend);
	mBody.mRot.mW      = rot[0];
	mBody.mRot.mAxis   = *(EggAxis*)&rot[1];
}

/*
 * --INFO--
 * Address:	8008CD94
 * Size:	0000A0
 */
// 0x8008CD94
void TMobileObject::moveAscent()
{
	Vec3f force;

	mBody.mFriction = mBodyParams->mFriction;
	mBody.mBounce = 0.0f;
	force.set(0.0f, mParams->mLobAscent, 0.0f);
	njAddVector(&mBody.mAccel, &force);
	capSpeedHigh();
	force.set(0.0f, -20.0f, 0.0f);
	mBody.mAngVel.x = force.x;
	mBody.mAngVel.y = force.y;
	mBody.mAngVel.z = force.z;
	stepBody();
	mBody.makeMatrix(mMatrix);
}

/*
 * --INFO--
 * Address:	8008CE34
 * Size:	0000D8
 */
// 0x8008CE34
void TMobileObject::moveDefault()
{
	f32 rot[4];

	mBody.mFriction = mBodyParams->mFriction;
	mBody.mBounce = 0.0f;
	mBody.mAngVel   = gVecZero;
	njAddVector(&mBody.mAccel, &mGravity);
	capSpeedHigh();
	stepBody();
	mBody.makeMatrix(mMatrix);

	rot[0]             = mBody.mRot.mW;
	*(EggAxis*)&rot[1] = mBody.mRot.mAxis;
	njInterpolateQuaternion((const NJS_QUATERNION*)rot, (const NJS_QUATERNION*)mFacing, (NJS_QUATERNION*)rot, mFacingBlend);
	mBody.mRot.mW      = rot[0];
	mBody.mRot.mAxis   = *(EggAxis*)&rot[1];
}

/*
 * --INFO--
 * Address:	8008CF0C
 * Size:	000084
 */
// 0x8008CF0C
void TMobileObject::moveDown()
{
	mBody.mFriction = mBodyParams->mFriction;
	mBody.mBounce = 0.0f;
	mBody.mAngVel   = gVecZero;
	njAddVector(&mBody.mAccel, &mGravity);
	capSpeedHigh();
	stepBody();
	mBody.makeMatrix(mMatrix);
}

/*
 * --INFO--
 * Address:	8008CF90
 * Size:	0000F8
 *
 * The spin comes from velocity crossed with the up direction, the same as rolling on a floor.
 */
// 0x8008CF90
// The vector pointing the other way
static inline Vec3f negated(const Vec3f& v)
{
	Vec3f r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

void TMobileObject::moveAirSpin()
{
	Vec3f up;
	Vec3f spin;

	mBody.mFriction = mBodyParams->mFriction;
	mBody.mBounce = 0.0f;
	njAddVector(&mBody.mAccel, &mGravity);
	spin    = mBody.mVel;
	Vec3f down = negated(gVecUp);
	njOuterProduct(&spin, &down, &spin);
	vecScale(1.0f / mBody.mRadius, &spin, &spin);
	mBody.mAngVel = spin;
	stepBody();
	mBody.makeMatrix(mMatrix);
}

/*
 * --INFO--
 * Address:	8008D088
 * Size:	000128
 */
// 0x8008D088
void TMobileObject::moveThrown()
{
	Vec3f vec;

	mBody.mFriction = mBodyParams->mFriction;
	mBody.mBounce = 0.0f;
	vec.set(0.0f, mParams->mThrownAscent, 0.0f);
	njAddVector(&mBody.mAccel, &vec);
	capSpeedHigh();

	// The same vector becomes the spin axis
	vec = mBody.mVel;
	vecScale(njScalor(&vec), &vec, &mAim);
	Vec3f down = negated(gVecUp);
	njOuterProduct(&vec, &down, &vec);
	vecScale(1.0f / mBody.mRadius, &vec, &vec);
	mBody.mAngVel = vec;
	stepBody();
	mBody.makeMatrix(mMatrix);
}

/*
 * --INFO--
 * Address:	8008D1B0
 * Size:	00007C
 */
// 0x8008D1B0
void TMobileObject::moveRollCap(f32 cap)
{
	mBody.mFriction = mBodyParams->mFriction;
	mBody.mBounce = 0.0f;
	njAddVector(&mBody.mAccel, &mGravity);
	capSpeed(cap);
	stepBody();
	mBody.makeMatrix(mMatrix);
}

/*
 * --INFO--
 * Address:	8008D22C
 * Size:	0000B8
 *
 * One physics step. It records the ground and face it ended on, and keeps only bit 0x20 of the old flags.
 */
// 0x8008D22C
void TMobileObject::stepBody()
{
	struct {
		void* mModel;  // _00
		u16 mFace;     // _04
		Vec3f mDir;    // _08
		u32 mHit;      // _14
		f32 mMinNormal; // _18
	} result;

	result.mModel     = 0;
	result.mHit       = 0;
	result.mMinNormal = 0.868f;
	result.mDir       = mBody.mVel;
	vecNormalize(&result.mDir, &result.mDir);
	mBody.stepRoll(1.0f / 60.0f, &result, -1, NULL, (EggFaceFn)mobileGroundContact);
	riderSetMesh(&mGround, (RiderList*)result.mModel);
	mFace = result.mFace;
	mFlags.mBits &= 0x20;
	flagWordOr(&mFlags, &FlagWord(result.mHit));
}

/*
 * --INFO--
 * Address:	8008D2E4
 * Size:	000028
 *
 * The contact callback for stepBody. Anything flatter than 45 degrees counts as ground.
 */
extern "C" void mobileGroundContact(void* model, u32 face, void* arg, Vec3f* normal, void* result)
{
	struct Result {
		void* mModel;
		u16 mFace;
		u8 _06[0xE];
		u32 mHit;
	}* r = (Result*)result;
	if (normal->y > 0.7071f) {
		r->mHit |= 1;
		r->mModel = model;
		r->mFace  = face;
	}
}

/*
 * --INFO--
 * Address:	8008D30C
 * Size:	000140
 *
 * Reached through EggHandleBase::send. What this class doesn't take goes back to the handle's virtual.
 */
// 0x8008D30C
int TMobileObject::onMessage(EggHandleBase* handle, ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x01:
		return msgPush(msg);
	case 0x05:
		return msgIgnore(msg);
	case 0x16:
		return msgAttach(msg);
	case 0x22:
		return msgLaunch(msg);
	case 0x23:
		return msgFling(msg);
	case 0x2A:
		return msgRumble2A(msg);
	case 0x74:
		return msgRumble74(msg);
	case 0x8B:
		return msgCombo(msg);
	case 0x3C:
		return msgStateFlags(msg);
	case 0x41:
		return msgAbilities(msg);
	case 0x43:
		return msgGetVel(msg);
	}
	return ((MobileHandle*)handle)->onMessage(msg);
}

/*
 * --INFO--
 * Address:	8008D44C
 * Size:	000050
 */
void TMobileObject::setState(int state)
{
	runState(mState, MOBILE_EXIT);
	runState(state, MOBILE_ENTER);
	mState = state;
}

/*
 * --INFO--
 * Address:	8008D49C
 * Size:	000128
 */
void TMobileObject::runState(int state, int phase)
{
	switch (state) {
	case MOBILE_IDLE:
		stateIdle(phase);
		break;
	case MOBILE_SETTLE:
		stateSettle(phase);
		break;
	case MOBILE_ATTACHED:
		stateAttached(phase);
		break;
	case MOBILE_BRAKE:
		stateBrake(phase);
		break;
	case MOBILE_ROLL:
		stateRoll(phase);
		break;
	case MOBILE_LOB:
		stateLob(phase);
		break;
	case MOBILE_AIR:
		stateAir(phase);
		break;
	case MOBILE_HOP:
		stateHop(phase);
		break;
	case MOBILE_DASH:
		stateDash(phase);
		break;
	case MOBILE_SLAM:
		stateSlam(phase);
		break;
	case MOBILE_SLAM_LAND:
		stateSlamLand(phase);
		break;
	case MOBILE_LAUNCH:
		stateLaunch(phase);
		break;
	case MOBILE_UP:
		stateUp(phase);
		break;
	case MOBILE_DOWN:
		stateDown(phase);
		break;
	case MOBILE_FLUNG:
		stateFlung(phase);
		break;
	case MOBILE_DRIVE:
		stateDrive(phase);
		break;
	case MOBILE_THROWN:
		stateThrown(phase);
		break;
	case MOBILE_LONG_JUMP:
		stateLongJump(phase);
		break;
	case MOBILE_RAIL:
		stateRail(phase);
		break;
	case MOBILE_NONE:
		stateNone(phase);
		break;
	}
}

/*
 * --INFO--
 * Address:	8008D5C4
 * Size:	00005C
 */
void TMobileObject::stateIdle(int phase)
{
	if (phase == MOBILE_ENTER) {
		riderDetach(&mGround);
		mFlags.mBits &= ~1;
	} else if (phase != MOBILE_CHECK && phase == MOBILE_UPDATE) {
		moveDefault();
	}
}

/*
 * --INFO--
 * Address:	8008D620
 * Size:	000044
 */
void TMobileObject::stateSettle(int phase)
{
	if (phase == MOBILE_CHECK) {
		if ((mFlags.mBits & 1) != 0) {
			sendLanded();
		}
	} else if (phase == MOBILE_UPDATE) {
		moveSettle();
	}
}

/*
 * --INFO--
 * Address:	8008D664
 * Size:	000070
 */
void TMobileObject::stateAttached(int phase)
{
	if (phase == MOBILE_ENTER) {
		riderDetach(&mGround);
	} else if (phase == MOBILE_CHECK) {
		if (mTarget.get() == 0) {
			setState(MOBILE_AIR);
		}
	} else if (phase == MOBILE_UPDATE) {
		moveAttached();
	}
}

/*
 * --INFO--
 * Address:	8008D6D4
 * Size:	000080
 */
// 0x8008D6D4
void TMobileObject::stateBrake(int phase)
{
	if (phase == MOBILE_ENTER) {
		mFacingBlend = 0.0f;
	} else if (phase == MOBILE_UPDATE) {
		if (1.0f != mFacingBlend) {
			mFacingBlend += 1.0f / 120.0f;
			if (mFacingBlend > 1.0f) {
				mFacingBlend = 1.0f;
			}
		}
		checkBumps();
		moveDefaultNoGravity();
	}
}

/*
 * --INFO--
 * Address:	8008D754
 * Size:	0000A4
 */
void TMobileObject::stateFlung(int phase)
{
	if (phase == MOBILE_ENTER) {
		riderDetach(&mGround);
		mFlags.mBits &= ~1;
	} else if (phase == MOBILE_CHECK) {
		if (checkHazards() == 0 && (mFlags.mBits & 1) != 0) {
			mBody.mVel = gVecZero;
			sendLanded();
		}
	} else if (phase == MOBILE_UPDATE) {
		checkBumps();
		moveAirSpin();
	}
}

/*
 * --INFO--
 * Address:	8008D7F8
 * Size:	000084
 *
 * For findContact. Keeps the nearest wall or floor marked as a hazard, and returns 0 to keep the search going.
 */
// 0x8008D7F8
extern "C" int mobileWallContact(f32 dist, void* model, u16 face, const Vec3f* pos, void* result)
{
	MobileWallResult* r = (MobileWallResult*)result;
	u8* tri             = *(u8**)((u8*)model + 0x14) + face * 0x28;
	if ((*(u32*)(tri + 8) & 0xC) != 0 && r->mDist > dist) {
		r->mModel = model;
		r->mFace  = face;
		r->mPos   = *pos;
		r->mDist  = dist;
		r->mFlags = *(u32*)(tri + 8);
		meshTriNormalU16((MeshColli*)model, face, &r->mNormal);
		return 0;
	}
	return 1;
}

/*
 * --INFO--
 * Address:	8008D87C
 * Size:	000050
 *
 * For the rail search in stateRoll. Takes the first rail of type 1.
 */
extern "C" void mobileRollRailContact(RailPath* rail, const RailPoint* point, Vec3f* pos, void* info, int* stop)
{
	if (rail->mData->mType == 1) {
		MobileRailResult* r = (MobileRailResult*)info;
		rail->getDir(point, &r->mPos);
		r->mFound = 1;
		*stop     = 1;
	}
}

/*
 * --INFO--
 * Address:	8008D8CC
 * Size:	000470
 *
 * The main rolling state. The two track effects run while a hazard wall is close by, and steering works differently on it.
 */
// 0x8008D8CC
void TMobileObject::stateRoll(int phase)
{
	if (phase == MOBILE_ENTER) {
		mRollFrames = 0;
		mSound.setSe(0x7B);
		mSound.start();
		padVibStart((u8)((TObjAction*)mOwner)->mObjId, (int)this, 99, 0, 6);
	} else if (phase == MOBILE_EXIT) {
		mWake.stop();
		mDust.stop();
		vibStopOwner((u8)((TObjAction*)mOwner)->mObjId, this);
		mSound.stop();
	} else if (phase == MOBILE_UPDATE) {
		Vec3f normal;
		Vec3f wallPos;
		Vec3f move;
		Vec3f push;
		Vec3f motion;
		Vec3f here;
		Vec3f up;
		Vec3f spin;
		MobileRailResult rail;
		Vec3f force;
		Vec3f pos;
		Vec3f vel;
		u32 flags;
		f32 wallDist;
		f32 amount;
		int angle;

		mRollFrames++;
		void* model = mGround.mMesh;
		if (model != 0) {
			if (getMove(&amount, &angle) != 0) {
				meshTriNormal((MeshColli*)model, mFace, &normal);
				if ((faceFlags(model, mFace) & 0x400) == 0 && normal.y > 0.866025f) {
					vecFromAngleXZ(amount, &move, angle);
					f32 dot = njInnerProduct(&move, &normal);
					vecScaleAdd(-dot, &move, &normal, &move);
					mBody.mVel = move;
				}
			}
			mWake.stop();
			mDust.stop();
		} else {
			if (findContact(&flags, &wallPos, &normal, &wallDist) == 0) {
				mWake.stop();
				mDust.stop();
			} else {
				here = mBody.mPos;
				motion = mBody.mVel;
				push.x = push.z = 0.0f;
				push.y = 720.0f * (mBody.mRadius - wallDist) - 3.0f * motion.y;
				if (push.y > 0.0f) {
					njAddVector(&mBody.mAccel, &push);
				}
				if ((flags & 4) != 0) {
					mWake.start();
				} else {
					mWake.stop();
				}
				if ((flags & 8) != 0) {
					mDust.start();
				} else {
					mDust.stop();
				}

				up.x = 0.0f;
				up.y = -1.0f;
				up.z = 0.0f;
				njOuterProduct(&motion, &up, &spin);
				vecScale(1.0f / mBody.mRadius, &spin, &spin);
				mBody.mAngVel = spin;

				if (normal.y < 1.0f && (flags & 4) != 0) {
					rail.mFound = 0;
					if (railSearchAll(70.0f, &here, mobileRollRailContact, &rail) != 0 && rail.mFound != 0) {
						vecScale(1000.0f, &force, &rail.mPos);
						njAddVector(&mBody.mAccel, &force);
					}
					if (getMove(0, &angle) != 0) {
						mobileTurnVector(&motion, &motion, &normal, angle, (1.0f / 60.0f) * (2.0f * (f32)mParams->mTurn));
						mBody.mVel = motion;
					}
				} else {
					if (getMove(&amount, &angle) != 0) {
						vecFromAngleXZ(amount, &move, angle);
						f32 dot = njInnerProduct(&move, &normal);
						vecScaleAdd(-dot, &move, &normal, &move);
						mBody.mVel = move;
					}
					if (vecLength(&motion) > 0.1f) {
						// The push vector is reused for the drag
						vecScale(-0.59999996f, &push, &motion);
						njAddVector(&mBody.mAccel, &push);
					}
				}
			}
		}
		checkBumps();
		moveRollCap(mBodyParams->mRollCap);
		f32 level = getMotion(&pos, &vel);
		setRollSound(level, &pos);
		mDust.update();
		mWake.update();
	}
}

/*
 * --INFO--
 * Address:	8008DD3C
 * Size:	000094
 */
void TMobileObject::stateLob(int phase)
{
	if (phase == MOBILE_ENTER) {
		riderDetach(&mGround);
		mFlags.mBits &= ~1;
	} else if (phase == MOBILE_CHECK) {
		if ((mFlags.mBits & 1) != 0) {
			mBody.mVel = gVecZero;
			sendLanded();
		}
	} else if (phase == MOBILE_UPDATE) {
		checkBumps();
		moveAirSpin();
	}
}

/*
 * --INFO--
 * Address:	8008DDD0
 * Size:	0001A4
 */
// 0x8008DDD0
void TMobileObject::stateAir(int phase)
{
	if (phase == MOBILE_ENTER) {
		mTrail = particleGenById(this, &gVecZero, 0, 0xE);
		mSound.setSe(0x7B);
		mSound.start();
		padVibStart((u8)((TObjAction*)mOwner)->mObjId, (int)this, 99, 0, 6);
	} else if (phase == MOBILE_EXIT) {
		((MobileTask*)mTrail)->mFlags |= 1;
		mSound.stop();
		vibStopOwner((u8)((TObjAction*)mOwner)->mObjId, this);
	} else if (phase == MOBILE_CHECK) {
		if (checkHazards() == 0) {
			if (isSinking() != 0) {
				sendSpecial();
			} else if (checkRail() != 0) {
				return;
			}
		}
	} else if (phase == MOBILE_UPDATE) {
		Vec3f pos;
		Vec3f vel;
		checkBumps();
		moveGround(mBodyParams->mAirCap);
		f32 speed = getMotion(&pos, &vel);
		setRollSound(speed, &pos);
		if (speed > 10.0f) {
			pos.y -= mBody.mRadius;
			((MobileTask*)mTrail)->mPos = pos;
			((MobileTask*)mTrail)->mFlags &= ~4;
		} else {
			((MobileTask*)mTrail)->mFlags |= 4;
		}
	}
}

/*
 * --INFO--
 * Address:	8008DF74
 * Size:	0001B8
 */
// 0x8008DF74
void TMobileObject::stateHop(int phase)
{
	if (phase == MOBILE_ENTER) {
		Vec3f pos;
		mFlags.mBits &= ~1;
		mFrames = 0;
		pos.x = mBody.mPos.x;
		pos.y = mBody.mPos.y;
		pos.z = mBody.mPos.z;
		pos.y += mBody.mRadius;
		mTrail = particleOnceById(this, &pos, 0, 0x38);
		*(f32**)((u8*)mTrail + 0x58) = mMatrix;
		mTrail2 = particleOnceById(this, &pos, 0, 0x39);
		*(f32**)((u8*)mTrail2 + 0x58) = mMatrix;
	} else if (phase == MOBILE_CHECK) {
		if (checkRail() == 0 && bounceOffWall() == 0 && (mFlags.mBits & 1) != 0) {
			f32 mtx[12];
			Vec3f pos;
			pos = mBody.mPos;
			f32 unit  = mBody.mRadius / 5.0f;
			f32 scale = 1.2f * unit;
			mtxSetTrans((float*)mtx, pos.x, 1.0f + (pos.y - mBody.mRadius), pos.z);
			mtxScale(scale, scale, scale, mtx, mtx);
			fxSpawnAt(1, 0, 0x17, mtx);
			pos.y += 5.0f;
			particleOnceById(gTL_PARTICLE, &pos, 0, 9);
			sendLanded();
			sePlay3D(0x84, &pos);
		}
	} else if (phase == MOBILE_UPDATE) {
		checkBumps();
		moveDefault();
		mFrames++;
	}
}

/*
 * --INFO--
 * Address:	8008E12C
 * Size:	000350
 *
 * Runs for mDashTime, or until it has been going five frames and has nearly stopped. The owner is told with message 0x4A.
 */
// 0x8008E12C
void TMobileObject::stateDash(int phase)
{
	if (phase == MOBILE_ENTER) {
		mTimer  = mParams->mDashTime;
		mTrail  = particleGenById(this, &gVecZero, 0, 0xF);
		mEffect = fxSpawnFollow(1, 0, 0x14, mTrailMatrix);
		if (mSpecial != 0) {
			mEffect2 = fxSpawnFollow(1, 0, 0x49, mTrailMatrix);
		} else {
			mEffect2 = 0;
		}
		Vec3f pos;
		pos = mBody.mPos;
		sePlay3D(0x7C, &pos);
		mSound.setSe(0x7B);
		mSound.start();
		padVibStart((u8)((TObjAction*)mOwner)->mObjId, (int)this, 99, 0, 7);
	} else if (phase == MOBILE_EXIT) {
		((MobileTask*)mTrail)->mFlags |= 1;
		fxStop(mEffect);
		fxStop(mEffect2);
		mSound.stop();
		vibStopOwner((u8)((TObjAction*)mOwner)->mObjId, this);
	} else if (phase == MOBILE_CHECK) {
		if (checkHazards() != 0) {
			return;
		}
		if (isSinking() != 0) {
			sendSpecial();
			return;
		}
		if (checkRail() != 0) {
			return;
		}
		if (tickTimer16((u16*)&mTimer) != 0) {
			ObjMsg msg;
			msg.mType = 0x4A;
			mOwner->sendToHandle(&msg);
		} else {
			Vec3f vel;
			vel = mBody.mVel;
			if (5 < (int)(mParams->mDashTime - (u16)mTimer) && vecLength(&vel) < 0.5f * mParams->mDashMinSpeed) {
				ObjMsg msg;
				msg.mType = 0x4A;
				mOwner->sendToHandle(&msg);
			}
		}
	} else if (phase == MOBILE_UPDATE) {
		Vec3f pos;
		Vec3f vel;
		checkBumps();
		moveGround(300.0f);
		f32 speed = getMotion(&pos, &vel);
		setRollSound(speed, &pos);
		pos.y -= mBody.mRadius;
		((MobileTask*)mTrail)->mPos = pos;
		if ((mTimer & 7) == 0) {
			f32 mtx[12];
			f32 scale = mBody.mRadius / 5.0f;
			mtxSetTransV(mtx, &pos);
			mtxRotateY(mtx, mtx, vecAngleXZ(&vel));
			mtxScale(scale, scale, scale, mtx, mtx);
			fxSpawnAt(0, 0, 0xF, mtx);
			if (mSpecial != 0) {
				pos = mBody.mPos;
				new MobileSpark(gTL_06, ((TObjAction*)mOwner)->mObjId, &pos, mBody.mRadius);
			}
		}
	}
}

/*
 * --INFO--
 * Address:	8008E47C
 * Size:	000114
 */
void TMobileObject::stateSlam(int phase)
{
	if (phase == MOBILE_ENTER) {
		mFlags.mBits &= ~1;
		mFrames = 0;
		mEffect = fxSpawnFollow(1, 0, 0x34, mDustMatrix);
	} else if (phase == MOBILE_EXIT) {
		fxStop(mEffect);
	} else if (phase == MOBILE_CHECK) {
		if (checkRail() == 0 && bounceOffWall() == 0 && (mFlags.mBits & 1) != 0) {
			spawnShockWave(1.3333334f);
			sendLanded();
			Vec3f pos;
			pos = mBody.mPos;
			sePlay3D(0x81, &pos);
			setState(MOBILE_SLAM_LAND);
		}
	} else if (phase == MOBILE_UPDATE) {
		checkBumps();
		moveDefault();
		updateDustMatrix();
		mFrames++;
	}
}

/*
 * --INFO--
 * Address:	8008E590
 * Size:	000088
 */
void TMobileObject::stateSlamLand(int phase)
{
	if (phase == MOBILE_ENTER) {
		mBody.mVel = gVecZero;
	} else if (phase != MOBILE_CHECK && phase == MOBILE_UPDATE) {
		checkBumps();
		moveDefault();
		mBody.mVel = gVecZero;
	}
}

/*
 * --INFO--
 * Address:	8008E618
 * Size:	000128
 */
void TMobileObject::stateUp(int phase)
{
	if (phase == MOBILE_ENTER) {
		mFrames = 0;
		mEffect = fxSpawnFollow(1, 0, 0x16, mDustMatrix);
		Vec3f pos;
		pos = mBody.mPos;
		sePlay3DHeld(400.0f, (SoundHandle*)&mLoop, 0x83, 200, (const Vec3f*)&pos);
	} else if (phase == MOBILE_EXIT) {
		seStop3D((SoundHandle*)&mLoop);
		fxStop(mEffect);
	} else if (phase == MOBILE_CHECK) {
		if (checkHazards() == 0 && checkRail() == 0 && (mFlags.mBits & 1) != 0) {
			slideOnGround();
			setState(MOBILE_DOWN);
			sendLanded();
		}
	} else if (phase == MOBILE_UPDATE) {
		checkBumps();
		steerInAir();
		moveAscent();
		mFrames++;
		updateDustMatrix();
		saveLoopPos();
	}
}

/*
 * --INFO--
 * Address:	8008E740
 * Size:	000088
 */
void TMobileObject::stateDown(int phase)
{
	if (phase == MOBILE_ENTER) {
		mBody.mAngVel = gVecZero;
	} else if (phase != MOBILE_CHECK && phase == MOBILE_UPDATE) {
		checkBumps();
		moveDown();
		mBody.mVel = gVecZero;
	}
}

/*
 * --INFO--
 * Address:	8008E7C8
 * Size:	000148
 */
// 0x8008E7C8
void TMobileObject::stateLaunch(int phase)
{
	if (phase == MOBILE_ENTER) {
		mFrames      = 0;
		mFacingBlend = 0.0f;
		mFlags.mBits &= ~0x20;
	} else if (phase == MOBILE_CHECK) {
		if (checkHazards() == 0 && checkRail() == 0 && (mFlags.mBits & 1) != 0) {
			if ((mFlags.mBits & 0x20) == 0) {
				Vec3f pos;
				*(volatile u32*)&mFlags.mBits |= 0x20;
				pos.x = mBody.mPos.x;
				pos.y = mBody.mPos.y;
				pos.z = mBody.mPos.z;
				pos.y -= mBody.mRadius;
				pos.y += 5.0f;
				particleOnceById(gTL_PARTICLE, &pos, 0, 0);
				sePlay3D(0x98, &pos);
			}
			sendLanded();
		}
	} else if (phase == MOBILE_UPDATE) {
		if (1.0f != mFacingBlend) {
			mFacingBlend += 1.0f / 30.0f;
			if (mFacingBlend > 1.0f) {
				mFacingBlend = 1.0f;
			}
		}
		checkBumps();
		steerInAir();
		moveDefault();
		mFrames++;
	}
}

/*
 * --INFO--
 * Address:	8008E910
 * Size:	00027C
 */
// 0x8008E910
void TMobileObject::stateDrive(int phase)
{
	if (phase == MOBILE_ENTER) {
		mTrail = particleGenById(this, &gVecZero, 0, 0xE);
		mSound.setSe(0x7B);
		mSound.start();
		padVibStart((u8)((TObjAction*)mOwner)->mObjId, (int)this, 99, 0, 6);
		mFlags.mBits &= ~0x20;
	} else if (phase == MOBILE_EXIT) {
		((MobileTask*)mTrail)->mFlags |= 1;
		mSound.stop();
		vibStopOwner((u8)((TObjAction*)mOwner)->mObjId, this);
		mRailNext.setPath(0);
	} else if (phase == MOBILE_CHECK) {
		if (checkHazards() != 0 || checkRail() != 0) {
			return;
		}
	} else if (phase == MOBILE_UPDATE) {
		Vec3f pos;
		Vec3f vel;
		steerOnGround();
		moveGround(mParams->mAirCap);
		f32 speed = getMotion(&pos, &vel);
		setRollSound(speed, &pos);

		if ((mFlags.mBits & 1) != 0) {
			if (speed > 10.0f) {
				pos.y -= mBody.mRadius;
				((MobileTask*)mTrail)->mPos = pos;
				((MobileTask*)mTrail)->mFlags &= ~4;
				vibResumeOwner((u8)((TObjAction*)mOwner)->mObjId, this);
			} else {
				((MobileTask*)mTrail)->mFlags |= 4;
				vibPauseOwner((u8)((TObjAction*)mOwner)->mObjId, this);
			}
			mSound.start();
		} else {
			((MobileTask*)mTrail)->mFlags |= 4;
			vibPauseOwner((u8)((TObjAction*)mOwner)->mObjId, this);
			mSound.stop();
		}
		if ((mFlags.mBits & 1) != 0 && (mFlags.mBits & 0x20) == 0) {
			Vec3f dust;
			*(volatile u32*)&mFlags.mBits |= 0x20;
			dust = mBody.mPos;
			dust.y -= mBody.mRadius;
			dust.y += 5.0f;
			particleOnceById(gTL_PARTICLE, &dust, 0, 0);
			rumble(3);
		}
	}
}

/*
 * --INFO--
 * Address:	8008EB8C
 * Size:	000284
 */
// 0x8008EB8C
void TMobileObject::stateThrown(int phase)
{
	if (phase == MOBILE_ENTER) {
		mTimer = mParams->mDriveTime;
		mTrail = particleGenById(this, &gVecZero, 0, 0xE);
		Vec3f pos;
		pos = mBody.mPos;
		sePlay3DHeld(400.0f, (SoundHandle*)&mLoop, 0x7B, 200, (const Vec3f*)&pos);
		mFlags.mBits &= ~0x20;
	} else if (phase == MOBILE_EXIT) {
		((MobileTask*)mTrail)->mFlags |= 1;
		seStop3D((SoundHandle*)&mLoop);
	} else if (phase == MOBILE_CHECK) {
		if (checkHazards() == 0 && checkRail() == 0) {
			if (tickTimer16((u16*)&mTimer) != 0) {
				ObjMsg msg;
				msg.mType = 0x4A;
				mOwner->sendToHandle(&msg);
				setState(MOBILE_DRIVE);
			} else {
				Vec3f vel;
				vel = mBody.mVel;
				if (njScalor(&vel) < mParams->mDriveMin) {
					ObjMsg msg;
					msg.mType = 0x4A;
					mOwner->sendToHandle(&msg);
					setState(MOBILE_DRIVE);
				}
			}
		}
	} else if (phase == MOBILE_UPDATE) {
		Vec3f pos;
		Vec3f vel;
		if ((mFlags.mBits & 1) != 0) {
			steerOnGround();
			moveGround(mParams->mDriveCap);
		} else {
			moveAirSpin();
		}
		f32 speed = getMotion(&pos, &vel);
		if (speed > 10.0f) {
			pos.y -= mBody.mRadius;
			((MobileTask*)mTrail)->mPos = pos;
			((MobileTask*)mTrail)->mFlags &= ~4;
		} else {
			((MobileTask*)mTrail)->mFlags |= 4;
		}
		saveLoopPos();
		if ((mFlags.mBits & 1) != 0 && (mFlags.mBits & 0x20) == 0) {
			Vec3f dust;
			*(volatile u32*)&mFlags.mBits |= 0x20;
			dust = mBody.mPos;
			dust.y -= mBody.mRadius;
			dust.y += 5.0f;
			particleOnceById(gTL_PARTICLE, &dust, 0, 0);
		}
	}
}

/*
 * --INFO--
 * Address:	8008EE10
 * Size:	0000D0
 */
// 0x8008EE10
void TMobileObject::stateLongJump(int phase)
{
	if (phase == MOBILE_ENTER) {
		Vec3f pos;
		pos = mBody.mPos;
		sePlay3DHeld(400.0f, (SoundHandle*)&mLoop, 0x82, 200, (const Vec3f*)&pos);
	} else if (phase == MOBILE_EXIT) {
		seStop3D((SoundHandle*)&mLoop);
	} else if (phase == MOBILE_CHECK) {
		if (checkHazards() == 0 && checkRail() == 0 && (mFlags.mBits & 1) != 0) {
			sendLanded();
		}
	} else if (phase == MOBILE_UPDATE) {
		steerInAir();
		moveThrown();
		saveLoopPos();
	}
}

/*
 * --INFO--
 * Address:	8008EEE0
 * Size:	000228
 *
 * Speed eases toward 100 or -100 while under 100. It leaves with message 0x51 at either end of the rail.
 */
// 0x8008EEE0
void TMobileObject::stateRail(int phase)
{
	if (phase == MOBILE_ENTER) {
		mBody.mAngVel = gVecZero;
		mBody.mAccel  = gVecZero;
		mBody.mVel    = gVecZero;
		mSound.setSe(0x8A);
		mSound.start();
		padVibStart((u8)((TObjAction*)mOwner)->mObjId, (int)this, 99, 0, 6);
	} else if (phase == MOBILE_EXIT) {
		mSound.stop();
		vibStopOwner((u8)((TObjAction*)mOwner)->mObjId, this);
	} else if (phase == MOBILE_CHECK) {
		// Off the rail or past either end of it
		if (mRail.mPath == 0 || (mRailSpeed < 0.0f && 0.0f == mRail.getDistance())
		    || (mRailSpeed >= 0.0f && mRail.getDistance() == mRail.mPath->getLength())) {
			Vec3f vel;
			mRail.getDir(&vel);
			vecScale(mRailSpeed, &vel, &vel);
			mBody.mVel = vel;
			mRail.setPath(0);
			ObjMsg msg;
			msg.mType = 0x51;
			mOwner->sendToHandle(&msg);
			setState(MOBILE_DRIVE);
		}
	} else if (phase == MOBILE_UPDATE) {
		Vec3f pos;
		Vec3f vel;
		f32 speed = mRailSpeed;
		f32 limit = 100.0f;
		if ((f32)__fabs(speed) < limit) {
			if (speed < 0.0f) {
				limit = -100.0f;
			}
			mRailSpeed = mobileLerp(0.5f, speed, limit);
		}
		moveRail();
		f32 level = getMotion(&pos, &vel);
		setRollSound(level, &pos);
	}
}

/*
 * --INFO--
 * Address:	8008F108
 * Size:	000004
 */
void TMobileObject::stateNone(int phase) { }

/*
 * --INFO--
 * Address:	8008F10C
 * Size:	000040
 */
int TMobileObject::sendHit(int kind)
{
	struct {
		u16 mType;
		u8 _02[2];
		u8 mKind;
	} msg;
	msg.mType = 0x4B;
	msg.mKind = kind;
	return mOwner->sendToHandle((ObjMsg*)&msg);
}

/*
 * --INFO--
 * Address:	8008F14C
 * Size:	000040
 */
int TMobileObject::sendCrush(int kind)
{
	struct {
		u16 mType;
		u8 _02[2];
		u8 mKind;
	} msg;
	msg.mType = 0x4C;
	msg.mKind = kind;
	return mOwner->sendToHandle((ObjMsg*)&msg);
}

/*
 * --INFO--
 * Address:	8008F18C
 * Size:	00003C
 */
void TMobileObject::sendLanded()
{
	ObjMsg msg;
	msg.mType = 0x4F;
	mOwner->sendToHandle(&msg);
}

/*
 * --INFO--
 * Address:	8008F1C8
 * Size:	00003C
 */
void TMobileObject::sendSpecial()
{
	ObjMsg msg;
	msg.mType = 0x53;
	mOwner->sendToHandle(&msg);
}

/*
 * --INFO--
 * Address:	8008F204
 * Size:	000060
 */
// 0x8008F204
void TMobileObject::setLaunchVel(f32 amount, int angle)
{
	Vec3f vel;
	vecFromAngleXZ(amount * mParams->mLaunchSpeed, &vel, angle);
	vel.y = mParams->mLaunchHeight;
	mBody.mVel = vel;
}

/*
 * --INFO--
 * Address:	8008F264
 * Size:	000088
 */
// 0x8008F264
void TMobileObject::steerInAir()
{
	f32 amount;
	int angle;
	if (getMove(&amount, &angle) != 0) {
		Vec3f force;
		amount *= mParams->mAirSteer;
		if (mBody.mVel.y < 0.0f) {
			amount *= 2.0f;
		}
		vecFromAngleXZ(amount, &force, angle);
		njAddVector(&mBody.mAccel, &force);
	}
}

/*
 * --INFO--
 * Address:	8008F2EC
 * Size:	000164
 *
 * Turns the velocity toward the stick. It turns at a tenth of the rate when the stick points up the surface.
 */
// 0x8008F2EC
void TMobileObject::steerOnGround()
{
	f32 amount;
	int angle;
	if (getMove(&amount, &angle) != 0 && mGround.mMesh != 0) {
		Vec3f normal;
		Vec3f vel;
		Vec3f dir;
		meshTriNormal(mGround.mMesh, mFace, &normal);
		vel = mBody.mVel;
		vecFromAngleXZ(1.0f, &dir, angle);
		f32 dot = njInnerProduct(&dir, &normal);
		vecScaleAdd(-dot, &dir, &normal, &dir);
		if (dir.y > 0.1f) {
			mobileTurnVector(&vel, &vel, &normal, angle, 0.1f * ((1.0f / 60.0f) * (f32)mParams->mTurn));
		} else {
			mobileTurnVector(&vel, &vel, &normal, angle, (1.0f / 60.0f) * (f32)mParams->mTurn);
		}
		mBody.mVel = vel;
	}
}

/*
 * --INFO--
 * Address:	8008F450
 * Size:	00002C
 */
int TMobileObject::msgPush(ObjMsg* msg)
{
	njAddVector(&mBody.mAccel, &((ObjVecMsg*)msg)->mVec);
	return 1;
}

/*
 * --INFO--
 * Address:	8008F47C
 * Size:	000008
 */
int TMobileObject::msgIgnore(ObjMsg* msg)
{
	return 0;
}

/*
 * --INFO--
 * Address:	8008F484
 * Size:	0000EC
 *
 * Attaches to part _06 of object _04, if the owner agrees with message 0x4D. Keeps the rotation relative to that part.
 */
// 0x8008F484
int TMobileObject::msgAttach(ObjMsg* msg)
{
	if ((int)(u16)mTarget.getId() == 0xFFFF) {
		ObjMsg ask;
		ask.mType = 0x4D;
		if (mOwner->sendToHandle(&ask) != 0) {
			MobileMsgAttach* attach = (MobileMsgAttach*)msg;
			u8 local[0x30];
			f32 part[12];
			mTarget.setId(attach->mObjId);
			mTargetPart = attach->mPart;
			findObjById(attach->mObjId)->getNodeMatrix(attach->mPart, part);
			mBody.makeMatrix((f32*)local);
			mtxInverse(part, part);
			mtxConcat((f32*)local, (const f32*)part, (const f32*)local);
			mtxToQuat((const f32*)local, (f32*)mAttachRot);
			setState(MOBILE_ATTACHED);
			return 1;
		}
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008F570
 * Size:	0000E0
 *
 * With the owner's agreement through message 0x4E.
 */
// 0x8008F570
int TMobileObject::msgLaunch(ObjMsg* msg)
{
	ObjMsg ask;
	ask.mType = 0x4E;
	if (mOwner->sendToHandle(&ask) != 0) {
		Vec3f* dir = &((ObjVecMsg*)msg)->mVec;
		mAim       = *dir;
		mBody.mAccel = gVecZero;
		mBody.mVel = *dir;
		mFlags.mBits &= ~0x20;
		mFlags.mBits &= ~1;
		mTarget.setId(0xFFFF);
		setState(MOBILE_LAUNCH);
		return 1;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008F650
 * Size:	0000C8
 *
 * With the owner's agreement through message 0x50.
 */
// 0x8008F650
int TMobileObject::msgFling(ObjMsg* msg)
{
	ObjMsg ask;
	ask.mType = 0x50;
	if (mOwner->sendToHandle(&ask) != 0) {
		Vec3f* dir = &((ObjVecMsg*)msg)->mVec;
		mAim       = *dir;
		mBody.mAccel = gVecZero;
		mBody.mVel = *dir;
		mTarget.setId(0xFFFF);
		setState(MOBILE_FLUNG);
		return 1;
	}
	return 0;
}

/*
 * --INFO--
 * Address:	8008F718
 * Size:	000028
 */
int TMobileObject::msgRumble2A(ObjMsg* msg)
{
	rumble(10);
	return 0;
}

/*
 * --INFO--
 * Address:	8008F740
 * Size:	000028
 */
int TMobileObject::msgRumble74(ObjMsg* msg)
{
	rumble(10);
	return 0;
}

/*
 * --INFO--
 * Address:	8008F768
 * Size:	00001C
 *
 * Each one counts as part of a combo for 100 frames. draw clears it.
 */
// 0x8008F768
int TMobileObject::msgCombo(ObjMsg* msg)
{
	mComboTimer = 100;
	return ++mCombo;
}

/*
 * --INFO--
 * Address:	8008F784
 * Size:	000044
 */
// 0x8008F784
u32 TMobileObject::msgStateFlags(ObjMsg* msg)
{
	u32 flags = 0;
	switch (mState) {
	case MOBILE_SLAM:
	case MOBILE_SLAM_LAND:
	case MOBILE_LAUNCH:
		flags |= 4;
		break;
	case MOBILE_ATTACHED:
		flags |= 0x2000;
		break;
	}
	return flags;
}

/*
 * --INFO--
 * Address:	8008F7C8
 * Size:	000068
 *
 * The bits for message 0x41: 1 on the ground, 0x10 always, and the rest by state.
 */
// 0x8008F7C8
u32 TMobileObject::msgAbilities(ObjMsg* msg)
{
	int grounded = mFlags.mBits & 1;
	u32 bits     = 0x10;
	if (grounded != 0) {
		bits |= 1;
	}
	switch (mState) {
	case MOBILE_IDLE:
		bits |= 8;
		break;
	case MOBILE_SLAM:
		bits |= 0xC;
		break;
	case MOBILE_SLAM_LAND:
	case MOBILE_DOWN:
		if (grounded != 0) {
			bits |= 2;
		}
		break;
	case MOBILE_LAUNCH:
		bits |= 8;
		break;
	}
	return bits;
}

/*
 * --INFO--
 * Address:	8008F830
 * Size:	000020
 */
int TMobileObject::msgGetVel(ObjMsg* msg)
{
	((ObjVecMsg*)msg)->mVec = mBody.mVel;
	return 1;
}

/*
 * --INFO--
 * Address:	8008F850
 * Size:	000020
 *
 * For checkRail. It stores the rail and the point, and ends the search.
 */
// 0x8008F850
extern "C" void mobileRailContact(RailPath* rail, const RailPoint* point, Vec3f* pos, void* info, int* stop)
{
	MobileRailInfo* out = (MobileRailInfo*)info;
	out->mRail          = rail;
	out->mPoint.mDist   = point->mDist;
	out->mPoint.mSeg    = point->mSeg;
	*stop               = 0;
}

/*
 * --INFO--
 * Address:	8008F870
 * Size:	00003C
 */
void TMobileObject::rumble(s16 strength)
{
	padVibStart((u8)((TObjAction*)mOwner)->mObjId, 0, 100, 0, strength);
}

/*
 * --INFO--
 * Address:	8008F8AC
 * Size:	0000F0
 *
 * The search reaches twice the radius, starting from one radius above the body.
 */
// 0x8008F8AC
int TMobileObject::findContact(u32* flags, Vec3f* pos, Vec3f* normal, f32* dist) const
{
	MobileWallResult result;
	Vec3f from;
	f32 radius = mBody.mRadius;
	from.x = mBody.mPos.x;
	from.y = mBody.mPos.y;
	from.z = mBody.mPos.z;
	from.y += radius;
	result.mModel = 0;
	result.mDist = 2.0f * radius;
	if (groundQueryDown((const void*)&from, (void*)0xC, (void*)mobileWallContact, (void*)&result) == 0) {
		return 0;
	}
	if (result.mModel == 0) {
		return 0;
	}
	*pos    = result.mPos;
	*normal = result.mNormal;
	*flags  = result.mFlags;
	*dist   = result.mDist;
	return 1;
}

/*
 * --INFO--
 * Address:	8008F99C
 * Size:	000088
 *
 * Turns dir around axis toward angle, by at most turn.
 */
// 0x8008F99C
extern "C" void mobileTurnVector(Vec3f* out, Vec3f* dir, Vec3f* axis, int angle, int turn)
{
	f32 quat[4];
	f32 mtx[12];
	int now = vecAngleXZ(dir);
	njCreateQuaternionV((const Vec3f*)axis, angleDiff(now, angleApproach(now, angle, turn)), (NJS_QUATERNION*)quat);
	mtxFromQuat(mtx, quat);
	mtxMultVecSR(mtx, dir, out);
}

/*
 * --INFO--
 * Address:	8008FA24
 * Size:	000024
 */
extern "C" f32 mobileLerp(f32 t, f32 from, f32 to)
{
	if (t > 1.0f) {
		t = 1.0f;
	}
	return from * (1.0f - t) + to * t;
}
