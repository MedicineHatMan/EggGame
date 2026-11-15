#include "engine/TMobileObject.h"
struct DrawTask;
#include "io/Resource.h"
#include "camera/TCamera.h"
#include "stage/SetManager.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "ui/AeArc.h"
#include "io/NinjaFile.h"
#include "stage/TEventCtrl.h"
#include "core/MathUtil.h"
#include "object/ObjShadow.h"
#include "egg/ObjEgg.h"
#include "object/TObjLauncher.h"
#include "Ninja/ninja.h"
#include "engine/DrawState.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "object/ObjAction.h"
#include "camera/TCameraCtrl.h"
#include "stage/TGameState.h"
#include "sound/Sound3DPlay.h"
#include "engine/FxSystem.h"
#include "stage/TStageScore.h"
#include "object/TObjEggRing.h"
#include "engine/Colli.h"
#include "engine/GroundQuery.h"
#include "egg/EggCreature.h"
#include "stage/SetSlot.h"
#include "io/ResManager.h"
#include "egg/EggBody.h"
#include "egg/EggLevel.h"
#include "engine/TFrameBlur.h"
#include "object/TObjSnowman.h"
#include "engine/ActorMotion.h"
struct SetLink;
struct CharaColli;
struct DrawState;
struct DrawStateSlot;
struct GjModel;
struct NJS_OBJECT;
struct NJS_TEXLIST;
struct ColliContact;
struct Fx;

extern "C" {
const char* gTObjSnowmanHeadName = "TObjSnowmanHead"; // what the object list prints

// obj_snowman.arc and what it holds

}

enum SnowHeadState {
	HEAD_SPAWN   = 0, // grows out of the ground over 30 frames
	HEAD_GROUND  = 1,
	HEAD_HELD    = 2,
	HEAD_SLAM    = 3,
	HEAD_ROLLING = 4,
	HEAD_BLINK   = 5,
	HEAD_JOINED  = 6, // sits on a snowman
	HEAD_BOUNCE  = 7,
	HEAD_DEAD    = 8, // no state function
};

enum SnowPhase {
	SNOW_ENTER  = 0,
	SNOW_EXIT   = 1,
	SNOW_UPDATE = 2,
	SNOW_CHECK  = 3,
};

enum SnowHeadFlags {
	HEAD_FLAG_SAFE    = 2, // can't be hurt
	HEAD_FLAG_BURST   = 4,
	HEAD_FLAG_GROWING = 0x20,
	HEAD_FLAG_FULL    = 0x40,
};

enum SnowmanState {
	SNOWMAN_EMPTY    = 0,
	SNOWMAN_EVENT    = 1,
	SNOWMAN_COMPLETE = 2,
};

#define HEAD_FULL_SIZE 4000

/** The holder's side of the head */
struct SnowHeadHandle : public EggHandleBase {
	inline SnowHeadHandle() { }
	virtual ~SnowHeadHandle() { }       // _08 0x800FA058
	virtual inline int onMessage(ObjMsg* msg); // _0C 0x800FBF68

	u32 _10; // _10
};

struct SnowHoldMsg {
	u16 mType;     // _00
	u8 _02[2];
	u16 mHolder;   // _04
	u8 _06[2];
	void* mHandle; // _08
	Vec3f mPos;    // _0C
	EggRot mRot;   // _18
	f32 mRadius;   // _28
};

struct SnowThrowMsg {
	u16 mType;   // _00
	u8 _02[2];
	u16 mHolder; // _04
	u8 _06[2];
	Vec3f mVel;  // _08
};

struct SnowJoinMsg {
	u16 mType;  // _00
	u8 _02[2];
	u16 mOther; // _04
	u16 _06;    // _06
};

struct SnowPlaceMsg {
	u16 mType;  // _00
	u8 _02[2];
	Vec3f mPos; // _04
	int mRotY;  // _10
};

class TObjSnowman;

class TObjSnowmanHead : public TObjAction {
public:
	TObjSnowmanHead(TObject* parent, const SetDesc* desc);
	virtual ~TObjSnowmanHead();
	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual void initFromDesc(SetDesc* desc);
	virtual int sendToHandle(ObjMsg* msg);
	virtual f32 getRadiusV();
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void setPos(const Vec3f* pos); // _6C
	virtual SetLink* getLink();            // _70

	EggRoll* roll() { return &mRoll; }

