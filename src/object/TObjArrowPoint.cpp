#include "stage/SetActor.h"
#include "object/ObjScript.h"
#include "object/ObjAction.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "object/TObjFirePole01.h"
#include "object/TObjSkull.h"
#include "object/TObjLauncher.h"
#include "sdk/stl/math.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "PowerPC_EABI_Support/Runtime/runtime.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
#include "object/TObjArrowPoint.h"
#include "animal/RacerMover.h"

static const ColliColor sMarkerColor = { 0xFF, 0xFF, 0xFF, 0x80 };
struct SetLink;

enum ArrowPointState {
	ARROW_WAIT  = 0, // for the key bit
	ARROW_TRACK = 1, // for the skull's jaw to open
	ARROW_FLY   = 2, // the ball is in the air
	ARROW_FIRE  = 3, // the fire burns
	ARROW_DONE  = 4,
};

extern "C" {
// 0x802C3D10
ObjClassName sArrowPointName = { "TObjArrowPoint", 0 };
}

typedef char TObjArrowPoint_size_check[(sizeof(TObjArrowPoint) == 0xA0) ? 1 : -1];

// 0x80053DCC
extern "C" TObject* createTObjArrowPoint(TObject* parent, SetDesc* desc)
{
	return new TObjArrowPoint(parent, desc);
}

// 0x80053E20
void TObjArrowPoint::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mSkull = (s16)desc->getInt(0);
	mKey   = (s16)desc->getInt(3);
	mAnim  = 0.4f + desc->getFloat(2);
	mScale = 1.0f + desc->getFloat(3);
}

// 0x80053ED0
void TObjArrowPoint::debugPrint(void*)
{
	njPrintC(0xA0013, ":DOKURO ID");
	njPrintC(0xA0014, ":COL TYPE");
	njPrintC(0xA0016, ":KEY ID");
	njPrintC(0xA0019, "+0.1:ANI SPEED");
	njPrintC(0xA001A, "+1.0:SCALE");
	ColliColor color = sMarkerColor;
	Vec3f at         = mPos;
	at.y += 10.0f * mScale;
	debugCylinder((const Vec3f*)&at, (GXColor*)&color, 10.0f * mScale, 10.0f * mScale);
}

// 0x80053F98
TObjArrowPoint::TObjArrowPoint(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)sArrowPointName.mName;
	mSize = 0xA0;
	initFromDesc(desc);
	vfn_74();
}

// 0x8005402C
TObjArrowPoint::~TObjArrowPoint()
{
	if (mChild != 0) {
		mChild->mFlags |= 1;
		mChild = 0;
	}
}

// 0x800540D0
void TObjArrowPoint::vfn_74()
{
	mFrom.setId(0xFFFF);
	mTrack.setId(0xFFFF);
	mMarker.setId(0xFFFF);
	mChild = 0;
	if (((u32)mLink.get()->getData() & 1) != 0) {
		raiseMarker();
		aimAtSkull();
		mAct = ARROW_FIRE;
	} else if (((u32)mLink.get()->getData() & 2) != 0) {
		aimAtSkull();
		mAct = ARROW_DONE;
	} else {
		mAct = ARROW_WAIT;
	}
}

// 0x80054188
void TObjArrowPoint::stateWait()
{
	if (keyBitIsSet(mKey) == 1) {
		wakeSkull();
	}
}

// 0x800541C8
void TObjArrowPoint::stateTrack()
{
	TObjAction* from;

	from = mFrom.get();
	if (from != 0 && ((TObjSkull*)from)->isJawOpen() != 0) {
		launchFrom(from);
		mAct = ARROW_FLY;
	}
}

// A hidden pole at twice the size its flame turning with the animation rate
inline void TObjArrowPoint::makeMarker()
{
	TObjFirePole01* marker = new TObjFirePole01(this);
	if (marker == 0) {
		return;
	}
	marker->setPos(&mPos);
	setLinkTimed((SetLink*)&marker->mLink, 0);
	marker->mRebirth = 0;
	marker->mWait    = 0;
	marker->mFire    = 0;
	marker->mStep    = __cvt_fp2unsigned(1.0f / mAnim);
	marker->mBits |= FIRE_POLE_HIDDEN;
	marker->mScale  = 2.0f;
	marker->mDamage = 10;
	mMarker.setId(marker->mObjId);
}

