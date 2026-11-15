#include "stage/SetManager.h"
struct DrawTask;
#include "stage/SetActor.h"
#include "engine/DrawTask.h"
#include "object/ObjScript.h"
#include "object/ObjArchive.h"
#include "object/ObjAction.h"
#include "object/ObjShadow.h"
#include "object/last/TObjLastCrowLemures.h"
#include "object/last/TObjLastHeavensGate.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "engine/Colli.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "object/last/TObjLastCrowLemuresCtrl.h"
#include "Ninja/ninja.h"
#include "engine/ModelDraw.h"
#include "object/ObjCommon.h"
#include "object/last/TObjLastExBoss_Gareki.h"
#include "io/NinjaFile.h"
#include "object/TObjLauncher.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "egg/EggCreature.h"
struct ColliContact;
struct SetLink;
struct ColliShape;
struct CharaColli;

extern "C" {
const f32 gLastCrowLemuresCtrlSetRange = 1000.0f;

extern const char* gTObjLastCrowLemuresName;
extern const char* gTObjLastHeavensGateName;

// 0x802C4650
const char* gTObjLastCrowLemuresCtrlName = "TObjLastCrowLemuresCtrl";

}

typedef char TObjLastCrowLemuresCtrl_size_check[(sizeof(TObjLastCrowLemuresCtrl) == 0xA0) ? 1 : -1];

extern "C" TObject* createTObjLastCrowLemuresCtrl(TObject* parent, SetDesc* desc)
{
	return new TObjLastCrowLemuresCtrl(parent, desc);
}

// 0x80190554
TObjLastCrowLemuresCtrl::TObjLastCrowLemuresCtrl(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_70    = 0;
	_90    = 0;
	mTimer = 60;
	_00    = (u32)gTObjLastCrowLemuresCtrlName;
	mSize  = 0xA0;
	mModel = getGeObjModels()[3];
	void* bank = *(void**)((u8*)gGeObjCommon + 0xC);
	mMotion    = *(void**)((u8*)bank + 0x1C);
	mDraw      = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x23, 0);
	mShape     = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 0xFF, 0xFF, 0xF7);
	mGate = new TObjLastHeavensGate(this);
	initFromDesc(desc);
	arm();
	for (u16 i = 0; i < 16.0f; i++) {
		TObjLastCrowLemures* crow = new TObjLastCrowLemures(this);
		if (crow != 0 && crow->mShape != 0) {
			Vec3f* shape = (Vec3f*)colliGetShape(crow->mShape, 0);
			*shape       = mPos;
		}
	}
}

// 0x8019071C
TObjLastCrowLemuresCtrl::~TObjLastCrowLemuresCtrl()
{
	if (mGate != 0) {
		((TObject*)mGate)->mFlags |= 1;
		mGate = 0;
	}
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDraw);
}

// 0x801907B4
void TObjLastCrowLemuresCtrl::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mSwitchId = (u16)CLAMP_RANGE(desc->getInt(0), 0, 0x800);
	colliShapeMakeCylinder(colliGetShape(mShape, 0), &mPos, 0, desc->getFloat(0), desc->getFloat(1));
	if (mGate != 0) {
		Vec3f off;
		off.set(0.0f, 0.0f, desc->getFloat(2));
		vecRotateY(&off, &off, mRot.y);
		vecAdd(&off, &off, &mPos);
		mGate->setRadius(desc->getFloat(3));
		mGate->place(&off);
	}
}

// 0x80190948
void TObjLastCrowLemuresCtrl::debugPrint(void*)
{
	njPrintC(0xA0013, ":SWITCH ID");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, ":AREA RAD");
	njPrintC(0xA0018, ":AREA HEIGHT");
	njPrintC(0xA0019, ":GATE DIST");
	njPrintC(0xA001A, ":GATE RAD");
}

// 0x801909F4
void TObjLastCrowLemuresCtrl::arm()
{
	if (keyBitIsSet(mSwitchId) == 0) {
		mMode = 0;
	} else {
		mMode = 2;
	}
}

// 0x80190A40
void TObjLastCrowLemuresCtrl::place(Vec3f* pos, f32* radius) const
{
	*pos = mPos;
	*radius = *(f32*)((u8*)colliGetShape(mShape, 0) + 0x18);
}

