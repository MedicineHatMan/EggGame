#include "object/last/TObjLastBossShot.h"
struct DrawTask;
#include "object/ObjAction.h"
#include "camera/TCamera.h"
#include "object/ObjShadow.h"
#include "enemy/boss/TEneLastExBoss.h"
#include "engine/DrawState.h"
#include "engine/DrawTask.h"
#include "engine/FxSystem.h"
#include "core/MathUtil.h"
#include "sound/Sound3DPlay.h"
#include "engine/TParticle.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "engine/GroundQuery.h"
#include "egg/EggCreature.h"
#include "core/VecMath.h"
#include "egg/EggLevel.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjPlayer.h"
#include "enemy/boss/TEneLastExBossCore.h"
#include "core/TMainTask.h"
struct CharaColli;
struct Fx;

typedef char TObjLastBossShot_size_check[(sizeof(TObjLastBossShot) == 0x24C) ? 1 : -1];

const char* gTObjLastBossShotName = "TObjLastBossShot";

// 0x8018D7D0
TObjLastBossShot::TObjLastBossShot(TObject* parent, const Vec3f* pos)
    : TObjAction(parent)
{
	ColliShape* shape;

	_00    = (u32)gTObjLastBossShotName;
	mSize  = 0x24C;
	mOwner = 0xFFFF;
	mState = 9;
	setObjId(this, 0xFFFF);
	mColli = colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)mObjId);
		colliSetMasks((CharaColli*)mColli, 8, 0xF3, 0xF0, 0);
		shape = colliGetShape((CharaColli*)mColli, 0);
		colliShapeSetSphere(shape);
		shape->mFlags = 0x42C;
		colliShapeSetPos(shape, pos);
		shape->mSizeX = 15.0f;
	}
	mBody.mPos    = *pos;
	mBody.mRadius = 15.0f;
	updateMatrix();
	mRoll.mOwner   = (TObjEgg*)this;
	mRoll.mBody    = &mBody;
	mHandle.mOwner = this;
	mFx            = fxSpawnFollow(1, 5, 8, mMatrix);
	setState(SHOT_WAIT);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	shadowSetGround(&mShadow, pos, 0);
	mShadow._30 = 27.0f;
	mShadow._34 = 27.0f;
	shadowEnable(&mShadow);
	mDone = 0;
}

// 0x8018D9E4
TObjLastBossShot::~TObjLastBossShot()
{
	runState(mState, SHOT_EXIT);
	removeDrawTask(mDraw);
	fxStop((Fx*)mFx);
	colliFree((CharaColli*)mColli);
	unregisterId();
}

// 0x8018DAC8
int TObjLastBossShot::handleMessage(ObjMsg* msg)
{
	switch (msg->mType) {
	case 0x01:
		return onPush(msg);
	case 0x2A:
		return onKick(msg);
	case 0x40:
		return onGetType();
	case 0x41:
		return onGetFlags();
	case 0x43:
		return onGetVel(msg);
	case 0x64:
		return onThrow(msg);
	case 0x65:
		return onStrike(msg);
	case 0x69:
		return onDrop(msg);
	case 0x6A:
		return onPickUp(msg);
	case 0x71:
		return onSlam(msg);
	default:
		return 0;
	}
}

// 0x8018DBB4
int TObjLastBossShot::sendToHandle(ObjMsg* msg)
{
	return mHandle.send(msg);
}

// 0x8018DBD8
void TObjLastBossShot::draw()
{
	runState(mState, SHOT_CHECK);
	runState(mState, SHOT_UPDATE);
	if (mColli != 0) {
		colliShapeSetPos(colliGetShape(mColli, 0), &mPos);
	}
	EggHits hits;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&mPos, 1)) {
		shadowSetGround(&mShadow, &hits.mHits[0].mPos, &hits.mHits[0].mNormal);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
	if (mDone != 0) {
		mFlags |= 1;
	}
}

// 0x8018DCA8
void TObjLastBossShot::vfn_10()
{
}

// 0x8018DCAC
// Model 1 before it is hit 4 after
void TObjLastBossShot::render()
{
	int model;

	njPushMatrixEx();
	s16 state = mState;
	if (state == SHOT_WAIT || state == SHOT_FLY || state == SHOT_AIM) {
		model = 1;
	} else {
		model = 4;
	}
	njTranslateEx(&mPos);
	shadowAlphaBegin();
	drawObjectDefault(modelTEneLastExBoss(model));
	shadowAlphaEnd();
	njPopMatrixEx();
}

