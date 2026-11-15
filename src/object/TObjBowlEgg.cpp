#include "engine/TMobileObject.h"
#include "object/ObjAction.h"
#include "egg/ObjEgg.h"
#include "object/ObjCommon.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/TObjGameBowling.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "io/NinjaFile.h"
#include "object/TObjLauncher.h"
#include "object/TObjBowlEgg.h"
#include "camera/TCamera.h"
#include "object/ObjArchive.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjEggRing.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "stage/SetSlot.h"
#include "egg/EggLevel.h"
struct SetLink;
struct CharaColli;
struct NJS_OBJECT;
struct NJS_TEXLIST;

enum BowlState {
	BOWL_GROUND  = 0,
	BOWL_HELD    = 1,
	BOWL_SLAM    = 2,
	BOWL_ROLLING = 3,
	BOWL_BROKEN  = 4,
	BOWL_PATH    = 5,
	BOWL_CARRIED = 6, // on the lane's node
	BOWL_BOUNCE  = 7,
};

enum BowlPhase {
	BOWL_ENTER  = 0,
	BOWL_EXIT   = 1,
	BOWL_UPDATE = 2,
	BOWL_CHECK  = 3,
};

/** mBits 4 = break now 8 = broken */
enum BowlFlags {
	BOWL_FLAG_BREAK  = 4,
	BOWL_FLAG_BROKEN = 8,
};

struct BowlPathMsg {
	u16 mType;   // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	void* mPath; // _08
	void* mPoint; // _0C
	f32 mSpeed;  // _10
};

struct BowlVecMsg {
	u16 mType;  // _00
	u8 _02[2];
	Vec3f mVec; // _04
};

typedef char TObjBowlEgg_size_check[(sizeof(TObjBowlEgg) == 0x24C) ? 1 : -1];

static void bowlTev(u8 stage);   // 0x8009F590
static void bowlTexGen(u8 coord); // 0x8009F640
static void* bowlModel();         // 0x8009F6EC
static void* bowlTexList();       // 0x8009F710

const char* gTObjBowlEggName = "TObjBowlEgg";

// 0x8009E10C
TObjBowlEgg::TObjBowlEgg(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	_00     = (u32)gTObjBowlEggName;
	mSize   = 0x24C;
	mOwner  = 0xFFFF;
	mLane   = 0xFFFF;
	mState  = -1;
	setObjId(this, desc->mObjId);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 8, 0xF3, 0xF0, 0);
	colliShapeSetSphere(colliGetShape((CharaColli*)mShape, 0));
	colliGetShape((CharaColli*)mShape, 0)->mFlags = 0x42C;
	colliGetShape((CharaColli*)mShape, 0)->mSizeX = 7.5f;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mBody.mBounce = 0.0f;
	mBody.mRadius     = 7.5f;
	EggRoll* r     = roll();
	r->mOwner      = (TObjEgg*)this;
	r->mPath       = &mCollide;
	r->mBody       = &mBody;
	mHandle.mOwner = this;
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
	setState(BOWL_GROUND);
}

// 0x8009E308
TObjBowlEgg::~TObjBowlEgg()
{
	removeDrawTask(mDraw);
	colliFree((CharaColli*)mShape);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	unregisterId();
}

// 0x8009E3F0
extern "C" TObject* createTObjBowlEgg(TObject* parent, SetDesc* desc)
{
	return new TObjBowlEgg(parent, desc);
}

/** What carryOn tells the lane 0x1C */
struct BowlLaneMsg {
	u16 mType;   // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
};

// 0x8009E444
// Rides the carrier's node from here on kept at the turn it has now
void TObjBowlEgg::carryOn(TObjAction* carrier, u16 node)
{
	f32 bodyMtx[12];
	f32 nodeMtx[12];
	TObjAction* lane;
	BowlLaneMsg msg;

	mCarrier = carrier->mObjId;
	mNode    = node;
	carrier->getNodeMatrix(node, nodeMtx);
	mBody.makeMatrix(bodyMtx);
	mtxInverse(nodeMtx, nodeMtx);
	mtxConcat(bodyMtx, nodeMtx, nodeMtx);
	mtxToQuat((const f32*)bodyMtx, (f32*)&mCarryRot);
	setState(BOWL_CARRIED);
	lane = findObjById(mOwner);
	if (lane != 0) {
		msg.mType = 0x1C;
		lane->sendToHandle((ObjMsg*)&msg);
	}
	mLane = carrier->mObjId;
}

