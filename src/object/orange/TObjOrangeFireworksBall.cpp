#include "object/orange/TObjOrangeFireworksBall.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "object/ObjCommon.h"
#include "engine/CharaColli.h"
#include "ui/AeArc.h"
#include "io/NinjaFile.h"
#include "object/TObjExplode.h"
#include "object/ObjShadow.h"
#include "object/TObjLauncher.h"
#include "engine/Color.h"
#include "engine/DrawState.h"
#include "object/ObjAction.h"
#include "sound/Sound3DPlay.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "camera/TCameraCtrl.h"
#include "stage/TGameState.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "egg/EggCreature.h"
#include "stage/SetSlot.h"
#include "egg/EggLevel.h"
#include "object/orange/TObjOrangeFireworks.h"
struct NJS_TEXLIST;
struct ObjMsg;
struct SetLink;
struct CharaColli;
struct NJS_OBJECT;
struct DrawState;
struct DrawStateSlot;
struct ColliContact;

typedef char TObjOrangeFireworksBall_size_check[(sizeof(TObjOrangeFireworksBall) == 0x278) ? 1 : -1];

// 0x8012F054
extern "C" void loadFireworksBallArc()
{
	TArc* arc;
	loadArc("obj_ms_fwball.arc", &arc);
	if (arc != 0) {
		sOrangeFireworksBallModels = (NJS_OBJECT**)findArcExport(arc, "models");
		sOrangeFireworksBallTexLists = (NJS_TEXLIST**)findArcExport(arc, "texlists");
		void** textures = (void**)findArcExport(arc, "textures");
		bindGvmTextures((u8*)textures[0], sOrangeFireworksBallTexLists[0]);
	}
	sFireworkBallArc = arc;
}

// 0x8012F0D8
extern "C" void releaseFireworksBallArc()
{
	if (sOrangeFireworksBallTexLists != 0) {
		releaseTextures(sOrangeFireworksBallTexLists[0]);
	}
	if (sFireworkBallArc != 0) {
		releaseResource(sFireworkBallArc);
	}
	sFireworkBallArc = 0;
	sOrangeFireworksBallModels = 0;
	sOrangeFireworksBallTexLists = 0;
}

// 0x8012F128
extern "C" TArc* getFireworksBallArc()
{
	return sFireworkBallArc;
}

// 0x8012F130
extern "C" TObject* createTObjOrangeFireworksBall(TObject* parent, SetDesc* desc)
{
	return new TObjOrangeFireworksBall(parent, desc);
}

const char* gTObjOrangeFireworksBallName = "TObjOrangeFireworksBall";

// 0x8012F184
TObjOrangeFireworksBall::TObjOrangeFireworksBall(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
    , mState(6)
    , mOwner(0xFFFF)
{
	_00   = (u32)gTObjOrangeFireworksBallName;
	mSize = 0x278;
	setObjId(this, desc->mObjId);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 8, 0xF3, 0xF0, 0);
	colliShapeSetSphere(colliGetShape((CharaColli*)mShape, 0));
	colliGetShape((CharaColli*)mShape, 0)->mFlags = 0x42C;
	colliGetShape((CharaColli*)mShape, 0)->mSizeX = 7.5f;
	mDrawTask     = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mBody.mBounce     = 0.0f;
	mBody.mRadius = 7.5f;
	roll()->mOwner = (TObjEgg*)this;
	roll()->mBody  = &mBody;
	mHandle.mOwner = this;
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
	mShadow._30 = 15.0f;
	mShadow._34 = 15.0f;
	setState(BALL_GROUND);
}

// 0x8012F388
TObjOrangeFireworksBall::~TObjOrangeFireworksBall()
{
	removeDrawTask(mDrawTask);
	colliFree((CharaColli*)mShape);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	unregisterId();
}

// 0x8012F47C
int TObjOrangeFireworksBall::sendToHandle(ObjMsg* msg)
{
	return mHandle.send(msg);
}

// 0x8012F4A0
int TObjOrangeFireworksBall::handleMessage(ObjMsg* msg)
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