// 0x8018DD20
int TObjLastBossShot::onHit(const TObjAction* other, int arg, int* result)
{
	int ret      = 0;
	HitInfo* hit = (HitInfo*)result;

	if (other->mObjId == mHolderId) {
		return 0;
	}
	hit->mFlags.clear();
	hit->mFlags.setBit(0x10);
	switch (mState) {
	case SHOT_WAIT:
	case SHOT_AIM:
	case SHOT_FLY:
		hit->mFlags.clear();
		hit->mDamage = 10;
		return 1;
	case SHOT_THROWN:
	case SHOT_SLOW:
	case SHOT_BOUNCE:
		ret = roll()->hitWhileRolling(result, 10);
		break;
	case SHOT_HELD:
		ret = ((TMobileObject*)mHandle.mHolder)->takeHit(hit, 10);
		break;
	case SHOT_SLAM:
		ret = roll()->hitWhileHeld(result, 10);
		break;
	}
	if (ret != 0 && ((TObjAction*)other)->getKind() == 2) {
		mDone = 1;
	}
	return ret;
}

// 0x8018DE68
void TObjLastBossShot::runState(int state, int phase)
{
	switch (state) {
	case SHOT_WAIT:
		stateWait(phase);
		break;
	case SHOT_FLY:
		stateFly(phase);
		break;
	case SHOT_GROUND:
		stateGround(phase);
		break;
	case SHOT_HELD:
		stateHeld(phase);
		break;
	case SHOT_SLAM:
		stateSlam(phase);
		break;
	case SHOT_THROWN:
		stateThrown(phase);
		break;
	case SHOT_AIM:
		stateAim(phase);
		break;
	case SHOT_BOUNCE:
		stateBounce(phase);
		break;
	case SHOT_SLOW:
		stateSlow(phase);
		break;
	}
}

// 0x8018DF48
void TObjLastBossShot::setState(int state)
{
	if (mState != -1) {
		runState(mState, SHOT_EXIT);
	}
	if (state != -1) {
		runState(state, SHOT_ENTER);
	}
	mState = state;
}

// 0x8018DFA4
int TObjLastBossShot::isHolder(ShotThrowMsg* msg)
{
	return (u8)(msg->mHolder == mHolderId);
}

// 0x8018DFBC
int TObjLastBossShot::attachTo(ShotGrabMsg* msg)
{
	if (mHandle.mHolder == 0 && roll()->isGrounded()) {
		msg->mHandle = &mHandle;
		msg->mPos    = mBody.mPos;
		msg->mRot    = mBody.mRot;
		msg->mRadius = mBody.mRadius;
		mOwner       = msg->mHolder;
		mHolderId    = msg->mHolder;
		return 1;
	}
	return 0;
}

// 0x8018E068
void TObjLastBossShot::release()
{
	mOwner = 0xFFFF;
}

// 0x8018E078
void TObjLastBossShot::updateMatrix()
{
	Vec3f pos;
	EggRot rot;

	pos.x = mBody.mPos.x;
	pos.y = mBody.mPos.y;
	pos.z = mBody.mPos.z;
	rot   = mBody.mRot;

	mtxSetTransV(mMatrix, &pos);
	mtxConcatQuat((f32*)mMatrix, (const f32*)mMatrix, (const f32*)&rot);
	mPos.x = pos.x;
	mPos.y = pos.y;
	mPos.z = pos.z;
}

// 0x8018E10C
void TObjLastBossShot::stateWait(int phase)
{
	if (phase == SHOT_ENTER) {
		mTimer = 60;
	} else if (phase == SHOT_CHECK) {
		if (mTimer == 0) {
			setState(SHOT_AIM);
		}
	} else if (phase == SHOT_UPDATE) {
		if (mTimer != 0) {
			mTimer--;
		}
	}
}

// 0x8018E178
void TObjLastBossShot::stateHeld(int phase)
{
	if (phase == SHOT_UPDATE) {
		eggPushHeld((void*)mColli, (EggContact*)(((CharaColli*)mColli)->mContacts), 0, mHandle.mHolder);
		TMobileObject* holder = (TMobileObject*)mHandle.mHolder;
		Vec3f pos;
		EggRot rot;
		pos        = holder->mBody.mPos;
		rot        = holder->mBody.mRot;
		mBody.mPos = pos;
		mBody.mRot = rot;
		updateMatrix();
		roll()->checkBumps();
	}
}

// 0x8018E214
// A slam that lands sends out a shock wave
void TObjLastBossShot::stateSlam(int phase)
{
	Vec3f at;

	if (phase == SHOT_CHECK) {
		if (roll()->isSlamDone()) {
			setState(SHOT_GROUND);
		} else if (roll()->hasLanded()) {
			at.x = mPos.x;
			at.y = mPos.y - 15.0f;
			at.z = mPos.z;
			eggSpawnShockWave(15.0f, mObjId, mHolderId, &at, 10, 0);
			sePlay3D(0x81, &at);
			at.y += 5.0f;
			particleOnceById(gTL_PARTICLE, &at, 0, 9);
			camShakeNearSoft(200.0f, &mPos);
			setState(SHOT_GROUND);
		}
	} else if (phase == SHOT_UPDATE) {
		roll()->pushOff(mColli, ((CharaColli*)mColli)->mContacts, 0);
		roll()->moveSlam();
		updateMatrix();
	}
}