// 0x8009E50C
void TObjBowlEgg::launch(Vec3f* vel)
{
	roll()->setVel(vel);
	setState(BOWL_ROLLING);
}

// 0x8009E548
void TObjBowlEgg::kill() { mFlags |= 1; }

// 0x8009E558
int TObjBowlEgg::sendToHandle(ObjMsg* msg) { return mHandle.send(msg); }

// 0x8009E57C
int TObjBowlEgg::handleMessage(ObjMsg* msg)
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
	case 0x6B:
		return onPath(msg);
	case 0x71:
		return onSlam(msg);
	default:
		return 0;
	}
}

// 0x8009E668
void TObjBowlEgg::initFromDesc(SetDesc* desc)
{
	Vec3f pos;

	desc->getPos(&pos);
	setPos(&pos);
	colliShapeSetPos(colliGetShape(mShape, 0), &pos);
}

// 0x8009E6D4
void TObjBowlEgg::setPos(const Vec3f* pos)
{
	mPos       = *pos;
	mBody.mPos = *pos;
}

// 0x8009E6FC
int TObjBowlEgg::onHit(const TObjAction* other, int, int* result)
{
	if (other->mObjId == mHolderId) {
		return 0;
	}
	switch (mState) {
	case BOWL_ROLLING:
	case BOWL_BOUNCE:
		return roll()->hitWhileRolling(result, 10);
	case BOWL_HELD:
		return ((TMobileObject*)mHandle.mHolder)->takeHit((HitInfo*)result, 10);
	case BOWL_SLAM:
		return roll()->hitWhileHeld(result, 10);
	}
	return 0;
}

// 0x8009E79C
void TObjBowlEgg::draw()
{
	if (debugEditActive()) {
		mBody.makeMatrix(mBodyMtx);
		mtxScale(1.0f, 1.0f, 1.0f, mMtx, mBodyMtx);
		return;
	}
	if (!(mBowlFlags.mBits & BOWL_FLAG_BROKEN)) {
		if (mBowlFlags.mBits & BOWL_FLAG_BREAK) {
			breakEgg();
		} else if (roll()->getMask() & 0x100) {
			breakEgg();
		} else if (eggFirstDamage((void*)mShape, (EggContact*)(((CharaColli*)mShape)->mContacts), 0, (void*)this)) {
			breakEgg();
			return;
		}
	}
	runState(mState, BOWL_CHECK);
	runState(mState, BOWL_UPDATE);
	mPos.x = mBody.mPos.x;
	mPos.y = mBody.mPos.y;
	mPos.z = mBody.mPos.z;
	colliShapeSetPos(colliGetShape(mShape, 0), &mPos);
}

// 0x8009E8A8
void TObjBowlEgg::vfn_10()
{
	if (!cameraIsVisible(1000.0f, mBody.mRadius, &mPos, 0)) {
		mDraw->mFlags |= 1;
	} else {
		mDraw->mFlags &= ~1;
	}
}

// 0x8009E910
// The model with a second environment-mapped stage
void TObjBowlEgg::render()
{
	f32 view[12];
	f32 tex[12];

	mtxConcat(view, gjGetMatrix(), mMtx);
	njPushMatrix(view);
	njSetTexture(bowlTexList());
	gjLoadTextureNum(1, 0, 0, 1);
	PSMTXCopy(_gj_matrix_, tex);
	tex[3]  = 0.0f;
	tex[7]  = 0.0f;
	tex[11] = 0.0f;
	GXLoadTexMtxImm(tex, GX_TEXMTX0, GX_MTX3x4);
	gjSetNumPostTexGens(1);
	gjSetNumPostTevStages(1);
	gjSetPostTevStageCallback((void*)bowlTev);
	gjSetPostTexGenCallback((void*)bowlTexGen);
	_nj_control_3d_flag_ |= 0x80000;
	gjDrawObject((NJS_OBJECT*)bowlModel());
	gjSetNumPostTexGens(0);
	gjSetNumPostTevStages(0);
	_nj_control_3d_flag_ &= ~0x80000;
	njPopMatrixEx();
}

