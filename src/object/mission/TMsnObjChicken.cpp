#include "object/purple/TObjPurplePropeller.h"
struct DrawTask;
#include "object/ObjAction.h"
#include "engine/DrawTask.h"
#include "camera/EventCamArc.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"
#include "engine/CharaColli.h"
#include "object/ObjShadow.h"
#include "stage/TEventCtrl.h"
#include "camera/TCamera.h"
#include "object/TObjLauncher.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "stage/TGameState.h"
#include "camera/TCameraCtrl.h"
#include "engine/Color.h"
#include "engine/Colli.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "stage/SetSlot.h"
#include "object/TObjKey00.h"
#include "core/VecMath.h"
#include "sound/SoundCtrl.h"
#include "engine/TParticle.h"
#include "object/mission/TMsnObjChicken.h"
#include "engine/LightSet.h"
#include "io/Resource.h"
#include "object/ObjArchive.h"
#include "ui/AeArc.h"

MotionEntry sMsnChickenMotions[4] = {
	{ 0, 0, -1, 0, 0.0f, 30.0f, 0.4f, 0.0f, 0 },
	{ 1, 1, -1, 0, 0.0f, 56.0f, 0.5f, 0.0f, 0 },
	{ 1, 2, -1, 0, 0.0f, 39.0f, 0.5f, 0.0f, 0 },
	{ 0, 3, -1, 0, 0.0f, 30.0f, 0.7f, 0.0f, 0 },
};
ColliShape sMsnChickenShape[1] = {
	{ 0.0f, 15.0f, 0.0f, 0x0, 0, 0x0, 30.0f, 15.0f, 0.0f, 0x0, 0x2, 0x0 },
};
struct CharaColli;
struct SetLink;
struct ColliShape;

static const char* sClassName = "TMsnObjChicken";

extern "C" {
TArc* gChickenArc;
NJS_OBJECT* gChickenModel;
NJS_TEXLIST** gChickenTexLists;
NJS_MOTION** gChickenMotions;
int sChickenLoaded;
}

// 0x8010CC3C
// Init half of the set-init pair
extern "C" void clearChickenArc()
{
	gChickenArc      = 0;
	gChickenModel    = 0;
	gChickenTexLists = 0;
	gChickenMotions  = 0;
}

static void openChickenArc();

// 0x8010CC54
// The script command load_chicken
extern "C" int loadChickenArc()
{
	openChickenArc();
	return 1;
}

// 0x8010CC78
static void openChickenArc()
{
	if (sChickenLoaded != 0) {
		return;
	}
	loadArc("geobj_chicken.arc", &gChickenArc);
	void** models    = (void**)findArcExport(gChickenArc, "models");
	gChickenModel    = (NJS_OBJECT*)models[0];
	gChickenTexLists = (NJS_TEXLIST**)findArcExport(gChickenArc, "texlists");
	gChickenMotions  = (NJS_MOTION**)findArcExport(gChickenArc, "motions");
	void* texture    = findArcExport(gChickenArc, "texture");
	for (int i = 0; i < 8; i++) {
		bindGvmTextures((u8*)texture, gChickenTexLists[i]);
	}
	sChickenLoaded = 1;
}

// 0x8010CD30
extern "C" void termChickenArc()
{
	if (gChickenArc != 0) {
		for (int i = 7; i >= 0; i--) {
			releaseTextures(gChickenTexLists[i]);
		}
		releaseResource(gChickenArc);
		gChickenArc      = 0;
		gChickenModel    = 0;
		gChickenTexLists = 0;
		gChickenMotions  = 0;
	}
	sChickenLoaded = 0;
}

enum {
	CHICKEN_WAIT  = 0, // for a rider
	CHICKEN_TAKE  = 1, // the pick-up event
	CHICKEN_LEAVE = 2,
	CHICKEN_FLY   = 3,
};

// Message 0x7D where the rider sits
struct ChickenSeatMsg {
	u16 mType;  // _00
	Vec3f mPos; // _04
	int mRotY;  // _10
};