// 0x8018E324
void TObjLastBossShot::stateThrown(int phase)
{
	if (phase == SHOT_ENTER) {
		mTimer = playerGetParams(0)->mRollTime;
	} else if (phase == SHOT_CHECK) {
		if (roll()->isRollDone()) {
			setState(SHOT_GROUND);
		} else if (roll()->hitWall()) {
			roll()->rebound();
			setState(SHOT_BOUNCE);
		} else if (mTimer == 0) {
			roll()->stopRolling();
			setState(SHOT_SLOW);
		}
	} else if (phase == SHOT_UPDATE) {
		if (mTimer != 0) {
			mTimer--;
		}
		roll()->pushOff(mColli, ((CharaColli*)mColli)->mContacts, 0);
		roll()->moveRolling();
		updateMatrix();
	}
}

// 0x8018E41C
// Rises to 15 over the player then turns to them and flies
void TObjLastBossShot::stateAim(int phase)
{
	TObjAction* target;
	Vec3f at;

	if (phase == SHOT_ENTER) {
		mTimer = 60;
	} else if (phase == SHOT_CHECK) {
		if (mTimer == 0) {
			target = findObjById(0);
			if (target != 0) {
				at     = *target->getPos();
				mAngle = angleToXZ(&mPos, &at);
			} else {
				mAngle = 0;
			}
			mSpeed = 0.0f;
			setState(SHOT_FLY);
		}
	} else if (phase == SHOT_UPDATE) {
		target = findObjById(0);
		if (target != 0) {
			Vec3f* tp = target->getPos();
			mPos.y    = approach(6.0f, mPos.y, 15.0f + tp->y);
		}
		mtxSetTransV(mMatrix, &mPos);
		if (mTimer != 0) {
			mTimer--;
		}
	}
}

// 0x8018E53C
void TObjLastBossShot::stateGround(int phase)
{
	if (phase == SHOT_UPDATE) {
		roll()->pushOff(mColli, ((CharaColli*)mColli)->mContacts, 0);
		roll()->moveGround();
		updateMatrix();
	}
}

// 0x8018E590
void TObjLastBossShot::stateBounce(int phase)
{
	if (phase == SHOT_CHECK) {
		if (roll()->isRollDone()) {
			setState(SHOT_GROUND);
		} else if (roll()->isPastHolder()) {
			setState(SHOT_GROUND);
		}
	} else if (phase == SHOT_UPDATE) {
		roll()->pushOff(mColli, ((CharaColli*)mColli)->mContacts, 0);
		roll()->moveBounce();
		updateMatrix();
	}
}

// 0x8018E62C
// Five seconds of flight and it is gone
void TObjLastBossShot::stateFly(int phase)
{
	Vec3f step;

	if (phase == SHOT_ENTER) {
		mTimer = 300;
	} else if (phase == SHOT_CHECK) {
		if (mTimer == 0) {
			mFlags |= 1;
		}
	} else if (phase == SHOT_UPDATE) {
		mSpeed = approach(30.0f, mSpeed, 300.0f);
		if (mTimer != 0) {
			mTimer--;
		}
		vecFromAngleXZ(mSpeed, &step, mAngle);
		vecScaleAdd(0.016666668f, &mPos, &step, &mPos);
		mtxSetTransV(mMatrix, &mPos);
	}
}

// 0x8018E6EC
void TObjLastBossShot::stateSlow(int phase)
{
	if (phase == SHOT_ENTER) {
		mTimer = playerGetParams(0)->mSlowTime;
		sePlay3D(0x80, &mPos);
	} else if (phase == SHOT_CHECK) {
		if (roll()->isRollDone()) {
			setState(SHOT_GROUND);
		} else if (roll()->hitWall()) {
			roll()->reboundHard();
			setState(SHOT_BOUNCE);
		} else if (mTimer == 0) {
			roll()->reboundHard();
			setState(SHOT_BOUNCE);
		}
	} else if (phase == SHOT_UPDATE) {
		if (mTimer != 0) {
			mTimer--;
		}
		roll()->pushOff(mColli, ((CharaColli*)mColli)->mContacts, 0);
		roll()->moveSlow();
		updateMatrix();
	}
}

// 0x8018E7F0
int TObjLastBossShot::onPush(ObjMsg* msg)
{
	njAddVector(&mBody.mAccel, &((ShotVecMsg*)msg)->mVec);
	return 1;
}

