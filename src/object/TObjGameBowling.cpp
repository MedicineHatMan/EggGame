#include "object/TObjGameBowling.h"
#include "object/ObjCommon.h"
#include "object/ObjArchive.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "core/Pad.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "stl/math.h"
#include "object/TObjBowlEgg.h"
#include "object/TObjLauncher.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "object/ObjAction.h"
#include "camera/TCameraCtrl.h"
#include "object/TObjEggRing.h"
#include "engine/Color.h"
#include "engine/GroundQuery.h"
#include "stage/SetSlot.h"
#include "core/VecMath.h"
#include "object/TObjPlayer.h"
#include "object/TObjEggFeed.h"
struct NJS_QUATERNION;
struct CharaColli;
struct SetLink;
struct TObjEggFeed;

extern "C" {

// The bowling egg still in the original bytes
}

/** What the lane sends the player */
struct BowlMsg {
	u16 mType; // _00
	u8 _02[2];
	u16 mFrom; // _04 or the throw's kind for 0x83
	u16 mArg;  // _06
};

/** 0x7C which asks the player to take hold of the lane */
struct BowlHoldMsg : public BowlMsg {
	u16 mArg2; // _08
};

static inline Vec3f scaled(const Vec3f& v, f32 s)
{
	Vec3f r;
	gjScaleVector(s, &r, &v);
	return r;
}

static inline Vec3f negated(const Vec3f& v)
{
	Vec3f r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

static inline Vec3f added(const Vec3f& a, const Vec3f& b)
{
	Vec3f r;
	gjAddVector(&r, &a, &b);
	return r;
}

static inline Vec3f above(const Vec3f& pos, f32 height)
{
	Vec3f up;
	up.x = 0.0f;
	up.y = height;
	up.z = 0.0f;
	return added(pos, up);
}

static inline f32 gameSqrtf(f32 x)
{
	if (x > 0.0f) {
		f64 guess      = __frsqrte((f64)x);
		guess          = 0.5 * guess * (3.0 - guess * guess * x);
		guess          = 0.5 * guess * (3.0 - guess * guess * x);
		guess          = 0.5 * guess * (3.0 - guess * guess * x);
		volatile f32 y = (f32)(x * guess);
		return y;
	}
	return x;
}

// 0x8009F734
// The object is 0x864 bytes most of it the ten pins
extern "C" TObject* createTObjGameBowling(TObject* parent, SetDesc* desc)
{
	return new TObjGameBowling(parent, desc);
}

// 0x8009F788
void TObjGameBowling::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
}

// 0x8009F7A8
// Registers two ids against the float table at sGameBowlingEmpty
void TObjGameBowling::debugPrint(void*)
{
	njPrint((s32)0xA0013, (const char*)&sGameBowlingEmpty);
	njPrintC((s32)0xA0017, (const char*)&sGameBowlingEmpty);
}

const char* gTObjGameBowlingName = "TObjGameBowling";

// 0x8009F7E8
TObjGameBowling::TObjGameBowling(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00    = (u32)gTObjGameBowlingName;
	mSize  = 0x864;
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 255, 246);
	initFromDesc(desc);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	setup();
}

// 0x8009F8F4
TObjGameBowling::~TObjGameBowling()
{
	removeDrawTask(mDraw);
	colliRetire(mShape);
	mShape = 0;
}

// 0x8009F9A8
// The lane the arm and the pin come from stgobj_common
void TObjGameBowling::setup()
{
	mMotion.init(sGameBowlingMotions, gStgObjCommon->mSet.mMotions);
	mModel.setModel(getStgObjModels()[0x54 / 4]);
	mModel.mTexList = getStgObjTextures()[0x50 / 4];
	mModel.mState   = getDefaultDrawState();
	mArmModel.setModel(getStgObjModels()[0x58 / 4]);
	mArmModel.mTexList = getStgObjTextures()[0x54 / 4];
	mArmModel.mState   = getDefaultDrawState();
	mArmModel.mMotion  = &mMotion;
	mPinModel.setModel(getStgObjModels()[0x5C / 4]);
	mPinModel.mTexList = getStgObjTextures()[0x58 / 4];
	mPinModel.mState   = getDefaultDrawState();
	setLinkFree((SetLink*)&mLink);
	reset();
}