	int handleMessage(ObjMsg* msg);
	int feed();
	int join(TObjSnowman* body);
	void setRadius(f32 radius);
	void runState(int state, int phase);
	void setState(int state);
	int getHolderFlags();
	int isHolder(SnowThrowMsg* msg);
	int attachTo(SnowHoldMsg* msg);
	void release();
	void updateMatrices();
	void shatter();
	void stateSpawn(int phase);
	void stateHeld(int phase);
	void stateSlam(int phase);
	void stateRolling(int phase);
	void stateBlink(int phase);
	void stateJoined(int phase);
	void stateGround(int phase);
	void stateBounce(int phase);
	int onPush(ObjMsg* msg);
	int onKick(ObjMsg* msg);
	int onGetType();
	int onGetFlags();
	int onGetVel(ObjMsg* msg);
	int onThrow(ObjMsg* msg);
	int onPickUp(ObjMsg* msg);
	int onDrop(ObjMsg* msg);
	int onSlam(ObjMsg* msg);

	s8 mState;            // _30
	u8 _31;               // _31
	SnowFlags mHeadFlags; // _32
	u16 mOwner;           // _34
	u8 _36[4];
	u16 mSnow;            // _3A up to HEAD_FULL_SIZE
	u16 mTimer;           // _3C
	u16 mGrowDelay;       // _3E
	int mAttack;          // _40
	CharaColli* mColli;         // _44
	DrawTask* mDrawTask;      // _48
	EggBody mBody;        // _4C
	SetLinkMember mLink;  // _164
	f32 mMatrix[12];      // _170
	f32 mModelMatrix[12]; // _1A0
	SnowHeadHandle mHandle; // _1D0
	Vec3f mPos;           // _1E4
	u8 _1F0[0x10];
	// An EggRoll without the fields only TObjEgg uses the next member here starts 0x30 in
	EggRoll mRoll;       // _200
	ObjShadow mShadow;    // _230
	Fx* mSparkle;       // _268
};

typedef char TObjSnowmanHead_size_check[(sizeof(TObjSnowmanHead) == 0x26C) ? 1 : -1];

typedef char TObjSnowman_size_check[(sizeof(TObjSnowman) == 0x64) ? 1 : -1];

// 0x800F9E6C
extern "C" TObject* createTObjSnowmanHead(TObject* parent, SetDesc* desc)
{
	return new TObjSnowmanHead(parent, desc);
}

// 0x800F9EC0
TObjSnowmanHead::TObjSnowmanHead(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
    , mOwner(0xFFFF)
    , mSnow(0)
    , mGrowDelay(0)
    , mAttack(0)
    , mSparkle(0)
{
	_00   = (u32)gTObjSnowmanHeadName;
	mSize = 0x26C;
	mState = -1;
	setObjId(this, desc->mObjId);

	mColli = colliCreate(1, 1);
	colliSetOwner(mColli, (u16)mObjId);
	colliSetMasks((CharaColli*)mColli, 8, 0xF3, 0xF0, 0);
	colliShapeSetSphere(colliGetShape((CharaColli*)mColli, 0));
	colliGetShape((CharaColli*)mColli, 0)->mFlags = 0x42C;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);

	mBody.mBounce      = 0.0f;
	mBody.mFriction = 0.2f;
	EggRoll* r     = roll();
	r->mOwner      = (TObjEgg*)this;
	r->mBody       = &mBody;
	mHandle.mOwner = this;
	setRadius(7.5f);
	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
	setState(HEAD_SPAWN);
}

// 0x800FA0B0
TObjSnowmanHead::~TObjSnowmanHead()
{
	fxStop((Fx*)mSparkle);
	runState(mState, SNOW_EXIT);
	removeDrawTask(mDrawTask);
	colliFree((CharaColli*)mColli);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	unregisterId();
}

// 0x800FA1B4
int TObjSnowmanHead::sendToHandle(ObjMsg* msg)
{
	return mHandle.send(msg);
}

// 0x800FA1D8
int TObjSnowmanHead::handleMessage(ObjMsg* msg)
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

// 0x800FA2B0
// The snow field calls this for each bit of snow the head rolls over
int TObjSnowmanHead::feed()
{
	if (mSnow < HEAD_FULL_SIZE) {
		if (++mSnow == HEAD_FULL_SIZE) {
			mHeadFlags.mBits |= HEAD_FLAG_FULL;
			mSparkle = fxSpawnFollow(1, 8, 0x13, mMatrix);
			sePlay3D(0x2D, &mPos);
		}
		if (mGrowDelay == 0) {
			mGrowDelay = 60;
		}
		mHeadFlags.mBits |= HEAD_FLAG_GROWING;
	}
	return 0;
}