// 0x8018E81C
int TObjLastBossShot::onKick(ObjMsg* msg)
{
	switch (mState) {
	case SHOT_GROUND:
		roll()->kick(((ShotThrowMsg*)msg)->mHolder);
		return 1;
	case SHOT_SLAM:
		roll()->kickHeld();
		setState(SHOT_GROUND);
		return 1;
	case SHOT_THROWN:
	case SHOT_SLOW:
	case SHOT_BOUNCE:
		roll()->reboundHard();
		setState(SHOT_BOUNCE);
		return 1;
	}
	return 0;
}

// 0x8018E8E0
int TObjLastBossShot::onGetType()
{
	return 0x3B;
}

// 0x8018E8E8
int TObjLastBossShot::onGetFlags()
{
	int flags = 0;

	switch (mState) {
	case SHOT_THROWN:
	case SHOT_SLOW:
	case SHOT_BOUNCE:
		flags |= 0x200;
		break;
	}
	return flags;
}

// 0x8018E918
int TObjLastBossShot::onGetVel(ObjMsg* msg)
{
	switch (mState) {
	case SHOT_GROUND:
	case SHOT_SLAM:
	case SHOT_THROWN:
	case SHOT_SLOW:
	case SHOT_BOUNCE:
		((ShotVecMsg*)msg)->mVec = mBody.mVel;
		return 1;
	case SHOT_FLY:
		vecFromAngleXZ(mSpeed, &((ShotVecMsg*)msg)->mVec, mAngle);
		return 1;
	}
	return 0;
}

// 0x8018E9C0
int TObjLastBossShot::onThrow(ObjMsg* msg)
{
	if (isHolder((ShotThrowMsg*)msg) == 0) {
		return 0;
	}
	if (mState == SHOT_HELD) {
		release();
		roll()->setVel(&((ShotThrowMsg*)msg)->mVel);
		setState(SHOT_THROWN);
		return 1;
	}
	return 0;
}

// 0x8018EA38
// Struck in flight it bursts and drops
int TObjLastBossShot::onStrike(ObjMsg*)
{
	f32 mtx[12];

	if (!(mState & 0x100)) {
		return 0;
	}
	if (mState == SHOT_FLY) {
		sePlay3D(0x1AD, &mPos);
		fxStop(mFx);
		mFx = fxSpawnFollow(1, 5, 0x10, mMatrix);
		mtxSetTransV(mtx, &mPos);
		mtxRotateY(mtx, mtx, (int)(3.141592f + (f32)mAngle));
		fxSpawnAt(1, 5, 0x1F, mtx);
		mBody.mPos = mPos;
		setState(SHOT_GROUND);
	}
	return 1;
}

// 0x8018EB30
int TObjLastBossShot::onPickUp(ObjMsg* msg)
{
	switch (mState) {
	case SHOT_GROUND:
		if (attachTo((ShotGrabMsg*)msg) != 0) {
			setState(SHOT_HELD);
			return 1;
		}
		return 0;
	case SHOT_THROWN:
	case SHOT_SLOW:
	case SHOT_BOUNCE:
		if (attachTo((ShotGrabMsg*)msg) != 0) {
			mBody.mVel = gVecZero;
			setState(SHOT_HELD);
			return 1;
		}
		return 0;
	}
	return 0;
}

// 0x8018EC10
int TObjLastBossShot::onDrop(ObjMsg* msg)
{
	if (isHolder((ShotThrowMsg*)msg) == 0) {
		return 0;
	}
	if (mState == SHOT_HELD) {
		release();
		setState(SHOT_GROUND);
		return 1;
	}
	return 0;
}

// 0x8018EC78
int TObjLastBossShot::onSlam(ObjMsg* msg)
{
	if (isHolder((ShotThrowMsg*)msg) == 0) {
		return 0;
	}
	if (mState == SHOT_HELD) {
		release();
		roll()->startSlam(&((ShotThrowMsg*)msg)->mVel);
		setState(SHOT_SLAM);
		return 1;
	}
	return 0;
}

// 0x8018ECF0
// Moves from toward to by rate a second
f32 TObjLastBossShot::approach(f32 rate, f32 from, f32 to)
{
	f32 t = 0.016666668f * rate;

	if (t > 1.0f) {
		t = 1.0f;
	}
	return from * (1.0f - t) + to * t;
}

// 0x8018ED1C
int TObjLastBossShot::getKind()
{
	return 1;
}

// 0x8018ED24
Vec3f* TObjLastBossShot::getPos()
{
	return &mPos;
}

// 0x8018ED2C
f32 TObjLastBossShot::getRadiusV()
{
	return mBody.mRadius;
}

// Not in the DOL calling the handler here before its body makes the compiler generate it in place
static int shotHandleMessage(ShotHandle* handle, ObjMsg* msg)
{
	return handle->ShotHandle::onMessage(msg);
}

// 0x8018ED34
int ShotHandle::onMessage(ObjMsg* msg)
{
	return ((TObjLastBossShot*)mOwner)->handleMessage(msg);
}