// 0x8009E9F8
void TObjBowlEgg::runState(int state, int phase)
{
	switch (state) {
	case BOWL_GROUND:
		stateGround(phase);
		break;
	case BOWL_HELD:
		stateHeld(phase);
		break;
	case BOWL_SLAM:
		stateSlam(phase);
		break;
	case BOWL_ROLLING:
		stateRolling(phase);
		break;
	case BOWL_BROKEN:
		stateBroken(phase);
		break;
	case BOWL_PATH:
		statePath(phase);
		break;
	case BOWL_CARRIED:
		stateCarried(phase);
		break;
	case BOWL_BOUNCE:
		stateBounce(phase);
		break;
	}
}

// 0x8009EA90
void TObjBowlEgg::setState(int state)
{
	if (mState != -1) {
		runState(mState, BOWL_EXIT);
	}
	if (state != -1) {
		runState(state, BOWL_ENTER);
	}
	mState = state;
}

// 0x8009EAEC
// The holder's flags when something holds it
int TObjBowlEgg::getMsgFlags()
{
	ObjMsg msg;

	if (mHandle.mHolder != 0) {
		msg.mType = 0x41;
		return mHandle.send(&msg);
	}
	return 0;
}

// 0x8009EB30
int TObjBowlEgg::isHolder(BowlThrowMsg* msg) { return (u8)(msg->mHolder == mHolderId); }

// 0x8009EB48
int TObjBowlEgg::attachTo(BowlGrabMsg* msg)
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

// 0x8009EBBC
void TObjBowlEgg::release() { mOwner = 0xFFFF; }

// 0x8009EBCC
void TObjBowlEgg::updateMatrix()
{
	mBody.makeMatrix(mBodyMtx);
	mtxCopy(mMtx, mBodyMtx);
}

// 0x8009EC0C
// Takes a path it is on if there is one
int TObjBowlEgg::tryPath()
{
	u32 point;
	void* path;

	if (roll()->findPath(&path, &point)) {
		roll()->attachPath(path, &point);
		return 1;
	}
	return 0;
}

// 0x8009EC68
void TObjBowlEgg::breakEgg()
{
	mHandle.release();
	mBowlFlags.mBits |= BOWL_FLAG_BROKEN;
	setState(BOWL_BROKEN);
}

// 0x8009ECB0
void TObjBowlEgg::stateHeld(int phase)
{
	if (phase == BOWL_UPDATE) {
		eggPushHeld((void*)mShape, (EggContact*)(((CharaColli*)mShape)->mContacts), 0, mHandle.mHolder);
		TMobileObject* holder = (TMobileObject*)mHandle.mHolder;
		Vec3f pos;
		EggRot rot;
		pos = holder->mBody.mPos;
		rot = holder->mBody.mRot;
		mBody.mPos = pos;
		mBody.mRot = rot;
		updateMatrix();
		roll()->checkBumps();
	}
}

// 0x8009ED4C
void TObjBowlEgg::stateSlam(int phase)
{
	Vec3f at;

	if (phase == BOWL_CHECK) {
		if (roll()->isSlamDone()) {
			setState(BOWL_GROUND);
		} else if (tryPath()) {
			setState(BOWL_PATH);
		} else if (roll()->hasLanded()) {
			at.x = mPos.x;
			at.y = mPos.y - mBody.mRadius;
			at.z = mPos.z;
			eggSpawnShockWave(mBody.mRadius, mObjId, mHolderId, &at, 10, 0);
			camShakeNearSoft(200.0f, &mPos);
			setState(BOWL_GROUND);
		}
	} else if (phase == BOWL_UPDATE) {
		roll()->moveSlam();
		updateMatrix();
	}
}

