#include "stage/SetActor.h"
#include "object/ObjScript.h"
#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "engine/CharaColli.h"
#include "object/ObjArchive.h"
#include "sound/SoundHandle.h"
#include "stage/StageObjUtil.h"
#include "engine/TParticle.h"
#include "object/TObjFirePole01.h"
#include "engine/DrawState.h"
#include "ui/AeArc.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "stage/SetSlot.h"
struct TParticleGenerator;
struct SetLink;

extern "C" {
static const ColliColor sFirePoleColliColor = { 0xFF, 0xFF, 0xFF, 0x80 };

// 0x802C3D30
ObjClassName sFirePoleName = { "TObjFirePole01", 0 };
}

typedef char TObjFirePole01_size_check[(sizeof(TObjFirePole01) == 0xCC) ? 1 : -1];

// 0x80056CB0
extern "C" TObject* createTObjFirePole01(TObject* parent, SetDesc* desc)
{
	return new TObjFirePole01(parent, desc);
}

// 0x80056D04
void TObjFirePole01::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mRebirth = (u32)(60.0f * (f32)(desc->getInt(0) + 10));
	mWait    = (u32)(60.0f * (f32)(desc->getInt(1) + 2));
	mFire    = (u32)(60.0f * (f32)(desc->getInt(2) + 1));
	mDamage  = desc->getInt(3) + 20;
	mScale   = 1.0f + desc->getFloat(3);
}

// 0x80056E50
void TObjFirePole01::debugPrint(void*)
{
	njPrintC(0xA0013, "+10SEC:REBIRTH TIME");
	njPrintC(0xA0014, "+2SEC:WAIT TIME");
	njPrintC(0xA0015, "+1SEC:FIRE TIME");
	njPrintC(0xA0016, "+20:DAMAGE");
	njPrintC(0xA001A, "+1.0:COLLI SCALE");
	ColliColor color = sFirePoleColliColor;
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY(mRot.y);
	debugCylinder((const Vec3f*)(&colliGetShapes(mShape)->mPos), (GXColor*)&color, colliGetShapes(mShape)->mSizeX, colliGetShapes(mShape)->mSizeY);
	njPopMatrixEx();
}

// 0x80056F20
TObjFirePole01::TObjFirePole01(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)sFirePoleName.mName;
	mSize = 0xCC;
	initFromDesc(desc);
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	vfn_74();
}

// 0x80056FC8
// TObjArrowPoint raises one of these with itself as the parent
TObjFirePole01::TObjFirePole01(TObject* parent)
    : TSetActor(parent)
{
	_00   = (u32)sFirePoleName.mName;
	mSize = 0xCC;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	vfn_74();
}

// 0x80057054
TObjFirePole01::~TObjFirePole01()
{
	seStop3D(&mSnd);
	if (mFlame != NULL) {
		mFlame->mFlags |= 1;
		mFlame = NULL;
	}
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDraw);
}

// 0x800570F4
void TObjFirePole01::vfn_74()
{
	mTick  = 0;
	mFrame = 0;
	mBits  = FIRE_POLE_IN_RANGE;
	mFlame = NULL;
	if ((mModel = getStgObjModels()[1]) == 0) {
		mFlags |= 1;
		return;
	}
	if ((mTexture = getStgObjTextures()[1]) == 0) {
		mFlags |= 1;
		return;
	}
	setLinkFree((SetLink*)&mLink);
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 255, 0xD6);
	placeColli();
	if (mWait == 0) {
		Vec3f at = mPos;
		at.y += 10.0f;
		mFlame = particleGenByIdDefault(&at, 0, 0xD8);
		startSound();
		mState = FIRE_POLE_BURN;
	} else {
		mFlame = NULL;
		colliDisable(mShape);
		mState = FIRE_POLE_WAIT;
	}
}

// 0x80057234
// The fire's roar heard further for a bigger pole
void TObjFirePole01::startSound()
{
	sePlay3DHeld(8.0f * mScale + 180.0f, &mSnd, 0x58, 0, (const Vec3f*)&mPos);
}

// 0x80057278
// A hit with bit 7 water puts the fire out and leaves smoke
void TObjFirePole01::checkDouse()
{
	ContactQuery query;
	if (query.findHit(this, (CharaColli*)mShape) && query.mHit.mFlags.testBit(7)) {
		Vec3f at = mPos;
		at.y += 5.0f;
		if (mFlame != NULL) {
			mFlame->mFlags |= 1;
			mFlame = NULL;
		}
		mFlame = particleGenByIdDefault(&at, 0, 0xDC);
		mTick  = 0;
		colliDisable(mShape);
		mBits |= FIRE_POLE_OUT;
		mState = FIRE_POLE_DOUSED;
	}
}