// 0x8009FA78
void TObjGameBowling::reset()
{
	mPhase    = 0;
	mAim      = 0;
	mAimSpeed = 0x100;
	mEggId    = 0xFFFF;
	mEgg      = 0;
	mPlayerId = 0xFFFF;
	mPlayer   = 0;
	mTimer    = 0;
	mArmY     = -20.0f;
	mMotion.play(0);
	placeShape();
	setupPins();
	setShowPins(0);
}

// 0x8009FB00
void TObjGameBowling::draw()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 900.0f) == 0) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this) != 0) {
		setupPins();
	}
	switch (mPhase) {
	case 0:
		phaseWait();
		break;
	case 1:
		phaseRise();
		break;
	case 2:
		phaseAim();
		break;
	case 3:
		phaseThrow();
		break;
	case 4:
		phaseRoll();
		break;
	case 5:
		phaseCount();
		break;
	case 6:
		phaseLower();
		break;
	}
	mtxSetTransV(mBaseMtx, &mPos);
	mtxRotateY(mBaseMtx, mBaseMtx, mRot.y + 0x8000);
	mtxRotateY(mArmMtx, mBaseMtx, mAim);
	mtxTranslate((float*)mArmMtx, (const float*)mArmMtx, 0.0f, mArmY, 0.0f);
}

// 0x8009FC24
// Needs a bowling egg within 8 and the player within 20 on the ground plane
void TObjGameBowling::phaseWait()
{
	int rot;
	int haveEgg    = 0;
	int havePlayer = 0;
	ColliContact* contact;

	for (contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliGetShape(contact->mOther, contact->mOtherShape)->mFlags & 0x10) {
			continue;
		}
		u32 owner       = colliContactOwner(contact);
		TObjAction* obj = findObjById(owner);
		if (obj == 0) {
			continue;
		}
		if (obj->_00 == (u32)gTObjBowlEggName) {
			if (distXZ(obj->getPos(), &mPos) <= 8.0f) {
				mEggId  = owner;
				haveEgg = 1;
				mEgg    = (TObjBowlEgg*)obj;
			}
		} else if (colliInGroup(contact->mOther, 1) != 0) {
			if (distXZ(obj->getPos(), &mPos) <= 20.0f) {
				mPlayerId  = owner;
				havePlayer = 1;
				mPlayer    = obj;
			}
		}
	}
	if (haveEgg != 0 && havePlayer != 0) {
		mEgg->carryOn(this, 0);
		BowlHoldMsg msg;
		msg.mType = 0x7C;
		msg.mFrom = mObjId;
		msg.mArg  = 1;
		msg.mArg2 = 0;
		if (mPlayer->sendToHandle((ObjMsg*)&msg) != 0) {
			rot    = mRot.y;
			mAim   = mPlayer->getRotY() - rot;
			mPhase = 1;
		}
	}
}

// 0x8009FDBC
void TObjGameBowling::phaseRise()
{
	mArmY += 0.4f;
	if (mArmY > 0.0f) {
		mArmY  = 0.0f;
		mPhase = 2;
	}
}

// 0x8009FDEC
// Swings until the arm passes the middle
void TObjGameBowling::phaseAim()
{
	if (mAim > 0x2000) {
		mAimSpeed = -0x100;
	} else if (mAim < -0x2000) {
		mAimSpeed = 0x100;
	}
	mAim += mAimSpeed;
	if ((f32)(mAim < 0 ? -mAim : mAim) < 256.0f) {
		mAim = 0;
		setShowPins(1);
		mPhase = 3;
	}
}

// 0x8009FEA0
// Button 0x200 rolls the egg along the arm
void TObjGameBowling::phaseThrow()
{
	if (checkEgg() == 0) {
		mPhase = 6;
	}
	TObjAction* player = (TObjAction*)findPlayer(playerGetDrawView());
	if (player != 0 && (padGet(0)->mPressed & 0x200)) {
		Vec3f vel;
		vel.x = 0.0f;
		vel.y = 0.0f;
		vel.z = 240.0f;
		vecRotateY(&vel, &vel, mAim + mRot.y);
		mEgg->launch(&vel);
		BowlMsg msg;
		msg.mType = 0x83;
		msg.mFrom = 9;
		player->sendToHandle((ObjMsg*)&msg);
		mPhase = 4;
	}
	if (mAim > 0x2000) {
		mAimSpeed = -0x100;
	} else if (mAim < -0x2000) {
		mAimSpeed = 0x100;
	}
	mAim += mAimSpeed;
}