// 0x800FA34C
// A full head that is being carried can join a snowman
int TObjSnowmanHead::join(TObjSnowman* body)
{
	if (mSnow == HEAD_FULL_SIZE && (int)(u8)mState == HEAD_HELD) {
		SnowJoinMsg msg;
		msg.mType  = 0x16;
		msg.mOther = 0xFFFF;
		msg.mOther = body->mObjId;
		msg._06    = 0;
		if (sendToHandle((ObjMsg*)&msg) != 0) {
			TObjAction* holder = findObjById(mHolderId);
			if (holder != 0) {
				ObjMsg a;
				a.mType = 0x25;
				holder->sendToHandle(&a);
				ObjMsg b;
				b.mType = 0x10;
				holder->sendToHandle(&b);
			}
			f32 mtx[12];
			body->getNodeMatrix(0, mtx);
			Vec3f pos;
			mtxGetPos(mtx, &pos);
			mBody.mPos = pos;
			updateMatrices();
			setLinkForever((SetLink*)&mLink);
			setState(HEAD_JOINED);
			return 1;
		}
	}
	return 0;
}

// 0x800FA484
void TObjSnowmanHead::initFromDesc(SetDesc* desc)
{
	Vec3f pos;
	desc->getPos(&pos);
	pos.y += mBody.mRadius;
	setPos(&pos);
	colliShapeSetPos(colliGetShape(mColli, 0), &pos);
}

// 0x800FA500
void TObjSnowmanHead::setPos(const Vec3f* pos)
{
	mPos       = *pos;
	mBody.mPos = *pos;
}

// 0x800FA528
int TObjSnowmanHead::onHit(const TObjAction* other, int, int* result)
{
	if (other->mObjId == mHolderId) {
		return 0;
	}
	switch (mState) {
	case HEAD_ROLLING:
	case HEAD_BOUNCE:
		result[0x14 / 4] = mAttack;
		return roll()->hitWhileRolling(result, 10);
	case HEAD_HELD:
		return ((TMobileObject*)mHandle.mHolder)->takeHit((HitInfo*)result, 10);
	case HEAD_SLAM:
		result[0x14 / 4] = mAttack;
		return roll()->hitWhileHeld(result, 10);
	}
	return 0;
}

// 0x800FA5DC
void TObjSnowmanHead::draw()
{
	if (debugEditActive() != 0) {
		mBody.makeMatrix(mModelMatrix);
		mtxScale(1.0f, 1.0f, 1.0f, mModelMatrix, mModelMatrix);
		return;
	}
	if ((mHeadFlags.mBits & HEAD_FLAG_SAFE) == 0) {
		if (mPos.y < -40.0f) {
			shatter();
			return;
		}
		if ((mHeadFlags.mBits & HEAD_FLAG_BURST) != 0) {
			shatter();
		} else if (eggFirstDamage((void*)mColli, (EggContact*)mColli->mContacts, 0, (void*)this) != 0) {
			shatter();
			return;
		}
	}
	if ((mHeadFlags.mBits & HEAD_FLAG_GROWING) != 0 && mGrowDelay != 0) {
		if (--mGrowDelay == 0) {
			mHeadFlags.mBits &= ~HEAD_FLAG_GROWING;
			setRadius(lerpF(7.5f, 20.0f, mSnow / 4000.0f));
		}
	}
	runState(mState, SNOW_CHECK);
	runState(mState, SNOW_UPDATE);
	mPos = mBody.mPos;
	colliShapeSetPos(colliGetShape(mColli, 0), &mPos);

	EggHits hits;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&mPos, -1) != 0 && (hits.mMask & 1) != 0) {
		shadowSetGround(&mShadow, &hits.mHits[0].mPos, &hits.mHits[0].mNormal);
		shadowEnable(&mShadow);
	} else {
		shadowDisable(&mShadow);
	}
}

// 0x800FA7C0
void TObjSnowmanHead::vfn_10()
{
	if (cameraIsVisible(1000.0f, mBody.mRadius, &mPos, 0) == 0) {
		mDrawTask->mFlags |= 1;
	} else {
		mDrawTask->mFlags &= ~1;
	}
}