// 0x80057360
void TObjFirePole01::stateWait()
{
	if (mTick++ > mWait) {
		mTick    = 0;
		Vec3f at = mPos;
		at.y += 10.0f * mScale;
		mFlame = particleGenByIdDefault(&at, 0, 0xD8);
		colliEnable(mShape);
		startSound();
		mState = FIRE_POLE_BURN;
	}
}

// 0x80057400
// With no wait it burns for good
void TObjFirePole01::stateBurn()
{
	if (mWait != 0 && mTick++ > mFire) {
		if (mFlame != NULL) {
			mFlame->mFlags |= 1;
			mFlame = NULL;
		}
		mTick = 0;
		colliDisable(mShape);
		seStop3D(&mSnd);
		mState = FIRE_POLE_WAIT;
	}
	checkDouse();
	TParticleGenerator* flame = mFlame;
	if (flame != NULL) {
		Vec3f at = mPos;
		at.y += 10.0f;
		flame->mPos.x = at.x;
		flame->mPos.y = at.y;
		flame->mPos.z = at.z;
	}
}

// 0x800574DC
// The smoke lasts 30 frames
void TObjFirePole01::stateDoused()
{
	if (mRebirth == 0) {
		if (mFlame != NULL) {
			mTick++;
			if ((f32)mTick > 30.0f) {
				mFlame->mFlags |= 1;
				mFlame = NULL;
				seStop3D(&mSnd);
			}
		}
	} else {
		mTick++;
		if ((f32)mTick > 30.0f && mFlame != NULL) {
			mFlame->mFlags |= 1;
			mFlame = NULL;
			seStop3D(&mSnd);
		}
		if (mTick > mRebirth) {
			Vec3f at = mPos;
			at.y += 10.0f * mScale;
			mFlame = particleGenByIdDefault(&at, 0, 0xD8);
			mTick  = 0;
			colliEnable(mShape);
			mBits &= ~FIRE_POLE_OUT;
			startSound();
			mState = FIRE_POLE_BURN;
		}
	}
}

// 0x8005764C
// A cylinder 8 times the scale across and as high raised by its radius
void TObjFirePole01::placeColli()
{
	Vec3f raise;
	raise.x      = 0.0f;
	raise.y      = 0.0f;
	raise.z      = 0.0f;
	f32 radius   = 8.0f * mScale;
	raise.y      = radius;
	Vec3f offset = raise;
	colliSetCylinder((CharaColli*)mShape, &mPos, 0x88, &offset, radius, radius);
	colliRefreshBounds((CharaColli*)mShape);
}

// 0x800576CC
// Anything but a set object takes mDamage as fire
int TObjFirePole01::onHit(const TObjAction* other, int, int* result)
{
	if (((TObjAction*)other)->getKind() != 3) {
		HitInfo* hit = (HitInfo*)result;
		resetHitInfo(hit);
		hit->mDamage = mDamage;
		hit->mFlags.setBit(4);
		hit->mFlags.setBit(0xD);
		return 1;
	}
	return 0;
}

// 0x80057748
void TObjFirePole01::draw()
{
	u32 frame;

	if ((mBits & FIRE_POLE_IN_RANGE) != 0 && !isInRange(this, &mPos, 600.0f)) {
		if (mLink.get()->mMode != 0) {
			setLinkFree((SetLink*)&mLink);
		}
		mFlags |= 1;
		return;
	}
	switch (mState) {
	case FIRE_POLE_WAIT:
		stateWait();
		break;
	case FIRE_POLE_BURN:
		stateBurn();
		break;
	case FIRE_POLE_DOUSED:
		stateDoused();
		break;
	}
	camGetDrawIndex();
	mRot.y = (int)(((u32 (*)())camGetYaw)() & 0xFFFF);
	if (mTick % mStep == 0) {
		frame  = mFrame;
		mFrame = frame + 1;
		if (frame == 11) {
			mFrame = 0;
		}
	}
	placeColli();
}

// 0x80057848
void TObjFirePole01::vfn_10()
{
	Vec3f screen;

	if (cameraIsVisible(500.0f, 30.0f, getPos(), &screen)) {
		mDraw->mSortKey = drawTaskDepthKey(screen.z);
		mDraw->mFlags &= ~1;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x800578CC
void TObjFirePole01::render()
{
	njPushMatrixEx();
	njTranslateEx(&mPos);
	if ((mBits & FIRE_POLE_HIDDEN) == 0) {
		njSetTexture(mTexture);
		drawObjectDefault(mModel);
	}
	njPopMatrixEx();
}