// 0x8009FFA0
// Moves the pins for 300 frames
void TObjGameBowling::phaseRoll()
{
	if (mMotion.isDone() == 0) {
		mMotion.update();
		mArmModel.calcMotion();
	}
	for (int k = 0; k < 10; k++) {
		TBowlPin* pin = &mPins[k];
		pin->draw();
	}
	for (int i = 0; i < 10; i++) {
		for (int j = i + 1; j < 10; j++) {
			mPins[i].collide(&mPins[j]);
		}
	}
	if (mTimer >= 300) {
		TObjAction* player = (TObjAction*)findPlayer(mPlayerId);
		if (player != 0) {
			BowlMsg msg;
			msg.mType = 5;
			player->sendToHandle((ObjMsg*)&msg);
		}
		mTimer = 0;
		mPhase = 5;
	}
	mTimer++;
}

// 0x800A00B8
void TObjGameBowling::phaseCount()
{
	if (mTimer >= 60) {
		setShowPins(0);
		mEgg->kill();
		dropFeed();
		mTimer = 0;
		mPhase = 6;
		return;
	}
	if (mTimer % 2) {
		setShowPins(1);
	} else {
		setShowPins(0);
	}
	mTimer++;
}

// 0x800A014C
void TObjGameBowling::phaseLower()
{
	mArmY -= 0.4f;
	if (mArmY < -20.0f) {
		mPhase = 0;
		reset();
	}
}

// 0x800A0194
// 4 to 6 pins down give one fruit 7 or 8 three 9 five and a strike ten
void TObjGameBowling::dropFeed()
{
	s16 down = 0;
	for (int i = 0; i < 10; i++) {
		if (mPins[i].mHits > 0) {
			down++;
		}
	}
	u16 count = 0;
	if (down > 3) {
		if (down >= 4 && down <= 6) {
			count = 1;
		} else if (down >= 7 && down <= 8) {
			count = 3;
		} else if (down == 9) {
			count = 5;
		} else if (down == 10) {
			count = 10;
		}
	}
	Vec3f pos = mPos;
	pos.y += 100.0f;
	while (count-- != 0) {
		createTObjEggFeedAt((int)(6.0f * (3.0517578e-05f * (f32)rand()) + 0.5f), &pos, 0);
	}
}

// 0x800A0310
void TObjGameBowling::placeShape()
{
	ColliShape shape;
	colliShapeInit(&shape);
	shape.mPos   = mPos;
	shape.mType  = 0;
	shape.mSizeX = 20.0f;
	colliCopyShapes(mShape, &shape);
}

// 0x800A0374
// Four rows 200 down the lane 46 apart
void TObjGameBowling::setupPins()
{
	static const f32 sRowStep = 46.0f * gameSqrtf(3.0f) / 2.0f;

	int pin;
	int count;
	int odd;
	int half;
	TBowlPin* p;
	int left;
	int row;
	int i;

	left = 9;
	for (row = 0, pin = 0;; row++) {
		left -= row;
		if (left < 0) {
			break;
		}
		odd   = row % 2;
		count = row + 1;
		half  = count >> 1;
		for (i = 0; i < count; i++) {
			Vec3f pos;
			f32 mtx[12];
			pos.x = 46.0f * ((f32)(half - i) - (odd ? 0.5f : 0.0f));
			pos.y = 0.0f;
			pos.z = (f32)row * sRowStep;
			mtxSetTransV(mtx, &mPos);
			mtxRotateY(mtx, mtx, mRot.y);
			mtxTranslate((float*)mtx, (const float*)mtx, 0.0f, 0.0f, 200.0f);
			mtxMultVec(mtx, &pos, &pos);
			p = &mPins[pin];
			p->reset();
			p->setPos(&pos);
			pin++;
		}
	}
}