// 0x8012F578
// The record position is lifted by the radius then copied onto the body and the shape
void TObjOrangeFireworksBall::initFromDesc(SetDesc* desc)
{
	Vec3f at;
	desc->getPos(&at);
	at.y += mBody.mRadius;
	mPos       = at;
	mBody.mPos = at;
	colliShapeSetPos(colliGetShape(mShape, 0), &at);
}

// 0x8012F60C
void TObjOrangeFireworksBall::draw()
{
	if (debugEditActive() != 0) {
		mBody.makeMatrix(mPlace);
		mtxScale(1.0f, 1.0f, 1.0f, mDrawMtx, mPlace);
		return;
	}
	if ((mBusy.mBits & 8) == 0) {
		if (mPos.y < -40.0f) {
			s16 msg = 0x9F;
			sendGameMsg((ObjMsg*)&msg);
			setLinkDone((SetLink*)&mLink);
			mFlags |= 1;
			return;
		}
		if (checkTouches() != 0) {
			setState(BALL_BURST);
			return;
		}
	}
	runState(mState, 3);
	runState(mState, 2);
}

// 0x8012F6F4
void TObjOrangeFireworksBall::vfn_10()
{
	if (cameraIsVisible(1000.0f, mBody.mRadius, &mPos, 0) == 0) {
		mDrawTask->mFlags |= 1;
	} else {
		mDrawTask->mFlags &= 0xFFFE;
	}
}

// 0x8012F75C
void TObjOrangeFireworksBall::render()
{
	f32 view[12];
	mtxConcat(view, gjGetMatrix(), mDrawMtx);
	if (sOrangeFireworksBallModels == 0) {
		debugWireSphere(view, &gColorBlack);
		return;
	}
	DrawStateSlot* slot = getDrawStateSlot();
	shadowAlphaBegin();
	njPushMatrix(view);
	beginDrawState(slot, getDefaultDrawState());
	njSetTexture(*sOrangeFireworksBallTexLists);
	gjDrawObject(sOrangeFireworksBallModels[0]);
	endDrawState(slot);
	njPopMatrixEx();
	shadowAlphaEnd();
}

// 0x8012F800
void TObjOrangeFireworksBall::runState(int state, int phase)
{
	switch (state) {
	case BALL_GROUND:
		stateGround(phase);
		break;
	case BALL_HELD:
		stateHeld(phase);
		break;
	case BALL_SLAM:
		stateSlam(phase);
		break;
	case BALL_ROLLING:
		stateRolling(phase);
		break;
	case BALL_BURST:
		stateBurst(phase);
		break;
	case BALL_BOUNCE:
		stateBounce(phase);
		break;
	}
}

// 0x8012F894
void TObjOrangeFireworksBall::setState(int state)
{
	if (mState != -1) {
		runState(mState, 1);
	}
	if (state != -1) {
		runState(state, 0);
	}
	mState = state;
}

// 0x8012F8F4
int TObjOrangeFireworksBall::getHolderFlags()
{
	if (mHandle.mHolder != 0) {
		ObjMsg msg;
		msg.mType = 0x41;
		return mHandle.send(&msg);
	}
	return 0;
}

// 0x8012F938
int TObjOrangeFireworksBall::isHolder(FireworksThrowMsg* msg)
{
	return msg->mHolder == mHolderId;
}

