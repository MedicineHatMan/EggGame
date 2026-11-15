#include "object/purple/TObjPurplePropeller.h"
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "object/ObjScript.h"
#include "object/ObjArchive.h"
#include "engine/TParticle.h"
#include "ui/AeArc.h"
#include "object/TObjLauncher.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "object/TObjKey00.h"
#include "stage/SetSlot.h"
struct NJS_TEXLIST;
struct SetLink;

extern "C" {

// 0x802C4220
ObjClassName gTObjPurplePropellerName = { "TObjPurplePropeller", 0 };
}

// A particle generator's position sits at +0x24
struct PropMesh {
	u8 _00[0x14];
	Vec3f mCentre; // _14
	f32 mRadius;   // _20
};

struct PropNode {
	u8 _00[4];
	PropMesh* mMesh; // _04
};

typedef char TObjPurplePropeller_size_check[(sizeof(TObjPurplePropeller) == 0x148) ? 1 : -1];

// 0x8010C4B0
extern "C" TObject* createTObjPurplePropeller(TObject* parent, SetDesc* desc)
{
	return new TObjPurplePropeller(parent, desc);
}

// 0x8010C504
TObjPurplePropeller::TObjPurplePropeller(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00       = (u32)gTObjPurplePropellerName.mName;
	mSize     = 0x148;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mGeo      = findGeObjData("MODEL_PROPELLER");
	mTex      = findGeObjData("TLS_MODEL_PROPELLER");
	mAngle    = 0;
	initFromDesc(desc);
	vfn_74();
}

// 0x8010C5DC
TObjPurplePropeller::~TObjPurplePropeller()
{
	seStop3D(&mSound);
	removeDrawTask(mDrawTask);
}

// 0x8010C658
void TObjPurplePropeller::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	mFrom     = (u16)desc->getInt(2);
	mTo       = (u16)desc->getInt(3);
	mRotSpeed = desc->getInt(0);
	mScale    = 1.0f + desc->getFloat(0);

	mtxSetTransV(mMatrix, &mPos);
	mtxScale(mScale, mScale, mScale, mMatrix, mMatrix);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxRotateX(mMatrix, mMatrix, mRot.x);
	mtxRotateY(mSpin, mMatrix, mAngle);

	mCentre = ((PropNode*)mGeo)->mMesh->mCentre;
	mtxMultVec(mSpin, &mCentre, &mCentre);
	mRadius = ((PropNode*)mGeo)->mMesh->mRadius * mScale;
}

// 0x8010C79C
void TObjPurplePropeller::debugPrint(void*)
{
	njPrintC(0xA0013, ":ROT SPD");
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, ":SW ID(FROM)");
	njPrintC(0xA0016, ":SW ID(TO)");
	njPrintC(0xA0017, "+1.0:SCALE");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x8010C848
void TObjPurplePropeller::vfn_74()
{
	mModel.setModel(mGeo);
	mModel.mTexList = (NJS_TEXLIST*)mTex;
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mSnow1          = 0;
	mSnow0          = 0;
	if (switchesSet() == 0) {
		mSnow0       = particleGenById(this, &gVecZero, 0, 0xF5);
		mSnow1       = particleGenById(this, &gVecZero, 0, 0xF6);
		mSnow0->mPos = mPos;
		mSnow1->mPos = mPos;
		mSnow0->setRot(mRot.x, mRot.y);
		mSnow1->setRot(mRot.x, mRot.y);
		sePlay3DHeld(300.0f, &mSound, 0x234, 0, &mPos);
		mAct = 0;
	} else {
		mAct = 2;
	}
	setLinkFree((SetLink*)&mLink);
}

// 0x8010C96C
// One clear bit in mFrom..mTo keeps the propeller going
int TObjPurplePropeller::switchesSet()
{
	u16 id = mFrom;
	for (; id <= mTo; id++) {
		if (keyBitIsSet(id) == 0) {
			return 0;
		}
	}
	return 1;
}

// 0x8010C9CC
void TObjPurplePropeller::draw()
{
	if (objDebugEnabled(this) != 0) {
		mAngle += mRotSpeed;
		if (mSnow0 != 0) {
			mSnow0->mFlags |= 4;
		}
		if (mSnow1 != 0) {
			mSnow1->mFlags |= 4;
		}
	} else {
		if (!isInRange(this, &mPos, 1100.0f)) {
			mFlags |= 1;
		} else {
			switch (mAct) {
			case 0:
				spin();
				break;
			case 1:
				stopSnow();
				break;
			case 2:
				stopped();
				break;
			}
		}
	}
}

// 0x8010CAB4
void TObjPurplePropeller::spin()
{
	mAngle += mRotSpeed;
	mtxRotateY(mSpin, mMatrix, mAngle);
	if (switchesSet() != 0) {
		mAct = 1;
	}
}

// 0x8010CB14
void TObjPurplePropeller::stopSnow()
{
	if (mSnow0 != 0) {
		mSnow0->mFlags |= 1;
		mSnow0 = 0;
	}
	if (mSnow1 != 0) {
		mSnow1->mFlags |= 1;
		mSnow1 = 0;
	}
	seStop3D(&mSound);
	mAct = 2;
}

// 0x8010CB8C
void TObjPurplePropeller::stopped()
{
}

// 0x8010CB90
void TObjPurplePropeller::vfn_10()
{
	if (cameraIsVisible(1000.0f, mRadius, &mCentre, 0) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8010CBF8
void TObjPurplePropeller::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mSpin);
	mModel.draw();
	njPopMatrixEx();
}
