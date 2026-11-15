#include "object/red/TObjRedMagmaShot.h"
#include "engine/DrawTask.h"
#include "object/ObjScript.h"
#include "engine/CharaColli.h"
#include "enemy/ObjEnemy.h"
#include "object/ObjArchive.h"
#include "engine/Colli.h"
#include "object/ObjAction.h"
#include "object/TObjLauncher.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "core/MathUtil.h"
#include "core/VecMath.h"
#include "stage/SetSlot.h"
#include "sound/SoundCtrl.h"
struct CharaColli;
struct SetLink;

extern "C" {

// 0x802C3EA8
// then WAIT / ALWAYS / null at 0x802C3EAC
const char* gTObjRedMagmaShotName = "TObjRedMagmaShot";
__declspec(section ".sdata") const char* sRedMagmaShotTypeNames[] = { "WAIT", "ALWAYS", 0 };

// 0x802440D8
// the shot's one motion
const MotionEntry sRedMagmaShotMotion = { 0, 0, -1, 0, 0.0f, 30.0f, 0.7f, 0.0f, 0 };

/** The offset a new shot copies into mOffset */
struct MagmaShotOffset : public Vec3f {
	MagmaShotOffset(f32 ax, f32 ay, f32 az)
	{
		x = ax;
		y = ay;
		z = az;
	}
};

// 0x80280330
// the offset a new shot copies into mOffset
MagmaShotOffset gMagmaShotOffset(0.0f, -40.0f, 0.0f);

}

typedef char TObjRedMagmaShot_size_check[(sizeof(TObjRedMagmaShot) == 0x120) ? 1 : -1];

// 0x800A30F8
extern "C" TObject* createTObjRedMagmaShot(TObject* parent, SetDesc* desc)
{
	return new TObjRedMagmaShot(parent, desc);
}

// 0x800A314C
void TObjRedMagmaShot::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);

	mType = desc->getInt(0);
	int type = mType;
	if (type < 0) {
		type = 0;
	} else if (type > 1) {
		type = 1;
	}
	mType = type;
	mPower  = desc->getInt(1);
	mWait   = desc->getInt(2);
	mRadius = 100.0f + desc->getFloat(0);
	mTop    = 800.0f;
	mOffset = gMagmaShotOffset;
	makeShapes();
}

// 0x800A3248
void TObjRedMagmaShot::debugPrint(void*)
{
	njPrint(0xA0013, ":TYPE %s", sRedMagmaShotTypeNames[mType]);
	njPrint(0xA0014, ":POWER");
	njPrint(0xA0015, ":WAIT TIME(FRAME)");
	njPrint(0xA0017, "+%.2f:INVOKE RAD", 100.0);
}

// 0x800A32D8
int TObjRedMagmaShot::onHit(const TObjAction* other, int, int* result)
{
	if (objDebugEnabled(this) != 0) {
		return 0;
	}
	if (((TObjAction*)other)->getKind() != 3) {
		resetHitInfo((HitInfo*)result);
		((HitInfo*)result)->mDamage = mPower;
		return 1;
	}
	return 0;
}

// 0x800A3354
TObjRedMagmaShot::TObjRedMagmaShot(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00       = (u32)gTObjRedMagmaShotName;
	mSize     = 0x120;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShape    = colliCreate(2, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 0xFF, 0xFF, 0xFE);
	initFromDesc(desc);
	setup();
}