// 0x800FA828
void TObjSnowmanHead::render()
{
	if (sSnowmanModels == 0) {
		return;
	}
	if (cameraIsVisible(1000.0f, 25.0f, &mPos, 0) == 0) {
		return;
	}
	DrawStateSlot* slot = getDrawStateSlot();
	f32 mtx[12];
	mtxConcat(mtx, gjGetMatrix(), mModelMatrix);
	shadowAlphaBegin();
	njPushMatrix(mtx);
	beginDrawState(slot, getDefaultDrawState());
	njSetTexture(sSnowmanTexLists[0]);
	if ((mHeadFlags.mBits & HEAD_FLAG_FULL) != 0) {
		gjDrawObject(sSnowmanModels[1]);
	} else {
		NJS_OBJECT* child = sSnowmanModels[1]->child;
		gjDrawModel(child->model);
	}
	endDrawState(slot);
	njPopMatrixEx();
	shadowAlphaEnd();
}

// 0x800FA8FC
void TObjSnowmanHead::setRadius(f32 radius)
{
	mBody.mRadius = radius;
	if (mHandle.mHolder != 0) {
		((TMobileObject*)mHandle.mHolder)->mBody.mRadius = radius;
	}
	colliGetShape(mColli, 0)->mSizeX = radius;
	mShadow._30 = 2.0f * radius;
	mShadow._34 = 2.0f * radius;
}

// 0x800FA96C
void TObjSnowmanHead::runState(int state, int phase)
{
	switch (state) {
	case HEAD_SPAWN:
		stateSpawn(phase);
		break;
	case HEAD_GROUND:
		stateGround(phase);
		break;
	case HEAD_HELD:
		stateHeld(phase);
		break;
	case HEAD_SLAM:
		stateSlam(phase);
		break;
	case HEAD_ROLLING:
		stateRolling(phase);
		break;
	case HEAD_BLINK:
		stateBlink(phase);
		break;
	case HEAD_JOINED:
		stateJoined(phase);
		break;
	case HEAD_BOUNCE:
		stateBounce(phase);
		break;
	}
}

// 0x800FAA04
void TObjSnowmanHead::setState(int state)
{
	if (mState != -1) {
		runState(mState, SNOW_EXIT);
	}
	if (state != -1) {
		runState(state, SNOW_ENTER);
	}
	mState = state;
}

// 0x800FAA64
int TObjSnowmanHead::getHolderFlags()
{
	if (mHandle.mHolder != 0) {
		ObjMsg msg;
		msg.mType = 0x41;
		return mHandle.send(&msg);
	}
	return 0;
}

// 0x800FAAA8
int TObjSnowmanHead::isHolder(SnowThrowMsg* msg)
{
	return msg->mHolder == mHolderId;
}

// 0x800FAAC0
int TObjSnowmanHead::attachTo(SnowHoldMsg* msg)
{
	if (mHandle.mHolder == 0) {
		msg->mHandle = &mHandle;
		msg->mPos    = mBody.mPos;
		msg->mRot    = mBody.mRot;
		msg->mRadius = mBody.mRadius;
		mHandle.mDamage = 10;
		mOwner       = msg->mHolder;
		mHolderId    = msg->mHolder;
		return 1;
	}
	return 0;
}

// 0x800FAB3C
void TObjSnowmanHead::release()
{
	mOwner = 0xFFFF;
}

// 0x800FAB4C
void TObjSnowmanHead::updateMatrices()
{
	Vec3f pos;
	pos = mBody.mPos;
	mtxSetTransV(mMatrix, &pos);
	mBody.makeMatrix(mModelMatrix);
	f32 scale = mBody.mRadius / 7.5f;
	mtxScale(scale, scale, scale, mModelMatrix, mModelMatrix);
}

// 0x800FABC4
void TObjSnowmanHead::shatter()
{
	f32 mtx[12];
	mtxSetTransV(mtx, &mPos);
	fxSpawnAt(1, 8, 0x1E, mtx);
	sePlay3D(0x23A, &mPos);
	mHandle.release();
	mHeadFlags.mBits |= HEAD_FLAG_SAFE;
	setLinkTimed((SetLink*)&mLink, 300);
	mFlags |= 1;
	setState(HEAD_DEAD);
}