// 0x80054220
// Once the ball lands the fire is lit where it came down
void TObjArrowPoint::stateFly()
{
	TObjAction* ball   = mTrack.get();
	TObjAction* marker = mMarker.get();
	if (ball == 0) {
		return;
	}
	if (((TObjLauncher*)ball)->mBits & TObjLauncher::BIT_STOPPED) {
		makeMarker();
		mAct = ARROW_FIRE;
		mLink.get()->setData((void*)1);
	}
	if (marker != 0) {
		((TSetActor*)marker)->setPos(ball->getPos());
	}
}

// 0x80054358
void TObjArrowPoint::stateFire()
{
	TObjAction* marker;

	marker = mMarker.get();
	if (marker != 0 && (((TObjFirePole01*)marker)->mBits & FIRE_POLE_OUT) != 0) {
		mAct = ARROW_DONE;
		mLink.get()->setData((void*)2);
	}
}

// 0x800543B0
void TObjArrowPoint::stateDone()
{
}

// 0x800543B4
void TObjArrowPoint::wakeSkull()
{
	TObjSkull* skull = (TObjSkull*)findSkull(mSkull);
	if (skull != 0) {
		mFrom.setId(skull->mObjId);
		if (skull->lightEyes(mKey) && skull->openJaw()) {
			mAct = ARROW_TRACK;
		}
	}
}

static inline Vec3f vecDiff(Vec3f* a, Vec3f* b)
{
	Vec3f out;
	gjSubVector(&out, a, b);
	return out;
}

// 0x80054424
// Out of one side of the mouth or the other timed to come down here under a gravity of 0.0817
void TObjArrowPoint::launchFrom(TObjAction* from)
{
	if (from == 0) {
		return;
	}
	Vec3f mouth;
	Vec3f start = *from->getPos();
	if (rand() % 2 == 0) {
		mouth.x = -9.5f;
		mouth.y = 0.8f;
		mouth.z = 10.0f;
	} else {
		mouth.x = 11.4f;
		mouth.y = 0.8f;
		mouth.z = 10.0f;
	}
	njUnitMatrix(NULL);
	gjRotateY(from->getRotY());
	njCalcPointEx(&mouth, &mouth);
	njAddVector(&start, &mouth);
	Vec3f target = mPos;
	f32 drop     = start.y - target.y;
	u32 frames   = __cvt_fp2unsigned(dolsqrtf(2.0f * drop / 0.08166667f));
	Vec3f flat   = start;
	flat.y       = 0.0f;
	target.y     = 0.0f;
	Vec3f dir    = vecDiff(&target, &flat);
	f32 dist     = njUnitVector(&dir);
	if (dist < gUnitVectorMin) {
		dir = gVecZero;
	}
	gjScaleVector(dist / (f32)frames, &dir, &dir);
	void* model        = getGeObjModels()[5];
	void* texture      = getGeObjTextures()[5];
	TObjLauncher* ball = new TObjLauncher(this, mPos.y, 1.0f, &start, &dir, model, texture);
	if (ball != 0) {
		ball->takeObjId();
		ball->faceVelocity();
		ball->setGravity(0.08166667f);
		ball->noDrag();
		ball->keepAtFloor();
		mTrack.setId(ball->mObjId);
	}
}

// 0x800546C4
void TObjArrowPoint::raiseMarker()
{
	makeMarker();
}

// 0x80054784
void TObjArrowPoint::aimAtSkull()
{
	TSetActor* skull;
	Vec3f* at;
	Vec3f neg;

	skull = (TSetActor*)findSkull(mSkull);
	if (skull == 0) {
		return;
	}
	Vec3f t = vecDiff(skull->getPos(), &mPos);
	neg.y   = t.y;
	neg.x   = t.x;
	neg.z   = t.z;
	neg.y *= 2.0f;
	gjScaleVector(-1.0f, &neg, &neg);
}

// 0x80054828
void TObjArrowPoint::draw()
{
	if (!isInRange(this, &mPos, 600.0f)) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if (mSkull == 0) {
		setLinkDone((SetLink*)&mLink);
		mFlags |= 1;
	}
	switch (mAct) {
	case ARROW_WAIT:
		stateWait();
		break;
	case ARROW_TRACK:
		stateTrack();
		break;
	case ARROW_FLY:
		stateFly();
		break;
	case ARROW_FIRE:
		stateFire();
		break;
	case ARROW_DONE:
		stateDone();
		break;
	}
}

// 0x8005490C
void TObjArrowPoint::render()
{
}

// 0x80054910
// The ball is where its transform is
Vec3f* TObjLauncher::getPos()
{
	return &mX.mPos;
}