// 0x80190A94
f32 TObjLastCrowLemuresCtrl::range()
{
	if (mGate != 0) {
		Vec3f gate;
		mGate->getCenter(&gate);
		vecSub(&gate, &gate, &mPos);
		return vecLength(&gate) - mGate->getRadius();
	}
	return *(f32*)((u8*)colliGetShape(mShape, 0) + 0x18);
}

// 0x80190B1C
void TObjLastCrowLemuresCtrl::draw()
{
	if (isInRange((void*)this, (const Vec3f*)&mPos, 1050.0f) == 0) {
		mFlags |= 1;
		return;
	}
	switch (mMode) {
	case 0:
		stateWatch();
		break;
	case 1:
		stateOpen();
		break;
	case 2:
		stateDone();
		break;
	}
}

// 0x80190BB0
void TObjLastCrowLemuresCtrl::stateWatch()
{
	int touched = 0;
	for (ColliContact* node = mShape->mContacts; node != 0; node = node->mNext) {
		ColliShape* shape = colliContactShape(node);
		if ((shape->mFlags & 0x400) != 0) {
			touched = 1;
		}
	}
	int any = 0;
	for (TObject* child = mChild; child != 0;
	     child = child->mNext) {
		if (child->_00 == (u32)gTObjLastCrowLemuresName) {
			((TObjLastCrowLemures*)child)->touch(touched);
			any = 1;
		}
	}
	if (any == 0) {
		mMode = 1;
	}
}

// 0x80190C54
void TObjLastCrowLemuresCtrl::stateOpen()
{
	u16 left = mTimer;
	mTimer    = left - 1;
	if (left == 0) {
		setKeyBit(mSwitchId);
		mMode = 2;
	}
}

// 0x80190CA0
void TObjLastCrowLemuresCtrl::stateDone()
{
	setLinkDone((SetLink*)&mLink);
	mFlags |= 1;
}

// 0x80190CDC
static inline ColliShape* firstShape(CharaColli* colli)
{
	ColliShape* s = colliGetShape(colli, 0);
	return s;
}

void TObjLastCrowLemuresCtrl::vfn_10()
{
	Vec3f* shape = (Vec3f*)firstShape(mShape);
	Vec3f pos;
	pos.set(shape->x, shape->y, shape->z);
	if (cameraIsVisible(1000.0f, *(f32*)((u8*)shape + 0x18), &pos, 0)) {
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x80190D6C
void TObjLastCrowLemuresCtrl::render()
{
	shadowVolumeBegin();
	for (TObject* child = mChild; child != 0;
	     child = child->mNext) {
		if (child->_00 == (u32)gTObjLastCrowLemuresName) {
			child->render();
		}
	}
	shadowVolumeEnd();
	if (objDebugEnabled(this) != 0) {
		if (mGate != 0) {
			mGate->drawShape();
		}
		colliDrawShapes(mShape);
	}
}

struct NJS_OBJECT;
struct ColliContact;
struct ColliShape;
struct CharaColli;
struct Fx;

extern "C" {

}

/** A message that carries a vector back */
struct VecMsg : public ObjMsg {
	Vec3f mVec; // _04
};

typedef char TObjLastCrowLemures_size_check[(sizeof(TObjLastCrowLemures) == 0x98) ? 1 : -1];

const char* gTObjLastCrowLemuresName = "TObjLastCrowLemures";

// 0x80190DFC
static inline f32 randomRange(f32 lo, f32 span)
{
	return span * (3.0517578125e-05f * (f32)(int)rand()) + lo;
}

TObjLastCrowLemures::TObjLastCrowLemures(TSetActor* owner)
    : TObject(owner), mOwner(owner)
{
	mScale = randomRange(2.0f, 1.0f);
	mYaw   = 0;
	mSpeed = 1.0f;
	_00    = (u32)gTObjLastCrowLemuresName;
	mSize  = 0x98;
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)owner->mObjId);
	colliSetMasks((CharaColli*)mShape, 2, 0xFF, 0xFF, 0xF6);
	colliShapeMakeCylinder(colliGetShapes((CharaColli*)mShape), &gVecZero, 0, 5.0f * mScale, 20.0f);
	f32 radius;
	((TObjLastCrowLemuresCtrl*)owner)->place(&mPos, &radius);
	mTarget = mPos;
	mtxIdentity(mMtx);
	void* motion = *(void**)((u8*)mOwner + 0x68);
	mTrack.set(motion, 1, 0.4f);
}