// 0x800FAC50
void TObjSnowmanHead::stateSpawn(int phase)
{
	if (phase == SNOW_ENTER) {
		mTimer = 30;
		mHeadFlags.mBits |= HEAD_FLAG_SAFE;
		mtxSetTransV(mMatrix, &mPos);
		fxSpawnAt(1, 7, 0x1D, mMatrix);
	} else if (phase == SNOW_EXIT) {
		mHeadFlags.mBits &= ~HEAD_FLAG_SAFE;
	} else if (phase == SNOW_CHECK) {
		if (--mTimer == 0) {
			setState(HEAD_GROUND);
		}
	} else if (phase == SNOW_UPDATE) {
		f32 scale = mBody.mRadius / 7.5f;
		mtxScale(scale, scale * (30 - mTimer) / 30.0f, scale, mModelMatrix, mMatrix);
	}
}

// 0x800FAD4C
void TObjSnowmanHead::stateHeld(int phase)
{
	if (phase == SNOW_UPDATE) {
		eggPushHeld((void*)mColli, (EggContact*)mColli->mContacts, 0, mHandle.mHolder);
		TMobileObject* holder = (TMobileObject*)mHandle.mHolder;
		Vec3f pos;
		EggRot rot;
		pos = holder->mBody.mPos;
		rot = holder->mBody.mRot;
		mBody.mPos = pos;
		mBody.mRot = rot;
		updateMatrices();
		roll()->checkBumps();
	}
}

// 0x800FADE8
void TObjSnowmanHead::stateSlam(int phase)
{
	if (phase == SNOW_CHECK) {
		if (roll()->isSlamDone() != 0) {
			setState(HEAD_GROUND);
		} else if (roll()->hasLanded() != 0) {
			Vec3f pos;
			pos.x = mPos.x;
			pos.y = mPos.y - mBody.mRadius;
			pos.z = mPos.z;
			eggSpawnShockWave(mBody.mRadius, mObjId, mHolderId, &pos, 10, (void*)mAttack);
			sePlay3D(0x84, &mPos);
			camShakeNearSoft(200.0f, &mPos);
			setState(HEAD_GROUND);
		}
	} else if (phase == SNOW_UPDATE) {
		roll()->moveSlam();
		updateMatrices();
	}
}

// 0x800FAEC0
void TObjSnowmanHead::stateRolling(int phase)
{
	if (phase == SNOW_CHECK) {
		if (roll()->isRollDone() != 0) {
			setState(HEAD_GROUND);
		} else if (roll()->hitWall() != 0) {
			mBody.mVel = gVecZero;
			setState(HEAD_GROUND);
		}
	} else if (phase == SNOW_UPDATE) {
		roll()->pushOff(mColli, mColli->mContacts, 0);
		roll()->moveRolling();
		updateMatrices();
	}
}

// 0x800FAF7C
void TObjSnowmanHead::stateBlink(int phase)
{
	if (phase == SNOW_ENTER) {
		mTimer = 60;
	} else if (phase == SNOW_CHECK) {
		if (tickTimer16(&mTimer) != 0) {
			setLinkTimed((SetLink*)&mLink, 300);
			mFlags |= 1;
		}
	} else if (phase == SNOW_UPDATE) {
		if (mTimer & 1) {
			mFlags &= ~0x10;
		} else {
			mFlags |= 0x10;
		}
	}
}

// 0x800FB01C
void TObjSnowmanHead::stateJoined(int phase)
{
	if (phase == SNOW_ENTER) {
		mTimer = 60;
		Vec3f pos;
		pos = mBody.mPos;
		pos.y -= mBody.mRadius;
		f32 mtx[12];
		mtxSetTransV(mtx, &pos);
		fxSpawnAt(1, 8, 0xE, mtx);
		if (mSparkle != 0) {
			fxStop(mSparkle);
		}
		mSparkle = fxSpawnAt(1, 8, 0x17, mtx);
	}
}

// 0x800FB0C0
void TObjSnowmanHead::stateGround(int phase)
{
	if (phase == SNOW_CHECK) {
		if (!isInRange(this, &mPos, 1000.0f)) {
			mFlags |= 1;
		}
	} else if (phase == SNOW_UPDATE) {
		roll()->pushOff(mColli, mColli->mContacts, 0);
		roll()->moveGround();
		updateMatrices();
	}
}

// 0x800FB140
void TObjSnowmanHead::stateBounce(int phase)
{
	if (phase == SNOW_CHECK) {
		if (roll()->isRollDone() != 0) {
			setState(HEAD_GROUND);
		}
	} else if (phase == SNOW_UPDATE) {
		roll()->pushOff(mColli, mColli->mContacts, 0);
		roll()->moveBounce();
		updateMatrices();
	}
}