// 0x800A0568
void TObjGameBowling::releaseEgg(TObjAction*)
{
}

// 0x800A056C
int TObjGameBowling::vfn_50()
{
	return 0;
}

// 0x800A0574
// Node 0 is the arm's root and node 1 its hand
void TObjGameBowling::getNodeMatrix(int node, f32* mtx)
{
	switch (node) {
	case 1:
		mtxSetTransV(mtx, &mPos);
		mtxRotateY(mtx, mtx, mAim + mRot.y);
		mtxTranslate(mtx, (const float*)mtx, 0.0f, 0.0f, -15.0f);
		break;
	case 0:
		mtxSetTransV(mtx, &mPos);
		mtxRotateY(mtx, mtx, mAim + mRot.y);
		mtxTranslate(mtx, (const float*)mtx, 0.0f, 0.0f, 0.0f);
		break;
	}
}

// 0x800A0634
int TObjGameBowling::checkEgg()
{
	TObjAction* egg = findObjById(mEggId);
	if (egg == 0 || egg != mEgg) {
		mEggId = 0xFFFF;
		mEgg   = 0;
		return 0;
	}
	return 1;
}

// 0x800A0698
void TObjGameBowling::vfn_10()
{
	if (cameraIsVisible(800.0f, 30.0f, &mPos, 0) != 0) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x800A0700
void TObjGameBowling::setShowPins(int show)
{
	mShowPins = show;
}

// 0x800A0708
void TObjGameBowling::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mBaseMtx);
	mModel.draw();
	njPopMatrixEx();
	njPushMatrixEx();
	njMultiMatrix(0, mArmMtx);
	mArmModel.draw();
	njPopMatrixEx();
	if (mShowPins != 0) {
		for (int i = 0; i < 10; i++) {
			f32 mtx[12];
			mtxCopy(mtx, mPins[i].mMtx);
			mtxTranslate((float*)mtx, (const float*)mtx, 0.0f, -20.0f, 0.0f);
			njPushMatrixEx();
			njMultiMatrix(0, mtx);
			mPinModel.draw();
			njPopMatrixEx();
		}
	}
}

// 0x800A07D0
TBowlPin::TBowlPin()
    : TObject(0)
{
	makeShape();
	reset();
}

// 0x800A0858
TBowlPin::~TBowlPin()
{
	colliRetire((CharaColli*)mShape);
	mShape = 0;
}

// 0x800A08C8
void TBowlPin::reset()
{
	mVel[0].x = 0.0f;
	mVel[0].y = 0.0f;
	mVel[0].z = 0.0f;
	mVel[1].x = 0.0f;
	mVel[1].y = 0.0f;
	mVel[1].z = 0.0f;
	mVel[2].x = 0.0f;
	mVel[2].y = 0.0f;
	mVel[2].z = 0.0f;
	njCreateQuaternion(0, 0.0f, 0.0f, 1.0f, (NJS_QUATERNION*)mQuat);
	mHits  = 0;
	mState = 0;
	updateShape();
}

// 0x800A0940
void TBowlPin::makeShape()
{
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, 0xFFFF);
	colliSetMasks(mShape, 4, 0xFF, 0xFF, 0xF7);
}

// 0x800A09A0
// A sphere of 20 on the middle point
void TBowlPin::updateShape()
{
	if (mShape != 0) {
		ColliShape shape;
		colliShapeInit(&shape);
		shape.mPos   = mPos[1];
		shape.mType  = 0;
		shape.mSizeX = 20.0f;
		colliCopyShapes(mShape, &shape);
	}
}

// 0x800A0A10
void TBowlPin::setPos(const Vec3f* pos)
{
	setPos(pos->x, pos->y, pos->z);
}

// 0x800A0A3C
// The three points are 8 20 and 32 above the floor
void TBowlPin::setPos(f32 x, f32 y, f32 z)
{
	mPos[0].x = x;
	mPos[0].y = 8.0f + y;
	mPos[0].z = z;
	mPos[1].x = x;
	mPos[1].y = 20.0f + y;
	mPos[1].z = z;
	mPos[2].x = x;
	mPos[2].y = (40.0f + y) - 8.0f;
	mPos[2].z = z;
	updateShape();
	updateMatrix();
}