// 0x8009EE38
// A wall stops it dead and gives it back to the lane
void TObjBowlEgg::stateRolling(int phase)
{
	TObjAction* lane;

	if (phase == BOWL_CHECK) {
		if (roll()->isRollDone()) {
			setState(BOWL_GROUND);
		} else if (tryPath()) {
			setState(BOWL_PATH);
		} else if (roll()->hitWall()) {
			mBody.mVel = gVecZero;
			if (mLane != 0xFFFF && (lane = findObjById(mLane)) != 0) {
				((TObjGameBowling*)lane)->releaseEgg(this);
			}
			setState(BOWL_GROUND);
		}
	} else if (phase == BOWL_UPDATE) {
		roll()->pushOff(mShape, ((CharaColli*)mShape)->mContacts, 0);
		roll()->moveRolling();
		updateMatrix();
	}
}

// 0x8009EF30
// Blinks for a second then is gone
void TObjBowlEgg::stateBroken(int phase)
{
	if (phase == BOWL_ENTER) {
		mTimer = 60;
	} else if (phase == BOWL_CHECK) {
		if (tickTimer32(&mTimer)) {
			setLinkTimed((SetLink*)&mLink, 300);
			mFlags |= 1;
		}
	} else if (phase == BOWL_UPDATE) {
		if (mTimer & 1) {
			mFlags &= ~0x10;
		} else {
			mFlags |= 0x10;
		}
	}
}

// 0x8009EFD0
void TObjBowlEgg::stateGround(int phase)
{
	if (phase == BOWL_UPDATE) {
		roll()->pushOff(mShape, ((CharaColli*)mShape)->mContacts, 0);
		roll()->moveGround();
		updateMatrix();
	}
}

// 0x8009F024
void TObjBowlEgg::statePath(int phase)
{
	if (phase == BOWL_CHECK) {
		if (roll()->isPathDone()) {
			roll()->leavePath();
			setState(BOWL_GROUND);
		}
	} else if (phase == BOWL_UPDATE) {
		roll()->movePath();
		updateMatrix();
	}
}

// 0x8009F094
void TObjBowlEgg::stateCarried(int phase)
{
	f32 mtx[12];
	Vec3f pos;
	EggRot rot;

	if (phase == BOWL_UPDATE) {
		findObjById(mCarrier)->getNodeMatrix(mNode, mtx);
		mtxConcatQuat((f32*)mtx, (const f32*)mtx, (const f32*)&mCarryRot);
		mtxTranslate(mMtx, mtx, 0.0f, 7.5f, 0.0f);
		mtxGetPos(mMtx, &pos);
		mtxToQuat((const f32*)mMtx, (f32*)&rot);
		mBody.mPos.x = pos.x;
		mBody.mPos.y = pos.y;
		mBody.mPos.z = pos.z;
		mBody.mRot   = rot;
	}
}

// 0x8009F15C
void TObjBowlEgg::stateBounce(int phase)
{
	if (phase == BOWL_CHECK) {
		if (roll()->isRollDone()) {
			setState(BOWL_GROUND);
		}
	} else if (phase == BOWL_UPDATE) {
		roll()->pushOff(mShape, ((CharaColli*)mShape)->mContacts, 0);
		roll()->moveBounce();
		updateMatrix();
	}
}

// 0x8009F1D8
int TObjBowlEgg::onPush(ObjMsg* msg)
{
	njAddVector(&mBody.mAccel, &((BowlVecMsg*)msg)->mVec);
	return 1;
}

// 0x8009F204
int TObjBowlEgg::onKick(ObjMsg* msg)
{
	switch (mState) {
	case BOWL_GROUND:
		roll()->kick(((BowlThrowMsg*)msg)->mHolder);
		return 1;
	case BOWL_SLAM:
		roll()->kickHeld();
		setState(BOWL_GROUND);
		return 1;
	case BOWL_ROLLING:
		roll()->reboundHard();
		setState(BOWL_BOUNCE);
		return 1;
	}
	return 0;
}

// 0x8009F2A4
int TObjBowlEgg::onGetType() { return 0x1B; }

// 0x8009F2AC
int TObjBowlEgg::onGetFlags()
{
	int flags = getMsgFlags();

	switch (mState) {
	case BOWL_ROLLING:
		flags |= 0x200;
		break;
	}
	return flags;
}