// 0x800FB1BC
int TObjSnowmanHead::onPush(ObjMsg* msg)
{
	njAddVector(&mBody.mAccel, &((ObjVecMsg*)msg)->mVec);
	return 1;
}

// 0x800FB1E8
int TObjSnowmanHead::onKick(ObjMsg* msg)
{
	switch (mState) {
	case HEAD_GROUND:
		roll()->kick(((EggKickMsg*)msg)->mAngle);
		return 1;
	case HEAD_SLAM:
		roll()->kickHeld();
		setState(HEAD_GROUND);
		return 1;
	case HEAD_ROLLING:
		roll()->reboundHard();
		setState(HEAD_BOUNCE);
		return 1;
	}
	return 0;
}

// 0x800FB28C
int TObjSnowmanHead::onGetType()
{
	return 0x1B;
}

// 0x800FB294
int TObjSnowmanHead::onGetFlags()
{
	int flags = getHolderFlags();
	switch (mState) {
	case HEAD_ROLLING:
	case HEAD_BOUNCE:
		flags |= 0x200;
		break;
	}
	return flags;
}

// 0x800FB2E4
int TObjSnowmanHead::onGetVel(ObjMsg* msg)
{
	((ObjVecMsg*)msg)->mVec = mBody.mVel;
	return 1;
}

// 0x800FB304
int TObjSnowmanHead::onThrow(ObjMsg* msg)
{
	if (isHolder((SnowThrowMsg*)msg) == 0) {
		return 0;
	}
	if ((s8)mState == HEAD_HELD) {
		release();
		roll()->setVel(&((SnowThrowMsg*)msg)->mVel);
		mAttack = scoreNewChain(0);
		setState(HEAD_ROLLING);
		return 1;
	}
	return 0;
}

// 0x800FB388
int TObjSnowmanHead::onPickUp(ObjMsg* msg)
{
	switch (mState) {
	case HEAD_GROUND:
		setState(HEAD_HELD);
		return attachTo((SnowHoldMsg*)msg);
	case HEAD_ROLLING:
	case HEAD_BOUNCE:
		if (attachTo((SnowHoldMsg*)msg) != 0) {
			mBody.mVel = gVecZero;
			setState(HEAD_HELD);
			return 1;
		}
		return 0;
	}
	return 0;
}

// 0x800FB44C
int TObjSnowmanHead::onDrop(ObjMsg* msg)
{
	if (isHolder((SnowThrowMsg*)msg) == 0) {
		return 0;
	}
	if ((s8)mState == HEAD_HELD) {
		release();
		setState(HEAD_GROUND);
		return 1;
	}
	return 0;
}

// 0x800FB4B4
int TObjSnowmanHead::onSlam(ObjMsg* msg)
{
	if (isHolder((SnowThrowMsg*)msg) == 0) {
		return 0;
	}
	if ((s8)mState == HEAD_HELD) {
		release();
		roll()->startSlam(&((SnowThrowMsg*)msg)->mVel);
		setState(HEAD_SLAM);
		return 1;
	}
	return 0;
}

// 0x800FB52C
extern "C" void initSnowman()
{
	loadArc("obj_snowman.arc", &sSnowmanArc);
	if (sSnowmanArc != 0) {
		sSnowmanModels = (NJS_OBJECT**)findArcExport(sSnowmanArc, "models");
		sSnowmanTexLists = (NJS_TEXLIST**)findArcExport(sSnowmanArc, "texlists");
		sSnowmanCameras = (void**)findArcExport(sSnowmanArc, "cameras");
		void** textures = (void**)findArcExport(sSnowmanArc, "textures");
		if (textures != 0) {
			numberTextures(sSnowmanTexLists[0]);
			bindGvmTextures((u8*)textures[0], sSnowmanTexLists[0]);
		}
	}
}

// 0x800FB5CC
extern "C" void termSnowman()
{
	if (sSnowmanTexLists != 0) {
		releaseTextures(sSnowmanTexLists[0]);
	}
	releaseResource(sSnowmanArc);
	sSnowmanModels = 0;
	sSnowmanTexLists = 0;
	sSnowmanCameras = 0;
	sSnowmanArc = 0;
}

const char* gTObjSnowmanName = "TObjSnowman";