// 0x800A0AB0
// Pins closer than 40 trade their middle speeds and are pushed apart
int TBowlPin::collide(TBowlPin* other)
{
	Vec3f diff;
	vecSub(&diff, &mPos[1], &other->mPos[1]);
	if (mHits < 32 && other->mHits < 32) {
		f32 dist = njScalor(&diff);
		if (dist <= 40.0f) {
			if (njScalor(&mVel[1]) > 0.4f || njScalor(&other->mVel[2]) > 0.4f) {
				vecAdd(&mVel[2], &mVel[2], &other->mVel[1]);
				vecAdd(&mVel[0], &mVel[0], &other->mVel[1]);
				vecAdd(&other->mVel[2], &other->mVel[2], &mVel[1]);
				vecAdd(&other->mVel[0], &other->mVel[0], &mVel[1]);
				vecSub(&mVel[2], &mVel[2], &mVel[1]);
				vecSub(&mVel[0], &mVel[0], &mVel[1]);
				vecSub(&other->mVel[2], &other->mVel[2], &other->mVel[1]);
				vecSub(&other->mVel[0], &other->mVel[0], &other->mVel[1]);
			}
			vecNormalize(&diff, &diff);
			gjScaleVector(0.5f * (40.0f - dist), &diff, &diff);
			Vec3f push = negated(diff);
			vecAdd(&other->mVel[2], &other->mVel[2], &push);
			Vec3f push2 = negated(diff);
			vecAdd(&other->mVel[0], &other->mVel[0], &push2);
			vecAdd(&mVel[2], &mVel[2], &diff);
			vecAdd(&mVel[0], &mVel[0], &diff);
			mState        = 1;
			other->mState = 1;
			mHits++;
			other->mHits++;
			return 1;
		}
	}
	return 0;
}

// 0x800A0CE8
void TBowlPin::draw()
{
	switch (mState) {
	case 0:
		checkHit();
		break;
	case 1:
		fall();
		break;
	case 2:
		rest();
		break;
	}
	updateMatrix();
}

// 0x800A0D54
// A bowling egg that touches the pin knocks it away from the egg the top flies the bottom kicks back
void TBowlPin::checkHit()
{
	Vec3f push;
	ObjMsg msg;
	Vec3f dir;
	ColliContact* contact;

	for (contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
		if (colliGetShape(contact->mOther, contact->mOtherShape)->mFlags & 0x10) {
			continue;
		}
		TObjAction* egg = findObjById(colliContactOwner(contact));
		if (egg == 0 || egg->_00 != (u32)gTObjBowlEggName) {
			continue;
		}
		msg.mType = 0x43;
		egg->sendToHandle(&msg);
		vecSub(&dir, &mPos[1], egg->getPos());
		vecNormalize(&dir, &dir);
		const Vec3f& ref1 = scaled(dir, njInnerProduct(&dir, &push));
		push              = ref1;
		Vec3f step        = scaled(push, 1.0f / 60.0f);
		Vec3f back        = negated(step);
		mVel[2]           = scaled(back, 3.0f);
		mVel[2].y         = 0.0f;
		const Vec3f& ref3 = scaled(step, 5.0f);
		mVel[0]           = ref3;
		mVel[0].y         = 0.0f;
		mVel[2].y         = 1.0f;
		mVel[0].y         = 6.0f;
		mHits++;
		mState = 1;
	}
}