// 0x80190F40
TObjLastCrowLemures::~TObjLastCrowLemures()
{
	colliRetire((CharaColli*)mShape);
}

// 0x80190FB4
void TObjLastCrowLemures::touch(int on)
{
	if (on != 0) {
		colliEnable(mShape);	
	} else {
		colliDisable(mShape);
	}
}

// 0x80190FEC
void TObjLastCrowLemures::draw()
{
	Vec3f center;
	Vec3f where;
	Vec3f push;
	Vec3f toTarget;
	VecMsg hitMsg;
	Vec3f away;
	Vec3f fromCenter;
	f32 radius;
	ObjMsg msg;
	((TObjLastCrowLemuresCtrl*)mOwner)->place(&center, &radius);
	radius -= *(f32*)((u8*)colliGetShape(mShape, 0) + 0x18);
	f32 inner = 1.0f + radius;
	f32 room  = ((TObjLastCrowLemuresCtrl*)mOwner)->range() - *(f32*)((u8*)colliGetShape(mShape, 0) + 0x18) - 1.0f;

	int bounced = 0;
	for (ColliContact* node = mShape->mContacts; node != 0; node = node->mNext) {
		TObjAction* other = findObjById(colliContactOwner(node));
		if (other == 0) {
			continue;
		}
		if (other->_00 == (u32)gTObjLastHeavensGateName) {
			if (debugEditActive() == 0) {
				fxSpawnAt(1, 8, 4, mMtx);
				sePlay3D(0x285, &mPos);
				sePlay3D(0x286, &mPos);
				mFlags |= 1;
			}
			break;
		}
		if (colliContactHasBit8(node) == 0) {
			continue;
		}
		ColliShape* shape = colliContactShape(node);
		if ((shape->mFlags & 0x400) == 0) {
			continue;
		}
		TObjAction* target = findObjById(colliContactOwner(node));
		if (target == 0) {
			continue;
		}
		msg.mType = 0x41;
		if ((target->sendToHandle(&msg) & 8) != 0) {
			continue;
		}
		HitInfo hit;
		if (target->onHit(mOwner, 0, (int*)&hit) == 0) {
			continue;
		}
		where = *target->getPos();
		hitMsg.mType = 0x43;
		target->sendToHandle(&hitMsg);
		vecSub(&away, &mPos, &where);
		away.y = 0.0f;
		hitMsg.mVec.y = 0.0f;
		vecNormalize(&hitMsg.mVec, &hitMsg.mVec);
		vecNormalize(&away, &away);
		if (vecDotXZ(&hitMsg.mVec, &away) < 0.0f) {
			continue;
		}
		int angle = vecAngleXZ(&hitMsg.mVec);
		vecFromAngleXZ(60.0f, &away, angle);
		vecNormalize(&push, &away);
		mSpeed = 2.0f;
		vecScale(2.0f, &push, &push);
		vecAdd(&mTarget, &away, &mPos);
		bounced = 1;
	}

	vecSub(&toTarget, &mTarget, &mPos);
	toTarget.y = 0.0f;
	mYaw       = vecAngleXZ(&toTarget);
	vecSub(&fromCenter, &mPos, &center);
	fromCenter.y = 0.0f;
	f32 dist     = vecLength(&fromCenter);
	if (bounced) {
		vecAdd(&mPos, &mPos, &push);
		vecSub(&toTarget, &mPos, &center);
		toTarget.y = 0.0f;
		if (vecLength(&toTarget) > inner) {
			vecNormalize(&mPos, &toTarget);
			vecScale(radius, &mPos, &mPos);
			vecRotateY(&mPos, &mTarget, 3.0517578125e-05f * (f32)(int)rand() < 0.5f ? 0x200 : -0x200);
			vecAdd(&mPos, &mPos, &center);
			vecAdd(&mTarget, &mTarget, &center);
		}
	} else if (vecLength(&toTarget) < 1.0f) {
		mPos      = mTarget;
		mTarget.x = 0.0f;
		mTarget.y = 0.0f;
		mTarget.z = room * (3.0517578125e-05f * (f32)(int)rand());
		int angle = (int)(65536.0f * (3.0517578125e-05f * (f32)(int)rand()));
		vecRotateY(&mTarget, &mTarget, angle);
		vecAdd(&mTarget, &mTarget, &center);
	} else if (dist > inner) {
		vecNormalize(&mPos, &fromCenter);
		vecScale(radius, &mPos, &mPos);
		vecAdd(&mPos, &mPos, &center);
		mTarget.x = 0.0f;
		mTarget.y = 0.0f;
		mTarget.z = room * (3.0517578125e-05f * (f32)(int)rand());
		int angle = (int)(65536.0f * (3.0517578125e-05f * (f32)(int)rand()));
		vecRotateY(&mTarget, &mTarget, angle);
		vecAdd(&mTarget, &mTarget, &center);
	} else {
		vecNormalize(&toTarget, &toTarget);
		vecScale(mSpeed, &toTarget, &toTarget);
		vecAdd(&mPos, &mPos, &toTarget);
		mSpeed *= 0.98f;
		mSpeed = mSpeed < 1.0f ? 1.0f : mSpeed;
	}

	mtxSetTransV(mMtx, &mPos);
	mtxRotateY(mMtx, mMtx, mYaw);
	mtxScale(mScale, 1.0f, mScale, mMtx, mMtx);
	Vec3f* shape = (Vec3f*)colliGetShape(mShape, 0);
	*shape       = mPos;
	mTrack.update();
}