// 0x8012F950
int TObjOrangeFireworksBall::attachTo(FireworksHoldMsg* msg)
{
	if (mHandle.mHolder == 0) {
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

// 0x8012F9C4
void TObjOrangeFireworksBall::release()
{
	mOwner = 0xFFFF;
}

// 0x8012F9D4
void TObjOrangeFireworksBall::updateMatrices()
{
	mBody.makeMatrix(mPlace);
	f32 scale = mBody.mRadius / 7.5f;
	mtxScale(scale, scale, scale, mDrawMtx, mPlace);
}

// 0x8012FA28
void TObjOrangeFireworksBall::updateGround()
{
	mPos = mBody.mPos;
	colliShapeSetPos(colliGetShape(mShape, 0), &mPos);

	EggHits hits;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&mPos, -1) != 0 && (hits.mMask & 1) != 0) {
		shadowSetGround(&mShadow, &hits.mHits[0].mPos, &hits.mHits[0].mNormal);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
}

// 0x8012FAE0
int TObjOrangeFireworksBall::checkTouches()
{
	HitInfo info;
	ColliContact* touch;

	for (touch = mShape->mContacts; touch != 0; touch = *(ColliContact**)touch) {
		TObjAction* other = findObjById(colliContactOwner(touch));
		if (other != 0) {
			if (other->getKind() == 2 && !(colliContactShape(touch)->mFlags & 0x10)) {
				return 1;
			}
			if ((colliContactShape(touch)->mFlags & 8) && other->onHit(this, colliContactTag(touch), (int*)&info)
			    && info.mFlags.testBit(13)) {
				return 1;
			}
		}
	}
	return 0;
}

// 0x8012FBDC
void TObjOrangeFireworksBall::remove()
{
	setLinkDone((SetLink*)&mLink);
	mFlags |= 1;
}

// 0x8012FC18
void TObjOrangeFireworksBall::stateHeld(int phase)
{
	if (phase == 2) {
		eggPushHeld((void*)mShape, (EggContact*)mShape->mContacts, 0, mHandle.mHolder);
		TMobileObject* holder = (TMobileObject*)mHandle.mHolder;
		Vec3f pos;
		EggRot rot;
		pos = holder->mBody.mPos;
		rot = holder->mBody.mRot;
		mBody.mPos = pos;
		mBody.mRot = rot;
		updateMatrices();
		roll()->checkBumps();
		updateGround();
	}
}

// 0x8012FCBC
void TObjOrangeFireworksBall::stateSlam(int phase)
{
	if (phase == 3) {
		if (roll()->isSlamDone() != 0) {
			setState(BALL_GROUND);
		} else if (roll()->hasLanded() != 0) {
			Vec3f pos;
			pos.x = mPos.x;
			pos.y = mPos.y - mBody.mRadius;
			pos.z = mPos.z;
			eggSpawnShockWave(mBody.mRadius, mObjId, mHolderId, &pos, 10, 0);
			sePlay3D(0x81, &pos);
			camShakeNearSoft(200.0f, &mPos);
			setState(BALL_GROUND);
		}
	} else if (phase == 2) {
		roll()->moveSlam();
		updateMatrices();
		updateGround();
	}
}

// 0x8012FD9C
void TObjOrangeFireworksBall::stateRolling(int phase)
{
	if (phase == 3) {
		if (roll()->isRollDone() != 0) {
			setState(BALL_GROUND);
		} else if (roll()->hitWall() != 0) {
			mBody.mVel = gVecZero;
			setState(BALL_GROUND);
		}
	} else if (phase == 2) {
		roll()->pushOff(mShape, mShape->mContacts, 0);
		roll()->moveRolling();
		updateMatrices();
		updateGround();
	}
}

// 0x8012FE60
void TObjOrangeFireworksBall::stateBurst(int phase)
{
	if (phase == 0) {
		mBusy.mBits |= 8;
		mTimer = 0;
		colliDisable(mShape);
		shadowDisable(&mShadow);
		mFlags |= 0x10;
		spawnExplode(&mPos, mHolderId, 0);
	} else if (phase == 3) {
		if ((f32)mTimer == 120.0f) {
			s16 msg = 0x9F;
			sendGameMsg((ObjMsg*)&msg);
			setLinkDone((SetLink*)&mLink);
			mFlags |= 1;
		}
	} else if (phase == 2) {
		mTimer++;
	}
}

// 0x8012FF40
void TObjOrangeFireworksBall::stateGround(int phase)
{
	if (phase == 2) {
		roll()->pushOff(mShape, mShape->mContacts, 0);
		roll()->moveGround();
		updateMatrices();
		updateGround();
	}
}

// 0x8012FF9C
void TObjOrangeFireworksBall::stateBounce(int phase)
{
	if (phase == 3) {
		if (roll()->isRollDone() != 0) {
			setState(BALL_GROUND);
		}
	} else if (phase == 2) {
		roll()->pushOff(mShape, mShape->mContacts, 0);
		roll()->moveBounce();
		updateMatrices();
		updateGround();
	}
}

// 0x80130020
int TObjOrangeFireworksBall::onPush(ObjMsg* msg)
{
	njAddVector(&mBody.mAccel, &((ObjVecMsg*)msg)->mVec);
	return 1;
}

// 0x8013004C
int TObjOrangeFireworksBall::onKick(ObjMsg* msg)
{
	switch (mState) {
	case BALL_GROUND:
		roll()->kick(((EggKickMsg*)msg)->mAngle);
		return 1;
	case BALL_SLAM:
		roll()->kickHeld();
		setState(BALL_GROUND);
		return 1;
	case BALL_ROLLING:
		roll()->reboundHard();
		setState(BALL_GROUND);
		return 1;
	}
	return 0;
}

// 0x801300F0
int TObjOrangeFireworksBall::onGetType()
{
	return 0x1B;
}

// 0x801300F8
int TObjOrangeFireworksBall::onGetFlags()
{
	int flags = getHolderFlags();
	switch ((u8)mState) {
	case BALL_ROLLING:
			flags |= 0x200;
		break;
	}
	return flags;
}

// 0x80130138
int TObjOrangeFireworksBall::onGetVel(ObjMsg* msg)
{
	((ObjVecMsg*)msg)->mVec = mBody.mVel;
	return 1;
}

// 0x80130158
int TObjOrangeFireworksBall::onThrow(ObjMsg* msg)
{
	if (isHolder((FireworksThrowMsg*)msg) == 0) {
		return 0;
	}
	if ((int)(u8)mState == BALL_HELD) {
		release();
		roll()->setVel(&((FireworksThrowMsg*)msg)->mVel);
		setState(BALL_ROLLING);
		return 1;
	}
	return 0;
}

// 0x801301D0
int TObjOrangeFireworksBall::onPickUp(ObjMsg* msg)
{
	switch (mState) {
	case BALL_GROUND:
		setState(BALL_HELD);
		return attachTo((FireworksHoldMsg*)msg);
	case BALL_ROLLING:
		if (attachTo((FireworksHoldMsg*)msg) != 0) {
			mBody.mVel = gVecZero;
			setState(BALL_HELD);
			return 1;
		}
		return 0;
	}
	return 0;
}

// 0x80130280
int TObjOrangeFireworksBall::onDrop(ObjMsg* msg)
{
	if (isHolder((FireworksThrowMsg*)msg) == 0) {
		return 0;
	}
	if ((int)(u8)mState == BALL_HELD) {
		release();
		setState(BALL_GROUND);
		return 1;
	}
	return 0;
}

// 0x801302E8
int TObjOrangeFireworksBall::onSlam(ObjMsg* msg)
{
	if (isHolder((FireworksThrowMsg*)msg) == 0) {
		return 0;
	}
	if ((int)(u8)mState == BALL_HELD) {
		release();
		roll()->startSlam(&((FireworksThrowMsg*)msg)->mVel);
		setState(BALL_SLAM);
		return 1;
	}
	return 0;
}

// 0x80130360
int TObjOrangeFireworksBall::getKind()
{
	return 1;
}

// 0x80130368
Vec3f* TObjOrangeFireworksBall::getPos()
{
	return &mPos;
}

// 0x80130370
SetLink* TObjOrangeFireworksBall::getLink()
{
	return mLink.get();
}

// 0x80130378
f32 TObjOrangeFireworksBall::getRadiusV()
{
	return mBody.mRadius;
}

// Not in the DOL calling the handler here before its body makes the compiler generate it in place
static int fireworksHandleMessage(FireworksBallHandle* handle, ObjMsg* msg) { return handle->FireworksBallHandle::onMessage(msg); }

// 0x80130380
int FireworksBallHandle::onMessage(ObjMsg* msg)
{
	return ((TObjOrangeFireworksBall*)mOwner)->handleMessage(msg);
}