// 0x800A3430
TObjRedMagmaShot::~TObjRedMagmaShot()
{
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x800A34B4
void TObjRedMagmaShot::setup()
{
	mTimer = 0;
	mState = 0;
	_F8    = 0;
	_F4    = 0;
	mMotion.init(&sRedMagmaShotMotion, gGeObjCommon->mSet.mMotions);
	mMotion.play(0);
	mModel.setModel(getGeObjModels()[0]);
	mModel.mTexList = getGeObjTextures()[0];
	mModel.mState   = (DrawState*)getDefaultDrawState();
	mModel.mMotion  = &mMotion;
	vecAdd(&mPos, &mHome, &mOffset);
	updateMatrix();
	setLinkFree((SetLink*)&mLink);
}

// 0x800A3564
void TObjRedMagmaShot::makeShapes()
{
	ColliShape shapes[2];

	colliShapeInit(&shapes[0]);
	shapes[0].mType = COLLISHAPE_CYLINDER;
	shapes[0].mSizeX = mRadius;
	shapes[0].mSizeY = 10000.0f;
	shapes[0].mFlags |= 0x10;

	colliShapeInit(&shapes[1]);
	shapes[1].mType = COLLISHAPE_SPHERE;
	shapes[1].mSizeX = 40.0f;
	shapes[1].mFlags |= 8;

	colliCopyShapes(mShape, shapes);
	placeShapes();
}

// 0x800A35F0
void TObjRedMagmaShot::placeShapes()
{
	sRedMagmaShotShapes[0].mPos.x = sRedMagmaShotShapes[0].mPos.y = sRedMagmaShotShapes[0].mPos.z = 0.0f;
	sRedMagmaShotShapes[1].mPos   = mOffset;
	colliPlaceShapes(mShape, &mHome, sRedMagmaShotShapes);
}

// 0x800A3648
void TObjRedMagmaShot::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
}

// 0x800A3674
void TObjRedMagmaShot::draw()
{
	if (isInRange(this, &mPos, 2100.0f) == 0) {
		mFlags |= 1;
		return;
	}
	if (objDebugEnabled(this) != 0) {
		placeShapes();
		vecAdd(&mPos, &mPos, &mOffset);
		updateMatrix();
	} else {
		switch (mState) {
		case 0:
				idle();
				break;
		case 1:
				rise();
				break;
		}
	}
}

// 0x800A3724
void TObjRedMagmaShot::idle()
{
	int go = 0;

	if (mType == 0) {
		for (ColliContact* touch = mShape->mContacts; touch != 0; touch = touch->mNext) {
			ColliShape* shape = colliGetShape(touch->mOther, touch->mOtherShape);
			if ((shape->mFlags & 0x10) == 0) {
				void* other = findObjById((u16)colliContactOwner(touch));
				if (other != 0 && touch->mShape == 0) {
					go = 1;
				}
			}
		}
	} else if (mType == 1) {
		mTimer += 1;
		if (mTimer > mWait) {
			go = 1;
		}
	}

	if (go != 0) {
		sePlay(0x222);
		mTimer = 0;
		mDrawTask->mFlags &= ~1;
		mState = 1;
	}
}

// 0x800A3810
void TObjRedMagmaShot::rise()
{
	mOffset.y += 5.0f;
	if (mOffset.y > mTop) {
		mOffset = gMagmaShotOffset;
		mTimer  = 0;
		mDrawTask->mFlags |= 1;
		mState = 0;
	}
	vecAdd(&mPos, &mHome, &mOffset);
	placeShapes();
	updateMatrix();
	mMotion.update();
	mTimer += 1;
}

// Not in the DOL the linker strips it
f32 magmaShotFar() { return 2000.0f; }

// 0x800A38C4
void TObjRedMagmaShot::vfn_10()
{
	if (objDebugEnabled(this) != 0) {
		mDrawTask->mFlags &= ~1;
	} else {
		ColliShape* shape = colliGetShape(mShape, 1);
		f32 size = shape->mSizeX;
		size = 2.0f * size;
		int visible = cameraIsVisible(2000.0f, size, &mPos, 0);
		sRedMagmaShotVisible      = visible;
		if (visible != 0 && mState == 1) {
			mDrawTask->mFlags &= ~1;
		} else {
			mDrawTask->mFlags |= 1;
		}
	}
}

// 0x800A3970
void TObjRedMagmaShot::render()
{
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	mModel.draw();
	njPopMatrixEx();
	if (objDebugEnabled(this) != 0) {
		colliDrawShapes(mShape);
	}
}