// 0x8010CDA4
extern "C" TObject* createTMsnObjChicken(TObject* parent, SetDesc* desc)
{
	return new TMsnObjChicken(parent, desc);
}

// 0x8010CDF8
// Int 0 is the type 0..7 and picks a model from the loaded list
void TMsnObjChicken::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);
	placeMatrix();
	mType         = CLAMP_RANGE(desc->getInt(0), 0, 7);
	mUnlockSwitch = (u16)CLAMP_RANGE(desc->getInt(1), 0, 0x800);
	if (gChickenTexLists != 0) {
		mModel.mTexList = (NJS_TEXLIST*)gChickenTexLists[mType];
	}
}

// 0x8010CF24
// The lines are the set-file labels
void TMsnObjChicken::debugPrint(void*)
{
	njPrintC(0xA0013, ":TYPE(0-7)");
	njPrintC(0xA0014, ":UNLOCK SWITCH");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x8010CFC8
// Plays the loaded motion builds the shape and plants the shadow
extern "C" void chickenBuild(TMsnObjChicken* self)
{
	self->mRiderAngle = 0;
	self->mState      = CHICKEN_WAIT;
	self->mCamera     = 0;
	self->mTimer      = 0;
	if (gChickenArc != 0) {
		self->mMotion.init(sMsnChickenMotions, gChickenMotions);
		self->mMotion.play(0);
		self->mModel.setModel(gChickenModel);
		self->mModel.mMotion = &self->mMotion;
		self->mModel.mState  = (DrawState*)getDefaultDrawState();
		self->mModel.calcMotion();
	}

	self->mShape = colliCreate(1, 1);
	colliSetOwner(self->mShape, (u16)self->mObjId);
	colliSetMasks((CharaColli*)self->mShape, 4, 255, 255, 0xFE);
	colliCopyShapes(self->mShape, sMsnChickenShape);
	colliPlaceShapes(self->mShape, &self->mPos, sMsnChickenShape);

	self->mShadow._30 = 10.0f;
	self->mShadow._34 = 10.0f;
	Vec3f up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	shadowSetGround(&self->mShadow, &self->mPos, &up);
	setLinkFree((SetLink*)&self->mLink);
	self->mLights = lightSetFindArea(&self->mPos);
}

// 0x8010D0F0
TMsnObjChicken::TMsnObjChicken(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)sClassName;
	mSize = 0x154;
	initFromDesc(desc);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 33, 0);
	chickenBuild(this);
}

// 0x8010D19C
TMsnObjChicken::~TMsnObjChicken()
{
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x8010D238
void TMsnObjChicken::placeMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
}

// 0x8010D27C
// The event's hide callback hides the rider
static int hideRider(int id)
{
	TObjAction* obj = findObjById(id);
	if (obj == 0) {
		return 0;
	}
	eventHideIdU16(((TMsnObjChicken*)obj)->mRider.getId());
	return 1;
}

// 0x8010D2C0
void TMsnObjChicken::waitRider()
{
	if (keyBitIsSet(mUnlockSwitch) == 1) {
		for (ColliContact* contact = ((CharaColli*)mShape)->mContacts; contact != 0; contact = contact->mNext) {
			if (colliContactInGroup1(contact) == 0 || colliContactNoBit10(contact) == 0) {
				continue;
			}
			TObjAction* other = findObjById(colliContactOwner(contact));
			if (other == 0) {
				continue;
			}
			ObjMsg ask;
			ask.mType = 0x7B;
			if (other->sendToHandle(&ask) == 0) {
				continue;
			}
			mRider.setId(colliContactOwnerU16(contact));
			mRiderAngle = angleToXZ(&mPos, other->getPos());
			if (eventStartById(mObjId, hideRider) != 0) {
				mMotion.play(1);
				mCamera = eventCamArcPlay(1.0f, 0.0f, 7);
				if (mCamera != 0) {
					mCamera->attachToMtx();
					f32* mtx = mCamera->getMtx();
					mtxSetTransV(mtx, &mPos);
					mtxRotateY(mtx, mtx, mRot.y);
				}
				if (mRider.get() != 0) {
					ChickenSeatMsg seat;
					seat.mType  = 0x7D;
					seat.mPos.x = 15.0f;
					seat.mPos.y = 0.0f;
					seat.mPos.z = 0.0f;
					vecRotateY(&seat.mPos, &seat.mPos, mRot.y);
					vecAdd(&seat.mPos, &seat.mPos, &mPos);
					seat.mRotY = mRot.y;
					mRider.get()->sendToHandle((ObjMsg*)&seat);
				}
				mState = CHICKEN_TAKE;
			}
		}
	}
}