// 0x800A0F84
// Moves both ends keeps them 24 apart and turns the pin to lie along them
void TBowlPin::fall()
{
	Vec3f bottom;
	Vec3f top;
	Vec3f diff;
	Vec3f axis;
	Vec3f tilt;

	move();
	bottom = mPos[0];
	top    = mPos[2];
	keepLength();
	vecSub(&diff, &mPos[0], &bottom);
	vecAdd(&mVel[0], &mVel[0], &diff);
	vecSub(&diff, &mPos[2], &top);
	vecAdd(&mVel[2], &mVel[2], &diff);
	vecAdd(&mVel[1], &mVel[2], &mVel[0]);
	gjScaleVector(0.5f, &mVel[1], &mVel[1]);
	vecSub(&axis, &mPos[2], &mPos[0]);
	f32 x  = axis.x;
	axis.x = axis.z;
	axis.z = -x;
	tilt.x = axis.x;
	tilt.y = axis.y;
	tilt.z = axis.z;
	axis.y = 0.0f;
	vecNormalize(&axis, &axis);
	vecNormalize(&tilt, &tilt);
	njCreateQuaternionV((const Vec3f*)&axis, (int)(10430.38043493439 * (f32)acos(tilt.y)), (NJS_QUATERNION*)mQuat);
}

// 0x800A10CC
void TBowlPin::rest()
{
}

// 0x800A10D0
// Each end falls and bounces off the floor 20 below where it would be
void TBowlPin::move()
{
	EggHits hits;

	mVel[0].y += -0.15f;
	mVel[2].y += -0.15f;
	if (mVel[0].y < 0.0f) {
		Vec3f probe = above(mPos[0], 20.0f);
		if (meshColliProbe(hits.mHits, &probe, -1) != 0 && (hits.mMask & 1)) {
			f32 floor = 8.0f + hits.mHits[0].mPos.y;
			if (mPos[0].y <= floor) {
				f32 dy = floor - mPos[0].y;
				if ((f32)__fabs(dy) > 0.4f) {
					mPos[2].y += dy;
				}
				mPos[0].y = floor;
				mVel[0].x *= 0.92f;
				mVel[0].y *= -0.6f;
				mVel[0].z *= 0.92f;
				if ((f32)__fabs(mVel[0].y) < 0.4f) {
					mVel[0].x = 0.0f;
					mVel[0].y = 0.0f;
					mVel[0].z = 0.0f;
				}
			}
		}
	}
	vecAdd(&mPos[0], &mPos[0], &mVel[0]);
	if (mVel[2].y < 0.0f) {
		Vec3f probe = above(mPos[2], 20.0f);
		if (meshColliProbe(hits.mHits, &probe, -1) != 0 && (hits.mMask & 1)) {
			f32 floor = 8.0f + hits.mHits[0].mPos.y;
			if (mPos[2].y <= floor) {
				f32 dy = floor - mPos[2].y;
				if ((f32)__fabs(dy) > 0.4f) {
					mPos[0].y += dy;
				}
				mPos[2].y = floor;
				mVel[2].x *= 0.92f;
				mVel[2].y *= -0.6f;
				mVel[2].z *= 0.92f;
				if ((f32)__fabs(mVel[2].y) < 0.4f) {
					mVel[2].x = 0.0f;
					mVel[2].y = 0.0f;
					mVel[2].z = 0.0f;
				}
			}
		}
	}
	vecAdd(&mPos[2], &mPos[2], &mVel[2]);
}

// 0x800A1358
// Puts the ends back 12 either side of the middle
void TBowlPin::keepLength()
{
	Vec3f dir;
	vecSub(&dir, &mPos[2], &mPos[0]);
	vecScaleAdd(0.5f, &mPos[1], &dir, &mPos[0]);
	vecNormalize(&dir, &dir);
	vecScaleAdd(12.0f, &mPos[2], &dir, &mPos[1]);
	vecScaleAdd(-12.0f, &mPos[0], &dir, &mPos[1]);
}

// 0x800A13D8
void TBowlPin::updateMatrix()
{
	mtxSetTransV(mMtx, &mPos[1]);
	mtxConcatQuat(mMtx, mMtx, mQuat);
}

// 0x800A141C
void TBowlPin::render()
{
	f32 mtx[12];
	f32 flat[12];
	u32 color;

	mtxTranslateV(mtx, gjGetMatrix(), &mPos[1]);
	mtxConcatQuat(mtx, mtx, mQuat);
	mtxScale(8.0f, 20.0f, 8.0f, mtx, mtx);
	mtxCopy(flat, mtx);
	mtxRotateX(flat, flat, 0x4000);
	color = 0x3F7FFF7F;
	debugDrawCylinder(flat, &color);
	debugWireCylinder(mtx, &gColorBlack);
}