// 0x8009F2EC
int TObjBowlEgg::onGetVel(ObjMsg* msg)
{
	((BowlVecMsg*)msg)->mVec = mBody.mVel;
	return 1;
}

// 0x8009F30C
int TObjBowlEgg::onThrow(ObjMsg* msg)
{
	if (isHolder((BowlThrowMsg*)msg) == 0) {
		return 0;
	}
	if (mState == BOWL_HELD) {
		release();
		roll()->setVel(&((BowlThrowMsg*)msg)->mVel);
		setState(BOWL_ROLLING);
		return 1;
	}
	return 0;
}

// 0x8009F384
int TObjBowlEgg::onPickUp(ObjMsg* msg)
{
	switch (mState) {
	case BOWL_GROUND:
		setState(BOWL_HELD);
		return attachTo((BowlGrabMsg*)msg);
	case BOWL_ROLLING:
		if (attachTo((BowlGrabMsg*)msg) != 0) {
			mBody.mVel = gVecZero;
			setState(BOWL_HELD);
			return 1;
		}
		return 0;
	}
	return 0;
}

// 0x8009F430
int TObjBowlEgg::onDrop(ObjMsg* msg)
{
	if (isHolder((BowlThrowMsg*)msg) == 0) {
		return 0;
	}
	if (mState == BOWL_HELD) {
		release();
		setState(BOWL_GROUND);
		return 1;
	}
	return 0;
}

// 0x8009F498
int TObjBowlEgg::onPath(ObjMsg* msg)
{
	BowlPathMsg* path = (BowlPathMsg*)msg;

	if (isHolder((BowlThrowMsg*)msg) == 0) {
		return 0;
	}
	if (mState == BOWL_HELD) {
		release();
		roll()->attachPathAt(path->mPath, path->mPoint, path->mSpeed);
		setState(BOWL_PATH);
		return 1;
	}
	return 0;
}

// 0x8009F518
int TObjBowlEgg::onSlam(ObjMsg* msg)
{
	if (isHolder((BowlThrowMsg*)msg) == 0) {
		return 0;
	}
	if (mState == BOWL_HELD) {
		release();
		roll()->startSlam(&((BowlThrowMsg*)msg)->mVel);
		setState(BOWL_SLAM);
		return 1;
	}
	return 0;
}

// 0x8009F590
static void bowlTev(u8 stage)
{
	GXSetTevKColorSel(stage, GX_TEV_KCSEL_1_4);
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(stage, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
}

// 0x8009F640
static void bowlTexGen(u8 coord) { GXSetTexCoordGen2(coord, GX_TG_MTX3X4, GX_TG_NRM, GX_TEXMTX0, GX_TRUE, GX_PTTEXMTX19); }

// 0x8009F678
int TObjBowlEgg::getKind() { return 1; }

// 0x8009F680
Vec3f* TObjBowlEgg::getPos() { return &mPos; }

// 0x8009F688
SetLink* TObjBowlEgg::getLink() { return (SetLink*)&mLink; }

// 0x8009F690
f32 TObjBowlEgg::getRadiusV() { return mBody.mRadius; }

// 0x8009F698
// Counts *timer down true on the frame it reaches 0
extern "C" int tickTimer32(int* timer)
{
	if (*timer > 0 && --*timer == 0) {
		return 1;
	}
	return 0;
}

// Not in the DOL calling the handler here before its body makes the compiler generate it in place
static int bowlHandleMessage(BowlEggHandle* handle, ObjMsg* msg) { return handle->BowlEggHandle::onMessage(msg); }

// 0x8009F6C0
int BowlEggHandle::onMessage(ObjMsg* msg) { return ((TObjBowlEgg*)mOwner)->handleMessage(msg); }

// 0x8009F6E4
extern "C" void bowlEggInit() {}

// 0x8009F6E8
extern "C" void bowlEggTerm() {}

// 0x8009F6EC
static void* bowlModel() { return getStgObjModels()[20]; }

// 0x8009F710
static void* bowlTexList() { return getStgObjTextures()[19]; }