// 0x800FB618
TObjSnowman::TObjSnowman(TObject* parent, const SetDesc* desc)
    : TObjAction(parent)
{
	_00   = (u32)gTObjSnowmanName;
	mSize = 0x64;
	mState  = -1;
	mHolder = 0xFFFF;
	setObjId(this, desc->mObjId);

	mColli = colliCreate(2, 1);
	colliSetOwner(mColli, (u16)mObjId);
	colliSetMasks((CharaColli*)mColli, 4, 0xFF, 0xF0, 0);
	colliShapeSetSphere(colliGetShape((CharaColli*)mColli, 0));
	colliGetShape((CharaColli*)mColli, 0)->mFlags = 0;
	colliGetShape((CharaColli*)mColli, 0)->mSizeX = 25.0f;
	colliShapeSetSphere(colliGetShape((CharaColli*)mColli, 1));
	colliGetShape((CharaColli*)mColli, 1)->mFlags = 0x10;
	colliGetShape((CharaColli*)mColli, 1)->mSizeX = 10.0f;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);

	mLink.get()->attach((SetDesc*)desc);
	initFromDesc((SetDesc*)desc);
	if ((u32)mLink.get()->getData() == 1) {
		setState(SNOWMAN_COMPLETE);
	} else {
		setState(SNOWMAN_EMPTY);
	}
}

// 0x800FB7B0
TObjSnowman::~TObjSnowman()
{
	runState(mState, SNOW_EXIT);
	removeDrawTask(mDrawTask);
	colliFree((CharaColli*)mColli);
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	unregisterId();
}

// 0x800FB850
extern "C" TObject* createTObjSnowman(TObject* parent, SetDesc* desc)
{
	return new TObjSnowman(parent, desc);
}

// 0x800FB8A4
// Node 0 is where the head goes
void TObjSnowman::getNodeMatrix(int node, f32* mtx)
{
	if (node == 0) {
		mtxSetTrans(mtx, mPos.x, mPos.y + mHeight, mPos.z);
	}
}

// 0x800FB8E8
void TObjSnowman::draw()
{
	if (debugEditActive() == 0) {
		runState(mState, SNOW_CHECK);
		runState(mState, SNOW_UPDATE);
	}
}

// 0x800FB944
void TObjSnowman::render()
{
	if (sSnowmanModels == 0) {
		return;
	}
	if (cameraIsVisible(1000.0f, 25.0f, &mPos, 0) == 0) {
		return;
	}
	DrawStateSlot* slot = getDrawStateSlot();
	shadowAlphaBegin();
	njPushMatrixEx();
	njTranslateEx(&mPos);
	njRotateY(0, mRotY);
	beginDrawState(slot, getDefaultDrawState());
	njSetTexture(sSnowmanTexLists[0]);
	gjDrawObject(sSnowmanModels[0]);
	if (mState == SNOWMAN_COMPLETE) {
		njTranslate(0, 0.0f, mHeight, 0.0f);
		njScale(0, 8.0f / 3.0f, 8.0f / 3.0f, 8.0f / 3.0f);
		gjDrawObject(sSnowmanModels[1]);
	}
	endDrawState(slot);
	njPopMatrixEx();
	shadowAlphaEnd();
}

// 0x800FBA30
void TObjSnowman::initFromDesc(SetDesc* desc)
{
	Vec3f pos;
	desc->getPos(&pos);
	mPos    = pos;
	mHeight = 65.0f;
	pos.y += 25.0f;
	colliShapeSetPos(colliGetShape(mColli, 0), &pos);

	Vec3f top;
	top.x = mPos.x;
	top.y = mPos.y + mHeight;
	top.z = mPos.z;
	colliShapeSetPos(colliGetShape(mColli, 1), &top);

	SetRot rot;
	desc->getRot(&rot);
	mRotY = rot.y;
}

// 0x800FBB0C
void TObjSnowman::runState(int state, int phase)
{
	switch (state) {
	case SNOWMAN_EMPTY:
		stateEmpty(phase);
		break;
	case SNOWMAN_EVENT:
		stateEvent(phase);
		break;
	case SNOWMAN_COMPLETE:
		stateComplete(phase);
		break;
	}
}

// 0x800FBB6C
void TObjSnowman::setState(int state)
{
	if (mState != -1) {
		runState(mState, SNOW_EXIT);
	}
	if (state != -1) {
		runState(state, SNOW_ENTER);
	}
	mState = state;
}

struct SnowmanSearch {
	TObjSnowman* mSelf;     // _00
	TObjSnowmanHead* mHead; // _04
	Vec3f mTop;             // _08
};