// 0x8010D48C
void TMsnObjChicken::takeRider()
{
	if (mTimer == 0) {
		sePlay(0x63);
	}
	if (mMotion.isDone() && mCamera != 0 && mCamera->isDone() != 0) {
		if (mCamera != 0) {
			mCamera->mFlags |= 1;
			mCamera = 0;
		}
		eventEnd();
		TObjAction* rider = (TObjAction*)mRider.get();
		if (rider != 0) {
			ObjMsg off;
			off.mType = 0x79;
			rider->sendToHandle(&off);
		}
		ObjMsg done;
		done.mType = 0xA3;
		sendGameMsg(&done);
		mMotion.play(2);
		mTimer = 0;
		mState = CHICKEN_LEAVE;
	}
	mTimer++;
}

// 0x8010D578
void TMsnObjChicken::leave()
{
	if (mMotion.isDone()) {
		mMotion.play(3);
		mState = CHICKEN_FLY;
	}
}

// 0x8010D5C4
void TMsnObjChicken::fly()
{
	f64 h      = __fabs(mPos.y - mHome.y);
	f32 height = h;
	if (height > 100.0f) {
		setLinkForever((SetLink*)&mLink);
		mFlags |= 1;
	}
	if (height < 15.0f) {
		mPos.y += 0.2f;
	} else {
		mPos.y += 0.1f * height + 0.04f;
	}
	switch (mTimer) {
	case 12:
	case 27:
		particleOnceByIdDefault(&mPos, 0, 0x17A);
		break;
	}
	mTimer++;
	placeMatrix();
}

// 0x8010D6A8
void TMsnObjChicken::draw()
{
	if (objDebugEnabled(this) == 0 && isInRange(this, &mPos, 700.0f) && gChickenArc != 0) {
		switch (mState) {
		case CHICKEN_WAIT:
			waitRider();
			break;
		case CHICKEN_TAKE:
			takeRider();
			break;
		case CHICKEN_LEAVE:
			leave();
			break;
		case CHICKEN_FLY:
			fly();
			break;
		}
		mMotion.update();
		mModel.calcMotion();
	}
}

// 0x8010D768
void TMsnObjChicken::vfn_10()
{
	if (cameraIsVisible(800.0f, 15.0f, &mPos, 0) != 0) {
		mDrawTask->mFlags &= ~1;
		shadowEnable(&mShadow);
	} else {
		mDrawTask->mFlags |= 1;
		shadowDisable(&mShadow);
	}
}

// 0x8010D7E0
// Without the model it draws a black box
void TMsnObjChicken::render()
{
	if (gChickenModel == 0) {
		f32 mtx[12];
		mtxConcat(mtx, (f32*)gjGetMatrix(), mMatrix);
		mtxTranslate(mtx, mtx, 0.0f, 12.5f, 0.0f);
		mtxScale(25.0f, 25.0f, 25.0f, mtx, mtx);
		debugWireSphere(mtx, &gColorBlack);
	} else {
		shadowAlphaBegin();
		njPushMatrixEx();
		njMultiMatrix(0, mMatrix);
		lightSetSelect(mLights);
		mModel.draw();
		njPopMatrixEx();
		shadowAlphaEnd();
		if (objDebugEnabled(this) != 0) {
			colliDrawShapes(mShape);
		}
	}
}

// 0x8010D8AC
int TMsnObjChicken::getKind()
{
	return 3;
}