// 0x801915FC
void TObjLastCrowLemures::render()
{
	f32 view[12];
	mtxConcat(view, gjGetMatrix(), mMtx);
	njPushMatrix(view);
	shadowVolumeAdd();
	gjDrawMotion((NJS_OBJECT*)(*(void**)((u8*)mOwner + 0x64)), mTrack.mMotion, mTrack.mFrame);
	shadowVolumeSubtract();
	gjDrawMotion((NJS_OBJECT*)(*(void**)((u8*)mOwner + 0x64)), mTrack.mMotion, mTrack.mFrame);
	njPopMatrixEx();
}

typedef char TObjLastHeavensGate_size_check[(sizeof(TObjLastHeavensGate) == 0x6C) ? 1 : -1];

const char* gTObjLastHeavensGateName = "TObjLastHeavensGate";

// 0x80191674
TObjLastHeavensGate::TObjLastHeavensGate(TSetActor* owner)
    : TObjAction(owner)
{
	mOwner = owner;
	_00    = (u32)gTObjLastHeavensGateName;
	mSize  = 0x6C;
	setObjId(this, -1);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 1, 0xFF, 0xFF, 0xFD);
	colliShapeMakeSphere(colliGetShapes((CharaColli*)mShape), &gVecZero, 0, 10.0f);
	mtxIdentity(mMtx);
	mFx = fxSpawnFollow(1, 8, 0xF, mMtx);
}

// 0x80191748
TObjLastHeavensGate::~TObjLastHeavensGate()
{
	fxStop((Fx*)mFx);
	colliRetire((CharaColli*)mShape);
	unregisterId();
}

// 0x801917C0
void TObjLastHeavensGate::place(const Vec3f* pos)
{
	*(Vec3f*)colliGetShapes(mShape) = *pos;
	((Vec3f*)colliGetShapes(mShape))->y += 37.5f;
	Vec3f* shape = (Vec3f*)colliGetShapes(mShape);
	Vec3f at;
	at.set(shape->x, shape->y, shape->z);
	mtxSetTransV(mMtx, &at);
	mtxRotateY(mMtx, mMtx, angleToXZ(pos, mOwner->getPos()));
}

// 0x80191880
void TObjLastHeavensGate::setRadius(f32 radius)
{
	*(f32*)((u8*)colliGetShapes(mShape) + 0x18) = radius;
}

// 0x801918B4
void TObjLastHeavensGate::getCenter(Vec3f* pos) const
{
	*pos = *(Vec3f*)colliGetShapes(mShape);
}

// 0x801918FC
f32 TObjLastHeavensGate::getRadius() const
{
	return *(f32*)((u8*)colliGetShapes(mShape) + 0x18);
}

// 0x80191924
void TObjLastHeavensGate::drawShape()
{
	colliDrawShapes(mShape);
}

// 0x80191948
int TObjLastHeavensGate::getKind()
{
	return 3;
}