// 0x800FBBCC
// Looks for a head touching the top of the body
static void findHead(void* contact, void* arg, int* out)
{
	SnowmanSearch* search = (SnowmanSearch*)arg;
	if ((colliContactShape((ColliContact*)contact)->mFlags & 0x400) == 0) {
		return;
	}
	if (((ColliContact*)contact)->mShape != 1) {
		return;
	}
	TObjAction* other = findObjById(colliContactOwner((ColliContact*)contact));
	if (other != 0 && other->_00 == (u32)gTObjSnowmanHeadName) {
		search->mHead = (TObjSnowmanHead*)other;
		*out          = 1;
	}
}

// 0x800FBC4C
void TObjSnowman::stateEmpty(int phase)
{
	if (phase == SNOW_CHECK) {
		SnowmanSearch search;
		search.mSelf  = this;
		search.mHead  = 0;
		search.mTop.x = mPos.x;
		search.mTop.y = mPos.y + mHeight;
		search.mTop.z = mPos.z;
		forEachContact((CharaColli*)mColli, (ContactCallback)findHead, &search);
		if (search.mHead != 0 && search.mHead->join(this) != 0) {
			mHolder = search.mHead->mHolderId;
			mLink.get()->setData((void*)1);
			setState(SNOWMAN_EVENT);
		}
	}
}

// 0x800FBCFC
void TObjSnowman::stateEvent(int phase)
{
	if (phase == SNOW_ENTER) {
		eventStartById(mObjId, 0);
		mEvent = createCamPath(0, 2, sSnowmanCameras[0], sSnowmanCameras[1], 0, 0.0f, 1.0f);
		mEvent->attachToMtx();
		frameBlurStart(60, 0.7f);
		mTimer = 0;
	} else if (phase == SNOW_EXIT) {
		ObjMsg msg;
		msg.mType = 0x79;
		sendMsgById(mHolder, &msg);
		eventEnd();
		mEvent->mFlags |= 1;
	} else if (phase == SNOW_CHECK) {
		if (mTimer == 294) {
			ObjMsg notice;
			notice.mType = 0x9D;
			sendGameMsg(&notice);
			setState(-1);
		}
	} else if (phase == SNOW_UPDATE) {
		f32 mtx[12];
		mtxSetTransV(mtx, &mPos);
		mtxRotateY(mtx, mtx, mRotY);
		mtxTranslate((float*)mtx, (const float*)mtx, 0.0f, 0.0f, 77.0f);
		mtxCopy(mEvent->getMtx(), mtx);
		if (mTimer == 1) {
			sePlay3D(0x81, &mPos);
		} else if (mTimer == 165) {
			TObjAction* holder = findObjById(mHolder);
			if (holder != 0) {
				ObjMsg a;
				a.mType = 0x13;
				SnowPlaceMsg b;
				b.mType = 0x78;
				eventHideObj((u8*)holder);
				holder->sendToHandle(&a);
				mtxGetPos(mtx, &b.mPos);
				b.mRotY = mRotY;
				holder->sendToHandle((ObjMsg*)&b);
			}
		}
		mTimer++;
	}
}

// 0x800FBECC
void TObjSnowman::stateComplete(int phase)
{
	if (phase != SNOW_ENTER && phase == SNOW_CHECK) {
		if (!isInRange(this, &mPos, 1000.0f)) {
			mFlags |= 1;
		}
	}
}

// 0x800FBF24
int TObjSnowman::getKind()
{
	return 3;
}

// 0x800FBF2C
Vec3f* TObjSnowman::getPos()
{
	return &mPos;
}

// 0x800FBF34
SetLink* TObjSnowman::getLink()
{
	return mLink.get();
}

// 0x800FBF3C
int TObjSnowmanHead::getKind()
{
	return 1;
}

// 0x800FBF44
Vec3f* TObjSnowmanHead::getPos()
{
	return &mPos;
}

// 0x800FBF4C
SetLink* TObjSnowmanHead::getLink()
{
	return mLink.get();
}

// 0x800FBF54
f32 TObjSnowmanHead::getRadiusV()
{
	return mBody.mRadius;
}

// 0x800FBF5C
SnowFlags::SnowFlags()
{
	mBits = 0;
}

// Not in the DOL the linker stripped it
static int snowHandleMessage(SnowHeadHandle* handle, ObjMsg* msg) { return handle->SnowHeadHandle::onMessage(msg); }

// 0x800FBF68
int SnowHeadHandle::onMessage(ObjMsg* msg)
{
	return ((TObjSnowmanHead*)mOwner)->handleMessage(msg);
}
